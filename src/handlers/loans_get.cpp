#include "loans_get.hpp"

#include <string>

#include <userver/formats/common/type.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/server/request/request_context.hpp>

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

LoansGetHandler::LoansGetHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context)
    : HttpHandlerBase(config, component_context),
      storage_(
          component_context.FindComponent<LibraryStorageComponent>().GetStorage()) {
}

std::string LoansGetHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  request.GetHttpResponse().SetContentType("application/json");

  const auto user_id_str = request.GetArg("user_id");
  if (user_id_str.empty()) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return R"({"error":"user_id query parameter is required"})";
  }

  try {
    const int user_id = std::stoi(user_id_str);

    if (user_id <= 0) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"invalid user_id"})";
    }

    if (!storage_.GetUserById(user_id)) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kNotFound);
      return R"({"error":"user not found"})";
    }

    const auto loans = storage_.GetLoansByUserId(user_id);

    userver::formats::json::ValueBuilder response(
        userver::formats::common::Type::kArray);
    for (const auto& loan : loans) {
      response.PushBack(MakeLoanJson(loan));
    }

    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(response.ExtractValue());
  } catch (const std::exception&) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return R"({"error":"invalid user_id"})";
  }
}

}  // namespace library
