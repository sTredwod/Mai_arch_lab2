#pragma once

#include <string>

#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_status.hpp>

#include "../storage/library_storage.hpp"

namespace library {

inline bool CheckBearerAuth(const userver::server::http::HttpRequest& request,
                            LibraryStorage& storage,
                            std::string& error_response) {
  const auto& header = request.GetHeader("Authorization");
  const std::string prefix = "Bearer ";

  if (header.empty()) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kUnauthorized);
    error_response = R"({"error":"missing Authorization header"})";
    return false;
  }

  if (header.rfind(prefix, 0) != 0) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kUnauthorized);
    error_response = R"({"error":"invalid Authorization header"})";
    return false;
  }

  const std::string token = header.substr(prefix.size());
  if (token.empty() || !storage.IsTokenValid(token)) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kUnauthorized);
    error_response = R"({"error":"invalid token"})";
    return false;
  }

  return true;
}

}  // namespace library
