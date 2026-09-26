#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace jev {

struct Settings {
  std::string api_key;
  std::string api_url;
  std::string model;
  long timeout_ms = 10000;
  std::size_t max_requests = 1000;
};

Settings settings_from_environment();
std::vector<std::string> parse_conditions(std::string_view json);
std::string make_request(std::string_view value,
                         const std::vector<std::string>& conditions,
                         std::string_view model);
std::vector<double> parse_response(std::string_view json, std::size_t expected);

class Client {
 public:
  explicit Client(Settings settings);
  ~Client();
  Client(const Client&) = delete;
  Client& operator=(const Client&) = delete;
  std::vector<double> evaluate(std::string_view value,
                               const std::vector<std::string>& conditions);
  std::size_t requests() const { return requests_; }

 private:
  Settings settings_;
  void* curl_ = nullptr;
  void* headers_ = nullptr;
  std::unordered_map<std::string, std::vector<double>> cache_;
  std::size_t cache_bytes_ = 0;
  std::size_t requests_ = 0;
};

}  // namespace jev
