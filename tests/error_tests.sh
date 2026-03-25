#!/usr/bin/env bash
set -euo pipefail

BASE_URL="${BASE_URL:-http://127.0.0.1:8080}"

check_status() {
  local expected="$1"
  shift
  local actual
  actual=$(curl -s -o /tmp/test_body.txt -w "%{http_code}" "$@")
  echo "expected=$expected actual=$actual"
  cat /tmp/test_body.txt
  echo
  if [[ "$actual" != "$expected" ]]; then
    echo "FAILED: expected HTTP $expected but got $actual"
    exit 1
  fi
}

echo "[1] users without params -> 400"
check_status 400 "$BASE_URL/users"

echo "[2] login with wrong password -> 401"
check_status 401 -X POST "$BASE_URL/auth/login" \
  -H "Content-Type: application/json" \
  -d '{"login":"reader1","password":"wrong"}'

echo "[3] books create without token -> 401"
check_status 401 -X POST "$BASE_URL/books" \
  -H "Content-Type: application/json" \
  -d '{"title":"Bad","author":"NoAuth","total_copies":1}'

echo "[4] loans create without token -> 401"
check_status 401 -X POST "$BASE_URL/loans" \
  -H "Content-Type: application/json" \
  -d '{"user_id":1,"book_id":1}'

echo "[5] return loan without token -> 401"
check_status 401 -X PATCH "$BASE_URL/loans/1/return"

echo "ERROR TESTS PASSED"
