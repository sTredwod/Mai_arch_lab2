#!/usr/bin/env bash
set -euo pipefail

BASE_URL="${BASE_URL:-http://127.0.0.1:8080}"

echo "[1] create user"
USER_RESPONSE=$(curl -s -X POST "$BASE_URL/users" \
  -H "Content-Type: application/json" \
  -d '{"login":"reader1","password":"12345","first_name":"Ilya","last_name":"Ivanov"}')
echo "$USER_RESPONSE"

echo "[2] login"
LOGIN_RESPONSE=$(curl -s -X POST "$BASE_URL/auth/login" \
  -H "Content-Type: application/json" \
  -d '{"login":"reader1","password":"12345"}')
echo "$LOGIN_RESPONSE"

TOKEN=$(printf '%s' "$LOGIN_RESPONSE" | python3 -c 'import sys, json; print(json.load(sys.stdin)["token"])')
echo "TOKEN=$TOKEN"

echo "[3] create book"
BOOK_RESPONSE=$(curl -s -X POST "$BASE_URL/books" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"title":"Master and Margarita","author":"Mikhail Bulgakov","total_copies":3}')
echo "$BOOK_RESPONSE"

echo "[4] get books by title"
curl -s "$BASE_URL/books?title=Master"
echo

echo "[5] create loan"
LOAN_RESPONSE=$(curl -s -X POST "$BASE_URL/loans" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"user_id":1,"book_id":1}')
echo "$LOAN_RESPONSE"

echo "[6] get loans by user"
curl -s "$BASE_URL/loans?user_id=1"
echo

echo "[7] return loan"
curl -s -X PATCH "$BASE_URL/loans/1/return" \
  -H "Authorization: Bearer $TOKEN"
echo

echo "SMOKE TESTS PASSED"
