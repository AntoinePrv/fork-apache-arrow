#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <bitset>
#include <format>
#include <optional>
#include <ranges>
#include <string>
#include <span>
#include <vector>
#include <random>
#include <bit>
#include <limits>

#include <xcpp/xdisplay.hpp>
#include <nlohmann/json.hpp>
#include <xwidgets/xall.hpp>

#include "arrow/util/bit_util.h"
#include "arrow/util/bpacking_simd_kernel_internal.h"
#include "arrow/util/bpacking_dispatch_internal.h"
#include "arrow/util/bpacking_internal.h"

#include "ui.hpp"
#include "utils.hpp"
#include "components.hpp"

// Since we are not linking with libarrow, we need to add all the C++ code
// that is indirectly depended upon by header inclusion
// #include "arrow/util/bpacking.cc"
#include "arrow/util/bpacking_simd_default.cc"
#include "arrow/util/bpacking_scalar.cc"
// #include "arrow/util/cpu_info.cc"
#include "arrow/util/logging.cc"
// #include "arrow/result.cc"
// #include "arrow/util/string_util.cc"
// #include "arrow/util/io_util.cc"
// #include "arrow/util/atfork_internal.cc"
// #include "arrow/status.cc"
