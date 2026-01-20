#pragma once

#include <charconv>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string_view>
#include <vector>

template <typename Uint = std::size_t>
auto parseCsvInt(std::string_view s) -> std::vector<Uint> {
  std::vector<Uint> out = {};
  std::size_t pos = 0;

  while (true) {
    pos = s.find_first_not_of(' ', pos);
    if (pos == std::string_view::npos) {
      throw std::invalid_argument("unexpected end");
    }

    Uint v = 0;
    const char* b = s.data() + pos;
    const char* e = s.data() + s.size();
    auto [p, ec] = std::from_chars(b, e, v);
    if (ec != std::errc()) {
      throw std::invalid_argument("invalid integer");
    }
    out.push_back(v);
    pos = static_cast<size_t>(p - s.data());

    pos = s.find_first_not_of(' ', pos);
    if (pos == std::string_view::npos) {
      break;
    }
    if (s[pos] != ',') {
      throw std::invalid_argument("expected comma");
    }
    ++pos;
  }

  return out;
}

enum struct Uint {
  u8 = 8,
  u16 = 16,
  u32 = 32,
  u64 = 64,
};

constexpr auto ParseUint(std::string_view s) -> Uint {
  constexpr std::string_view prefix = "uint";
  constexpr std::string_view suffix = "_t";
  if (!s.starts_with(prefix) || !s.ends_with(suffix)) {
    throw std::invalid_argument("expected uint like name (e.g. uint32_t)");
  }

  s.remove_prefix(prefix.size());
  s.remove_suffix(suffix.size());

  std::uint64_t val = 0;
  auto [_, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
  if (ec != std::errc()) {
    throw std::invalid_argument("invalid integer");
  }

  return static_cast<Uint>(val);
}

constexpr auto UintMax(Uint u) -> std::uint64_t {
  constexpr auto kOnes = ~std::uint64_t{0};
  return kOnes >> (8 * sizeof(std::uint64_t) - static_cast<std::uint8_t>(u));
}

inline auto RandomBytes(std::size_t n, std::mt19937::result_type seed = 33)
    -> std::vector<uint8_t> {
  std::mt19937 gen(seed);
  std::uniform_int_distribution<std::uint8_t> dist(0, 255);

  auto result = std::vector<std::uint8_t>();
  result.reserve(n);
  for (std::size_t i = 0; i < n; ++i) {
    result.push_back(dist(gen));
  }
  return result;
}
