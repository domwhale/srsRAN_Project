/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * This file is part of srsRAN.
 *
 * srsRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include "common/e2ap_asn1_packer.h"
#include "lib/e2/common/e2ap_asn1_utils.h"
#include "tests/unittests/e2/common/e2_test_helpers.h"
#include "srsran/support/async/async_test_utils.h"
#include "srsran/support/test_utils.h"
#include <gtest/gtest.h>

using namespace srsran;

TEST_F(e2_test_setup, ric_control_procedure_setup)
{
  e2->handle_e2_tnl_connection_request();
  e2_message request_msg = generate_ric_control_request_style2_action6(test_logger);
  e2->handle_message(request_msg);

  e2_message& msg = e2_client->last_tx_e2_pdu;
  ASSERT_EQ(msg.pdu.type().value, asn1::e2ap::e2ap_pdu_c::types_opts::successful_outcome);
  ASSERT_EQ(msg.pdu.successful_outcome().value.type(),
            asn1::e2ap::e2ap_elem_procs_o::successful_outcome_c::types_opts::ric_ctrl_ack);
  auto ack = msg.pdu.successful_outcome().value.ric_ctrl_ack();
  ASSERT_EQ(ack->ran_function_id, 3);
  e2->handle_e2_disconnection_request();
}

TEST_F(e2_test_setup, ric_control_procedure_fail)
{
  e2->handle_e2_tnl_connection_request();
  e2_message request_msg = generate_ric_control_request(test_logger, 2, 6, 1, 12);
  e2->handle_message(request_msg);

  e2_message& msg = e2_client->last_tx_e2_pdu;
  ASSERT_EQ(msg.pdu.type().value, asn1::e2ap::e2ap_pdu_c::types_opts::unsuccessful_outcome);
  ASSERT_EQ(msg.pdu.unsuccessful_outcome().value.type(),
            asn1::e2ap::e2ap_elem_procs_o::unsuccessful_outcome_c::types_opts::ric_ctrl_fail);
  auto ack = msg.pdu.unsuccessful_outcome().value.ric_ctrl_fail();
  ASSERT_EQ(ack->ran_function_id, 3);
  e2->handle_e2_disconnection_request();
}

TEST_F(e2_test_setup, ric_control_procedure_packed)
{
  e2->handle_e2_tnl_connection_request();
  uint8_t e2ap_ctrl_req[] = {
      0x00, 0x04, 0x00, 0x78, 0x00, 0x00, 0x05, 0x00, 0x1d, 0x00, 0x05, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x05,
      0x00, 0x02, 0x00, 0x03, 0x00, 0x16, 0x00, 0x09, 0x08, 0x01, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x05, 0x00,
      0x17, 0x00, 0x50, 0x4f, 0x00, 0x00, 0x01, 0x00, 0x00, 0x60, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x01, 0x44,
      0x00, 0x03, 0x00, 0x02, 0x44, 0x00, 0x00, 0x00, 0x04, 0x60, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x05, 0x44,
      0x00, 0x01, 0x00, 0x06, 0x2a, 0x00, 0x03, 0x00, 0xf1, 0x10, 0x00, 0x07, 0x44, 0x00, 0x01, 0x00, 0x08, 0x2a,
      0x00, 0x01, 0x01, 0x00, 0x09, 0x2a, 0x00, 0x03, 0x00, 0x00, 0x01, 0x00, 0x0a, 0x28, 0x80, 0x01, 0x01, 0x00,
      0x0b, 0x28, 0x80, 0x01, 0x32, 0x00, 0x0c, 0x28, 0x80, 0x01, 0x64, 0x00, 0x15, 0x00, 0x01, 0x40};
  byte_buffer e2ap_ctrl_req_buf = byte_buffer::create(e2ap_ctrl_req, e2ap_ctrl_req + sizeof(e2ap_ctrl_req)).value();
  packer->handle_packed_pdu(std::move(e2ap_ctrl_req_buf));

  e2_message& msg = e2_client->last_tx_e2_pdu;
  ASSERT_EQ(msg.pdu.type().value, asn1::e2ap::e2ap_pdu_c::types_opts::successful_outcome);
  e2->handle_e2_disconnection_request();
}

TEST_F(e2_test_setup, ric_control_procedure_ho)
{
  e2->handle_e2_tnl_connection_request();
  uint8_t     e2_ctrl_req[] = {0x00, 0x04, 0x00, 0x53, 0x00, 0x00, 0x05, 0x00, 0x1d, 0x00, 0x05, 0x00, 0x00, 0x01, 0x00,
                               0x00, 0x00, 0x05, 0x00, 0x02, 0x00, 0x03, 0x00, 0x16, 0x00, 0x13, 0x12, 0x00, 0x40, 0x00,
                               0x07, 0x00, 0x00, 0xf1, 0x10, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00,
                               0x00, 0x17, 0x00, 0x21, 0x20, 0x00, 0x00, 0x01, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x01,
                               0x44, 0x00, 0x00, 0x00, 0x02, 0x44, 0x00, 0x00, 0x00, 0x03, 0x2a, 0x00, 0x09, 0x00, 0x00,
                               0xf1, 0x10, 0x00, 0x00, 0x01, 0x9b, 0x10, 0x00, 0x15, 0x00, 0x01, 0x40};
  byte_buffer e2_ctrl_req_buf = byte_buffer::create(e2_ctrl_req, e2_ctrl_req + sizeof(e2_ctrl_req)).value();
  packer->handle_packed_pdu(std::move(e2_ctrl_req_buf));
  e2_message& msg = e2_client->last_tx_e2_pdu;
  ASSERT_EQ(msg.pdu.type().value, asn1::e2ap::e2ap_pdu_c::types_opts::successful_outcome);
  e2->handle_e2_disconnection_request();
}
