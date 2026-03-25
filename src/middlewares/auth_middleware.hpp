#pragma once

#include <memory>
#include <string>
#include <string_view>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/middlewares/http_middleware_base.hpp>
#include <userver/yaml_config/schema.hpp>
#include <userver/yaml_config/yaml_config.hpp>

#include "../storage/library_storage.hpp"

namespace library {

class AuthMiddleware final : public userver::server::middlewares::HttpMiddlewareBase {
 public:
  static constexpr std::string_view kName = "auth-middleware";

  AuthMiddleware(const userver::server::handlers::HttpHandlerBase& handler,
                 LibraryStorage& storage);

 private:
  void HandleRequest(userver::server::http::HttpRequest& request,
                     userver::server::request::RequestContext& context) const override;

  static bool RequiresAuth(const userver::server::http::HttpRequest& request);

  LibraryStorage& storage_;
};

class AuthMiddlewareFactory final
    : public userver::server::middlewares::HttpMiddlewareFactoryBase {
 public:
  static constexpr std::string_view kName = AuthMiddleware::kName;

  AuthMiddlewareFactory(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& context);

 private:
  std::unique_ptr<userver::server::middlewares::HttpMiddlewareBase> Create(
      const userver::server::handlers::HttpHandlerBase& handler,
      userver::yaml_config::YamlConfig middleware_config) const override;

  userver::yaml_config::Schema GetMiddlewareConfigSchema() const override;

  LibraryStorage& storage_;
};

}  // namespace library
