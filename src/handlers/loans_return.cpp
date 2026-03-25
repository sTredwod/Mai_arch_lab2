#include "loans_return.hpp"

#include <string>

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

LoansReturnHandler::LoansReturnHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context)
    : HttpHandlerBase(config, component_context),
      storage_(
          component_context.FindComponent<LibraryStorageComponent>().GetStorage()) {
}

std::string LoansReturnHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  request.GetHttpResponse().SetContentType("application/json");

  std::string auth_error;
  if (!CheckBearerAuth(request, storage_, auth_error)) {
    return auth_error;
  }

  try {
    const auto loan_id_str = request.GetPathArg("loan_id");
    const int loan_id = std::stoi(loan_id_str);

    if (loan_id <= 0) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"invalid loan_id"})";
    }

    const auto existing_loan = storage_.GetLoanById(loan_id);
    if (!existing_loan) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kNotFound);
      return R"({"error":"loan not found"})";
    }

    if (existing_loan->returned) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kConflict);
      return R"({"error":"book already returned"})";
    }

    const auto returned_loan = storage_.ReturnLoan(loan_id);
    if (!returned_loan) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kConflict);
      return R"({"error":"unable to return loan"})";
    }

    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(MakeLoanJson(*returned_loan));
  } catch (const std::exception&) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return R"({"error":"invalid loan_id"})";
  }
}

}  // namespace library
