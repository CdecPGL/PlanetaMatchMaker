#include "authentication/steam_authentication_verifier.hpp"

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

		std::string player_name_to_display_name(const player_name_t& player_name) {
			const auto name = player_name.to_string();
			return reinterpret_cast<const char*>(name.c_str());
		}

		game_host_external_id_t steam_external_id_from_steam_id64(const uint64_t steam_id64) {
			game_host_external_id_t external_id{};
			for (auto i = 0u; i < sizeof(uint64_t); ++i) {
				external_id[i] = static_cast<uint8_t>(steam_id64 >> ((sizeof(uint64_t) - 1 - i) * 8));
			}
			return external_id;
		}

		class steam_authentication_verifier final : public authentication_credential_verifier {
		public:
			[[nodiscard]] authentication_method method() const override {
				return authentication_method::steam;
			}

			[[nodiscard]] authentication_verification_result verify(
				const std::vector<uint8_t>& credential,
				const player_name_t& player_name,
				const server_authentication_setting& setting) const override {
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

					const auto auth_response =
						authentication_http::get(authenticate_url, std::chrono::seconds(setting.timeout_seconds));
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
					const auto steam_id64 = std::stoull(*steam_id_string);

					auto ownership_url = steam.check_app_ownership_url;
					ownership_url = authentication_http::append_query(ownership_url, "key", steam.publisher_key);
					ownership_url = authentication_http::append_query(ownership_url, "steamid", *steam_id_string);
					ownership_url = authentication_http::append_query(ownership_url, "appid", std::to_string(steam.app_id));
					const auto ownership_response =
						authentication_http::get(ownership_url, std::chrono::seconds(setting.timeout_seconds));
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
					identity.method = authentication_method::steam;
					identity.verified_user_id = *steam_id_string;
					identity.external_id = steam_external_id_from_steam_id64(steam_id64);
					identity.display_name = player_name_to_display_name(player_name);
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
}
