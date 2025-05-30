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

#include "srsran/ran/pdcch/dci_packing.h"
#include "fmt/std.h"
#include <gtest/gtest.h>
#include <random>

using namespace srsran;

// Maximum number of resource block groups per BWP.
static constexpr unsigned MAX_NOF_RBGS = 18;

// Asserts that the validator returns false on an invalid configuration, and that the assertion message matches the
// expected message.
static void test_validator(const dci_size_config& invalid_config, const std::string& expected_assert_message)
{
  const error_type<std::string> error = validate_dci_size_config(invalid_config);

  // Make sure the configuration is invalid.
  ASSERT_FALSE(error);

  // Perform size alignment procedure.
  ASSERT_EQ(error.error(), expected_assert_message);
}

namespace {

class DciValidatorFallbackFixture : public ::testing::Test
{
protected:
  // Returns a copy of the base DCI configuration.
  dci_size_config get_base_dci_config() { return base_dci_fallback_config; }

private:
  // DCI payload size configuration parameters.
  const dci_size_config base_dci_fallback_config = {

      // Flag for fallback DCI formats on a UE-specific Search Space.
      true,
      // Flag for non-fallback DCI formats on a UE-specific Search Space.
      false,
      // Bandwidth of the initial DL BWP in number of Resource Blocks.
      52,
      // Bandwidth of the active DL BWP in number of Resource Blocks.
      106,
      // Bandwidth of the initial UL BWP in number of Resource Blocks.
      52,
      // Bandwidth of the active UL BWP in number of Resource Blocks.
      106,
      // CORESET 0 DL bandwidth. Set to 0 if not configured.
      0,
      // Supplementary uplink flag.
      false};
};

class DciValidatorNonFallbackFixture : public ::testing::Test
{
protected:
  // Returns a copy of the base DCI configuration.
  dci_size_config get_base_dci_config() { return base_dci_non_fallback_config; }

private:
  // DCI payload size configuration parameters.
  const dci_size_config base_dci_non_fallback_config = {
      // Flag for fallback DCI formats on a UE-specific Search Space.
      true,
      // Flag for non-fallback DCI formats on a UE-specific Search Space.
      true,
      // Bandwidth of the initial DL BWP in number of Resource Blocks.
      52,
      // Bandwidth of the active DL BWP in number of Resource Blocks.
      106,
      // Bandwidth of the initial UL BWP in number of Resource Blocks.
      52,
      // Bandwidth of the active UL BWP in number of Resource Blocks.
      106,
      // CORESET 0 DL bandwidth. Set to 0 if not configured.
      0,
      // Supplementary uplink flag.
      false,
      // Cross carrier scheduling flag.
      false,
      // PDSCH HARQ-ACK codebook type.
      pdsch_harq_ack_codebook::semistatic,
      // Number of UL BWP configured by higher layers.
      0,
      // Number of UL time domain allocations.
      16,
      // Size of the CSI request DCI field.
      0,
      // PUSCH frequency hopping flag.
      false,
      // PUSCH transmission scheme.
      tx_scheme_non_codebook(),
      // UL transform precoding flag.
      false,
      // PT-RS signals flag.
      false,
      // PUSCH dynamic beta offsets flag.
      false,
      // Dual dynamic HARQ-ACK codebook flag.
      false,
      // PUSCH resource allocation type for DCI format 0_1.
      resource_allocation::resource_allocation_type_1,
      // Number of UL RB groups.
      1,
      // Number of antenna ports used for PUSCH codebook-based transmission.
      1,
      // Number of SRS resources.
      1,
      // Maximum number of PUSCH layers for non-codebook based operation.
      1,
      // UL DM-RS type for DM-RS mapping type A.
      dmrs_config_type::type1,
      // Maximum number of OFDM symbols occupied by the front-loaded UL DM-RS for DM-RS mapping type A.
      dmrs_max_length::len1,
      // UL DM-RS type for DM-RS mapping type B.
      dmrs_config_type::type1,
      // Maximum number of OFDM symbols occupied by the front-loaded UL DM-RS for DM-RS mapping type B.
      dmrs_max_length::len1,
      // Maximum number of PUSCH Code Block Groups (CBG) per Transport Block (TB).
      2,
      // Number of DL BWP configured by higher layers.
      0,
      // Number of UL time domain allocations.
      16,
      // Number of configured aperiodic ZP CSI-RS resource sets.
      0,
      // Number of PDSCH to the DL ACK timings.
      1,
      // Dynamic PRB bundling flag.
      false,
      // Rate matching pattern group 1 flag.
      false,
      // Rate matching pattern group 2 flag.
      false,
      // Two codewords in PDSCH flag.
      false,
      // Multiple serving cells flag.
      false,
      // Transmission Configuration Indication (TCI) flag.
      false,
      // CBG flushing out information flag.
      false,
      // PDSCH resource allocation type for DCI format 1_1.
      resource_allocation::resource_allocation_type_1,
      // Number of DL RB groups.
      1,
      // Interleaved VRB-to-PRB mapping flag.
      false,
      // DM-RS type for PDSCH DM-RS mapping type A.
      dmrs_config_type::type1,
      // Maximum number of OFDM symbols occupied by the front-loaded DL DM-RS for DM-RS mapping type A.
      dmrs_max_length::len1,
      // DM-RS type for PDSCH DM-RS mapping type B.
      dmrs_config_type::type1,
      // Maximum number of OFDM symbols occupied by the front-loaded DL DM-RS for DM-RS mapping type B.
      dmrs_max_length::len1,
      // Maximum number of PDSCH Code Block Groups (CBG) per Transport Block (TB).
      2};
};

TEST_F(DciValidatorFallbackFixture, ValidConfiguration)
{
  dci_size_config config = get_base_dci_config();

  // Make sure the configuration is valid.
  ASSERT_TRUE(validate_dci_size_config(config));

  // Perform size alignment procedure.
  get_dci_sizes(config);
}

TEST_F(DciValidatorNonFallbackFixture, ValidConfiguration)
{
  dci_size_config config = get_base_dci_config();

  // Make sure the configuration is valid.
  ASSERT_TRUE(validate_dci_size_config(config));

  // Perform size alignment procedure.
  get_dci_sizes(config);
}

// Initial UL and DL BWP bandwidth checks.
TEST_F(DciValidatorFallbackFixture, BadInitialBandwidth)
{
  {
    dci_size_config config   = get_base_dci_config();
    config.ul_bwp_initial_bw = MAX_NOF_PRBS + 1;
    std::string assert_message =
        fmt::format("UL initial BWP bandwidth {} is out of range [1..{}].", config.ul_bwp_initial_bw, MAX_NOF_PRBS);

    test_validator(config, assert_message);
  }
  {
    dci_size_config config   = get_base_dci_config();
    config.ul_bwp_initial_bw = 0;
    std::string assert_message =
        fmt::format("UL initial BWP bandwidth {} is out of range [1..{}].", config.ul_bwp_initial_bw, MAX_NOF_PRBS);

    test_validator(config, assert_message);
  }
  {
    dci_size_config config   = get_base_dci_config();
    config.dl_bwp_initial_bw = MAX_NOF_PRBS + 1;
    std::string assert_message =
        fmt::format("DL initial BWP bandwidth {} is out of range [1..{}].", config.dl_bwp_initial_bw, MAX_NOF_PRBS);

    test_validator(config, assert_message);
  }
  {
    dci_size_config config   = get_base_dci_config();
    config.dl_bwp_initial_bw = 0;
    std::string assert_message =
        fmt::format("DL initial BWP bandwidth {} is out of range [1..{}].", config.dl_bwp_initial_bw, MAX_NOF_PRBS);

    test_validator(config, assert_message);
  }
  {
    dci_size_config config = get_base_dci_config();
    config.coreset0_bw     = MAX_NOF_PRBS + 1;
    std::string assert_message =
        fmt::format("CORESET0 bandwidth {} is out of range [0..{}].", config.coreset0_bw, MAX_NOF_PRBS);

    test_validator(config, assert_message);
  }
}

// Active UL and DL BWP bandwidth checks.
TEST_F(DciValidatorFallbackFixture, BadActiveBandwidth)
{
  {
    dci_size_config config  = get_base_dci_config();
    config.ul_bwp_active_bw = MAX_NOF_PRBS + 1;
    std::string assert_message =
        fmt::format("UL active BWP bandwidth {} is out of range [1..{}].", config.ul_bwp_active_bw, MAX_NOF_PRBS);

    test_validator(config, assert_message);
  }

  {
    dci_size_config config  = get_base_dci_config();
    config.dl_bwp_active_bw = MAX_NOF_PRBS + 1;
    std::string assert_message =
        fmt::format("DL active BWP bandwidth {} is out of range [1..{}].", config.dl_bwp_active_bw, MAX_NOF_PRBS);

    test_validator(config, assert_message);
  }
}

// SUL not configured check.
TEST_F(DciValidatorFallbackFixture, SupplementaryUplinkNotSupported)
{
  dci_size_config config     = get_base_dci_config();
  config.sul_configured      = true;
  std::string assert_message = fmt::format("SUL is not currently supported by the DCI size alignment procedure.");

  test_validator(config, assert_message);
}

// Checks for the number of UL and DL BWP configured by higher layers.
TEST_F(DciValidatorNonFallbackFixture, BadNofBwpConfiguredByHigherLayers)
{
  {
    dci_size_config config = get_base_dci_config();
    config.nof_ul_bwp_rrc  = 5;
    std::string assert_message =
        fmt::format("The number of RRC configured UL BWP {} is out of range [0..4].", config.nof_ul_bwp_rrc);

    test_validator(config, assert_message);
  }

  {
    dci_size_config config = get_base_dci_config();
    config.nof_dl_bwp_rrc  = 5;
    std::string assert_message =
        fmt::format("The number of RRC configured DL BWP {} is out of range [0..4].", config.nof_dl_bwp_rrc);

    test_validator(config, assert_message);
  }
}

// Checks for the number of UL and DL time domain resources.
TEST_F(DciValidatorNonFallbackFixture, BadNofTimeDomainResources)
{
  {
    dci_size_config config        = get_base_dci_config();
    config.nof_ul_time_domain_res = 17;
    std::string assert_message    = fmt::format("The number of UL time domain resources {} is out of range [1..16].",
                                             config.nof_ul_time_domain_res);

    test_validator(config, assert_message);
  }
  {
    dci_size_config config        = get_base_dci_config();
    config.nof_ul_time_domain_res = 0;
    std::string assert_message    = fmt::format("The number of UL time domain resources {} is out of range [1..16].",
                                             config.nof_ul_time_domain_res);

    test_validator(config, assert_message);
  }
  {
    dci_size_config config        = get_base_dci_config();
    config.nof_dl_time_domain_res = 17;
    std::string assert_message    = fmt::format("The number of DL time domain resources {} is out of range [1..16].",
                                             config.nof_dl_time_domain_res);

    test_validator(config, assert_message);
  }
  {
    dci_size_config config        = get_base_dci_config();
    config.nof_dl_time_domain_res = 0;
    std::string assert_message    = fmt::format("The number of DL time domain resources {} is out of range [1..16].",
                                             config.nof_dl_time_domain_res);

    test_validator(config, assert_message);
  }
}

// Check the number of aperiodic ZP CSI-RS resource sets.
TEST_F(DciValidatorNonFallbackFixture, MissingTransmitScheme)
{
  {
    dci_size_config config     = get_base_dci_config();
    config.pusch_tx_scheme     = std::nullopt;
    std::string assert_message = "PUSCH Transmit scheme is required for DCI 0_1.";

    test_validator(config, assert_message);
  }
}

// Check the number of aperiodic ZP CSI-RS resource sets.
TEST_F(DciValidatorNonFallbackFixture, BadAperiodicZpCsiResourceSets)
{
  dci_size_config config      = get_base_dci_config();
  config.nof_aperiodic_zp_csi = 4;
  std::string assert_message  = fmt::format("The number of aperiodic ZP-CSI resource sets {} is out of range [0..3].",
                                           config.nof_aperiodic_zp_csi);

  test_validator(config, assert_message);
}

// Check the CSI report trigger size.
TEST_F(DciValidatorNonFallbackFixture, BadReportTriggerSize)
{
  dci_size_config config     = get_base_dci_config();
  config.report_trigger_size = 7;
  std::string assert_message =
      fmt::format("CSI report trigger size {} is out of range [0..6].", config.report_trigger_size);

  test_validator(config, assert_message);
}

// Checks for the number of PDSCH HARQ-ACK timings.
TEST_F(DciValidatorNonFallbackFixture, BadNumberOfDonwlinkAckTimings)
{
  {
    dci_size_config config       = get_base_dci_config();
    config.nof_pdsch_ack_timings = 9;
    std::string assert_message   = fmt::format(
        "The number of HARQ-ACK feedback timing entries {} is out of range [1..8].", config.nof_pdsch_ack_timings);

    test_validator(config, assert_message);
  }
  {
    dci_size_config config       = get_base_dci_config();
    config.nof_pdsch_ack_timings = 0;
    std::string assert_message   = fmt::format(
        "The number of HARQ-ACK feedback timing entries {} is out of range [1..8].", config.nof_pdsch_ack_timings);

    test_validator(config, assert_message);
  }
}

// Checks for the number of CBGs per TB.
TEST_F(DciValidatorNonFallbackFixture, BadNumberOfCodeBlockGroupsPerTransportBlock)
{
  {
    dci_size_config config  = get_base_dci_config();
    config.max_cbg_tb_pusch = 1;
    std::string assert_message =
        fmt::format("The maximum CBG per PUSCH TB {} is neither 2, 4, 6, nor 8.", config.max_cbg_tb_pusch);

    test_validator(config, assert_message);
  }
  {
    dci_size_config config  = get_base_dci_config();
    config.max_cbg_tb_pdsch = 1;
    std::string assert_message =
        fmt::format("The maximum CBG per PDSCH TB {} is neither 2, 4, 6, nor 8.", config.max_cbg_tb_pdsch);

    test_validator(config, assert_message);
  }
}

// Checks for invalid DCI configurations with transform precoding enabled.
TEST_F(DciValidatorNonFallbackFixture, TransformPrecodingInvalidConfig)
{
  {
    dci_size_config config             = get_base_dci_config();
    config.transform_precoding_enabled = true;
    config.pusch_dmrs_A_type           = dmrs_config_type::type2;
    std::string assert_message = fmt::format("PUSCH DM-RS (A) Type2 is not supported with transform precoding.");

    test_validator(config, assert_message);
  }
  {
    dci_size_config config             = get_base_dci_config();
    config.transform_precoding_enabled = true;
    config.pusch_dmrs_B_type           = dmrs_config_type::type2;
    std::string assert_message = fmt::format("PUSCH DM-RS (B) Type2 is not supported with transform precoding.");

    test_validator(config, assert_message);
  }
}

// Checks for invalid DCI configurations with dynamic PDSCH HARQ-ACK codebook.
TEST_F(DciValidatorNonFallbackFixture, DynamicPdschHarqAckCodebookInvalidConfig)
{
  dci_size_config config   = get_base_dci_config();
  config.pdsch_harq_ack_cb = pdsch_harq_ack_codebook::dynamic;
  config.dynamic_dual_harq_ack_cb.reset();
  std::string assert_message =
      fmt::format("Dynamic dual HARQ-ACK codebook flag is required for dynamic PDSCH HARQ-ACK codebook.");

  test_validator(config, assert_message);
}

// Checks for invalid DCI configurations with UL and DL resource allocation type 0.
TEST_F(DciValidatorNonFallbackFixture, ResourceAllocationTypeZeroInvalidConfig)
{
  {
    dci_size_config config           = get_base_dci_config();
    config.pusch_res_allocation_type = resource_allocation::resource_allocation_type_0;
    config.nof_ul_rb_groups.reset();
    std::string assert_message = fmt::format("The number of UL RBGs is required for resource allocation type 0.");

    test_validator(config, assert_message);
  }
  {
    dci_size_config config           = get_base_dci_config();
    config.pusch_res_allocation_type = resource_allocation::resource_allocation_type_0;
    config.nof_ul_rb_groups          = MAX_NOF_RBGS + 1;
    std::string assert_message =
        fmt::format("The number of UL RBGs {} is out of range [1..{}].", config.nof_ul_rb_groups, MAX_NOF_RBGS);

    test_validator(config, assert_message);
  }
  {
    dci_size_config config           = get_base_dci_config();
    config.pusch_res_allocation_type = resource_allocation::resource_allocation_type_0;
    config.nof_ul_rb_groups          = 0;
    std::string assert_message =
        fmt::format("The number of UL RBGs {} is out of range [1..{}].", config.nof_ul_rb_groups, MAX_NOF_RBGS);

    test_validator(config, assert_message);
  }
  {
    dci_size_config config           = get_base_dci_config();
    config.pdsch_res_allocation_type = resource_allocation::resource_allocation_type_0;
    config.nof_dl_rb_groups.reset();
    std::string assert_message = fmt::format("The number of DL RBGs is required for resource allocation type 0.");

    test_validator(config, assert_message);
  }
  {
    dci_size_config config           = get_base_dci_config();
    config.pdsch_res_allocation_type = resource_allocation::resource_allocation_type_0;
    config.nof_dl_rb_groups          = MAX_NOF_RBGS + 1;
    std::string assert_message =
        fmt::format("The number of DL RBGs {} is out of range [1..{}].", config.nof_dl_rb_groups, MAX_NOF_RBGS);

    test_validator(config, assert_message);
  }
  {
    dci_size_config config           = get_base_dci_config();
    config.pdsch_res_allocation_type = resource_allocation::resource_allocation_type_0;
    config.nof_dl_rb_groups          = 0;
    std::string assert_message =
        fmt::format("The number of DL RBGs {} is out of range [1..{}].", config.nof_dl_rb_groups, MAX_NOF_RBGS);

    test_validator(config, assert_message);
  }
}

// Checks for invalid DCI configurations with DL resource allocation type 1.
TEST_F(DciValidatorNonFallbackFixture, ResourceAllocationTypeOneInvalidConfig)
{
  dci_size_config config           = get_base_dci_config();
  config.pdsch_res_allocation_type = resource_allocation::resource_allocation_type_1;
  config.interleaved_vrb_prb_mapping.reset();
  std::string assert_message =
      fmt::format("Interleaved VRB to PRB mapping flag is required for PDSCH resource allocation type 1.");

  test_validator(config, assert_message);
}

// Checks for invalid DCI configurations with non-codebook based transmission.
TEST_F(DciValidatorNonFallbackFixture, NonCodebookTransmissionInvalidConfig)
{
  {
    dci_size_config config = get_base_dci_config();
    config.pusch_tx_scheme = tx_scheme_non_codebook();
    config.pusch_max_layers.reset();
    std::string assert_message =
        fmt::format("Maximum number of PUSCH layers is required for non-codebook transmission.");

    test_validator(config, assert_message);
  }
  {
    dci_size_config config  = get_base_dci_config();
    config.pusch_tx_scheme  = tx_scheme_non_codebook();
    config.pusch_max_layers = 5;
    std::string assert_message =
        fmt::format("Maximum number of PUSCH layers {} is out of range [1..4].", config.pusch_max_layers);

    test_validator(config, assert_message);
  }
  {
    dci_size_config config  = get_base_dci_config();
    config.pusch_tx_scheme  = tx_scheme_non_codebook();
    config.pusch_max_layers = 0;
    std::string assert_message =
        fmt::format("Maximum number of PUSCH layers {} is out of range [1..4].", config.pusch_max_layers);

    test_validator(config, assert_message);
  }
  {
    dci_size_config config   = get_base_dci_config();
    config.pusch_tx_scheme   = tx_scheme_non_codebook();
    config.nof_srs_resources = 5;
    std::string assert_message =
        fmt::format("The number of SRS resources {} is out of range [1..4] for non-codebook transmission.",
                    config.nof_srs_resources);
    test_validator(config, assert_message);
  }
  {
    dci_size_config config   = get_base_dci_config();
    config.pusch_tx_scheme   = tx_scheme_non_codebook();
    config.nof_srs_resources = 0;
    std::string assert_message =
        fmt::format("The number of SRS resources {} is out of range [1..4] for non-codebook transmission.",
                    config.nof_srs_resources);
    test_validator(config, assert_message);
  }
}

// Checks for unsupported DCI configurations with non-codebook based transmission.
TEST_F(DciValidatorNonFallbackFixture, NonCodebookTransmissionUnsupportedConfig)
{
  {
    dci_size_config config     = get_base_dci_config();
    config.pusch_tx_scheme     = tx_scheme_non_codebook();
    config.pusch_max_layers    = 2;
    std::string assert_message = fmt::format("Multiple layers on PUSCH are not currently supported.");
    test_validator(config, assert_message);
  }
}

// Checks for invalid DCI configurations with codebook based transmission.
TEST_F(DciValidatorNonFallbackFixture, CodebookTransmissionInvalidConfig)
{
  {
    dci_size_config config = get_base_dci_config();
    config.pusch_tx_scheme = tx_scheme_codebook{
        .max_rank = 1, .codebook_subset = tx_scheme_codebook_subset::fully_and_partial_and_non_coherent};
    config.nof_srs_resources   = 3;
    std::string assert_message = fmt::format(
        "The number of SRS resources {} is out of range [1..2] for codebook transmission.", config.nof_srs_resources);

    test_validator(config, assert_message);
  }
  {
    dci_size_config config = get_base_dci_config();
    config.pusch_tx_scheme = tx_scheme_codebook{
        .max_rank = 1, .codebook_subset = tx_scheme_codebook_subset::fully_and_partial_and_non_coherent};
    config.nof_srs_ports.reset();
    std::string assert_message = fmt::format("Number of SRS ports is required for codebook transmission.");

    test_validator(config, assert_message);
  }
  {
    dci_size_config config = get_base_dci_config();
    config.pusch_tx_scheme = tx_scheme_codebook{
        .max_rank = 1, .codebook_subset = tx_scheme_codebook_subset::fully_and_partial_and_non_coherent};
    config.nof_srs_ports = 3;
    std::string assert_message =
        fmt::format("The number of SRS ports {} is neither 1, 2, nor 4.", config.nof_srs_ports);

    test_validator(config, assert_message);
  }
  {
    dci_size_config config = get_base_dci_config();
    config.pusch_tx_scheme =
        tx_scheme_codebook{.max_rank = 1, .codebook_subset = tx_scheme_codebook_subset::partial_and_non_coherent};
    config.nof_srs_ports = 2;
    std::string assert_message =
        fmt::format("Codebook subset \"partial and non-coherent\" is not supported for 2 SRS ports.");

    test_validator(config, assert_message);
  }
  {
    dci_size_config config = get_base_dci_config();
    config.pusch_tx_scheme =
        tx_scheme_codebook{.max_rank = 3, .codebook_subset = tx_scheme_codebook_subset::partial_and_non_coherent};
    config.nof_srs_ports = 2;
    std::string assert_message =
        fmt::format("Maximum rank {} cannot be larger than the number of SRS antenna ports {}.",
                    std::get<tx_scheme_codebook>(config.pusch_tx_scheme.value()).max_rank,
                    config.nof_srs_ports);

    test_validator(config, assert_message);
  }
}

// Checks for unsupported DCI configurations with PT-RS.
TEST_F(DciValidatorNonFallbackFixture, PtrsUnsupportedConfig)
{
  dci_size_config config        = get_base_dci_config();
  config.ptrs_uplink_configured = true;
  std::string assert_message    = "PT-RS is not currently supported.";

  test_validator(config, assert_message);
}

// Checks for DCI configurations where neither DM-RS mapping type A nor type B are properly configured.
TEST_F(DciValidatorNonFallbackFixture, DmrsConfigurationNotPresent)
{
  {
    dci_size_config config = get_base_dci_config();
    config.pusch_dmrs_A_type.reset();
    config.pusch_dmrs_B_max_len.reset();
    std::string assert_message = fmt::format("At least one PUSCH DM-RS mapping (type A or type B) must be configured.");

    test_validator(config, assert_message);
  }
  {
    dci_size_config config = get_base_dci_config();
    config.pdsch_dmrs_A_type.reset();
    config.pdsch_dmrs_B_max_len.reset();
    std::string assert_message = fmt::format("At least one PDSCH DM-RS mapping (type A or type B) must be configured.");

    test_validator(config, assert_message);
  }
}

} // namespace
