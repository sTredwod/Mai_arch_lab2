#include "books_create.hpp"

#include <stdexcept>
#include <string>

#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/server/request/request_context.hpp>

#include "auth_utils.hpp"
#include "../storage/library_storage_component.hpp"

namespace library {

BooksCreateHandler::BooksCreateHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context)
    : HttpHandlerBase(config, component_context),
      storage_(
          component_context.FindComponent<LibraryStorageComponent>().GetStorage()) {
}

std::string BooksCreateHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  request.GetHttpResponse().SetContentType("application/json");

  std::string auth_error;
  if (!CheckBearerAuth(request, storage_, auth_error)) {
    return auth_error;
  }

  userver::formats::json::Value body;
  try {
    body = userver::formats::json::FromString(request.RequestBody());
  } catch (const std::exception&) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return R"({"error":"invalid json"})";
  }

  try {
    if (!body.HasMember("title") || !body.HasMember("author") ||
        !body.HasMember("total_copies")) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"missing required fields"})";
    }

    const auto title = body["title"].As<std::string>();
    const auto author = body["author"].As<std::string>();
    const auto total_copies = body["total_copies"].As<int>();

    if (title.empty() || author.empty() || total_copies <= 0) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"invalid field values"})";
    }

    const auto created_book =
        storage_.CreateBook(title, author, total_copies);

    if (!created_book) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kConflict);
      return R"({"error":"book already exists"})";
    }

    userver::formats::json::ValueBuilder response;
    response["id"] = created_book->id;
    response["title"] = created_book->title;
    response["author"] = created_book->author;
    response["total_copies"] = created_book->total_copies;
    response["available_copies"] = created_book->available_copies;

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
