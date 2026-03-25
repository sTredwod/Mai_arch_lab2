#pragma once

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/components/loggable_component_base.hpp>
#include <userver/yaml_config/schema.hpp>

#include "library_storage.hpp"

namespace library {

class LibraryStorageComponent final
    : public userver::components::LoggableComponentBase {
 public:
  static constexpr std::string_view kName = "library-storage";

  LibraryStorageComponent(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& context);

  LibraryStorage& GetStorage();
  const LibraryStorage& GetStorage() const;

  static userver::yaml_config::Schema GetStaticConfigSchema();

 private:
  LibraryStorage storage_;
};

}  // namespace library
