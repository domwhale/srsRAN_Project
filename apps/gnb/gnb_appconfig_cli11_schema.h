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

#include "CLI/CLI11.hpp"

namespace srsran {

struct gnb_appconfig;
struct cu_cp_unit_config;
struct du_high_unit_config;

/// Configures the given CLI11 application with the gNB application configuration schema.
void configure_cli11_with_gnb_appconfig_schema(CLI::App& app, gnb_appconfig& gnb_parsed_cfg);

/// Auto derive gNB parameters after the parsing.
void autoderive_gnb_parameters_after_parsing(CLI::App& app, gnb_appconfig& parsed_cfg);

/// Auto derive the supported TAs for the CU-CP AMF config from the DU cells config.
void autoderive_supported_tas_for_amf_from_du_cells(const du_high_unit_config& du_hi_cfg, cu_cp_unit_config& cu_cp_cfg);

} // namespace srsran
