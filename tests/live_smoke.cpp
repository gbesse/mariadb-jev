#include "engine.h"

#include <iostream>
#include <stdexcept>

int main() {
  try {
    jev::Client client(jev::settings_from_environment());
    const auto scores = client.evaluate(
        "The movie's final scene was surprising. I recommend watching it.",
        {"The review discusses the ending", "The review recommends the movie"});
    if (scores.size() != 2) throw std::runtime_error("expected two answers");
    std::cout << "Live API protocol smoke test passed\n";
  } catch (const std::exception&) {
    std::cerr << "Live API protocol smoke test failed\n";
    return 1;
  }
}
