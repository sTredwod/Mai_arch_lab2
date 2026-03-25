#include "users_create.hpp"

#include <stdexcept>
#include <string>

#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/server/request/request_context.hpp>

#include "../storage/library_storage_component.hpp"

namespace library {

UsersCreateHandler::UsersCreateHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context)
    : HttpHandlerBase(config, component_context),
      storage_(
          component_context.FindComponent<LibraryStorageComponent>().GetStorage()) {
}

std::string UsersCreateHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  request.GetHttpResponse().SetContentType("application/json");

  userver::formats::json::Value body;
  try {
    body = userver::formats::json::FromString(request.RequestBody());
  } catch (const std::exception&) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return R"({"error":"invalid json"})";
  }

  try {
    if (!body.HasMember("login") || !body.HasMember("password") ||
        !body.HasMember("first_name") || !body.HasMember("last_name")) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"missing required fields"})";
    }

    const auto login = body["login"].As<std::string>();
    const auto password = body["password"].As<std::string>();
    const auto first_name = body["first_name"].As<std::string>();
    const auto last_name = body["last_name"].As<std::string>();

    if (login.empty() || password.empty() || first_name.empty() ||
        last_name.empty()) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"fields must not be empty"})";
    }

    const auto created_user =
        storage_.CreateUser(login, password, first_name, last_name);

    if (!created_user) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kConflict);
      return R"({"error":"user with this login already exists"})";
    }

    userver::formats::json::ValueBuilder response;
    response["id"] = created_user->id;
    response["login"] = created_user->login;
    response["first_name"] = created_user->first_name;
    response["last_name"] = created_user->last_name;

    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kCreated);
    return userver::formats::json::ToString(response.ExtractValue());
  } catch (const std::exception&) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return R"({"error":"invalid field types"})";
  }
}

}  // namespace library
