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

#pragma once

#include "uplink_request_processor_impl.h"
#include "upper_phy_error_handler_impl.h"
#include "upper_phy_pdu_validators.h"
#include "upper_phy_rx_results_notifier_wrapper.h"
#include "upper_phy_rx_symbol_handler_impl.h"
#include "srsran/phy/support/prach_buffer_pool.h"
#include "srsran/phy/support/resource_grid_pool.h"
#include "srsran/phy/upper/downlink_processor.h"
#include "srsran/phy/upper/rx_buffer_pool.h"
#include "srsran/phy/upper/uplink_processor.h"
#include "srsran/phy/upper/upper_phy.h"
#include "srsran/phy/upper/upper_phy_metrics_collector.h"
#include "srsran/phy/upper/upper_phy_timing_handler.h"
#include "srsran/phy/upper/upper_phy_timing_notifier.h"
#include "srsran/srslog/srslog.h"

namespace srsran {
/// Upper PHY implementation configuration.
struct upper_phy_impl_config {
  /// Uplink bandwidth in resource blocks.
  unsigned ul_bw_rb;
  /// Number of receive antenna ports.
  unsigned nof_rx_ports;
  /// Maximum number of layers for PUSCH transmissions.
  unsigned pusch_max_nof_layers;
  /// Downlink processor pool.
  std::unique_ptr<downlink_processor_pool> dl_processor_pool;
  /// Uplink processor pool.
  std::unique_ptr<uplink_processor_pool> ul_processor_pool;
  /// Downlink resource grid pool.
  std::unique_ptr<resource_grid_pool> dl_rg_pool;
  /// PRACH buffer pool.
  std::unique_ptr<prach_buffer_pool> prach_pool;
  /// Receive buffer pool.
  std::unique_ptr<rx_buffer_pool_controller> rx_buf_pool;
  /// Upper PHY results notifier.
  std::unique_ptr<upper_phy_rx_results_notifier_wrapper> rx_results_notifier;
  /// Symbol request notifier.
  upper_phy_rx_symbol_request_notifier* rx_symbol_request_notifier;
  /// Log level.
  srslog::basic_levels log_level;
  /// Receive symbol printer. Leave empty to disable.
  std::string rx_symbol_printer_filename;
  /// Receive port the symbols are dumped from. Leave emtpy for all ports.
  std::optional<unsigned> rx_symbol_printer_port;
  /// Boolean flag for dumping PRACH symbols when set to true.
  bool rx_symbol_printer_prach;
  /// Number of slots supported by the uplink PDU repository.
  size_t nof_slots_ul_pdu_repository;
  /// Downlink PDU validator.
  std::unique_ptr<downlink_pdu_validator> dl_pdu_validator;
  /// Uplink PDU validator.
  std::unique_ptr<uplink_pdu_validator> ul_pdu_validator;
  /// Metrics collector.
  std::unique_ptr<upper_phy_metrics_collector> metrics_collector;
};

/// \brief Implementation of the upper PHY interface.
///
/// Instances of this class will handle the ownership of the upper PHY components.
class upper_phy_impl : public upper_phy
{
  /// Upper PHY timing handler implementation class.
  class upper_phy_timing_handler_impl : public upper_phy_timing_handler
  {
    std::reference_wrapper<upper_phy_timing_notifier> notifier;

  public:
    upper_phy_timing_handler_impl(upper_phy_timing_notifier& notifier_) : notifier(notifier_) {}

    // See interface for documentation.
    void handle_tti_boundary(const upper_phy_timing_context& context) override
    {
      // Propagate the event.
      notifier.get().on_tti_boundary(context);
    }

    // See interface for documentation.
    void handle_ul_half_slot_boundary(const upper_phy_timing_context& context) override {}

    // See interface for documentation.
    void handle_ul_full_slot_boundary(const upper_phy_timing_context& context) override {}

    void set_upper_phy_notifier(upper_phy_timing_notifier& n) { notifier = std::ref(n); }
  };

public:
  /// Constructs an upper PHY implementation object with the given configuration.
  explicit upper_phy_impl(upper_phy_impl_config&& config);

  // See interface for documentation.
  upper_phy_error_handler& get_error_handler() override;

  // See interface for documentation.
  upper_phy_rx_symbol_handler& get_rx_symbol_handler() override;

  // See interface for documentation.
  upper_phy_timing_handler& get_timing_handler() override;

  // See interface for documentation.
  downlink_processor_pool& get_downlink_processor_pool() override;

  // See interface for documentation.
  resource_grid_pool& get_downlink_resource_grid_pool() override;

  // See interface for documentation.
  uplink_request_processor& get_uplink_request_processor() override;

  // See interface for documentation.
  uplink_pdu_slot_repository_pool& get_uplink_pdu_slot_repository() override;

  // See interface for documentation.
  upper_phy_metrics_collector* get_metrics_collector() override;

  // See interface for documentation.
  const downlink_pdu_validator& get_downlink_pdu_validator() const override;

  // See interface for documentation.
  const uplink_pdu_validator& get_uplink_pdu_validator() const override;

  // See interface for documentation.
  void set_error_notifier(upper_phy_error_notifier& notifier) override;

  // See interface for documentation.
  void set_timing_notifier(upper_phy_timing_notifier& notifier) override;

  // See interface for documentation.
  void set_rx_results_notifier(upper_phy_rx_results_notifier& notifier) override;

  void stop() override;

private:
  /// Upper PHY logger.
  srslog::basic_logger& logger;
  /// Metrics collector.
  std::unique_ptr<upper_phy_metrics_collector> metrics_collector;
  /// Receive buffer pool.
  std::unique_ptr<rx_buffer_pool_controller> rx_buf_pool;
  /// Downlink resource grid pool.
  std::unique_ptr<resource_grid_pool> dl_rg_pool;
  /// PRACH buffer pool.
  std::unique_ptr<prach_buffer_pool> prach_pool;
  /// Downlink processor pool.
  std::unique_ptr<downlink_processor_pool> dl_processor_pool;
  /// Uplink processor pool.
  std::unique_ptr<uplink_processor_pool> ul_processor_pool;
  /// Downlink processor PDUs validator.
  std::unique_ptr<downlink_pdu_validator> dl_pdu_validator;
  /// Uplink processor PDUs validator.
  std::unique_ptr<uplink_pdu_validator> ul_pdu_validator;
  /// Uplink request processor.
  uplink_request_processor_impl ul_request_processor;
  /// Upper PHY results notifier.
  std::unique_ptr<upper_phy_rx_results_notifier_wrapper> rx_results_notifier;
  /// Received symbols handler.
  std::unique_ptr<upper_phy_rx_symbol_handler> rx_symbol_handler;
  /// Timing events handler.
  upper_phy_timing_handler_impl timing_handler;
  /// Error events handler.
  upper_phy_error_handler_impl error_handler;
};
} // namespace srsran
