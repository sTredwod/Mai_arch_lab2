# Домашнее задание 02: Разработка REST API сервиса

**Выполнил:** Губенков Илья, 105 гр.

## Содержание

- [Задание](#задание)
- [Результат](#результат)
- [Вариант 15 — Библиотечный сервис](#вариант-15--библиотечный-сервис)
- [Архитектура](#архитектура)
  - [POST /users](#post-users)
  - [GET /users](#get-users)
  - [POST /auth/login](#post-authlogin)
  - [POST /books](#post-books)
  - [GET /books](#get-books)
  - [POST /loans](#post-loans)
  - [GET /loans](#get-loans)
  - [PATCH /loans/{loan_id}/return](#patch-loansloan_idreturn)
- [Аутентификация](#аутентификация)
- [Middleware](#middleware)
- [Тестирование](#тестирование)
- [HTTP статус-коды](#http-статус-коды)
- [OpenAPI](#openapi)
- [Хранилище](#хранилище)
- [Запуск](#запуск)
  - [Локальный запуск в userver-контейнере](#локальный-запуск-в-userver-контейнере)
  - [Запуск через Docker Compose](#запуск-через-docker-compose)
- [Ограничения текущей реализации](#ограничения-текущей-реализации)
- [Вывод](#вывод)

---

## Задание

Цель работы — получить практические навыки разработки REST API сервиса с использованием принципов REST, обработки HTTP-запросов, реализации аутентификации, middleware и документирования API.

В рамках лабораторной работы требовалось:

1. Спроектировать REST API для выбранной предметной области.
2. Реализовать REST API сервис на выбранном языке и фреймворке.
3. Реализовать минимум 5 API endpoint-ов.
4. Реализовать простую аутентификацию.
5. Защитить минимум 2 endpoint с помощью аутентификации.
6. Добавить middleware для проверки аутентификации.
7. Создать OpenAPI/Swagger спецификацию.
8. Подготовить простые тесты успешных и ошибочных сценариев.
9. Подготовить Docker-совместимый запуск сервиса.

---

## Результат

Реализован REST API сервис библиотеки на **C++20** с использованием **Yandex userver**.

Сервис поддерживает:

- создание пользователя;
- поиск пользователя по логину;
- поиск пользователя по маске имени и фамилии;
- логин пользователя и получение токена;
- добавление книги;
- поиск книги по названию;
- поиск книги по автору;
- оформление выдачи книги пользователю;
- получение списка активных выдач пользователя;
- возврат книги.

Также реализованы:

- Bearer token аутентификация;
- middleware для проверки токена;
- OpenAPI спецификация;
- smoke-тесты и error-тесты;
- запуск через Docker и Docker Compose.

---

## Вариант 15 — Библиотечный сервис

Приложение содержит следующие основные сущности:

- **Пользователь**
- **Книга**
- **Выдача книги**

Реализованы следующие операции:

- создание нового пользователя;
- поиск пользователя по логину;
- поиск пользователя по маске имени и фамилии;
- добавление новой книги;
- поиск книги по названию;
- поиск книги по автору;
- выдача книги пользователю;
- просмотр списка активных выдач пользователя;
- возврат книги.

---

## Архитектура

Сервис реализован как один backend на **Yandex userver**.

Основные части проекта:

- `src/models` — модели предметной области:
  - `User`
  - `Book`
  - `Loan`
- `src/storage` — in-memory хранилище и компонент userver:
  - `LibraryStorage`
  - `LibraryStorageComponent`
- `src/handlers` — HTTP handler-ы
- `src/middlewares` — middleware для проверки Bearer token
- `configs` — конфигурация userver
- `tests` — bash-скрипты для smoke/error тестирования
- `openapi.yaml` — спецификация API
- `Dockerfile`, `docker-compose.yaml` — контейнеризация

В текущей реализации используется **in-memory storage**, то есть данные существуют только во время работы процесса.

---

### POST /users

Создание нового пользователя.

**Request**

```json
curl -i -X POST http://127.0.0.1:8080/users \
  -H "Content-Type: application/json" \
  -d '{
    "login": "ilya",
    "first_name": "Ilya",
    "last_name": "Gubenkov",
    "password": "123456"
  }'
```

**Response**

```json
{
  "id": 1,
  "login": "reader1",
  "first_name": "Ilya",
  "last_name": "Ivanov"
}
```

---

### GET /users

Поиск пользователя.

**Поиск по логину**

```http
GET /users?login=reader1
```

**Response**

```json
{
  "id": 1,
  "login": "reader1",
  "first_name": "Ilya",
  "last_name": "Ivanov"
}
```

**Поиск по маске имени/фамилии**

```http
GET /users?name_mask=Ily
```

**Response**

```json
[
  {
    "id": 1,
    "login": "reader1",
    "first_name": "Ilya",
    "last_name": "Ivanov"
  }
]
```

---

### POST /auth/login

Логин пользователя и получение токена.

**Request**

```json
{
  "login": "reader1",
  "password": "12345"
}
```

**Response**

```json
{
  "token": "token-1-1"
}
```

---

### POST /books

Добавление новой книги.

Требует авторизацию.

**Headers**

```http
Authorization: Bearer token-1-1
```

**Request**

```json
{
  "title": "Master and Margarita",
  "author": "Mikhail Bulgakov",
  "total_copies": 3
}
```

**Response**

```json
{
  "id": 1,
  "title": "Master and Margarita",
  "author": "Mikhail Bulgakov",
  "total_copies": 3,
  "available_copies": 3
}
```

---

### GET /books

Поиск книги.

**По названию**

```http
GET /books?title=Master
```

**По автору**

```http
GET /books?author=Bulgakov
```

**Response**

```json
[
  {
    "id": 1,
    "title": "Master and Margarita",
    "author": "Mikhail Bulgakov",
    "total_copies": 3,
    "available_copies": 3
  }
]
```

---

### POST /loans

Оформление выдачи книги пользователю.

Требует авторизацию.

**Headers**

```http
Authorization: Bearer token-1-1
```

**Request**

```json
{
  "user_id": 1,
  "book_id": 1
}
```

**Response**

```json
{
  "id": 1,
  "user_id": 1,
  "book_id": 1,
  "returned": false
}
```

---

### GET /loans

Получение списка активных выдач пользователя.

```http
GET /loans?user_id=1
```

**Response**

```json
[
  {
    "id": 1,
    "user_id": 1,
    "book_id": 1,
    "returned": false
  }
]
```

---

### PATCH /loans/{loan_id}/return

Возврат книги.

Требует авторизацию.

**Headers**

```http
Authorization: Bearer token-1-1
```

**Пример**

```http
PATCH /loans/1/return
```

**Response**

```json
{
  "id": 1,
  "user_id": 1,
  "book_id": 1,
  "returned": true
}
```

---

## Аутентификация

В проекте реализована простая token-based аутентификация.

Пользователь получает токен через endpoint:

```http
POST /auth/login
```

Далее токен передаётся в заголовке:

```http
Authorization: Bearer <token>
```

Защищены следующие endpoint-ы:

- `POST /books`
- `POST /loans`
- `PATCH /loans/{loan_id}/return`

---

## Middleware

Для проверки токена реализован отдельный middleware.

Middleware:

- перехватывает запросы к защищённым endpoint-ам;
- проверяет наличие заголовка `Authorization`;
- проверяет формат `Bearer <token>`;
- валидирует токен через `LibraryStorage`;
- в случае ошибки возвращает `401 Unauthorized`.

Таким образом, проверка аутентификации вынесена из общей логики маршрутов в отдельный слой.

---

## Тестирование

Для тестирования реализованы два bash-скрипта:

- `tests/smoke_tests.sh` — успешные сценарии;
- `tests/error_tests.sh` — ошибочные сценарии.

### Smoke tests

Проверяются:

- создание пользователя;
- логин;
- создание книги;
- поиск книги;
- оформление выдачи;
- получение выдач;
- возврат книги.

**Запуск**

```bash
Mai_arch_dz_var15/Laba2/library-service/library_service/tests$ bash smoke_tests.sh
```

### Error tests

Проверяются:

- запрос без обязательных query-параметров;
- логин с неверным паролем;
- доступ к защищённым endpoint-ам без токена;
- ошибки авторизации.

**Запуск**

```bash
Mai_arch_dz_var15/Laba2/library-service/library_service/tests$ bash error_tests.sh
```

---

## HTTP статус-коды

В сервисе используются следующие основные HTTP статус-коды:

- `200 OK` — успешный GET, логин, возврат;
- `201 Created` — успешное создание ресурса;
- `400 Bad Request` — невалидные данные или параметры запроса;
- `401 Unauthorized` — отсутствует или неверен токен, неверные учётные данные;
- `404 Not Found` — пользователь, книга или выдача не найдены;
- `409 Conflict` — конфликт состояния.

К `409 Conflict` относятся, например, случаи, когда пользователь уже существует, книга уже существует, пользователь уже взял эту книгу, книга уже возвращена или книга недоступна.

---

## OpenAPI

Для проекта подготовлена спецификация:

```text
openapi.yaml
```

В ней описаны:

- все endpoint-ы;
- параметры запросов;
- request/response схемы;
- ошибки;
- Bearer token security scheme.

---

## Хранилище

Хранилище реализовано как in-memory storage.

Используются контейнеры в памяти процесса:

- пользователи;
- книги;
- выдачи;
- токены.

Плюсы такого решения — простота реализации, быстрый запуск и отсутствие внешней базы данных. Минус заключается в том, что данные не сохраняются между перезапусками сервиса или контейнера.

---

## Запуск

### Локальный запуск в userver-контейнере

Из каталога проекта  
**(именно нижнее подчёркивание, то есть `/Mai_arch_dz_var15/Laba2/library-service/library_service`)**

```bash
docker run --rm -it \
  --user "$(id -u):$(id -g)" \
  --security-opt seccomp=unconfined \
  -p 8080:8080 \
  -v "$PWD":/work \
  -w /work \
  --entrypoint bash \
  ghcr.io/userver-framework/ubuntu-22.04-userver:latest
```

Внутри контейнера:

```bash
cd library_service
export HOME=/tmp
export CCACHE_DIR=/tmp/.ccache
mkdir -p "$CCACHE_DIR"
make build-debug
./build-debug/library_service -c configs/static_config.yaml
```

### Запуск через Docker Compose (если будет запускаться долго, подождите)

Из каталога `library_service`  
**(именно нижнее подчёркивание, то есть `/Mai_arch_dz_var15/Laba2/library-service/library_service`)**

```bash
docker compose up --build
```

**Проверка**

```bash
curl -i "http://127.0.0.1:8080/ping"
```

---

## Ограничения текущей реализации

В текущей реализации используется in-memory storage вместо PostgreSQL/SQLite, поэтому данные не переживают перезапуск процесса. Токены не имеют срока жизни, роли пользователей не разделяются, а полноценная бизнес-логика библиотечной системы с историей выдач и штрафами не реализована.

---

## Вывод

В ходе лабораторной работы был разработан REST API сервис библиотеки на C++ с использованием Yandex userver.

В проекте реализованы базовые CRUD-подобные операции над сущностями, поиск ресурсов через query-параметры, простая token-based аутентификация, middleware для проверки авторизации, OpenAPI документация, тестовые сценарии и Docker-совместимый запуск. В результате получена рабочая реализация REST API сервиса для выбранной предметной области, соответствующая требованиям задания.
