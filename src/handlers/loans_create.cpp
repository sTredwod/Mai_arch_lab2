#include "loans_create.hpp"

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

namespace {

userver::formats::json::Value MakeLoanJson(const Loan& loan) {
  userver::formats::json::ValueBuilder builder;
  builder["id"] = loan.id;
  builder["user_id"] = loan.user_id;
  builder["book_id"] = loan.book_id;
  builder["returned"] = loan.returned;
  return builder.ExtractValue();
}

}  // namespace

LoansCreateHandler::LoansCreateHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context)
    : HttpHandlerBase(config, component_context),
      storage_(
          component_context.FindComponent<LibraryStorageComponent>().GetStorage()) {
}

std::string LoansCreateHandler::HandleRequestThrow(
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
    if (!body.HasMember("user_id") || !body.HasMember("book_id")) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"missing required fields"})";
    }

    const auto user_id = body["user_id"].As<int>();
    const auto book_id = body["book_id"].As<int>();

    if (user_id <= 0 || book_id <= 0) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"invalid field values"})";
    }

    if (!storage_.GetUserById(user_id)) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kNotFound);
      return R"({"error":"user not found"})";
    }

    if (!storage_.GetBookById(book_id)) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kNotFound);
      return R"({"error":"book not found"})";
    }

    if (storage_.HasActiveLoan(user_id, book_id)) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kConflict);
      return R"({"error":"user already has this book"})";
    }

    const auto created_loan = storage_.CreateLoan(user_id, book_id);
    if (!created_loan) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kConflict);
      return R"({"error":"book is unavailable"})";
    }

    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kCreated);
    return userver::formats::json::ToString(MakeLoanJson(*created_loan));
  } catch (const std::exception&) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return R"({"error":"invalid field types"})";
  }
}

}  // namespace library
