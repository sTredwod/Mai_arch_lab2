#include "library_storage_component.hpp"

#include <userver/yaml_config/merge_schemas.hpp>

namespace library {

LibraryStorageComponent::LibraryStorageComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::components::LoggableComponentBase(config, context) {}

LibraryStorage& LibraryStorageComponent::GetStorage() {
  return storage_;
}

const LibraryStorage& LibraryStorageComponent::GetStorage() const {
  return storage_;
}

userver::yaml_config::Schema LibraryStorageComponent::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<
      userver::components::LoggableComponentBase>(R"(
type: object
description: in-memory storage for library service
additionalProperties: false
properties: {}
)");
}

}  // namespace library
