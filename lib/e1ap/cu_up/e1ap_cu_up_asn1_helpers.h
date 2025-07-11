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

#include "../common/e1ap_asn1_converters.h"
#include "srsran/asn1/e1ap/e1ap.h"
#include "srsran/asn1/e1ap/e1ap_ies.h"
#include "srsran/asn1/e1ap/e1ap_pdu_contents.h"
#include "srsran/e1ap/common/e1_setup_messages.h"
#include "srsran/e1ap/common/e1ap_types.h"
#include "srsran/e1ap/cu_up/e1ap_cu_up_bearer_context_update.h"
#include "srsran/ran/bcd_helper.h"
#include "srsran/ran/qos/five_qi.h"
#include "srsran/ran/qos/qos_prio_level.h"

namespace srsran {
namespace srs_cu_up {

inline asn1::e1ap::gnb_cu_up_e1_setup_request_s cu_up_e1_setup_request_to_asn1(const cu_up_e1_setup_request& request)
{
  asn1::e1ap::gnb_cu_up_e1_setup_request_s asn1_request;

  // Fill gNB CU-UP ID.
  asn1_request->gnb_cu_up_id = request.gnb_cu_up_id;

  // Fill gNB CU-UP name.
  if (request.gnb_cu_up_name.has_value()) {
    asn1_request->gnb_cu_up_name_present = true;
    asn1_request->gnb_cu_up_name.from_string(request.gnb_cu_up_name.value());
  }

  // Fill CN support.
  asn1_request->cn_support.value = static_cast<asn1::e1ap::cn_support_opts::options>(request.cn_support);

  // Fill supported PLMNs.
  for (const auto& plmn_item : request.supported_plmns) {
    asn1::e1ap::supported_plmns_item_s asn1_plmn_item;

    // Fill PLMN ID.
    asn1_plmn_item.plmn_id.from_number(bcd_helper::plmn_string_to_bcd(plmn_item.plmn_id));

    // Slice support list.
    for (const auto& slice_item : plmn_item.slice_support_list) {
      asn1::e1ap::slice_support_item_s asn1_slice_item;

      // Fill S-MSSAI.
      asn1_slice_item.snssai = snssai_to_e1ap_asn1(slice_item.s_nssai);

      asn1_plmn_item.slice_support_list.push_back(asn1_slice_item);
    }

    // Fill NR CGI support list.
    for (const auto& cgi_item : plmn_item.nr_cgi_support_list) {
      asn1::e1ap::nr_cgi_support_item_s asn1_cgi_item;

      // Fill NR CGI.
      asn1_cgi_item.nr_cgi = nr_cgi_to_e1ap_asn1(cgi_item.nr_cgi);

      asn1_plmn_item.nr_cgi_support_list.push_back(asn1_cgi_item);
    }

    // Fill NG-RAN QoS support list.
    if (!plmn_item.ng_ran_qos_support_list.empty()) {
      asn1_plmn_item.qos_params_support_list_present = true;

      for (const auto& ran_item : plmn_item.ng_ran_qos_support_list) {
        asn1::e1ap::ng_ran_qos_support_item_s asn1_ran_item;

        // Fill five QI.
        asn1_ran_item.non_dyn_5qi_descriptor.five_qi = five_qi_to_uint(ran_item.non_dyn_5qi_desc.five_qi);

        // Fill QoS prio level.
        if (ran_item.non_dyn_5qi_desc.qos_prio_level.has_value()) {
          asn1_ran_item.non_dyn_5qi_descriptor.qos_prio_level_present = true;
          asn1_ran_item.non_dyn_5qi_descriptor.qos_prio_level =
              ran_item.non_dyn_5qi_desc.qos_prio_level.value().value();
        }

        // Fill averaging window.
        if (ran_item.non_dyn_5qi_desc.averaging_win.has_value()) {
          asn1_ran_item.non_dyn_5qi_descriptor.averaging_win_present = true;
          asn1_ran_item.non_dyn_5qi_descriptor.averaging_win         = ran_item.non_dyn_5qi_desc.averaging_win.value();
        }

        // Fill max data burst.
        if (ran_item.non_dyn_5qi_desc.max_data_burst_volume.has_value()) {
          asn1_ran_item.non_dyn_5qi_descriptor.max_data_burst_volume_present = true;
          asn1_ran_item.non_dyn_5qi_descriptor.max_data_burst_volume =
              ran_item.non_dyn_5qi_desc.max_data_burst_volume.value();
        }

        asn1_plmn_item.qos_params_support_list.ng_ran_qos_support_list.push_back(asn1_ran_item);
      }
    }

    asn1_request->supported_plmns.push_back(asn1_plmn_item);
  }

  // Fill gNB CU-UP capacity.
  if (request.gnb_cu_up_capacity.has_value()) {
    asn1_request->gnb_cu_up_capacity_present = true;
    asn1_request->gnb_cu_up_capacity         = request.gnb_cu_up_capacity.value();
  }

  return asn1_request;
}

inline bool
check_e1ap_bearer_context_setup_request_valid(const asn1::e1ap::bearer_context_setup_request_s& asn1_request,
                                              srslog::basic_logger&                             logger)
{
  // We only support NG-RAN Bearer.
  if (asn1_request->sys_bearer_context_setup_request.type() !=
      asn1::e1ap::sys_bearer_context_setup_request_c::types::ng_ran_bearer_context_setup_request) {
    logger.error("Not handling E-UTRAN Bearers");
    return false;
  }

  // Check UE-AMBR makes sense.
  if (asn1_request->ue_dl_aggr_max_bit_rate == 0) {
    logger.error("Invalid UE-DL-AMBR");
    return false;
  }

  // Check activity level.
  if (asn1_request->activity_notif_level.value != asn1::e1ap::activity_notif_level_e::ue) {
    logger.warning("Unsupported activity notification level: {}", asn1_request->activity_notif_level.to_string());
    return false;
  }
  if (!asn1_request->ue_inactivity_timer_present) {
    logger.warning("Activity notification level is UE, but no UE inactivity timer present.");
    return false;
  }
  return true;
}

inline void fill_e1ap_drb_to_setup_item(e1ap_drb_to_setup_item_ng_ran&                drb_to_setup_item,
                                        const asn1::e1ap::drb_to_setup_item_ng_ran_s& asn1_drb_to_setup_item)
{
  drb_to_setup_item.drb_id = uint_to_drb_id(asn1_drb_to_setup_item.drb_id);

  // Fill SDAP config.
  drb_to_setup_item.sdap_cfg = e1ap_asn1_to_sdap_config(asn1_drb_to_setup_item.sdap_cfg);

  // Fill PDCP config.
  drb_to_setup_item.pdcp_cfg = e1ap_asn1_to_pdcp_config(asn1_drb_to_setup_item.pdcp_cfg);

  // Fill cell group info.
  e1ap_asn1_to_cell_group_info(drb_to_setup_item.cell_group_info, asn1_drb_to_setup_item.cell_group_info);

  // Fill QoS flow info to be setup.
  e1ap_asn1_to_flow_map_info(drb_to_setup_item.qos_flow_info_to_be_setup,
                             asn1_drb_to_setup_item.qos_flow_info_to_be_setup);

  // Fill DRB data forwarding info request.
  if (asn1_drb_to_setup_item.drb_data_forwarding_info_request_present) {
    drb_to_setup_item.drb_data_forwarding_info_request =
        e1ap_asn1_to_data_forwarding_info_request(asn1_drb_to_setup_item.drb_data_forwarding_info_request);
  }

  // Fill DRB inactivity timer.
  if (asn1_drb_to_setup_item.drb_inactivity_timer_present) {
    drb_to_setup_item.drb_inactivity_timer = std::chrono::seconds(asn1_drb_to_setup_item.drb_inactivity_timer);
  }

  // Fill PDCP SN status info.
  if (asn1_drb_to_setup_item.pdcp_sn_status_info_present) {
    drb_to_setup_item.pdcp_sn_status_info =
        e1ap_asn1_to_pdcp_sn_status_info(asn1_drb_to_setup_item.pdcp_sn_status_info);
  }
}

inline bool fill_e1ap_bearer_context_setup_request(e1ap_bearer_context_setup_request&                request,
                                                   const asn1::e1ap::bearer_context_setup_request_s& asn1_request)
{
  // Fill security info.
  request.security_info.security_algorithm.ciphering_algo =
      e1ap_asn1_to_ciphering_algorithm(asn1_request->security_info.security_algorithm.ciphering_algorithm);

  if (asn1_request->security_info.security_algorithm.integrity_protection_algorithm_present) {
    request.security_info.security_algorithm.integrity_protection_algorithm =
        e1ap_asn1_to_integrity_algorithm(asn1_request->security_info.security_algorithm.integrity_protection_algorithm);
  }

  request.security_info.up_security_key.encryption_key =
      asn1_request->security_info.up_securitykey.encryption_key.copy();

  request.security_info.up_security_key.integrity_protection_key =
      asn1_request->security_info.up_securitykey.integrity_protection_key.copy();

  // Fill UE DL aggregate maximum bit rate.
  request.ue_dl_aggregate_maximum_bit_rate = asn1_request->ue_dl_aggr_max_bit_rate;

  // Fill serving PLMN.
  request.serving_plmn = bcd_helper::plmn_bcd_to_string(asn1_request->serving_plmn.to_number());

  // Fill activity notification level.
  request.activity_notif_level = asn1_to_activity_notification_level(asn1_request->activity_notif_level.value);
  if (request.activity_notif_level == activity_notification_level_t::ue) {
    request.ue_inactivity_timer = std::chrono::seconds(asn1_request->ue_inactivity_timer);
  }

  // Fill PDU session resource to setup list.
  for (const auto& asn1_pdu_session_res_list_item :
       asn1_request->sys_bearer_context_setup_request.ng_ran_bearer_context_setup_request()) {
    for (const auto& asn1_pdu_session_res_item : asn1_pdu_session_res_list_item->pdu_session_res_to_setup_list()) {
      e1ap_pdu_session_res_to_setup_item pdu_session_res_item;

      // Fill PDU session ID.
      pdu_session_res_item.pdu_session_id = uint_to_pdu_session_id(asn1_pdu_session_res_item.pdu_session_id);

      // Fill PDU session type.
      if (!asn1_to_pdu_session_type(pdu_session_res_item.pdu_session_type,
                                    asn1_pdu_session_res_item.pdu_session_type)) {
        return false;
      }

      // Fill S-NSSAI.
      pdu_session_res_item.snssai = e1ap_asn1_to_snssai(asn1_pdu_session_res_item.snssai);

      // Fill NG UL UP transport layer information.
      pdu_session_res_item.ng_ul_up_tnl_info =
          asn1_to_up_transport_layer_info(asn1_pdu_session_res_item.ng_ul_up_tnl_info);

      // Fill security indication.
      asn1_to_security_indication(pdu_session_res_item.security_ind, asn1_pdu_session_res_item.security_ind);

      // Fill DRB to setup list NG-RAN.
      for (const auto& asn1_drb_to_setup_item : asn1_pdu_session_res_item.drb_to_setup_list_ng_ran) {
        e1ap_drb_to_setup_item_ng_ran drb_to_setup_item;
        fill_e1ap_drb_to_setup_item(drb_to_setup_item, asn1_drb_to_setup_item);
        pdu_session_res_item.drb_to_setup_list_ng_ran.emplace(uint_to_drb_id(asn1_drb_to_setup_item.drb_id),
                                                              drb_to_setup_item);
      }

      // Fill PDU session resource DL aggregate maximum bit rate.
      if (asn1_pdu_session_res_item.pdu_session_res_dl_ambr_present) {
        pdu_session_res_item.pdu_session_res_dl_ambr = asn1_pdu_session_res_item.pdu_session_res_dl_ambr;
      }

      // Fill PDU session data forwarding info request.
      if (asn1_pdu_session_res_item.pdu_session_data_forwarding_info_request_present) {
        e1ap_data_forwarding_info_request data_forwarding_info_req = {};
        data_forwarding_info_req.data_forwarding_request =
            asn1_pdu_session_res_item.pdu_session_data_forwarding_info_request.data_forwarding_request.to_string();

        for (const auto& asn1_qos_flows_forwarded_item :
             asn1_pdu_session_res_item.pdu_session_data_forwarding_info_request.qos_flows_forwarded_on_fwd_tunnels) {
          e1ap_qos_flow_map_item qos_flows_forwarded_item =
              asn1_e1ap_to_qos_flow_map_item(asn1_qos_flows_forwarded_item);

          data_forwarding_info_req.qos_flows_forwarded_on_fwd_tunnels.emplace(
              uint_to_qos_flow_id(asn1_qos_flows_forwarded_item.qos_flow_id), qos_flows_forwarded_item);
        }
      }

      // Fill PDU session inactivity timer.
      if (asn1_pdu_session_res_item.pdu_session_inactivity_timer_present) {
        pdu_session_res_item.pdu_session_inactivity_timer =
            std::chrono::seconds(asn1_pdu_session_res_item.pdu_session_inactivity_timer);
      }

      // Fill existing allocated NG DL UP TNL info.
      if (asn1_pdu_session_res_item.existing_allocated_ng_dl_up_tnl_info_present) {
        pdu_session_res_item.existing_allocated_ng_dl_up_tnl_info =
            asn1_to_up_transport_layer_info(asn1_pdu_session_res_item.existing_allocated_ng_dl_up_tnl_info);
      }

      // Fill network instance.
      if (asn1_pdu_session_res_item.network_instance_present) {
        pdu_session_res_item.network_instance = asn1_pdu_session_res_item.network_instance;
      }
      request.pdu_session_res_to_setup_list.emplace(uint_to_pdu_session_id(asn1_pdu_session_res_item.pdu_session_id),
                                                    pdu_session_res_item);
    }
  }

  // Fill UE DL maximum integrity protection data rate.
  if (asn1_request->ue_dl_max_integrity_protected_data_rate_present) {
    request.ue_dl_maximum_integrity_protected_data_rate = asn1_request->ue_dl_max_integrity_protected_data_rate;
  }

  // Fill UE inactivity timer.
  if (asn1_request->ue_inactivity_timer_present) {
    request.ue_inactivity_timer = std::chrono::seconds(asn1_request->ue_inactivity_timer);
  }

  return true;
}

inline void fill_asn1_bearer_context_setup_response(asn1::e1ap::sys_bearer_context_setup_resp_c& asn1_response,
                                                    const e1ap_bearer_context_setup_response&    response)
{
  auto& asn1_bearer_context_setup_response = asn1_response.set_ng_ran_bearer_context_setup_resp();

  // Fill PDU Session Resource Setup List.
  for (const auto& res_setup_item : response.pdu_session_resource_setup_list) {
    asn1::e1ap::pdu_session_res_setup_item_s asn1_res_setup_item;

    // Fill PDU Session ID.
    asn1_res_setup_item.pdu_session_id = pdu_session_id_to_uint(res_setup_item.pdu_session_id);

    // Fill NG DL UP TNL Info.
    up_transport_layer_info_to_asn1(asn1_res_setup_item.ng_dl_up_tnl_info, res_setup_item.ng_dl_up_tnl_info);

    // Fill DRB Setup List NG-RAN.
    e1ap_drb_item_list_to_asn1(asn1_res_setup_item.drb_setup_list_ng_ran, res_setup_item.drb_setup_list_ng_ran);

    // Fill DRB Failed List NG-RAN.
    e1ap_drb_failed_item_list_to_asn1(asn1_res_setup_item.drb_failed_list_ng_ran,
                                      res_setup_item.drb_failed_list_ng_ran);

    // Fill Security Result.
    if (res_setup_item.security_result.has_value()) {
      asn1_res_setup_item.security_result_present = true;
      security_result_to_asn1(asn1_res_setup_item.security_result, res_setup_item.security_result.value());
    }

    // Fill PDU Session Data Forwarding Info Response.
    if (res_setup_item.pdu_session_data_forwarding_info_resp.has_value()) {
      asn1_res_setup_item.pdu_session_data_forwarding_info_resp_present = true;
      if (res_setup_item.pdu_session_data_forwarding_info_resp.value().ul_data_forwarding.has_value()) {
        asn1_res_setup_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding_present = true;
        up_transport_layer_info_to_asn1(
            asn1_res_setup_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding,
            res_setup_item.pdu_session_data_forwarding_info_resp.value().ul_data_forwarding.value());
      }
      if (res_setup_item.pdu_session_data_forwarding_info_resp.value().dl_data_forwarding.has_value()) {
        asn1_res_setup_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding_present = true;
        up_transport_layer_info_to_asn1(
            asn1_res_setup_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding,
            res_setup_item.pdu_session_data_forwarding_info_resp.value().dl_data_forwarding.value());
      }
    }

    // Fill NG DL UP Unchanged.
    if (res_setup_item.ng_dl_up_unchanged.has_value()) {
      asn1_res_setup_item.ng_dl_up_unchanged_present = true;
      if (res_setup_item.ng_dl_up_unchanged.value()) {
        asn1_res_setup_item.ng_dl_up_unchanged.value =
            asn1::e1ap::pdu_session_res_setup_item_s::ng_dl_up_unchanged_opts::options::true_value;
      } else {
        asn1_res_setup_item.ng_dl_up_unchanged.value =
            asn1::e1ap::pdu_session_res_setup_item_s::ng_dl_up_unchanged_opts::options::nulltype;
      }
    }

    asn1_bearer_context_setup_response.pdu_session_res_setup_list.push_back(asn1_res_setup_item);
  }

  // Fill PDU session res failed list.
  if (!response.pdu_session_resource_failed_list.empty()) {
    asn1_bearer_context_setup_response.pdu_session_res_failed_list_present = true;
    for (const auto& failed_item : response.pdu_session_resource_failed_list) {
      asn1::e1ap::pdu_session_res_failed_item_s asn1_failed_item;
      asn1_failed_item.pdu_session_id = pdu_session_id_to_uint(failed_item.pdu_session_id);
      asn1_failed_item.cause          = cause_to_asn1(failed_item.cause);

      asn1_bearer_context_setup_response.pdu_session_res_failed_list.push_back(asn1_failed_item);
    }
  }
}

inline bool fill_e1ap_bearer_context_modification_request(e1ap_bearer_context_modification_request&       request,
                                                          const asn1::e1ap::bearer_context_mod_request_s& asn1_request)
{
  // Fill security info.
  if (asn1_request->security_info_present) {
    e1ap_security_info security_info;
    security_info.security_algorithm.ciphering_algo =
        e1ap_asn1_to_ciphering_algorithm(asn1_request->security_info.security_algorithm.ciphering_algorithm);
    security_info.security_algorithm.integrity_protection_algorithm =
        e1ap_asn1_to_integrity_algorithm(asn1_request->security_info.security_algorithm.integrity_protection_algorithm);
    security_info.up_security_key.encryption_key = asn1_request->security_info.up_securitykey.encryption_key.copy();
    security_info.up_security_key.integrity_protection_key =
        asn1_request->security_info.up_securitykey.integrity_protection_key.copy();

    request.security_info = security_info;
  }

  // Fill UE DL aggregate max bit rate.
  if (asn1_request->ue_dl_aggr_max_bit_rate_present) {
    request.ue_dl_aggr_max_bit_rate = asn1_request->ue_dl_aggr_max_bit_rate;
  }

  // Fill UE DL max integrity protected data rate.
  if (asn1_request->ue_dl_max_integrity_protected_data_rate_present) {
    request.ue_dl_max_integrity_protected_data_rate = asn1_request->ue_dl_max_integrity_protected_data_rate;
  }

  // Fill bearer context status change.
  if (asn1_request->bearer_context_status_change_present) {
    request.bearer_context_status_change =
        static_cast<e1ap_bearer_context_status_change>((int)asn1_request->bearer_context_status_change);
  }

  // Fill new UL TNL info required.
  if (asn1_request->new_ul_tnl_info_required_present) {
    request.new_ul_tnl_info_required =
        asn1_request->new_ul_tnl_info_required.value == asn1::e1ap::new_ul_tnl_info_required_opts::options::required;
  }

  // Fill UE inactivity timer.
  if (asn1_request->ue_inactivity_timer_present) {
    request.ue_inactivity_timer = std::chrono::seconds(asn1_request->ue_inactivity_timer);
  }

  // Fill data discard required.
  if (asn1_request->data_discard_required_present) {
    request.data_discard_required =
        asn1_request->data_discard_required.value == asn1::e1ap::data_discard_required_opts::required;
  }

  // Fill NG-RAN bearer context mod.
  if (asn1_request->sys_bearer_context_mod_request_present) {
    e1ap_ng_ran_bearer_context_mod_request ng_ran_bearer_context_mod_request;

    const auto& asn1_ng_ran_bearer_context_mod_request =
        asn1_request->sys_bearer_context_mod_request.ng_ran_bearer_context_mod_request();

    // Fill PDU session res to setup mod list.
    if (asn1_ng_ran_bearer_context_mod_request.pdu_session_res_to_setup_mod_list_present) {
      for (const auto& asn1_res_to_setup_mod_item :
           asn1_ng_ran_bearer_context_mod_request.pdu_session_res_to_setup_mod_list) {
        e1ap_pdu_session_res_to_setup_item pdu_session_res_to_setup_mod_item;

        // Fill PDU session ID.
        pdu_session_res_to_setup_mod_item.pdu_session_id =
            uint_to_pdu_session_id(asn1_res_to_setup_mod_item.pdu_session_id);

        // Fill PDU session type.
        if (!asn1_to_pdu_session_type(pdu_session_res_to_setup_mod_item.pdu_session_type,
                                      asn1_res_to_setup_mod_item.pdu_session_type)) {
          return false;
        }

        // Fill S-NSSAI.
        pdu_session_res_to_setup_mod_item.snssai = e1ap_asn1_to_snssai(asn1_res_to_setup_mod_item.snssai);

        // Fill security indication.
        asn1_to_security_indication(pdu_session_res_to_setup_mod_item.security_ind,
                                    asn1_res_to_setup_mod_item.security_ind);

        // Fill PDU session res ambr.
        if (asn1_res_to_setup_mod_item.pdu_session_res_ambr_present) {
          pdu_session_res_to_setup_mod_item.pdu_session_res_dl_ambr = asn1_res_to_setup_mod_item.pdu_session_res_ambr;
        }

        // Fill NG UL UP TNL info.
        pdu_session_res_to_setup_mod_item.ng_ul_up_tnl_info =
            asn1_to_up_transport_layer_info(asn1_res_to_setup_mod_item.ng_ul_up_tnl_info);

        // Fill PDU session data forwarding info request.
        if (asn1_res_to_setup_mod_item.pdu_session_data_forwarding_info_request_present) {
          e1ap_data_forwarding_info_request pdu_session_data_forwarding_info_request;
          pdu_session_data_forwarding_info_request.data_forwarding_request =
              asn1_res_to_setup_mod_item.pdu_session_data_forwarding_info_request.data_forwarding_request.to_string();

          for (const auto& asn1_qos_flows_forwarded_item :
               asn1_res_to_setup_mod_item.pdu_session_data_forwarding_info_request.qos_flows_forwarded_on_fwd_tunnels) {
            e1ap_qos_flow_map_item qos_flows_forwarded_item =
                asn1_e1ap_to_qos_flow_map_item(asn1_qos_flows_forwarded_item);

            pdu_session_data_forwarding_info_request.qos_flows_forwarded_on_fwd_tunnels.emplace(
                uint_to_qos_flow_id(asn1_qos_flows_forwarded_item.qos_flow_id), qos_flows_forwarded_item);
          }

          pdu_session_res_to_setup_mod_item.pdu_session_data_forwarding_info_request =
              pdu_session_data_forwarding_info_request;
        }

        // Fill PDU session inactivity timer.
        if (asn1_res_to_setup_mod_item.pdu_session_inactivity_timer_present) {
          pdu_session_res_to_setup_mod_item.pdu_session_inactivity_timer =
              std::chrono::seconds(asn1_res_to_setup_mod_item.pdu_session_inactivity_timer);
        }

        // Fill DRB to setup mod list NG-RAN.
        for (const auto& asn1_drb_to_setup_mod_item_ng_ran : asn1_res_to_setup_mod_item.drb_to_setup_mod_list_ng_ran) {
          e1ap_drb_to_setup_item_ng_ran drb_to_setup_mod_item_ng_ran;

          // Fill DRB ID.
          drb_to_setup_mod_item_ng_ran.drb_id = uint_to_drb_id(asn1_drb_to_setup_mod_item_ng_ran.drb_id);

          // Fill SDAP config.
          drb_to_setup_mod_item_ng_ran.sdap_cfg = e1ap_asn1_to_sdap_config(asn1_drb_to_setup_mod_item_ng_ran.sdap_cfg);

          // Fill PDCP config.
          drb_to_setup_mod_item_ng_ran.pdcp_cfg = e1ap_asn1_to_pdcp_config(asn1_drb_to_setup_mod_item_ng_ran.pdcp_cfg);

          // Fill cell group info.
          e1ap_asn1_to_cell_group_info(drb_to_setup_mod_item_ng_ran.cell_group_info,
                                       asn1_drb_to_setup_mod_item_ng_ran.cell_group_info);

          // Fill flow map info.
          e1ap_asn1_to_flow_map_info(drb_to_setup_mod_item_ng_ran.qos_flow_info_to_be_setup,
                                     asn1_drb_to_setup_mod_item_ng_ran.flow_map_info);

          // Fill DRB data forwarding info request.
          if (asn1_drb_to_setup_mod_item_ng_ran.drb_data_forwarding_info_request_present) {
            drb_to_setup_mod_item_ng_ran.drb_data_forwarding_info_request = e1ap_asn1_to_data_forwarding_info_request(
                asn1_drb_to_setup_mod_item_ng_ran.drb_data_forwarding_info_request);
          }

          // Fill DRB inactivity timer.
          if (asn1_drb_to_setup_mod_item_ng_ran.drb_inactivity_timer_present) {
            drb_to_setup_mod_item_ng_ran.drb_inactivity_timer =
                std::chrono::seconds(asn1_drb_to_setup_mod_item_ng_ran.drb_inactivity_timer);
          }

          // Fill PDCP SN status info.
          if (asn1_drb_to_setup_mod_item_ng_ran.pdcp_sn_status_info_present) {
            drb_to_setup_mod_item_ng_ran.pdcp_sn_status_info =
                e1ap_asn1_to_pdcp_sn_status_info(asn1_drb_to_setup_mod_item_ng_ran.pdcp_sn_status_info);
          }

          pdu_session_res_to_setup_mod_item.drb_to_setup_list_ng_ran.emplace(
              uint_to_drb_id(asn1_drb_to_setup_mod_item_ng_ran.drb_id), drb_to_setup_mod_item_ng_ran);
        }

        ng_ran_bearer_context_mod_request.pdu_session_res_to_setup_mod_list.emplace(
            uint_to_pdu_session_id(asn1_res_to_setup_mod_item.pdu_session_id), pdu_session_res_to_setup_mod_item);
      }
    }

    // Fill PDU session res to modify list.
    if (asn1_ng_ran_bearer_context_mod_request.pdu_session_res_to_modify_list_present) {
      for (const auto& asn1_res_to_mod_item : asn1_ng_ran_bearer_context_mod_request.pdu_session_res_to_modify_list) {
        e1ap_pdu_session_res_to_modify_item pdu_session_res_to_mod_item;
        pdu_session_res_to_mod_item.pdu_session_id = uint_to_pdu_session_id(asn1_res_to_mod_item.pdu_session_id);

        // Fill DRB to setup list.
        for (const auto& asn1_drb_to_setup_item : asn1_res_to_mod_item.drb_to_setup_list_ng_ran) {
          e1ap_drb_to_setup_item_ng_ran drb_to_setup_item;
          fill_e1ap_drb_to_setup_item(drb_to_setup_item, asn1_drb_to_setup_item);
          pdu_session_res_to_mod_item.drb_to_setup_list_ng_ran.emplace(uint_to_drb_id(asn1_drb_to_setup_item.drb_id),
                                                                       drb_to_setup_item);
        }

        // Fill DRB to modify list.
        for (const auto& asn1_drb_to_mod_item : asn1_res_to_mod_item.drb_to_modify_list_ng_ran) {
          e1ap_drb_to_modify_item_ng_ran drb_to_mod_item;
          drb_to_mod_item.drb_id = uint_to_drb_id(asn1_drb_to_mod_item.drb_id);

          // Fill SDAP config.
          if (asn1_drb_to_mod_item.sdap_cfg_present) {
            drb_to_mod_item.sdap_cfg = e1ap_asn1_to_sdap_config(asn1_drb_to_mod_item.sdap_cfg);
          }
          // Fill PDCP config.
          if (asn1_drb_to_mod_item.pdcp_cfg_present) {
            drb_to_mod_item.pdcp_cfg = e1ap_asn1_to_pdcp_config(asn1_drb_to_mod_item.pdcp_cfg);
          }
          // Fill DRB data forwarding info.
          if (asn1_drb_to_mod_item.drb_data_forwarding_info_present) {
            e1ap_data_forwarding_info drb_data_forwarding_info;
            drb_data_forwarding_info.ul_data_forwarding =
                asn1_to_up_transport_layer_info(asn1_drb_to_mod_item.drb_data_forwarding_info.ul_data_forwarding);
            drb_data_forwarding_info.dl_data_forwarding =
                asn1_to_up_transport_layer_info(asn1_drb_to_mod_item.drb_data_forwarding_info.dl_data_forwarding);
            drb_to_mod_item.drb_data_forwarding_info = drb_data_forwarding_info;
          }
          // Fill PDCP SN status request.
          if (asn1_drb_to_mod_item.pdcp_sn_status_request_present) {
            drb_to_mod_item.pdcp_sn_status_request = asn1_drb_to_mod_item.pdcp_sn_status_request.to_string();
          }
          // Fill DL UP params.
          for (const auto& asn1_dl_up_param : asn1_drb_to_mod_item.dl_up_params) {
            e1ap_up_params_item dl_up_params;
            dl_up_params.cell_group_id = asn1_dl_up_param.cell_group_id;
            dl_up_params.up_tnl_info   = asn1_to_up_transport_layer_info(asn1_dl_up_param.up_tnl_info);
            drb_to_mod_item.dl_up_params.push_back(dl_up_params);
          }
          // Fill cell group to add.
          for (const auto& asn1_cell_group_item_to_add : asn1_drb_to_mod_item.cell_group_to_add) {
            e1ap_cell_group_info_item cell_group_item_to_add;
            cell_group_item_to_add.cell_group_id = asn1_cell_group_item_to_add.cell_group_id;
            if (asn1_cell_group_item_to_add.ul_cfg_present) {
              cell_group_item_to_add.ul_cfg = static_cast<e1ap_ul_cfg>((int)asn1_cell_group_item_to_add.ul_cfg);
            }
            if (asn1_cell_group_item_to_add.dl_tx_stop_present) {
              cell_group_item_to_add.dl_tx_stop =
                  static_cast<e1ap_dl_tx_stop>((int)asn1_cell_group_item_to_add.dl_tx_stop);
            }
            if (asn1_cell_group_item_to_add.rat_type_present) {
              cell_group_item_to_add.rat_type = static_cast<e1ap_rat_type>((int)asn1_cell_group_item_to_add.rat_type);
            }
            drb_to_mod_item.cell_group_to_add.push_back(cell_group_item_to_add);
          }
          // Fill cell group to modify.
          for (const auto& asn1_cell_group_item_to_mod : asn1_drb_to_mod_item.cell_group_to_modify) {
            e1ap_cell_group_info_item cell_group_item_to_mod;
            cell_group_item_to_mod.cell_group_id = asn1_cell_group_item_to_mod.cell_group_id;
            if (asn1_cell_group_item_to_mod.ul_cfg_present) {
              cell_group_item_to_mod.ul_cfg = static_cast<e1ap_ul_cfg>((int)asn1_cell_group_item_to_mod.ul_cfg);
            }
            if (asn1_cell_group_item_to_mod.dl_tx_stop_present) {
              cell_group_item_to_mod.dl_tx_stop =
                  static_cast<e1ap_dl_tx_stop>((int)asn1_cell_group_item_to_mod.dl_tx_stop);
            }
            if (asn1_cell_group_item_to_mod.rat_type_present) {
              cell_group_item_to_mod.rat_type = static_cast<e1ap_rat_type>((int)asn1_cell_group_item_to_mod.rat_type);
            }
            drb_to_mod_item.cell_group_to_modify.push_back(cell_group_item_to_mod);
          }
          // Fill cell group to remove.
          for (const auto& asn1_cell_group_item_to_rem : asn1_drb_to_mod_item.cell_group_to_rem) {
            e1ap_cell_group_info_item cell_group_item_to_rem;
            cell_group_item_to_rem.cell_group_id = asn1_cell_group_item_to_rem.cell_group_id;
            if (asn1_cell_group_item_to_rem.ul_cfg_present) {
              cell_group_item_to_rem.ul_cfg = static_cast<e1ap_ul_cfg>((int)asn1_cell_group_item_to_rem.ul_cfg);
            }
            if (asn1_cell_group_item_to_rem.dl_tx_stop_present) {
              cell_group_item_to_rem.dl_tx_stop =
                  static_cast<e1ap_dl_tx_stop>((int)asn1_cell_group_item_to_rem.dl_tx_stop);
            }
            if (asn1_cell_group_item_to_rem.rat_type_present) {
              cell_group_item_to_rem.rat_type = static_cast<e1ap_rat_type>((int)asn1_cell_group_item_to_rem.rat_type);
            }
            drb_to_mod_item.cell_group_to_rem.push_back(cell_group_item_to_rem);
          }
          // Fill flow map info.
          e1ap_asn1_to_flow_map_info(drb_to_mod_item.flow_map_info, asn1_drb_to_mod_item.flow_map_info);

          // Fill DRB inactivity timer.
          if (asn1_drb_to_mod_item.drb_inactivity_timer_present) {
            drb_to_mod_item.drb_inactivity_timer = asn1_drb_to_mod_item.drb_inactivity_timer;
          }

          pdu_session_res_to_mod_item.drb_to_modify_list_ng_ran.emplace(uint_to_drb_id(asn1_drb_to_mod_item.drb_id),
                                                                        drb_to_mod_item);
        }

        // Fill DRB to rem list.
        for (const auto& asn1_drb_to_rem_item : asn1_res_to_mod_item.drb_to_rem_list_ng_ran) {
          pdu_session_res_to_mod_item.drb_to_rem_list_ng_ran.push_back(uint_to_drb_id(asn1_drb_to_rem_item.drb_id));
        }

        ng_ran_bearer_context_mod_request.pdu_session_res_to_modify_list.emplace(
            uint_to_pdu_session_id(asn1_res_to_mod_item.pdu_session_id), pdu_session_res_to_mod_item);
      }
    }

    // Fill PDU session res to rem list.
    if (asn1_ng_ran_bearer_context_mod_request.pdu_session_res_to_rem_list_present) {
      for (const auto& asn1_res_to_rem_item : asn1_ng_ran_bearer_context_mod_request.pdu_session_res_to_rem_list) {
        ng_ran_bearer_context_mod_request.pdu_session_res_to_rem_list.push_back(
            uint_to_pdu_session_id(asn1_res_to_rem_item.pdu_session_id));
      }
    }

    request.ng_ran_bearer_context_mod_request = ng_ran_bearer_context_mod_request;
  }

  // Fill RAN UE ID.
  if (asn1_request->ran_ue_id_present) {
    request.ran_ue_id = uint_to_ran_ue_id(asn1_request->ran_ue_id.to_number());
  }

  // Fill gNB DU ID.
  if (asn1_request->gnb_du_id_present) {
    request.gnb_du_id = asn1_request->gnb_du_id;
  }

  // Fill activity notif level.
  if (asn1_request->activity_notif_level_present) {
    request.activity_notif_level = static_cast<e1ap_activity_notif_level>((int)asn1_request->activity_notif_level);
  }

  return true;
}

inline void fill_asn1_bearer_context_modification_response(asn1::e1ap::sys_bearer_context_mod_resp_c& asn1_response,
                                                           const e1ap_bearer_context_modification_response& response)
{
  auto& asn1_bearer_context_modification_response = asn1_response.set_ng_ran_bearer_context_mod_resp();

  // Fill PDU session resource setup mod list.
  if (!response.pdu_session_resource_setup_list.empty()) {
    asn1_bearer_context_modification_response.pdu_session_res_setup_mod_list_present = true;

    for (const auto& res_setup_mod_item : response.pdu_session_resource_setup_list) {
      asn1::e1ap::pdu_session_res_setup_mod_item_s asn1_res_setup_mod_item;

      // Fill PDU session ID.
      asn1_res_setup_mod_item.pdu_session_id = pdu_session_id_to_uint(res_setup_mod_item.pdu_session_id);

      // Fill security result.
      if (res_setup_mod_item.security_result.has_value()) {
        asn1_res_setup_mod_item.security_result_present = true;
        security_result_to_asn1(asn1_res_setup_mod_item.security_result, res_setup_mod_item.security_result.value());
      }

      // Fill NG DL UP TNL info.
      up_transport_layer_info_to_asn1(asn1_res_setup_mod_item.ng_dl_up_tnl_info, res_setup_mod_item.ng_dl_up_tnl_info);

      // Fill PDU session data forwarding info response.
      if (res_setup_mod_item.pdu_session_data_forwarding_info_resp.has_value()) {
        asn1_res_setup_mod_item.pdu_session_data_forwarding_info_resp_present = true;
        if (res_setup_mod_item.pdu_session_data_forwarding_info_resp.value().ul_data_forwarding.has_value()) {
          asn1_res_setup_mod_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding_present = true;
          up_transport_layer_info_to_asn1(
              asn1_res_setup_mod_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding,
              res_setup_mod_item.pdu_session_data_forwarding_info_resp.value().ul_data_forwarding.value());
        }
        if (res_setup_mod_item.pdu_session_data_forwarding_info_resp.value().dl_data_forwarding.has_value()) {
          asn1_res_setup_mod_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding_present = true;
          up_transport_layer_info_to_asn1(
              asn1_res_setup_mod_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding,
              res_setup_mod_item.pdu_session_data_forwarding_info_resp.value().dl_data_forwarding.value());
        }
      }

      // Fill DRB setup mod list NG-RAN.
      e1ap_drb_item_list_to_asn1(asn1_res_setup_mod_item.drb_setup_mod_list_ng_ran,
                                 res_setup_mod_item.drb_setup_list_ng_ran);

      // Fill DRB failed mod list NG-RAN.
      e1ap_drb_failed_item_list_to_asn1(asn1_res_setup_mod_item.drb_failed_mod_list_ng_ran,
                                        res_setup_mod_item.drb_failed_list_ng_ran);

      asn1_bearer_context_modification_response.pdu_session_res_setup_mod_list.push_back(asn1_res_setup_mod_item);
    }
  }

  // Fill PDU session resource failed mod list.
  if (!response.pdu_session_resource_failed_list.empty()) {
    asn1_bearer_context_modification_response.pdu_session_res_failed_mod_list_present = true;

    for (const auto& res_failed_mod_item : response.pdu_session_resource_failed_list) {
      asn1::e1ap::pdu_session_res_failed_mod_item_s asn1_res_failed_mod_item;
      asn1_res_failed_mod_item.pdu_session_id = pdu_session_id_to_uint(res_failed_mod_item.pdu_session_id);
      asn1_res_failed_mod_item.cause          = cause_to_asn1(res_failed_mod_item.cause);

      asn1_bearer_context_modification_response.pdu_session_res_failed_mod_list.push_back(asn1_res_failed_mod_item);
    }
  }

  // Fill PDU session resource modified list.
  if (!response.pdu_session_resource_modified_list.empty()) {
    asn1_bearer_context_modification_response.pdu_session_res_modified_list_present = true;

    for (const auto& res_modified_item : response.pdu_session_resource_modified_list) {
      asn1::e1ap::pdu_session_res_modified_item_s asn1_res_modified_item;

      // Fill PDU session ID.
      asn1_res_modified_item.pdu_session_id = pdu_session_id_to_uint(res_modified_item.pdu_session_id);

      // Fill NG DL UP TNL info.
      if (res_modified_item.ng_dl_up_tnl_info.has_value()) {
        asn1_res_modified_item.ng_dl_up_tnl_info_present = true;
        up_transport_layer_info_to_asn1(asn1_res_modified_item.ng_dl_up_tnl_info,
                                        res_modified_item.ng_dl_up_tnl_info.value());
      }

      // Fill security result.
      if (res_modified_item.security_result.has_value()) {
        asn1_res_modified_item.security_result_present = true;
        security_result_to_asn1(asn1_res_modified_item.security_result, res_modified_item.security_result.value());
      }

      // Fill PDU session data forwarding info response.
      if (res_modified_item.pdu_session_data_forwarding_info_resp.has_value()) {
        asn1_res_modified_item.pdu_session_data_forwarding_info_resp_present = true;
        if (res_modified_item.pdu_session_data_forwarding_info_resp.value().ul_data_forwarding.has_value()) {
          asn1_res_modified_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding_present = true;
          up_transport_layer_info_to_asn1(
              asn1_res_modified_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding,
              res_modified_item.pdu_session_data_forwarding_info_resp.value().ul_data_forwarding.value());
        }
        if (res_modified_item.pdu_session_data_forwarding_info_resp.value().dl_data_forwarding.has_value()) {
          asn1_res_modified_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding_present = true;
          up_transport_layer_info_to_asn1(
              asn1_res_modified_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding,
              res_modified_item.pdu_session_data_forwarding_info_resp.value().dl_data_forwarding.value());
        }
      }

      // Fill DRB setup list NG-RAN.
      e1ap_drb_item_list_to_asn1(asn1_res_modified_item.drb_setup_list_ng_ran, res_modified_item.drb_setup_list_ng_ran);

      // Fill DRB failed list NG-RAN.
      e1ap_drb_failed_item_list_to_asn1(asn1_res_modified_item.drb_failed_list_ng_ran,
                                        res_modified_item.drb_failed_list_ng_ran);

      // Fill DRB modified list NG-RAN.
      for (const auto& drb_modified_item : res_modified_item.drb_modified_list_ng_ran) {
        asn1::e1ap::drb_modified_item_ng_ran_s asn1_drb_modified_item;

        // Fill DRB ID.
        asn1_drb_modified_item.drb_id = drb_id_to_uint(drb_modified_item.drb_id);

        // Fill UL UP transport params.
        for (const auto& ul_up_transport_param : drb_modified_item.ul_up_transport_params) {
          asn1::e1ap::up_params_item_s asn1_up_transport_param;
          up_transport_layer_info_to_asn1(asn1_up_transport_param.up_tnl_info, ul_up_transport_param.up_tnl_info);
          asn1_drb_modified_item.ul_up_transport_params.push_back(asn1_up_transport_param);
        }

        // Fill PDCP SN status info.
        if (drb_modified_item.pdcp_sn_status_info.has_value()) {
          asn1_drb_modified_item.pdcp_sn_status_info_present = true;

          // Fill PDCP status transfer UL.
          if (drb_modified_item.pdcp_sn_status_info.value()
                  .pdcp_status_transfer_ul.receive_status_of_pdcp_sdu.has_value()) {
            asn1_drb_modified_item.pdcp_sn_status_info.pdcp_status_transfer_ul.receive_statusof_pdcp_sdu_present = true;
            asn1_drb_modified_item.pdcp_sn_status_info.pdcp_status_transfer_ul.receive_statusof_pdcp_sdu.from_number(
                drb_modified_item.pdcp_sn_status_info.value()
                    .pdcp_status_transfer_ul.receive_status_of_pdcp_sdu.value());
          }
          asn1_drb_modified_item.pdcp_sn_status_info.pdcp_status_transfer_ul.count_value.pdcp_sn =
              drb_modified_item.pdcp_sn_status_info.value().pdcp_status_transfer_ul.count_value.pdcp_sn;
          asn1_drb_modified_item.pdcp_sn_status_info.pdcp_status_transfer_ul.count_value.hfn =
              drb_modified_item.pdcp_sn_status_info.value().pdcp_status_transfer_ul.count_value.hfn;

          // Fill PDCP status transfer DL.
          asn1_drb_modified_item.pdcp_sn_status_info.pdcp_status_transfer_dl.pdcp_sn =
              drb_modified_item.pdcp_sn_status_info.value().pdcp_status_transfer_dl.pdcp_sn;
          asn1_drb_modified_item.pdcp_sn_status_info.pdcp_status_transfer_dl.hfn =
              drb_modified_item.pdcp_sn_status_info.value().pdcp_status_transfer_dl.hfn;
        }

        // Fill flow setup list.
        for (const auto& qos_flow_item : drb_modified_item.flow_setup_list) {
          asn1::e1ap::qos_flow_item_s asn1_flow_item;
          asn1_flow_item.qos_flow_id = qos_flow_id_to_uint(qos_flow_item.qos_flow_id);
          asn1_drb_modified_item.flow_setup_list.push_back(asn1_flow_item);
        }

        // Fill flow failed list.
        for (const auto& flow_failed_item : drb_modified_item.flow_failed_list) {
          asn1::e1ap::qos_flow_failed_item_s asn1_flow_failed_item;
          asn1_flow_failed_item.qos_flow_id = qos_flow_id_to_uint(flow_failed_item.qos_flow_id);
          asn1_drb_modified_item.flow_failed_list.push_back(asn1_flow_failed_item);
        }

        asn1_res_modified_item.drb_modified_list_ng_ran.push_back(asn1_drb_modified_item);
      }

      // Fill DRB failed to modify list NG-RAN.
      e1ap_drb_failed_item_list_to_asn1(asn1_res_modified_item.drb_failed_to_modify_list_ng_ran,
                                        res_modified_item.drb_failed_to_modify_list_ng_ran);

      asn1_bearer_context_modification_response.pdu_session_res_modified_list.push_back(asn1_res_modified_item);
    }
  }

  // Fill PDU session resource failed to modify list.
  if (!response.pdu_session_resource_failed_to_modify_list.empty()) {
    asn1_bearer_context_modification_response.pdu_session_res_failed_to_modify_list_present = true;

    for (const auto& res_failed_to_modify_item : response.pdu_session_resource_failed_to_modify_list) {
      asn1::e1ap::pdu_session_res_failed_to_modify_item_s asn1_res_failed_to_modify_item;
      asn1_res_failed_to_modify_item.pdu_session_id = pdu_session_id_to_uint(res_failed_to_modify_item.pdu_session_id);
      asn1_res_failed_to_modify_item.cause          = cause_to_asn1(res_failed_to_modify_item.cause);

      asn1_bearer_context_modification_response.pdu_session_res_failed_to_modify_list.push_back(
          asn1_res_failed_to_modify_item);
    }
  }
}

} // namespace srs_cu_up
} // namespace srsran
