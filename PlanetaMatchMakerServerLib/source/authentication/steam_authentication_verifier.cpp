#include "authentication/steam_authentication_verifier.hpp"

#include <charconv>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>

#include <boost/json.hpp>

#include "authentication/authentication_http_client.hpp"

namespace pgl {
	namespace {
		namespace json = boost::json;

		std::string to_hex(const std::vector<uint8_t>& bytes) {
			std::ostringstream stream;
			stream << std::hex << std::setfill('0');
			for (const auto byte : bytes) { stream << std::setw(2) << static_cast<int>(byte); }
			return stream.str();
		}

		std::optional<std::string> get_json_string(const json::object& object, const std::string& key) {
			const auto* value = object.if_contains(key);
			if (value == nullptr || !value->is_string()) { return std::nullopt; }
			return std::string(value->as_string().c_str());
		}

		std::optional<bool> get_json_bool(const json::object& object, const std::string& key) {
			const auto* value = object.if_contains(key);
			if (value == nullptr || !value->is_bool()) { return std::nullopt; }
			return value->as_bool();
		}

		const json::object* get_child_object(const json::object& object, const std::string& key) {
			const auto* value = object.if_contains(key);
			if (value == nullptr) { return nullptr; }
			return value->if_object();
		}

		authentication_verification_result failure(const authentication_result result) {
			return {result, std::nullopt};
		}

		authentication_verification_result success(authenticated_identity identity) {
			return {authentication_result::success, std::move(identity)};
		}

		uint64_t parse_canonical_steam_id64(const std::string& value) {
			if (value.empty() || value.size() > authentication_provider_user_id_bytes) {
				throw std::invalid_argument("SteamID64 has an invalid length.");
			}
			uint64_t parsed = 0;
			const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), parsed);
			if (error != std::errc{} || end != value.data() + value.size() || std::to_string(parsed) != value) {
				throw std::invalid_argument("SteamID64 is not a canonical decimal uint64 value.");
			}
			return parsed;
		}

		std::u8string as_u8string(const std::string& value) {
			std::u8string result;
			result.reserve(value.size());
			for (const auto character : value) { result.push_back(static_cast<char8_t>(character)); }
			return result;
		}

		class steam_authentication_verifier final : public authentication_credential_verifier {
		public:
			[[nodiscard]] authentication_method method() const override {
				return authentication_method::steam;
			}

			[[nodiscard]] authentication_verification_result verify(
				const std::vector<uint8_t>& credential,
				const server_authentication_setting& setting,
				const authentication_execution_context& context) const override {
				const auto& steam = setting.steam;
				try {
					auto authenticate_url = steam.authenticate_user_ticket_url;
					authenticate_url = authentication_http::append_query(authenticate_url, "key", steam.publisher_key);
					authenticate_url = authentication_http::append_query(authenticate_url, "appid",
						std::to_string(steam.app_id));
					authenticate_url = authentication_http::append_query(authenticate_url, "ticket", to_hex(credential));
					if (!steam.identity.empty()) {
						authenticate_url = authentication_http::append_query(authenticate_url, "identity", steam.identity);
					}

					const auto auth_response = authentication_http::get(authenticate_url, context);
					if (auth_response.status < 200 || auth_response.status >= 300) {
						return failure(authentication_result::steam_authentication_service_unavailable);
					}

					const auto auth_json = json::parse(auth_response.body);
					const auto* root = auth_json.if_object();
					const auto* response = root == nullptr ? nullptr : get_child_object(*root, "response");
					const auto* params = response == nullptr ? nullptr : get_child_object(*response, "params");
					if (params == nullptr) { params = response; }
					if (params == nullptr) { return failure(authentication_result::steam_ticket_invalid); }

					if (const auto result = get_json_string(*params, "result");
						result.has_value() && *result != "OK") {
						return failure(authentication_result::steam_ticket_invalid);
					}

					const auto steam_id_string = get_json_string(*params, "steamid");
					if (!steam_id_string.has_value()) { return failure(authentication_result::steam_ticket_invalid); }
					parse_canonical_steam_id64(*steam_id_string);

					auto ownership_url = steam.check_app_ownership_url;
					ownership_url = authentication_http::append_query(ownership_url, "key", steam.publisher_key);
					ownership_url = authentication_http::append_query(ownership_url, "steamid", *steam_id_string);
					ownership_url = authentication_http::append_query(ownership_url, "appid", std::to_string(steam.app_id));
					const auto ownership_response = authentication_http::get(ownership_url, context);
					if (ownership_response.status < 200 || ownership_response.status >= 300) {
						return failure(authentication_result::steam_authentication_service_unavailable);
					}

					const auto ownership_json = json::parse(ownership_response.body);
					const auto* ownership_root = ownership_json.if_object();
					const auto* ownership_response_obj =
						ownership_root == nullptr ? nullptr : get_child_object(*ownership_root, "response");
					const auto* appownership =
						ownership_root == nullptr ? nullptr : get_child_object(*ownership_root, "appownership");
					const auto* ownership_source = appownership != nullptr ? appownership : ownership_response_obj;
					if (ownership_source == nullptr) { ownership_source = ownership_root; }
					const auto owns_app = ownership_source == nullptr
						                      ? std::nullopt
						                      : get_json_bool(*ownership_source, "ownsapp");
					if (!owns_app.value_or(false)) {
						return failure(authentication_result::steam_ownership_check_failed);
					}

					authenticated_identity identity;
					identity.authentication_provider_user_id =
						authentication_provider_user_id_t(as_u8string(*steam_id_string));
					return success(std::move(identity));
				}
				catch (const std::invalid_argument&) {
					return failure(authentication_result::steam_ticket_invalid);
				}
				catch (const std::out_of_range&) {
					return failure(authentication_result::steam_ticket_invalid);
				}
				catch (const std::exception&) {
					return failure(authentication_result::steam_authentication_service_unavailable);
				}
			}
		};
	}

	std::unique_ptr<authentication_credential_verifier> make_steam_authentication_verifier() {
		return std::make_unique<steam_authentication_verifier>();
	}

	p2p_service_peer_id_t derive_steam_p2p_service_peer_id(
		const authentication_provider_user_id_t& authentication_provider_user_id) {
		const auto value = authentication_provider_user_id.to_string();
		const std::string ascii(value.begin(), value.end());
		parse_canonical_steam_id64(ascii);
		return p2p_service_peer_id_t(value);
	}
}
