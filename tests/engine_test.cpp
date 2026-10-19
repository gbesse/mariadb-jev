#include "engine.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

template <typename F>
void rejects(F&& callable) {
  try {
    callable();
  } catch (const std::exception&) {
    return;
  }
  throw std::runtime_error("invalid input was accepted");
}

int main() {
  const auto conditions = jev::parse_conditions(
      R"(["discusses the ending","recommends the movie"])");
  assert(conditions.size() == 2);
  rejects([] { jev::parse_conditions(R"(["okay",2])"); });
  rejects([] { jev::parse_conditions("[]"); });
  rejects([] { jev::parse_conditions("not json"); });
  const std::string invalid_utf8("\xC3(", 2);
  rejects([&] { jev::make_request(invalid_utf8, conditions, "jev-1.13.0"); });
  rejects([&] { jev::make_request("valid", {invalid_utf8}, "jev-1.13.0"); });
  rejects([&] { jev::make_request("valid", conditions, invalid_utf8); });
  const auto request = jev::make_request("The ending was good. Watch it!",
                                         conditions, "jev-1.13.0");
  assert(request.find("\"q0\"") != std::string::npos);
  assert(request.find("\"q1\"") != std::string::npos);
  assert(request.find("Watch it!") != std::string::npos);
  const auto scores = jev::parse_response(
      R"({"answers":{"q0":{"type":"noul","noul":0.91},"q1":{"type":"noul","noul":0.84}}})",
      2);
  assert(scores.size() == 2 && std::abs(scores[0] - 0.91) < 0.001);
  rejects([] { jev::parse_response(R"({"answers":{"q0":{"type":"noul","noul":1.5}}})", 1); });
  rejects([] { jev::parse_response(R"({"answers":{}})", 1); });

  rejects([] { jev::parse_response(R"({"answers":{}})", 0); });
  rejects([] { jev::parse_response(R"({"answers":{}})", 9); });
  rejects([] { jev::parse_response(R"({"answers":{}})", static_cast<std::size_t>(-1)); });
  rejects([] { jev::parse_response(R"({"answers":{"q0":{"type":"noul","noul":0.9},"extra":{"type":"noul","noul":0.2}}})", 1); });
  rejects([] { jev::parse_response(R"({"model":"other","answers":{"q0":{"type":"noul","noul":0.9}}})", 1, "jev-1.13.0"); });
  rejects([] { jev::parse_response(R"({"model":null,"answers":{"q0":{"type":"noul","noul":0.9}}})", 1, "jev-1.13.0"); });
  assert(jev::parse_response(R"({"model":"jev-1.13.0","answers":{"q0":{"type":"noul","noul":0.9}}})", 1, "jev-1.13.0")[0] == 0.9);
  assert(jev::parse_response(R"({"answers":{"q0":{"type":"noul","noul":0.9}}})", 1, "jev-1.13.0")[0] == 0.9);
  auto settings = jev::settings_from_environment();
  for (const auto* endpoint : {"http://127.0.0.1:18765@evil.example/v1/systemone",
       "http://127.0.0.1.evil.example/v1/systemone", "http://example.test/v1/systemone",
       "https://user:password@example.test/v1/systemone", "https://example.test/#fragment",
       "ftp://127.0.0.1/v1/systemone", "https://"}) {
    auto invalid = settings; invalid.api_url = endpoint;
    rejects([&] { jev::Client forbidden(invalid); });
  }
  { auto valid = settings; valid.api_url = "https://example.test/v1/systemone"; jev::Client accepted(valid); }
  { auto invalid = settings; invalid.api_key = "synthetic\r\nInjected: value"; rejects([&] { jev::Client forbidden(invalid); }); }
  { auto invalid = settings; invalid.timeout_ms = 0; rejects([&] { jev::Client forbidden(invalid); }); }
  { auto invalid = settings; invalid.max_requests = 0; rejects([&] { jev::Client forbidden(invalid); }); }
  settings.max_requests = 1;
  jev::Client client(settings);
  rejects([&] { client.evaluate(invalid_utf8, conditions); });
  assert(client.requests() == 0);
  const auto actual = client.evaluate("The ending was good. Watch it!", conditions);
  assert(actual.size() == 2 && actual[0] == 0.91 && actual[1] == 0.84);
  const auto cached = client.evaluate("The ending was good. Watch it!", conditions);
  assert(cached == actual && client.requests() == 1);
  rejects([&] { client.evaluate("A different review", conditions); });
  std::cout << "engine tests passed\n";
}
