#pragma once

#include <concepts>
#include <cstdint>
#include <functional>
#include <iostream>
#include <optional>
#include <ranges>

#include "arrow/util/endian.h"

struct RGB {
  std::uint8_t red = 0;
  std::uint8_t green = 0;
  std::uint8_t blue = 0;

  constexpr static auto FromHex(std::size_t hex) -> RGB {
    return {
        .red = static_cast<std::uint8_t>((hex >> 8 * 2) & 0xFF),
        .green = static_cast<std::uint8_t>((hex >> 8 * 1) & 0xFF),
        .blue = static_cast<std::uint8_t>((hex >> 8 * 0) & 0xFF),
    };
  }
};

inline constexpr auto WHITE = RGB::FromHex(0xFFFFFF);
inline constexpr auto BLACK = RGB::FromHex(0x000000);
inline constexpr auto LIGHT_GREY = RGB::FromHex(0xD9D9D9);
inline constexpr auto DARK_GREY = RGB::FromHex(0x525252);
inline constexpr auto DARK_ORANGE = RGB::FromHex(0XB34B00);
inline constexpr auto LIGHT_ORANGE = RGB::FromHex(0xFFC69D);
inline constexpr auto DARK_BLUE = RGB::FromHex(0x177C9B);
inline constexpr auto LIGHT_BLUE = RGB::FromHex(0xB8E6F4);

struct TextStyle {
  std::optional<RGB> fg = std::nullopt;
  std::optional<RGB> bg = std::nullopt;
};

inline void PrintColored(auto text, const TextStyle& style = {},
                         std::ostream& out = std::cout) {
  if (style.fg) {
    out << "\033[38;2;" << static_cast<int>(style.fg->red) << ";"
        << static_cast<int>(style.fg->green) << ";" << static_cast<int>(style.fg->blue)
        << "m";
  }
  if (style.bg) {
    out << "\033[48;2;" << static_cast<int>(style.bg->red) << ";"
        << static_cast<int>(style.bg->green) << ";" << static_cast<int>(style.bg->blue)
        << "m";
  }
  out << text;
  if (style.fg || style.bg) {
    out << "\033[0m";
  }
}

inline auto NoTextStyle(std::size_t) -> TextStyle { return {}; }

struct PrintBytesParams {
  std::optional<std::size_t> lane_bit_size = std::nullopt;
  std::string lane_end = "│";
  std::function<TextStyle(std::size_t)> bit_style = &NoTextStyle;
  std::optional<std::size_t> max_bytes = std::nullopt;
};

template <typename Range>
  requires std::ranges::input_range<Range>
void PrintBytes(Range&& values, const PrintBytesParams& params,
                std::ostream& out = std::cout) {
  using value_type = std::ranges::range_value_t<Range>;
  constexpr auto kBitsPerByte = std::size_t{8};
  constexpr auto kBytePerValue = sizeof(value_type);
  constexpr auto kFirstBitMask = std::uint8_t{0b1};

  const auto lane_bit_size =
      params.lane_bit_size.value_or(std::numeric_limits<std::size_t>::max());

  auto out_bit_idx = std::size_t{0};
  for (const value_type val_any : values) {
    // Respect limit on number of bytes
    if (params.max_bytes && out_bit_idx / 8 >= *params.max_bytes) {
      break;
    }

    const auto val_le = arrow::bit_util::ToLittleEndian(val_any);
    for (std::size_t i = 0; i < kBytePerValue; ++i) {
      // Print byte separator at beginning of lane
      if (out_bit_idx % lane_bit_size == 0) {
        out << "│";
      }

      const std::uint8_t byte = reinterpret_cast<const std::uint8_t*>(&val_le)[i];
      for (std::size_t j = 0; j < kBitsPerByte; ++j) {
        const std::uint32_t bit = (byte >> j) & kFirstBitMask;
        const auto style = params.bit_style(out_bit_idx);
        PrintColored(bit, style, out);
        ++out_bit_idx;
      }

      // Print byte separator at end of byte
      out << "│";
      // Print lane end at end of lane
      if (out_bit_idx % lane_bit_size == 0) {
        out << params.lane_end;
      }
    }
  }
}

template <typename Uint>
  requires std::unsigned_integral<Uint>
void PrintBytes(Uint value, const PrintBytesParams& params,
                std::ostream& out = std::cout) {
  auto* const bytes = reinterpret_cast<const std::uint8_t*>(&value);
  return PrintBytes(std::span(bytes, sizeof(Uint)), params, out);
}

struct PrintPackedColorParams {
  std::size_t packed_bit_size;
  std::optional<std::size_t> n_valid_bits = std::nullopt;
  std::function<bool(std::size_t)> bit_highlight = [](auto) { return false; };
};

inline auto MakePackedColorFn(const PrintPackedColorParams& params) {
  return [=](std::size_t bit_idx) -> TextStyle {
    // Avoid coloring past given the valid number of bits
    if (!params.n_valid_bits || bit_idx < params.n_valid_bits) {
      const auto value_idx = bit_idx / params.packed_bit_size;
      const bool is_even_value = value_idx % 2 == 0;
      if (params.bit_highlight(bit_idx)) {
        return {.fg = WHITE, .bg = is_even_value ? DARK_ORANGE : DARK_BLUE};
      }
      return {.fg = BLACK, .bg = is_even_value ? LIGHT_ORANGE : LIGHT_BLUE};
    }
    return {};
  };
}

struct PrintUnpackedColorParams {
  std::size_t packed_bit_size;
  std::size_t unpacked_bit_size;
  std::function<bool(std::size_t)> bit_highlight = [](auto) { return false; };
};

inline auto MakeUnpackedColorFn(const PrintUnpackedColorParams& params) {
  return [=](std::size_t bit_idx) -> TextStyle {
    const auto value_idx = bit_idx / params.unpacked_bit_size;
    const auto bit_idx_in_val = bit_idx % params.unpacked_bit_size;
    // Color the packed bits
    if (bit_idx_in_val < params.packed_bit_size) {
      const bool is_even_value = value_idx % 2 == 0;
      if (params.bit_highlight(bit_idx)) {
        return {.fg = WHITE, .bg = is_even_value ? DARK_ORANGE : DARK_BLUE};
      }
      return {.fg = BLACK, .bg = is_even_value ? LIGHT_ORANGE : LIGHT_BLUE};
    }
    // Leave padding uncolored
    return {};
  };
}
