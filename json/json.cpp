#include "json/json.h"

#include <regex>
#include <cstdlib>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4715)
#endif // __GNUC__

void JsonParser::skipWhitespace() noexcept {
  while (current_ < end_ && (
         *current_ == ' ' ||
         *current_ == '\t' ||
         *current_ == '\n' ||
         *current_ == '\r')) 
  {
    ++current_;
  }
}

void JsonParser::consume() noexcept {
  ++current_;
}

void JsonParser::consume(char c) {
  if (*current_++ == c)
    return;
  
    throw JsonException("Except \'") << c << "\' but got \'" << *(current_ - 1) << "\'.";
}

Json JsonParser::parse() {
  skipWhitespace();
  switch (*current_) {
  case '{': return parseObject();
  case '[': return parseArray();
  default:
    throw JsonException("Not a support Json.");
  }
}

Json JsonParser::parseObject() {
  skipWhitespace();
  consume('{');
  
  JsonObject obj;
  while (current_ < end_) {
    if (*current_ == '}') {
      consume();
      return Json(std::move(obj));
    }
    
    parsePair(obj);
    skipWhitespace();

    // More than one pair.
    if (*current_ == ',')
      consume();
  }

  throw JsonException("Got unclosed json object.");
}

void JsonParser::parsePair(JsonObject& obj) {
  std::string key = parseString();
  skipWhitespace();
  consume(':');
  auto value = parseValue();
  obj[key] = std::move(value);
}

Json JsonParser::parseArray() {
  skipWhitespace();
  consume('[');

  JsonArray arr;
  while (current_ < end_) {
    if (*current_ == ']') {
      consume();
      return Json(std::move(arr));
    }

    arr.push_back(parseValue());
    skipWhitespace();
    if (*current_ == ',')
      consume();
  }
}

Json JsonParser::parseValue() {
  skipWhitespace();

  switch (*current_) {
    case '{': return parseObject();
    case '[': return parseArray();
    case '"': return parseString();
    case 't': case 'f': return parseBool();
    case 'n': return parseNull();
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
    case '8': case '9': case '.':
      return parseNumber();
    default:
      throw JsonException("Got an unsupport character: ") << *current_;
  }
}

Json JsonParser::parseString() {
  skipWhitespace();
  consume('"');

  const char* start = current_;

  while (current_ < end_) {
    if (*current_ == '"') {
      consume();
      return Json(start, current_ - start - 1);
    }

    current_++;
  }
  throw JsonException("Unparded string: ") << current_;
}

Json JsonParser::parseNumber() {
  static std::regex isInteger{"^[+-]?\\d+"};
  static std::regex isFloat{"^[+-]?((\\d+\\.\\d)|(\\.\\d+)|(\\d+\\.))(?:[eE][+-]?\\d+)?"};

  std::size_t pos = 0;
  if (std::regex_search(current_, isFloat)) {
    float num = std::stod(current_, &pos);
    if (!pos)
      throw JsonException("Parse float number failed, got: ")
        << current_;
    current_ += pos;
    return Json(num);
  }

  if (std::regex_search(current_, isInteger)) {
    int num = std::stod(current_, &pos);
    if (!pos)
      throw JsonException("Parse integer number failed, got: ")
        << current_;
    current_ += pos;
    return Json(num);
  }

  throw JsonException("Parse number failed, got: ")
    << current_;
}

Json JsonParser::parseBool() {
  static std::regex isTrue{"^true"};
  static std::regex isFalse{"^false"};

  skipWhitespace();
  // if (end_ - current_ > 3 && 0 == strcmp(current_, "true"))
  //   return Json(true);
  // else if (end_ - current_ > 4 && 0 == strcmp(current_, "false"))
  //   return Json(false);
  if (std::regex_search(current_, isTrue)) {
    current_ += 4;
    return Json(true);
  } else if (std::regex_search(current_, isFalse)) {
    current_ += 5;
    return Json(false);
  }

  throw JsonException("Parse boolean failed: ") << current_;
}

Json JsonParser::parseNull() {
  if (end_ - current_ > 3 && 0 == strcmp(current_, "null"))
    return Json();
}

template <>
std::nullptr_t Json::get<std::nullptr_t>() { if (isNull()) return nullptr; }

template <>
bool Json::get<bool>() {
  if (isNull())
    return false;
  if (isBool())
    return std::get<bool>(value_);
  if (isInteger())
    return std::get<int>(value_) == 0 ? false : true;
  if (isFloat())
    return std::get<float>(value_) == 0 ? false : true;
  if (isString())
    return std::get<std::string>(value_).empty() ? false : true;
  if (isArray())
    return std::get<JsonArray>(value_).empty() ? false : true;
  if (isObject())
    return std::get<JsonObject>(value_).empty() ? false : true;
}

template <>
int Json::get<int>() {
  if (isNull()) return 0;
  if (isBool()) return std::get<bool>(value_) ? 1 : 0;
  if (isInteger()) return std::get<int>(value_);
  if (isFloat()) return std::get<float>(value_);

  throw JsonException("Except int but store type is: ")
    << json_type_to_string(type_);
}

template <>
float Json::get<float>() {
  if (isNull()) return 0;
  if (isBool()) return std::get<bool>(value_) ? 1 : 0;
  if (isInteger()) return std::get<int>(value_);
  if (isFloat()) return std::get<float>(value_);

  throw JsonException("Except float but store type is: ")
    << json_type_to_string(type_);
}

template <>
std::string Json::get<std::string>() {
  if (isString()) return std::get<std::string>(value_);

  throw JsonException("Except string but store type is: ")
    << json_type_to_string(type_);
}

template <>
JsonArray Json::get<JsonArray>() {
  if (isArray()) return std::get<JsonArray>(value_);

  throw JsonException("Except array but store type is: ")
    << json_type_to_string(type_);
}

template <>
JsonObject Json::get<JsonObject>() {
  if (isObject()) return std::get<JsonObject>(value_);

  throw JsonException("Except object but store type is: ")
    << json_type_to_string(type_);
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
