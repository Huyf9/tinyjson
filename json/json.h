#ifndef JSON_H_
#define JSON_H_

#include <string>
#include <vector>
#include <variant>
#include <cstring>
#include <cstdint>
#include <sstream>
#include <exception>
#include <unordered_map>

enum class JsonType : uint8_t {
  Null,
  Boolean,
  Integer,
  Float,
  String,
  Array,
  Object
};


class JsonException : public std::exception {
public:
  JsonException(const std::string& msg) {
    oss_ << msg;
  }

  JsonException(const JsonException& rhs) {
    if (&rhs != this)
      oss_ << rhs.oss_.str();
  }

  template <typename T>
  JsonException& operator<<(T&& msg) {
    oss_ << msg;
    return *this;
  }

  const char* what() const noexcept override {
    return oss_.str().c_str();
  }

private:
  std::stringstream oss_;
};

inline std::string json_type_to_string(JsonType type) {
#define CASE_TYPE(TYPE) \
  case JsonType::TYPE: \
    return #TYPE

  switch (type) {
    CASE_TYPE(Null);
    CASE_TYPE(Boolean);
    CASE_TYPE(Integer);
    CASE_TYPE(Float);
    CASE_TYPE(String);
    CASE_TYPE(Array);
    CASE_TYPE(Object);
    default:
      throw JsonException("Unsupport Json Type.");
  }
}

class Json;
using JsonObject = std::unordered_map<std::string, Json>;
using JsonArray = std::vector<Json>;


/// Json type traits
template <typename T, typename = void>
struct json_type {
  using value = T;
};

template <typename T>
struct json_type<T, 
  std::enable_if_t<std::is_floating_point_v<T>>>
  : std::true_type {
  using value = float;
};

template <typename T>
struct json_type<T,
  std::enable_if_t<std::is_integral_v<T> &&
    !std::is_same_v<std::remove_cv_t<T>, bool>>>
  : std::true_type {
  using value = int;
};

template <typename T>
using json_type_v = typename json_type<T>::value;

class JsonParser {
public:
  JsonParser(std::string_view source)
    : current_(source.data()), start_(source.data()), end_(source.data() + source.size()) {}

  Json parse();

private:
  // Skip whitespace
  void skipWhitespace() noexcept;
  void consume() noexcept;
  void consume(char c);

  Json parseObject();
  void parsePair(JsonObject& obj);
  Json parseArray();
  Json parseValue();
  Json parseString();
  Json parseNumber();
  Json parseBool();
  Json parseNull();

private:
  const char* current_;
  const char* start_;
  const char* end_;
};


class Json {
public:
  Json() : type_(JsonType::Null) {}
  Json(std::nullptr_t) : Json() {}
  Json(int v) : type_(JsonType::Integer), value_(v) {}
  Json(float v) : type_(JsonType::Float), value_(v) {}
  Json(const std::string& v) : type_(JsonType::String), value_(v) {}
  Json(std::string&& v) : type_(JsonType::String), value_(v) {}
  template<typename... Args, typename = 
    std::enable_if_t<std::is_constructible_v<
      std::string, Args...>>>
  Json(Args&&... args)
    : type_(JsonType::String), value_(std::string(std::forward<Args>(args)...)) {}
  Json(const JsonArray& v) : type_(JsonType::Array), value_(v) {}
  Json(JsonArray&& v) : type_(JsonType::Array), value_(v) {}
  Json(const JsonObject&& v) : type_(JsonType::Object), value_(v) {}
  Json(JsonObject&& v) : type_(JsonType::Object), value_(v) {}
  
  Json(Json&& rhs) : type_(rhs.type_), value_(rhs.value_) {}
  Json(const Json& rhs) : type_(rhs.type_), value_(rhs.value_) {}
  Json& operator=(const Json& rhs) {
    if (&rhs != this) {
      type_ = rhs.type_;
      value_ = rhs.value_;
    }
    return *this;
  }
  Json& operator=(Json&& rhs) {
    if (&rhs != this) {
      type_ = rhs.type_;
      value_ = rhs.value_;
    }
    return *this;
  }

  template <typename T>
  Json& operator[](T&& key) {
    if constexpr (std::is_convertible_v<T, std::string>) {
      if (isObject()) {
        return std::get<JsonObject>(value_)[key];
      }
      throw JsonException("Only json object support operaotr[](key).");
    } else if constexpr (std::is_integral_v<T>) {
      if (isArray())
        return std::get<JsonArray>(value_)[key];
      throw JsonException("Only json array support operaotr[](index).");
    }
    throw JsonException("Not support operator[] for giving type.");

  }

  static Json parse(const std::string& source) {
    static JsonParser parser(source);
    return parser.parse();
  }

  bool isNull() noexcept { return type_ == JsonType::Null; }
  bool isBool() noexcept { return type_ == JsonType::Boolean; }
  bool isInteger() noexcept { return type_ == JsonType::Integer; }
  bool isFloat() noexcept { return type_ == JsonType::Float; }
  bool isString() noexcept { return type_ == JsonType::String; }
  bool isArray() noexcept { return type_ == JsonType::Array; }
  bool isObject() noexcept { return type_ == JsonType::Object; }

  template <typename T>
  T get();

  template <typename T>
  operator T() { return get<json_type_v<T>>(); }

private:
  using Value = std::variant<
                  bool,
                  int,
                  float,
                  std::string,
                  JsonArray,
                  JsonObject>;

  JsonType type_;
  Value value_;
};



#endif // JSON_H_
