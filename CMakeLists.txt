cmake_minimum_required(VERSION 3.20)
project(ArrowBpacking CXX)

include(CheckCXXCompilerFlag)

# Collect SIMD compile definitions and per-file flags
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")

    set(ARROW_SIMD_SOURCES
        arrow/util/bpacking_simd_default.cc
        arrow/util/bpacking_simd_avx2.cc
        arrow/util/bpacking_simd_avx512.cc
    )

    check_cxx_compiler_flag("-msse4.2" COMPILER_SUPPORTS_SSE4_2)
    if(COMPILER_SUPPORTS_SSE4_2)
        list(APPEND ARROW_COMPILE_DEFINITIONS ARROW_HAVE_SSE4_2)
        # Explicitly disable AVX2/AVX512 so this TU only uses SSE4.2 intrinsics
        set_source_files_properties(arrow/util/bpacking_simd_default.cc
            PROPERTIES COMPILE_FLAGS "-msse4.2 -mno-avx2 -mno-avx512f"
        )
    endif()

    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    if(COMPILER_SUPPORTS_AVX2)
        list(APPEND ARROW_COMPILE_DEFINITIONS ARROW_HAVE_RUNTIME_AVX2)
        # Per-file flag so only this TU is compiled with avx2 intrinsics
        set_source_files_properties(arrow/util/bpacking_simd_avx2.cc
            PROPERTIES COMPILE_FLAGS "-mavx2 -mno-avx512f"
        )
    endif()

    check_cxx_compiler_flag("-mavx512f" COMPILER_SUPPORTS_AVX512)
    if(COMPILER_SUPPORTS_AVX512)
        list(APPEND ARROW_COMPILE_DEFINITIONS ARROW_HAVE_RUNTIME_AVX512)
        set_source_files_properties(arrow/util/bpacking_simd_avx512.cc
            PROPERTIES COMPILE_FLAGS "-mavx512f -mavx512bw -mavx512cd -mavx512dq -mavx512vl"
        )
    endif()

elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")

    set(ARROW_SIMD_SOURCES
        arrow/util/bpacking_simd_default.cc
    )
    list(APPEND ARROW_COMPILE_DEFINITIONS ARROW_HAVE_NEON)
    set(NEON_COMPILE_FLAGS "-march=armv8-a+simd")
    check_cxx_compiler_flag("-mno-sve" COMPILER_SUPPORTS_NO_SVE)
    if(COMPILER_SUPPORTS_NO_SVE)
        string(APPEND NEON_COMPILE_FLAGS " -mno-sve")
    endif()
    set_source_files_properties(arrow/util/bpacking_simd_default.cc
        PROPERTIES COMPILE_FLAGS "${NEON_COMPILE_FLAGS}"
    )

endif()

# Library
find_package(xsimd REQUIRED)

add_library(arrow
    arrow/util/bpacking.cc
    arrow/util/bpacking_scalar.cc
    ${ARROW_SIMD_SOURCES}
)
target_compile_features(arrow PUBLIC cxx_std_20)
target_include_directories(arrow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(arrow PUBLIC xsimd)
target_compile_definitions(arrow PRIVATE ${ARROW_COMPILE_DEFINITIONS})
# On Windows, mark symbols as exported when building the DLL
if(BUILD_SHARED_LIBS)
    target_compile_definitions(arrow PRIVATE ARROW_EXPORTING)
endif()

# Tests
find_package(GTest REQUIRED)
enable_testing()

add_executable(bpacking_test arrow/util/bpacking_test.cc arrow/testing/util.cc)
target_link_libraries(bpacking_test PRIVATE arrow GTest::gtest GTest::gtest_main)
target_compile_definitions(bpacking_test PRIVATE ${ARROW_COMPILE_DEFINITIONS})
add_test(NAME bpacking_test COMMAND bpacking_test)

# Benchmarks
find_package(benchmark REQUIRED)

add_executable(bpacking_benchmark arrow/util/bpacking_benchmark.cc arrow/testing/util.cc)
target_link_libraries(bpacking_benchmark PRIVATE arrow benchmark::benchmark benchmark::benchmark_main)
target_compile_definitions(bpacking_benchmark PRIVATE ${ARROW_COMPILE_DEFINITIONS})
