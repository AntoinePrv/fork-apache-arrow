// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include <memory>
#include <stdexcept>
#include <vector>

#include <benchmark/benchmark.h>

#include "arrow/testing/util.h"
#include "arrow/util/bpacking_internal.h"
#include "arrow/util/bpacking_scalar_internal.h"
#include "arrow/util/bpacking_simd_internal.h"

#if defined(ARROW_HAVE_RUNTIME_AVX2)
#  include "arrow/util/cpu_info.h"
#endif

namespace arrow::internal {
namespace {

template <typename Int>
using UnpackFunc = void (*)(const uint8_t*, Int*, const UnpackOptions&);

/// Get the number of bytes associate with a packing.
constexpr int32_t GetNumBytes(int32_t num_values, int32_t bit_width) {
  const auto num_bits = num_values * bit_width;
  if (num_bits % 8 != 0) {
    throw std::invalid_argument("Must pack a multiple of 8 bits.");
  }
  return num_bits / 8;
}

/// Generate random bytes as packed integers.
std::vector<uint8_t> GenerateRandomPackedValues(int32_t num_values, int32_t bit_width) {
  constexpr uint32_t kSeed = 3214;
  const auto num_bytes = GetNumBytes(num_values, bit_width);

  std::vector<uint8_t> out(num_bytes);
  random_bytes(num_bytes, kSeed, out.data());

  return out;
}

template <typename Int>
void BM_Unpack(benchmark::State& state, UnpackFunc<Int> unpack, bool skip,
               std::string skip_msg) {
  if (skip) {
    state.SkipWithMessage(skip_msg);
  }

  const auto bit_width = static_cast<int32_t>(state.range(0));
  const auto num_values = static_cast<int32_t>(state.range(1));

  const auto packed = GenerateRandomPackedValues(num_values, bit_width);
  const uint8_t* packed_ptr = packed.data();

  auto unpacked = std::make_unique<Int[]>(num_values);

  const ::arrow::internal::UnpackOptions opts{
      .batch_size = num_values,
      .bit_width = bit_width,
      .bit_offset = 0,
      .max_read_bytes = -1,
  };

  for (auto _ : state) {
    unpack(packed_ptr, unpacked.get(), opts);
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(num_values * state.iterations());
}

// Currently, the minimum unpack SIMD kernel size is 32 and the RLE-bit-packing encoder
// will not emit runs larger than 512 (though other implementation might), so we biased
// the benchmarks towards a rather small scale.
static const auto kNumValuesRange = benchmark::CreateRange(32, 512, 2);
constexpr std::initializer_list<int64_t> kBitWidths8 = {1, 2, 8};
constexpr std::initializer_list<int64_t> kBitWidths16 = {1, 2, 8, 13};
constexpr std::initializer_list<int64_t> kBitWidths32 = {1, 2, 8, 20};
constexpr std::initializer_list<int64_t> kBitWidths64 = {1, 2, 8, 20, 47};

static const std::vector<std::vector<int64_t>> kBitWidthsNumValuesBool = {
    {0, 1},
    kNumValuesRange,
};
static const std::vector<std::vector<int64_t>> kBitWidthsNumValues8 = {
    kBitWidths8,
    kNumValuesRange,
};
static const std::vector<std::vector<int64_t>> kBitWidthsNumValues16 = {
    kBitWidths16,
    kNumValuesRange,
};
static const std::vector<std::vector<int64_t>> kBitWidthsNumValues32 = {
    kBitWidths32,
    kNumValuesRange,
};
static const std::vector<std::vector<int64_t>> kBitWidthsNumValues64 = {
    kBitWidths64,
    kNumValuesRange,
};

/// Nudge for MSVC template inside BENCHMARK_CAPTURE macro.
void BM_UnpackBool(benchmark::State& state, UnpackFunc<bool> unpack, bool skip = false,
                   std::string skip_msg = "") {
  return BM_Unpack<bool>(state, unpack, skip, std::move(skip_msg));
}
/// Nudge for MSVC template inside BENCHMARK_CAPTURE macro.
void BM_UnpackUint8(benchmark::State& state, UnpackFunc<uint8_t> unpack,
                    bool skip = false, std::string skip_msg = "") {
  return BM_Unpack<uint8_t>(state, unpack, skip, std::move(skip_msg));
}
/// Nudge for MSVC template inside BENCHMARK_CAPTURE macro.
void BM_UnpackUint16(benchmark::State& state, UnpackFunc<uint16_t> unpack,
                     bool skip = false, std::string skip_msg = "") {
  return BM_Unpack<uint16_t>(state, unpack, skip, std::move(skip_msg));
}
/// Nudge for MSVC template inside BENCHMARK_CAPTURE macro.
void BM_UnpackUint32(benchmark::State& state, UnpackFunc<uint32_t> unpack,
                     bool skip = false, std::string skip_msg = "") {
  return BM_Unpack<uint32_t>(state, unpack, skip, std::move(skip_msg));
}
/// Nudge for MSVC template inside BENCHMARK_CAPTURE macro.
void BM_UnpackUint64(benchmark::State& state, UnpackFunc<uint64_t> unpack,
                     bool skip = false, std::string skip_msg = "") {
  return BM_Unpack<uint64_t>(state, unpack, skip, std::move(skip_msg));
}

BENCHMARK_CAPTURE(BM_UnpackBool, Scalar, &bpacking::unpack_scalar<bool>)
    ->ArgsProduct(kBitWidthsNumValuesBool);
BENCHMARK_CAPTURE(BM_UnpackUint8, Scalar, &bpacking::unpack_scalar<uint8_t>)
    ->ArgsProduct(kBitWidthsNumValues8);
BENCHMARK_CAPTURE(BM_UnpackUint16, Scalar, &bpacking::unpack_scalar<uint16_t>)
    ->ArgsProduct(kBitWidthsNumValues16);
BENCHMARK_CAPTURE(BM_UnpackUint32, Scalar, &bpacking::unpack_scalar<uint32_t>)
    ->ArgsProduct(kBitWidthsNumValues32);
BENCHMARK_CAPTURE(BM_UnpackUint64, Scalar, &bpacking::unpack_scalar<uint64_t>)
    ->ArgsProduct(kBitWidthsNumValues64);

#if defined(ARROW_HAVE_SSE4_2)
BENCHMARK_CAPTURE(BM_UnpackBool, Sse42, &bpacking::unpack_sse4_2<bool>)
    ->ArgsProduct(kBitWidthsNumValuesBool);
BENCHMARK_CAPTURE(BM_UnpackUint8, Sse42, &bpacking::unpack_sse4_2<uint8_t>)
    ->ArgsProduct(kBitWidthsNumValues8);
BENCHMARK_CAPTURE(BM_UnpackUint16, Sse42, &bpacking::unpack_sse4_2<uint16_t>)
    ->ArgsProduct(kBitWidthsNumValues16);
BENCHMARK_CAPTURE(BM_UnpackUint32, Sse42, &bpacking::unpack_sse4_2<uint32_t>)
    ->ArgsProduct(kBitWidthsNumValues32);
BENCHMARK_CAPTURE(BM_UnpackUint64, Sse42, &bpacking::unpack_sse4_2<uint64_t>)
    ->ArgsProduct(kBitWidthsNumValues64);
#endif

#if defined(ARROW_HAVE_RUNTIME_AVX2)
BENCHMARK_CAPTURE(BM_UnpackBool, Avx2, &bpacking::unpack_avx2<bool>,
                  !CpuInfo::GetInstance()->IsSupported(CpuInfo::AVX2),
                  "Avx2 not available")
    ->ArgsProduct(kBitWidthsNumValuesBool);
BENCHMARK_CAPTURE(BM_UnpackUint8, Avx2, &bpacking::unpack_avx2<uint8_t>,
                  !CpuInfo::GetInstance()->IsSupported(CpuInfo::AVX2),
                  "Avx2 not available")
    ->ArgsProduct(kBitWidthsNumValues8);
BENCHMARK_CAPTURE(BM_UnpackUint16, Avx2, &bpacking::unpack_avx2<uint16_t>,
                  !CpuInfo::GetInstance()->IsSupported(CpuInfo::AVX2),
                  "Avx2 not available")
    ->ArgsProduct(kBitWidthsNumValues16);
BENCHMARK_CAPTURE(BM_UnpackUint32, Avx2, &bpacking::unpack_avx2<uint32_t>,
                  !CpuInfo::GetInstance()->IsSupported(CpuInfo::AVX2),
                  "Avx2 not available")
    ->ArgsProduct(kBitWidthsNumValues32);
BENCHMARK_CAPTURE(BM_UnpackUint64, Avx2, &bpacking::unpack_avx2<uint64_t>,
                  !CpuInfo::GetInstance()->IsSupported(CpuInfo::AVX2),
                  "Avx2 not available")
    ->ArgsProduct(kBitWidthsNumValues64);
#endif

#if defined(ARROW_HAVE_RUNTIME_AVX512)
BENCHMARK_CAPTURE(BM_UnpackBool, Avx512, &bpacking::unpack_avx512<bool>,
                  !CpuInfo::GetInstance()->IsSupported(CpuInfo::AVX512),
                  "Avx512 not available")
    ->ArgsProduct(kBitWidthsNumValuesBool);
BENCHMARK_CAPTURE(BM_UnpackUint8, Avx512, &bpacking::unpack_avx512<uint8_t>,
                  !CpuInfo::GetInstance()->IsSupported(CpuInfo::AVX512),
                  "Avx512 not available")
    ->ArgsProduct(kBitWidthsNumValues8);
BENCHMARK_CAPTURE(BM_UnpackUint16, Avx512, &bpacking::unpack_avx512<uint16_t>,
                  !CpuInfo::GetInstance()->IsSupported(CpuInfo::AVX512),
                  "Avx512 not available")
    ->ArgsProduct(kBitWidthsNumValues16);
BENCHMARK_CAPTURE(BM_UnpackUint32, Avx512, &bpacking::unpack_avx512<uint32_t>,
                  !CpuInfo::GetInstance()->IsSupported(CpuInfo::AVX512),
                  "Avx512 not available")
    ->ArgsProduct(kBitWidthsNumValues32);
BENCHMARK_CAPTURE(BM_UnpackUint64, Avx512, &bpacking::unpack_avx512<uint64_t>,
                  !CpuInfo::GetInstance()->IsSupported(CpuInfo::AVX512),
                  "Avx512 not available")
    ->ArgsProduct(kBitWidthsNumValues64);
#endif

#if defined(ARROW_HAVE_NEON)
BENCHMARK_CAPTURE(BM_UnpackBool, Neon, &bpacking::unpack_neon<bool>)
    ->ArgsProduct(kBitWidthsNumValuesBool);
BENCHMARK_CAPTURE(BM_UnpackUint8, Neon, &bpacking::unpack_neon<uint8_t>)
    ->ArgsProduct(kBitWidthsNumValues8);
BENCHMARK_CAPTURE(BM_UnpackUint16, Neon, &bpacking::unpack_neon<uint16_t>)
    ->ArgsProduct(kBitWidthsNumValues16);
BENCHMARK_CAPTURE(BM_UnpackUint32, Neon, &bpacking::unpack_neon<uint32_t>)
    ->ArgsProduct(kBitWidthsNumValues32);
BENCHMARK_CAPTURE(BM_UnpackUint64, Neon, &bpacking::unpack_neon<uint64_t>)
    ->ArgsProduct(kBitWidthsNumValues64);
#endif

BENCHMARK_CAPTURE(BM_UnpackBool, Dynamic, &unpack<bool>)
    ->ArgsProduct(kBitWidthsNumValuesBool);
BENCHMARK_CAPTURE(BM_UnpackUint8, Dynamic, &unpack<uint8_t>)
    ->ArgsProduct(kBitWidthsNumValues8);
BENCHMARK_CAPTURE(BM_UnpackUint16, Dynamic, &unpack<uint16_t>)
    ->ArgsProduct(kBitWidthsNumValues16);
BENCHMARK_CAPTURE(BM_UnpackUint32, Dynamic, &unpack<uint32_t>)
    ->ArgsProduct(kBitWidthsNumValues32);
BENCHMARK_CAPTURE(BM_UnpackUint64, Dynamic, &unpack<uint64_t>)
    ->ArgsProduct(kBitWidthsNumValues64);

}  // namespace
}  // namespace arrow::internal
