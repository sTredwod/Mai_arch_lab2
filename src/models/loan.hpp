#pragma once

namespace library {

struct Loan {
  int id{};
  int user_id{};
  int book_id{};
  bool returned{false};
};

}  // namespace library
