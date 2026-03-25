#include "users_get.hpp"

#include <string>

#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/server/request/request_context.hpp>

#include "../storage/library_storage_component.hpp"

namespace library {

namespace {

userver::formats::json::Value MakeUserJson(const User& user) {
  userver::formats::json::ValueBuilder builder;
  builder["id"] = user.id;
  builder["login"] = user.login;
  builder["first_name"] = user.first_name;
  builder["last_name"] = user.last_name;
  return builder.ExtractValue();
}

}  // namespace

UsersGetHandler::UsersGetHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context)
    : HttpHandlerBase(config, component_context),
      storage_(
          component_context.FindComponent<LibraryStorageComponent>().GetStorage()) {
}

std::string UsersGetHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  request.GetHttpResponse().SetContentType("application/json");

  const auto login = request.GetArg("login");
  const auto name_mask = request.GetArg("name_mask");

  const bool has_login = !login.empty();
  const bool has_name_mask = !name_mask.empty();

  if (has_login == has_name_mask) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return R"({"error":"use exactly one query parameter: login or name_mask"})";
  }

  if (has_login) {
    const auto user = storage_.GetUserByLogin(login);
    if (!user) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kNotFound);
      return R"({"error":"user not found"})";
    }

    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(MakeUserJson(*user));
  }

  const auto users = storage_.FindUsersByNameMask(name_mask);
  userver::formats::json::ValueBuilder response(userver::formats::common::Type::kArray);
  for (const auto& user : users) {
    response.PushBack(MakeUserJson(user));
  }

  request.GetHttpResponse().SetStatus(
      userver::server::http::HttpStatus::kOk);
  return userver::formats::json::ToString(response.ExtractValue());
}

}  // namespace library
