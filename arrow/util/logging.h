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

#pragma once

#include <cassert>

#define ARROW_CHECK(condition) assert(condition)
#define ARROW_CHECK_OK(s) assert((s).ok())
#define ARROW_CHECK_EQ(val1, val2) assert((val1) == (val2))
#define ARROW_CHECK_NE(val1, val2) assert((val1) != (val2))
#define ARROW_CHECK_LE(val1, val2) assert((val1) <= (val2))
#define ARROW_CHECK_LT(val1, val2) assert((val1) < (val2))
#define ARROW_CHECK_GE(val1, val2) assert((val1) >= (val2))
#define ARROW_CHECK_GT(val1, val2) assert((val1) > (val2))

#define ARROW_DCHECK    ARROW_CHECK
#define ARROW_DCHECK_OK ARROW_CHECK_OK
#define ARROW_DCHECK_EQ ARROW_CHECK_EQ
#define ARROW_DCHECK_NE ARROW_CHECK_NE
#define ARROW_DCHECK_LE ARROW_CHECK_LE
#define ARROW_DCHECK_LT ARROW_CHECK_LT
#define ARROW_DCHECK_GE ARROW_CHECK_GE
#define ARROW_DCHECK_GT ARROW_CHECK_GT
