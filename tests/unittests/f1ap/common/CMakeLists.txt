#
# Copyright 2021-2025 Software Radio Systems Limited
#
# This file is part of srsRAN
#
# srsRAN is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# srsRAN is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# A copy of the GNU Affero General Public License can be found in
# the LICENSE file in the top-level directory of this distribution
# and at http://www.gnu.org/licenses/.
#

add_library(f1ap_test_helpers f1ap_cu_test_messages.cpp f1ap_du_test_messages.cpp)
target_include_directories(f1ap_test_helpers PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(f1ap_test_helpers srsran_support f1ap_asn1 srslog)

add_executable(f1ap_asn1_packer_test f1ap_asn1_packer_test.cpp f1ap_asn1_helpers_test.cpp)
target_link_libraries(f1ap_asn1_packer_test
        f1ap_test_helpers
        srsran_f1ap_common
        f1ap_test_doubles
        srsran_support
        srslog
        f1ap_asn1
        srsran_ran
        gtest
        gtest_main)
target_include_directories(f1ap_asn1_packer_test PRIVATE ${CMAKE_SOURCE_DIR})
gtest_discover_tests(f1ap_asn1_packer_test)
