#include "books_get.hpp"

#include <string>

#include <userver/formats/common/type.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/server/request/request_context.hpp>

#include "../storage/library_storage_component.hpp"

namespace library {

namespace {

userver::formats::json::Value MakeBookJson(const Book& book) {
  userver::formats::json::ValueBuilder builder;
  builder["id"] = book.id;
  builder["title"] = book.title;
  builder["author"] = book.author;
  builder["total_copies"] = book.total_copies;
  builder["available_copies"] = book.available_copies;
  return builder.ExtractValue();
}

}  // namespace

BooksGetHandler::BooksGetHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context)
    : HttpHandlerBase(config, component_context),
      storage_(
          component_context.FindComponent<LibraryStorageComponent>().GetStorage()) {
}

std::string BooksGetHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  request.GetHttpResponse().SetContentType("application/json");

  const auto title = request.GetArg("title");
  const auto author = request.GetArg("author");

  const bool has_title = !title.empty();
  const bool has_author = !author.empty();

  if (has_title == has_author) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return R"({"error":"use exactly one query parameter: title or author"})";
  }

  const auto books = has_title ? storage_.FindBooksByTitle(title)
                               : storage_.FindBooksByAuthor(author);

  userver::formats::json::ValueBuilder response(
      userver::formats::common::Type::kArray);
  for (const auto& book : books) {
    response.PushBack(MakeBookJson(book));
  }

  request.GetHttpResponse().SetStatus(
      userver::server::http::HttpStatus::kOk);
  return userver::formats::json::ToString(response.ExtractValue());
}

}  // namespace library
