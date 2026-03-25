#include "auth_login.hpp"

#include <stdexcept>
#include <string>

#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/server/request/request_context.hpp>

#include "../storage/library_storage_component.hpp"

namespace library {

AuthLoginHandler::AuthLoginHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context)
    : HttpHandlerBase(config, component_context),
      storage_(
          component_context.FindComponent<LibraryStorageComponent>().GetStorage()) {
}

std::string AuthLoginHandler::HandleRequestThrow(
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
    if (!body.HasMember("login") || !body.HasMember("password")) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"missing required fields"})";
    }

    const auto login = body["login"].As<std::string>();
    const auto password = body["password"].As<std::string>();

    if (login.empty() || password.empty()) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"fields must not be empty"})";
    }

    const auto token = storage_.Login(login, password);
    if (!token) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kUnauthorized);
      return R"({"error":"invalid login or password"})";
    }

    userver::formats::json::ValueBuilder response;
    response["token"] = *token;

    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(response.ExtractValue());
  } catch (const std::exception&) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return R"({"error":"invalid field types"})";
  }
}

}  // namespace library
