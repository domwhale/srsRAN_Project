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

#include "srsran/ran/pdsch/pdsch_mcs.h"
#include "srsran/ran/pusch/pusch_mcs.h"
#include "srsran/ran/sch/sch_mcs.h"
#include <optional>

namespace srsran {

/// \brief Performs CQI to MCS mapping - for DL.
///
/// This is based on Tables 5.2.2.1-2, 5.2.2.1-3, 5.2.2.1-4 and Tables 5.1.3.1-1, 5.1.3.1-2, 5.1.3.1-3, TS 38.214.
///
/// \param[in] cqi CQI reported by the UE.
/// \param[in] mcs_table MCS table to be used for the mapping.
/// \return The MCS corresponding to map.
std::optional<sch_mcs_index> map_cqi_to_mcs(unsigned cqi, pdsch_mcs_table mcs_table);

/// \brief Maps the (PUSCH) SNR to a given MCS for PUSCH.
///
/// The objective of this function is to find the maximum MCS that can be used for a given SNR.
/// TODO: revise this function once the SNR to BLER curves will have been prepared.
sch_mcs_index map_snr_to_mcs_ul(double snr, pusch_mcs_table mcs_table, bool use_transform_precoder);

/// \brief Retrieves the maximum MCS value for a given MCS table.
///
/// The MCS value ranges are given in TS 38.214 Section 6.1.4.2.
inline sch_mcs_index get_max_mcs_ul(pusch_mcs_table mcs_table, bool transform_precoding)
{
  return ((mcs_table == pusch_mcs_table::qam256) or transform_precoding) ? 27 : 28;
}

} // namespace srsran
