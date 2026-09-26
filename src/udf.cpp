#include "engine.h"

#include <mysql.h>

#include <algorithm>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
enum class Mode { All, Any, Probability };
struct Context {
  explicit Context(jev::Settings settings) : client(std::move(settings)) {}
  jev::Client client;
};

bool initialize(UDF_INIT* init, UDF_ARGS* args, char* message) {
  try {
    if (args->arg_count != 2 || args->arg_type[0] != STRING_RESULT ||
        args->arg_type[1] != STRING_RESULT) {
      throw std::runtime_error("expected two strings: value and condition(s)");
    }
    auto context = std::make_unique<Context>(jev::settings_from_environment());
    init->maybe_null = true;
    init->const_item = false;
    init->max_length = 1;
    init->ptr = reinterpret_cast<char*>(context.release());
    return false;
  } catch (const std::exception& failure) {
    std::snprintf(message, MYSQL_ERRMSG_SIZE, "%s", failure.what());
    return true;
  }
}

std::vector<double> evaluate(UDF_INIT* init, UDF_ARGS* args, Mode mode) {
  auto* context = reinterpret_cast<Context*>(init->ptr);
  const std::string value(args->args[0], args->lengths[0]);
  std::vector<std::string> conditions;
  if (mode == Mode::Probability) {
    conditions.emplace_back(args->args[1], args->lengths[1]);
  } else {
    conditions = jev::parse_conditions(
        std::string_view(args->args[1], args->lengths[1]));
  }
  return context->client.evaluate(value, conditions);
}

long long boolean_result(UDF_INIT* init, UDF_ARGS* args, char* is_null,
                         char* error, Mode mode) {
  if (!args->args[0] || !args->args[1]) {
    *is_null = 1;
    return 0;
  }
  try {
    const auto probabilities = evaluate(init, args, mode);
    if (mode == Mode::All) {
      return std::all_of(probabilities.begin(), probabilities.end(),
                         [](double probability) { return probability >= 0.5; });
    }
    return std::any_of(probabilities.begin(), probabilities.end(),
                       [](double probability) { return probability >= 0.5; });
  } catch (...) {
    *is_null = 1;
    *error = 1;
    return 0;
  }
}
}  // namespace

extern "C" {
bool jev_all_init(UDF_INIT* init, UDF_ARGS* args, char* message) {
  return initialize(init, args, message);
}
bool jev_any_init(UDF_INIT* init, UDF_ARGS* args, char* message) {
  return initialize(init, args, message);
}
bool jev_probability_init(UDF_INIT* init, UDF_ARGS* args, char* message) {
  return initialize(init, args, message);
}
void jev_all_deinit(UDF_INIT* init) { delete reinterpret_cast<Context*>(init->ptr); }
void jev_any_deinit(UDF_INIT* init) { delete reinterpret_cast<Context*>(init->ptr); }
void jev_probability_deinit(UDF_INIT* init) { delete reinterpret_cast<Context*>(init->ptr); }
long long jev_all(UDF_INIT* init, UDF_ARGS* args, char* is_null, char* error) {
  return boolean_result(init, args, is_null, error, Mode::All);
}
long long jev_any(UDF_INIT* init, UDF_ARGS* args, char* is_null, char* error) {
  return boolean_result(init, args, is_null, error, Mode::Any);
}
double jev_probability(UDF_INIT* init, UDF_ARGS* args, char* is_null,
                       char* error) {
  if (!args->args[0] || !args->args[1]) {
    *is_null = 1;
    return 0;
  }
  try {
    return evaluate(init, args, Mode::Probability).front();
  } catch (...) {
    *is_null = 1;
    *error = 1;
    return 0;
  }
}
}  // extern "C"
