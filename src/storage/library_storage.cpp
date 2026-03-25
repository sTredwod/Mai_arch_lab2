#include "library_storage.hpp"

#include <algorithm>
#include <cctype>

namespace library {

namespace {

std::string MakeToken(int user_id, int token_id) {
  return "token-" + std::to_string(user_id) + "-" + std::to_string(token_id);
}

}  // namespace

std::optional<User> LibraryStorage::CreateUser(const std::string& login,
                                               const std::string& password,
                                               const std::string& first_name,
                                               const std::string& last_name) {
  std::lock_guard<std::mutex> lock(mutex_);

  for (const auto& [id, user] : users_) {
    if (user.login == login) {
      return std::nullopt;
    }
  }

  User user;
  user.id = next_user_id_++;
  user.login = login;
  user.password = password;
  user.first_name = first_name;
  user.last_name = last_name;

  users_.emplace(user.id, user);
  return user;
}

std::optional<User> LibraryStorage::GetUserById(int user_id) const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = users_.find(user_id);
  if (it == users_.end()) {
    return std::nullopt;
  }

  return it->second;
}

std::optional<User> LibraryStorage::GetUserByLogin(
    const std::string& login) const {
  std::lock_guard<std::mutex> lock(mutex_);

  for (const auto& [id, user] : users_) {
    if (user.login == login) {
      return user;
    }
  }

  return std::nullopt;
}

std::vector<User> LibraryStorage::FindUsersByNameMask(
    const std::string& name_mask) const {
  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<User> result;
  for (const auto& [id, user] : users_) {
    if (ContainsCaseInsensitive(user.first_name, name_mask) ||
        ContainsCaseInsensitive(user.last_name, name_mask)) {
      result.push_back(user);
    }
  }

  return result;
}

std::optional<std::string> LibraryStorage::Login(const std::string& login,
                                                 const std::string& password) {
  std::lock_guard<std::mutex> lock(mutex_);

  for (const auto& [id, user] : users_) {
    if (user.login == login && user.password == password) {
      const std::string token = MakeToken(user.id, next_token_id_++);
      tokens_[token] = user.id;
      return token;
    }
  }

  return std::nullopt;
}

bool LibraryStorage::IsTokenValid(const std::string& token) const {
  std::lock_guard<std::mutex> lock(mutex_);
  return tokens_.find(token) != tokens_.end();
}

std::optional<Book> LibraryStorage::CreateBook(const std::string& title,
                                               const std::string& author,
                                               int total_copies) {
  std::lock_guard<std::mutex> lock(mutex_);

  for (const auto& [id, book] : books_) {
    if (book.title == title && book.author == author) {
      return std::nullopt;
    }
  }

  Book book;
  book.id = next_book_id_++;
  book.title = title;
  book.author = author;
  book.total_copies = total_copies;
  book.available_copies = total_copies;

  books_.emplace(book.id, book);
  return book;
}

std::optional<Book> LibraryStorage::GetBookById(int book_id) const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = books_.find(book_id);
  if (it == books_.end()) {
    return std::nullopt;
  }

  return it->second;
}

std::vector<Book> LibraryStorage::FindBooksByTitle(
    const std::string& title) const {
  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<Book> result;
  for (const auto& [id, book] : books_) {
    if (ContainsCaseInsensitive(book.title, title)) {
      result.push_back(book);
    }
  }

  return result;
}

std::vector<Book> LibraryStorage::FindBooksByAuthor(
    const std::string& author) const {
  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<Book> result;
  for (const auto& [id, book] : books_) {
    if (ContainsCaseInsensitive(book.author, author)) {
      result.push_back(book);
    }
  }

  return result;
}

bool LibraryStorage::HasActiveLoan(int user_id, int book_id) const {
  std::lock_guard<std::mutex> lock(mutex_);

  for (const auto& [id, loan] : loans_) {
    if (loan.user_id == user_id && loan.book_id == book_id && !loan.returned) {
      return true;
    }
  }

  return false;
}

std::optional<Loan> LibraryStorage::GetLoanById(int loan_id) const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = loans_.find(loan_id);
  if (it == loans_.end()) {
    return std::nullopt;
  }

  return it->second;
}

std::optional<Loan> LibraryStorage::CreateLoan(int user_id, int book_id) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (users_.find(user_id) == users_.end()) {
    return std::nullopt;
  }

  auto book_it = books_.find(book_id);
  if (book_it == books_.end()) {
    return std::nullopt;
  }

  if (book_it->second.available_copies <= 0) {
    return std::nullopt;
  }

  for (const auto& [id, loan] : loans_) {
    if (loan.user_id == user_id && loan.book_id == book_id && !loan.returned) {
      return std::nullopt;
    }
  }

  Loan loan;
  loan.id = next_loan_id_++;
  loan.user_id = user_id;
  loan.book_id = book_id;
  loan.returned = false;

  loans_.emplace(loan.id, loan);
  book_it->second.available_copies -= 1;

  return loan;
}

std::vector<Loan> LibraryStorage::GetLoansByUserId(int user_id) const {
  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<Loan> result;
  for (const auto& [id, loan] : loans_) {
    if (loan.user_id == user_id && !loan.returned) {
      result.push_back(loan);
    }
  }

  return result;
}

std::optional<Loan> LibraryStorage::ReturnLoan(int loan_id) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto loan_it = loans_.find(loan_id);
  if (loan_it == loans_.end()) {
    return std::nullopt;
  }

  if (loan_it->second.returned) {
    return std::nullopt;
  }

  loan_it->second.returned = true;

  auto book_it = books_.find(loan_it->second.book_id);
  if (book_it != books_.end() &&
      book_it->second.available_copies < book_it->second.total_copies) {
    book_it->second.available_copies += 1;
  }

  return loan_it->second;
}

std::string LibraryStorage::ToLower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char ch) {
                   return static_cast<char>(std::tolower(ch));
                 });
  return value;
}

bool LibraryStorage::ContainsCaseInsensitive(const std::string& haystack,
                                             const std::string& needle) {
  const std::string lower_haystack = ToLower(haystack);
  const std::string lower_needle = ToLower(needle);

  return lower_haystack.find(lower_needle) != std::string::npos;
}

}  // namespace library
