# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

# Contains utilities for working with Arrow data been stored
# in a row-major order.

arrow_install_all_headers("arrow/compute/row")

if(ARROW_BUILD_BENCHMARKS AND ARROW_COMPUTE)
  add_arrow_benchmark(grouper_benchmark PREFIX "arrow-compute")
  if(ARROW_BUILD_STATIC)
    target_link_libraries(arrow-compute-grouper-benchmark PUBLIC arrow_compute_static)
  else()
    target_link_libraries(arrow-compute-grouper-benchmark PUBLIC arrow_compute_shared)
  endif()
endif()
