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

#include "rrc_ue_test_helpers.h"
#include "rrc_ue_test_messages.h"
#include "srsran/support/async/async_test_utils.h"
#include <gtest/gtest.h>

using namespace srsran;
using namespace srs_cu_cp;

/// Fixture class RRC Setup tests preparation
class rrc_ue_capability_transfer_proc_test : public rrc_ue_test_helper, public ::testing::Test
{
protected:
  static void SetUpTestSuite() { srslog::init(); }

  void SetUp() override
  {
    init();

    srslog::basic_logger& rrc_logger = srslog::fetch_basic_logger("RRC", false);
    rrc_logger.set_level(srslog::basic_levels::debug);
    rrc_logger.set_hex_dump_max_size(30);

    receive_setup_request();

    // check if the RRC setup message was generated
    ASSERT_EQ(get_srb0_pdu_type(), asn1::rrc_nr::dl_ccch_msg_type_c::c1_c_::types::rrc_setup);

    // check if SRB1 was created
    check_srb1_exists();

    receive_setup_complete();

    // setup security
    const char* sk_gnb_cstr = "45cbc3f8a81193fd5c5229300d59edf812e998a115ec4e0ce903ba89367e2628";
    const char* k_enc_cstr  = "4ea96992c8c7e82977231ad001309062ae9f31ead90a4d0842af6cd25cb44dc4";
    const char* k_int_cstr  = "aeeb5e0ae02c6188ecb1625c4a9e022fdfc2a1fc845b44b44443ac9a3bda667c";

    // Pack hex strings into srsgnb types
    security::sec_key sk_gnb = make_sec_key(sk_gnb_cstr);
    security::sec_key k_enc  = make_sec_key(k_enc_cstr);
    security::sec_key k_int  = make_sec_key(k_int_cstr);

    // Create expected SRB1 sec config
    security::sec_as_config sec_cfg = {};
    sec_cfg.domain                  = security::sec_domain::rrc;
    sec_cfg.integ_algo              = security::integrity_algorithm::nia2;
    sec_cfg.cipher_algo             = security::ciphering_algorithm::nea0;
    sec_cfg.k_enc                   = k_enc;
    sec_cfg.k_int                   = k_int;

    // Initialize security context and capabilities.
    security::security_context init_sec_ctx = {};
    init_sec_ctx.k                          = sk_gnb;
    std::fill(init_sec_ctx.supported_int_algos.begin(), init_sec_ctx.supported_int_algos.end(), true);
    std::fill(init_sec_ctx.supported_enc_algos.begin(), init_sec_ctx.supported_enc_algos.end(), true);
    ue_mng.find_ue(allocated_ue_index)->get_security_manager().init_security_context(init_sec_ctx);

    // Initialize security context.
    init_security_context();

    // Enable security
    enable_security();
  }

  void TearDown() override
  {
    // flush logger after each test
    srslog::flush();
  }
};

/// Test the RRC setup with connected AMF
TEST_F(rrc_ue_capability_transfer_proc_test,
       when_ue_capability_enquiry_is_sent_then_ue_capability_information_is_received)
{
  // Trigger UE capability transfer
  rrc_ue_capability_transfer_request msg;
  async_task<bool>         t = get_rrc_ue_control_message_handler()->handle_rrc_ue_capability_transfer_request(msg);
  lazy_task_launcher<bool> t_launcher(t);

  ASSERT_FALSE(t.ready());
  check_rrc_ue_enquiry_pdu(2);

  // Receive UE capability information
  receive_ue_capability_information(2);

  ASSERT_TRUE(t.ready());
}
