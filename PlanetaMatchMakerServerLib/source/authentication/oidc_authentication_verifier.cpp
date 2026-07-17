#include "authentication/oidc_authentication_verifier.hpp"

#include <algorithm>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <unordered_map>

#include <boost/json.hpp>
#include <jwt-cpp/traits/boost-json/defaults.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/param_build.h>
#include <openssl/pem.h>

#include "authentication/authentication_http_client.hpp"

namespace pgl {
	namespace {
		namespace json = boost::json;
		using jwt_json_traits = jwt::traits::boost_json;
		using default_jwk = jwt::jwk<jwt_json_traits>;

		struct cached_oidc_jwks final {
			std::string jwks_url;
			std::string body;
			std::chrono::steady_clock::time_point expires_at{};
		};

		std::mutex oidc_jwks_cache_mutex;
		std::unordered_map<std::string, cached_oidc_jwks> oidc_jwks_cache;

		std::optional<std::string> get_json_string(const json::object& object, const std::string& key) {
			const auto* value = object.if_contains(key);
			if (value == nullptr || !value->is_string()) { return std::nullopt; }
			return std::string(value->as_string().c_str());
		}

		std::string make_oidc_jwks_cache_key(const server_authentication_setting::oidc_setting& oidc) {
			if (!oidc.jwks_url.empty()) { return "jwks_url\n" + oidc.jwks_url; }
			return "discovery_url\n" + oidc.discovery_url + "\nissuer\n" + oidc.issuer;
		}

		std::optional<cached_oidc_jwks> get_cached_oidc_jwks(const std::string& cache_key) {
			const std::lock_guard lock(oidc_jwks_cache_mutex);
			const auto found = oidc_jwks_cache.find(cache_key);
			if (found == oidc_jwks_cache.end()) { return std::nullopt; }
			if (found->second.expires_at <= std::chrono::steady_clock::now()) {
				oidc_jwks_cache.erase(found);
				return std::nullopt;
			}
			return found->second;
		}

		void cache_oidc_jwks(const std::string& cache_key, const std::string& jwks_url,
			const std::string& body, const uint32_t cache_seconds) {
			if (cache_seconds == 0) { return; }
			const std::lock_guard lock(oidc_jwks_cache_mutex);
			oidc_jwks_cache[cache_key] = {
				jwks_url,
				body,
				std::chrono::steady_clock::now() + std::chrono::seconds(cache_seconds)
			};
		}

		authentication_verification_result failure(const authentication_result result) {
			return {result, std::nullopt};
		}

		authentication_verification_result success(authenticated_identity identity) {
			return {authentication_result::success, std::move(identity)};
		}

		std::string player_name_to_display_name(const player_name_t& player_name) {
			const auto name = player_name.to_string();
			return reinterpret_cast<const char*>(name.c_str());
		}

		std::string pem_from_x5c(const std::string& x5c) {
			std::error_code error;
			auto pem = jwt::helper::convert_base64_der_to_pem(x5c, error);
			if (error) { throw std::runtime_error("Failed to convert JWK x5c certificate."); }
			auto public_key = jwt::helper::extract_pubkey_from_cert(pem, "", error);
			if (error) { throw std::runtime_error("Failed to extract public key from JWK x5c certificate."); }
			return public_key;
		}

		std::string base64url_decode(const std::string& value) {
			return jwt::base::decode<jwt::alphabet::base64url>(jwt::base::pad<jwt::alphabet::base64url>(value));
		}

		std::string pem_from_rsa_components(const std::string& modulus, const std::string& exponent) {
			const auto n_raw = base64url_decode(modulus);
			const auto e_raw = base64url_decode(exponent);

			BIGNUM* n = BN_bin2bn(reinterpret_cast<const unsigned char*>(n_raw.data()),
				static_cast<int>(n_raw.size()), nullptr);
			BIGNUM* e = BN_bin2bn(reinterpret_cast<const unsigned char*>(e_raw.data()),
				static_cast<int>(e_raw.size()), nullptr);
			if (n == nullptr || e == nullptr) {
				BN_free(n);
				BN_free(e);
				throw std::runtime_error("Failed to read RSA JWK components.");
			}

			EVP_PKEY_CTX* context = EVP_PKEY_CTX_new_from_name(nullptr, "RSA", nullptr);
			OSSL_PARAM_BLD* parameter_builder = OSSL_PARAM_BLD_new();
			if (context == nullptr || parameter_builder == nullptr ||
				EVP_PKEY_fromdata_init(context) != 1 ||
				OSSL_PARAM_BLD_push_BN(parameter_builder, OSSL_PKEY_PARAM_RSA_N, n) != 1 ||
				OSSL_PARAM_BLD_push_BN(parameter_builder, OSSL_PKEY_PARAM_RSA_E, e) != 1) {
				BN_free(n);
				BN_free(e);
				OSSL_PARAM_BLD_free(parameter_builder);
				EVP_PKEY_CTX_free(context);
				throw std::runtime_error("Failed to create RSA public key.");
			}

			OSSL_PARAM* parameters = OSSL_PARAM_BLD_to_param(parameter_builder);
			EVP_PKEY* pkey = nullptr;
			if (parameters == nullptr ||
				EVP_PKEY_fromdata(context, &pkey, EVP_PKEY_PUBLIC_KEY, parameters) != 1) {
				OSSL_PARAM_free(parameters);
				OSSL_PARAM_BLD_free(parameter_builder);
				EVP_PKEY_CTX_free(context);
				EVP_PKEY_free(pkey);
				BN_free(n);
				BN_free(e);
				throw std::runtime_error("Failed to create EVP public key.");
			}
			OSSL_PARAM_free(parameters);
			OSSL_PARAM_BLD_free(parameter_builder);
			EVP_PKEY_CTX_free(context);
			BN_free(n);
			BN_free(e);

			BIO* bio = BIO_new(BIO_s_mem());
			if (bio == nullptr || PEM_write_bio_PUBKEY(bio, pkey) != 1) {
				BIO_free(bio);
				EVP_PKEY_free(pkey);
				throw std::runtime_error("Failed to write RSA public key.");
			}

			BUF_MEM* buffer = nullptr;
			BIO_get_mem_ptr(bio, &buffer);
			std::string pem(buffer->data, buffer->length);
			BIO_free(bio);
			EVP_PKEY_free(pkey);
			return pem;
		}

		std::string public_key_pem_from_jwk(const default_jwk& jwk) {
			if (jwk.has_x5c()) { return pem_from_x5c(jwk.get_x5c_key_value()); }
			if (!jwk.has_key_type() || jwk.get_key_type() != "RSA") {
				throw std::runtime_error("JWK key type is not RSA.");
			}
			return pem_from_rsa_components(
				jwk.get_jwk_claim("n").as_string(),
				jwk.get_jwk_claim("e").as_string());
		}

		std::string fetch_oidc_jwks(const server_authentication_setting& setting,
			const authentication_execution_context& context,
			const bool force_refresh = false) {
			const auto& oidc = setting.oidc;
			if (!oidc.jwks.empty()) { return oidc.jwks; }

			const auto cache_key = make_oidc_jwks_cache_key(oidc);
			const auto cached = oidc.jwks_cache_seconds > 0
				                    ? get_cached_oidc_jwks(cache_key)
				                    : std::nullopt;
			if (cached.has_value() && !force_refresh) { return cached->body; }

			auto jwks_url = oidc.jwks_url;
			if (jwks_url.empty()) {
				if (cached.has_value()) {
					jwks_url = cached->jwks_url;
				}
				else {
					const auto discovery_response = authentication_http::get(oidc.discovery_url, context);
					if (discovery_response.status < 200 || discovery_response.status >= 300) {
						throw std::runtime_error("Failed to fetch OIDC discovery document.");
					}
					const auto discovery_json = json::parse(discovery_response.body);
					const auto* discovery_obj = discovery_json.if_object();
					const auto discovered_issuer =
						discovery_obj == nullptr ? std::nullopt : get_json_string(*discovery_obj, "issuer");
					if (!discovered_issuer.has_value()) {
						throw std::runtime_error("OIDC discovery document has no issuer.");
					}
					if (*discovered_issuer != oidc.issuer) {
						throw std::runtime_error("OIDC discovery issuer does not match configured issuer.");
					}
					const auto discovered_jwks_url = get_json_string(*discovery_obj, "jwks_uri");
					if (!discovered_jwks_url.has_value()) {
						throw std::runtime_error("OIDC discovery document has no jwks_uri.");
					}
					jwks_url = *discovered_jwks_url;
				}
			}

			const auto jwks_response = authentication_http::get(jwks_url, context);
			if (jwks_response.status < 200 || jwks_response.status >= 300) {
				throw std::runtime_error("Failed to fetch OIDC JWKS.");
			}

			cache_oidc_jwks(cache_key, jwks_url, jwks_response.body, oidc.jwks_cache_seconds);
			return jwks_response.body;
		}

		default_jwk select_jwk(const std::string& jwks_body, const std::string& kid) {
			const auto jwks = jwt::parse_jwks<jwt_json_traits>(jwks_body);
			if (!kid.empty()) { return jwks.get_jwk(kid); }
			if (jwks.begin() == jwks.end()) { throw std::runtime_error("JWKS has no keys."); }
			return *jwks.begin();
		}

		default_jwk select_jwk_with_refresh(
			const server_authentication_setting& setting,
			const authentication_execution_context& context,
			const std::string& kid) {
			const auto jwks_body = fetch_oidc_jwks(setting, context);
			try { return select_jwk(jwks_body, kid); }
			catch (const std::exception&) {
				if (kid.empty() || !setting.oidc.jwks.empty()) { throw; }
			}

			return select_jwk(fetch_oidc_jwks(setting, context, true), kid);
		}

		authentication_result verify_signature_and_claims(
			const jwt::decoded_jwt<jwt_json_traits>& decoded,
			const std::string& pem,
			const server_authentication_setting& setting) {
			try {
				auto verifier = jwt::verify<jwt_json_traits>()
					.leeway(setting.clock_skew_seconds)
					.with_issuer(setting.oidc.issuer)
					.with_audience(setting.oidc.audience);

				const auto algorithm = decoded.get_algorithm();
				if (algorithm == "RS256") {
					verifier.allow_algorithm(jwt::algorithm::rs256(pem));
				}
				else if (algorithm == "RS384") {
					verifier.allow_algorithm(jwt::algorithm::rs384(pem));
				}
				else if (algorithm == "RS512") {
					verifier.allow_algorithm(jwt::algorithm::rs512(pem));
				}
				else {
					return authentication_result::oidc_disallowed_algorithm;
				}

				verifier.verify(decoded);
				return authentication_result::success;
			}
			catch (const jwt::error::signature_verification_exception&) {
				return authentication_result::oidc_signature_verification_failed;
			}
			catch (const jwt::error::token_verification_exception& e) {
				if (e.code() == jwt::error::token_verification_error::audience_missmatch) {
					return authentication_result::oidc_audience_mismatch;
				}
				if (e.code() == jwt::error::token_verification_error::token_expired) {
					return authentication_result::oidc_token_expired;
				}
				if (e.code() == jwt::error::token_verification_error::wrong_algorithm) {
					return authentication_result::oidc_disallowed_algorithm;
				}
				return authentication_result::oidc_token_invalid;
			}
			catch (const std::exception&) {
				return authentication_result::oidc_signature_verification_failed;
			}
		}

		class oidc_authentication_verifier final : public authentication_credential_verifier {
		public:
			[[nodiscard]] authentication_method method() const override {
				return authentication_method::oidc;
			}

			[[nodiscard]] authentication_verification_result verify(
				const std::vector<uint8_t>& credential,
				const player_name_t& player_name,
				const server_authentication_setting& setting,
				const authentication_execution_context& context) const override {
				try {
					const std::string token(reinterpret_cast<const char*>(credential.data()), credential.size());
					const auto decoded = [&token] {
						try { return jwt::decode<jwt_json_traits>(token); }
						catch (const std::exception&) {
							throw std::invalid_argument("OIDC token is not a valid JWT.");
						}
					}();
					const auto algorithm = decoded.get_algorithm();
					if (std::ranges::find(setting.oidc.algorithms, algorithm) == setting.oidc.algorithms.end()) {
						return failure(authentication_result::oidc_disallowed_algorithm);
					}
					if (!decoded.has_issuer() || decoded.get_issuer() != setting.oidc.issuer) {
						return failure(authentication_result::oidc_issuer_mismatch);
					}
					if (!decoded.has_audience() ||
						!decoded.get_audience().contains(setting.oidc.audience)) {
						return failure(authentication_result::oidc_audience_mismatch);
					}
					if (!decoded.has_expires_at()) { return failure(authentication_result::oidc_token_expired); }
					if (!decoded.has_subject() || decoded.get_subject().empty()) {
						return failure(authentication_result::oidc_subject_missing);
					}

					const auto kid = decoded.has_header_claim("kid")
						                 ? decoded.get_header_claim("kid").as_string()
						                 : std::string();
					const auto jwk = select_jwk_with_refresh(setting, context, kid);
					if (jwk.has_algorithm() && jwk.get_algorithm() != algorithm) {
						return failure(authentication_result::oidc_disallowed_algorithm);
					}
					const auto pem = public_key_pem_from_jwk(jwk);
					const auto verify_result = verify_signature_and_claims(decoded, pem, setting);
					if (verify_result != authentication_result::success) { return failure(verify_result); }

					authenticated_identity identity;
					identity.method = authentication_method::oidc;
					identity.verified_user_id = decoded.get_subject();
					identity.display_name = player_name_to_display_name(player_name);
					identity.oidc_issuer = decoded.get_issuer();
					identity.oidc_subject = decoded.get_subject();
					identity.oidc_audience = setting.oidc.audience;

					try {
						if (decoded.has_payload_claim("name")) {
							identity.display_name = decoded.get_payload_claim("name").as_string();
						}
						else if (decoded.has_payload_claim("preferred_username")) {
							identity.display_name = decoded.get_payload_claim("preferred_username").as_string();
						}
					}
					catch (const std::exception&) { }

					if (identity.verified_user_id.size() <= std::tuple_size_v<game_host_external_id_t>) {
						game_host_external_id_t external_id{};
						std::ranges::copy(identity.verified_user_id, external_id.begin());
						identity.external_id = external_id;
					}
					return success(std::move(identity));
				}
				catch (const std::invalid_argument&) {
					return failure(authentication_result::oidc_token_invalid);
				}
				catch (const jwt::error::signature_verification_exception&) {
					return failure(authentication_result::oidc_signature_verification_failed);
				}
				catch (const std::exception&) {
					return failure(authentication_result::oidc_key_fetch_failed);
				}
			}
		};
	}

	std::unique_ptr<authentication_credential_verifier> make_oidc_authentication_verifier() {
		return std::make_unique<oidc_authentication_verifier>();
	}
}
