#pragma once

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../models/book.hpp"
#include "../models/loan.hpp"
#include "../models/user.hpp"

namespace library {

class LibraryStorage {
 public:
  std::optional<User> CreateUser(const std::string& login,
                                 const std::string& password,
                                 const std::string& first_name,
                                 const std::string& last_name);

  std::optional<User> GetUserById(int user_id) const;
  std::optional<User> GetUserByLogin(const std::string& login) const;
  std::vector<User> FindUsersByNameMask(const std::string& name_mask) const;

  std::optional<std::string> Login(const std::string& login,
                                   const std::string& password);
  bool IsTokenValid(const std::string& token) const;

  std::optional<Book> CreateBook(const std::string& title,
                                 const std::string& author,
                                 int total_copies);

  std::optional<Book> GetBookById(int book_id) const;
  std::vector<Book> FindBooksByTitle(const std::string& title) const;
  std::vector<Book> FindBooksByAuthor(const std::string& author) const;

  bool HasActiveLoan(int user_id, int book_id) const;
  std::optional<Loan> GetLoanById(int loan_id) const;
  std::optional<Loan> CreateLoan(int user_id, int book_id);
  std::vector<Loan> GetLoansByUserId(int user_id) const;
  std::optional<Loan> ReturnLoan(int loan_id);

 private:
  static std::string ToLower(std::string value);
  static bool ContainsCaseInsensitive(const std::string& haystack,
                                      const std::string& needle);

  mutable std::mutex mutex_;

  int next_user_id_{1};
  int next_book_id_{1};
  int next_loan_id_{1};
  int next_token_id_{1};

  std::unordered_map<int, User> users_;
  std::unordered_map<int, Book> books_;
  std::unordered_map<int, Loan> loans_;
  std::unordered_map<std::string, int> tokens_;
};

}  // namespace library
