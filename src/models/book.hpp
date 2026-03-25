#pragma once

#include <string>

namespace library {

struct Book {
  int id{};
  std::string title;
  std::string author;
  int total_copies{};
  int available_copies{};
};

}  // namespace library
