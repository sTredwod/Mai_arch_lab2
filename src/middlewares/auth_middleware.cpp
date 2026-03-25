#include "auth_middleware.hpp"

#include <memory>
#include <string>

#include <userver/server/http/http_status.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include "../storage/library_storage_component.hpp"

namespace library {

AuthMiddleware::AuthMiddleware(
    const userver::server::handlers::HttpHandlerBase&,
    LibraryStorage& storage)
    : storage_(storage) {
}

bool AuthMiddleware::RequiresAuth(
    const userver::server::http::HttpRequest& request) {
  const auto& method = request.GetMethodStr();
  const auto& path = request.GetRequestPath();

  if (method == "POST" && (path == "/books" || path == "/loans")) {
    return true;
  }

  if (method == "PATCH" &&
      path.rfind("/loans/", 0) == 0 &&
      path.ends_with("/return")) {
    return true;
  }

  return false;
}

void AuthMiddleware::HandleRequest(
    userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {
  if (!RequiresAuth(request)) {
    Next(request, context);
    return;
  }

  auto& response = request.GetHttpResponse();
  response.SetContentType("application/json");

  const auto& header = request.GetHeader("Authorization");
  const std::string prefix = "Bearer ";

  if (header.empty()) {
    response.SetStatus(userver::server::http::HttpStatus::kUnauthorized);
    response.SetData(R"({"error":"missing Authorization header"})");
    return;
  }

  if (header.rfind(prefix, 0) != 0) {
    response.SetStatus(userver::server::http::HttpStatus::kUnauthorized);
    response.SetData(R"({"error":"invalid Authorization header"})");
    return;
  }

  const std::string token = header.substr(prefix.size());
  if (token.empty() || !storage_.IsTokenValid(token)) {
    response.SetStatus(userver::server::http::HttpStatus::kUnauthorized);
    response.SetData(R"({"error":"invalid token"})");
    return;
  }

  Next(request, context);
}

AuthMiddlewareFactory::AuthMiddlewareFactory(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::middlewares::HttpMiddlewareFactoryBase(config, context),
      storage_(context.FindComponent<LibraryStorageComponent>().GetStorage()) {
}

std::unique_ptr<userver::server::middlewares::HttpMiddlewareBase>
AuthMiddlewareFactory::Create(
    const userver::server::handlers::HttpHandlerBase& handler,
    userver::yaml_config::YamlConfig) const {
  return std::make_unique<AuthMiddleware>(handler, storage_);
}

userver::yaml_config::Schema AuthMiddlewareFactory::GetMiddlewareConfigSchema() const {
  return userver::yaml_config::MergeSchemas<
      userver::server::middlewares::HttpMiddlewareFactoryBase>(R"(
type: object
description: auth middleware config
additionalProperties: false
properties: {}
)");
}

}  // namespace library
