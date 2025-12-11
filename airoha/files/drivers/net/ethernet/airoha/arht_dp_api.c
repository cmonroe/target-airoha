// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 AIROHA Inc
 * Author:  2024 AIROHA Inc
 */

#include <linux/proc_fs.h>
#include <net/ip.h>
#include <net/dsfield.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_bridge.h>
#include <linux/rhashtable.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <linux/rculist_nulls.h>
#include <linux/inet.h>
#include <net/netfilter/nf_flow_table.h>
#include <../net/bridge/br_private.h>
#include <../net/dsa/tag.h>
#include <linux/bitops.h>
#include <linux/regmap.h>
#include <arht_hook/ecnt_hook_gen_offload.h>

#include "arht_dp_api.h"
#include "airoha_eth.h"
#include "airoha_regs.h"
#include "eth_sal.h"
#include "arht_hook/ecnt_hook_qdma_type.h"

/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/
int eth_proc_init(void);
int eth_proc_exit(void);
int arht_multicast_hwnat_state_handler_lan_only(struct airoha_ppe *ppe, struct airoha_foe_entry *hwe, unsigned int foe_index, unsigned int  port_mask, int priority);
int arht_multicast_hwnat_state_handler_wlan_only(struct airoha_ppe *ppe, struct airoha_foe_entry *hwe, unsigned int foe_index, unsigned int  port_mask, int priority);
int arht_multicast_hwnat_state_handler_lan_wlan(struct airoha_ppe *ppe, struct airoha_foe_entry *hwe, unsigned int foe_index, unsigned int  port_mask, int priority);
int arht_multicast_hwnat_state_handler_lan_hsgmii_1toN(struct airoha_ppe *ppe, struct airoha_foe_entry *hwe, unsigned int foe_index, unsigned int  port_mask, int priority);
int arht_multicast_hwnat_state_handler_unknown(struct airoha_ppe *ppe, struct airoha_foe_entry *hwe, unsigned int foe_index, unsigned int  port_mask, int priority);
int find_and_update_shrink_table(int select, struct hwnat_shrink_field *shrinkFieldPtr);
int arht_multicast_handler_for_sfu(struct sk_buff* skb);
extern int (*arht_xpon_igmp_sfu_enable_hook)(struct net_device *port_dev);

QDMA_Private_T *gpQdmaPriv = NULL ;

extern struct airoha_eth *glb_eth;
extern int (*airoha_ppe_foe_commit_entry_ptr)(struct airoha_ppe *ppe,struct airoha_foe_entry *e,u32 hash,bool rx_wlan);

extern u32 airoha_ppe_foe_get_entry_hash(struct airoha_foe_entry *hwe);

extern bool airoha_ppe_foe_compare_entry(struct airoha_flow_table_entry *e,
					 struct airoha_foe_entry *hwe);
extern void airoha_ppe_delete_entry(struct airoha_ppe *ppe, struct airoha_foe_entry *hwe, uint entry_idx);

extern void EphyMonitor(void);

extern spinlock_t ppe_lock, flow_offload_lock;
/* airoha pon onu type
 * 0 --> unknown
 * 1 --> SFU
 * 2 --> HGU
 * */
int airoha_pon_onu_type = 0;
EXPORT_SYMBOL(airoha_pon_onu_type);


/* Warning: same sequence with enum 'FE_HookFunctionID_t' in ecnt_hook_fe.h */
fe_api_op_t
fe_operation[]=
{
	fe_api_set_pkt_length,
	fe_api_set_channel_enable,
	fe_api_set_mac_addr,
	fe_set_hwfwd_channel,
	fe_api_set_channel_retire,
	fe_api_set_crc_strip,
	fe_api_set_padding,
	fe_api_get_ext_tpid,
	fe_api_set_ext_tpid,
	fe_api_get_fw_cfg,
	fe_api_set_fw_cfg,
	fe_api_set_drop_udp_chksum_err_enable,
	fe_api_set_drop_tcp_chksum_err_enable,
	fe_api_set_drop_ip_chksum_err_enable,
	fe_api_set_drop_crc_err_enable,
	fe_api_set_drop_runt_enable,
	fe_api_set_drop_long_enable,
	fe_api_set_vlan_check,
	fe_api_get_ok_cnt,
	fe_api_get_rx_err_crc_cnt,
	fe_api_get_rx_drop_fifo_cnt,
	fe_api_get_rx_drop_err_cnt,
	fe_api_get_ok_byte_cnt,
	fe_api_get_tx_get_cnt,
	fe_api_get_tx_drop_cnt,
	fe_api_get_time_stamp,
	fe_api_set_time_stamp,
	fe_api_set_ins_vlan_tpid,
	fe_api_set_vlan_enable,
	fe_api_set_black_list,
	fe_api_set_ether_type,
	fe_api_set_L2U_key,
	fe_api_get_ac_group_pkt_cnt,
	fe_api_get_ac_group_byte_cnt,
	fe_api_clear_ac_group_pkt_cnt,
	fe_api_clear_ac_group_byte_cnt,
	fe_api_set_meter_group,
	fe_api_get_meter_group,
	fe_api_set_gdm_pcp_coding,
	fe_api_set_cdm_pcp_coding,
	fe_api_set_vip_enable,
	fe_api_get_eth_rx_cnt,
	fe_api_get_eth_tx_cnt,
	fe_api_get_eth_frame_cnt,
	fe_api_get_eth_err_cnt,
	fe_api_set_clear_mib,
	fe_api_set_cdm_rx_red_drop_mode,
	fe_api_get_cdm_rx_red_drop_mode,
	fe_api_set_channel_retire_all,
	fe_api_set_channel_retire_one,
	fe_api_set_tx_rate,
	fe_api_set_rxuc_rate,
	fe_api_set_rxbc_rate,
	fe_api_set_rxmc_rate,
	fe_api_set_rxoc_rate,
	fe_api_add_vip_ether,
	fe_api_add_vip_ppp,
	fe_api_add_vip_ip,
	fe_api_add_vip_tcp,
	fe_api_add_vip_udp,
	fe_api_del_vip_ether,
	fe_api_del_vip_ppp,
	fe_api_del_vip_ip,
	fe_api_del_vip_tcp,
	fe_api_del_vip_udp,	
	fe_api_add_l2lu_vlan_dscp,
	fe_api_add_l2lu_vlan_trfc,
	fe_api_del_l2lu_vlan_dscp,
	fe_api_del_l2lu_vlan_trfc,
	fe_api_add_traffic_class,
    fe_api_del_traffic_class,
	fe_api_set_tx_favor_oam_enable,
	fe_api_set_tls_cfg,
	fe_api_tls_forwad,
	fe_api_do_fe_reset,
	fe_api_set_mac_addr_7516,
	fe_api_set_wan_port_7516,
	fe_api_set_loopback_enable,
	fe_api_set_loopback_mode,
	fe_api_get_unknown_mul_pkt,
	fe_api_set_meter_ratelimit,
	fe_api_get_meter_ratelimit,
	fe_api_get_meter_idx,
	fe_api_get_acnt1_idx,
	fe_api_get_acnt0_idx,
	fe_api_init_resource_manage,
	fe_api_deinit_resource_manage,
	fe_api_set_rx_ratelimit_rule,
	fe_api_set_rx_ratelimit_mode,
	fe_api_set_meter_ctl_by_olt,
	fe_api_get_flow_cnt,
	fe_api_clear_flow_cnt,
	fe_api_get_acnt0_mode,
	fe_api_get_acnt1_mode,
	fe_api_set_acnt0_mode,
	fe_api_set_acnt1_mode,
	fe_api_get_meter_enable,
	fe_api_get_dev_mac_index,
	fe_api_set_pse_oq_threshold,
	fe_api_get_acnt2_idx,
	fe_api_set_acnt2_mode,
	fe_api_get_wan_itf_index,
	fe_api_set_glo_rate_byte,
	fe_api_get_pppoe_info,
	fe_api_set_pppoe_info_clean,
	fe_api_get_tx_traffic,  /* kbps */
	fe_api_get_rx_traffic,
	fe_api_get_tx_rate,     /* pps */
	fe_api_get_rx_rate,
	fe_api_get_tx_octets,  /* byte cnt in 15 minutes */
	fe_api_get_rx_octets,
	fe_api_get_rx_discard_counter, /*cnt in 15 minutes*/
	fe_api_get_tx_discard_counter,
	fe_api_get_rx_error_counter,
	fe_api_get_tx_error_counter,
	fe_api_add_dev_to_total_account,
	fe_api_add_stb_src_ip,
	fe_api_del_stb_src_ip,
	fe_api_set_ratelimit_for_pkt_formate,
	fe_api_set_mc_vlan_global,
	fe_api_get_mc_vlan_global,
	fe_api_set_mc_vlan_table_cfg,
	fe_api_get_mc_vlan_table_cfg,
	fe_api_set_mc_vlan_action_cfg,
	fe_api_get_mc_vlan_action_cfg,
	fe_api_set_mc_vlan_clear_all,
	fe_api_set_rx_mac_filter,
	fe_api_set_rx_mac_filter_rate,
	fe_api_xfi_link_change,
	fe_api_set_gdma_misc_config,
    fe_api_get_rx_ratelimit_mode,
	fe_api_get_hsgmii_rx_cnt,
	fe_api_get_hsgmii_tx_cnt,
	fe_api_set_aewan_fwdfq,
	fe_api_set_aewan_ifcdisable,
	fe_api_set_gdm2_sptag_for_loopback,
	fe_api_set_rx_rate,
	fe_api_set_tunnel_cfg,
	fe_api_set_gdm_sptag_for_extswitch,
	fe_api_pse_oq_rsv_en,
	fe_api_set_hsgmii_rx_port_ratelimit,
	fe_api_set_mbi_arb_rst,
	fe_api_set_rmbi_frag,
	fe_api_get_chn_rls,
	fe_api_set_tmbi_frag,
	fe_api_set_gdma_enable,
	fe_api_set_gdma_disable,
	fe_api_get_pse_drop_cnt,
    fe_api_set_chn_retire_action,
    fe_api_set_chn_retire_done,
    fe_api_set_qbi_fttr_chn_disable,
    fe_api_set_force_slow_enable,
    fe_api_set_force_slow_duty,
    fe_api_set_vip_rxq_selection,
    fe_api_set_vip_for_tcp_speedtest,
    fe_api_check_chn_rls,
	#ifndef TCSUPPORT_ACCOUNT_METER_V2
	fe_api_set_dev_stat_ratelimit_mode,
	#endif
	fe_api_set_clr_cnt
};

/* Warning: same sequence with enum 'QDMA_HookFunction_t' */
qdma_api_op_t
qdma_operation[]=
{
	/* init */
	/* 0x00 */
	qdma_init,
	qdma_deinit,
	qdma_tx_dma_mode,
	qdma_rx_dma_mode,
	qdma_loopback_mode,
	qdma_register_callback_function,
	qdma_unregister_callback_function,
	qdma_enable_rxpkt_int,
	qdma_disable_rxpkt_int,
	qdma_receive_packets,
	qdma_transmit_packet,
	qdma_set_tx_qos_weight,
	qdma_get_tx_qos_weight,
	qdma_set_tx_qos,
	qdma_get_tx_qos,
	qdma_set_mac_limit_threshold,
	qdma_get_mac_limit_threshold,
	qdma_bm_transmit_packet_wifi_fast,
	
	/* other */
	/* 0x12 */
	qdma_set_txbuf_threshold,
	qdma_get_txbuf_threshold,
	qdma_set_prefetch_mode,
	qdma_set_pktsize_overhead_en,
	qdma_get_pktsize_overhead_en,
	qdma_set_pktsize_overhead_value,
	qdma_get_pktsize_overhead_value, 
	qdma_set_lmgr_low_threshold,
	qdma_get_lmgr_low_threshold,
	qdma_get_lmgr_status,

	/* test */
	/* 0x1C */
	qdma_set_dbg_level,
	qdma_dump_dma_busy,
	qdma_dump_reg_polling,
	qdma_set_force_receive_rx_ring1,

	/* tx rate limit */
	/* 0x20 */
	qdma_set_tx_drop_en,
	qdma_get_tx_drop_en,
	qdma_set_tx_ratemeter,
	qdma_get_tx_ratemeter,
	qdma_enable_tx_ratelimit,
	qdma_set_tx_ratelimit_cfg,
	qdma_get_tx_ratelimit_cfg,
	qdma_set_tx_ratelimit,
	qdma_get_tx_ratelimit,
	qdma_set_tx_dba_report,
	qdma_get_tx_dba_report,
	
	/* rx rate limit */
	/* 0x2B */
	qdma_set_rx_protect_en,
	qdma_get_rx_protect_en,
	qdma_set_rx_low_threshold,
	qdma_get_rx_low_threshold,
	qdma_set_rx_ratelimit_en,
	qdma_set_rx_ratelimit_pkt_mode,
	qdma_get_rx_ratelimit_cfg,
	qdma_set_rx_ratelimit,
	qdma_get_rx_ratelimit,
	
	/* txq cngst */
	/* 0x34 */
	qdma_set_txq_dei_drop_mode,
	qdma_get_txq_dei_drop_mode,
	qdma_set_txq_cngst_mode,
	qdma_get_txq_cngst_mode,
	qdma_set_txq_dei_threshold_scale,
	qdma_get_txq_dei_threshold_scale,
	qdma_set_txq_cngst_auto_config,
	qdma_get_txq_cngst_auto_config,
	qdma_set_txq_cngst_dynamic_threshold,
	qdma_get_txq_cngst_dynamic_threshold,
	qdma_set_txq_cngst_total_threshold,
	qdma_get_txq_cngst_total_threshold,
	qdma_set_txq_cngst_channel_threshold,
	qdma_get_txq_cngst_channel_threshold,
	qdma_set_txq_cngst_queue_threshold,
	qdma_get_txq_cngst_queue_threshold,
	qdma_set_txq_peekrate_params,
	qdma_get_txq_peekrate_params,
	qdma_set_txq_cngst_static_queue_normal_threshold,
	qdma_set_txq_cngst_static_queue_dei_threshold,
	qdma_get_txq_cngst_dynamic_info,
	qdma_get_txq_cngst_static_info,
	qdma_set_txq_cngst_queue_nonblocking,
	qdma_get_txq_cngst_queue_nonblocking,
	qdma_set_txq_cngst_channel_nonblocking,
	qdma_get_txq_cngst_channel_nonblocking,
		
	/* virtual channel */
	/* 0x4E */
	qdma_set_virtual_channel_mode,
	qdma_get_virtual_channel_mode,
	qdma_set_virtual_channel_qos,
	qdma_get_virtual_channel_qos,

	/* dbg cntr */
	/* 0x52 */
	qdma_clear_and_set_dbg_cntr_channel_group,
	qdma_clear_and_set_dbg_cntr_queue_group,
	qdma_clear_dbg_cntr_value_all,
	qdma_dump_dbg_cntr_value,
	qdma_set_cntr_channel,
	qdma_dump_cntr_channel,
	qdma_get_dbg_cntr_all_queue_value,
	qdma_get_dbg_cntr_rx_ring,
	/* dump */
	/* 0x5A */
	qdma_dump_tx_qos,
	qdma_dump_virtual_channel_qos,
	qdma_dump_tx_ratelimit,
	qdma_dump_rx_ratelimit,
	qdma_dump_tx_dba_report,
	qdma_dump_txq_cngst,
	/* 0x60 */
	qdma_clear_cpu_counters,
	qdma_dump_cpu_counters,
	qdma_dump_register_value,
	qdma_dump_descriptor_info,
	qdma_dump_irq_info,
	qdma_dump_hwfwd_info,
	qdma_dump_info_all,
	/* 0x67 */
	qdma_read_vip_info,
    qdma_set_mac_qos_config,
	/* 0x69 */
	qdma_general_set_trtcm_cfg,
	qdma_general_get_trtcm_cfg,
	qdma_general_set_ratelimit_mode_cfg,
	qdma_general_get_ratelimit_mode_cfg,
	qdma_general_set_ratelimit_mode_value,
	qdma_general_get_ratelimit_mode_value,
	qdma_general_set_ratelimit_bucket_size,
    
	/*EN7527/16: new APIs*/
	/* 0x70 */
	qdma_clear_and_set_dbg_cntr_ring_group,
	/*EN7580: new APIs*/
	/* 0x71 */
	qdma_enable_rxpkt_int2,
	qdma_disable_rxpkt_int2,
	qdma_receive_packets_int2,
	/* 0x74 */
	qdma_general_set_trtcm_mode_cfg,
	qdma_general_get_trtcm_mode_cfg,
	qdma_general_set_trtcm_mode_value,
	qdma_general_get_trtcm_mode_value,
	qdma_general_set_trtcm_bucket_size,
	/* 0x79 */
	qdma_set_flow_cntr_cfg,
	qdma_get_flow_cntr_cfg,
	qdma_get_flow_cntr_value,
	qdma_clear_flow_cntr_value,	
	qdma_set_tx_wred_mode,
	qdma_get_tx_wred_mode,
	qdma_set_tx_wred_threshold,
	qdma_set_tx_wred_probability,
	qdma_get_tx_wred_cfg,
	qdma_set_cpu_rx_red_probability,
	qdma_get_cpu_rx_red_probability,
	/* 0x84 */
	qdma_set_channel_close_status,
	qdma_get_channel_close_status,
	qdma_get_channel_empty_status,
	qdma_set_oam_modify_fp_en,
	qdma_get_oam_modify_fp_en,
	qdma_set_multicast_en,
	qdma_get_multicast_en,
	/* 0x8B */
    qdma_allocate_meter,
    qdma_free_meter,
    qdma_get_qos_flag,
    qdma_allocate_acnt,
    qdma_free_acnt,

	/* 0x90 */
	qdma_set_sla_chnl_cfg,
	qdma_get_sla_chnl_cfg,
	qdma_set_qos_aging_cfg,
	qdma_get_qos_aging_cfg,
	qdma_set_per_queue_aging_cfg,
	qdma_get_per_queue_aging_cfg,
	qdma_set_multicast_sptag_cfg,
	qdma_get_multicast_sptag_cfg,
	qdma_set_multicast_fport_cfg,
	qdma_get_multicast_fport_cfg,
	qdma_set_txq_cngst_static_channel_en,
	qdma_set_txq_cngst_static_queue_ratio,
	qdma_get_hqos_en,

	/* 0x9D */
	qdma_reset,
	/* DRAM TEST APIs */
	qdma_dram_test_dma_config,
	qdma_dram_test_dma_enable,
	qdma_dram_test_is_rx_done,
	qdma_dram_test_dma_skb_get,

	qdma_get_using_tx_dscp_channel,
	qdma_set_multicast_1toN_cfg,
	qdma_get_channel_cfg,
	qdma_api_set_txmsg,
    qdma_api_get_rxmsg,
    qdma_api_get_rxmsg_eth,
	qdma_api_check_dscp_is_free,
	qdma_api_set_downstream_qos_mode,
	qdma_api_get_downstream_qos_mode,
	/* QDMA_FUNCTION_MAX_NUM */
	/* 0xAB */
	NULL ,
};

#define qdmaDisableUmacLoopback(qdma)	airoha_qdma_clear(qdma, REG_QDMA_GLOBAL_CFG, GLB_CFG_UMAC_LOOPBACK);
#define qdmaEnableQdmaLoopback(qdma)	airoha_qdma_set(qdma, REG_QDMA_GLOBAL_CFG, GLB_CFG_QDMA_LOOPBACK);
#define qdmaEnableUmacLoopback(qdma)	airoha_qdma_set(qdma, REG_QDMA_GLOBAL_CFG, GLB_CFG_UMAC_LOOPBACK);
#define qdmaDisableQdmaLoopback(qdma)	airoha_qdma_clear(qdma, REG_QDMA_GLOBAL_CFG, GLB_CFG_QDMA_LOOPBACK);

#define QDMA_CSR_QUEUE_CLOSE_CFG(idx)         (0x00a0+TXQ_DIS_QUEUE_CLOSE_OFFSET(idx))


#define GLB_CFG_TX_DMA_EN							(1<<0)
#define qdmaEnableTxDma(qdma)			airoha_qdma_set(qdma, REG_QDMA_GLOBAL_CFG, GLB_CFG_TX_DMA_EN);
#define qdmaDisableTxDma(qdma)			airoha_qdma_clear(qdma, REG_QDMA_GLOBAL_CFG, GLB_CFG_TX_DMA_EN);

#define GLB_CFG_RX_DMA_EN							(1<<2)
#define qdmaEnableRxDma(qdma)			airoha_qdma_set(qdma, REG_QDMA_GLOBAL_CFG, GLB_CFG_RX_DMA_EN);
#define qdmaDisableRxDma(qdma)			airoha_qdma_clear(qdma, REG_QDMA_GLOBAL_CFG, GLB_CFG_RX_DMA_EN);


/* QDMA_CSR_TXWRR_MODE_CFG	 */
#define TXWRR_WEIGHT_SCALE							(1<<31)
#define TXWRR_WEIGHT_BASE							(1<<3)

#define QDMA_CSR_TXWRR_MODE_CFG				(0x1020)
#define SMAC_PON_IDX 14
#define DEFAULT_SMAC_IDX 15

#define qdmaSetTxQosWeightByPacket(qdma)	airoha_qdma_clear(qdma, QDMA_CSR_TXWRR_MODE_CFG, TXWRR_WEIGHT_BASE);
#define qdmaSetTxQosWeightByByte(qdma)		airoha_qdma_set(qdma, QDMA_CSR_TXWRR_MODE_CFG, TXWRR_WEIGHT_BASE);
#define qdmaIsTxQosWeightByByte(qdma)		(airoha_qdma_rr(qdma, QDMA_CSR_TXWRR_MODE_CFG) & TXWRR_WEIGHT_BASE)
#define qdmaSetTxQosWeightScale64(qdma)		airoha_qdma_clear(qdma, QDMA_CSR_TXWRR_MODE_CFG, TXWRR_WEIGHT_SCALE);
#define qdmaSetTxQosWeightScale16(qdma)		airoha_qdma_set(qdma, QDMA_CSR_TXWRR_MODE_CFG, TXWRR_WEIGHT_SCALE);
#define qdmaIsTxQosWeightScale16(qdma)		(airoha_qdma_rr(qdma, QDMA_CSR_TXWRR_MODE_CFG) & TXWRR_WEIGHT_SCALE)


#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_WRR_WEIGHT_255 1
#else
#define SUPPORT_WRR_WEIGHT_255 0
#endif

#define QDMA_CSR_TXWRR_WEIGHT_CFG				(0x1024)

/* QDMA_CSR_TXWRR_WEIGHT_CFG	 */
#define TXWRR_RWCMD									(1<<31)
#define TXWRR_RWCMD_DONE							(1<<30)
#define TXWRR_CHNL_IDX_SHIFT						(19)
#define TXWRR_CHNL_IDX_MASK							(0x1F<<TXWRR_CHNL_IDX_SHIFT)
#define TXWRR_QUEUE_IDX_SHIFT						(16)
#define TXWRR_QUEUE_IDX_MASK						(0x7<<TXWRR_QUEUE_IDX_SHIFT)
#define TXWRR_WRR_VALUE_SHIFT						(0)
#define TXWRR_WRR_VALUE_MASK						(0xFF<<TXWRR_WRR_VALUE_SHIFT)

/* QDMA_CSR_PERCHNL_QOS_MODE */
#define TXQOS_CHNL_QOS_MODE_SHIFT(idx)				((idx&0x7)<<2)
#define TXQOS_CHNL_QOS_MODE_MASK(idx)				(0x7<<TXQOS_CHNL_QOS_MODE_SHIFT(idx))
#define QDMA_CSR_PERCHNL_QOS_MODE(base, i)			(base+0x1040+(i<<2))
//#define qdmaGetPerChnlQosMode(base, chnl)			IO_GMASK(QDMA_CSR_PERCHNL_QOS_MODE(base, (chnl>>3)), TXQOS_CHNL_QOS_MODE_MASK(chnl), TXQOS_CHNL_QOS_MODE_SHIFT(chnl))

//#define qdmaSetPerChnlQosMode(base, chnl, val)		IO_SMASK(QDMA_CSR_PERCHNL_QOS_MODE(base, (chnl>>3)), TXQOS_CHNL_QOS_MODE_MASK(chnl), TXQOS_CHNL_QOS_MODE_SHIFT(chnl), val)


#define qdmaGetPerChnlQosMode(qdma, chnl) \
    ((airoha_qdma_rr(qdma, QDMA_CSR_PERCHNL_QOS_MODE(0, (chnl >> 3))) & TXQOS_CHNL_QOS_MODE_MASK(chnl)) >> TXQOS_CHNL_QOS_MODE_SHIFT(chnl))


#define qdmaSetPerChnlQosMode(qdma, chnl, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_PERCHNL_QOS_MODE(0, (chnl >> 3)), TXQOS_CHNL_QOS_MODE_MASK(chnl), (val << TXQOS_CHNL_QOS_MODE_SHIFT(chnl)))

#define TXQ_DIS_CFG_REG_NUM				8
#define TXQ_DIS_QUEUE_CLOSE_OFFSET(chnl)		((chnl)&0xFC)
#define TXQ_DIS_CHANNEL_QUEUE_OFFSET(chnl,queue)	(1<<((queue)+(((chnl)&0x03)*8)))

#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_EN7580)
#define QDMA_CSR_TXQ_CNGST_CFG				(0x10a0)
#else
#define QDMA_CSR_TXQ_CNGST_CFG				(0x00A0)
#endif
#define qdmaEnableTxqCngstDeiDrop(qdma)		airoha_qdma_set(qdma, QDMA_CSR_TXQ_CNGST_CFG, TXQ_CNGST_DEI_DROP_EN)
#define qdmaDisableTxqCngstDeiDrop(qdma)	airoha_qdma_clear(qdma, QDMA_CSR_TXQ_CNGST_CFG, TXQ_CNGST_DEI_DROP_EN)

/* QDMA_CSR_TXQ_CNGST_QUEUE_NONBLOCKING_CFG */
#define TXQ_CNGST_QUEUE_NONBLOCKING_EN(idx)					(1<<idx)
#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_EN7580)
#define QDMA_CSR_TXQ_CNGST_QUEUE_NONBLOCKING_CFG	 	    (0x102c)
#else
#define QDMA_CSR_TXQ_CNGST_QUEUE_NONBLOCKING_CFG			(0x63c)
#endif
#define qdmaEnableTxqCngstQueueNonblocking(qdma, idx)		airoha_qdma_clear(qdma, QDMA_CSR_TXQ_CNGST_QUEUE_NONBLOCKING_CFG,  TXQ_CNGST_QUEUE_NONBLOCKING_EN(idx))
#define qdmaDisableTxqCngstQueueNonblocking(qdma, idx)		airoha_qdma_set(qdma, QDMA_CSR_TXQ_CNGST_QUEUE_NONBLOCKING_CFG,  TXQ_CNGST_QUEUE_NONBLOCKING_EN(idx))


/* QDMA_CSR_TXQ_CNGST_CHANNEL_NONBLOCKING_CFG */
#define TXQ_CNGST_CHANNEL_NONBLOCKING_EN(idx)				(1<<idx)
#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_EN7580)
#define QDMA_CSR_TXQ_CNGST_CHANNEL_NONBLOCKING_CFG		(0x1030)
#else
#define QDMA_CSR_TXQ_CNGST_CHANNEL_NONBLOCKING_CFG		(0x640)
#endif
#define qdmaEnableTxqCngstChannelNonblocking(qdma, idx)		airoha_qdma_clear(qdma, QDMA_CSR_TXQ_CNGST_CHANNEL_NONBLOCKING_CFG, TXQ_CNGST_CHANNEL_NONBLOCKING_EN(idx))
#define qdmaDisableTxqCngstChannelNonblocking(qdma, idx)		airoha_qdma_set(qdma, QDMA_CSR_TXQ_CNGST_CHANNEL_NONBLOCKING_CFG, TXQ_CNGST_CHANNEL_NONBLOCKING_EN(idx))


#define GLB_CFG_OAM_MODIFY_FP                       (1<<24)
#define qdmaEnableOamModifyFpEn(qdma)   airoha_qdma_set(qdma, QDMA_CSR_GLB_CFG, GLB_CFG_OAM_MODIFY_FP)
#define qdmaDisableOamModifyFpEn(qdma)  airoha_qdma_clear(qdma, QDMA_CSR_GLB_CFG, GLB_CFG_OAM_MODIFY_FP)
unchar qdmaLanHqosMode = 0;	/* 0:disable, 1:enable */
unchar qdmaWanHqosMode = 0;

typedef void (*qdma_rxmsg_op_t)(rxMsgWord_t* pRxMsg,uint* pValue);

#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_EN7580)
/*EN7580: new PSE buf manager engine, the function is not needed*/
#define QDMA_CSR_PSE_BUF_USAGE_CFG			(0x1028)
#else
#define QDMA_CSR_PSE_BUF_USAGE_CFG			(0x0090)
#endif
#define QDMA_CSR_PSE_BUF_USAGE_CFG1			(0x10d0)
/*QDMA_CSR_PSE_BUF_USAGE_CFG1*/
#define PSE_BUF_CHNL_THRSHLD_SHIFT					(16)
#define PSE_BUF_CHNL_THRSHLD_MASK					(0x7FF<<PSE_BUF_CHNL_THRSHLD_SHIFT)
#define PSE_BUF_TOTAL_THRSHLD_SHIFT					(0)
#define PSE_BUF_TOTAL_THRSHLD_MASK					(0x7FF<<PSE_BUF_TOTAL_THRSHLD_SHIFT)
#define PSE_BUF_ESTIMATE_EN_SHIFT					(29)

#define qdmaSetPseBufChnnelThreshold(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_PSE_BUF_USAGE_CFG1, PSE_BUF_CHNL_THRSHLD_MASK, (val << PSE_BUF_CHNL_THRSHLD_SHIFT))

#define qdmaSetPseBufTotalThreshold(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_PSE_BUF_USAGE_CFG1, PSE_BUF_TOTAL_THRSHLD_MASK, (val << PSE_BUF_TOTAL_THRSHLD_SHIFT))

#define qdmaDisablePseBufEstimate(qdma)				airoha_qdma_clear(qdma, QDMA_CSR_PSE_BUF_USAGE_CFG, PSE_BUF_ESTIMATE_EN_MASK)

/*
#define qdmaIsPseBufEstimateEnable(base)			(IO_GREG(QDMA_CSR_PSE_BUF_USAGE_CFG(base)) & PSE_BUF_ESTIMATE_EN_MASK)
#define qdmaGetPseBufChnnelThreshold(base)			IO_GMASK(QDMA_CSR_PSE_BUF_USAGE_CFG1(base), PSE_BUF_CHNL_THRSHLD_MASK, PSE_BUF_CHNL_THRSHLD_SHIFT)
#define qdmaGetPseBufTotalThreshold(base)			IO_GMASK(QDMA_CSR_PSE_BUF_USAGE_CFG1(base), PSE_BUF_TOTAL_THRSHLD_MASK, PSE_BUF_TOTAL_THRSHLD_SHIFT)
*/
#define qdmaIsPseBufEstimateEnable(qdma)            (airoha_qdma_rr(qdma, QDMA_CSR_PSE_BUF_USAGE_CFG) & PSE_BUF_ESTIMATE_EN_MASK)
#define qdmaGetPseBufChnnelThreshold(qdma)          ((airoha_qdma_rr(qdma, QDMA_CSR_PSE_BUF_USAGE_CFG1) & PSE_BUF_CHNL_THRSHLD_MASK) >> PSE_BUF_CHNL_THRSHLD_SHIFT)
#define qdmaGetPseBufTotalThreshold(qdma)           ((airoha_qdma_rr(qdma, QDMA_CSR_PSE_BUF_USAGE_CFG1) & PSE_BUF_TOTAL_THRSHLD_MASK) >> PSE_BUF_TOTAL_THRSHLD_SHIFT)

#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_EN7580)
#define QDMA_CSR_DBG_CNTR_VAR(i)				(0x0404+(i<<3))
#else
#define QDMA_CSR_DBG_CNTR_VAR(i)				(0x0304+(i<<3))
#endif

#define qdmaGetCntrCounter(qdma, idx)		airoha_qdma_rr(qdma, QDMA_CSR_DBG_CNTR_VAR(idx));


#define TXQ_DYN_CNGSTCTL_MAX_THRH_TXRING_EN			(1<<1)
#define TXQ_DYN_CNGSTCTL_MIN_THRH_TXRING_EN			(1<<0)

#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_EN7580)
#define RING_OFFSET(idx)                            (idx<<5)
#else
#define RING_OFFSET(idx)                            ((idx&0xE)?((idx-2)<<5):(idx<<8))
#endif

#if defined(TCSUPPORT_CPU_EN7581)
#define QDMA_CSR_TX_DSCP_BASE(idx)			(((idx)<8) ? (0x0100+RING_OFFSET(idx)) : (0x0b00+RING_OFFSET(((idx)-8))))
#define QDMA_CSR_TX_BLOCKING(idx)				(((idx)<8) ? (0x0104+RING_OFFSET(idx)) : (0x0b04+RING_OFFSET(((idx)-8))))
#define QDMA_CSR_TX_CPU_IDX(idx)				(((idx)<8) ? (0x0108+RING_OFFSET(idx)) : (0x0b08+RING_OFFSET(((idx)-8))))
#define QDMA_CSR_TX_DMA_IDX(idx)				(((idx)<8) ? (0x010c+RING_OFFSET(idx)) : (0x0b0c+RING_OFFSET(((idx)-8))))
#define QDMA_CSR_TX_CPU_DONE_NUM(idx)			(((idx)<8) ? (0x011c+RING_OFFSET(idx)) : (0x0b1C+RING_OFFSET(((idx)-8)))) 
#else
#define QDMA_CSR_TX_DSCP_BASE(idx)			(0x0100+RING_OFFSET(idx))
#define QDMA_CSR_TX_BLOCKING(idx)				(0x0104+RING_OFFSET(idx))
#define QDMA_CSR_TX_CPU_IDX(idx)				(0x0108+RING_OFFSET(idx))
#define QDMA_CSR_TX_DMA_IDX(idx)				(0x010C+RING_OFFSET(idx))
#define QDMA_CSR_TX_CPU_DONE_NUM(idx)			(0x011C+RING_OFFSET(idx))
#endif

#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
#define NEED_ENABLE_TX_RING_BLOCKING 1
#else
#define NEED_ENABLE_TX_RING_BLOCKING 0
#endif

#define EGRESS_RATEMETER_EN							(1<<31)
#define EGRESS_RATEMETER_PEEKRATE_EN				(1<<30)
#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_EN7580)
#define QDMA_CSR_EGRESS_RATEMETER_CFG			(0x100c)
#else
#define QDMA_CSR_EGRESS_RATEMETER_CFG			(0x0094)
#endif
#define TXQ_DYN_CNGSTCTL_MODE_CONFIG_TRIGGER		(1<<18)
#define TXQ_DYN_CNGSTCTL_MODE_PACKET_TRIGGER		(1<<17)
#define TXQ_DYN_CNGSTCTL_MODE_TIME_TRIGGER			(1<<16)

int BUFFER_THRESHOLD[6] = {0};
#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_TXQ_CNGST_MIN_THRH 1
#else
#define SUPPORT_TXQ_CNGST_MIN_THRH 0
#endif

#define TXQ_CNGST_MIN_THRH 0

#define TXQ_DYN_CNGSTCTL_EN							(1<<29)
#define TXQ_DYN_CNGSTCTL_EN							(1<<29)

#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_EN7581)
#define SUPPORT_TX_PEAK_MODE		1
#else
#define SUPPORT_TX_PEAK_MODE		0
#endif

#define EGRESS_RATEMETER_PEEKRATE_DURATION_SHIFT	(18)
#define EGRESS_RATEMETER_PEEKRATE_DURATION_MASK		(0xFFF<<EGRESS_RATEMETER_PEEKRATE_DURATION_SHIFT)

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
#define SUPPORT_TXQ_DEI_DROP 1
#else
#define SUPPORT_TXQ_DEI_DROP 0
#endif

/* QDMA_CSR_TXQ_DEI_TOTALTHR	 */
#define TXQ_CNGST_DEI_TOTAL_MAX_THR_SHIFT			(16)
#define TXQ_CNGST_DEI_TOTAL_MAX_THR_MASK			(0xFFFF<<TXQ_CNGST_DEI_TOTAL_MAX_THR_SHIFT)
#define TXQ_CNGST_DEI_TOTAL_MIN_THR_SHIFT			(0)
#define TXQ_CNGST_DEI_TOTAL_MIN_THR_MASK			(0xFFFF<<TXQ_CNGST_DEI_TOTAL_MIN_THR_SHIFT)

/* QDMA_CSR_TXQ_DEI_CHNLTHR_CFG	 */
#define TXQ_CNGST_DEI_CHNL_MAX_THR_SHIFT			(16)
#define TXQ_CNGST_DEI_CHNL_MAX_THR_MASK				(0xFFFF<<TXQ_CNGST_DEI_CHNL_MAX_THR_SHIFT)
#define TXQ_CNGST_DEI_CHNL_MIN_THR_SHIFT			(0)
#define TXQ_CNGST_DEI_CHNL_MIN_THR_MASK				(0xFFFF<<TXQ_CNGST_DEI_CHNL_MIN_THR_SHIFT)

/* QDMA_CSR_TXQ_DEI_QUEUETHR_CFG	 */
#define TXQ_CNGST_DEI_QUEUE_MAX_THR_SHIFT			(16)
#define TXQ_CNGST_DEI_QUEUE_MAX_THR_MASK			(0xFFFF<<TXQ_CNGST_DEI_QUEUE_MAX_THR_SHIFT)
#define TXQ_CNGST_DEI_QUEUE_MIN_THR_SHIFT			(0)
#define TXQ_CNGST_DEI_QUEUE_MIN_THR_MASK			(0xFFFF<<TXQ_CNGST_DEI_QUEUE_MIN_THR_SHIFT)

#define QDMA_CSR_TXQ_DEI_TOTALTHR				(0x10d4)
#define QDMA_CSR_TXQ_DEI_CHNLTHR_CFG			(0x10d8)
#define QDMA_CSR_TXQ_DEI_QUEUETHR_CFG			(0x10dc)

#if defined(TCSUPPORT_CPU_AN7583)
#define SUPPORT_FAST_THR	1
#else
#define SUPPORT_FAST_THR	0
#endif

#define QDMA_CSR_TXQ_DYN_TOTALTHR				(0x00A4)
#define QDMA_CSR_TXQ_DYN_CHNLTHR_CFG			(0x00A8)
#define QDMA_CSR_TXQ_DYN_QUEUETHR_CFG			(0x00AC)

/* QDMA_CSR_TXQ_DYN_TOTALTHR	 */
#define TXQ_CNGST_TOTAL_MAX_THR_SHIFT				(16)
#define TXQ_CNGST_TOTAL_MAX_THR_MASK				(0xFFFF<<TXQ_CNGST_TOTAL_MAX_THR_SHIFT)
#define TXQ_CNGST_TOTAL_MIN_THR_SHIFT				(0)
#define TXQ_CNGST_TOTAL_MIN_THR_MASK				(0xFFFF<<TXQ_CNGST_TOTAL_MIN_THR_SHIFT)

/* QDMA_CSR_TXQ_DYN_CHNLTHR_CFG	 */
#define TXQ_CNGST_CHNL_MAX_THR_SHIFT				(16)
#define TXQ_CNGST_CHNL_MAX_THR_MASK					(0xFFFF<<TXQ_CNGST_CHNL_MAX_THR_SHIFT)
#define TXQ_CNGST_CHNL_MIN_THR_SHIFT				(0)
#define TXQ_CNGST_CHNL_MIN_THR_MASK					(0xFFFF<<TXQ_CNGST_CHNL_MIN_THR_SHIFT)

/* QDMA_CSR_TXQ_DYN_QUEUETHR_CFG	 */
#define TXQ_CNGST_QUEUE_MAX_THR_SHIFT				(16)
#define TXQ_CNGST_QUEUE_MAX_THR_MASK				(0xFFFF<<TXQ_CNGST_QUEUE_MAX_THR_SHIFT)
#define TXQ_CNGST_QUEUE_MIN_THR_SHIFT				(0)
#define TXQ_CNGST_QUEUE_MIN_THR_MASK				(0xFFFF<<TXQ_CNGST_QUEUE_MIN_THR_SHIFT)


#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_TXQ_CNGST_MIN_THRH 1
#else
#define SUPPORT_TXQ_CNGST_MIN_THRH 0
#endif

#define TXQ_MIN_DSCP_THRSHLD_SHIFT					(0)
#define TXQ_MIN_DSCP_THRSHLD_MASK					(0xFFFF<<TXQ_MIN_DSCP_THRSHLD_SHIFT)
#define QDMA_CSR_TXQ_MIN_DSCP_THRH			(0x1008)

/*
#define qdmaDisableTxqDynCngstEn(base)		IO_CBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_EN)

#define qdmaDisableTxRateMeterPeakRate(base)		IO_CBITS(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_PEEKRATE_EN)

#define qdmaEnableTxqDynCngstEn(base)		IO_SBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_EN)

#define qdmaDisableCngstMaxThrhEn(base, idx)	IO_CBITS(QDMA_CSR_TX_BLOCKING(base, idx), TXQ_DYN_CNGSTCTL_MAX_THRH_TXRING_EN)

#define qdmaDisableCngstMinThrhEn(base, idx)	IO_CBITS(QDMA_CSR_TX_BLOCKING(base, idx), TXQ_DYN_CNGSTCTL_MIN_THRH_TXRING_EN)

#define qdmaEnableCngstMinThrhEn(base, idx)		IO_SBITS(QDMA_CSR_TX_BLOCKING(base, idx), TXQ_DYN_CNGSTCTL_MIN_THRH_TXRING_EN)

#define qdmaEnableCngstModeConfigTrig(base)		IO_SBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_MODE_CONFIG_TRIGGER)

#define qdmaEnableCngstModePacketTrig(base)		IO_SBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_MODE_PACKET_TRIGGER)

#define qdmaEnableCngstModeTimeTrig(base)		IO_SBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_MODE_TIME_TRIGGER)

#define qdmaEnableTxRateMeter(base)			IO_SBITS(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_EN)

#define qdmaEnableTxRateMeterPeakRate(base)			IO_SBITS(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_PEEKRATE_EN)
*/

#define qdmaDisableTxqDynCngstEn(qdma) \
    airoha_qdma_clear(qdma, QDMA_CSR_TXQ_CNGST_CFG, TXQ_DYN_CNGSTCTL_EN)

#define qdmaDisableTxRateMeterPeakRate(qdma) \
    airoha_qdma_clear(qdma, QDMA_CSR_EGRESS_RATEMETER_CFG, EGRESS_RATEMETER_PEEKRATE_EN)

#define qdmaEnableTxqDynCngstEn(qdma) \
    airoha_qdma_set(qdma, QDMA_CSR_TXQ_CNGST_CFG, TXQ_DYN_CNGSTCTL_EN)

#define qdmaDisableCngstMaxThrhEn(qdma, idx) \
    airoha_qdma_clear(qdma, QDMA_CSR_TX_BLOCKING(idx), TXQ_DYN_CNGSTCTL_MAX_THRH_TXRING_EN)

#define qdmaDisableCngstMinThrhEn(qdma, idx) \
    airoha_qdma_clear(qdma, QDMA_CSR_TX_BLOCKING(idx), TXQ_DYN_CNGSTCTL_MIN_THRH_TXRING_EN)

#define qdmaEnableCngstMinThrhEn(qdma, idx) \
    airoha_qdma_set(qdma, QDMA_CSR_TX_BLOCKING(idx), TXQ_DYN_CNGSTCTL_MIN_THRH_TXRING_EN)

#define qdmaEnableCngstModeConfigTrig(qdma) \
    airoha_qdma_set(qdma, QDMA_CSR_TXQ_CNGST_CFG, TXQ_DYN_CNGSTCTL_MODE_CONFIG_TRIGGER)

#define qdmaEnableCngstModePacketTrig(qdma) \
    airoha_qdma_set(qdma, QDMA_CSR_TXQ_CNGST_CFG, TXQ_DYN_CNGSTCTL_MODE_PACKET_TRIGGER)

#define qdmaEnableCngstModeTimeTrig(qdma) \
    airoha_qdma_set(qdma, QDMA_CSR_TXQ_CNGST_CFG, TXQ_DYN_CNGSTCTL_MODE_TIME_TRIGGER)

#define qdmaEnableTxRateMeter(qdma) \
    airoha_qdma_set(qdma, QDMA_CSR_EGRESS_RATEMETER_CFG, EGRESS_RATEMETER_EN)

#define qdmaEnableTxRateMeterPeakRate(qdma) \
    airoha_qdma_set(qdma, QDMA_CSR_EGRESS_RATEMETER_CFG, EGRESS_RATEMETER_PEEKRATE_EN)

/*
#define qdmaSetTxPeekRateDuration(base, val)		IO_SMASK(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_PEEKRATE_DURATION_MASK, EGRESS_RATEMETER_PEEKRATE_DURATION_SHIFT, val)

#define qdmaSetTxqDEITotalMaxThrh(base, val)	IO_SMASK(QDMA_CSR_TXQ_DEI_TOTALTHR(base), TXQ_CNGST_DEI_TOTAL_MAX_THR_MASK, TXQ_CNGST_DEI_TOTAL_MAX_THR_SHIFT, val)

#define qdmaSetTxqDEITotalMinThrh(base, val)	IO_SMASK(QDMA_CSR_TXQ_DEI_TOTALTHR(base), TXQ_CNGST_DEI_TOTAL_MIN_THR_MASK, TXQ_CNGST_DEI_TOTAL_MIN_THR_SHIFT, val)

#define qdmaSetTxqDEIChnlMaxThrh(base, val)		IO_SMASK(QDMA_CSR_TXQ_DEI_CHNLTHR_CFG(base), TXQ_CNGST_DEI_CHNL_MAX_THR_MASK, TXQ_CNGST_DEI_CHNL_MAX_THR_SHIFT, val)

#define qdmaSetTxqDEIChnlMinThrh(base, val)		IO_SMASK(QDMA_CSR_TXQ_DEI_CHNLTHR_CFG(base), TXQ_CNGST_DEI_CHNL_MIN_THR_MASK, TXQ_CNGST_DEI_CHNL_MIN_THR_SHIFT, val)

#define qdmaSetTxqDEIQueueMaxThrh(base, val)	IO_SMASK(QDMA_CSR_TXQ_DEI_QUEUETHR_CFG(base), TXQ_CNGST_DEI_QUEUE_MAX_THR_MASK, TXQ_CNGST_DEI_QUEUE_MAX_THR_SHIFT, val)

#define qdmaSetTxqDEIQueueMinThrh(base, val)	IO_SMASK(QDMA_CSR_TXQ_DEI_QUEUETHR_CFG(base), TXQ_CNGST_DEI_QUEUE_MIN_THR_MASK, TXQ_CNGST_DEI_QUEUE_MIN_THR_SHIFT, val)

#define qdmaSetTxqDynTotalMaxThrh(base, val)	IO_SMASK(QDMA_CSR_TXQ_DYN_TOTALTHR(base), TXQ_CNGST_TOTAL_MAX_THR_MASK, TXQ_CNGST_TOTAL_MAX_THR_SHIFT, val)

#define qdmaSetTxqDynTotalMinThrh(base, val)	IO_SMASK(QDMA_CSR_TXQ_DYN_TOTALTHR(base), TXQ_CNGST_TOTAL_MIN_THR_MASK, TXQ_CNGST_TOTAL_MIN_THR_SHIFT, val)

#define qdmaSetTxqDynChnlMaxThrh(base, val)		IO_SMASK(QDMA_CSR_TXQ_DYN_CHNLTHR_CFG(base), TXQ_CNGST_CHNL_MAX_THR_MASK, TXQ_CNGST_CHNL_MAX_THR_SHIFT, val)

#define qdmaSetTxqDynChnlMinThrh(base, val)		IO_SMASK(QDMA_CSR_TXQ_DYN_CHNLTHR_CFG(base), TXQ_CNGST_CHNL_MIN_THR_MASK, TXQ_CNGST_CHNL_MIN_THR_SHIFT, val)

#define qdmaSetTxqDynQueueMaxThrh(base, val)	IO_SMASK(QDMA_CSR_TXQ_DYN_QUEUETHR_CFG(base), TXQ_CNGST_QUEUE_MAX_THR_MASK, TXQ_CNGST_QUEUE_MAX_THR_SHIFT, val)

#define qdmaSetTxqDynQueueMinThrh(base, val)	IO_SMASK(QDMA_CSR_TXQ_DYN_QUEUETHR_CFG(base), TXQ_CNGST_QUEUE_MIN_THR_MASK, TXQ_CNGST_QUEUE_MIN_THR_SHIFT, val)

#define qdmaSetTxqMinDscpThrshld(base, val)	IO_SMASK(QDMA_CSR_TXQ_MIN_DSCP_THRH(base), TXQ_MIN_DSCP_THRSHLD_MASK, TXQ_MIN_DSCP_THRSHLD_SHIFT, val)

#define qdmaGetTxqDynTotalMaxThrh(base)			IO_GMASK(QDMA_CSR_TXQ_DYN_TOTALTHR(base), TXQ_CNGST_TOTAL_MAX_THR_MASK, TXQ_CNGST_TOTAL_MAX_THR_SHIFT)

#define qdmaGetTxqDynTotalMinThrh(base)			IO_GMASK(QDMA_CSR_TXQ_DYN_TOTALTHR(base), TXQ_CNGST_TOTAL_MIN_THR_MASK, TXQ_CNGST_TOTAL_MIN_THR_SHIFT)

#define qdmaGetTxqDynChnlMaxThrh(base)			IO_GMASK(QDMA_CSR_TXQ_DYN_CHNLTHR_CFG(base), TXQ_CNGST_CHNL_MAX_THR_MASK, TXQ_CNGST_CHNL_MAX_THR_SHIFT)

#define qdmaGetTxqDynChnlMinThrh(base)			IO_GMASK(QDMA_CSR_TXQ_DYN_CHNLTHR_CFG(base), TXQ_CNGST_CHNL_MIN_THR_MASK, TXQ_CNGST_CHNL_MIN_THR_SHIFT)

#define qdmaGetTxqDynQueueMaxThrh(base)			IO_GMASK(QDMA_CSR_TXQ_DYN_QUEUETHR_CFG(base), TXQ_CNGST_QUEUE_MAX_THR_MASK, TXQ_CNGST_QUEUE_MAX_THR_SHIFT)

#define qdmaGetTxqDynQueueMinThrh(base)			IO_GMASK(QDMA_CSR_TXQ_DYN_QUEUETHR_CFG(base), TXQ_CNGST_QUEUE_MIN_THR_MASK, TXQ_CNGST_QUEUE_MIN_THR_SHIFT)

#define qdmaGetTxqMinDscpThrshld(base)		IO_GMASK(QDMA_CSR_TXQ_MIN_DSCP_THRH(base), TXQ_MIN_DSCP_THRSHLD_MASK, TXQ_MIN_DSCP_THRSHLD_SHIFT)
*/

#define qdmaSetTxPeekRateDuration(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_EGRESS_RATEMETER_CFG, EGRESS_RATEMETER_PEEKRATE_DURATION_MASK, (val << EGRESS_RATEMETER_PEEKRATE_DURATION_SHIFT))

#define qdmaSetTxqDEITotalMaxThrh(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_TXQ_DEI_TOTALTHR, TXQ_CNGST_DEI_TOTAL_MAX_THR_MASK, (val << TXQ_CNGST_DEI_TOTAL_MAX_THR_SHIFT))

#define qdmaSetTxqDEITotalMinThrh(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_TXQ_DEI_TOTALTHR, TXQ_CNGST_DEI_TOTAL_MIN_THR_MASK, (val << TXQ_CNGST_DEI_TOTAL_MIN_THR_SHIFT))

#define qdmaSetTxqDEIChnlMaxThrh(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_TXQ_DEI_CHNLTHR_CFG, TXQ_CNGST_DEI_CHNL_MAX_THR_MASK, (val << TXQ_CNGST_DEI_CHNL_MAX_THR_SHIFT))

#define qdmaSetTxqDEIChnlMinThrh(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_TXQ_DEI_CHNLTHR_CFG, TXQ_CNGST_DEI_CHNL_MIN_THR_MASK, (val << TXQ_CNGST_DEI_CHNL_MIN_THR_SHIFT))

#define qdmaSetTxqDEIQueueMaxThrh(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_TXQ_DEI_QUEUETHR_CFG, TXQ_CNGST_DEI_QUEUE_MAX_THR_MASK, (val << TXQ_CNGST_DEI_QUEUE_MAX_THR_SHIFT))

#define qdmaSetTxqDEIQueueMinThrh(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_TXQ_DEI_QUEUETHR_CFG, TXQ_CNGST_DEI_QUEUE_MIN_THR_MASK, (val << TXQ_CNGST_DEI_QUEUE_MIN_THR_SHIFT))

#define qdmaSetTxqDynTotalMaxThrh(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_TXQ_DYN_TOTALTHR, TXQ_CNGST_TOTAL_MAX_THR_MASK, (val << TXQ_CNGST_TOTAL_MAX_THR_SHIFT))

#define qdmaSetTxqDynTotalMinThrh(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_TXQ_DYN_TOTALTHR, TXQ_CNGST_TOTAL_MIN_THR_MASK, (val << TXQ_CNGST_TOTAL_MIN_THR_SHIFT))

#define qdmaSetTxqDynChnlMaxThrh(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_TXQ_DYN_CHNLTHR_CFG, TXQ_CNGST_CHNL_MAX_THR_MASK, (val << TXQ_CNGST_CHNL_MAX_THR_SHIFT))

#define qdmaSetTxqDynChnlMinThrh(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_TXQ_DYN_CHNLTHR_CFG, TXQ_CNGST_CHNL_MIN_THR_MASK, (val << TXQ_CNGST_CHNL_MIN_THR_SHIFT))

#define qdmaSetTxqDynQueueMaxThrh(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_TXQ_DYN_QUEUETHR_CFG, TXQ_CNGST_QUEUE_MAX_THR_MASK, (val << TXQ_CNGST_QUEUE_MAX_THR_SHIFT))

#define qdmaSetTxqDynQueueMinThrh(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_TXQ_DYN_QUEUETHR_CFG, TXQ_CNGST_QUEUE_MIN_THR_MASK, (val << TXQ_CNGST_QUEUE_MIN_THR_SHIFT))

#define qdmaSetTxqMinDscpThrshld(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_TXQ_MIN_DSCP_THRH, TXQ_MIN_DSCP_THRSHLD_MASK, (val << TXQ_MIN_DSCP_THRSHLD_SHIFT))

#define qdmaGetTxqDynTotalMaxThrh(qdma) \
    (airoha_qdma_rr(qdma, QDMA_CSR_TXQ_DYN_TOTALTHR & TXQ_CNGST_TOTAL_MAX_THR_MASK) >> TXQ_CNGST_TOTAL_MAX_THR_SHIFT)

#define qdmaGetTxqDynTotalMinThrh(qdma) \
    (airoha_qdma_rr(qdma, QDMA_CSR_TXQ_DYN_TOTALTHR & TXQ_CNGST_TOTAL_MIN_THR_MASK) >> TXQ_CNGST_TOTAL_MIN_THR_SHIFT)

#define qdmaGetTxqDynChnlMaxThrh(qdma) \
    (airoha_qdma_rr(qdma, QDMA_CSR_TXQ_DYN_CHNLTHR_CFG & TXQ_CNGST_CHNL_MAX_THR_MASK) >> TXQ_CNGST_CHNL_MAX_THR_SHIFT)

#define qdmaGetTxqDynChnlMinThrh(qdma) \
    (airoha_qdma_rr(qdma, QDMA_CSR_TXQ_DYN_CHNLTHR_CFG & TXQ_CNGST_CHNL_MIN_THR_MASK) >> TXQ_CNGST_CHNL_MIN_THR_SHIFT)

#define qdmaGetTxqDynQueueMaxThrh(qdma) \
    (airoha_qdma_rr(qdma, QDMA_CSR_TXQ_DYN_QUEUETHR_CFG & TXQ_CNGST_QUEUE_MAX_THR_MASK) >> TXQ_CNGST_QUEUE_MAX_THR_SHIFT)

#define qdmaGetTxqDynQueueMinThrh(qdma) \
    (airoha_qdma_rr(qdma, QDMA_CSR_TXQ_DYN_QUEUETHR_CFG & TXQ_CNGST_QUEUE_MIN_THR_MASK) >> TXQ_CNGST_QUEUE_MIN_THR_SHIFT)

#define qdmaGetTxqMinDscpThrshld(qdma) \
    (airoha_qdma_rr(qdma, QDMA_CSR_TXQ_MIN_DSCP_THRH & TXQ_MIN_DSCP_THRSHLD_MASK) >> TXQ_MIN_DSCP_THRSHLD_SHIFT)


#define TX_WRED_THR_NUM                         (5)

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_TXQ_WRED 1
#else
#define SUPPORT_TXQ_WRED 0
#endif

#define WRED_THR_SHIFT_SHIFT(thrIdx)                (thrIdx*5+3)
#define WRED_THR_CFG_SHIFT(thrIdx)                  (thrIdx*5)

#define QDMA_CSR_CNGST_WRED_NORM_CFG          (0x1090)
#define QDMA_CSR_CNGST_WRED_DEI_CFG           (0x1094)

#define qdmaSetWredNormThreshold(qdma, val)                 airoha_qdma_wr(qdma, QDMA_CSR_CNGST_WRED_NORM_CFG, val)

#define qdmaSetWredDeiThreshold(qdma, val)                  airoha_qdma_wr(qdma, QDMA_CSR_CNGST_WRED_DEI_CFG, val)


#define DBG_CNTR_SRC_FWD_TX							(1)
#define DBG_CNTR_SRC_CPU_TX							(0)
#define DBG_CNTR_ENABLE								(1)
#if defined(TCSUPPORT_CPU_EN7517)
#define DBG_CNTR_FWD_CHNL_GROUP_NUM 		(12)
#else
#define DBG_CNTR_FWD_CHNL_GROUP_NUM 		(16)
#endif

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7583)
    #define CONFIG_MAX_CNTR_NUM 				(32)	/*DBG CNT GROUP*/
#elif defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7528)
    #define CONFIG_MAX_CNTR_NUM                 (64)    /*DBG CNT GROUP*/
#else
    #define CONFIG_MAX_CNTR_NUM                 (40)	
#endif

#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_EN7580)
#define QDMA_CSR_DBG_CNTR_CFG(i)				(0x0400+(i<<3))
#else
#define QDMA_CSR_DBG_CNTR_CFG(i)				(0x0300+(i<<3))
#endif

#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_EN7580)
#define DBG_CNTR_SRC_SHIFT							(24)
#define DBG_CNTR_SRC_MASK							(0xF<<DBG_CNTR_SRC_SHIFT)
#else
#define DBG_CNTR_SRC_SHIFT							(28)
#define DBG_CNTR_SRC_MASK							(0x7<<DBG_CNTR_SRC_SHIFT)
#endif

#define DBG_CNTR_CHNL_SHIFT							(3)
#define DBG_CNTR_CHNL_MASK							(0x1F<<DBG_CNTR_CHNL_SHIFT)

#define DBG_CNTR_QUEUE_SHIFT						(0)
#define DBG_CNTR_QUEUE_MASK							(0x7<<DBG_CNTR_QUEUE_SHIFT)

#define DBG_CNTR_DSCPRING_IDX_SHIFT					(16)
#define DBG_CNTR_DSCPRING_IDX_MASK					(0xF<<DBG_CNTR_DSCPRING_IDX_SHIFT)

#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_EN7580)
#define DBG_CNTR_ALL_CHNL_SHIFT						(30)
#define DBG_CNTR_ALL_CHNL							(1<<DBG_CNTR_ALL_CHNL_SHIFT)
#else
#define DBG_CNTR_ALL_CHNL							(1<<27)
#endif

#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_EN7580)
#define DBG_CNTR_ALL_DSCPRING_SHIFT					(28)
#define DBG_CNTR_ALL_DSCPRING						(1<<DBG_CNTR_ALL_DSCPRING_SHIFT)
#else
#define DBG_CNTR_ALL_DSCPRING						(1<<25)
#endif

#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_EN7580)
#define QDMA_CSR_DBG_CNTR_VAR(i)				(0x0404+(i<<3))
#else
#define QDMA_CSR_DBG_CNTR_VAR(i)				(0x0304+(i<<3))
#endif

#define DBG_CNTR_EN									(1<<31)

#define qdmaSetCntrSrc(qdma, idx, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_DBG_CNTR_CFG(idx), DBG_CNTR_SRC_MASK, (val << DBG_CNTR_SRC_SHIFT))

#define qdmaSetCntrDscpIdx(qdma, idx, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_DBG_CNTR_CFG(idx), DBG_CNTR_DSCPRING_IDX_MASK, (val << DBG_CNTR_DSCPRING_IDX_SHIFT))

#define qdmaSetCntrChnlIdx(qdma, idx, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_DBG_CNTR_CFG(idx), DBG_CNTR_CHNL_MASK, (val << DBG_CNTR_CHNL_SHIFT))

#define qdmaSetCntrQueueIdx(qdma, idx, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_DBG_CNTR_CFG(idx), DBG_CNTR_QUEUE_MASK, (val << DBG_CNTR_QUEUE_SHIFT))


#define qdmaSetCntrChnlSingle(qdma, idx)	airoha_qdma_clear(qdma, QDMA_CSR_DBG_CNTR_CFG(idx), DBG_CNTR_ALL_CHNL)

#define qdmaSetCntrChnlAll(qdma, idx)		airoha_qdma_set(qdma, QDMA_CSR_DBG_CNTR_CFG(idx), DBG_CNTR_ALL_CHNL)

#define qdmaSetCntrDscpSingle(qdma, idx)	airoha_qdma_clear(qdma, QDMA_CSR_DBG_CNTR_CFG(idx), DBG_CNTR_ALL_DSCPRING)

#define qdmaSetCntrDscpAll(qdma, idx)		airoha_qdma_set(qdma, QDMA_CSR_DBG_CNTR_CFG(idx), DBG_CNTR_ALL_DSCPRING)

#define qdmaClearCntrCounter(qdma, idx)		airoha_qdma_wr(qdma, QDMA_CSR_DBG_CNTR_VAR(idx), 0)

#define qdmaDisableCntrCfg(qdma, idx)		airoha_qdma_clear(qdma, QDMA_CSR_DBG_CNTR_CFG(idx), DBG_CNTR_EN)

#define qdmaEnableCntrCfg(qdma, idx)		airoha_qdma_set(qdma, QDMA_CSR_DBG_CNTR_CFG(idx), DBG_CNTR_EN)



#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_EN7580)
#define DBG_CNTR_ALL_QUEUE_SHIFT					(29)
#define DBG_CNTR_ALL_QUEUE							(1<<DBG_CNTR_ALL_QUEUE_SHIFT)
#else
#define DBG_CNTR_ALL_QUEUE							(1<<26)
#endif

#define qdmaSetCntrQueueAll(qdma, idx)		airoha_qdma_set(qdma, QDMA_CSR_DBG_CNTR_CFG(idx), DBG_CNTR_ALL_QUEUE)
#define qdmaSetCntrQueueSingle(qdma, idx)	airoha_qdma_clear(qdma, QDMA_CSR_DBG_CNTR_CFG(idx), DBG_CNTR_ALL_QUEUE)



#define GET_METER_IDX(index)    (index&0xff)
#define GET_METER_GRP(index)    ((index>>8)&0x3)

#define TRTCM_MODE_SHIFT					(30)



#define QDMA_CSR_INGRESS_TRTCM_CFG			(0x0070)
#define QDMA_CSR_SLA_TRTCM_CFG				(0x1150)
#define QDMA_CSR_EGRESS_TRTCM_CFG				(0x1010)
#define QDMA_CSR_GLB_RATECTL_CFG              (0x1080)

#define qdmaGetGeneralTrtcmMode(qdma,trtcm_offset) \
    ((airoha_qdma_rr(qdma, trtcm_offset) & TRTCM_MODE_MASK) >> TRTCM_MODE_SHIFT)

#define TRTCM_PARA_RWCMD				(1<<31)
#define TRTCM_PARA_RWCMD_DONE			(1<<30)
#define TRTCM_PARA_TYPE_SHIFT			(28)
#define TRTCM_PARA_TYPE_MASK			(0x3<<TRTCM_PARA_TYPE_SHIFT)
#define TRTCM_PARA_METER_GROUP_SHIFT    (26)
#define TRTCM_PARA_IDX_RATE_TYPE_SHIFT	(16)
#define TRTCM_PARA_IDX_RATE_TYPE_MASK	(1<<TRTCM_PARA_IDX_RATE_TYPE_SHIFT)  //pir&pbs or cir&cbs
#define TRTCM_PARA_IDX_INDEX_SHIFT		(17)
#define TRTCM_PARA_IDX_INDEX_MASK		(0x3F<<TRTCM_PARA_IDX_INDEX_SHIFT)	 //7580

#define TRTCM_CFG(trtcm_offset)				(trtcm_offset)


#define TRTCM_FAST_TICK_SHIFT				(0)
#define TRTCM_FAST_TICK_MASK				(0xFFFF<<TRTCM_FAST_TICK_SHIFT)
//#define qdmaGetGeneralFastTick(trtcm_base)			IO_GMASK(trtcm_base, TRTCM_FAST_TICK_MASK, TRTCM_FAST_TICK_SHIFT)

#define qdmaGetGeneralFastTick(qdma,trtcm_offset) \
    ((airoha_qdma_rr(qdma, trtcm_offset) & TRTCM_FAST_TICK_MASK) >> TRTCM_FAST_TICK_SHIFT)

#define TRTCM_SLOW_TICKRATIO_SHIFT			(16)
#define TRTCM_SLOW_TICKRATIO_MASK			(0x3FFF<<TRTCM_SLOW_TICKRATIO_SHIFT)

//#define qdmaGetGeneralSlowRatio(trtcm_base)			IO_GMASK(trtcm_base, TRTCM_SLOW_TICKRATIO_MASK, TRTCM_SLOW_TICKRATIO_SHIFT)

#define qdmaGetGeneralSlowRatio(qdma,trtcm_offset) \
    ((airoha_qdma_rr(qdma, trtcm_offset) & TRTCM_SLOW_TICKRATIO_MASK) >> TRTCM_SLOW_TICKRATIO_SHIFT)
	
#define TRTCM_PKT_MODE				(1<<1)

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_AN7552)
#define SUPPORT_INGRESS_3X_RATELIMIT 1
#else
#define SUPPORT_INGRESS_3X_RATELIMIT 0
#endif
#define METER_1K	(1000)
#define METER_1M	(METER_1K<<10)
#define RATELIMIT_PKT_MODE_BUCKET_SHIFT			(0)


#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
#define SUPPORT_BUCKETSIZE_BITWIDTH_17 1
#else
#define SUPPORT_BUCKETSIZE_BITWIDTH_17 0
#endif

#define GENERAL_SLA_INDEX_MAX					(31)    /*EN7581 || EN7523: SLA TRTCM INDEX, 2bit: SLA index, 3bit: queue index*/
#define QUEUE_ALL_NUM                           (CONFIG_QDMA_CHANNEL*CONFIG_QDMA_QUEUE)

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_MTR_ACNT_IN_QDMA 1
#else
#define SUPPORT_MTR_ACNT_IN_QDMA 0
#endif

#define GENERAL_INGRESS_INDEX_MAX               (127)
#define GENERAL_INGRESS_INDEX_MAX_GRP1               (31)   /*grp1: 32 ratelimit. or 16 trtcm*/
#define GENERAL_INGRESS_INDEX_MAX_GRP2               (15)   /*grp2: 16 ratelimit. or 8 trtcm*/

#define TRTCM_TICK_SEL						(1<<0)
#define TRTCM_TOKEN_RATE_INTEGER_SHIFT		(6)

#if defined(TCSUPPORT_CPU_EN7580)
#define TRTCM_BUCKET_SIZE_SHIFT_MASK		(0xF)
#else
#define TRTCM_BUCKET_SIZE_SHIFT_MASK		(0x1F)
#endif

#define TRTCM_TOKEN_RATE_INTEGER_SHIFT		(6)
#define TRTCM_TOKEN_RATE_INTEGER_MASK		(0x3FFFF<<TRTCM_TOKEN_RATE_INTEGER_SHIFT)

//#define TRTCM_TOKEN_RATE_FRACTION_MASK		(0x3F)

#define TRTCM_METER_MODE					(1<<2)

/*
before 7523 time to meter window size is fixed 4ms,now we can set other time
*/
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
#define SUPPORT_METER_WINDOW_SIZE_TIME_SLIDE 1
#else
#define SUPPORT_METER_WINDOW_SIZE_TIME_SLIDE 0
#endif


#define EGRESS_RATEMETER_WINDSZ_SHIFT				(12)
#define EGRESS_RATEMETER_WINDSZ_MASK				(0x1F<<EGRESS_RATEMETER_WINDSZ_SHIFT)

#define EGRESS_RATEMETER_TIMESLICE_SHIFT			(0)
#define EGRESS_RATEMETER_TIMESLICE_MASK				(0x7FF<<EGRESS_RATEMETER_TIMESLICE_SHIFT)

#define EGRESS_RATEMETER_TIMEDIVESEL_SHIFT			(16)
#define EGRESS_RATEMETER_TIMEDIVESEL_MASK			(0x3<<EGRESS_RATEMETER_TIMEDIVESEL_SHIFT)

/*
#define qdmaIsTxRateMeterEnable(base)		(IO_GREG(QDMA_CSR_EGRESS_RATEMETER_CFG(base)) & EGRESS_RATEMETER_EN) 

#define qdmaEnableTxRateMeter(base)			IO_SBITS(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_EN)

#define qdmaDisableTxRateMeter(base)		IO_CBITS(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_EN)

#define qdmaSetTxRateMeterWindowSize(base, val)		IO_SMASK(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_WINDSZ_MASK, EGRESS_RATEMETER_WINDSZ_SHIFT, val)

#define qdmaSetTxRateMeterTimeSlice(base, val)		IO_SMASK(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_TIMESLICE_MASK, EGRESS_RATEMETER_TIMESLICE_SHIFT, val)

#define qdmaSetTxRateMeterTimeDivisor(base, val)	IO_SMASK(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_TIMEDIVESEL_MASK, EGRESS_RATEMETER_TIMEDIVESEL_SHIFT, val)
*/

#define qdmaIsTxRateMeterEnable(qdma) \
    (airoha_qdma_rr(qdma, QDMA_CSR_EGRESS_RATEMETER_CFG & EGRESS_RATEMETER_EN)

#define qdmaEnableTxRateMeter(qdma) \
    airoha_qdma_set(qdma, QDMA_CSR_EGRESS_RATEMETER_CFG, EGRESS_RATEMETER_EN)

#define qdmaDisableTxRateMeter(qdma) \
    airoha_qdma_clear(qdma, QDMA_CSR_EGRESS_RATEMETER_CFG, EGRESS_RATEMETER_EN)

#define qdmaSetTxRateMeterWindowSize(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_EGRESS_RATEMETER_CFG, EGRESS_RATEMETER_WINDSZ_MASK, (val << EGRESS_RATEMETER_WINDSZ_SHIFT))

#define qdmaSetTxRateMeterTimeSlice(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_EGRESS_RATEMETER_CFG, EGRESS_RATEMETER_TIMESLICE_MASK, (val << EGRESS_RATEMETER_TIMESLICE_SHIFT))

#define qdmaSetTxRateMeterTimeDivisor(qdma, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_EGRESS_RATEMETER_CFG, EGRESS_RATEMETER_TIMEDIVESEL_MASK, (val << EGRESS_RATEMETER_TIMEDIVESEL_SHIFT))


#define TX_RATELIMIT_TICKER0_ACCURACY 1 /*kbps -->8000us*/
#define TX_RATELIMIT_TICKER1_ACCURACY 400 /*kbps -->20us*/


/* QDMA_CSR_RATELIMIT_PARAMETER_CFG	 */
#define EGRESS_RATELIMIT_PARA_RWCMD					(1<<31)
#define EGRESS_RATELIMIT_PARA_RWCMD_DONE			(1<<30)
#define EGRESS_RATELIMIT_PARA_TYPE_SHIFT			(28)
#define EGRESS_RATELIMIT_PARA_TYPE_MASK				(0x3<<EGRESS_RATELIMIT_PARA_TYPE_SHIFT)
#define EGRESS_RATELIMIT_PARA_IDX_SHIFT				(16)
#define EGRESS_RATELIMIT_PARA_IDX_MASK				(0x1F<<EGRESS_RATELIMIT_PARA_IDX_SHIFT)
#define EGRESS_RATELIMIT_PARA_VALUE_SHIFT			(0)
#define EGRESS_RATELIMIT_PARA_VALUE_MASK			(0xFFFF<<TXWRR_WRR_VALUE_SHIFT)

#define QDMA_CSR_RATELIMIT_PARAMETER_CFG		(0x009C)

#define QDMA_CSR_RATELIMIT_PER_CHNL_TICKSEL	(0x0600)

#define QDMA_CSR_EGRESS_RATELIMIT_CFG			(0x0098)
#define QDMA_CSR_EGRESS_RATELIMIT_CFG2		(0x0620)
#define EGRESS_RATELIMIT_TICKSEL_SHIFT				(0)
#define EGRESS_RATELIMIT_TICKSEL_MASK				(0xFFFF<<EGRESS_RATELIMIT_TICKSEL_SHIFT)

/* QDMA_CSR_RATELIMIT_EN_CFG */
#define TX_CHNL_RATELIMIT_EN(idx)					(1<<idx)
#define QDMA_CSR_RATELIMIT_EN_CFG				(0x0160)

#define EGRESS_RATELIMIT_BUCKETSCALE_SHIFT			(16)
#define EGRESS_RATELIMIT_BUCKETSCALE_MASK			(0xF<<EGRESS_RATELIMIT_BUCKETSCALE_SHIFT)

/*
#define qdmaIsTxRateLimitChnlTickSel1(base,idx) 	(IO_GREG(QDMA_CSR_RATELIMIT_PER_CHNL_TICKSEL(base)) & (1<<(idx)))

#define qdmaSetTxRateLimitPerChnlTickSelIdx(base,idx,ticksel_idx)		IO_SMASK(QDMA_CSR_RATELIMIT_PER_CHNL_TICKSEL(base), 0x1<<(idx), idx, ticksel_idx)



#define qdmaGetTxRateLimitTicksel(base,ticksel_idx)	IO_GMASK((((ticksel_idx)==1) ? QDMA_CSR_EGRESS_RATELIMIT_CFG(base) : QDMA_CSR_EGRESS_RATELIMIT_CFG2(base)),EGRESS_RATELIMIT_TICKSEL_MASK, EGRESS_RATELIMIT_TICKSEL_SHIFT)


#define qdmaSetTxRateLimitTicksel(base, ticksel_idx, val)		IO_SMASK((((ticksel_idx)==1) ? QDMA_CSR_EGRESS_RATELIMIT_CFG(base) : QDMA_CSR_EGRESS_RATELIMIT_CFG2(base)),EGRESS_RATELIMIT_TICKSEL_MASK, EGRESS_RATELIMIT_TICKSEL_SHIFT, val)

#define qdmaIsChnlRateLimitEnable(base, idx)	(IO_GREG(QDMA_CSR_RATELIMIT_EN_CFG(base)) & TX_CHNL_RATELIMIT_EN(idx))

#define qdmaGetTxRateLimitBucketScale(base)			IO_GMASK(QDMA_CSR_EGRESS_RATELIMIT_CFG(base), EGRESS_RATELIMIT_BUCKETSCALE_MASK, EGRESS_RATELIMIT_BUCKETSCALE_SHIFT)
*/

#define qdmaIsTxRateLimitChnlTickSel1(qdma, idx) \
    (airoha_qdma_rr(qdma, QDMA_CSR_RATELIMIT_PER_CHNL_TICKSEL & (1 << (idx))))

#define qdmaSetTxRateLimitPerChnlTickSelIdx(qdma, idx, ticksel_idx) \
    airoha_qdma_rmw(qdma, QDMA_CSR_RATELIMIT_PER_CHNL_TICKSEL, 0x1 << (idx), (ticksel_idx << (idx)))

#define qdmaGetTxRateLimitTicksel(qdma, ticksel_idx) \
    (airoha_qdma_rr(qdma, (((ticksel_idx) == 1) ? QDMA_CSR_EGRESS_RATELIMIT_CFG : QDMA_CSR_EGRESS_RATELIMIT_CFG2) & EGRESS_RATELIMIT_TICKSEL_MASK) >> EGRESS_RATELIMIT_TICKSEL_SHIFT)

#define qdmaSetTxRateLimitTicksel(qdma, ticksel_idx, val) \
    airoha_qdma_rmw(qdma, (((ticksel_idx) == 1) ? QDMA_CSR_EGRESS_RATELIMIT_CFG : QDMA_CSR_EGRESS_RATELIMIT_CFG2), EGRESS_RATELIMIT_TICKSEL_MASK, (val << EGRESS_RATELIMIT_TICKSEL_SHIFT))

#define qdmaIsChnlRateLimitEnable(qdma, idx) \
    (airoha_qdma_rr(qdma, QDMA_CSR_RATELIMIT_EN_CFG & TX_CHNL_RATELIMIT_EN(idx)))
	
#define qdmaGetTxRateLimitBucketScale(qdma) \
    (airoha_qdma_rr(qdma, QDMA_CSR_EGRESS_RATELIMIT_CFG & EGRESS_RATELIMIT_BUCKETSCALE_MASK) >> EGRESS_RATELIMIT_BUCKETSCALE_SHIFT)
	

#ifndef MAX
#define MAX(a, b) ((a>b)?(a):(b))
#endif

#define SWITCH_BUFFER_THRESHOLD 40 /* unit is 40kByte */

/*
#define qdmaDisableChnlRateLimit(base, idx)		IO_CBITS(QDMA_CSR_RATELIMIT_EN_CFG(base), TX_CHNL_RATELIMIT_EN(idx))

#define qdmaEnableChnlRateLimit(base, idx)		IO_SBITS(QDMA_CSR_RATELIMIT_EN_CFG(base), TX_CHNL_RATELIMIT_EN(idx))
*/

#define qdmaDisableChnlRateLimit(qdma, idx) \
    airoha_qdma_clear(qdma, QDMA_CSR_RATELIMIT_EN_CFG, TX_CHNL_RATELIMIT_EN(idx))

#define qdmaEnableChnlRateLimit(qdma, idx) \
    airoha_qdma_set(qdma, QDMA_CSR_RATELIMIT_EN_CFG, TX_CHNL_RATELIMIT_EN(idx))
	
#define RX_RATELIMIT_EN								(1<<30)

#define QDMA_CSR_RX_PROTECT_CFG				(0x0120)

/*
#define qdmaDisableRxRateLimit(base)		IO_CBITS(QDMA_CSR_RX_PROTECT_CFG(base), RX_RATELIMIT_EN)

#define qdmaEnableRxRateLimit(base)			IO_SBITS(QDMA_CSR_RX_PROTECT_CFG(base), RX_RATELIMIT_EN)
*/

#define qdmaDisableRxRateLimit(qdma) \
    airoha_qdma_clear(qdma, QDMA_CSR_RX_PROTECT_CFG, RX_RATELIMIT_EN)

#define qdmaEnableRxRateLimit(qdma) \
    airoha_qdma_set(qdma, QDMA_CSR_RX_PROTECT_CFG, RX_RATELIMIT_EN)


#define RX_RATELIMIT_PKTMODE						(1<<29)


#define qdmaSetRxRateLimitByPacket(qdma) \
    airoha_qdma_set(qdma, QDMA_CSR_RX_PROTECT_CFG, RX_RATELIMIT_PKTMODE)
	
#define qdmaSetRxRateLimitByByte(qdma) \
    airoha_qdma_clear(qdma, QDMA_CSR_RX_PROTECT_CFG, RX_RATELIMIT_PKTMODE)
	
#define RX_RATELIMIT_TICKSEL_SHIFT					(0)
#define RX_RATELIMIT_TICKSEL_MASK					(0xFFFF<<RX_RATELIMIT_TICKSEL_SHIFT)

#define RX_RATELIMIT_BUCKET_SCALE_SHIFT				(24)
#define RX_RATELIMIT_BUCKET_SCALE_MASK				(0xF<<RX_RATELIMIT_BUCKET_SCALE_SHIFT)

#define QDMA_CSR_RX_TICK_CFG					(0x0128)

/*
#define qdmaIsRxRateLimitByPacket(base)			(IO_GREG(QDMA_CSR_RX_PROTECT_CFG(base)) & RX_RATELIMIT_PKTMODE)

#define qdmaGetRxRateLimitTicksel(base, tickSel)			IO_GMASK(((tickSel==1)? (QDMA_CSR_RX_PROTECT_CFG(base)):(QDMA_CSR_RX_TICK_CFG(base))), RX_RATELIMIT_TICKSEL_MASK, RX_RATELIMIT_TICKSEL_SHIFT)

#define qdmaGetRxRateLimitBucketScale(base)			IO_GMASK(QDMA_CSR_RX_PROTECT_CFG(base), RX_RATELIMIT_BUCKET_SCALE_MASK, RX_RATELIMIT_BUCKET_SCALE_SHIFT)
*/
#define qdmaIsRxRateLimitByPacket(qdma) \
    (airoha_qdma_rr(qdma, QDMA_CSR_RX_PROTECT_CFG & RX_RATELIMIT_PKTMODE))

#define qdmaGetRxRateLimitTicksel(qdma, tickSel) \
    (airoha_qdma_rr(qdma, ((tickSel == 1) ? QDMA_CSR_RX_PROTECT_CFG : QDMA_CSR_RX_TICK_CFG) & RX_RATELIMIT_TICKSEL_MASK) >> RX_RATELIMIT_TICKSEL_SHIFT)

#define qdmaGetRxRateLimitBucketScale(qdma) \
    (airoha_qdma_rr(qdma, QDMA_CSR_RX_PROTECT_CFG & RX_RATELIMIT_BUCKET_SCALE_MASK) >> RX_RATELIMIT_BUCKET_SCALE_SHIFT)
	


#define qdmaIsRxRateLimitEnable(qdma) \
    (airoha_qdma_rr(qdma, QDMA_CSR_RX_PROTECT_CFG & RX_RATELIMIT_EN))

/*QDMA_CSR_RX_TICK_CFG*/
#define RX_RATELIMIT_SELTICKER_SHIFT(ringIdx)		(16+ringIdx)
#define RX_RATELIMIT_SELTICKER_MASK(ringIdx)		(1<<RX_RATELIMIT_SELTICKER_SHIFT(ringIdx))

#define QDMA_CSR_RX_TICK_CFG					(0x0128)

/*

#define qdmaGetRxRatelimitSelTick(base, rxIdx)				IO_GMASK(QDMA_CSR_RX_TICK_CFG(base), RX_RATELIMIT_SELTICKER_MASK(rxIdx), RX_RATELIMIT_SELTICKER_SHIFT(rxIdx))
#define qdmaSetRxRateLimitTickVal(base, tickSel, tickVal)	IO_SMASK(((tickSel==1)? (QDMA_CSR_RX_PROTECT_CFG(base)):(QDMA_CSR_RX_TICK_CFG(base))), RX_RATELIMIT_TICKSEL_MASK, RX_RATELIMIT_TICKSEL_SHIFT, tickVal)
*/

#define qdmaGetRxRatelimitSelTick(qdma, rxIdx) \
    (airoha_qdma_rr(qdma, QDMA_CSR_RX_TICK_CFG & RX_RATELIMIT_SELTICKER_MASK(rxIdx)) >> RX_RATELIMIT_SELTICKER_SHIFT(rxIdx))

#define qdmaSetRxRateLimitTickVal(qdma, tickSel, tickVal) \
    airoha_qdma_rmw(qdma, ((tickSel == 1) ? QDMA_CSR_RX_PROTECT_CFG : QDMA_CSR_RX_TICK_CFG), RX_RATELIMIT_TICKSEL_MASK, (tickVal << RX_RATELIMIT_TICKSEL_SHIFT))

#define RX_RATELIMIT_RING_IDX(idx)					((idx&0xF)<<16)
#define RX_RATELIMIT_TYPE_SHIFT						(28) 

#define QDMA_CSR_RX_RATELIMIT_CFG				(0x0124)
#define RX_RATELIMIT_RWCMD_DONE						(1<<30)

#define RX_RATELIMIT_VALUE_SHIFT					(0)
#define RX_RATELIMIT_VALUE_MASK						(0xFFFF<<RX_RATELIMIT_VALUE_SHIFT)

#define RX_RATELIMIT_RWCMD							(1<<31)

DEFINE_SPINLOCK(qdma_config_lock);

//#if SUPPORT_QUEUE_DISABLE
/* For EN7580,EN7528: record TXQ_DIS_CFG_CHN register value , the register only can write, can not read */
uint TXQ_DIS_CFG_VALUE[TXQ_DIS_CFG_REG_NUM] = {0, 0, 0, 0, 0, 0, 0, 0};
//#endif
DEFINE_SPINLOCK(qdma_cfg_lock);
uint trtcmBucketByteUnit[TRTCM_MODE_MAX] ;
EXPORT_SYMBOL(trtcmBucketByteUnit);
uint trtcmBucketPacketUnit[TRTCM_MODE_MAX] ;
EXPORT_SYMBOL(trtcmBucketPacketUnit);
uint trtcmCfgBase[TRTCM_MODE_MAX] ;

int packet_is_transparent_mode = 1;
EXPORT_SYMBOL(packet_is_transparent_mode);


#define LRO_RING_NUM    4
#define LRO_RING_START  0xC
#define LRO_RING_END    0XF
enum lro_ring_state{
	LRO_RING_FREE = 0,
	LRO_RING_RESERVED,
};

static u32 lro_ring_reserve[2 * LRO_RING_NUM][2] = {{0}};


/* FE */
#define isEN751221		0
#define PSE_BASE			0x0100
#define CSR_IFC_BASE			0x0200
#define CDM1_BASE			0x0400
#define GDM1_BASE			0x0500
#define PPE1_BASE			0x0c00

#define CDM2_BASE			0x1400
#define GDM2_BASE			0x1500

#define GDM3_BASE			0x1100
#define GDM4_BASE			0x2500

#define GDM_BASE(_n)			\
	((_n) == 4 ? GDM4_BASE :	\
	 (_n) == 3 ? GDM3_BASE :	\
	 (_n) == 2 ? GDM2_BASE : GDM1_BASE)

#define REG_FE_WAN_MAC_H		0x0030
#define REG_FE_MAC_LMIN(_n)		((_n) + 0x04)
#define REG_FE_MAC_LMAX(_n)		((_n) + 0x08)

#define GDMA1_TXCHN_EN      (GDM1_BASE + 0x24)
#define GDMA1_RXCHN_EN      (GDM1_BASE + 0x28)

#define GDMA2_TXCHN_EN       (GDM2_BASE + 0x24)
#define GDMA2_RXCHN_EN       (GDM2_BASE + 0x28)


#define CDMA1_HWF_CHN_EN     (CDM1_BASE + 0x0c)
#define CDMA2_HWF_CHN_EN     (CDM2_BASE + 0x0c)
#define GDMA_CHN_RLS_CHN_OFFSET     4
#define GDMA_CHN_RLS_EN_OFFSET      0 

#define GDMA1_CHN_RLS       (GDM1_BASE + 0x20)
#define GDMA2_CHN_RLS       (GDM2_BASE + 0x20)



#define L2BR_ETYPE_EN			(0x284)

#define L2BR_ETYPE_N(x)			(0x290 + 4*(x/2))

#define GDMA1_MIB_CLER			(GDM1_BASE + 0xf0)	
#define GDMA2_MIB_CLER			(GDM2_BASE + 0xf0)	
#define isEN7526c		0
#define isEN751627		0
#define GDMA2_FWD_CFG       (GDM2_BASE + 0x00)

#define GDMA2_TX_FAVOR_OAM_OFFSET		19


#define GDMA_MISC_CFG           (0x148)
#define GDMA2_RLS_MODE_BIT           (1<<1)

#define GDMA1_LEN_CFG       (GDM1_BASE + 0x14)
#define GDMA2_LEN_CFG    	(GDM2_BASE + 0x14)
#define GDMA3_LEN_CFG        (GDM3_BASE + 0x14)
#define GDMA4_LEN_CFG        (GDM4_BASE + 0x114)

#define GDMA1_COUNT_BASE 	    (0x600)
#define GDMA1_RX_OVER_DROP_CNT  (GDMA1_COUNT_BASE + 0x54)


#define GDMA2_COUNT_BASE 	(0x1600)
#define GDMA2_RX_OVDROPCNT    (GDMA2_COUNT_BASE + 0x54)

#define PSE_IQ_STA1     (0x110)
#define PSE_IQ_STA2     (0x114)

#define GDMA1_TX_CHN_VLD     (GDM1_BASE + 0x70)
#define GDMA2_TX_CHN_VLD     (GDM1_BASE + 0x70)

#define GDMA_CHN_RLS_STAT_OFFSET    1

#define GDMA_CHN_RLS_TIMEOUT        (10)

spinlock_t fe_pse_reset_lock;


/* GDM2 RX eth MIB, support EN7523, EN7581, AN7552 */
#if defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_EN7581)
#define GDM2_RX_ETH_MIB_SUPPORT	1
#else
#define GDM2_RX_ETH_MIB_SUPPORT	0
#endif

/* GDM2 TX eth MIB, support EN7523, EN7581, AN7552 */
#if defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_EN7581)
#define GDM2_TX_ETH_MIB_SUPPORT	1
#else
#define GDM2_TX_ETH_MIB_SUPPORT	0
#endif


#define GDMA2_RX_OKCNT     	(GDMA2_COUNT_BASE + 0x48)
#define GDMA2_RX_FCDROPCNT     	(GDMA2_COUNT_BASE + 0x4c)
#define GDMA2_RX_RCDROPCNT     	(GDMA2_COUNT_BASE + 0x50)

#define GDMA2_RX_OVDROPCNT    (GDMA2_COUNT_BASE + 0x54)
#define GDMA2_RX_ERRDROPCNT    (GDMA2_COUNT_BASE + 0x58)
#define GDMA2_RX_OKBYTECNT    (GDMA2_COUNT_BASE + 0x5c)

#define GDMA2_RX_ETHERPCNT  (GDMA2_COUNT_BASE + 0x60)
#define GDMA2_RX_ETHERPLEN  (GDMA2_COUNT_BASE + 0x64)
#define GDMA2_RX_ETHDROPCNT (GDMA2_COUNT_BASE + 0x68)
#define GDMA2_RX_ETHBCCNT   (GDMA2_COUNT_BASE + 0x6c)
#define GDMA2_RX_ETHMCCNT   (GDMA2_COUNT_BASE + 0x70)
#define GDMA2_RX_ETHCRCCNT  (GDMA2_COUNT_BASE + 0x74)
#define GDMA2_RX_ETHFRACCNT (GDMA2_COUNT_BASE + 0x78)
#define GDMA2_RX_ETHJABCNT  (GDMA2_COUNT_BASE + 0x7c)
#define GDMA2_RX_ETHRUNTCNT (GDMA2_COUNT_BASE + 0x80)
#define GDMA2_RX_ETHLONGCNT (GDMA2_COUNT_BASE + 0x84)
#define GDMA2_RX_ETH_64_CNT (GDMA2_COUNT_BASE + 0x88)
#define GDMA2_RX_ETH_65_TO_127_CNT (GDMA2_COUNT_BASE + 0x8C)
#define GDMA2_RX_ETH_128_TO_255_CNT (GDMA2_COUNT_BASE + 0x90)
#define GDMA2_RX_ETH_256_TO_511_CNT (GDMA2_COUNT_BASE + 0x94)
#define GDMA2_RX_ETH_512_TO_1023_CNT (GDMA2_COUNT_BASE + 0x98)
#define GDMA2_RX_ETH_1024_TO_1518_CNT (GDMA2_COUNT_BASE + 0x9C)


#define GDMA2_RX_OKCNT_H	(GDMA2_COUNT_BASE + 0x190)
#define GDMA2_RX_OKBYTECNT_H	(GDMA2_COUNT_BASE + 0x194)
#define GDMA2_RX_ETHERPCNT_H	(GDMA2_COUNT_BASE + 0x198)
#define GDMA2_RX_ETHERPLEN_H	(GDMA2_COUNT_BASE + 0x19c)

#define GDMA2_RX_ETH_64_CNT_H			(GDMA2_COUNT_BASE + 0x1e8)
#define GDMA2_RX_ETH_65_TO_127_CNT_H	(GDMA2_COUNT_BASE + 0x1ec)
#define GDMA2_RX_ETH_128_TO_255_CNT_H	(GDMA2_COUNT_BASE + 0x1f0)
#define GDMA2_RX_ETH_256_TO_511_CNT_H	(GDMA2_COUNT_BASE + 0x1f4)
#define GDMA2_RX_ETH_512_TO_1023_CNT_H	(GDMA2_COUNT_BASE + 0x1f8)
#define GDMA2_RX_ETH_1024_TO_1518_CNT_H	(GDMA2_COUNT_BASE + 0x1fc)


#define GDMA2_TX_ETHCNT   	(GDMA2_COUNT_BASE + 0x10)
#define GDMA2_TX_ETHLENCNT   	(GDMA2_COUNT_BASE + 0x14)
#define GDMA2_TX_ETHDROPCNT   	(GDMA2_COUNT_BASE + 0x18)
#define GDMA2_TX_ETHBCDCNT   	(GDMA2_COUNT_BASE + 0x1C)
#define GDMA2_TX_ETHMULTICASTCNT   	(GDMA2_COUNT_BASE + 0x20)
#define GDMA2_TX_ETH_LESS64_CNT   	(GDMA2_COUNT_BASE + 0x24)
#define GDMA2_TX_ETH_MORE1518_CNT   	(GDMA2_COUNT_BASE + 0x28)
#define GDMA2_TX_ETH_64_CNT   			(GDMA2_COUNT_BASE + 0x2C)
#define GDMA2_TX_ETH_65_TO_127_CNT   	(GDMA2_COUNT_BASE + 0x30)
#define GDMA2_TX_ETH_128_TO_255_CNT   	(GDMA2_COUNT_BASE + 0x34)
#define GDMA2_TX_ETH_256_TO_511_CNT   	(GDMA2_COUNT_BASE + 0x38)
#define GDMA2_TX_ETH_512_TO_1023_CNT   	(GDMA2_COUNT_BASE + 0x3C)
#define GDMA2_TX_ETH_1024_TO_1518_CNT   	(GDMA2_COUNT_BASE + 0x40)
#define GDMA2_TX_ETHCNT_H	(GDMA2_COUNT_BASE + 0x188)
#define GDMA2_TX_ETHLENCNT_H	(GDMA2_COUNT_BASE + 0x18c)
#define GDMA2_TX_ETH_64_CNT_H		(GDMA2_COUNT_BASE + 0x1b8)
#define GDMA2_TX_ETH_65_TO_127_CNT_H	(GDMA2_COUNT_BASE + 0x1bc)
#define GDMA2_TX_ETH_128_TO_255_CNT_H	(GDMA2_COUNT_BASE + 0x1c0)
#define GDMA2_TX_ETH_256_TO_511_CNT_H	(GDMA2_COUNT_BASE + 0x1c4)
#define GDMA2_TX_ETH_512_TO_1023_CNT_H	(GDMA2_COUNT_BASE + 0x1c8)
#define GDMA2_TX_ETH_1024_TO_1518_CNT_H	(GDMA2_COUNT_BASE + 0x1cc)


unsigned long long gdm2_rx_len_high = 0;
unsigned long long gdm2_rx_drop_high = 0;

unsigned long long gdm2_tx_len_high = 0;
unsigned long long gdm2_tx_drop_high = 0;




#define CHANNEL_RETIRE 1
#define CHANNEL_DROP 0


atomic_t qdma_stop_flag = ATOMIC_INIT(0);

#ifdef TCSUPPORT_CPU_EN7580
int channel_retire = CHANNEL_RETIRE;
#else
int channel_retire = CHANNEL_DROP;
#endif


#define GDMA2_LPBP_CFG      (GDM2_BASE + 0x1c)
#define GDMA2_FWD_CFG       (GDM2_BASE + 0x00)

#define MBI_TX_BUSY_OFFSET          19
#define MBI_TX_TERMINATE_OFFSET     16
#define MBI_RX_BUSY_OFFSET          27
#define MBI_RX_TERMINATE_OFFSET     24

#define QDMA_GLB_CFG     0x5004


#define QDMA1_CHN_EN_BASE     (0x40a0)
#define QDMA2_CHN_EN_BASE     (0x60a0)
#define QDMA1_CHN_VLD_BASE    (0x5280)
#define QDMA2_CHN_VLD_BASE    (0x7280)
#define QDMA_CHN_VLD_OFFSET   (0x1280)

#define GDMA3_CHN_RLS	(GDM3_BASE + 0x20)
#define GDMA3_TXCHN_EN	(GDM3_BASE + 0x24)
#define GDMA3_RXCHN_EN	(GDM3_BASE + 0x28)
#define GDMA3_TX_CHN_VLD (GDM3_BASE + 0x70)
#define GDMA3_RX_CHN_VLD (GDM3_BASE + 0x74)

#if defined(TCSUPPORT_CPU_AN7583)
#define HWF_QDMA_SEL_SUPPORT	1
#else
#define HWF_QDMA_SEL_SUPPORT	0
#endif

#define FE_HWF_QDMA_SEL_OFFSET (0x20f4)
 #define ETH_OFT 5/*for 7583 GDM3*/
 #define PCIE_OFT 6/*for 7583 GDM4 OQ0*/
 #define USB_OFT 7/*for 7583 GDM4 OQ1*/

/*
#define FeGetHwfQdmaSelGdm3() IO_GMASK(FE_HWF_QDMA_SEL,1<<ETH_OFT,ETH_OFT)
#define FeGetHwfQdmaSelGdm4() (IO_GMASK(FE_HWF_QDMA_SEL,1<<PCIE_OFT,PCIE_OFT)||IO_GMASK(FE_HWF_QDMA_SEL,1<<USB_OFT,USB_OFT))
*/

#define FeGetHwfQdmaSelGdm3(eth) ((airoha_fe_rr(eth, FE_HWF_QDMA_SEL_OFFSET) & (1 << ETH_OFT)) >> ETH_OFT)

#define FeGetHwfQdmaSelGdm4(eth) (((airoha_fe_rr(eth, FE_HWF_QDMA_SEL_OFFSET) & (1 << PCIE_OFT)) >> PCIE_OFT) || \
                                  ((airoha_fe_rr(eth, FE_HWF_QDMA_SEL_OFFSET) & (1 << USB_OFT)) >> USB_OFT))

 
#define GDMA4_TXCHN_EN	(GDM3_BASE + 0x124)
#define GDMA4_RXCHN_EN	(GDM3_BASE + 0x128)
#define GDMA4_TX_CHN_VLD (GDM3_BASE + 0x170)
#define GDMA4_RX_CHN_VLD (GDM3_BASE + 0x174)
#define GDMA4_CHN_RLS      (GDM4_BASE + 0x120)

#define PSE_RSV_PAGE_DEFAULT    0x80/*hw default rsv page*/


#if defined(TCSUPPORT_CPU_EN7523)
#define PSE_SHARE_USED_MTHD_OFFSET 16
#define PSE_SHARE_USED_MTHD_MASK (0xffff<<PSE_SHARE_USED_MTHD_OFFSET)
#define PSE_SHARE_USED_THD	(0x94)
#define PSE_FQ_STA			(0x108)
#define PSE_FQFC_CFG_STA(i)	(0x118 + (i<<2))
#define PSE_FQFC_CFG_STA0	(0x118)
#define PSE_FQFC_CFG_STA1	(0x11C)
#define PSE_DROP_CNT(i)		(0x120 + (i<<2))
#define PSE_DROP_CNT_0      (0x120)
#define PSE_DROP_CNT_1      (0x124)
#define PSE_DROP_CNT_2      (0x128)
#define PSE_DROP_CNT_3      (0x12C)
#define PSE_DROP_CNT_4      (0x130)
#define PSE_DROP_CNT_5      (0x134)
#define PSE_DROP_CNT_6      (0x138)
#else
#define PSE_SHARE_USED_THD  (0xD0)
#define PSE_FQ_STA          (0x150)
#define PSE_FQFC_CFG_STA(i) (0x10C + (i<<2))
#define PSE_FQFC_CFG_STA0   (0x10C)
#define PSE_FQFC_CFG_STA1   (0x110)
#define PSE_DROP_CNT(i)     (0x178 + (i<<2))
#define PSE_DROP_CNT_0      (0x178)
#define PSE_DROP_CNT_1      (0x17C)
#define PSE_DROP_CNT_2      (0x180)
#define PSE_DROP_CNT_3      (0x184)
#define PSE_DROP_CNT_4      (0x188)
#define PSE_DROP_CNT_5      (0x18C)
#define PSE_DROP_CNT_6      (0x190)
#endif

#define PSE_QUEUE_CFG_WR		(0x80)
#define PSE_CFG_PORT_ID_SHIFT	(24)
#define PSE_CFG_QUEUE_ID_SHIFT	(16)
#define PSE_QUEUE_CFG_VAL		(0x84)

#if defined(TCSUPPORT_CPU_AN7552)
#define PSE_CFG_OQ_LTHD_SHIFT	(18)
#define PSE_CFG_OQ_EN_SHIFT	    (16)
#define PSE_CFG_OQ_EN_MASK		(0x1<<PSE_CFG_OQ_EN_SHIFT)
#define PSE_CFG_OQ_RSV_SHIFT	(0)
#else
#define PSE_CFG_OQ_LTHD_SHIFT	(30)
#define PSE_CFG_OQ_RSV_SHIFT	(0)
#endif

#define PSE_CFG_OQRSV_SEL_SHIFT	(0)
#define PSE_CFG_WR_EN_SHIFT		(8)
#define PSE_CFG_WR_EN			(0x1<<PSE_CFG_WR_EN_SHIFT)
#define PSE_CFG_OQRSV_SEL		(0x1<<PSE_CFG_OQRSV_SEL_SHIFT)

#define PSE_SHARE_USED_LTHD_OFFSET 16
#define PSE_BUFF_SET	(0x90)

/*
#define GET_PSE_ALL_RSV()    IO_GMASK(PSE_BUFF_SET,0x7fff,0)

#define SET_PSE_ALL_RSV(val)    IO_SMASK(PSE_BUFF_SET,0x7fff,0,val)

#define GET_PSE_FQ_LITMI() IO_GMASK(PSE_FQ_CFG,0x7fff,0)


#define SET_PSE_SHARED_USED_HTHD(val)   IO_SMASK(PSE_SHARE_USED_THD,0xffff,0,val)

#define SET_PSE_SHARED_USED_MTHD(val)   IO_SMASK(PSE_SHARE_USED_THD,PSE_SHARE_USED_MTHD_MASK,PSE_SHARE_USED_MTHD_OFFSET,val)

#define SET_PSE_SHARED_USED_LTHD(val)   IO_SMASK(PSE_BUFF_SET,PSE_SHARE_USED_LTHD_MASK,PSE_SHARE_USED_LTHD_OFFSET,val)
*/

#define GET_PSE_ALL_RSV(eth) \
    ((airoha_fe_rr(eth, PSE_BUFF_SET) & 0x7FFF) >> 0)

#define SET_PSE_ALL_RSV(eth, val) \
    airoha_fe_rmw(eth, PSE_BUFF_SET, 0x7FFF, (val << 0))

#define GET_PSE_FQ_LITMI(eth) \
    ((airoha_fe_rr(eth, PSE_FQ_CFG_OFFSET) & 0x7FFF) >> 0)

#define SET_PSE_SHARED_USED_HTHD(eth, val) \
    airoha_fe_rmw(eth, PSE_SHARE_USED_THD, 0xFFFF, (val << 0))

#define SET_PSE_SHARED_USED_MTHD(eth, val) \
    airoha_fe_rmw(eth, PSE_SHARE_USED_THD, PSE_SHARE_USED_MTHD_MASK, (val << PSE_SHARE_USED_MTHD_OFFSET))

#define SET_PSE_SHARED_USED_LTHD(eth, val) \
    airoha_fe_rmw(eth, PSE_BUFF_SET, PSE_SHARE_USED_LTHD_MASK, (val << PSE_SHARE_USED_LTHD_OFFSET))



/*software hook defines*/
int (*pwan_cb_rx_hook)(void *pMsg, uint msgLen, struct sk_buff *skb, uint pktLen)=NULL;
EXPORT_SYMBOL(pwan_cb_rx_hook);

int (*ra_sw_nat_hook_clean_entry_by_channel)(int channelIdx) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_clean_entry_by_channel);

int (*ra_sw_nat_hook_rx) (struct sk_buff * skb) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_rx);

#ifdef TCSUPPORT_MT7510_FE
int (*ra_sw_nat_hook_tx) (struct sk_buff * skb, struct port_info * pinfo, int magic);
#else
int (*ra_sw_nat_hook_tx) (struct sk_buff * skb, int gmac_no) = NULL;
#endif
EXPORT_SYMBOL(ra_sw_nat_hook_tx);

int is_hwnat_dont_clean = 0;
EXPORT_SYMBOL(is_hwnat_dont_clean);

int (*ra_sw_nat_hook_rxinfo) (struct sk_buff * skb, int magic, char *data, int data_length) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_rxinfo);

int (*ra_sw_nat_hook_magic) (struct sk_buff * skb, int magic) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_magic);

int (*ra_sw_nat_hook_xfer) (struct sk_buff *skb, const struct sk_buff *prev_p) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_xfer);

int (*ra_sw_nat_hook_drop_packet) (struct sk_buff * skb) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_drop_packet);

int (*ra_sw_nat_hook_clean_table) (void) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_clean_table);

int (*ra_sw_nat_hook_tls_vtag_handle_hook)(struct sk_buff** pskb) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_tls_vtag_handle_hook);

int (*wan_speed_test_hook)(struct sk_buff*) = NULL;
EXPORT_SYMBOL(wan_speed_test_hook);

int (*wan_tr471_hook)(struct sk_buff*) = NULL;
EXPORT_SYMBOL(wan_tr471_hook);

int (*ra_sw_nat_hook_set_magic) (struct sk_buff * skb, int magic) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_set_magic);

int (*ra_sw_nat_hook_sendto_ppe)(struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_sendto_ppe);

int (*offload_eth_fast_tx_hook)(struct sk_buff *skb, int channel) = NULL;
EXPORT_SYMBOL(offload_eth_fast_tx_hook);

int (*wan_speed_test_tso_hook)(struct sk_buff*) = NULL;
EXPORT_SYMBOL(wan_speed_test_tso_hook);

int (*wan_speed_test_pinpong_handle_hook)(struct sk_buff*) = NULL;
EXPORT_SYMBOL(wan_speed_test_pinpong_handle_hook);
int (*airoha_tunnel_hook_tx) (struct sk_buff * skb,struct airoha_ppe *ppe,struct port_info *pinfo) =NULL;
EXPORT_SYMBOL(airoha_tunnel_hook_tx);
int (*airoha_tunnel_pingpong_hook)(struct sk_buff * skb, unsigned short sptag, unsigned short udf) = NULL;
EXPORT_SYMBOL(airoha_tunnel_pingpong_hook);
int (*set_tunnel_entry_reason_hook)(u32 foe_entry_idx,u32 reason)=NULL;
EXPORT_SYMBOL(set_tunnel_entry_reason_hook);


int (*ra_sw_nat_hook_ifc_hit_info) (struct sk_buff* skb, unsigned char hit, unsigned int ifc_id) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_ifc_hit_info);

int (*hwnat_multicast_set_info_for_sfu_hook)(int index, int tag) = NULL;
EXPORT_SYMBOL(hwnat_multicast_set_info_for_sfu_hook);

int (*hwnat_delete_foe_entry_hook)(int index) = NULL; 
EXPORT_SYMBOL(hwnat_delete_foe_entry_hook);

int (*hwnat_delete_foe_entry_hook_unlock)(int index) = NULL;
EXPORT_SYMBOL(hwnat_delete_foe_entry_hook_unlock);

int (*hwnat_is_multicast_entry_hook)(int index ,unsigned char* grp_addr,unsigned char* src_addr,int type) = NULL;
EXPORT_SYMBOL(hwnat_is_multicast_entry_hook);

int (*hwnat_is_drop_entry_hook)(int index ,unsigned char* grp_addr,unsigned char* src_addr,int type) = NULL;
EXPORT_SYMBOL(hwnat_is_drop_entry_hook);

int (*hwnat_set_multicast_vlan_hook)(int index, int vid, int vpm) = NULL;
EXPORT_SYMBOL(hwnat_set_multicast_vlan_hook);

int  (*multicast_speed_learn_flow_hook)(struct sk_buff* skb) = NULL;
EXPORT_SYMBOL(multicast_speed_learn_flow_hook);

int  (*hwnat_set_rule_according_to_state_hook)(int index, int state,unsigned long mask) = NULL;
EXPORT_SYMBOL(hwnat_set_rule_according_to_state_hook);

int  (*xpon_igmp_learn_flow_hook)(struct sk_buff* skb) = NULL;
EXPORT_SYMBOL(xpon_igmp_learn_flow_hook);

int  (*multicast_hwnat_drop_entry_hook)(struct sk_buff* skb) = NULL;
EXPORT_SYMBOL(multicast_hwnat_drop_entry_hook);


int (*wan_multicast_drop_hook)(struct sk_buff* skb) = NULL;
EXPORT_SYMBOL(wan_multicast_drop_hook);

int (*wan_multicast_undrop_hook)(void) = NULL;
EXPORT_SYMBOL(wan_multicast_undrop_hook);

int (*wan_multicast_undrop_by_grpip_hook)(unsigned char is_ipv6,unsigned char* grp_ip) = NULL;
EXPORT_SYMBOL(wan_multicast_undrop_by_grpip_hook);

int (*wan_mvlan_change_hook)(void) = NULL;
EXPORT_SYMBOL(wan_mvlan_change_hook);

void (*restore_offload_info_hook)(struct sk_buff *skb, struct port_info *pinfo, int magic) = NULL;
EXPORT_SYMBOL(restore_offload_info_hook);

int (*sw_upstream_nat_tx_hook)(struct sk_buff * skb, uint msg0, uint msg1, struct port_info* qdma_info)= NULL;
EXPORT_SYMBOL(sw_upstream_nat_tx_hook);

int (*sw_downstream_nat_rx_hook) (struct sk_buff * skb) = NULL;
EXPORT_SYMBOL(sw_downstream_nat_rx_hook);

int (*ra_sw_nat_cds_all_ratelimit_hook) (struct sk_buff* skb) = NULL;
EXPORT_SYMBOL(ra_sw_nat_cds_all_ratelimit_hook);

int (*multicast_flood_find_entry_hook)(int index) = NULL;
EXPORT_SYMBOL(multicast_flood_find_entry_hook);

int (*multicast_speed_find_entry_hook)(int index) = NULL;
EXPORT_SYMBOL(multicast_speed_find_entry_hook);

int (*ra_sw_nat_hook_rx_set_l2lu)(struct sk_buff * skb, unsigned int direction, int PpeIndex) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_rx_set_l2lu);

int (*ra_sw_nat_hook_free) (struct sk_buff * skb) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_free);

int (*hwnat_skb_to_foe_hook)(struct sk_buff* skb) = NULL;
EXPORT_SYMBOL(hwnat_skb_to_foe_hook);

int (*arht_soe_ipsec_rcv_packek_from_soe)(struct sk_buff *skb, unsigned int rx_len, unsigned int sa_index, unsigned int hop_flags) = NULL;
EXPORT_SYMBOL(arht_soe_ipsec_rcv_packek_from_soe);

int (*airoha_pon_sfu_point_to_point_transmit_hook)(struct sk_buff *skb, u32 sptag) = NULL;
EXPORT_SYMBOL(airoha_pon_sfu_point_to_point_transmit_hook);

int (*airoha_pon_is_sfu_point_to_point_mode_hook)(struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(airoha_pon_is_sfu_point_to_point_mode_hook);

int (*arht_force_to_cpu_hook)(struct sk_buff*, unsigned short stag) = NULL;
EXPORT_SYMBOL(arht_force_to_cpu_hook);
int (*local_out_pingpong_hook)(struct sk_buff*) = NULL;
EXPORT_SYMBOL(local_out_pingpong_hook);

static int qdma_dev_trtcm_cfg_init(void);
/************************************************************************
*                  QDMA APIs
*************************************************************************
*/

/******************************************************************************
 Descriptor:	It's used to enable/disable the QDMA loopback mode
 Input Args:	arg1: loopback mode (QDMA_LOOPBACK_DISABLE/QDMA_LOOPBACK_QDMA/QDMA_LOOPBACK_UMAC)
 Ret Value: 	No
******************************************************************************/
int qdma_loopback_mode(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	/* in case both whether it is QDMA WAN or QDMA_LAN
	if defined QDMA_LAN
		base=&glb_eth->qdma[0]
	else
		base=&glb_eth->qdma[1]
	*/
	struct airoha_qdma *base = &glb_eth->qdma[1];
	QDMA_LoopbackMode_t lbMode = qdma_data->qdma_private.lbMode ;

	if(lbMode == QDMA_LOOPBACK_DISABLE) {
#if SUPPORT_QDMA_UMAC_LOOPBACK
		qdmaDisableUmacLoopback(base) ;
#endif
		qdmaDisableQdmaLoopback(base) ;
	} else if(lbMode == QDMA_LOOPBACK_QDMA) {
#if SUPPORT_QDMA_UMAC_LOOPBACK
		qdmaDisableUmacLoopback(base) ;
#endif
		qdmaEnableQdmaLoopback(base) ;
	}
#if SUPPORT_QDMA_UMAC_LOOPBACK
	else if(lbMode == QDMA_LOOPBACK_UMAC) {
		qdmaDisableQdmaLoopback(base) ;
		qdmaEnableUmacLoopback(base) ;
	}
#endif
	return 0 ;
}


//#if SUPPORT_QUEUE_DISABLE
int qdmaSetChannelCfg_sw(unchar channel, uint value){
	unchar chnl_offset = (channel>>2);
	uint txq_dis_cfg = 0;

	if(chnl_offset >= TXQ_DIS_CFG_REG_NUM){
		QDMA_ERR("wrong channel index.\n") ;
		return -1;
	}

	txq_dis_cfg = TXQ_DIS_CFG_VALUE[chnl_offset];
	TXQ_DIS_CFG_VALUE[chnl_offset] = ((txq_dis_cfg&~(TXQ_DIS_CHANNEL_MASK(channel)))
									|((value<<TXQ_DIS_CHANNEL_SHIFT(channel))&TXQ_DIS_CHANNEL_MASK(channel)));
	//IO_SREG(QDMA_CSR_QUEUE_CLOSE_CFG(base,channel), TXQ_DIS_CFG_VALUE[chnl_offset]);
	
	airoha_qdma_wr(&glb_eth->qdma[1], QDMA_CSR_QUEUE_CLOSE_CFG(channel), TXQ_DIS_CFG_VALUE[chnl_offset]);

	return 0;
}
//#endif

int qdma_set_channel_close_status(struct ECNT_QDMA_Data *qdma_data)
{
//#if SUPPORT_CHANNEL_CLOSE_STATUS
    uint chnl_idx= 0 ;
	//uint base = gpQdmaPriv->csrBaseAddr ;
    QDMA_ChannelStatus_T *chnlCloseStatusSet = qdma_data->qdma_private.chnlCloseStatusSet ;

    if( chnlCloseStatusSet->chnlIdx < CONFIG_QDMA_CHANNEL ){
        qdmaSetChannelCfg_sw(chnlCloseStatusSet->chnlIdx,chnlCloseStatusSet->chnlStatus);
    }else if(chnlCloseStatusSet->chnlIdx == CONFIG_QDMA_CHANNEL){
        for(chnl_idx=0; chnl_idx<CONFIG_QDMA_CHANNEL; chnl_idx++){
            qdmaSetChannelCfg_sw(chnl_idx,chnlCloseStatusSet->chnlStatus);
        }
    }else{
        QDMA_ERR("Fault: input channel index should between 0 and %d\n", CONFIG_QDMA_CHANNEL ) ; 
        return -EINVAL ;
    }
//#endif
    return 0 ;
}

__IMEM int qdmaDisableInt(struct airoha_qdma *base, uint bit, QDMA_InterruptNo_t intIdx,QDMA_IntEnable_t enableIdx)
{
	//ulong flags=0 ;

#if SUPPORT_INT_ENABLE_NUM_2 /*intNo 1-4, enNo 1-2*/
	if(intIdx<=0 || intIdx>=QDMA_INT_NUM || enableIdx<1 || enableIdx>QDMA_INT_ENABLE_NUM){
		printk("qdmaEnableInt Error: ParaMeter should intIdx 1~%d, enableIdx:1~%d\n", QDMA_INT_NUM, QDMA_INT_ENABLE_NUM);
		return -EINVAL;
	}
#else /*intNo 1-4, enNo 0-2*/
	if(intIdx<=0 || intIdx>=QDMA_INT_NUM || enableIdx<0 || enableIdx>=QDMA_INT_ENABLE_NUM){
		printk("qdmaEnableInt Error: ParaMeter should intIdx 1~%d, enableIdx:0~%d\n", QDMA_INT_NUM, QDMA_INT_ENABLE_NUM);
		return -EINVAL;
	}
#endif

	
	//spin_lock_irqsave(&gpQdmaPriv->irqLock, flags) ;
	
	airoha_qdma_clear(base, QDMA_CSR_INT_EN(intIdx, enableIdx), bit);
	//spin_unlock_irqrestore(&gpQdmaPriv->irqLock, flags) ;
	
	return 0 ;
}

__IMEM int qdma_disable_rxpkt_int(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;	
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516) || defined(TCSUPPORT_CPU_EN7580)
	struct airoha_qdma *base = &glb_eth->qdma[1];
#if defined(TCSUPPORT_CPU_EN7581)
	qdmaDisableInt(base, INT1_MASK_RX_DONE_LOW_16_RING, QDMA_INT1, QDMA_INT_ENABLE2) ;
	qdmaDisableInt(base, INT1_MASK_RX_DONE_HIGH_16_RING, QDMA_INT1, QDMA_INT_ENABLE3) ;
#elif defined(TCSUPPORT_CPU_EN7580)
	qdmaDisableInt(base, INT1_MASK_RX_DONE, QDMA_INT1, QDMA_INT_ENABLE2) ;
#elif defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
	qdmaDisableInt(base, INT1_MASK_RX_DONE_0_1, QDMA_INT1, QDMA_INT_ENABLE0) ;
	qdmaDisableInt(base, INT1_MASK_RX_DONE_2_15, QDMA_INT1, QDMA_INT_ENABLE1) ;
#endif
#endif
	return 0 ;
}

__IMEM int qdmaEnableInt(struct airoha_qdma *base, uint bit, QDMA_InterruptNo_t intIdx,QDMA_IntEnable_t enableIdx)
{
	//ulong flags=0 ;

#if SUPPORT_INT_ENABLE_NUM_2 /*intNo 1-4, enNo 1-2*/
	if(intIdx<=0 || intIdx>=QDMA_INT_NUM || enableIdx<1 || enableIdx>QDMA_INT_ENABLE_NUM){
		printk("qdmaEnableInt Error: ParaMeter should intIdx 1~%d, enableIdx:1~%d\n", QDMA_INT_NUM, QDMA_INT_ENABLE_NUM);
		return -EINVAL;
	}
#else /*intNo 1-4, enNo 0-2*/
	if(intIdx<=0 || intIdx>=QDMA_INT_NUM || enableIdx<0 || enableIdx>=QDMA_INT_ENABLE_NUM){
		printk("qdmaEnableInt Error: ParaMeter should intIdx 1~%d, enableIdx:0~%d\n", QDMA_INT_NUM, QDMA_INT_ENABLE_NUM);
		return -EINVAL;
	}
#endif
	
	//spin_lock_irqsave(&gpQdmaPriv->irqLock, flags) ;
	
	airoha_qdma_set(base, QDMA_CSR_INT_EN(intIdx, enableIdx), bit);
	//spin_unlock_irqrestore(&gpQdmaPriv->irqLock, flags) ;
	
	return 0 ;
}

int qdma_enable_rxpkt_int(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516) || defined(TCSUPPORT_CPU_EN7580)
	struct airoha_qdma *base = &glb_eth->qdma[1];
#if defined(TCSUPPORT_CPU_EN7581)
	qdmaEnableInt(base, INT1_MASK_RX_DONE_LOW_16_RING, QDMA_INT1, QDMA_INT_ENABLE2) ;
	qdmaEnableInt(base, INT1_MASK_RX_DONE_HIGH_16_RING, QDMA_INT1, QDMA_INT_ENABLE3) ;
#elif defined(TCSUPPORT_CPU_EN7580)
	qdmaEnableInt(base, INT1_MASK_RX_DONE , QDMA_INT1, QDMA_INT_ENABLE2) ;
#elif defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
	qdmaEnableInt(base, INT1_MASK_RX_DONE_0_1, QDMA_INT1, QDMA_INT_ENABLE0) ;
	qdmaEnableInt(base, INT1_MASK_RX_DONE_2_15, QDMA_INT1, QDMA_INT_ENABLE1) ;
#endif
#endif
	return 0 ;
}


/******************************************************************************
 Descriptor:	It's used to enable/disable TXDMA mode
 Input Args:	arg1: TX DMA mode (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value: 	No
******************************************************************************/
int qdma_tx_dma_mode(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	QDMA_Mode_t txMode = qdma_data->qdma_private.mode ;

	if(txMode == QDMA_ENABLE) {
		qdmaEnableTxDma(base) ;
	} else {
		qdmaDisableTxDma(base) ;
	}
	return 0 ;
}


/******************************************************************************
 Descriptor:	It's used to enable/disable the RXDMA mode
 Input Args:	arg1: RX DMA mode (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value: 	No
******************************************************************************/
int qdma_rx_dma_mode(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	QDMA_Mode_t rxMode = qdma_data->qdma_private.mode ;
	
	printk("\nbefore base: %px \n",(base)->regs);
	printk("before: %08x \n",readl(((base)->regs)+REG_QDMA_GLOBAL_CFG));
	
	if(rxMode == QDMA_ENABLE) {
		qdmaEnableRxDma(base) ;
	} else {
		qdmaDisableRxDma(base) ;
	}
	printk("\nafter base: %px \n",(base)->regs);
	printk("after: %08x \n",readl(((base)->regs)+REG_QDMA_GLOBAL_CFG));
	return 0 ;
}

/******************************************************************************
 Descriptor:	It's used to configure the TXQOS weight type and scale.
 Input Args:	arg1: setting the WRR weighting value is base on packet or byte
 					  (QDMA_TXQOS_WEIGHT_BY_PACKET/QDMA_TXQOS_WEIGHT_BY_BYTE)
 				arg2: setting the byte weighting scale(QDMA_TXQOS_WEIGHT_SCALE_64B
 				      /QDMA_TXQOS_WEIGHT_SCALE_16B). when the weigthing value 
 				      is base on packet, these argument is don't care.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_qos_weight(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	QDMA_TxQosWeightType_t weightBase = qdma_data->qdma_private.qdma_tx_qos.weightBase ;
	QDMA_TxQosWeightScale_t weightScale = qdma_data->qdma_private.qdma_tx_qos.weightScale ;

	if(weightBase == QDMA_TXQOS_WEIGHT_BY_PACKET) {
		qdmaSetTxQosWeightByPacket(base) ;
	} else if(weightBase == QDMA_TXQOS_WEIGHT_BY_BYTE) {
		qdmaSetTxQosWeightByByte(base) ;
#if SUPPORT_WRR_WEIGHT_SCALE_1Byte
		if(weightScale == QDMA_TXQOS_WEIGHT_SCALE_1B) {
			qdmaSetTxQosWeightScale1(base) ;
		} else if(weightScale == QDMA_TXQOS_WEIGHT_SCALE_16B) {
			qdmaSetTxQosWeightScale16(base) ;
		} else {
			return -EINVAL ;
		}
#else
		if(weightScale == QDMA_TXQOS_WEIGHT_SCALE_64B) {
			qdmaSetTxQosWeightScale64(base) ;
		} else if(weightScale == QDMA_TXQOS_WEIGHT_SCALE_16B) {
			qdmaSetTxQosWeightScale16(base) ;
		} else {
			return -EINVAL ;
		}
#endif
	} else {
		return -EINVAL ;
	}
	
	return 0 ;
}

/******************************************************************************
 Descriptor:	It's used to get the TXQOS weight type and scale.
 Input Args:	arg1: the pointer of the weight base value 
 				      (QDMA_TXQOS_WEIGHT_BY_PACKET/QDMA_TXQOS_WEIGHT_BY_BYTE)
 				arg2: the pointer of the weight scale value
 					  (QDMA_TXQOS_WEIGHT_SCALE_64B/QDMA_TXQOS_WEIGHT_SCALE_16B)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_qos_weight(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];

	if(qdmaIsTxQosWeightByByte(base)) {
		qdma_data->qdma_private.qdma_tx_qos.weightBase = QDMA_TXQOS_WEIGHT_BY_BYTE ;
	} else {
		qdma_data->qdma_private.qdma_tx_qos.weightBase = QDMA_TXQOS_WEIGHT_BY_PACKET ;
	}
	
	if(qdmaIsTxQosWeightScale16(base)) {
		qdma_data->qdma_private.qdma_tx_qos.weightScale = QDMA_TXQOS_WEIGHT_SCALE_16B ;
	} else {
#if SUPPORT_WRR_WEIGHT_SCALE_1Byte
		qdma_data->qdma_private.qdma_tx_qos.weightScale = QDMA_TXQOS_WEIGHT_SCALE_1B ;
#else
		qdma_data->qdma_private.qdma_tx_qos.weightScale = QDMA_TXQOS_WEIGHT_SCALE_64B ;
#endif
	}
	
	return 0 ;
}


/******************************************************************************
******************************************************************************/
static int __inline__ qdmaChecConfigDone(struct airoha_qdma *base, uint offset, uint doneBit)
{
	int RETRY = 3 ;
	volatile uint regValue = 0 ;
	
	while(RETRY--) {
		//regValue = IO_GREG(reg) ;
		regValue = airoha_qdma_rr(base, offset);
		if(regValue & doneBit) {
			break ;
		}
		mdelay(1) ;
	}
	if(RETRY < 0) {
		return -ETIME ;
	}

	return 0;
}

int qdmaSetTxQosScheduler(unchar channel, unchar mode, ushort weight[8])
{
	int i = 0 ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	uint wrrCfg = 0 ;
	
	for(i=0 ; i<8 ; i++) {
#if !SUPPORT_WRR_WEIGHT_255
		if(weight[i] == 255)
            continue;
#endif
		wrrCfg = (TXWRR_RWCMD | 
				  ((weight[i]<<TXWRR_WRR_VALUE_SHIFT)&TXWRR_WRR_VALUE_MASK) |
				  ((channel<<TXWRR_CHNL_IDX_SHIFT)&TXWRR_CHNL_IDX_MASK) |
				  ((i<<TXWRR_QUEUE_IDX_SHIFT)&TXWRR_QUEUE_IDX_MASK)) ;
		
		//IO_SREG(QDMA_CSR_TXWRR_WEIGHT_CFG(base), wrrCfg) ;
		
		airoha_qdma_wr(base, QDMA_CSR_TXWRR_WEIGHT_CFG, wrrCfg);
	
		if(qdmaChecConfigDone(base, QDMA_CSR_TXWRR_WEIGHT_CFG, TXWRR_RWCMD_DONE) < 0) {
			QDMA_ERR("Timeout for setting WRR configuration, channel:%d, queue:%d.\n", channel, i) ;
			return -ETIME ;
		}
	}

	qdmaSetPerChnlQosMode(base, channel, mode) ;
	
	return 0 ;
}

int qdmaGetTxQosScheduler(unchar channel, unchar *pMode, ushort weight[8])
{
	int i = 0 ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	uint wrrCfg = 0 ;
	
	*pMode = qdmaGetPerChnlQosMode(base, channel) ;

	for(i=0 ; i<8 ; i++) {
		wrrCfg = (((channel<<TXWRR_CHNL_IDX_SHIFT)&TXWRR_CHNL_IDX_MASK) |
				  ((i<<TXWRR_QUEUE_IDX_SHIFT)&TXWRR_QUEUE_IDX_MASK)) ;
		//IO_SREG(QDMA_CSR_TXWRR_WEIGHT_CFG(base), wrrCfg) ;
		airoha_qdma_wr(base, QDMA_CSR_TXWRR_WEIGHT_CFG, wrrCfg);
		
		if(qdmaChecConfigDone(base, QDMA_CSR_TXWRR_WEIGHT_CFG, TXWRR_RWCMD_DONE) < 0) {
			QDMA_ERR("Timeout for getting WRR configuration, channel:%d, queue:%d.\n", channel, i) ;
			return -ETIME ;
		}
		//wrrCfg = IO_GREG(QDMA_CSR_TXWRR_WEIGHT_CFG(base)) ;
		
		wrrCfg = airoha_qdma_rr(base, QDMA_CSR_TXWRR_WEIGHT_CFG);
		weight[i] =  ((wrrCfg&TXWRR_WRR_VALUE_MASK)>>TXWRR_WRR_VALUE_SHIFT) ;
	}
	return 0 ;
}

int qdmaSetQueueClose_sw(struct airoha_qdma *base, unchar channel, unchar queue){
	unchar chnl_offset = (channel>>2);
	uint txq_dis_cfg = 0;

	if(chnl_offset >= TXQ_DIS_CFG_REG_NUM){
		QDMA_ERR("wrong channel index.\n") ;
		return -1;
	}

	txq_dis_cfg = TXQ_DIS_CFG_VALUE[chnl_offset];
	TXQ_DIS_CFG_VALUE[chnl_offset] = (txq_dis_cfg | TXQ_DIS_CHANNEL_QUEUE_OFFSET(channel,queue));
	//IO_SREG(QDMA_CSR_QUEUE_CLOSE_CFG(base,channel), TXQ_DIS_CFG_VALUE[chnl_offset]);
	airoha_qdma_wr(base, QDMA_CSR_QUEUE_CLOSE_CFG(channel), TXQ_DIS_CFG_VALUE[chnl_offset]);
	
	return 0;
}

int qdmaSetQueueOpen_sw(struct airoha_qdma *base, unchar channel, unchar queue){
	unchar chnl_offset = (channel>>2);
	uint txq_dis_cfg = 0;

	if(chnl_offset >= TXQ_DIS_CFG_REG_NUM){
		QDMA_ERR("wrong channel index.\n") ;
		return -1;
	}

	txq_dis_cfg = TXQ_DIS_CFG_VALUE[chnl_offset];
	TXQ_DIS_CFG_VALUE[chnl_offset] = (txq_dis_cfg & (~(TXQ_DIS_CHANNEL_QUEUE_OFFSET(channel,queue))));
	//IO_SREG(QDMA_CSR_QUEUE_CLOSE_CFG(base,channel), TXQ_DIS_CFG_VALUE[chnl_offset]);
	airoha_qdma_wr(base, QDMA_CSR_QUEUE_CLOSE_CFG(channel), TXQ_DIS_CFG_VALUE[chnl_offset]);
	
	return 0;
}


/******************************************************************************
 Descriptor:	It's used to configure the tx queue scheduler and queue weigth
 				for specific channel.
 Input Args:	The pointer of the tx qos scheduler struct. It includes:
 				- channel: specific the channel ID (0~15)
 				- qosType: The QoS type is define in QDMA_TxQosType_t enum.
 				- weight: The unit of WRR weight is packets (0~100, 255 means don't care).
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_qos(struct ECNT_QDMA_Data *qdma_data) 
{
    //uint base = gpQdmaPriv->csrBaseAddr ;
#if SUPPORT_QUEUE_DISABLE
	struct airoha_qdma *base = &glb_eth->qdma[1];
#endif
	int ret = 0, i = 0, j = 0;
	unchar qosType = 0 ;
	ushort weight[CONFIG_QDMA_QUEUE];
    unchar qosStr[128], qosStrTmp[16] ;
	int weightNum[QDMA_TXQOS_TYPE_NUMS]={8, 0, 7, 6, 5, 4, 3, 2};
	QDMA_TxQosScheduler_T *pTxQos = qdma_data->qdma_private.qdma_tx_qos.pTxQos ;
	
	if(pTxQos->channel >= CONFIG_QDMA_CHANNEL) {
		return -EINVAL ;
	}
	
	if(pTxQos->qosType >= QDMA_TXQOS_TYPE_NUMS) {
		return -EINVAL ;
	}
	qosType = (unchar)pTxQos->qosType;

#if SUPPORT_QUEUE_DISABLE
    for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
        if( i >= weightNum[qosType] ){/*SP : open the queue*/
            qdmaSetQueueOpen_sw(base,pTxQos->channel,i) ;
        }else{/*WRR: check weight */
            if(pTxQos->queue[i].weight == 0){
                qdmaSetQueueClose_sw(base,pTxQos->channel,i) ;
            }else{
                qdmaSetQueueOpen_sw(base,pTxQos->channel,i) ;
            }
        }
    }

#elif 0
    for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
        if( i >= weightNum[qosType] ){/*SP : open the queue*/
            qdmaSetQueueOpen(base,pTxQos->channel,i) ;
        }else{/*WRR: check weight */
            if(pTxQos->queue[i].weight == 0){
                qdmaSetQueueClose(base,pTxQos->channel,i) ;
            }else{
                qdmaSetQueueOpen(base,pTxQos->channel,i) ;
            }
        }
    }
#else
	for(i=0 ; i<weightNum[pTxQos->qosType] ; i++) {
		if(pTxQos->queue[i].weight == 0) {	
            /* if fixed weight has value 0 in new qosType, then config weight, but use old qosType  */
			ret = qdmaGetTxQosScheduler(pTxQos->channel, &qosType, weight) ;
			if(ret < 0) {
				QDMA_ERR("qdmaGetTxQosScheduler error\n");
				return -EFAULT ;
			}
			for(j=0 ; j<weightNum[qosType] ; j++) {
				if(pTxQos->queue[j].weight == 0) {	/* if fixed weight has value 0 in old qosType, then exit  */
					QDMA_ERR("channel:%d, old qosTYpe:%d, qosWeight[%d] is %d\n", pTxQos->channel, qosType, j, pTxQos->queue[j].weight);
					return -EFAULT ;
				}
			}
			sprintf(qosStr, "channel is %d, old qosType is %d, new qosType is %d, Q0:Q1:...:Q7=", pTxQos->channel, qosType, pTxQos->qosType);
			for(j=0 ; j<CONFIG_QDMA_QUEUE ; j++) {
				sprintf(qosStrTmp, "%d ", pTxQos->queue[j].weight);
				strcat(qosStr, qosStrTmp);
			}
			strcat(qosStr, "\n");
			QDMA_MSG(QDMA_DBG_ST, "%s", qosStr);
		}
	}
#endif

	for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
#if SUPPORT_QUEUE_DISABLE
        /*in case ,the packet queued in QDMA*/
        /*if wrr=0 , set wrr=1 and close the queue*/
        /*if DE add ageout function, need to be deleted*/
        if(pTxQos->queue[i].weight == 0)
            pTxQos->queue[i].weight = 1 ;
#else
		if((pTxQos->queue[i].weight>100) && (pTxQos->queue[i].weight!=255)) {
			return -EINVAL ;
		}
#endif
		weight[i] = pTxQos->queue[i].weight ;
	}
	
	return qdmaSetTxQosScheduler(pTxQos->channel, qosType, weight) ;
}


uint qdmaIsQueueClosed_sw(unchar channel, unchar queue){
	unchar chnl_offset = (channel>>2);
	uint txq_dis_cfg = 0;
	uint ret = 0;

	if(chnl_offset >= TXQ_DIS_CFG_REG_NUM){
		QDMA_ERR("wrong channel index.\n") ;
		return 0;
	}

	txq_dis_cfg = TXQ_DIS_CFG_VALUE[chnl_offset];
	ret = (txq_dis_cfg & TXQ_DIS_CHANNEL_QUEUE_OFFSET(channel, queue));

	return ret;
}

/******************************************************************************
 Descriptor:	It's used to set the tx queue scheduler and queue weigth
 				for specific channel.
 Input Args:	The pointer of the tx qos scheduler struct. It includes:
 				- channel: specific the channel ID (0~15)
 				- qosType: The QoS type is define in QDMA_TxQosType_t enum.
 				- weight: The unit of WRR weight is packets.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_qos(struct ECNT_QDMA_Data *qdma_data)
{
    //uint base = gpQdmaPriv->csrBaseAddr ;
	int ret = 0, i = 0 ;
	unchar qosType = 0 ;
	ushort weight[CONFIG_QDMA_QUEUE] ;
	QDMA_TxQosScheduler_T *pTxQos = qdma_data->qdma_private.qdma_tx_qos.pTxQos ;
	
	if(pTxQos->channel >= CONFIG_QDMA_CHANNEL) {
		return -EINVAL ;
	}

	ret = qdmaGetTxQosScheduler(pTxQos->channel, &qosType, weight) ;
	if(ret < 0) {
		return -EFAULT ;
	}
	
	pTxQos->qosType = qosType ;
	
	for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
    //if DE add ageout function, need to be deleted
#if SUPPORT_QUEUE_DISABLE/*if wrr=0 , set wrr=1 and close the queue*/
        if((weight[i]==1) && (qdmaIsQueueClosed_sw(pTxQos->channel,i)>0))
            weight[i] = 0 ;
#endif
		pTxQos->queue[i].weight = weight[i] ;
	}

	return 0 ;
}

/******************************************************************************
 Description:	It's used to unregister the QDMA callback functions.
 Input Args:	arg1: callback function type that is define in QDMA_CbType_t enum.
 Ret Value:		0: unregister successful otherwise failed.
******************************************************************************/
int qdma_unregister_callback_function(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
	struct airoha_qdma *base = &glb_eth->qdma[1];
#endif
	QDMA_CbType_t type = qdma_data->qdma_private.qdma_reg_cb.type ;

	switch(type) {
		case QDMA_CALLBACK_RX_PACKET:
			gpQdmaPriv->devCfg.bmRecvCallbackFunction = NULL ;	
			break ;
	#if defined(TCSUPPORT_FTTDP_V2) && defined(QDMA_LAN)
        case QDMA_CALLBACK_RX_PACKET_PTM_LAN:
            gpQdmaPriv->devCfg.bmRecvCallbackPtmLanFunction = NULL ;  
            break ;
    #endif
		case QDMA_CALLBACK_EVENT_HANDLER:
			gpQdmaPriv->devCfg.bmEventCallbackFunction = NULL ;	
			break ;
			
		case QDMA_CALLBACK_RECYCLE_PACKET:
			gpQdmaPriv->devCfg.bmRecyPktCallbackFunction = NULL ;	
			break ;
#if !SUPPORT_INT_ENABLE_NUM_2	
		case QDMA_CALLBACK_GPON_MAC_HANDLER :
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
			qdmaDisableInt(base, INT_MASK_GPON_MAC, QDMA_INT1, QDMA_INT_ENABLE0) ;
#endif		
			gpQdmaPriv->devCfg.bmGponMacIntHandler = NULL ;
			break ;
			
		case QDMA_CALLBACK_EPON_MAC_HANDLER :
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)		
			qdmaDisableInt(base, INT_MASK_EPON_MAC, QDMA_INT1, QDMA_INT_ENABLE0) ;
#endif
			gpQdmaPriv->devCfg.bmEponMacIntHandler = NULL ;
			break ;
			
		case QDMA_CALLBACK_XPON_PHY_HANDLER :
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)		
			qdmaDisableInt(base, INT_MASK_XPON_PHY, QDMA_INT1, QDMA_INT_ENABLE0) ;
#endif
			gpQdmaPriv->devCfg.bmXponPhyIntHandler = NULL ;
			break ;
#endif
        /* multi-INTs extend*/
       case QDMA_CALLBACK_INT2_RX_PACKET:
           gpQdmaPriv->devCfg.bmInt2RecvCallbackFunction = NULL ;   
           break ;

       case QDMA_CALLBACK_INT2_EVENT_HANDLER:
           gpQdmaPriv->devCfg.bmInt2EventCallbackFunction = NULL ;    
           break ;
           
       case QDMA_CALLBACK_INT3_RX_PACKET:
           gpQdmaPriv->devCfg.bmInt3RecvCallbackFunction = NULL ;   
           break ;

       case QDMA_CALLBACK_INT3_EVENT_HANDLER:
           gpQdmaPriv->devCfg.bmInt3EventCallbackFunction = NULL ;    
           break ;

       case QDMA_CALLBACK_INT4_RX_PACKET:
           gpQdmaPriv->devCfg.bmInt4RecvCallbackFunction = NULL ;   
           break ;

       case QDMA_CALLBACK_INT4_EVENT_HANDLER:
           gpQdmaPriv->devCfg.bmInt4EventCallbackFunction = NULL ;    
           break ;
			
		default:
			return -EFAULT ;	
	}
	return 0 ;
}

/******************************************************************************
 Descriptor:	It's used to register the QDMA callback functions. The QDMA 
 				driver support several callback function type that is define 
 				in QDMA_CbType_t enum.
 Input Args:	arg1: callback function type that is define in QDMA_CbType_t enum.
 				arg2: the pointer of the callback function.
 Ret Value:		0: register successful otherwise failed.
******************************************************************************/
int qdma_register_callback_function(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
	struct airoha_qdma *base = &glb_eth->qdma[1];
#endif
	QDMA_CbType_t type = qdma_data->qdma_private.qdma_reg_cb.type ;
	void *pCbFun = qdma_data->qdma_private.qdma_reg_cb.pCbFun ;

	if(!pCbFun) {
		return -EINVAL ;	
	}
	
	switch(type) {
		case QDMA_CALLBACK_RX_PACKET:
			gpQdmaPriv->devCfg.bmRecvCallbackFunction = (qdma_callback_recv_packet_t)pCbFun ;	
			break ;
#if defined(TCSUPPORT_FTTDP_V2) && defined(QDMA_LAN)
        case QDMA_CALLBACK_RX_PACKET_PTM_LAN:
            gpQdmaPriv->devCfg.bmRecvCallbackPtmLanFunction = (qdma_callback_recv_packet_t)pCbFun ;   
            break ;
#endif
			
		case QDMA_CALLBACK_EVENT_HANDLER:
			gpQdmaPriv->devCfg.bmEventCallbackFunction = (qdma_callback_event_handler_t)pCbFun ;	
			break ;
			
		case QDMA_CALLBACK_RECYCLE_PACKET:
			gpQdmaPriv->devCfg.bmRecyPktCallbackFunction = (qdma_callback_recycle_packet_t)pCbFun ;	
			break ;
			
#if !SUPPORT_INT_ENABLE_NUM_2
		case QDMA_CALLBACK_GPON_MAC_HANDLER :
			gpQdmaPriv->devCfg.bmGponMacIntHandler = (qdma_callback_int_handler_t)pCbFun ;
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
			qdmaEnableInt(base, INT_MASK_GPON_MAC, QDMA_INT1, QDMA_INT_ENABLE0) ;
#endif
			break ;
			
		case QDMA_CALLBACK_EPON_MAC_HANDLER :
			gpQdmaPriv->devCfg.bmEponMacIntHandler = (qdma_callback_int_handler_t)pCbFun ;
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
			qdmaEnableInt(base, INT_MASK_EPON_MAC, QDMA_INT1, QDMA_INT_ENABLE0) ;
#endif
			break ;
			
		case QDMA_CALLBACK_XPON_PHY_HANDLER :
			gpQdmaPriv->devCfg.bmXponPhyIntHandler = (qdma_callback_int_handler_t)pCbFun ;
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
			qdmaEnableInt(base, INT_MASK_XPON_PHY, QDMA_INT1, QDMA_INT_ENABLE0) ;
#endif
			break ;
#endif
        /* multi-INTs extend*/
        case QDMA_CALLBACK_INT2_RX_PACKET:
            gpQdmaPriv->devCfg.bmInt2RecvCallbackFunction = (qdma_callback_recv_packet_with_info_t)pCbFun ;
			break ;

        case QDMA_CALLBACK_INT2_EVENT_HANDLER:
			gpQdmaPriv->devCfg.bmInt2EventCallbackFunction = (qdma_callback_event_handler_t)pCbFun ;	
			break ;

        case QDMA_CALLBACK_INT3_RX_PACKET:
            gpQdmaPriv->devCfg.bmInt3RecvCallbackFunction = (qdma_callback_recv_packet_with_info_t)pCbFun ;	
			break ;

        case QDMA_CALLBACK_INT3_EVENT_HANDLER:
			gpQdmaPriv->devCfg.bmInt3EventCallbackFunction = (qdma_callback_event_handler_t)pCbFun ;	
			break ;

        case QDMA_CALLBACK_INT4_RX_PACKET:
            gpQdmaPriv->devCfg.bmInt4RecvCallbackFunction = (qdma_callback_recv_packet_with_info_t)pCbFun ;	
			break ;

        case QDMA_CALLBACK_INT4_EVENT_HANDLER:
			gpQdmaPriv->devCfg.bmInt4EventCallbackFunction = (qdma_callback_event_handler_t)pCbFun ;	
			break ;

		default:
			return -EFAULT ;	
	}
	return 0 ;
}

/******************************************************************************
 Descriptor:	It's used to init the QDMA software driver and hardware device.
 				This function must be called if the upper layer application wanna
 				use the QDMA to send/receive packets.
 Input Args:	The pointer of the QDMA_InitCfg_t
 Ret Value:		0: init successful otherwise failed.
******************************************************************************/
int qdma_init(struct ECNT_QDMA_Data *qdma_data)
{
	QDMA_InitCfg_t *pInitCfg = qdma_data->qdma_private.pInitCfg ;

    gpQdmaPriv->devCfg.macType = pInitCfg->macType;
	if(gpQdmaPriv->devCfg.flags.isTxPolling == QDMA_ENABLE)
   		gpQdmaPriv->devCfg.txRecycleThreshold = pInitCfg->txRecycleThrh ;
	
	qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_RX_PACKET;
	qdma_data->qdma_private.qdma_reg_cb.pCbFun = pInitCfg->cbRecvPkts;
	qdma_register_callback_function(qdma_data) ; 
#if defined(TCSUPPORT_FTTDP_V2) && defined(QDMA_LAN)
	qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_RX_PACKET_PTM_LAN;
	qdma_data->qdma_private.qdma_reg_cb.pCbFun = pInitCfg->cbRecvPktsPtmLan;
	qdma_register_callback_function(qdma_data) ;
#endif
	qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_EVENT_HANDLER;
	qdma_data->qdma_private.qdma_reg_cb.pCbFun = pInitCfg->cbEventHandler;
	qdma_register_callback_function(qdma_data) ;
	qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_RECYCLE_PACKET;
	qdma_data->qdma_private.qdma_reg_cb.pCbFun = pInitCfg->cbRecyclePkts;
	qdma_register_callback_function(qdma_data) ; 
	qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_GPON_MAC_HANDLER;
	qdma_data->qdma_private.qdma_reg_cb.pCbFun = pInitCfg->cbGponMacHandler;
	qdma_register_callback_function(qdma_data) ;
	qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_EPON_MAC_HANDLER;
	qdma_data->qdma_private.qdma_reg_cb.pCbFun = pInitCfg->cbEponMacHandler;
	qdma_register_callback_function(qdma_data) ;
	qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_XPON_PHY_HANDLER;
	qdma_data->qdma_private.qdma_reg_cb.pCbFun = pInitCfg->cbXponPhyHandler;
	qdma_register_callback_function(qdma_data) ;

    /*multi-INTs extend*/
    qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_INT2_RX_PACKET;
	qdma_data->qdma_private.qdma_reg_cb.pCbFun = pInitCfg->cbInt2RecvPkts;
	qdma_register_callback_function(qdma_data) ;
    qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_INT2_EVENT_HANDLER;
	qdma_data->qdma_private.qdma_reg_cb.pCbFun = pInitCfg->cbInt2EventHandler;
	qdma_register_callback_function(qdma_data) ;
    qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_INT3_RX_PACKET;
	qdma_data->qdma_private.qdma_reg_cb.pCbFun = pInitCfg->cbInt3RecvPkts;
	qdma_register_callback_function(qdma_data) ;
    qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_INT3_EVENT_HANDLER;
	qdma_data->qdma_private.qdma_reg_cb.pCbFun = pInitCfg->cbInt3EventHandler;
	qdma_register_callback_function(qdma_data) ;
    qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_INT4_RX_PACKET;
	qdma_data->qdma_private.qdma_reg_cb.pCbFun = pInitCfg->cbInt4RecvPkts;
	qdma_register_callback_function(qdma_data) ;
    qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_INT4_EVENT_HANDLER;
	qdma_data->qdma_private.qdma_reg_cb.pCbFun = pInitCfg->cbInt4EventHandler;
	qdma_register_callback_function(qdma_data) ;

	return 0 ;
}

/******************************************************************************
 Descriptor:	It's used to deinit the QDMA software driver and hardware device.
 				This function must be called if the upper layer application wanna
 				transfer to another application.
 Input Args:	None
 Ret Value:		0: init successful otherwise failed.
******************************************************************************/
int qdma_deinit(struct ECNT_QDMA_Data *qdma_data)
{
	qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_RX_PACKET;
	qdma_unregister_callback_function(qdma_data) ; 
#if defined(TCSUPPORT_FTTDP_V2) && defined(QDMA_LAN)
	qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_RX_PACKET_PTM_LAN;
	qdma_unregister_callback_function(qdma_data) ;
#endif
	qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_EVENT_HANDLER;
	qdma_unregister_callback_function(qdma_data) ;
	qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_RECYCLE_PACKET;
	qdma_unregister_callback_function(qdma_data) ; 
	qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_GPON_MAC_HANDLER;
	qdma_unregister_callback_function(qdma_data) ;
	qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_EPON_MAC_HANDLER;
	qdma_unregister_callback_function(qdma_data) ;
	qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_XPON_PHY_HANDLER;
	qdma_unregister_callback_function(qdma_data) ;

    /*multi-INTs extend*/
    qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_INT2_RX_PACKET;
	qdma_unregister_callback_function(qdma_data) ; 
    qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_INT2_EVENT_HANDLER;
	qdma_unregister_callback_function(qdma_data) ;
    qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_INT3_RX_PACKET;
	qdma_unregister_callback_function(qdma_data) ; 
    qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_INT3_EVENT_HANDLER;
	qdma_unregister_callback_function(qdma_data) ;
    qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_INT4_RX_PACKET;
	qdma_unregister_callback_function(qdma_data) ; 
    qdma_data->qdma_private.qdma_reg_cb.type = QDMA_CALLBACK_INT4_EVENT_HANDLER;
	qdma_unregister_callback_function(qdma_data) ;

	return 0 ;
}

/******************************************************************************
 Descriptor:	It's used to enable/disable tx queue DEI dropped function. 
 Input Args:	arg1: Enable/disable tx queue DEI function (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_dei_drop_mode(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	QDMA_Mode_t deiDropMode = qdma_data->qdma_private.mode ;

	if(deiDropMode == QDMA_ENABLE) {
		qdmaEnableTxqCngstDeiDrop(base) ;
	} else {
		qdmaDisableTxqCngstDeiDrop(base) ;
	}
	
	return 0;
}


/******************************************************************************
 Descriptor:	It's used to set txQ queue nonblocking or not.
 Input Args:	arg1: The pointer of the TXQ Cngst Queue Cfg struct.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_queue_nonblocking(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	QDMA_TxQCngstQueueCfg_T *txqCngstQueueCfgPtr = qdma_data->qdma_private.txqCngstQueueCfgPtr;
	
	if (txqCngstQueueCfgPtr->queueMode == QDMA_QUEUE_NONBLOCKING) {
		qdmaEnableTxqCngstQueueNonblocking(base, txqCngstQueueCfgPtr->queue);
	}
	else {
		qdmaDisableTxqCngstQueueNonblocking(base, txqCngstQueueCfgPtr->queue);
	}

	return 0;
}

/******************************************************************************
 Descriptor:	It's used to set txQ channel nonblocking or not.
 Input Args:	arg1: The pointer of the TXQ Cngst Channel Cfg struct.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_channel_nonblocking(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	QDMA_TxQCngstChannelCfg_T *txqCngstChannelCfgPtr = qdma_data->qdma_private.txqCngstChannelCfgPtr;
	
	if (txqCngstChannelCfgPtr->channelMode == QDMA_CHANNEL_NONBLOCKING) {
		qdmaEnableTxqCngstChannelNonblocking(base, txqCngstChannelCfgPtr->channel);
	}
	else {
		qdmaDisableTxqCngstChannelNonblocking(base, txqCngstChannelCfgPtr->channel);
	}

	return 0;
}

/******************************************************************************
 Descriptor:	It's used to enable/disable QDMA_OAM_MODIFY_FP.
 Input Args:	arg1: Enable/disable QDMA_OAM_MODIFY_FP (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:     0: successful, otherwise failed.
******************************************************************************/
int qdma_set_oam_modify_fp_en(struct ECNT_QDMA_Data *qdma_data)
{
#if SUPPORT_OAM_MODIFY_FP
    //uint base = gpQdmaPriv->csrBaseAddr ;
    struct airoha_qdma *base = &glb_eth->qdma[1];
	QDMA_Mode_t oamFpMode = qdma_data->qdma_private.mode;
    
    if(oamFpMode == QDMA_ENABLE) {
#ifdef QDMA_LAN
		if(qdmaLanHqosMode == 0){
			qdmaEnableOamModifyFpEn(base) ;
		}
#else
		if(qdmaWanHqosMode == 0){
			qdmaEnableOamModifyFpEn(base) ;
		}
#endif
    } else {
        qdmaDisableOamModifyFpEn(base) ;
    }
#endif

    return 0 ;
}

/*rx msg*/
static inline void qdma_get_rxmsg_whole_rxmsg0(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW0.word;
    
    return;
}

static inline void qdma_get_rxmsg_whole_rxmsg1(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.word;
    
    return;
}

static inline void qdma_get_rxmsg_whole_rxmsg2(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW2.word;
    
    return;
}

static inline void qdma_get_rxmsg_whole_rxmsg3(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW3.word;
    
    return;
}

static inline void qdma_get_rxmsg_sptag(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW0.raw.sp_tag;
    
    return;
}
static inline void qdma_get_rxmsg_foe_index(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.raw.ppe_entry;
    
    return;
}

static inline void qdma_get_rxmsg_crsn(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.raw.crsn;
    
    return;
}

static inline void qdma_get_rxmsg_sport(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.raw.sport;
    
    return;
}

static inline void qdma_get_rxmsg_ip4f(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.raw.ip4f;
    
    return;
}

static inline void qdma_get_rxmsg_l4f(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.raw.l4f;
    
    return;
}

static inline void qdma_get_rxmsg_l4vld(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.raw.l4vld;
    
    return;
}

static inline void qdma_get_rxmsg_ip4(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.raw.ip4;
    
    return;
}

static inline void qdma_get_rxmsg_ip6(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.raw.ip6;
    
    return;
}

static inline void qdma_get_rxmsg_l2_len(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW2.raw.l2_len;
    
    return;
}

static inline void qdma_get_rxmsg_ifc_hit(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW2.raw.ifc_hit;
    
    return;
}

static inline void qdma_get_rxmsg_ifc_id(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW2.raw.ifc_id;
    
    return;
}

qdma_rxmsg_op_t
qdma_rxmsg_operation[]=
{
    qdma_get_rxmsg_whole_rxmsg0,
    qdma_get_rxmsg_whole_rxmsg1,
    qdma_get_rxmsg_whole_rxmsg2,
    qdma_get_rxmsg_whole_rxmsg3,
    qdma_get_rxmsg_sptag,
    qdma_get_rxmsg_foe_index,
    qdma_get_rxmsg_crsn,
    qdma_get_rxmsg_sport,
    qdma_get_rxmsg_ip4f,
    qdma_get_rxmsg_l4f,
    qdma_get_rxmsg_l4vld,
    qdma_get_rxmsg_ip4,
    qdma_get_rxmsg_ip6,
    qdma_get_rxmsg_l2_len,
    qdma_get_rxmsg_ifc_hit,
    qdma_get_rxmsg_ifc_id,
};

int qdma_api_get_rxmsg(struct ECNT_QDMA_Data *qdma_data)
{
    rxMsgWord_t* pRxMsg = (rxMsgWord_t*)(qdma_data->qdma_private.qdmaGetRxMsgCfg.pRxMsg);
    QDMA_GET_RXMSG_FIELD_t field = qdma_data->qdma_private.qdmaGetRxMsgCfg.field;
    unsigned int* pValue = qdma_data->qdma_private.qdmaGetRxMsgCfg.pValue;

    if(field < RXMSG_FIELD_MAX_NUM){
        qdma_rxmsg_operation[field](pRxMsg,pValue);
    }
	return 0;
}


static inline int qdma_set_txbuf_threshold_inline(struct airoha_qdma *base,QDMA_Mode_t mode,unsigned short chnThrh,unsigned short totalThrh)
{
    if(QDMA_ENABLE == mode) {
		qdmaSetPseBufChnnelThreshold(base, chnThrh) ;
		qdmaSetPseBufTotalThreshold(base, totalThrh) ;
	} else if(QDMA_DISABLE == mode) {
		qdmaDisablePseBufEstimate(base) ;
	} else {
		return -EINVAL ;
	}  

    return 0;
}


/******************************************************************************
 Descriptor:	It's used to configure the tx buffer threshold. For the buffer 
 				management, the total available on-chip buffer is 64Kbyte (256 
 				Blocks, 256 bytes per block). It is shared among WAN and LAN 
 				Tx/Rx interface. If the buffer usage exceeds the threshold, the
 				Tx DMA will stop retrieving packets.
 Input Args:	The pointer of the tx buffer control struct. It includes:
 				- mode: Eanble/Disable tx buffer usage control
 				- chnThreshold: Per tx per channel block usage threshold.
 				- totalThreshold: Total tx block usage threshold.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txbuf_threshold(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	QDMA_TxBufCtrl_T *pTxBufCtrl = qdma_data->qdma_private.pQdmaTxBufCtrl ;

    qdma_set_txbuf_threshold_inline(base,pTxBufCtrl->mode,pTxBufCtrl->chnThreshold,pTxBufCtrl->totalThreshold);

	return 0 ;
}


static inline int qdma_get_txbuf_threshold_inline(struct airoha_qdma *base,QDMA_Mode_t *p_mode,unsigned short *p_chnThrh,unsigned short *p_totalThrh)
{
    *p_mode = (qdmaIsPseBufEstimateEnable(base)?(QDMA_ENABLE):(QDMA_DISABLE));
    *p_chnThrh= qdmaGetPseBufChnnelThreshold(base);
    *p_totalThrh= qdmaGetPseBufTotalThreshold(base);

    return 0;
}

/******************************************************************************
 Descriptor:	It's used to get the tx buffer threshold.
 Input Args:	The pointer of the tx buffer control struct. It includes:
 				- mode: Eanble/Disable tx buffer usage control
 				- chnThreshold: Per tx per channel block usage threshold.
 				- totalThreshold: Total tx block usage threshold.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_txbuf_threshold(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	
	QDMA_TxBufCtrl_T *pTxBufCtrl = qdma_data->qdma_private.pQdmaTxBufCtrl ;
    QDMA_Mode_t	mode;
	unsigned short  chnThreshold;
	unsigned short  totalThreshold;
    
    qdma_get_txbuf_threshold_inline(base,&mode,&chnThreshold,&totalThreshold);
    
	pTxBufCtrl->mode = mode;
	pTxBufCtrl->chnThreshold = chnThreshold;
	pTxBufCtrl->totalThreshold = totalThreshold;

	return 0 ;
}


static inline int QDMA_API_GET_TXBUF_THRESHOLD(QDMA_TxBufCtrl_T *pTxBufCtrl){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TXBUF_THRESHOLD;
	in_data.qdma_private.pQdmaTxBufCtrl = pTxBufCtrl ;
	ret = qdma_get_txbuf_threshold(&in_data);
	return ret;
}

static inline int QDMA_API_SET_TXBUF_THRESHOLD(QDMA_TxBufCtrl_T *pTxBufCtrl){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TXBUF_THRESHOLD;
	in_data.qdma_private.pQdmaTxBufCtrl = pTxBufCtrl ;
	ret = qdma_set_txbuf_threshold(&in_data);
	return ret;
}

int qdmaSetTxQosSchedulerUpstream(struct airoha_qdma *base, unchar channel, unchar mode, ushort weight[8])
{
	int i = 0 ;
	//struct airoha_qdma *base = &glb_eth->qdma[1];
	uint wrrCfg = 0 ;
	
	for(i=0 ; i<8 ; i++) {
		wrrCfg = (TXWRR_RWCMD | 
				  ((weight[i]<<TXWRR_WRR_VALUE_SHIFT)&TXWRR_WRR_VALUE_MASK) |
				  ((channel<<TXWRR_CHNL_IDX_SHIFT)&TXWRR_CHNL_IDX_MASK) |
				  ((i<<TXWRR_QUEUE_IDX_SHIFT)&TXWRR_QUEUE_IDX_MASK)) ;
		
		//IO_SREG(QDMA_CSR_TXWRR_WEIGHT_CFG(base), wrrCfg) ;
		
		airoha_qdma_wr(base, QDMA_CSR_TXWRR_WEIGHT_CFG, wrrCfg);
	
		if(qdmaChecConfigDone(base, QDMA_CSR_TXWRR_WEIGHT_CFG, TXWRR_RWCMD_DONE) < 0) {
			QDMA_ERR("Timeout for setting WRR configuration, channel:%d, queue:%d.\n", channel, i) ;
			return -ETIME ;
		}
	}

	qdmaSetPerChnlQosMode(base, channel, mode) ;
	
	return 0 ;
}

int qdmaGetTxQosSchedulerUpstream(struct airoha_qdma *base, unchar channel, unchar *pMode, ushort weight[8])
{
	int i = 0 ;
	uint wrrCfg = 0 ;
	
	*pMode = qdmaGetPerChnlQosMode(base, channel) ;

	for(i=0 ; i<8 ; i++) {
		wrrCfg = (((channel<<TXWRR_CHNL_IDX_SHIFT)&TXWRR_CHNL_IDX_MASK) |
				  ((i<<TXWRR_QUEUE_IDX_SHIFT)&TXWRR_QUEUE_IDX_MASK)) ;
		//IO_SREG(QDMA_CSR_TXWRR_WEIGHT_CFG(base), wrrCfg) ;
		airoha_qdma_wr(base, QDMA_CSR_TXWRR_WEIGHT_CFG, wrrCfg);
		
		if(qdmaChecConfigDone(base, QDMA_CSR_TXWRR_WEIGHT_CFG, TXWRR_RWCMD_DONE) < 0) {
			QDMA_MSG(QDMA_DBG_WARN,"Timeout for getting WRR configuration, channel:%d, queue:%d.\n", channel, i) ;
			return -ETIME ;
		}
		//wrrCfg = IO_GREG(QDMA_CSR_TXWRR_WEIGHT_CFG(base)) ;
		
		wrrCfg = airoha_qdma_rr(base, QDMA_CSR_TXWRR_WEIGHT_CFG);
		weight[i] =  ((wrrCfg&TXWRR_WRR_VALUE_MASK)>>TXWRR_WRR_VALUE_SHIFT) ;
	}
	return 0 ;
}

//only support IC after 7580 & 7528
int qdma_set_tx_qos_upstream(unsigned int mainType, struct ECNT_QDMA_Data *qdma_data) 
{
	struct airoha_qdma *base = &glb_eth->qdma[1];

	int i = 0;
	unchar qosType = 0 ;
	ushort weight[CONFIG_QDMA_QUEUE];
	int weightNum[QDMA_TXQOS_TYPE_NUMS]={8, 0, 7, 6, 5, 4, 3, 2};
	QDMA_TxQosScheduler_T *pTxQos = qdma_data->qdma_private.qdma_tx_qos.pTxQos ;

	if(mainType == ECNT_QDMA_LAN)
		base = &glb_eth->qdma[0];


	if(pTxQos->channel >= CONFIG_QDMA_CHANNEL) {
		return -EINVAL ;
	}
	
	if(pTxQos->qosType >= QDMA_TXQOS_TYPE_NUMS) {
		return -EINVAL ;
	}
	qosType = (unchar)pTxQos->qosType;

    for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
        if( i >= weightNum[qosType] ){/*SP : open the queue*/
            qdmaSetQueueOpen_sw(base,pTxQos->channel,i) ;
        }else{/*WRR: check weight */
            if(pTxQos->queue[i].weight == 0){
                qdmaSetQueueClose_sw(base,pTxQos->channel,i) ;
            }else{
                qdmaSetQueueOpen_sw(base,pTxQos->channel,i) ;
            }
        }
    }

	for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
        /*in case ,the packet queued in QDMA*/
        /*if wrr=0 , set wrr=1 and close the queue*/
        /*if DE add ageout function, need to be deleted*/
        if(pTxQos->queue[i].weight == 0)
            pTxQos->queue[i].weight = 1 ;

		weight[i] = pTxQos->queue[i].weight ;
	}
	
	return qdmaSetTxQosSchedulerUpstream(base, pTxQos->channel, qosType, weight) ;
}

int qdma_api_set_tx_qos_upstream(unsigned int mainType, QDMA_TxQosScheduler_T *pTxQos){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TX_QOS;
	in_data.qdma_private.qdma_tx_qos.pTxQos = pTxQos ;
	ret = qdma_set_tx_qos_upstream(mainType, &in_data);
	return ret;
}
EXPORT_SYMBOL(qdma_api_set_tx_qos_upstream);

int qdma_get_tx_qos_upstream(unsigned int mainType, struct ECNT_QDMA_Data *qdma_data)
{
    struct airoha_qdma *base = &glb_eth->qdma[1];

	int ret = 0, i = 0 ;
	unchar qosType = 0 ;
	ushort weight[CONFIG_QDMA_QUEUE] ;
	QDMA_TxQosScheduler_T *pTxQos = qdma_data->qdma_private.qdma_tx_qos.pTxQos ;

	if(mainType == ECNT_QDMA_LAN)
		base = &glb_eth->qdma[0];
	
	if(pTxQos->channel >= CONFIG_QDMA_CHANNEL) {
		return -EINVAL ;
	}

	ret = qdmaGetTxQosSchedulerUpstream(base, pTxQos->channel, &qosType, weight) ;
	if(ret < 0) {
		return -EFAULT ;
	}
	
	pTxQos->qosType = qosType ;
	
	for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
    //if DE add ageout function, need to be deleted
        if((weight[i]==1) && (qdmaIsQueueClosed_sw(pTxQos->channel,i)>0))
            weight[i] = 0 ;

		pTxQos->queue[i].weight = weight[i] ;
	}

	return 0 ;
}

int qdma_api_get_tx_qos_upstream(unsigned int mainType, QDMA_TxQosScheduler_T *pTxQos){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TX_QOS;
	in_data.qdma_private.qdma_tx_qos.pTxQos = pTxQos ;
	ret = qdma_get_tx_qos_upstream(mainType, &in_data);
	return ret;
}
EXPORT_SYMBOL(qdma_api_get_tx_qos_upstream);

int qdma_dump_cntr_channel(struct ECNT_QDMA_Data *qdma_data)
{
#ifdef QDMA_LAN
	return 0;
#else
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	uint channel_id = qdma_data->qdma_private.channel;

	if(16<=channel_id)
	{
		printk("the channel id_is must between 0 and 15 \n");
		return 0;
	}
	qdma_data->retValue = qdmaGetCntrCounter(base, channel_id) + qdmaGetCntrCounter(base, (channel_id + DBG_CNTR_FWD_CHNL_GROUP_NUM));
	return qdma_data->retValue;
#endif
}

int qdma_set_tx_wred_threshold(struct ECNT_QDMA_Data *qdma_data){
    #if SUPPORT_TXQ_WRED
    //uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	
	QDMA_TxWredCfg_T *txWredCfgPtr = qdma_data->qdma_private.txWredCfgPtr;
    uint i=0, regVal=0;
    unchar wredThrShift=0, wredThrCfg=0;

    /*shift-0: 0/8, 1/8, 2/8, 3/8, 4/8, 5/8, 6/8, 7/8*/
    /*shift-1: 8/8, 10/8, 12/8, 14/8 ==> 4/4, 5/4, 6/4, 7/4*/
    /*shift-2: 16/8, 20/8, 24/8, 28/8 ==> 4/2, 5/2, 6/2, 7/2*/
    /*shift-3: 32/8, 40/8, 48/8, 56/8 ==> 4/1, 5/1, 6/1, 7/1*/
    for(i=0;i<TX_WRED_THR_NUM;i++){
        if(txWredCfgPtr->txWredThrValue[i]<=56){
            if(txWredCfgPtr->txWredThrValue[i]>=32){
                wredThrShift = 3;
                wredThrCfg = txWredCfgPtr->txWredThrValue[i]/8;
            }else if((txWredCfgPtr->txWredThrValue[i]>=16) && (txWredCfgPtr->txWredThrValue[i]<32)){
                wredThrShift = 2;
                wredThrCfg = txWredCfgPtr->txWredThrValue[i]/4;
            }else if((txWredCfgPtr->txWredThrValue[i]>=8) && (txWredCfgPtr->txWredThrValue[i]<16)){
                wredThrShift=1;
                wredThrCfg = txWredCfgPtr->txWredThrValue[i]/2;
            }else{
                wredThrShift=0;
                wredThrCfg = txWredCfgPtr->txWredThrValue[i]; 
            }
            regVal |= ((wredThrShift<<WRED_THR_SHIFT_SHIFT(i))|(wredThrCfg<<WRED_THR_CFG_SHIFT(i)));
        }else{
            QDMA_ERR("Fault: Tx Wred threshold range: 0 ~ 56 !\n") ; 
            return 0;
        }
    }
    if(txWredCfgPtr->txWredPacketType>=0 && txWredCfgPtr->txWredPacketType<PACKET_TYPE_ITEMS){
        if(txWredCfgPtr->txWredPacketType == NORMAL_PACKET){
            qdmaSetWredNormThreshold(base, regVal);
        }else{ /*DEI_PACKET*/
            qdmaSetWredDeiThreshold(base, regVal);
        }
    }
    #endif

    return 0;
}

void qdmaSetTxPeekRateMargin(struct airoha_qdma *base, QDMA_PeekRateMargin_t val){
    struct ECNT_QDMA_Data qdma_data;
    QDMA_TxWredCfg_T txWredCfg;
    unchar thrValue = 0;
    unchar i = 0;

    qdma_data.qdma_private.txWredCfgPtr = &txWredCfg ;

    txWredCfg.txWredPacketType = NORMAL_PACKET;
    if(val == QDMA_TXCNGST_PEEKRATE_MARGIN_0){
        thrValue = 8; /*8/8*/
    }else if(val == QDMA_TXCNGST_PEEKRATE_MARGIN_25){
        thrValue = 10; /*10/8*/
    }else if(val == QDMA_TXCNGST_PEEKRATE_MARGIN_50){
        thrValue = 12; /*12/8*/
    }else if(val == QDMA_TXCNGST_PEEKRATE_MARGIN_100){
        thrValue = 16; /*16/8*/
    }else{
        QDMA_ERR("Fault: wrong dei threshold scale !\n");
        return;
    }
    for(i=0; i < TX_WRED_THR_NUM ; i++){
        txWredCfg.txWredThrValue[i] = thrValue;
    }

    qdma_set_tx_wred_threshold(&qdma_data);
}

/******************************************************************************
 Descriptor:	It's used to get the value of tx dynamic threshold. It includes 
 			total max threshold, total min Threshold, channel max threshold, 
 			channel min threshold, queue max threshold, queue min threshold.
 Input Args:	arg1: The pointer of the TX dynamic threshold struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_txq_cngst_dynamic_threshold(struct ECNT_QDMA_Data *qdma_data) 
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	
	QDMA_TxQDynCngstThrh_T *txqCngstThrhPtr = qdma_data->qdma_private.dynCngstThrhPtr;
	
	txqCngstThrhPtr->dynCngstTotalMaxThrh = qdmaGetTxqDynTotalMaxThrh(base) ;
	txqCngstThrhPtr->dynCngstTotalMinThrh = qdmaGetTxqDynTotalMinThrh(base) ;
	
	txqCngstThrhPtr->dynCngstChnlMaxThrh = qdmaGetTxqDynChnlMaxThrh(base) ;
	txqCngstThrhPtr->dynCngstChnlMinThrh = qdmaGetTxqDynChnlMinThrh(base) ;
	
	txqCngstThrhPtr->dynCngstQueueMaxThrh = qdmaGetTxqDynQueueMaxThrh(base) ;
	txqCngstThrhPtr->dynCngstQueueMinThrh = qdmaGetTxqDynQueueMinThrh(base) ;

#if SUPPORT_TXQ_CNGST_MIN_THRH
	txqCngstThrhPtr->dynCngstMinDscpThrh = qdmaGetTxqMinDscpThrshld(base) ;
#endif

	return 0 ;
}


/******************************************************************************
 Descriptor:	It's used to set the value of tx dynamic threshold. It includes 
 			total max threshold, total min Threshold, channel max threshold, 
 			channel min threshold, queue max threshold, queue min threshold.
 Input Args:	arg1: The pointer of the TX dynamic threshold struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_dynamic_threshold(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	
	QDMA_TxQDynCngstThrh_T *txqCngstThrhPtr = qdma_data->qdma_private.dynCngstThrhPtr;
	
	if(txqCngstThrhPtr->dynCngstTotalMinThrh > txqCngstThrhPtr->dynCngstTotalMaxThrh) {
		QDMA_ERR("Fault: Dynamic Total Min Threshold should less than Dynamic Total Max Threshold.\n") ; 
		return -EINVAL ;
	}
	
	if(txqCngstThrhPtr->dynCngstChnlMinThrh > txqCngstThrhPtr->dynCngstChnlMaxThrh) {
		QDMA_ERR("Fault: Dynamic Channel Min Threshold should less than Dynamic Channel Max Threshold.\n") ; 
		return -EINVAL ;
	}
	
	if(txqCngstThrhPtr->dynCngstQueueMinThrh > txqCngstThrhPtr->dynCngstQueueMaxThrh) {
		QDMA_ERR("Fault: Dynamic Queue Min Threshold should less than Dynamic Queue Max Threshold.\n") ; 
		return -EINVAL ;
	}

	qdmaSetTxqDynTotalMaxThrh(base, txqCngstThrhPtr->dynCngstTotalMaxThrh) ;
	qdmaSetTxqDynTotalMinThrh(base, txqCngstThrhPtr->dynCngstTotalMinThrh) ;
	
	qdmaSetTxqDynChnlMaxThrh(base, txqCngstThrhPtr->dynCngstChnlMaxThrh) ;
	qdmaSetTxqDynChnlMinThrh(base, txqCngstThrhPtr->dynCngstChnlMinThrh) ;
	
	qdmaSetTxqDynQueueMaxThrh(base, txqCngstThrhPtr->dynCngstQueueMaxThrh) ;
	qdmaSetTxqDynQueueMinThrh(base, txqCngstThrhPtr->dynCngstQueueMinThrh) ;

#if SUPPORT_TXQ_CNGST_MIN_THRH
	qdmaSetTxqMinDscpThrshld(base, txqCngstThrhPtr->dynCngstMinDscpThrh) ;
#endif

	return 0;
}


/* need modify later */
int qdma_set_txq_cngst_auto_config(struct ECNT_QDMA_Data *qdma_data)
{
    uint ringIdx=0;
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	QDMA_TxQDynCngstThrh_T txqDynCngstThrh;
	QDMA_txCngstCfg_t *pTxCngstCfg = qdma_data->qdma_private.pTxCngstCfg;

	/********************************************
	* Setting tx dynamic cngst register               *
	*********************************************/
	for(ringIdx=0; ringIdx<TX_RING_NUM; ringIdx++) {
		qdmaDisableCngstMaxThrhEn(base, ringIdx) ;
		qdmaDisableCngstMinThrhEn(base, ringIdx) ;
	}

#if NEED_ENABLE_TX_RING_BLOCKING
#if defined(QDMA_LAN)
	for(ringIdx=0; ringIdx<TX_RING_NUM; ringIdx++) {
		qdmaEnableCngstMinThrhEn(base, ringIdx) ;
	}
#endif
#endif
	qdmaEnableCngstModeConfigTrig(base);
	qdmaEnableCngstModePacketTrig(base);
	qdmaEnableCngstModeTimeTrig(base);

	qdmaEnableTxRateMeter(base);

	if(gpQdmaPriv->hwFwdDscpNum <= 256) {
		txqDynCngstThrh.dynCngstTotalMinThrh = 48;
		txqDynCngstThrh.dynCngstChnlMinThrh = 2;
		txqDynCngstThrh.dynCngstQueueMinThrh = 2;
		txqDynCngstThrh.dynCngstTotalMaxThrh = BUFFER_THRESHOLD[0];
		txqDynCngstThrh.dynCngstChnlMaxThrh = BUFFER_THRESHOLD[2];
		txqDynCngstThrh.dynCngstQueueMaxThrh = BUFFER_THRESHOLD[4];
	} else if(gpQdmaPriv->hwFwdDscpNum <= gpQdmaPriv->hwFwdDscpMaxNum){ 
		txqDynCngstThrh.dynCngstTotalMinThrh = BUFFER_THRESHOLD[1];
		txqDynCngstThrh.dynCngstChnlMinThrh = BUFFER_THRESHOLD[3];
        txqDynCngstThrh.dynCngstQueueMinThrh = BUFFER_THRESHOLD[5];
		txqDynCngstThrh.dynCngstTotalMaxThrh = BUFFER_THRESHOLD[0];
		txqDynCngstThrh.dynCngstChnlMaxThrh = BUFFER_THRESHOLD[2];
		txqDynCngstThrh.dynCngstQueueMaxThrh = BUFFER_THRESHOLD[4];
	} else {
		printk("Error: CONFIG_HWFWD_DSCP_NUM should less than %d\n",gpQdmaPriv->hwFwdDscpMaxNum);
		return -1;
	}

    #if SUPPORT_TXQ_CNGST_MIN_THRH
        txqDynCngstThrh.dynCngstMinDscpThrh = TXQ_CNGST_MIN_THRH;
    #endif
	
	if(pTxCngstCfg->txCngstMode == QDMA_TXCNGST_STATIC) {
		qdmaDisableTxqDynCngstEn(base);
		qdmaDisableTxRateMeterPeakRate(base);
		/*  open api for mac setting */
	} else if(pTxCngstCfg->txCngstMode == QDMA_TXCNGST_DYNAMIC_NORMAL) {
		qdmaEnableTxqDynCngstEn(base);
#if 0
#if SUPPORT_TXQ_CNGS_DYN_MODE_ALWAYS_ON
		qdmaEnableTxqCngstDrop(base);
#endif
#endif

#if (!SUPPORT_TX_PEAK_MODE) || (!defined(QDMA_LAN))
		qdmaDisableTxRateMeterPeakRate(base);
#endif
		qdma_data->qdma_private.dynCngstThrhPtr = &txqDynCngstThrh;
		qdma_set_txq_cngst_dynamic_threshold(qdma_data);
	} else if(pTxCngstCfg->txCngstMode == QDMA_TXCNGST_DYNAMIC_PEAKRATE_MARGIN){
		qdmaEnableTxqDynCngstEn(base);
		qdmaEnableTxRateMeterPeakRate(base);
		qdmaSetTxPeekRateMargin(base, pTxCngstCfg->peekRateMargin);
		qdmaSetTxPeekRateDuration(base, pTxCngstCfg->peekRateDuration>>4);
		qdma_data->qdma_private.dynCngstThrhPtr = &txqDynCngstThrh;
		qdma_set_txq_cngst_dynamic_threshold(qdma_data);	/*  notice that peekrate mode should set different threshold */
	}

#if SUPPORT_TXQ_DEI_DROP
	/* DEI buffer threshold init, half of normal threshold */
	qdmaSetTxqDEITotalMaxThrh(base, (txqDynCngstThrh.dynCngstTotalMaxThrh >> 1));
	qdmaSetTxqDEITotalMinThrh(base, (txqDynCngstThrh.dynCngstTotalMinThrh >> 1));
	qdmaSetTxqDEIChnlMaxThrh(base, (txqDynCngstThrh.dynCngstChnlMaxThrh >> 1));
	qdmaSetTxqDEIChnlMinThrh(base, (txqDynCngstThrh.dynCngstChnlMinThrh >> 1));
	qdmaSetTxqDEIQueueMaxThrh(base, (txqDynCngstThrh.dynCngstQueueMaxThrh >> 1));
	qdmaSetTxqDEIQueueMinThrh(base, (txqDynCngstThrh.dynCngstQueueMinThrh >> 1));
#endif

#if	SUPPORT_FAST_THR
	qdmaSetTxqDEITotalMaxThrh(base, BUFF_FAST_TOTAL_MAX_THRH);
	qdmaSetTxqDEITotalMinThrh(base, BUFF_FAST_TOTAL_MIN_THRH);
#endif
	return 0;
}


/******************************************************************************
 Descriptor:	It's used to configure dbg counter, which has 40 groups for EN7516, 64 for EN7580.
 Input Args:		- cntrIdx:
 				- cntrEn:
 				- cntrSrc:
 				- chnlIdx:
 				- queueIdx:
 				- dscpRingIdx:
 				- isChnlAll:
 				- isQueueAll:
 				- isDscpRingAll:
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_clear_and_set_dbg_cntr_info(QDMA_DBG_CNTR_T *dbgCntrPtr)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	unchar idx = dbgCntrPtr->cntrIdx;
	
	if(dbgCntrPtr->cntrIdx>=CONFIG_MAX_CNTR_NUM) {
		QDMA_ERR("Fault: dbg Cntr counter index shoule between 0 and %d\n", CONFIG_MAX_CNTR_NUM-1) ; 
		return -EINVAL ;
	}
	if(dbgCntrPtr->cntrSrc<0 || dbgCntrPtr->cntrSrc>=QDMA_DBG_CNTR_SRC_ITEMS) {
		QDMA_ERR("Fault: dbg Cntr source shoule between 0 and %d\n", QDMA_DBG_CNTR_SRC_ITEMS-1) ; 
		return -EINVAL ;
	}
	if((dbgCntrPtr->isChnlAll == QDMA_DISABLE) && (dbgCntrPtr->chnlIdx>=CONFIG_QDMA_CHANNEL)) {
		QDMA_ERR("Fault: Channel index shoule between 0 and %d\n", CONFIG_QDMA_CHANNEL-1) ; 
		return -EINVAL ;
	}
	if((dbgCntrPtr->isQueueAll == QDMA_DISABLE) && ( dbgCntrPtr->queueIdx>=CONFIG_QDMA_QUEUE)) {
		QDMA_ERR("Fault: Queue index shoule between 0 and %d\n", CONFIG_QDMA_QUEUE-1) ; 
		return -EINVAL ;
	}
	if((dbgCntrPtr->isDscpRingAll == QDMA_DISABLE) && (dbgCntrPtr->dscpRingIdx>=RX_RING_NUM)) {
		QDMA_ERR("Fault: ringIdx should be 0 ~ %d\n", (RX_RING_NUM-1)) ; 
		return -EINVAL ;
	}

	qdmaSetCntrSrc(base, idx, dbgCntrPtr->cntrSrc);
	qdmaSetCntrChnlIdx(base, idx, dbgCntrPtr->chnlIdx);
	qdmaSetCntrQueueIdx(base, idx, dbgCntrPtr->queueIdx);
	qdmaSetCntrDscpIdx(base, idx, dbgCntrPtr->dscpRingIdx);
	
	if(dbgCntrPtr->isChnlAll == QDMA_DISABLE) {
		qdmaSetCntrChnlSingle(base,idx);
	} else {
		qdmaSetCntrChnlAll(base,idx);
	}
	
	if(dbgCntrPtr->isQueueAll == QDMA_DISABLE) {
		qdmaSetCntrQueueSingle(base,idx);
	} else {
		qdmaSetCntrQueueAll(base,idx);
	}
	
	if(dbgCntrPtr->isDscpRingAll == QDMA_DISABLE) {
		qdmaSetCntrDscpSingle(base,idx);
	} else {
		qdmaSetCntrDscpAll(base,idx);
	}
	
	qdmaClearCntrCounter(base, idx);
	if(dbgCntrPtr->cntrEn == QDMA_DISABLE) {
		qdmaDisableCntrCfg(base, idx);
	} else {
		qdmaEnableCntrCfg(base, idx);
	}
	
	return 0 ;
}

int qdma_set_cntr_channel(struct ECNT_QDMA_Data *qdma_data)
{
#ifdef QDMA_LAN
    return 0;
#else
    uint channel_id = qdma_data->qdma_private.channel;
    QDMA_DBG_CNTR_T dbgCntr;
    if(16<=channel_id)
    {
        printk("the channel id_is must between 0 and 15  \n");
	    return 0;
	}
    dbgCntr.cntrEn = DBG_CNTR_ENABLE;
    dbgCntr.isChnlAll = 0;
    dbgCntr.isDscpRingAll = 1;
    dbgCntr.isQueueAll = 1;
    dbgCntr.queueIdx = 0 ;
    dbgCntr.dscpRingIdx = 0;
    dbgCntr.cntrIdx = channel_id;
    dbgCntr.chnlIdx= channel_id;
    dbgCntr.cntrSrc = DBG_CNTR_SRC_CPU_TX;
    qdma_clear_and_set_dbg_cntr_info(&dbgCntr);
    dbgCntr.cntrSrc = DBG_CNTR_SRC_FWD_TX;
    dbgCntr.cntrIdx = channel_id+DBG_CNTR_FWD_CHNL_GROUP_NUM;
    qdma_clear_and_set_dbg_cntr_info(&dbgCntr);
    return 0;
#endif

}






/*get trtcm enable/disable*/
int generalGetTrtcmMode(GENERAL_TrtcmModuleType_T trtcmModuleType){
	uint trtcmOffset =0;
	
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	qdma_dev_trtcm_cfg_init();
	
	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType >= TRTCM_MODE_MAX) )
		return -EINVAL;
	
	trtcmOffset = trtcmCfgBase[trtcmModuleType];

	return qdmaGetGeneralTrtcmMode(base, trtcmOffset);	
}

int qdma_general_check_index_valid(GENERAL_TrtcmModuleType_T trtcmModule, GENERAL_TrtcmMode_T trtcmMode , ushort index)
{
    unchar meterIdx = GET_METER_IDX(index);
    unchar meterGrp = GET_METER_GRP(index);
    unchar maxMeterIdx[3] = {GENERAL_INGRESS_INDEX_MAX,GENERAL_INGRESS_INDEX_MAX_GRP1,GENERAL_INGRESS_INDEX_MAX_GRP2};
    
    /*Global ratelimit: no index check*/
    if(trtcmModule == GLB_RATECTL){
        return 0 ;
    }
	/*check index range*/
	if( (trtcmModule == INGRESS_TRTCM) && (trtcmMode == TRTCM_RATELIMIT_MODE) ){
		if( meterIdx >= maxMeterIdx[meterGrp] ){ /*0~127*/
			QDMA_MSG(QDMA_DBG_WARN,"Fault: index should between 0 and %d\n", (maxMeterIdx[meterGrp]-1) ) ; 
			return -EINVAL ;
		}
	}else if( (trtcmModule == INGRESS_TRTCM) && (trtcmMode == TRTCM_MODE) ){
		if( meterIdx >= (maxMeterIdx[meterGrp]>>1) ){/*0~63*/
			QDMA_MSG(QDMA_DBG_WARN,"Fault: index should between 0 and %d\n", ((maxMeterIdx[meterGrp]>>1)-1) ) ; 
			return -EINVAL ;
		}
	}else if( trtcmModule == SLA_TRTCM ){
		if( meterIdx > GENERAL_SLA_INDEX_MAX ){/*SLA index 0~3 + queue 0~7 => max value is 0x1F*/
			QDMA_MSG(QDMA_DBG_WARN,"Fault: index should between 0 and %d\n", GENERAL_SLA_INDEX_MAX ) ; 
			return -EINVAL ;
		}
	}else if( trtcmModule == EGRESS_QUEUE_RATELIMIT ){
		if( meterIdx > QUEUE_ALL_NUM ){/*0~255*/
			QDMA_MSG(QDMA_DBG_WARN,"Fault: index should between 0 and %d\n", QUEUE_ALL_NUM ) ; 
			return -EINVAL ;
		}
	}else{/*Egress : the hardware fix TRTCM_MODE*/
		if( meterIdx > (CONFIG_QDMA_CHANNEL-1) ){/*LAN: 0~12 ; WAN: 0~31*/
			QDMA_MSG(QDMA_DBG_WARN,"Fault: index should between 0 and %d\n", (CONFIG_QDMA_CHANNEL-1) ) ; 
			return -EINVAL ;
		}
	}

	return 0 ;
}


static int __inline__ generalChecConfigDone(struct airoha_qdma *base, uint reg, uint doneBit)
{
	int RETRY = 3 ;
	volatile uint regValue = 0 ;
	
	while(RETRY--) {
		//regValue = IO_GREG(reg) ;
		regValue = airoha_qdma_rr(base, reg);
		
		if(regValue & doneBit) {
			break ;
		}
		mdelay(1) ;
	}
	if(RETRY < 0) {
		return -ETIME ;
	}

	return 0;
}

int generalGetTrtcmParaConfig(GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmParaType_T paraType, GENERAL_TrtcmRateType_T rateType, ushort index, uint *valueLo,uint *valueHi) {
	uint trtcmParaCfg = 0 ;
	uint trtcmOffset =0;
	
	struct airoha_qdma *base = &glb_eth->qdma[1];
	
	/*get trtcm config addr*/
	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType > EGRESS_TRTCM) )
		return -EINVAL;
	trtcmOffset = trtcmCfgBase[trtcmModuleType];

	trtcmParaCfg = (((paraType<<TRTCM_PARA_TYPE_SHIFT)&TRTCM_PARA_TYPE_MASK) |
				((GET_METER_IDX(index)<<TRTCM_PARA_IDX_INDEX_SHIFT)&TRTCM_PARA_IDX_INDEX_MASK) |
				((rateType<<TRTCM_PARA_IDX_RATE_TYPE_SHIFT)&TRTCM_PARA_IDX_RATE_TYPE_MASK)) ;

    #if SUPPORT_INGRESS_3X_RATELIMIT
    trtcmParaCfg |= (GET_METER_GRP(index)<<TRTCM_PARA_METER_GROUP_SHIFT);
    #endif

	//IO_SREG(TRTCM_PARAM_CFG(trtcmBase), trtcmParaCfg);
	airoha_qdma_wr(base, TRTCM_PARAM_CFG(trtcmOffset), trtcmParaCfg);
	
	if(generalChecConfigDone(base, TRTCM_PARAM_CFG(trtcmOffset), TRTCM_PARA_RWCMD_DONE) != 0){
		printk("Timeout for Get TRTCM configuration.\n") ;
		return -ETIME ;
	}

	//*valueLo = IO_GREG(TRTCM_DATA_LO(trtcmOffset));
	*valueLo = airoha_qdma_rr(base, TRTCM_DATA_LO(trtcmOffset));
	//*valueHi = IO_GREG(TRTCM_DATA_HI(trtcmOffset));
	*valueHi = airoha_qdma_rr(base, TRTCM_DATA_HI(trtcmOffset));
	return 0;
}

int generalGetTrtcmTickSel(GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmRateType_T rateType, ushort index){

	uint valueLo = 0, valueHi = 0;

	generalGetTrtcmParaConfig(trtcmModuleType,TRTCM_MISC, rateType, index, &valueLo, &valueHi);

	return  (valueLo & TRTCM_TICK_SEL);
}


/*get trtcm  ratelimit fasttick*/
int generalGetTrtcmFastTick(GENERAL_TrtcmModuleType_T trtcmModuleType){
	uint trtcmOffset =0;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	
	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType >= TRTCM_MODE_MAX) )
		return -EINVAL;
	trtcmOffset = trtcmCfgBase[trtcmModuleType]; 

	return qdmaGetGeneralFastTick(base, trtcmOffset) ;
}



/*get trtcm  ratelimit slow tick ratio*/
int generalGetTrtcmSlowTickRatio(GENERAL_TrtcmModuleType_T trtcmModuleType){
	uint trtcmOffset =0;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	
	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType >= TRTCM_MODE_MAX) )
		return -EINVAL;
	trtcmOffset = trtcmCfgBase[trtcmModuleType];

	return qdmaGetGeneralSlowRatio(base, trtcmOffset) ;
}

uint generalGetTrtcmSlowTick(GENERAL_TrtcmModuleType_T trtcmModuleType){
	ushort fastTick = 0;
	ushort slowTickRatio = 0;
	uint slowTick = 0;
	fastTick = generalGetTrtcmFastTick(trtcmModuleType);
	slowTickRatio = generalGetTrtcmSlowTickRatio(trtcmModuleType);

	slowTick = fastTick * slowTickRatio ;
	return  slowTick;
}


int generalGetTrtcmPktMode(GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmRateType_T rateType, ushort index){

	uint valueLo = 0, valueHi = 0;

	generalGetTrtcmParaConfig(trtcmModuleType,TRTCM_MISC, rateType, index, &valueLo, &valueHi);

	return  ((valueLo & TRTCM_PKT_MODE) >> 1);
}



int generalSetTrtcmParaConfig(GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmParaType_T paraType, GENERAL_TrtcmRateType_T rateType, ushort index, uint valueLo) {
	uint trtcmParaCfg = 0 ;
	ulong flags=0 ;
	uint valueLo_tmp = 0;
	uint valueHi_tmp = 0;
	uint trtcmOffset =0;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	/*get trtcm config addr*/
	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType > EGRESS_TRTCM) )
		return -EINVAL;
	trtcmOffset = trtcmCfgBase[trtcmModuleType];

	trtcmParaCfg = (TRTCM_PARA_RWCMD |
				((paraType<<TRTCM_PARA_TYPE_SHIFT)&TRTCM_PARA_TYPE_MASK) |
				((GET_METER_IDX(index)<<TRTCM_PARA_IDX_INDEX_SHIFT)&TRTCM_PARA_IDX_INDEX_MASK) |
				((rateType<<TRTCM_PARA_IDX_RATE_TYPE_SHIFT)&TRTCM_PARA_IDX_RATE_TYPE_MASK)) ;
    
    #if SUPPORT_INGRESS_3X_RATELIMIT
    trtcmParaCfg |= (GET_METER_GRP(index)<<TRTCM_PARA_METER_GROUP_SHIFT);
    #endif
	
	do{	
		spin_lock_irqsave(&qdma_config_lock, flags) ;
		//IO_SREG(TRTCM_DATA_LO(trtcmOffset), valueLo);
		airoha_qdma_wr(base, TRTCM_DATA_LO(trtcmOffset), valueLo);
#ifdef TCSUPPORT_CPU_ARMV8
		wmb();
#endif
		//IO_SREG(TRTCM_PARAM_CFG(trtcmOffset), trtcmParaCfg) ;
		airoha_qdma_wr(base, TRTCM_PARAM_CFG(trtcmOffset), trtcmParaCfg);
		spin_unlock_irqrestore(&qdma_config_lock, flags) ;
		generalGetTrtcmParaConfig(trtcmModuleType, paraType, rateType, index, &valueLo_tmp, &valueHi_tmp);
	}while(valueLo != valueLo_tmp);
	
	if( generalChecConfigDone(base, TRTCM_PARAM_CFG(trtcmOffset), TRTCM_PARA_RWCMD_DONE) != 0) {
		printk("Timeout for set TRTCM configuration.\n") ;
		return -ETIME ;
	}

	return 0;
}




int generalGetBucketSizeByRate(uint rateLimitValue)
{
	int bucksize = 0;

	if(rateLimitValue <= (METER_1K<<2))
	{
		bucksize = (64<<10);
	}
	else if(rateLimitValue <= (METER_1K<<3))
	{
		bucksize = (512<<10);
	}
	else if(rateLimitValue <= (METER_1K<<4))
	{
		bucksize = (METER_1M);
	}
	else if(rateLimitValue <= (METER_1K<<7))
	{
		bucksize = (METER_1M * 8);
	}
	else if(rateLimitValue <= (METER_1K<<8))
	{
		bucksize = (METER_1M * 24);
	}
	else
	{
		bucksize = (METER_1M * 64);
	}

	return bucksize;
}



/*
 tokenRate_integer = ratelimitvalue /unit 
 tokenRate_fraction =( ratelimtvalue % unit )
so should make sure the 2 params valid.
*/
int generalSetTrtcmTokenRate(GENERAL_TrtcmModuleType_T trtcmModuleType, uint rateLimitValue, GENERAL_TrtcmRateType_T rateType, ushort index){
	int bucketSize = 0;
	uint tokenRate = 0, tokenRate_integer = 0;
	ushort tokenRate_fraction = 0;
	int curTicksel = 0;
	ushort rateLimitUnit = 0 ;
	int min_bucket_size = 0;
#ifndef QDMA_LAN 
		min_bucket_size = 32768;
#else
		min_bucket_size = 4096;
#endif


	/*1. get tick sel*/
	if( TRTCM_FAST_TICK == generalGetTrtcmTickSel(trtcmModuleType, rateType, index) )/*fast tick mode*/
		curTicksel = generalGetTrtcmFastTick(trtcmModuleType) ;
	else/*slow tick mode*/
		curTicksel = generalGetTrtcmSlowTick(trtcmModuleType) ;

	/*2. get ratelimit mode, calculate Unit*/
	if(0 == curTicksel)
	{
		printk("tick = 0 , set error.\n");
		return -EINVAL;
	}
	else
	{
		if( TRTCM_BYTE_MODE == generalGetTrtcmPktMode(trtcmModuleType, rateType, index) )
			rateLimitUnit = 8000 / curTicksel ;  	/** 8bits X 1000 / (curTicksel X 10e-6 s)  kbps **/
		else
			rateLimitUnit = 1000000 / curTicksel ;  /** 1 / (curTicksel X 10e-6 s) pps **/
	}

	if( 0 == rateLimitUnit )
	{
		printk("rateLimitUnit = 0 , set error.\n");
		return -EINVAL;
	}

	/*3. calculate tokenRate*/
	tokenRate_integer = rateLimitValue / rateLimitUnit ;
	tokenRate_fraction =( rateLimitValue % rateLimitUnit) * 64 / rateLimitUnit ;

	if( (tokenRate_integer > 0x3FFFF) || (tokenRate_fraction > 0x3F) ){
		printk("tokenRate overflow.\n");
		return -EINVAL;
	}

	tokenRate = (tokenRate_integer << TRTCM_TOKEN_RATE_INTEGER_SHIFT) | tokenRate_fraction;

	if(generalSetTrtcmParaConfig(trtcmModuleType, TRTCM_TOKEN_RATE, rateType, index, tokenRate) < 0){
		printk("Set TRTCM Token Rate Failed.\n");
		return -EFAULT;
	}

	/*4. calculate bucketSize*/
	/*Egress & Global & SLA: bucketsize should be small to prevent burst*/
	/*Ingress: bucketsize should be bigger for TCP flow*/
	if(trtcmModuleType == INGRESS_TRTCM){
		if( TRTCM_BYTE_MODE == generalGetTrtcmPktMode(trtcmModuleType, rateType, index) )
			bucketSize = generalGetBucketSizeByRate(rateLimitValue);
		else
			bucketSize = rateLimitValue<<RATELIMIT_PKT_MODE_BUCKET_SHIFT;
	}else{
		bucketSize = tokenRate_integer+1;
		if(bucketSize < min_bucket_size)/* if bucketsize is lower than 4096, 1518Byte flow will lead to bucket overflow */
			bucketSize = min_bucket_size;
	}
    
	return bucketSize;

}


/*return the max bucketSize shift close to value*/
int biSearchGetBucketSizeShift(uint value, uint lo, uint hi, uint unit){

	int mid = 0;
	
	if(lo > hi )
		return -EINVAL;

	if((value > 0) && (value < unit))
		return 0;

	mid = (lo + hi) / 2;

	if((unit<<mid) == value){
		return mid;
	}else if((unit<<lo) == value){
		return lo;
	}else if((unit<<hi) == value){
		return hi;
	}else if((unit<<mid) > value){
		if((mid - lo) <= 1)
			return mid;
		
		return biSearchGetBucketSizeShift(value, lo, mid, unit);
	}else{
		if((hi - mid) <= 1)
			return hi;
		
		return biSearchGetBucketSizeShift(value, mid, hi, unit);
	}
	
}


int generalSetTrtcmBucketSize(GENERAL_TrtcmModuleType_T trtcmModuleType, uint bucketSize, GENERAL_TrtcmRateType_T rateType, ushort index){
	int bucketSize_shift = 0;
	uint bucketUnit = 0;

	/*max: 128Mbyte*/
	if(bucketSize > 0x8000000){
		return -EINVAL;	
	}

	if (TRTCM_BYTE_MODE == generalGetTrtcmPktMode(trtcmModuleType, rateType, index)){
		bucketUnit = trtcmBucketByteUnit[trtcmModuleType];
	}else{
		bucketUnit = trtcmBucketPacketUnit[trtcmModuleType];
	}

#if SUPPORT_BUCKETSIZE_BITWIDTH_17
	bucketSize_shift = biSearchGetBucketSizeShift(bucketSize, 0, 17, bucketUnit);
#else
	bucketSize_shift = biSearchGetBucketSizeShift(bucketSize, 0, 15, bucketUnit);
#endif
	
	if(generalSetTrtcmParaConfig(trtcmModuleType, TRTCM_BUCKETSIZE_SHIFT, rateType, index, bucketSize_shift) < 0){
		printk("Set TRTCM Bucket Size Failed.\n");
		return -EFAULT;
	}
	
	return 0;
}


int qdma_general_set_trtcm_mode_value(struct ECNT_QDMA_Data *qdma_data)
{
    #if SUPPORT_MTR_ACNT_IN_QDMA
    int bucketSize = 0;
	int ret = 0;
	GENERAL_TrtcmCbsPbsSet_T *generalTrtcmCbsPbsSetPtr = qdma_data->qdma_private.generalTrtcmCbsPbsSetPtr;

	/*1. check if the model works as trtcm mode*/
	if( TRTCM_RATELIMIT_MODE == generalGetTrtcmMode(generalTrtcmCbsPbsSetPtr->trtcmModule) )
	{
		QDMA_ERR("Fault: TRTCM mode is not enable.\n");
		return -EINVAL ;
	}

	/*2. check index range*/
	if( (ret = qdma_general_check_index_valid(generalTrtcmCbsPbsSetPtr->trtcmModule, TRTCM_MODE, generalTrtcmCbsPbsSetPtr->Index)) < 0 )
		return ret ;

	/*3. set CIR value*/
	if((bucketSize = generalSetTrtcmTokenRate(generalTrtcmCbsPbsSetPtr->trtcmModule, generalTrtcmCbsPbsSetPtr->CbsParamValue, TRTCM_COMMIT_MODE1, generalTrtcmCbsPbsSetPtr->Index)) < 0) {
		QDMA_ERR("Fault: set trtcm mode value error.\n");
		return bucketSize ;
	}
	if((ret = generalSetTrtcmBucketSize(generalTrtcmCbsPbsSetPtr->trtcmModule, bucketSize, TRTCM_COMMIT_MODE1, generalTrtcmCbsPbsSetPtr->Index))< 0) {
		QDMA_ERR("Fault: set trtcm mode value error.\n");
		return ret ;
	}

	/*4. set PIR value*/
	if((bucketSize = generalSetTrtcmTokenRate(generalTrtcmCbsPbsSetPtr->trtcmModule, generalTrtcmCbsPbsSetPtr->PbsParamValue, TRTCM_PEAK_MODE1, generalTrtcmCbsPbsSetPtr->Index)) < 0) {
		QDMA_ERR("Fault: set trtcm mode value error.\n");
		return bucketSize ;
	}
	if((ret = generalSetTrtcmBucketSize(generalTrtcmCbsPbsSetPtr->trtcmModule, bucketSize, TRTCM_PEAK_MODE1, generalTrtcmCbsPbsSetPtr->Index))< 0) {
		QDMA_ERR("Fault: set trtcm mode value error.\n");
		return ret ;
	}
    #endif

	return 0 ;
}



uint generalGetTrtcmBucketSize(GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmRateType_T rateType, ushort index){
	uint valueLo = 0, valueHi = 0;
	unchar bucketSize_shift = 0;
	uint bucketSize = 0;
    uint bucketUnit = 0;

    if (TRTCM_BYTE_MODE == generalGetTrtcmPktMode(trtcmModuleType, rateType, index)){
		bucketUnit = trtcmBucketByteUnit[trtcmModuleType];
	}else{
		bucketUnit = trtcmBucketPacketUnit[trtcmModuleType];
	}
    
	generalGetTrtcmParaConfig(trtcmModuleType, TRTCM_BUCKETSIZE_SHIFT, rateType, index, &valueLo, &valueHi);

	bucketSize_shift = valueLo & TRTCM_BUCKET_SIZE_SHIFT_MASK ;
	bucketSize = bucketUnit << bucketSize_shift ;
    
	return bucketSize;
}



uint generalGetTrtcmTokenRate(GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmRateType_T rateType, ushort index){
	uint tokenRate_integer = 0 , tokenRate_fraction = 0;
	uint valueLo = 0 , valueHi = 0;
	int curTicksel = 0;
	ushort rateLimitUnit = 0 ;

	/*1. get tick sel*/
	if( TRTCM_FAST_TICK == generalGetTrtcmTickSel(trtcmModuleType, rateType, index) )/*fast tick mode*/
		curTicksel = generalGetTrtcmFastTick(trtcmModuleType) ;
	else/*slow tick mode*/
		curTicksel = generalGetTrtcmSlowTick(trtcmModuleType) ;

	/*2. get ratelimit mode, calculate Unit*/
	if(0 == curTicksel)
	{
		printk("tick = 0 , get error.\n");
		return -EINVAL;
	}
	else
	{
		if( TRTCM_BYTE_MODE == generalGetTrtcmPktMode(trtcmModuleType, rateType, index) )
			rateLimitUnit = 8000 / curTicksel ; 	/** 8bits X 1000 / (curTicksel X 10e-6 s)  kbps **/
		else
			rateLimitUnit = 1000000 / curTicksel ;	/** 1 / (curTicksel X 10e-6 s) pps **/
	}

	if( 0 == rateLimitUnit )
	{
		printk("rateLimitUnit = 0 , get error.\n");
		return -EINVAL;
	}

	/*3.calculate ratelimit value*/
	generalGetTrtcmParaConfig(trtcmModuleType, TRTCM_TOKEN_RATE, rateType, index, &valueLo, &valueHi);
	tokenRate_integer = (valueLo &  TRTCM_TOKEN_RATE_INTEGER_MASK ) >> TRTCM_TOKEN_RATE_INTEGER_SHIFT;
	tokenRate_fraction = (valueLo & TRTCM_TOKEN_RATE_FRACTION_MASK );

	return	( (tokenRate_integer*rateLimitUnit) + (tokenRate_fraction*rateLimitUnit/64) );

}



int qdma_general_get_trtcm_mode_value(struct ECNT_QDMA_Data *qdma_data)
{
    #if SUPPORT_MTR_ACNT_IN_QDMA
	int ret = 0;
	GENERAL_TrtcmCbsPbsSet_T *generalTrtcmCbsPbsSetPtr = qdma_data->qdma_private.generalTrtcmCbsPbsSetPtr;

	/*1. check if the model works as trtcm mode*/
	if( TRTCM_RATELIMIT_MODE == generalGetTrtcmMode(generalTrtcmCbsPbsSetPtr->trtcmModule) )
	{
		QDMA_ERR("Fault: TRTCM mode is not enable.\n");
		return -EINVAL ;
	}

	/*2. check index range*/
	if( (ret = qdma_general_check_index_valid(generalTrtcmCbsPbsSetPtr->trtcmModule, TRTCM_MODE, generalTrtcmCbsPbsSetPtr->Index)) < 0 )
		return ret ;

	/*3. get CIR value*/
	generalTrtcmCbsPbsSetPtr->CbsParamValue = generalGetTrtcmTokenRate(generalTrtcmCbsPbsSetPtr->trtcmModule, TRTCM_COMMIT_MODE1, generalTrtcmCbsPbsSetPtr->Index);
	generalTrtcmCbsPbsSetPtr->CbsBucketSize = generalGetTrtcmBucketSize(generalTrtcmCbsPbsSetPtr->trtcmModule, TRTCM_COMMIT_MODE1, generalTrtcmCbsPbsSetPtr->Index);

	/*4. get PIR value*/
	generalTrtcmCbsPbsSetPtr->PbsParamValue = generalGetTrtcmTokenRate(generalTrtcmCbsPbsSetPtr->trtcmModule, TRTCM_PEAK_MODE1, generalTrtcmCbsPbsSetPtr->Index);
	generalTrtcmCbsPbsSetPtr->PbsBucketSize = generalGetTrtcmBucketSize(generalTrtcmCbsPbsSetPtr->trtcmModule, TRTCM_PEAK_MODE1, generalTrtcmCbsPbsSetPtr->Index);
    #endif
    
	return 0 ;

}



/*set chnl/ring/flow/etc trtcm enable/disable*/
int generalSetTrtcmMeterMode(GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmMeter_T meterMode, GENERAL_TrtcmRateType_T rateType, ushort index){
	uint valueLo = 0, valueHi = 0;

	if( (meterMode < GENERAL_METER_DISABLE) || (meterMode > GENERAL_METER_ENABLE)){
		printk("Trtcm Meter Mode should be 0 or 1.\n");
		return -EINVAL;
	}

	if(generalGetTrtcmParaConfig(trtcmModuleType, TRTCM_MISC, rateType, index, &valueLo,&valueHi) < 0){
		printk("Get TRTCM Para Config Failed.\n");
		return -EFAULT;
	}

	valueLo = (meterMode == GENERAL_METER_ENABLE) ? (valueLo|TRTCM_METER_MODE):(valueLo &(~TRTCM_METER_MODE)) ;
	
	if(generalSetTrtcmParaConfig(trtcmModuleType, TRTCM_MISC, rateType, index, valueLo) < 0){
		printk("Set TRTCM Para Config : %s Meter Enable Failed.\n",rateType ? "PIR" : "CIR");
		return -EFAULT;	
	}

	return 0;
}



int generalSetTrtcmPktMode(GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmPktMode_T pktMode, GENERAL_TrtcmRateType_T rateType, ushort index){
	uint valueLo = 0, valueHi = 0;

	if( (pktMode < TRTCM_BYTE_MODE) || (pktMode > TRTCM_PACKET_MODE) ){
		printk("Trtcm Packet Mode should be 0 or 1.\n");
		return -EINVAL;
	}

	if(generalGetTrtcmParaConfig(trtcmModuleType, TRTCM_MISC, rateType, index, &valueLo,&valueHi) < 0){
		printk("Get TRTCM Para Config Failed.\n");
		return -EFAULT;
	}

	valueLo = (pktMode == TRTCM_PACKET_MODE) ? (valueLo|TRTCM_PKT_MODE):(valueLo &(~TRTCM_PKT_MODE)) ;
	
	if(generalSetTrtcmParaConfig(trtcmModuleType, TRTCM_MISC, rateType, index, valueLo) < 0){
		printk("Set TRTCM Para Config : %s Packet Enable Failed.\n",rateType ? "PIR" : "CIR");
		return -EFAULT;	
	}
		
	return 0;
}


int generalSetTrtcmTickSel(GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmTickSel_T tickSel, GENERAL_TrtcmRateType_T rateType, ushort index){
	uint valueLo = 0, valueHi = 0;

	if( (tickSel < TRTCM_FAST_TICK) || (tickSel > TRTCM_SLOW_TICK) ){
		printk("Trtcm TickSel Index should be 0 or 1.\n");
		return -EINVAL;
	}

	if(generalGetTrtcmParaConfig(trtcmModuleType, TRTCM_MISC, rateType, index, &valueLo,&valueHi) < 0){
		printk("Get TRTCM Para Config Failed.\n");
		return -EFAULT;
	}

	valueLo = (tickSel == TRTCM_SLOW_TICK) ? (valueLo|TRTCM_TICK_SEL):(valueLo &(~TRTCM_TICK_SEL)) ;
			
	if(generalSetTrtcmParaConfig(trtcmModuleType, TRTCM_MISC, rateType, index, valueLo) < 0){
		printk("Set TRTCM Para Config : %s Tick Sel Failed.\n",rateType ? "PIR" : "CIR");
		return -EFAULT;	
	}

	return 0;

}



int qdma_general_set_trtcm_mode_cfg(struct ECNT_QDMA_Data *qdma_data)
{
    #if SUPPORT_MTR_ACNT_IN_QDMA
	int ret = 0;
	GENERAL_TrtcmCbsPbsCfg_T *generalTrtcmCbsPbsCfgPtr = qdma_data->qdma_private.generalTrtcmCbsPbsCfgPtr;

	/*1. check if the model works as trtcm mode*/
	if( TRTCM_RATELIMIT_MODE == generalGetTrtcmMode(generalTrtcmCbsPbsCfgPtr->trtcmModule) )
	{
		QDMA_ERR("Fault: TRTCM mode is not enable.\n");
		return -EINVAL ;
	}

	/*2. check index range*/
	if( (ret = qdma_general_check_index_valid(generalTrtcmCbsPbsCfgPtr->trtcmModule, TRTCM_MODE, generalTrtcmCbsPbsCfgPtr->Index)) < 0 )
		return ret ;

	/*3. set CIR cfg*/
	if((ret = generalSetTrtcmMeterMode( generalTrtcmCbsPbsCfgPtr->trtcmModule, generalTrtcmCbsPbsCfgPtr->CbsMeterEn, TRTCM_COMMIT_MODE1, generalTrtcmCbsPbsCfgPtr->Index )) < 0){
		QDMA_ERR("Fault:Set trtcm mode cfg error.\n") ; 
		return ret;
	}
	if((ret = generalSetTrtcmPktMode( generalTrtcmCbsPbsCfgPtr->trtcmModule, generalTrtcmCbsPbsCfgPtr->CbsPktMode, TRTCM_COMMIT_MODE1, generalTrtcmCbsPbsCfgPtr->Index )) < 0){
		QDMA_ERR("Fault:Set trtcm mode cfg error.\n") ; 
		return ret;
	}
	if((ret = generalSetTrtcmTickSel( generalTrtcmCbsPbsCfgPtr->trtcmModule, generalTrtcmCbsPbsCfgPtr->CbsTickSel, TRTCM_COMMIT_MODE1, generalTrtcmCbsPbsCfgPtr->Index )) < 0){
		QDMA_ERR("Fault:Set trtcm mode cfg error.\n") ; 
		return ret;
	}

	/*4. set PIR cfg*/
	if((ret = generalSetTrtcmMeterMode( generalTrtcmCbsPbsCfgPtr->trtcmModule, generalTrtcmCbsPbsCfgPtr->PbsMeterEn, TRTCM_PEAK_MODE1, generalTrtcmCbsPbsCfgPtr->Index )) < 0){
		QDMA_ERR("Fault:Set trtcm mode cfg error.\n") ; 
		return ret;
	}
	if((ret = generalSetTrtcmPktMode( generalTrtcmCbsPbsCfgPtr->trtcmModule, generalTrtcmCbsPbsCfgPtr->PbsPktMode, TRTCM_PEAK_MODE1, generalTrtcmCbsPbsCfgPtr->Index )) < 0){
		QDMA_ERR("Fault:Set trtcm mode cfg error.\n") ; 
		return ret;
	}
	if((ret = generalSetTrtcmTickSel( generalTrtcmCbsPbsCfgPtr->trtcmModule, generalTrtcmCbsPbsCfgPtr->PbsTickSel, TRTCM_PEAK_MODE1, generalTrtcmCbsPbsCfgPtr->Index )) < 0){
		QDMA_ERR("Fault:Set trtcm mode cfg error.\n") ; 
		return ret;
	}
    #endif

	return 0 ;
}
#if 0
int qdma_general_set_trtcm_mode_value(struct ECNT_QDMA_Data *qdma_data)
{
    #if SUPPORT_MTR_ACNT_IN_QDMA
    int bucketSize = 0;
	int ret = 0;
	GENERAL_TrtcmCbsPbsSet_T *generalTrtcmCbsPbsSetPtr = qdma_data->qdma_private.generalTrtcmCbsPbsSetPtr;

	/*1. check if the model works as trtcm mode*/
	if( TRTCM_RATELIMIT_MODE == generalGetTrtcmMode(generalTrtcmCbsPbsSetPtr->trtcmModule) )
	{
		QDMA_ERR("Fault: TRTCM mode is not enable.\n");
		return -EINVAL ;
	}

	/*2. check index range*/
	if( (ret = qdma_general_check_index_valid(generalTrtcmCbsPbsSetPtr->trtcmModule, TRTCM_MODE, generalTrtcmCbsPbsSetPtr->Index)) < 0 )
		return ret ;

	/*3. set CIR value*/
	if((bucketSize = generalSetTrtcmTokenRate(generalTrtcmCbsPbsSetPtr->trtcmModule, generalTrtcmCbsPbsSetPtr->CbsParamValue, TRTCM_COMMIT_MODE, generalTrtcmCbsPbsSetPtr->Index)) < 0) {
		QDMA_ERR("Fault: set trtcm mode value error.\n");
		return bucketSize ;
	}
	if((ret = generalSetTrtcmBucketSize(generalTrtcmCbsPbsSetPtr->trtcmModule, bucketSize, TRTCM_COMMIT_MODE, generalTrtcmCbsPbsSetPtr->Index))< 0) {
		QDMA_ERR("Fault: set trtcm mode value error.\n");
		return ret ;
	}

	/*4. set PIR value*/
	if((bucketSize = generalSetTrtcmTokenRate(generalTrtcmCbsPbsSetPtr->trtcmModule, generalTrtcmCbsPbsSetPtr->PbsParamValue, TRTCM_PEAK_MODE, generalTrtcmCbsPbsSetPtr->Index)) < 0) {
		QDMA_ERR("Fault: set trtcm mode value error.\n");
		return bucketSize ;
	}
	if((ret = generalSetTrtcmBucketSize(generalTrtcmCbsPbsSetPtr->trtcmModule, bucketSize, TRTCM_PEAK_MODE, generalTrtcmCbsPbsSetPtr->Index))< 0) {
		QDMA_ERR("Fault: set trtcm mode value error.\n");
		return ret ;
	}
    #endif

	return 0 ;

}
#endif

/*RxRing Ratelimit APIs*/
static inline void qdma_init_trtcm(void)
{
    trtcmCfgBase[INGRESS_TRTCM] = REG_INGRESS_TRTCM_CFG;
    trtcmBucketByteUnit[INGRESS_TRTCM] = 1024;
    trtcmBucketPacketUnit[INGRESS_TRTCM] = 16;
	
    trtcmCfgBase[SLA_TRTCM]= REG_SLA_TRTCM_CFG;
    trtcmBucketByteUnit[SLA_TRTCM] = 1024;
    trtcmBucketPacketUnit[SLA_TRTCM] = 16;
	
    trtcmCfgBase[EGRESS_TRTCM]=  REG_EGRESS_TRTCM_CFG;
    trtcmBucketByteUnit[EGRESS_TRTCM] = 1024;
    trtcmBucketPacketUnit[EGRESS_TRTCM] = 16;

    trtcmCfgBase[GLB_RATECTL]= REG_GLB_TRTCM_CFG;
    trtcmBucketByteUnit[GLB_RATECTL] = 256;
    trtcmBucketPacketUnit[GLB_RATECTL] = 256;

    return;
}

static int airoha_generalGetTrtcmMode(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType){
	uint offset =0;

	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType >= TRTCM_MODE_MAX) )
		return -EINVAL;
	offset = trtcmCfgBase[trtcmModuleType]; 

	return FIELD_GET(TRTCM_MODE_MASK, airoha_qdma_rr(qdma, offset));
}

static int generalGetRatelimitParaConfig(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType
	, GENERAL_TrtcmParaType_T paraType, ushort index, uint *valueLo,uint *valueHi) {
	uint trtcmParaCfg = 0 ;
	uint trtcmBase =0;
	u32 status;

	/*get trtcm config addr*/
	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType >= TRTCM_MODE_MAX) )
		return -EINVAL;
	trtcmBase = trtcmCfgBase[trtcmModuleType];

	trtcmParaCfg = (((paraType<<RATELIMIT_PARA_TYPE_SHIFT)&RATELIMIT_PARA_TYPE_MASK) |
				((QDMA_METER_IDX(index)<<RATELIMIT_PARA_IDX_INDEX_SHIFT)&RATELIMIT_PARA_IDX_INDEX_MASK)) ;

    trtcmParaCfg |= (QDMA_METER_GROUP(index)<<TRTCM_PARA_METER_GROUP_SHIFT);

	airoha_qdma_wr(qdma, TRTCM_PARAM_CFG(trtcmBase), trtcmParaCfg);
/*
	if(generalChecConfigDone(TRTCM_PARAM_CFG(trtcmBase), TRTCM_PARA_RWCMD_DONE) != 0){
		printk("Timeout for Get TRTCM configuration.\n") ;
		return -ETIME ;
	}
*/
	if(read_poll_timeout(airoha_qdma_rr, status,
					status & TRTCM_PARAM_RW_DONE_MASK,
					USEC_PER_MSEC, 10 * USEC_PER_MSEC,
					true, qdma,
					TRTCM_PARAM_CFG(trtcmBase)))
			return -ETIMEDOUT;

	*valueLo = airoha_qdma_rr(qdma, TRTCM_DATA_LO(trtcmBase));
	*valueHi = airoha_qdma_rr(qdma, TRTCM_DATA_HI(trtcmBase));
	
	return 0;
}
static int generalSetRatelimitParaConfig(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType
	, GENERAL_TrtcmParaType_T paraType, ushort index, uint valueLo) {
	uint trtcmParaCfg = 0 ;
	uint trtcmBase =0;
	ulong flags=0 ;
	uint valueLo_tmp = 0;
	uint valueHi_tmp = 0;
	u32 status;

	/*get trtcm config addr*/
	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType >= TRTCM_MODE_MAX) )
		return -EINVAL;
	trtcmBase = trtcmCfgBase[trtcmModuleType];

	trtcmParaCfg = (RATELIMIT_PARA_RWCMD |
				((paraType<<RATELIMIT_PARA_TYPE_SHIFT)&RATELIMIT_PARA_TYPE_MASK) |
				((QDMA_METER_IDX(index)<<RATELIMIT_PARA_IDX_INDEX_SHIFT)&RATELIMIT_PARA_IDX_INDEX_MASK));

    trtcmParaCfg |= (QDMA_METER_GROUP(index)<<TRTCM_PARA_METER_GROUP_SHIFT);
	
	do{	
		spin_lock_irqsave(&qdma_cfg_lock, flags) ;
		airoha_qdma_wr(qdma, TRTCM_DATA_LO(trtcmBase), valueLo);
		wmb();
		airoha_qdma_wr(qdma, TRTCM_PARAM_CFG(trtcmBase), trtcmParaCfg) ;
		spin_unlock_irqrestore(&qdma_cfg_lock, flags) ;
	
		generalGetRatelimitParaConfig(qdma, trtcmModuleType, paraType, index, &valueLo_tmp, &valueHi_tmp);
	}while(valueLo != valueLo_tmp);
/*
	if( generalChecConfigDone(TRTCM_PARAM_CFG(trtcmBase), TRTCM_PARA_RWCMD_DONE) != 0) {
		printk("Timeout for set TRTCM configuration.\n") ;
		return -ETIME ;
	}
*/
	if(read_poll_timeout(airoha_qdma_rr, status,
					status & TRTCM_PARAM_RW_DONE_MASK,
					USEC_PER_MSEC, 10 * USEC_PER_MSEC,
					true, qdma,
					TRTCM_PARAM_CFG(trtcmBase)))
			return -ETIMEDOUT;

	return 0;
}


/*set chnl/ring/flow/etc trtcm enable/disable*/
static int generalSetRatelimitMeterMode(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType
, GENERAL_TrtcmMeter_T meterMode, ushort index){
	uint valueLo = 0, valueHi = 0;

	if( (meterMode < GENERAL_METER_DISABLE) || (meterMode > GENERAL_METER_ENABLE)){
		pr_err("Trtcm Meter Mode should be 0 or 1.\n");
		return -EINVAL;
	}

	if(generalGetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_MISC, index, &valueLo, &valueHi) < 0){
		pr_err("Get TRTCM Para Config Failed.\n");
		return -EFAULT;
	}

	valueLo = (meterMode == GENERAL_METER_ENABLE) ? (valueLo|TRTCM_METER_MODE):(valueLo &(~TRTCM_METER_MODE)) ;
	
	if(generalSetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_MISC, index, valueLo) < 0){
		pr_err("Set TRTCM Para Config : Ratelimit Meter Enable Failed.\n");
		return -EFAULT;	
	}

	return 0;
}



static int generalSetRatelimitPktMode(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmPktMode_T pktMode, ushort index){
	uint valueLo = 0, valueHi = 0;

	if( (pktMode < TRTCM_BYTE_MODE) || (pktMode > TRTCM_PACKET_MODE) ){
		printk("Trtcm Packet Mode should be 0 or 1.\n");
		return -EINVAL;
	}

	if(generalGetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_MISC, index, &valueLo, &valueHi) < 0){
		printk("Get TRTCM Para Config Failed.\n");
		return -EFAULT;
	}

	valueLo = (pktMode == TRTCM_PACKET_MODE) ? (valueLo|TRTCM_PKT_MODE):(valueLo &(~TRTCM_PKT_MODE)) ;
	
	if(generalSetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_MISC, index, valueLo) < 0){
		printk("Set TRTCM Para Config : Ratelimit Packet Enable Failed.\n");
		return -EFAULT;	
	}
		
	return 0;
}

static int generalSetRatelimitTickSel(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmTickSel_T tickSel, ushort index){
	uint valueLo = 0, valueHi = 0;

	if( (tickSel < TRTCM_FAST_TICK) || (tickSel > TRTCM_SLOW_TICK) ){
		printk("Trtcm TickSel Index should be 0 or 1.\n");
		return -EINVAL;
	}

	if(generalGetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_MISC, index, &valueLo, &valueHi) < 0){
		printk("Get TRTCM Para Config Failed.\n");
		return -EFAULT;
	}

	valueLo = (tickSel == TRTCM_SLOW_TICK) ? (valueLo|TRTCM_TICK_SEL):(valueLo &(~TRTCM_TICK_SEL)) ;
			
	if(generalSetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_MISC, index, valueLo) < 0){
		printk("Set TRTCM Para Config : Ratelimit Tick Sel Failed.\n");
		return -EFAULT;	
	}

	return 0;
}

static int airoha_generalGetTrtcmParaConfig(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmParaType_T paraType, enum trtcm_mode_type rateType, ushort index, uint *valueLo,uint *valueHi) {
	uint trtcmParaCfg = 0 ;
	uint trtcmBase =0;
	u32 status;

	/*get trtcm config addr*/
	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType > EGRESS_TRTCM) )
		return -EINVAL;
	trtcmBase = trtcmCfgBase[trtcmModuleType];

	trtcmParaCfg = (((paraType<<TRTCM_PARA_TYPE_SHIFT)&TRTCM_PARA_TYPE_MASK) |
				((QDMA_METER_IDX(index)<<TRTCM_PARA_IDX_INDEX_SHIFT)&TRTCM_PARA_IDX_INDEX_MASK) |
				((rateType<<TRTCM_PARA_IDX_RATE_TYPE_SHIFT)&TRTCM_PARA_IDX_RATE_TYPE_MASK)) ;

    trtcmParaCfg |= (QDMA_METER_GROUP(index)<<TRTCM_PARA_METER_GROUP_SHIFT);

	airoha_qdma_wr(qdma, TRTCM_PARAM_CFG(trtcmBase), trtcmParaCfg);
	if(read_poll_timeout(airoha_qdma_rr, status,
					status & TRTCM_PARAM_RW_DONE_MASK,
					USEC_PER_MSEC, 10 * USEC_PER_MSEC,
					true, qdma,
					TRTCM_PARAM_CFG(trtcmBase))){
		printk("Timeout for Get TRTCM configuration.\n") ;
		return -ETIME ;
	}

	*valueLo = airoha_qdma_rr(qdma, TRTCM_DATA_LO(trtcmBase));
	*valueHi = airoha_qdma_rr(qdma, TRTCM_DATA_HI(trtcmBase));
	
	return 0;
}
static int airoha_generalSetTrtcmParaConfig(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmParaType_T paraType, enum trtcm_mode_type rateType, ushort index, uint valueLo) {
	uint trtcmParaCfg = 0 ;
	uint trtcmBase =0;
	ulong flags=0 ;
	uint valueLo_tmp = 0;
	uint valueHi_tmp = 0;
	u32 status;
	/*get trtcm config addr*/
	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType > EGRESS_TRTCM) )
		return -EINVAL;
	trtcmBase = trtcmCfgBase[trtcmModuleType];

	trtcmParaCfg = (TRTCM_PARA_RWCMD |
				((paraType<<TRTCM_PARA_TYPE_SHIFT)&TRTCM_PARA_TYPE_MASK) |
				((QDMA_METER_IDX(index)<<TRTCM_PARA_IDX_INDEX_SHIFT)&TRTCM_PARA_IDX_INDEX_MASK) |
				((rateType<<TRTCM_PARA_IDX_RATE_TYPE_SHIFT)&TRTCM_PARA_IDX_RATE_TYPE_MASK)) ;

    trtcmParaCfg |= (QDMA_METER_GROUP(index)<<TRTCM_PARA_METER_GROUP_SHIFT);
	
	do{	
		spin_lock_irqsave(&qdma_cfg_lock, flags) ;
		airoha_qdma_wr(qdma, TRTCM_DATA_LO(trtcmBase), valueLo);
		wmb();
		airoha_qdma_wr(qdma, TRTCM_PARAM_CFG(trtcmBase), trtcmParaCfg) ;
		spin_unlock_irqrestore(&qdma_cfg_lock, flags) ;
		airoha_generalGetTrtcmParaConfig(qdma, trtcmModuleType, paraType, rateType, index, &valueLo_tmp, &valueHi_tmp);
	}while(valueLo != valueLo_tmp);
	
	if(read_poll_timeout(airoha_qdma_rr, status,
					status & TRTCM_PARAM_RW_DONE_MASK,
					USEC_PER_MSEC, 10 * USEC_PER_MSEC,
					true, qdma,
					TRTCM_PARAM_CFG(trtcmBase))) {
		printk("Timeout for set TRTCM configuration.\n") ;
		return -ETIME ;
	}

	return 0;
}

/*set chnl/ring/flow/etc trtcm enable/disable*/
static int airoha_generalSetTrtcmMeterMode(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmMeter_T meterMode, enum trtcm_mode_type rateType, ushort index){
	uint valueLo = 0, valueHi = 0;

	if( (meterMode < GENERAL_METER_DISABLE) || (meterMode > GENERAL_METER_ENABLE)){
		printk("Trtcm Meter Mode should be 0 or 1.\n");
		return -EINVAL;
	}

	if(airoha_generalGetTrtcmParaConfig(qdma, trtcmModuleType, TRTCM_MISC, rateType, index, &valueLo,&valueHi) < 0){
		printk("Get TRTCM Para Config Failed.\n");
		return -EFAULT;
	}

	valueLo = (meterMode == GENERAL_METER_ENABLE) ? (valueLo|TRTCM_METER_MODE):(valueLo &(~TRTCM_METER_MODE)) ;
	
	if(airoha_generalSetTrtcmParaConfig(qdma, trtcmModuleType, TRTCM_MISC, rateType, index, valueLo) < 0){
		printk("Set TRTCM Para Config : %s Meter Enable Failed.\n",rateType ? "PIR" : "CIR");
		return -EFAULT;	
	}

	return 0;
}


static int airoha_generalSetTrtcmPktMode(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmPktMode_T pktMode, enum trtcm_mode_type rateType, ushort index){
	uint valueLo = 0, valueHi = 0;

	if( (pktMode < TRTCM_BYTE_MODE) || (pktMode > TRTCM_PACKET_MODE) ){
		printk("Trtcm Packet Mode should be 0 or 1.\n");
		return -EINVAL;
	}

	if(airoha_generalGetTrtcmParaConfig(qdma, trtcmModuleType, TRTCM_MISC, rateType, index, &valueLo,&valueHi) < 0){
		printk("Get TRTCM Para Config Failed.\n");
		return -EFAULT;
	}

	valueLo = (pktMode == TRTCM_PACKET_MODE) ? (valueLo|TRTCM_PKT_MODE):(valueLo &(~TRTCM_PKT_MODE)) ;
	
	if(airoha_generalSetTrtcmParaConfig(qdma, trtcmModuleType, TRTCM_MISC, rateType, index, valueLo) < 0){
		printk("Set TRTCM Para Config : %s Packet Enable Failed.\n",rateType ? "PIR" : "CIR");
		return -EFAULT;	
	}
		
	return 0;
}

static int airoha_generalSetTrtcmTickSel(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmTickSel_T tickSel, enum trtcm_mode_type rateType, ushort index){
	uint valueLo = 0, valueHi = 0;

	if( (tickSel < TRTCM_FAST_TICK) || (tickSel > TRTCM_SLOW_TICK) ){
		printk("Trtcm TickSel Index should be 0 or 1.\n");
		return -EINVAL;
	}

	if(airoha_generalGetTrtcmParaConfig(qdma, trtcmModuleType, TRTCM_MISC, rateType, index, &valueLo,&valueHi) < 0){
		printk("Get TRTCM Para Config Failed.\n");
		return -EFAULT;
	}

	valueLo = (tickSel == TRTCM_SLOW_TICK) ? (valueLo|TRTCM_TICK_SEL):(valueLo &(~TRTCM_TICK_SEL)) ;
			
	if(airoha_generalSetTrtcmParaConfig(qdma, trtcmModuleType, TRTCM_MISC, rateType, index, valueLo) < 0){
		printk("Set TRTCM Para Config : %s Tick Sel Failed.\n",rateType ? "PIR" : "CIR");
		return -EFAULT;	
	}

	return 0;

}

static int airoha_qdma_general_set_ratelimit_mode_cfg(struct airoha_qdma *qdma, GENERAL_TrtcmRatelimitCfg_T rxRateLimitCfg)
{
	int ret=0;
	enum trtcm_mode_type rateType;
	GENERAL_TrtcmMode_T trtcmMode = airoha_generalGetTrtcmMode(qdma, rxRateLimitCfg.trtcmModule);

    /*no trtcm mode for mtr_grp1&mtr_grp2*/
    if(QDMA_METER_GROUP(rxRateLimitCfg.Index) > 0){
        trtcmMode = TRTCM_RATELIMIT_MODE;
	}

	/*1. check index range*/
	if( (ret = qdma_general_check_index_valid(rxRateLimitCfg.trtcmModule, trtcmMode, rxRateLimitCfg.Index)) < 0 ){
  	pr_err("Invalid meter index: %d\n", rxRateLimitCfg.Index);
          return ret ;}

	/*2. set cfg*/
	if( trtcmMode == TRTCM_RATELIMIT_MODE ){/*ratelimit mode, set just one index*/
		if((ret = generalSetRatelimitMeterMode(qdma, rxRateLimitCfg.trtcmModule, rxRateLimitCfg.MeterEn, rxRateLimitCfg.Index )) < 0){
			pr_err("Fault:Set ratelimit mode cfg error.\n") ; 
			return ret;
		}
		if((ret = generalSetRatelimitPktMode(qdma, rxRateLimitCfg.trtcmModule, rxRateLimitCfg.PktMode, rxRateLimitCfg.Index )) < 0){
			pr_err("Fault:Set ratelimit mode cfg error.\n") ; 
			return ret;
		}
		if((ret = generalSetRatelimitTickSel(qdma, rxRateLimitCfg.trtcmModule, rxRateLimitCfg.TickSel, rxRateLimitCfg.Index )) < 0){
			pr_err("Fault:Set ratelimit mode cfg error.\n") ; 
			return ret;
		}
	}else{/*trtcm mode, set CIR & PIR the cfg*/
		for( rateType = TRTCM_COMMIT_MODE ; rateType <= TRTCM_PEAK_MODE ; rateType++ ){
			if((ret = airoha_generalSetTrtcmMeterMode(qdma, rxRateLimitCfg.trtcmModule, rxRateLimitCfg.MeterEn, rateType , rxRateLimitCfg.Index )) < 0){
				pr_err("Fault:Set ratelimit mode cfg error.\n") ; 
				return ret;
			}
			if((ret = airoha_generalSetTrtcmPktMode(qdma, rxRateLimitCfg.trtcmModule, rxRateLimitCfg.PktMode, rateType ,rxRateLimitCfg.Index )) < 0){
				pr_err("Fault:Set ratelimit mode cfg error.\n") ; 
				return ret;
			}
			if((ret = airoha_generalSetTrtcmTickSel(qdma, rxRateLimitCfg.trtcmModule, rxRateLimitCfg.TickSel, rateType ,rxRateLimitCfg.Index )) < 0){
				pr_err("Fault:Set ratelimit mode cfg error.\n") ; 
				return ret;
			}
		}
	}
	
	return 0 ;
}
/*set trtcm mode: TRTCM mode or Ratelimit Mode*/
static int generalSetTrtcmMode(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmMode_T trtcmMode){
	uint trtcmBase =0;

	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType >= TRTCM_MODE_MAX) )
		return -EINVAL;
	trtcmBase = trtcmCfgBase[trtcmModuleType];
	
	if( (trtcmMode < TRTCM_RATELIMIT_MODE) || (trtcmMode > TRTCM_MODE) )
		return -EINVAL;
/*
	qdmaSetGeneralTrtcmMode(trtcmBase,trtcmMode);
*/
	airoha_qdma_rmw(qdma, trtcmBase,
			TRTCM_MODE_MASK, FIELD_PREP(TRTCM_MODE_MASK, trtcmMode));

	return 0;
}
static int airoha_generalGetTrtcmPktMode(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, enum trtcm_mode_type rateType, ushort index){

	uint valueLo = 0, valueHi = 0;

	airoha_generalGetTrtcmParaConfig(qdma, trtcmModuleType,TRTCM_MISC, rateType, index, &valueLo, &valueHi);

	return  ((valueLo & TRTCM_PKT_MODE) >> 1);
}
#if 0
static /*return the max bucketSize shift close to value*/
int biSearchGetBucketSizeShift(uint value, uint lo, uint hi, uint unit){

	int mid = 0;
	
	if(lo > hi )
		return -EINVAL;

	if((value > 0) && (value < unit))
		return 0;

	mid = (lo + hi) / 2;

	if((unit<<mid) == value){
		return mid;
	}else if((unit<<lo) == value){
		return lo;
	}else if((unit<<hi) == value){
		return hi;
	}else if((unit<<mid) > value){
		if((mid - lo) <= 1)
			return mid;
		
		return biSearchGetBucketSizeShift(value, lo, mid, unit);
	}else{
		if((hi - mid) <= 1)
			return hi;
		
		return biSearchGetBucketSizeShift(value, mid, hi, unit);
	}
}
#endif
static int airoha_generalSetTrtcmBucketSize(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, uint bucketSize, enum trtcm_mode_type rateType, ushort index){
	int bucketSize_shift = 0;
	uint bucketUnit = 0;

	/*max: 128Mbyte*/
	if(bucketSize > 0x8000000){
		return -EINVAL;	
	}

	if ((int)TRTCM_BYTE_MODE == airoha_generalGetTrtcmPktMode(qdma, trtcmModuleType, rateType, index)){
		bucketUnit = trtcmBucketByteUnit[trtcmModuleType];
	}else{
		bucketUnit = trtcmBucketPacketUnit[trtcmModuleType];
	}

	bucketSize_shift = biSearchGetBucketSizeShift(bucketSize, 0, 17, bucketUnit);
	
	if(airoha_generalSetTrtcmParaConfig(qdma, trtcmModuleType, TRTCM_BUCKETSIZE_SHIFT, rateType, index, bucketSize_shift) < 0){
		printk("Set TRTCM Bucket Size Failed.\n");
		return -EFAULT;
	}
	
	return 0;
}

static int airoha_generalGetTrtcmTickSel(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, enum trtcm_mode_type rateType, ushort index){

	uint valueLo = 0, valueHi = 0;

	airoha_generalGetTrtcmParaConfig(qdma, trtcmModuleType,TRTCM_MISC, rateType, index, &valueLo, &valueHi);

	return  (valueLo & TRTCM_TICK_SEL);
}
/*get trtcm  ratelimit fasttick*/
static int airoha_generalGetTrtcmFastTick(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType){
	uint trtcmBase =0;

	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType >= TRTCM_MODE_MAX) )
		return -EINVAL;
	trtcmBase = trtcmCfgBase[trtcmModuleType]; 

	return FIELD_GET(TRTCM_FAST_TICK_MASK, airoha_qdma_rr(qdma, trtcmBase));
}
/*get trtcm  ratelimit slow tick ratio*/
static ushort airoha_generalGetTrtcmSlowTickRatio(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType){
	uint trtcmBase =0;

	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType >= TRTCM_MODE_MAX) )
		return -EINVAL;
	trtcmBase = trtcmCfgBase[trtcmModuleType];

	return FIELD_GET(TRTCM_SLOW_TICKRATIO_MASK, airoha_qdma_rr(qdma, trtcmBase));
}
static uint airoha_generalGetTrtcmSlowTick(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType){
	ushort fastTick = 0;
	ushort slowTickRatio = 0;
	uint slowTick = 0;
	fastTick = airoha_generalGetTrtcmFastTick(qdma, trtcmModuleType);
	slowTickRatio = airoha_generalGetTrtcmSlowTickRatio(qdma, trtcmModuleType);

	slowTick = fastTick * slowTickRatio ;
	return  slowTick;
}
#if 0
static int generalGetBucketSizeByRate(uint rateLimitValue)
{
	int bucksize = 0;

	if(rateLimitValue <= (METER_1K<<2))
	{
		bucksize = (64<<10);
	}
	else if(rateLimitValue <= (METER_1K<<3))
	{
		bucksize = (512<<10);
	}
	else if(rateLimitValue <= (METER_1K<<4))
	{
		bucksize = (METER_1M);
	}
	else if(rateLimitValue <= (METER_1K<<7))
	{
		bucksize = (METER_1M * 8);
	}
	else if(rateLimitValue <= (METER_1K<<8))
	{
		bucksize = (METER_1M * 24);
	}
	else
	{
		bucksize = (METER_1M * 64);
	}

	return bucksize;
}
#endif
/*
 tokenRate_integer = ratelimitvalue /unit 
 tokenRate_fraction =( ratelimtvalue % unit )
so should make sure the 2 params valid.
*/
static int airoha_generalSetTrtcmTokenRate(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, uint rateLimitValue, enum trtcm_mode_type rateType, ushort index){
	uint tokenRate,bucketSize,tokenRate_integer = 0;
	ushort tokenRate_fraction = 0;
	int curTicksel = 0;
	ushort rateLimitUnit = 0 ;

	/*1. get tick sel*/
	if( (int)TRTCM_FAST_TICK == airoha_generalGetTrtcmTickSel(qdma, trtcmModuleType, rateType, index) )/*fast tick mode*/
		curTicksel = airoha_generalGetTrtcmFastTick(qdma, trtcmModuleType) ;
	else/*slow tick mode*/
		curTicksel = airoha_generalGetTrtcmSlowTick(qdma, trtcmModuleType) ;

	/*2. get ratelimit mode, calculate Unit*/
	if(0 == curTicksel)
	{
		printk("tick = 0 , set error.\n");
		return -EINVAL;
	}
	else
	{
		if( (int)TRTCM_BYTE_MODE == airoha_generalGetTrtcmPktMode(qdma, trtcmModuleType, rateType, index) )
			rateLimitUnit = 8000 / curTicksel ;  	/** 8bits X 1000 / (curTicksel X 10e-6 s)  kbps **/
		else
			rateLimitUnit = 1000000 / curTicksel ;  /** 1 / (curTicksel X 10e-6 s) pps **/
	}

	if( 0 == rateLimitUnit )
	{
		printk("rateLimitUnit = 0 , set error.\n");
		return -EINVAL;
	}

	/*3. calculate tokenRate*/
	tokenRate_integer = rateLimitValue / rateLimitUnit ;
	tokenRate_fraction =( rateLimitValue % rateLimitUnit) * 64 / rateLimitUnit ;

	if( (tokenRate_integer > 0x3FFFF) || (tokenRate_fraction > 0x3F) ){
		printk("tokenRate overflow.\n");
		return -EINVAL;
	}

	tokenRate = (tokenRate_integer << TRTCM_TOKEN_RATE_INTEGER_SHIFT) | tokenRate_fraction;

	if(airoha_generalSetTrtcmParaConfig(qdma, trtcmModuleType, TRTCM_TOKEN_RATE, rateType, index, tokenRate) < 0){
		printk("Set TRTCM Token Rate Failed.\n");
		return -EFAULT;
	}

	/*4. calculate bucketSize*/
	/*Egress & Global & SLA: bucketsize should be small to prevent burst*/
	/*Ingress: bucketsize should be bigger for TCP flow*/
	if(trtcmModuleType == INGRESS_TRTCM){
		if( (int)TRTCM_BYTE_MODE == airoha_generalGetTrtcmPktMode(qdma, trtcmModuleType, rateType, index) )
			bucketSize = generalGetBucketSizeByRate(rateLimitValue);
		else
			bucketSize = rateLimitValue<<RATELIMIT_PKT_MODE_BUCKET_SHIFT;
	}else{
		bucketSize = tokenRate_integer+1;
		if(bucketSize < 4096)/* if bucketsize is lower than 4096, 1518Byte flow will lead to bucket overflow */
			bucketSize = 4096;
	}
    
	return bucketSize;

}
uint airoha_generalGetTrtcmTokenRate(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType,enum trtcm_mode_type rateType, ushort index){
	uint valueLo = 0, valueHi = 0;
	uint tokenRate_integer = 0;
	ushort tokenRate_fraction = 0;
	int curTicksel = 0;
	ushort rateLimitUnit = 0 ;
	/*1. get tick sel*/
	if((int)TRTCM_FAST_TICK == airoha_generalGetTrtcmTickSel(qdma, trtcmModuleType, rateType, index) )/*fast tick mode*/
		curTicksel = airoha_generalGetTrtcmFastTick(qdma, trtcmModuleType) ;
	else/*slow tick mode*/
		curTicksel = airoha_generalGetTrtcmSlowTick(qdma, trtcmModuleType) ;

	/*2. get ratelimit mode, calculate Unit*/
	if(0 == curTicksel)
	{
		printk("tick = 0 , set error.\n");
		return -EINVAL;
	}
	else
	{
		if((int) TRTCM_BYTE_MODE == airoha_generalGetTrtcmPktMode(qdma, trtcmModuleType, rateType, index) )
			rateLimitUnit = 8000 / curTicksel ;  	/** 8bits X 1000 / (curTicksel X 10e-6 s)  kbps **/
		else
			rateLimitUnit = 1000000 / curTicksel ;  /** 1 / (curTicksel X 10e-6 s) pps **/
	}

	if( 0 == rateLimitUnit )
	{
		printk("rateLimitUnit = 0 , set error.\n");
		return -EINVAL;
	}

	

	if( 0 == rateLimitUnit )
	{
		printk("rateLimitUnit = 0 , get error.\n");
		return -EINVAL;
	}

	/*3.calculate ratelimit value*/
	generalGetRatelimitParaConfig(qdma,trtcmModuleType, TRTCM_TOKEN_RATE, index, &valueLo, &valueHi);
	tokenRate_integer = (valueLo &  TRTCM_TOKEN_RATE_INTEGER_MASK ) >> TRTCM_TOKEN_RATE_INTEGER_SHIFT;
	tokenRate_fraction = (valueLo & TRTCM_TOKEN_RATE_FRACTION_MASK );

	return	( (tokenRate_integer*rateLimitUnit) + (tokenRate_fraction*rateLimitUnit/64) );
}

static int generalGetRatelimitPktMode(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, ushort index){

	uint valueLo = 0, valueHi = 0;

	generalGetRatelimitParaConfig(qdma, trtcmModuleType,TRTCM_MISC, index, &valueLo, &valueHi);

	return  ((valueLo & TRTCM_PKT_MODE) >> 1);
}
static int generalSetRatelimitBucketSize(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, uint bucketSize, ushort index){
	int bucketSize_shift = 0;
    uint bucketUnit = 0;

    /*max: 128Mbyte*/
	if(bucketSize > 0x8000000){
		return -EINVAL;	
	}

	if (TRTCM_BYTE_MODE == generalGetRatelimitPktMode(qdma, trtcmModuleType, index)){
		bucketUnit = trtcmBucketByteUnit[trtcmModuleType];
	}else{
		bucketUnit = trtcmBucketPacketUnit[trtcmModuleType];
	}

	bucketSize_shift = biSearchGetBucketSizeShift(bucketSize, 0, 17, bucketUnit);
	
	if(generalSetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_BUCKETSIZE_SHIFT, index, bucketSize_shift) < 0){
		printk("Set TRTCM Bucket Size Failed.\n");
		return -EFAULT;
	}
	
	return 0;
}
static int generalGetRatelimitTickSel(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, ushort index){

	uint valueLo = 0, valueHi = 0;

	generalGetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_MISC, index, &valueLo, &valueHi);

	return  (valueLo & TRTCM_TICK_SEL);
}
/*
 tokenRate_integer = ratelimitvalue /unit 
 tokenRate_fraction =( ratelimtvalue % unit )
so should make sure the 2 params valid.
*/
static int generalSetRatelimitTokenRate(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, uint rateLimitValue, ushort index){
	int bucketSize = 0;
	uint tokenRate = 0, tokenRate_integer = 0;
	ushort tokenRate_fraction = 0;
	int curTicksel = 0;
	ushort rateLimitUnit = 0 ;

	/*1. get tick sel*/
	if( TRTCM_FAST_TICK == generalGetRatelimitTickSel(qdma, trtcmModuleType, index) )/*fast tick mode*/
		curTicksel = airoha_generalGetTrtcmFastTick(qdma, trtcmModuleType) ;
	else/*slow tick mode*/
		curTicksel = airoha_generalGetTrtcmSlowTick(qdma, trtcmModuleType) ;

	/*2. get ratelimit mode, calculate Unit*/
	if(0 == curTicksel)
	{
		printk("tick = 0 , set error.\n");
		return -EINVAL;
	}
	else
	{
		if( TRTCM_BYTE_MODE == generalGetRatelimitPktMode(qdma, trtcmModuleType, index) )
			rateLimitUnit = 8000 / curTicksel ;  	/** 8bits X 1000 / (curTicksel X 10e-6 s)  kbps **/
		else
			rateLimitUnit = 1000000 / curTicksel ;  /** 1 / (curTicksel X 10e-6 s) pps **/
	}

	if( 0 == rateLimitUnit )
	{
		printk("rateLimitUnit = 0 , set error.\n");
		return -EINVAL;
	}

	/*3. calculate tokenRate*/
	tokenRate_integer = rateLimitValue / rateLimitUnit ;
	tokenRate_fraction =( rateLimitValue % rateLimitUnit) * 64 / rateLimitUnit ;

	if( (tokenRate_integer > 0x3FFFF) || (tokenRate_fraction > 0x3F) ){
		printk("tokenRate overflow.\n");
		return -EINVAL;
	}

	tokenRate = (tokenRate_integer << TRTCM_TOKEN_RATE_INTEGER_SHIFT) | tokenRate_fraction;
	
	if(generalSetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_TOKEN_RATE, index, tokenRate) < 0){
		printk("Set TRTCM Token Rate Failed.\n");
		return -EFAULT;
	}

	/*4. calculate bucketSize*/
	/*Egress & Global: bucketsize should be small to prevent burst*/
	/*Ingress: bucketsize should be bigger for TCP flow*/
	if(trtcmModuleType == INGRESS_TRTCM){
		if( TRTCM_BYTE_MODE == generalGetRatelimitPktMode(qdma, trtcmModuleType, index) )
			bucketSize = generalGetBucketSizeByRate(rateLimitValue);
		else
			bucketSize = rateLimitValue<<RATELIMIT_PKT_MODE_BUCKET_SHIFT;
	}else{
		bucketSize = tokenRate_integer+1;
		if(bucketSize < 4096)/* if bucketsize is lower than 4096, 1518Byte flow will lead to bucket overflow */
			bucketSize = 4096;
	}
    
	return bucketSize;
}
static int airoha_qdma_general_set_ratelimit_mode_value(struct airoha_qdma *qdma, GENERAL_TrtcmRatelimitSet_T rxRateLimitCfg)
{
	int ret=0;
	enum trtcm_mode_type rateType ;
	GENERAL_TrtcmMode_T trtcmMode = airoha_generalGetTrtcmMode(qdma, rxRateLimitCfg.trtcmModule) ;
    int bucketSize=0;

    /*no trtcm mode for mtr_grp1&mtr_grp2*/
    if(QDMA_METER_GROUP(rxRateLimitCfg.Index) > 0){
        trtcmMode = TRTCM_RATELIMIT_MODE;
	}

	/*1. check index range*/
	if( (ret = qdma_general_check_index_valid(rxRateLimitCfg.trtcmModule, trtcmMode, rxRateLimitCfg.Index)) < 0 )
		return ret ;

	/*2. set value*/
	if( trtcmMode == TRTCM_RATELIMIT_MODE ){/*ratelimit mode, set just one index*/
		if((bucketSize = generalSetRatelimitTokenRate(qdma, rxRateLimitCfg.trtcmModule, rxRateLimitCfg.RateLimitValue, rxRateLimitCfg.Index)) < 0) {
			pr_err("Fault: set ratelimit mode value error.\n");
			return bucketSize ;
		}
        /*set BucketSize*/
		if((ret = generalSetRatelimitBucketSize(qdma, rxRateLimitCfg.trtcmModule, bucketSize, rxRateLimitCfg.Index))< 0) {
			pr_err("Fault: set ratelimit mode value error.\n");
			return ret ;
		}
	}else{/*trtcm mode, set CIR & PIR the cfg*/
		for( rateType = TRTCM_COMMIT_MODE ; rateType <= TRTCM_PEAK_MODE ; rateType++ ){
			if((bucketSize = airoha_generalSetTrtcmTokenRate(qdma, rxRateLimitCfg.trtcmModule, rxRateLimitCfg.RateLimitValue, rateType, rxRateLimitCfg.Index)) < 0) {
				pr_err("Fault: set ratelimit mode value error.\n");
				return bucketSize ;
			}
			if((ret = airoha_generalSetTrtcmBucketSize(qdma, rxRateLimitCfg.trtcmModule, bucketSize, rateType, rxRateLimitCfg.Index))< 0) {
				pr_err("Fault: set ratelimit mode value error.\n");
				return ret ;
			}
		}
	}
	
	return 0 ;
}

static int qdma_dev_trtcm_cfg_init(void)
{
	memset(trtcmCfgBase, 0, sizeof(uint) * TRTCM_MODE_MAX);
	memset(trtcmBucketByteUnit, 0, sizeof(uint) * TRTCM_MODE_MAX);
	memset(trtcmBucketPacketUnit, 0, sizeof(uint) * TRTCM_MODE_MAX);

    qdma_init_trtcm();
	
	return 0;
}
int airoha_dp_api_qdma_get_meter_value(struct airoha_qdma *qdma,uint meterIdx){
	int rate = 0;	
	uint trtcmModule = INGRESS_TRTCM;
	uint rateType = TRTCM_COMMIT_MODE;
	
	if((rate = airoha_generalGetTrtcmTokenRate(qdma, trtcmModule, rateType, meterIdx)) < 0) {
		pr_err("Fault: get ratelimit mode value error.\n");
		return rate ;
	}
	return rate ;
}
int airoha_dp_api_qdma_set_meter_value(struct airoha_qdma *qdma,uint meterIdx,uint value){
    GENERAL_TrtcmRatelimitSet_T rxRateLimitSet;
	  
    /*ratelimit glb setting*/
    rxRateLimitSet.trtcmModule = INGRESS_TRTCM;

	/*ratelimit each meter*/
	rxRateLimitSet.Index = meterIdx; 
	rxRateLimitSet.RateLimitValue = value;
	airoha_qdma_general_set_ratelimit_mode_value(qdma, rxRateLimitSet);
	return 0;
    
}


void airoha_dp_api_qdma_meter_default_config(struct airoha_qdma *qdma){
    int ringIdx = 0;
	int meterIdx = 0;
	
	GENERAL_TrtcmRatelimitCfg_T rxRateLimitCfg;
    GENERAL_TrtcmRatelimitSet_T rxRateLimitSet;
    int tickerSel[AIROHA_NUM_RX_RING] = {TRTCM_SLOW_TICK, TRTCM_FAST_TICK, TRTCM_SLOW_TICK, TRTCM_FAST_TICK
                                , TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK
                                , TRTCM_SLOW_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK
                                , TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK
                                , TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK
                                , TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK
                                , TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK
                                , TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK};
    /* ring2 for DLF/icmp flow*/
     u32 rateVal[AIROHA_NUM_RX_RING] = {4000, 1000000, 4000, 4000, 4000, 1000000, 1000000, 4000,
                            200, 200, 1000000, 1000000, 1000000, 1000000, 1000000, 1000000,
                            1000000, 1000000, 1000000, 1000000, 1000000, 1000000, 1000000, 1000000,
                            1000000, 1000000, 1000000, 1000000, 1000000, 1000000, 1000000, 1000000}; /* array length need adjust according to demand*/

	/* set default setting for global array */
	qdma_dev_trtcm_cfg_init();
    
    /*cfg glb setting*/
    rxRateLimitCfg.MeterEn = GENERAL_METER_ENABLE;
    rxRateLimitCfg.PktMode = TRTCM_PACKET_MODE;
    
    rxRateLimitCfg.trtcmModule = INGRESS_TRTCM;
    
    /*ratelimit glb setting*/
    rxRateLimitSet.trtcmModule = INGRESS_TRTCM;

    for(ringIdx=0; ringIdx<AIROHA_NUM_RX_RING; ringIdx++){
        /*cfg each ring*/
        rxRateLimitCfg.TickSel = tickerSel[ringIdx];
        rxRateLimitCfg.Index = ringIdx; 
        airoha_qdma_general_set_ratelimit_mode_cfg(qdma, rxRateLimitCfg);

        /*ratelimit each ring*/
        rxRateLimitSet.Index = ringIdx; 
        rxRateLimitSet.RateLimitValue = rateVal[ringIdx];
        airoha_qdma_general_set_ratelimit_mode_value(qdma, rxRateLimitSet);
    }
    generalSetTrtcmMode(qdma, INGRESS_TRTCM, TRTCM_RATELIMIT_MODE);
	
	// 32-127: default values, BYTE_MODE in meter
    for(meterIdx = 32; meterIdx < 127; meterIdx++) {
        rxRateLimitCfg.Index = meterIdx;
        rxRateLimitCfg.TickSel = TRTCM_FAST_TICK; // default FAST
        rxRateLimitSet.RateLimitValue = 1000000;  // default rate
        rxRateLimitCfg.PktMode = TRTCM_BYTE_MODE; // kbps

        airoha_qdma_general_set_ratelimit_mode_cfg(qdma, rxRateLimitCfg);
        rxRateLimitSet.Index = meterIdx;
        airoha_qdma_general_set_ratelimit_mode_value(qdma, rxRateLimitSet);
    }
	
    return;

}

/******************************************************************************
 Descriptor:	It's used to set the value of tx ratemeter config. It includes
 			timeDivisor and timeSlice, while tx_rateMeter_En always enable.
 Input Args:	arg1: The pointer of the  Tx RateMeter Config struct.
                     tx ratemeter default enable.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_ratemeter(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	QDMA_TxRateMeter_T *txRateMeterPtr = qdma_data->qdma_private.txRateMeterPtr;

#if SUPPORT_METER_WINDOW_SIZE_TIME_SLIDE
	if((uint)qdmaIsTxRateMeterEnable(base) > 0){
		qdmaDisableTxRateMeter(base) ;
		qdmaSetTxRateMeterWindowSize(base, txRateMeterPtr->txRateMeterWindowSize) ;
		qdmaSetTxRateMeterTimeSlice(base, txRateMeterPtr->txRateMeterTimeSlice) ;
		qdmaEnableTxRateMeter(base)	;
	}else {
		qdmaSetTxRateMeterWindowSize(base, txRateMeterPtr->txRateMeterWindowSize) ;
		qdmaSetTxRateMeterTimeSlice(base, txRateMeterPtr->txRateMeterTimeSlice) ;
	}
#else
	if(txRateMeterPtr->txRateMeterTimeDivisor<0 || txRateMeterPtr->txRateMeterTimeDivisor>=QDMA_TX_RATE_METER_TIME_DIVISOR_ITEMS) {
		QDMA_ERR("Fault: TX Rate Meter Time Divisor value shoule between 0 and %d\n", QDMA_TX_RATE_METER_TIME_DIVISOR_ITEMS-1) ; 
		return -EINVAL ;
	}

	qdmaSetTxRateMeterTimeDivisor(base, txRateMeterPtr->txRateMeterTimeDivisor) ;
	qdmaSetTxRateMeterTimeSlice(base, txRateMeterPtr->txRateMeterTimeSlice) ;
#endif

	return 0 ;
}




#if SUPPORT_MTR_ACNT_IN_QDMA
/*old APIs*/
int qdmaGetTxRateLimitConfig(unchar chnlIdx, unchar type){
	return 0 ;
}
int qdmaUpdateAllTxRateLimitValue(ushort curUnit, ushort newUnit, int ticksel_idx){
	return 0 ;
}
int qdmaSetTxRateLimitConfig(unchar chnlIdx, unchar type, ushort value){
	return 0 ;
}

#else
	
int qdmaSetTxRateLimitConfig(unchar chnlIdx, unchar type, ushort value) 
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	uint txRateLimitCfg = 0 ;
	
	txRateLimitCfg = (EGRESS_RATELIMIT_PARA_RWCMD | 
					((value<<EGRESS_RATELIMIT_PARA_VALUE_SHIFT)&EGRESS_RATELIMIT_PARA_VALUE_MASK) |
					((chnlIdx<<EGRESS_RATELIMIT_PARA_IDX_SHIFT)&EGRESS_RATELIMIT_PARA_IDX_MASK) |
					((type<<EGRESS_RATELIMIT_PARA_TYPE_SHIFT)&EGRESS_RATELIMIT_PARA_TYPE_MASK)) ;
	//QDMA_MSG(QDMA_DBG_MSG, "txRateLimitCfg is 0x%08x.\n", txRateLimitCfg) ;
	//IO_SREG(QDMA_CSR_RATELIMIT_PARAMETER_CFG(base), txRateLimitCfg) ;
	airoha_qdma_wr(base, QDMA_CSR_RATELIMIT_PARAMETER_CFG, txRateLimitCfg);
	
	if(qdmaChecConfigDone(base, QDMA_CSR_RATELIMIT_PARAMETER_CFG, EGRESS_RATELIMIT_PARA_RWCMD_DONE) < 0) {
		QDMA_ERR("Timeout for set Tx Rate Limit configuration.\n") ;
		return -ETIME ;
	}
	
	return 0 ;
}

int qdmaGetTxRateLimitConfig(unchar chnlIdx, unchar type)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	uint txRateLimitCfg = 0 ;
	
	txRateLimitCfg = (((chnlIdx<<EGRESS_RATELIMIT_PARA_IDX_SHIFT)&EGRESS_RATELIMIT_PARA_IDX_MASK) |
					((type<<EGRESS_RATELIMIT_PARA_TYPE_SHIFT)&EGRESS_RATELIMIT_PARA_TYPE_MASK)) ;
	//IO_SREG(QDMA_CSR_RATELIMIT_PARAMETER_CFG(base), txRateLimitCfg) ;
	airoha_qdma_wr(base, QDMA_CSR_RATELIMIT_PARAMETER_CFG, txRateLimitCfg);
	
	if(qdmaChecConfigDone(base, QDMA_CSR_RATELIMIT_PARAMETER_CFG, EGRESS_RATELIMIT_PARA_RWCMD_DONE) < 0) {
		QDMA_ERR("Timeout for get Tx Rate Limit configuration.\n") ;
		return -ETIME ;
	}
	
	//txRateLimitCfg = IO_GREG(QDMA_CSR_RATELIMIT_PARAMETER_CFG(base)) ;	
	txRateLimitCfg = airoha_qdma_rr(base, QDMA_CSR_RATELIMIT_PARAMETER_CFG);
	
	return ((txRateLimitCfg&EGRESS_RATELIMIT_PARA_VALUE_MASK)>>EGRESS_RATELIMIT_PARA_VALUE_SHIFT) ;
}

int qdmaUpdateAllTxRateLimitValue(ushort curUnit, ushort newUnit, int ticksel_idx)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	int ret=0, index=0, curPirValue=0, newPirValue=0;
	int chnlTickId=0, curPbsValue=0, newPbsValue=0;
	
	if(newUnit == 0)
		return -1;
	
	for(index=0; index<CONFIG_QDMA_CHANNEL; index++) 
	{
		chnlTickId=qdmaIsTxRateLimitChnlTickSel1(base,index)?1:0;
		
		if ((chnlTickId == ticksel_idx) && qdmaIsChnlRateLimitEnable(base, index))
		{
			curPirValue = qdmaGetTxRateLimitConfig(index, QDMA_Tx_Rate_Limit_PIR) ;
			curPbsValue = qdmaGetTxRateLimitConfig(index, QDMA_Tx_Rate_Limit_PBS);
			
			if(curPirValue < 0)
				return curPirValue ;			
			if(curPbsValue < 0)
				return curPbsValue ;
			
			newPirValue = curPirValue * curUnit / newUnit;
			newPbsValue = curPbsValue * curUnit / newUnit;
			
			if((ret = qdmaSetTxRateLimitConfig(index, QDMA_Tx_Rate_Limit_PIR, newPirValue)) < 0) {
				return ret ;
			}
			if((ret = qdmaSetTxRateLimitConfig(index, QDMA_Tx_Rate_Limit_PBS, newPbsValue)) < 0) {
				return ret ;
			}
		}
	}
	
	return 0 ;
}

#endif

/******************************************************************************
 Descriptor:	It's used to set the value of tx dba report parameter. It includes CIR, CBS, PIR and PBS.
 Input Args:	arg1: The pointer of the TX RateLimit Parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_dba_report(struct ECNT_QDMA_Data *qdma_data)
{
	int ret = 0 ;
#if !SUPPORT_MTR_ACNT_IN_QDMA
	//uint base = gpQdmaPriv->csrBaseAddr, 
	struct airoha_qdma *base = &glb_eth->qdma[1];
	
	uint maxPirValue=0;
	int curTicksel=0, curUnit=0, newUnit=0 ;
	int ticksel_idx=0;
#endif
	QDMA_TxDbaReport_T *txDbaReportPtr = qdma_data->qdma_private.txDbaReportPtr;
	
	if(txDbaReportPtr->channel>=CONFIG_QDMA_CHANNEL) {
		QDMA_ERR("Fault: channel index shoule between 0 and %d\n", CONFIG_QDMA_CHANNEL-1) ; 
		return -EINVAL ;
	}

#if SUPPORT_MTR_ACNT_IN_QDMA
	/*1. set CIR cfg & value*/
	if((ret = airoha_generalSetTrtcmMeterMode( EGRESS_TRTCM, GENERAL_METER_ENABLE, TRTCM_COMMIT_MODE1, txDbaReportPtr->channel )) < 0){
		QDMA_ERR("Fault:Set trtcm mode cfg error.\n") ; 
		return ret;
	}
	if((ret = generalSetTrtcmPktMode( EGRESS_TRTCM, TRTCM_BYTE_MODE, TRTCM_COMMIT_MODE1, txDbaReportPtr->channel )) < 0){
		QDMA_ERR("Fault:Set trtcm mode cfg error.\n") ; 
		return ret;
	}
	if((ret = generalSetTrtcmTickSel( EGRESS_TRTCM, TRTCM_FAST_TICK, TRTCM_COMMIT_MODE1, txDbaReportPtr->channel )) < 0){
		QDMA_ERR("Fault:Set trtcm mode cfg error.\n") ; 
		return ret;
	}
	if((ret = generalSetTrtcmTokenRate(EGRESS_TRTCM, txDbaReportPtr->cirParamValue, TRTCM_COMMIT_MODE1, txDbaReportPtr->channel)) < 0) {
		QDMA_ERR("Fault: set trtcm mode value error.\n");
		return ret ;
	}
	if((ret = generalSetTrtcmBucketSize(EGRESS_TRTCM, txDbaReportPtr->cbsParamValue, TRTCM_COMMIT_MODE1, txDbaReportPtr->channel))< 0) {
		QDMA_ERR("Fault: set trtcm mode value error.\n");
		return ret ;
	}

	/*2. set PIR cfg & value*/
	if((ret = airoha_generalSetTrtcmMeterMode( EGRESS_TRTCM, GENERAL_METER_ENABLE, TRTCM_PEAK_MODE1, txDbaReportPtr->channel )) < 0){
		QDMA_ERR("Fault:Set trtcm mode cfg error.\n") ; 
		return ret;
	}
	if((ret = generalSetTrtcmPktMode( EGRESS_TRTCM, TRTCM_BYTE_MODE, TRTCM_PEAK_MODE1, txDbaReportPtr->channel )) < 0){
		QDMA_ERR("Fault:Set trtcm mode cfg error.\n") ; 
		return ret;
	}
	if((ret = generalSetTrtcmTickSel( EGRESS_TRTCM, TRTCM_FAST_TICK, TRTCM_PEAK_MODE1, txDbaReportPtr->channel )) < 0){
		QDMA_ERR("Fault:Set trtcm mode cfg error.\n") ; 
		return ret;
	}
	if((ret = generalSetTrtcmTokenRate(EGRESS_TRTCM, txDbaReportPtr->pirParamValue, TRTCM_PEAK_MODE1, txDbaReportPtr->channel)) < 0) {
		QDMA_ERR("Fault: set trtcm mode value error.\n");
		return ret ;
	}
	if((ret = generalSetTrtcmBucketSize(EGRESS_TRTCM, txDbaReportPtr->pbsParamValue, TRTCM_PEAK_MODE1, txDbaReportPtr->channel))< 0) {
		QDMA_ERR("Fault: set trtcm mode value error.\n");
		return ret ;
	}
#else
	if(txDbaReportPtr->pirParamValue > txDbaReportPtr->cirParamValue)
		maxPirValue = txDbaReportPtr->pirParamValue ;
	else
		maxPirValue = txDbaReportPtr->cirParamValue ;
	

	/*ticker0 default as 1kbps, you'd better not change ticker0*/
	if(maxPirValue <= TX_RATELIMIT_TICKER0_ACCURACY*0xFFFF){    /* 0kbps ~64Mbps */
		newUnit = TX_RATELIMIT_TICKER0_ACCURACY;
		ticksel_idx= 0;
		if(qdmaIsTxRateLimitChnlTickSel1(base,txDbaReportPtr->channel))
			qdmaSetTxRateLimitPerChnlTickSelIdx(base, txDbaReportPtr->channel,0);
	}else{     /* 64Mbps ~4Gbps */
		newUnit = (maxPirValue - 1)/0xFFFF + 1;  /*round up to nearest int num*/
		ticksel_idx= 1;
		if(!qdmaIsTxRateLimitChnlTickSel1(base,txDbaReportPtr->channel))
			qdmaSetTxRateLimitPerChnlTickSelIdx(base, txDbaReportPtr->channel,1);
	}

	curTicksel = qdmaGetTxRateLimitTicksel(base, ticksel_idx);
	if(curTicksel == 0)
		curUnit = 0 ;
	else
		curUnit = 8000 / curTicksel ;
	
	/* update ratelimit unit and all channel's pir value */		
	if(newUnit > curUnit) {
		curTicksel = 8000 / newUnit;
		qdmaSetTxRateLimitTicksel(base,ticksel_idx, curTicksel) ;	
		qdmaUpdateAllTxRateLimitValue(curUnit, newUnit, ticksel_idx) ;	
		curUnit = newUnit;
	}
	
	if((ret = qdmaSetTxRateLimitConfig(txDbaReportPtr->channel, QDMA_Tx_Rate_Limit_CIR, txDbaReportPtr->cirParamValue / curUnit)) < 0) {
		return ret ;
	}
	if((ret = qdmaSetTxRateLimitConfig(txDbaReportPtr->channel, QDMA_Tx_Rate_Limit_CBS, txDbaReportPtr->cbsParamValue / (1<<qdmaGetTxRateLimitBucketScale(base)))) < 0) {
		return ret ;
	}
	if((ret = qdmaSetTxRateLimitConfig(txDbaReportPtr->channel, QDMA_Tx_Rate_Limit_PIR, txDbaReportPtr->pirParamValue / curUnit)) < 0) {
		return ret ;
	}
	if((ret = qdmaSetTxRateLimitConfig(txDbaReportPtr->channel, QDMA_Tx_Rate_Limit_PBS, txDbaReportPtr->pbsParamValue) / (1<<qdmaGetTxRateLimitBucketScale(base))) < 0) {
		return ret ;
	}
#endif
	
	return 0;
}


/*channel 32 means get maxpir channel of all , otherwise means get maxpir channel except the input channel*/
int qdmaGetLimitRateMaxChnl(int ticksel_idx, int channel)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	int index=0, ret=0;
	int chnlTickId=0, maxChnl=CONFIG_QDMA_CHANNEL;
	int MaxTxLimitRate=0;
	
	if(channel < 0 || channel > CONFIG_QDMA_CHANNEL) {
		QDMA_ERR("Fault: channel index should between 0 and %d\n", CONFIG_QDMA_CHANNEL) ; 
		return -EINVAL ;
	}
	
	for(index=0; index<CONFIG_QDMA_CHANNEL; index++) {
		if(channel != index){
			chnlTickId=qdmaIsTxRateLimitChnlTickSel1(base,index)?1:0;
			if(chnlTickId == ticksel_idx){
				if(qdmaIsChnlRateLimitEnable(base, index)) {
					ret = qdmaGetTxRateLimitConfig(index, QDMA_Tx_Rate_Limit_PIR) ;
					if(ret < 0)
						return ret ;
					if(ret > MaxTxLimitRate){
						MaxTxLimitRate = ret ;	
						maxChnl = index;							
					}
				}
			}
		}
	}
	
	return maxChnl ;
}



/******************************************************************************
 Descriptor:	It's used to set the value of tx rate limit parameter. It includes chnlRateLimitEn and PIR.
 Input Args:	arg1: The pointer of the TX RateLimit Parameter struct.It includes
 			- chnlRateLimitEn: means current channel ratelimit enable or disable.
 			- PIR: unit is kbps, scope is [1kbps~1Gbps]
 			ticker0 as 1Kbps,we should not change ticker0 unit, only ticker1 can be changed.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_ratelimit(struct ECNT_QDMA_Data *qdma_data)
{
	int ret=0 , enMode=0, revertToTicker0=0;
	int curUnit=0, newUnit=0, unit1=0;
	int curTicksel=0, curTicksel_idx=0;
	int maxPirChnl=0, tmpMaxPirValue=0, tmpMaxTxLimitRate=0;
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	
	QDMA_TxRateLimitSet_T *txRateLimitPtr = qdma_data->qdma_private.txRateLimitSetPtr;
/*
#ifdef QDMA_LAN
	int port = 0;
	int switch_port =0, switchLeastUnit=0;
#endif
*/
	if(txRateLimitPtr->chnlIdx >= CONFIG_QDMA_CHANNEL) {
		QDMA_ERR("Fault: channel index should between 0 and %d\n", CONFIG_QDMA_CHANNEL-1) ; 
		return -EINVAL ;
	}	
/*
	if(TCSUPPORT_FWC_ENV_VAL)
	{

#ifdef QDMA_LAN
		if(0 != txRateLimitPtr->chnlIdx)
		{
			port = txRateLimitPtr->chnlIdx - 1;   
			if(MT7530LanPortMap2Switch_hook)
			{
				switch_port = MT7530LanPortMap2Switch_hook(port);
			}
			txRateLimitPtr->chnlIdx = switch_port;
		}  
#endif

	} 
*/
	if(txRateLimitPtr->chnlRateLimitEn == QDMA_DISABLE) {
		qdmaDisableChnlRateLimit(base, txRateLimitPtr->chnlIdx) ;
		if(qdmaIsTxRateLimitChnlTickSel1(base,txRateLimitPtr->chnlIdx)){
			curTicksel_idx = 1;
			txRateLimitPtr->rateLimitValue = 0;
			goto t1;
		}
		return 0;
	} else {
		enMode = qdmaIsChnlRateLimitEnable(base, txRateLimitPtr->chnlIdx) ;		
		qdmaEnableChnlRateLimit(base, txRateLimitPtr->chnlIdx) ;
		if(!enMode) {	/* change enMode from disable to enable */
			if(!qdmaIsTxRateLimitChnlTickSel1(base,txRateLimitPtr->chnlIdx)){
				qdmaSetTxRateLimitPerChnlTickSelIdx(base, txRateLimitPtr->chnlIdx, 1);
			}
			curTicksel = qdmaGetTxRateLimitTicksel(base,1) ;
			qdmaSetTxRateLimitTicksel(base, 1, 1) ;
			ret = qdmaSetTxRateLimitConfig(txRateLimitPtr->chnlIdx, QDMA_Tx_Rate_Limit_PIR, 0xFFFF) ;
			mdelay(100) ;
			qdmaSetTxRateLimitTicksel(base, 1, curTicksel) ;
		}		
	}
	
	/*ticker0 default as 1kbps, you'd better not change ticker0*/
	if(txRateLimitPtr->rateLimitValue <= 1000){	 /*1kbps ~1000kbps */
 		curTicksel_idx = 0;
		if(qdmaIsTxRateLimitChnlTickSel1(base,txRateLimitPtr->chnlIdx)){
			qdmaSetTxRateLimitPerChnlTickSelIdx(base, txRateLimitPtr->chnlIdx, 0);
			revertToTicker0 = 1;
		}
	}else{//1Mbps ~4Gbps
		newUnit = (txRateLimitPtr->rateLimitValue - 1)/0xFFFF + 1;	/*round up to nearest int num*/
		if(newUnit <= TX_RATELIMIT_TICKER1_ACCURACY)
			newUnit = TX_RATELIMIT_TICKER1_ACCURACY;
		curTicksel_idx = 1;
		if(!qdmaIsTxRateLimitChnlTickSel1(base,txRateLimitPtr->chnlIdx))
			qdmaSetTxRateLimitPerChnlTickSelIdx(base, txRateLimitPtr->chnlIdx, 1);
	}
	
t1:
	/* get current unit */
	curTicksel = qdmaGetTxRateLimitTicksel(base, curTicksel_idx) ;
	if(curTicksel == 0)
		curUnit = 0 ;
	else
		curUnit = 8000 / curTicksel ;  /**8bits *1000/(curTicksel*10e-6 s) kbps **/
	if(1 == revertToTicker0 || 1 == curTicksel_idx){ /*we only update channel bind to ticker1 */
		if(curTicksel_idx == 1)
			unit1 = curUnit;
		else /*revert from ticker1 to Ticker0*/
			unit1 = 8000 / qdmaGetTxRateLimitTicksel(base, 1) ;
		maxPirChnl = qdmaGetLimitRateMaxChnl(1, txRateLimitPtr->chnlIdx);
		if(maxPirChnl != CONFIG_QDMA_CHANNEL){
			tmpMaxTxLimitRate = qdmaGetTxRateLimitConfig(maxPirChnl, QDMA_Tx_Rate_Limit_PIR);
		}else{                 //tmpMaxTxLimitRate =0
            if(txRateLimitPtr->chnlRateLimitEn == QDMA_DISABLE)
			    newUnit = TX_RATELIMIT_TICKER1_ACCURACY;       //t1 -> disable
		}

		tmpMaxPirValue = tmpMaxTxLimitRate * unit1;
		if(tmpMaxPirValue > txRateLimitPtr->rateLimitValue)
			newUnit = (tmpMaxPirValue - 1)/0xFFFF + 1;  /*round up to nearest int num*/;
		if(newUnit <= TX_RATELIMIT_TICKER1_ACCURACY)
        	newUnit = TX_RATELIMIT_TICKER1_ACCURACY;
	}

#ifdef QDMA_LAN
	/* to make sure qdma_lan burst size smaller than 40Kbyte */
	switchLeastUnit = txRateLimitPtr->rateLimitValue / (SWITCH_BUFFER_THRESHOLD*1000) ;
	newUnit = MAX(newUnit, switchLeastUnit) ;
#endif
	
	/* update ratelimit unit1 and all channel's pir value */		
	if(newUnit != unit1)
	{
		curTicksel = 8000 / newUnit;
		qdmaSetTxRateLimitTicksel(base, 1, curTicksel) ;
		qdmaUpdateAllTxRateLimitValue(unit1, newUnit,1) ;
		if(curTicksel_idx == 1)
			curUnit = newUnit;
	}
    if( curUnit == 0) {
		QDMA_ERR("Fault:qdma tx ratelimit Unit is zero\n") ; 
		return -EINVAL ;
	}
	if((ret = qdmaSetTxRateLimitConfig(txRateLimitPtr->chnlIdx, QDMA_Tx_Rate_Limit_PIR, txRateLimitPtr->rateLimitValue / curUnit)) < 0) {
		return ret ;
	}
	/* pbs = 2*0.625*pir */
	if((ret = qdmaSetTxRateLimitConfig(txRateLimitPtr->chnlIdx, QDMA_Tx_Rate_Limit_PBS, ((txRateLimitPtr->rateLimitValue>>1) + (txRateLimitPtr->rateLimitValue>>3)) / curUnit)) < 0) {
		return ret ;
	}

	return 0;
}


/******************************************************************************
 Descriptor:	It's used to set the value of rx rate limit config. It includes
 			rx_rateLimit_En, bucketScale and tickSel.
 Input Args:	arg1: The pointer of the RX Rate Limit config struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_rx_ratelimit_en(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	
	//int curTicksel=0, ringIdx=0;
	//uint rxRateLimitPir[RX_RING_NUM], rxRateLimitPbs[RX_RING_NUM] ;
	QDMA_Mode_t rxRateLimitEn = qdma_data->qdma_private.mode;

	if(rxRateLimitEn == QDMA_DISABLE) {
		qdmaDisableRxRateLimit(base) ;
	} else {
		qdmaEnableRxRateLimit(base) ;
	}

	return 0;
}


/******************************************************************************
 Descriptor:	It's used to set the value of rx rate limit config. It includes
 			rx_rateLimit_En, bucketScale and tickSel.
 Input Args:	arg1: The pointer of the RX Rate Limit config struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_rx_ratelimit_pkt_mode(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	QDMA_RxPktMode_t pktMode = qdma_data->qdma_private.pktMode;

	if(pktMode == QDMA_RX_RATE_LIMIT_BY_PACKET) {
		qdmaSetRxRateLimitByPacket(base) ;
	} else {
		qdmaSetRxRateLimitByByte(base) ;
	}
	
	return 0;
}


/******************************************************************************
 Descriptor:	It's used to get the value of rx rate limit config. It includes
 			rx_rateLimit_En, bucketScale and tickSel.
 Input Args:	arg1: The pointer of the RX Rate Limit config struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_rx_ratelimit_cfg(struct ECNT_QDMA_Data *qdma_data)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	
	int curTicksel=0, unitBase=0;	//unitBase means the unit when ticksel is 1us.
	
	QDMA_RxRateLimitCfg_T *rxRateLimitCfgPtr = qdma_data->qdma_private.rxRateLimitCfgPtr;

	if(qdmaIsRxRateLimitEnable(base)) {
		rxRateLimitCfgPtr->rxRateLimitEn = QDMA_ENABLE ;
	} else {
		rxRateLimitCfgPtr->rxRateLimitEn = QDMA_DISABLE ;
	}
	
	if(qdmaIsRxRateLimitByPacket(base)) {
		rxRateLimitCfgPtr->rxRateLimitPktMode = QDMA_RX_RATE_LIMIT_BY_PACKET ;
		unitBase = 1000000 ;
	} else {
		rxRateLimitCfgPtr->rxRateLimitPktMode = QDMA_RX_RATE_LIMIT_BY_BYTE ;
		unitBase = 8000 ;
	}
	
	curTicksel = qdmaGetRxRateLimitTicksel(base, 1);
	if(curTicksel == 0)
		rxRateLimitCfgPtr->rxRateLimitUnit = 0 ;
	else
		rxRateLimitCfgPtr->rxRateLimitUnit = unitBase/curTicksel ;	/*default ticekr(ticker1)*/
	
	curTicksel = qdmaGetRxRateLimitTicksel(base, 0);
	
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
	if(curTicksel == 0)
		rxRateLimitCfgPtr->rxRateLimitUnit0 = 0 ;
	else
		rxRateLimitCfgPtr->rxRateLimitUnit0 = unitBase/curTicksel ; /*ticekr0*/
#endif

	rxRateLimitCfgPtr->rxRateLimitBucketScale = qdmaGetRxRateLimitBucketScale(base) ;
	
	return 0;
}



int qdmaGetRxRateLimitConfig(unchar ringIdx, unchar type)
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	
	uint rxRateLimitCfg = 0 ;
	
	rxRateLimitCfg = (RX_RATELIMIT_RING_IDX(ringIdx) |
					(type<<RX_RATELIMIT_TYPE_SHIFT)) ;
	//IO_SREG(QDMA_CSR_RX_RATELIMIT_CFG(base), rxRateLimitCfg) ;
	airoha_qdma_wr(base, QDMA_CSR_RX_RATELIMIT_CFG, rxRateLimitCfg);
	
	if(qdmaChecConfigDone(base, QDMA_CSR_RX_RATELIMIT_CFG, RX_RATELIMIT_RWCMD_DONE) < 0) {
		QDMA_ERR("Timeout for get Rx Rate Limit configuration.\n") ;
		return -ETIME ;
	}
	
	//rxRateLimitCfg = IO_GREG(QDMA_CSR_RX_RATELIMIT_CFG(base)) ;	
	rxRateLimitCfg = airoha_qdma_rr(base, QDMA_CSR_RX_RATELIMIT_CFG);
	
	return (rxRateLimitCfg&RX_RATELIMIT_VALUE_MASK)>>RX_RATELIMIT_VALUE_SHIFT ;
}


int qdmaSetRxRateLimitConfig(unchar ringIdx, unchar type, ushort value) 
{
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	
	uint rxRateLimitCfg = 0 ;
	
	rxRateLimitCfg = (RX_RATELIMIT_RWCMD | 
					((value<<RX_RATELIMIT_VALUE_SHIFT)&RX_RATELIMIT_VALUE_MASK) |
					RX_RATELIMIT_RING_IDX(ringIdx) |
					(type<<RX_RATELIMIT_TYPE_SHIFT)) ;
	QDMA_MSG(QDMA_DBG_MSG, "rxRateLimitCfg is 0x%08x.\n", rxRateLimitCfg) ;
	//IO_SREG(QDMA_CSR_RX_RATELIMIT_CFG(base), rxRateLimitCfg) ;
	airoha_qdma_wr(base, QDMA_CSR_RX_RATELIMIT_CFG, rxRateLimitCfg);
	
	if(qdmaChecConfigDone(base, QDMA_CSR_RX_RATELIMIT_CFG, RX_RATELIMIT_RWCMD_DONE) < 0) {
		QDMA_ERR("Timeout for set Rx Rate Limit configuration.\n") ;
		return -ETIME ;
	}
	
	return 0 ;
}

int qdmaUpdateAllRxRateLimitValue(ushort curUnit, ushort newUnit, unchar ticker)
{
	int ret=0, index=0, curPirValue=0, newPirValue=0;
	int curPbsValue=0, newPbsValue=0;
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	
	if(newUnit == 0)
		return -1;
	
	for(index=0; index<RX_RING_NUM; index++) {
		if(qdmaGetRxRatelimitSelTick(base,index) != ticker)
			continue;
		
		curPirValue = qdmaGetRxRateLimitConfig(index, QDMA_Rx_Rate_Limit_PIR) ;
		curPbsValue = qdmaGetRxRateLimitConfig(index, QDMA_Rx_Rate_Limit_PBS) ;
		if(curPirValue < 0)
			return curPirValue ;

		newPirValue = curPirValue * curUnit / newUnit;
		newPbsValue = curPbsValue * curUnit / newUnit;
		
		if((ret = qdmaSetRxRateLimitConfig(index, QDMA_Rx_Rate_Limit_PIR, newPirValue)) < 0) {
			return ret ;
		}
		if((ret = qdmaSetRxRateLimitConfig(index, QDMA_Rx_Rate_Limit_PBS, curPbsValue)) < 0) {
			return ret ;
		}
	}
	
	return 0 ;
}

/******************************************************************************
 Descriptor:	It's used to set the value of rx rate limit parameter. It includes PIR and PBS.
 Input Args:	arg1: The pointer of the RX RateLimit Parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_rx_ratelimit(struct ECNT_QDMA_Data *qdma_data)
{
	int ret = 0, unitBase=0;	//unitBase means the unit when ticksel is 1us.
	int curTicksel=0, curUnit=0, newUnit=0 ;
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	
	QDMA_RxRateLimitSet_T *rxRateLimitPtr = qdma_data->qdma_private.rxRateLimitSetPtr;
	unchar ticker=0;
	if(rxRateLimitPtr->ringIdx>=RX_RING_NUM) {
		QDMA_ERR("Fault: ringIdx shoule be 0 ~ %d\n",(RX_RING_NUM-1)) ; 
		return -EINVAL ;
	}
	
	if(qdmaIsRxRateLimitByPacket(base)) {
		unitBase = 1000000 ;
	} else {
		unitBase = 8000 ;
	}
	
	/* get new unit and current unit */
	newUnit = 1 + rxRateLimitPtr->rateLimitValue / 0xFFFF;
	ticker = qdmaGetRxRatelimitSelTick(base, rxRateLimitPtr->ringIdx);	/*ticker0 or ticker1*/
	curTicksel = qdmaGetRxRateLimitTicksel(base, ticker);

	if(curTicksel == 0) 
		curUnit = 0 ;
	else
		curUnit = unitBase / curTicksel ;
	
	/* update ratelimit unit and all channel's pir value */		
	if(newUnit > curUnit) {
		curTicksel = unitBase / newUnit;

		qdmaSetRxRateLimitTickVal(base, ticker, curTicksel) ;
		qdmaUpdateAllRxRateLimitValue(curUnit, newUnit, ticker) ;

		curUnit = newUnit;
	}
	
	if((ret = qdmaSetRxRateLimitConfig(rxRateLimitPtr->ringIdx, QDMA_Rx_Rate_Limit_PIR, rxRateLimitPtr->rateLimitValue / curUnit)) < 0) {
		return ret ;
	}
	if((ret = qdmaSetRxRateLimitConfig(rxRateLimitPtr->ringIdx, QDMA_Rx_Rate_Limit_PBS, rxRateLimitPtr->rateLimitValue / curUnit)) < 0) {
		return ret ;
	}

	return 0;
}


/******************************************************************************
 Descriptor:	It's used to get the value of rx rate limit parameter. It includes PIR and PBS.
 Input Args:	arg1: The pointer of the  RX RateLimit Parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_rx_ratelimit(struct ECNT_QDMA_Data *qdma_data) 
{
	int ret = 0 , unitBase=0;	/* unitBase means the unit when ticksel is 1us. */
	//uint base = gpQdmaPriv->csrBaseAddr ;
	struct airoha_qdma *base = &glb_eth->qdma[1];
	
	
	int curTicksel=0, curUnit=0 ;
	QDMA_RxRateLimitGet_T *rxRateLimitPtr = qdma_data->qdma_private.rxRateLimitGetPtr;
	unchar ticker=0;

	if(rxRateLimitPtr->ringIdx>=RX_RING_NUM) {
		QDMA_ERR("Fault: ringIdx shoule be 0 ~ %d\n",(RX_RING_NUM-1)) ; 
		return -EINVAL ;
	}
	
	if(qdmaIsRxRateLimitByPacket(base)) {
		unitBase = 1000000 ;
	} else {
		unitBase = 8000 ;
	}
		
	ticker = qdmaGetRxRatelimitSelTick(base, rxRateLimitPtr->ringIdx);	/*ticker0 or ticker1*/
	curTicksel = qdmaGetRxRateLimitTicksel(base, ticker);
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
	rxRateLimitPtr->rxBindTicker = ticker;
#endif
	if(curTicksel == 0) 
		curUnit = 0 ;
	else
		curUnit = unitBase / curTicksel ;
	
	ret = qdmaGetRxRateLimitConfig(rxRateLimitPtr->ringIdx, QDMA_Rx_Rate_Limit_PIR) ;
	if(ret < 0)
		return ret ;
	rxRateLimitPtr->rateLimitValue = ret * curUnit ;
	
	ret = qdmaGetRxRateLimitConfig(rxRateLimitPtr->ringIdx, QDMA_Rx_Rate_Limit_PBS) ;
	if(ret < 0)
		return ret ;
	rxRateLimitPtr->pbsParamValue = ret * (1<<qdmaGetRxRateLimitBucketScale(base));
		
	return 0 ;
}


static inline int channel_is_recorded(unsigned char channel, int num, unsigned char *channelPtr)
{
	int i = 0 ;
	unsigned char * tmpPtr = channelPtr;
	for(i = 0; i < num; i++){
		if(channel == *tmpPtr){
			/*This channel id has been recorded already.*/
			return 0;
		}
		tmpPtr ++;
	}
	return -1;
}


__IMEM int qdma_get_using_tx_dscp_channel(struct ECNT_QDMA_Data *qdma_data)
{
	struct QDMA_DscpInfo_S *diPtr = NULL;
	int ringIdx = 0;
	int i = 0;
	int number = 0;
	unsigned char * tmpPtr = NULL;
	unsigned char channel = 0;

	if(NULL == qdma_data)
	{
		QDMA_ERR("The input arguments are wrong.\n"); 
		return -EFAULT;
	}

	if(NULL != gpQdmaPriv->txHeadPtr[ringIdx]) {
		QDMA_MSG(QDMA_DBG_MSG,"txHeadPtr is not NULL\n"); 
		return -ENOSR;
	}

	tmpPtr = qdma_data->qdma_private.qdma_dscp_chnl_info->channel;

	for(i = 0; i < gpQdmaPriv->txDscpNum[ringIdx]; i++) {
		diPtr = (struct QDMA_DscpInfo_S *)((uintptr_t)gpQdmaPriv->txBaseAddr[ringIdx] + i * sizeof(struct QDMA_DscpInfo_S));
		if(diPtr->next == NULL && diPtr!=gpQdmaPriv->txTailPtr[ringIdx] && !diPtr->dscpPtr->ctrl.done) {
			QDMA_MSG(QDMA_DBG_MSG," msg[0]:%.8x\n", diPtr->dscpPtr->msg[0]);
			channel = (unsigned char)(diPtr->dscpPtr->msg[0] >> 3);
			if(channel_is_recorded(channel, number, qdma_data->qdma_private.qdma_dscp_chnl_info->channel) != 0){
				*tmpPtr = channel;
				number ++;
				if(number > CONFIG_QDMA_CHANNEL){
					QDMA_ERR("Channel number = %d, exceeds CONFIG_QDMA_CHANNEL(%d)\n", number, CONFIG_QDMA_CHANNEL) ;
					return -EFAULT;
				}
				tmpPtr ++;
				QDMA_MSG(QDMA_DBG_MSG," channel = %d\n", channel) ;
			}
		}
	}
	qdma_data->qdma_private.qdma_dscp_chnl_info->chnlNum = number;
	QDMA_MSG(QDMA_DBG_MSG," num = %d\n", number) ;

	return 0;
}

// Function definitions
int qdma_receive_packets(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_transmit_packet(struct ECNT_QDMA_Data *qdma_data) {
	u32 msg0, msg1;
	struct port_info *pinfo;
	struct sk_buff *skb = qdma_data->qdma_private.qdma_transmit.skb ;
	msg0 = qdma_data->qdma_private.qdma_transmit.pTxMsg->txmsg0;
	msg1 = qdma_data->qdma_private.qdma_transmit.pTxMsg->txmsg1;
	pinfo = qdma_data->qdma_private.qdma_transmit.pMacInfo ;
	
	qdma_wan_tx(skb, msg0, msg1, pinfo);
    return 0;
}

int qdma_set_mac_limit_threshold(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_mac_limit_threshold(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_bm_transmit_packet_wifi_fast(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_prefetch_mode(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_pktsize_overhead_en(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_pktsize_overhead_en(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_pktsize_overhead_value(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_pktsize_overhead_value(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_lmgr_low_threshold(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_lmgr_low_threshold(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_lmgr_status(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_dbg_level(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dump_dma_busy(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dump_reg_polling(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_force_receive_rx_ring1(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_tx_drop_en(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_tx_drop_en(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_tx_ratemeter(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_enable_tx_ratelimit(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_tx_ratelimit_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_tx_ratelimit_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_tx_ratelimit(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_tx_dba_report(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_rx_protect_en(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_rx_protect_en(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_rx_low_threshold(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_rx_low_threshold(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_txq_dei_drop_mode(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_txq_cngst_mode(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_txq_cngst_mode(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_txq_dei_threshold_scale(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_txq_dei_threshold_scale(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_txq_cngst_auto_config(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_txq_cngst_total_threshold(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_txq_cngst_total_threshold(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_txq_cngst_channel_threshold(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_txq_cngst_channel_threshold(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_txq_cngst_queue_threshold(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_txq_cngst_queue_threshold(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_txq_peekrate_params(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_txq_peekrate_params(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_txq_cngst_static_queue_normal_threshold(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_txq_cngst_static_queue_dei_threshold(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_txq_cngst_dynamic_info(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_txq_cngst_static_info(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_txq_cngst_queue_nonblocking(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_txq_cngst_channel_nonblocking(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_virtual_channel_mode(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_virtual_channel_mode(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_virtual_channel_qos(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_virtual_channel_qos(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_clear_and_set_dbg_cntr_channel_group(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_clear_and_set_dbg_cntr_queue_group(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_clear_dbg_cntr_value_all(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dump_dbg_cntr_value(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_dbg_cntr_all_queue_value(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_dbg_cntr_rx_ring(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dump_tx_qos(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dump_virtual_channel_qos(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dump_tx_ratelimit(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dump_rx_ratelimit(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dump_tx_dba_report(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dump_txq_cngst(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_clear_cpu_counters(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dump_cpu_counters(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dump_register_value(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dump_descriptor_info(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dump_irq_info(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dump_hwfwd_info(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dump_info_all(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_read_vip_info(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_mac_qos_config(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_general_set_trtcm_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_general_get_trtcm_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_general_set_ratelimit_mode_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_general_get_ratelimit_mode_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_general_set_ratelimit_mode_value(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_general_get_ratelimit_mode_value(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_general_set_ratelimit_bucket_size(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_clear_and_set_dbg_cntr_ring_group(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_enable_rxpkt_int2(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_disable_rxpkt_int2(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_receive_packets_int2(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_general_get_trtcm_mode_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_general_set_trtcm_bucket_size(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_flow_cntr_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_flow_cntr_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_flow_cntr_value(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_clear_flow_cntr_value(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_tx_wred_mode(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_tx_wred_mode(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_tx_wred_probability(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_tx_wred_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_cpu_rx_red_probability(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_cpu_rx_red_probability(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_channel_close_status(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_channel_empty_status(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_oam_modify_fp_en(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_multicast_en(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_multicast_en(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_allocate_meter(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_free_meter(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_qos_flag(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_allocate_acnt(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_free_acnt(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_sla_chnl_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_sla_chnl_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_qos_aging_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_qos_aging_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_per_queue_aging_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_per_queue_aging_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_multicast_sptag_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_multicast_sptag_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_multicast_fport_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_multicast_fport_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_txq_cngst_static_channel_en(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_txq_cngst_static_queue_ratio(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_hqos_en(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_reset(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dram_test_dma_config(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dram_test_dma_enable(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dram_test_is_rx_done(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_dram_test_dma_skb_get(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_set_multicast_1toN_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_get_channel_cfg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_api_set_txmsg(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_api_get_rxmsg_eth(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_api_check_dscp_is_free(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_api_set_downstream_qos_mode(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}

int qdma_api_get_downstream_qos_mode(struct ECNT_QDMA_Data *qdma_data) {
    return 0;
}


/************************************************************************
*                  FE APIs
*************************************************************************
*/

int fe_api_set_channel_enable(struct ecnt_fe_data *fe_data)
{
	unsigned long int offset, val;
	struct airoha_eth *eth = glb_eth;
	
	FE_Gdma_Sel_t gdm_sel = fe_data->gdm_sel;
	FE_TXRX_Sel_t txrx_sel = fe_data->txrx_sel;
	FE_Enable_t enable = fe_data->api_data.enable;
	uint channel = fe_data->channel;
    ulong flags = 0;

    if (isEN751221)
    {
	    spin_lock_irqsave(&fe_pse_reset_lock, flags);
    }

	if(gdm_sel == FE_GDM_SEL_GDMA1){
		if(txrx_sel == FE_GDM_SEL_TX)
			offset = GDMA1_TXCHN_EN;
		else
			offset = GDMA1_RXCHN_EN;
	}else{
		if(txrx_sel == FE_GDM_SEL_TX)
			offset = GDMA2_TXCHN_EN;
		else
			offset = GDMA2_RXCHN_EN;
	}

	//val = read_reg_word(base_addr);
	val = airoha_fe_rr(eth, offset);
	
	if(enable == FE_DISABLE){
		val &= ~(1<<channel);
	}else{
		val |= (1<<channel);
	}
	
	//write_reg_word(base_addr, val);
	airoha_fe_wr(eth, offset, val);
	

    if (isEN751221)
    {
	    spin_unlock_irqrestore(&fe_pse_reset_lock, flags);
    }
    
	return 0;
}



int fe_set_hwfwd_channel(struct ecnt_fe_data *fe_data)
{
	struct airoha_eth *eth = glb_eth;
	
	FE_Cdma_Sel_t	cdm_sel = fe_data->cdm_sel;
	uint			channel = fe_data->channel;
	FE_Enable_t		enable = fe_data->api_data.enable;
	unsigned int	offset, value ;

	if (cdm_sel == FE_CDM_SEL_CDMA1){
		offset = CDMA1_HWF_CHN_EN;
	}else if(cdm_sel == FE_CDM_SEL_CDMA2){
		offset = CDMA2_HWF_CHN_EN;
	}else{
		return -1;
	}
	printk("\n In fe_set_hwfwd_channel:: \n");
	printk("before: %08x \n",readl(((eth)->fe_regs)+offset));
	//value = read_reg_word(reg);
	value = airoha_fe_rr(eth, offset);

	value &= ~(1 << channel);

	value |= (enable << channel);
	
	//write_reg_word(reg, value);
	airoha_fe_wr(eth, offset, value);
	
	printk("after: %08x \n",readl(((eth)->fe_regs)+offset));

	return 0;
}



int fe_api_set_chn_retire_done(struct ecnt_fe_data *fe_data)
{
#if defined(TCSUPPORT_CPU_EN7580)
	struct airoha_eth *eth = glb_eth;
    unsigned int   offset = 0;
    unsigned int value = 0;
    uint      chn_done = 0;
    uint           chn = fe_data->channel;
    FE_Gdma_Sel_t gdm_sel = fe_data->gdm_sel;
	
	
	
    if (gdm_sel == FE_GDM_SEL_GDMA1)
    {
        offset = GDMA1_CHN_RLS;
    }
    else if(gdm_sel == FE_GDM_SEL_GDMA2)
    {
        offset = GDMA2_CHN_RLS;
    }
    else
    {
        return -1;
    }
    chn &= 0x1f;
    //value = read_reg_word(reg);
	value = airoha_fe_rr(eth, offset);
	
    chn_done = (value >> GDMA_CHN_RLS_CHN_OFFSET) & 0x1f;
    if(chn_done == chn)
    {
        //write_reg_word(reg, 0);
		airoha_fe_wr(eth, offset, 0);
    }
    else
    {
        return -1;
    }
#endif

    return 0;
}



int fe_api_set_chn_retire_action(struct ecnt_fe_data *fe_data)
{
#if defined(TCSUPPORT_CPU_EN7580)
	struct airoha_eth *eth = glb_eth;
    unsigned int      offset = 0;
    unsigned int    value = 0;
    unsigned int      chn = fe_data->channel;
    FE_Gdma_Sel_t gdm_sel = fe_data->gdm_sel;
    if (gdm_sel == FE_GDM_SEL_GDMA1)
    {
        offset = GDMA1_CHN_RLS;
    }
    else if(gdm_sel == FE_GDM_SEL_GDMA2)
    {
        offset = GDMA2_CHN_RLS;
    }
    else
    {
        return -1;
    }
    chn &= 0x1f;
    value = (chn << GDMA_CHN_RLS_CHN_OFFSET) | (1 << GDMA_CHN_RLS_EN_OFFSET);
    //write_reg_word(reg, value);
	airoha_fe_wr(eth, offset, value);
#endif

    return 0;
}



/* Check occurrence of FE HW bug, in case of which channel retire can not be performed */
static int fe_api_pse_iq_abnormal(void)
{
	struct airoha_eth *eth = glb_eth;
    uint32 pse_iq_cnt = 0, pse_iq_stat1 = 0, pse_iq_stat2 = 0;
   
    //pse_iq_stat1 = read_reg_word(0xbfb50110);
    //pse_iq_stat2 = read_reg_word(0xbfb50114);
	pse_iq_stat1 = airoha_fe_rr(eth, PSE_IQ_STA1);
	
    pse_iq_stat2 = airoha_fe_rr(eth, PSE_IQ_STA1);
	
    pse_iq_cnt = (pse_iq_stat1 & 0xff) + ((pse_iq_stat1 >> 8) & 0xff) + ((pse_iq_stat1 >> 16) & 0xff) + ((pse_iq_stat2 >> 8) & 0xff);

    if (pse_iq_cnt > 0xA0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


int fe_get_hwfwd_channel(struct ecnt_fe_data *fe_data)
{
	struct airoha_eth *eth = glb_eth;
	unsigned int	offset, value ;
	FE_Cdma_Sel_t	cdm_sel = fe_data->cdm_sel;
	uint			channel = fe_data->channel;

	if (cdm_sel == FE_CDM_SEL_CDMA1){
		offset = CDMA1_HWF_CHN_EN;
	}else if(cdm_sel == FE_CDM_SEL_CDMA2){
		offset = CDMA2_HWF_CHN_EN;
	}else{
		return -1;
	}

	//value = read_reg_word(offset);
	value = airoha_fe_rr(eth, offset);

	value &= (1 << channel);

	fe_data->api_data.enable = (value >> channel);

	return 0;
}


int fe_api_set_channel_retire(struct ecnt_fe_data *fe_data)
{
	struct airoha_eth *eth = glb_eth;
	unsigned int reg,reg2,value,mask,ret = 0;
	unsigned int txreg,rxreg,txchn,rxchn;
	FE_Enable_t rxHwfwd;
	struct ecnt_fe_data cdm_fe_data;
	FE_Gdma_Sel_t gdm_sel = fe_data->gdm_sel;
	uint channel = fe_data->channel;
	uint mode = fe_data->reg_val;

    if (isEN751221 && fe_api_pse_iq_abnormal())
    {
        printk("Info: fe_api_set_channel_retire ingored due to PSE IQ resource abnormal\n");
        return 0;
    }

	if (gdm_sel == FE_GDM_SEL_GDMA1){
		reg = GDMA1_CHN_RLS;
		reg2 = GDMA1_TX_CHN_VLD;
		txreg = GDMA1_TXCHN_EN;
		rxreg = GDMA1_RXCHN_EN;
	}else if(gdm_sel == FE_GDM_SEL_GDMA2){
		reg = GDMA2_CHN_RLS;
		reg2= GDMA2_TX_CHN_VLD;
		txreg = GDMA2_TXCHN_EN;
		rxreg = GDMA2_RXCHN_EN;		
	}else{
		return -1;
	}	
	
	//mask = read_reg_word(reg2);
	mask = airoha_fe_rr(eth, reg2);
	
	/* this comment to be removed later 
	if ( ((1<< channel)& mask) == 0)
		return 0;
	*/
	if (gdm_sel == FE_GDM_SEL_GDMA1) {
		cdm_fe_data.cdm_sel = FE_CDM_SEL_CDMA1;
		cdm_fe_data.channel = channel;
		
		fe_get_hwfwd_channel(&cdm_fe_data);
		rxHwfwd = cdm_fe_data.api_data.enable;

		cdm_fe_data.api_data.enable = FE_DISABLE;
		fe_set_hwfwd_channel(&cdm_fe_data);
	}
	else {
		cdm_fe_data.cdm_sel = FE_CDM_SEL_CDMA2;
		cdm_fe_data.channel = channel;
		
		fe_get_hwfwd_channel(&cdm_fe_data);
		rxHwfwd = cdm_fe_data.api_data.enable;
		
		cdm_fe_data.api_data.enable = FE_DISABLE;
		fe_set_hwfwd_channel(&cdm_fe_data);
	}

	if (mode == FE_LINKDOWN){
		//txchn = read_reg_word(txreg);
		//rxchn = read_reg_word(rxreg);
		txchn = airoha_fe_rr(eth, txreg);
		rxchn = airoha_fe_rr(eth, rxreg);
		//write_reg_word(txreg,0);
		//write_reg_word(rxreg,0);
		airoha_fe_wr(eth, txreg, 0);
		airoha_fe_wr(eth, rxreg, 0);
	}

	value = (channel << GDMA_CHN_RLS_CHN_OFFSET) | (1 << GDMA_CHN_RLS_EN_OFFSET);
	
	//write_reg_word(reg,value);
	airoha_fe_wr(eth, reg, value);
	
	mdelay(1);
	
	value = 0;
		
	while( ( (airoha_fe_rr(eth, reg) & (1 << GDMA_CHN_RLS_STAT_OFFSET)) == 0 
				||  (airoha_fe_rr(eth, reg2) & (1 << channel)) != 0 )
				&& (value++ < GDMA_CHN_RLS_TIMEOUT)){
		mdelay(1);
	}
	
	if (value >= GDMA_CHN_RLS_TIMEOUT){
		printk("fe_api_set_channel_retire: timeout \n");
		ret = -1;
	}

	//write_reg_word(reg,0);
	airoha_fe_wr(eth, reg, 0);
	
	if (mode == FE_LINKDOWN){
		//write_reg_word(txreg,txchn);
		//write_reg_word(rxreg,rxchn);
		airoha_fe_wr(eth, txreg, txchn);
		airoha_fe_wr(eth, rxreg, rxchn);
	}
	
	if (gdm_sel == FE_GDM_SEL_GDMA1) {
		cdm_fe_data.cdm_sel = FE_CDM_SEL_CDMA1;
		cdm_fe_data.channel = channel;
		cdm_fe_data.api_data.enable = rxHwfwd;
		fe_set_hwfwd_channel(&cdm_fe_data);
	} else {
		cdm_fe_data.cdm_sel = FE_CDM_SEL_CDMA2;
		cdm_fe_data.channel = channel;
		cdm_fe_data.api_data.enable = rxHwfwd;
		fe_set_hwfwd_channel(&cdm_fe_data);
	}
	
	return ret;

}



int fe_api_set_ether_type(struct ecnt_fe_data *fe_data)
{
	struct airoha_eth *eth = glb_eth;
	unsigned long int val, type;
	uint index = fe_data->index;
	FE_Enable_t enable = fe_data->api_data.eth_cfg.enable;
	FE_PPPOE_t is_pppoe = fe_data->api_data.eth_cfg.is_pppoe;
	uint value = fe_data->api_data.eth_cfg.value;
	
	if(index > 15){
		printk("Error index %d, should be 0~15!\n", index);
		return 1;
	}
	//val = read_reg_word(L2BR_ETYPE_EN);
	val = airoha_fe_rr(eth, L2BR_ETYPE_EN);
	
	if(enable == FE_DISABLE){
		if(is_pppoe){	
			val &= ~(1<<(index+16));
		}
			val &= ~(1<<index);
	}else{
		if(is_pppoe){
			val |= (1<<(index+16));
		}
			val |= (1<<index);
	}

	//write_reg_word(L2BR_ETYPE_EN, val);
	airoha_fe_wr(eth, L2BR_ETYPE_EN, val);
	
	//type = read_reg_word(L2BR_ETYPE_N(index));
	type = airoha_fe_rr(eth, L2BR_ETYPE_N(index));
	
	type &= ~(0xffff << (16*(index%2)));
	type |= value << (16*(index%2));
	//write_reg_word(L2BR_ETYPE_N(index), type);
	airoha_fe_wr(eth, L2BR_ETYPE_N(index), type);
	
	return 0;
}


int fe_api_set_clear_mib(struct ecnt_fe_data *fe_data)
{
	struct airoha_eth *eth = glb_eth;
	unsigned long int base_addr, val, offset;
	FE_Gdma_Sel_t gdm_sel = fe_data->gdm_sel;
	FE_TXRX_Sel_t txrx_sel = fe_data->txrx_sel;
	
	if(gdm_sel == FE_GDM_SEL_GDMA1){
		base_addr = GDMA1_MIB_CLER;
	}else{
		base_addr = GDMA2_MIB_CLER;
	}
	offset = txrx_sel;
	
	//val = read_reg_word(base_addr);
	val = airoha_fe_rr(eth, base_addr);
	
	val |= (1<<offset);
	
	//write_reg_word(base_addr, val);
	airoha_fe_wr(eth, base_addr, val);
	
	return 0;
}



int fe_set_tx_favor_oam_enable(unchar enable)
{
	struct airoha_eth *eth = glb_eth;
	unsigned int val;
	
	if((0 != enable)&&(1 != enable))
		return -1;
		
	if(!isEN7526c && !isEN751627)
		return -1;

	//val = read_reg_word(GDMA2_FWD_CFG);
	val = airoha_fe_rr(eth, GDMA2_FWD_CFG);

	val &= ~(1 << GDMA2_TX_FAVOR_OAM_OFFSET);
	val |= enable<<GDMA2_TX_FAVOR_OAM_OFFSET;

	//write_reg_word(GDMA2_FWD_CFG,val);
	airoha_fe_wr(eth, GDMA2_FWD_CFG, val);

	return 0;
}

int fe_api_set_tx_favor_oam_enable(struct ecnt_fe_data *fe_data)
{
	unchar enable = fe_data->api_data.enable;
	fe_data->retValue = fe_set_tx_favor_oam_enable(enable);	
	return 0;
}



int fe_api_set_gdma_misc_config(struct ecnt_fe_data *fe_data)
{
	struct airoha_eth *eth = glb_eth;
	FE_Gdma_Sel_t gdm_sel = fe_data->gdm_sel;
	uint mode = fe_data->api_data.fe_misc_cfg;

    //if(isEN7523){ need to revisit again and modify 
	if(1){
		if (FE_GDM_SEL_GDMA2 == gdm_sel){
			if(FE_MISC_CONFIG_GPON == mode){
				//IO_SBITS(GDMA_MISC_CFG,GDMA2_RLS_MODE_BIT);
				airoha_fe_set(eth, GDMA_MISC_CFG, GDMA2_RLS_MODE_BIT);	
			}else{
				//IO_CBITS(GDMA_MISC_CFG,GDMA2_RLS_MODE_BIT);
				airoha_fe_clear(eth, GDMA_MISC_CFG, GDMA2_RLS_MODE_BIT);	
			}
		}
	}
	else{
		return 0;
	}
	
	return 0;
}

int fe_api_set_pkt_length(struct ecnt_fe_data *fe_data)
{
	struct airoha_eth *eth = glb_eth;
	unsigned long int base_addr, val;
	FE_Gdma_Sel_t gdm_sel = fe_data->gdm_sel;
	uint length_long = fe_data->api_data.pkt_len.length_long;
	uint length_short = fe_data->api_data.pkt_len.length_short;

	if(gdm_sel == FE_GDM_SEL_GDMA1){
		base_addr = GDMA1_LEN_CFG;
	}else if(gdm_sel == FE_GDM_SEL_GDMA2){
	    base_addr = GDMA2_LEN_CFG;
    }else if(gdm_sel == FE_GDM_SEL_GDMA3){
		base_addr = GDMA3_LEN_CFG;
	}else if(gdm_sel == FE_GDM_SEL_GDMA4){
		base_addr = GDMA4_LEN_CFG;
	}else{
		printk("input gdm_sel error gdm_sel=%d\n",gdm_sel);
		return 0;
	}
	
	//val = read_reg_word(base_addr);
	val = airoha_fe_rr(eth, base_addr);
	
	if(length_long != 0)
		val &= ~(0xffff<<16);
	if(length_short != 0)
		val &= ~(0xffff);
	
	val |= ((length_long << 16) | length_short);
	//write_reg_word(base_addr, val);
	airoha_fe_wr(eth, base_addr, val);

	return 0;
}



int fe_api_get_rx_drop_fifo_cnt(struct ecnt_fe_data *fe_data)
{
	struct airoha_eth *eth = glb_eth;
	unsigned long int base_addr;
	FE_Gdma_Sel_t gdm_sel = fe_data->gdm_sel;
	
	if(gdm_sel == FE_GDM_SEL_GDMA1){
		base_addr = GDMA1_RX_OVER_DROP_CNT;
	}else{
		base_addr = GDMA2_RX_OVDROPCNT;
	}

	//fe_data->cnt = read_reg_word(base_addr);
	fe_data->cnt = airoha_fe_rr(eth, base_addr);
	
	return 0;
}

int fe_api_get_eth_rx_cnt(struct ecnt_fe_data *fe_data)
{
	struct airoha_eth *eth = glb_eth;
	if (GDM2_RX_ETH_MIB_SUPPORT)
	{
		unsigned long long gdm2_rx_high = 0;
		gdm2_rx_high = airoha_fe_rr(eth, GDMA2_RX_OKCNT_H);
		fe_data->api_data.FE_RxCnt.rxOKPktCnt = ((gdm2_rx_high << 32) + airoha_fe_rr(eth, GDMA2_RX_OKCNT));
		fe_data->api_data.FE_RxCnt.rxFCDropCnt = airoha_fe_rr(eth, GDMA2_RX_FCDROPCNT);
		fe_data->api_data.FE_RxCnt.rxRCDropCnt = airoha_fe_rr(eth, GDMA2_RX_RCDROPCNT);
		fe_data->api_data.FE_RxCnt.rxOVDropCnt = airoha_fe_rr(eth, GDMA2_RX_OVDROPCNT);
		fe_data->api_data.FE_RxCnt.rxERRDropCnt= airoha_fe_rr(eth, GDMA2_RX_ERRDROPCNT);

		gdm2_rx_high = airoha_fe_rr(eth, GDMA2_RX_OKBYTECNT_H);
		fe_data->api_data.FE_RxCnt.rxOKByteCnt = ((gdm2_rx_high << 32) + airoha_fe_rr(eth, GDMA2_RX_OKBYTECNT));
		fe_data->api_data.FE_RxCnt.rxOversizeCnt = airoha_fe_rr(eth, GDMA2_RX_ETHLONGCNT);
		fe_data->api_data.FE_RxCnt.rxUnderSizeCnt = airoha_fe_rr(eth, GDMA2_RX_ETHRUNTCNT);

		gdm2_rx_high = airoha_fe_rr(eth, GDMA2_RX_ETHERPCNT_H);
		fe_data->api_data.FE_RxCnt.rxFrameCnt = ((gdm2_rx_high << 32) + airoha_fe_rr(eth, GDMA2_RX_ETHERPCNT));

		gdm2_rx_high = airoha_fe_rr(eth, GDMA2_RX_ETHERPLEN_H);
		fe_data->api_data.FE_RxCnt.rxFrameLen = ((gdm2_rx_high << 32) + airoha_fe_rr(eth, GDMA2_RX_ETHERPLEN));
		fe_data->api_data.FE_RxCnt.rxDropCnt = (gdm2_rx_drop_high << 32) + airoha_fe_rr(eth, GDMA2_RX_ETHDROPCNT);
		fe_data->api_data.FE_RxCnt.rxBroadcastCnt = airoha_fe_rr(eth, GDMA2_RX_ETHBCCNT);
		fe_data->api_data.FE_RxCnt.rxMulticastCnt = airoha_fe_rr(eth, GDMA2_RX_ETHMCCNT);
		fe_data->api_data.FE_RxCnt.rxCrcCnt = airoha_fe_rr(eth, GDMA2_RX_ETHCRCCNT);
		fe_data->api_data.FE_RxCnt.rxFragFameCnt = airoha_fe_rr(eth, GDMA2_RX_ETHFRACCNT);
		fe_data->api_data.FE_RxCnt.rxJabberFameCnt = airoha_fe_rr(eth, GDMA2_RX_ETHJABCNT);
		fe_data->api_data.FE_RxCnt.rxLess64Cnt = airoha_fe_rr(eth, GDMA2_RX_ETHRUNTCNT);
		fe_data->api_data.FE_RxCnt.rxMore1518Cnt = airoha_fe_rr(eth, GDMA2_RX_ETHLONGCNT);

		gdm2_rx_high = airoha_fe_rr(eth, GDMA2_RX_ETH_64_CNT_H);
		fe_data->api_data.FE_RxCnt.rxEq64Cnt = ((gdm2_rx_high << 32) + airoha_fe_rr(eth, GDMA2_RX_ETH_64_CNT));

		gdm2_rx_high = airoha_fe_rr(eth, GDMA2_RX_ETH_65_TO_127_CNT_H);
		fe_data->api_data.FE_RxCnt.rxFrom65To127Cnt = ((gdm2_rx_high << 32) + airoha_fe_rr(eth, GDMA2_RX_ETH_65_TO_127_CNT));

		gdm2_rx_high = airoha_fe_rr(eth, GDMA2_RX_ETH_128_TO_255_CNT_H);
		fe_data->api_data.FE_RxCnt.rxFrom128To255Cnt = ((gdm2_rx_high << 32) + airoha_fe_rr(eth, GDMA2_RX_ETH_128_TO_255_CNT));

		gdm2_rx_high = airoha_fe_rr(eth, GDMA2_RX_ETH_256_TO_511_CNT_H);
		fe_data->api_data.FE_RxCnt.rxFrom256To511Cnt = ((gdm2_rx_high << 32) + airoha_fe_rr(eth, GDMA2_RX_ETH_256_TO_511_CNT));

		gdm2_rx_high = airoha_fe_rr(eth, GDMA2_RX_ETH_512_TO_1023_CNT_H);
		fe_data->api_data.FE_RxCnt.rxFrom512To1023Cnt = ((gdm2_rx_high << 32) + airoha_fe_rr(eth, GDMA2_RX_ETH_512_TO_1023_CNT));

		gdm2_rx_high = airoha_fe_rr(eth, GDMA2_RX_ETH_1024_TO_1518_CNT_H);
		fe_data->api_data.FE_RxCnt.rxFrom1024To1518Cnt = ((gdm2_rx_high << 32) + airoha_fe_rr(eth, GDMA2_RX_ETH_1024_TO_1518_CNT));
	}
	else
	{
		fe_data->api_data.FE_RxCnt.rxOKPktCnt = airoha_fe_rr(eth, GDMA2_RX_OKCNT);
		fe_data->api_data.FE_RxCnt.rxFCDropCnt = airoha_fe_rr(eth, GDMA2_RX_FCDROPCNT);
		fe_data->api_data.FE_RxCnt.rxRCDropCnt = airoha_fe_rr(eth, GDMA2_RX_RCDROPCNT);
		fe_data->api_data.FE_RxCnt.rxOVDropCnt = airoha_fe_rr(eth, GDMA2_RX_OVDROPCNT);
		fe_data->api_data.FE_RxCnt.rxERRDropCnt= airoha_fe_rr(eth, GDMA2_RX_ERRDROPCNT);
		fe_data->api_data.FE_RxCnt.rxOKByteCnt = airoha_fe_rr(eth, GDMA2_RX_OKBYTECNT);
		fe_data->api_data.FE_RxCnt.rxOversizeCnt = airoha_fe_rr(eth, GDMA2_RX_ETHLONGCNT);
		fe_data->api_data.FE_RxCnt.rxUnderSizeCnt = airoha_fe_rr(eth, GDMA2_RX_ETHRUNTCNT);
		fe_data->api_data.FE_RxCnt.rxFrameCnt = airoha_fe_rr(eth, GDMA2_RX_ETHERPCNT);
		fe_data->api_data.FE_RxCnt.rxFrameLen = (gdm2_rx_len_high<<32) + airoha_fe_rr(eth, GDMA2_RX_ETHERPLEN);
		fe_data->api_data.FE_RxCnt.rxDropCnt = airoha_fe_rr(eth, GDMA2_RX_ETHDROPCNT);
		fe_data->api_data.FE_RxCnt.rxBroadcastCnt = airoha_fe_rr(eth, GDMA2_RX_ETHBCCNT);
		fe_data->api_data.FE_RxCnt.rxMulticastCnt = airoha_fe_rr(eth, GDMA2_RX_ETHMCCNT);
		fe_data->api_data.FE_RxCnt.rxCrcCnt = airoha_fe_rr(eth, GDMA2_RX_ETHCRCCNT);
		fe_data->api_data.FE_RxCnt.rxFragFameCnt = airoha_fe_rr(eth, GDMA2_RX_ETHFRACCNT);
		fe_data->api_data.FE_RxCnt.rxJabberFameCnt = airoha_fe_rr(eth, GDMA2_RX_ETHJABCNT);
		fe_data->api_data.FE_RxCnt.rxLess64Cnt = airoha_fe_rr(eth, GDMA2_RX_ETHRUNTCNT);
		fe_data->api_data.FE_RxCnt.rxMore1518Cnt = airoha_fe_rr(eth, GDMA2_RX_ETHLONGCNT);
		fe_data->api_data.FE_RxCnt.rxEq64Cnt = airoha_fe_rr(eth, GDMA2_RX_ETH_64_CNT);
		fe_data->api_data.FE_RxCnt.rxFrom65To127Cnt = airoha_fe_rr(eth, GDMA2_RX_ETH_65_TO_127_CNT);
		fe_data->api_data.FE_RxCnt.rxFrom128To255Cnt = airoha_fe_rr(eth, GDMA2_RX_ETH_128_TO_255_CNT);
		fe_data->api_data.FE_RxCnt.rxFrom256To511Cnt = airoha_fe_rr(eth, GDMA2_RX_ETH_256_TO_511_CNT);
		fe_data->api_data.FE_RxCnt.rxFrom512To1023Cnt = airoha_fe_rr(eth, GDMA2_RX_ETH_512_TO_1023_CNT);
		fe_data->api_data.FE_RxCnt.rxFrom1024To1518Cnt = airoha_fe_rr(eth, GDMA2_RX_ETH_1024_TO_1518_CNT);
	}

	return 0;
}


int fe_api_get_eth_tx_cnt(struct ecnt_fe_data *fe_data)
{
	struct airoha_eth *eth = glb_eth;
	if (GDM2_TX_ETH_MIB_SUPPORT)
	{
		unsigned long long gdm2_tx_high = 0;
		gdm2_tx_high = airoha_fe_rr(eth, GDMA2_TX_ETHCNT_H);
		fe_data->api_data.FE_TxCnt.txFrameCnt = ((gdm2_tx_high << 32) + airoha_fe_rr(eth, GDMA2_TX_ETHCNT));

		gdm2_tx_high = airoha_fe_rr(eth, GDMA2_TX_ETHLENCNT_H);
		fe_data->api_data.FE_TxCnt.txFrameLen = ((gdm2_tx_high << 32) + airoha_fe_rr(eth, GDMA2_TX_ETHLENCNT));
		fe_data->api_data.FE_TxCnt.txDropCnt =  (gdm2_tx_drop_high << 32) + airoha_fe_rr(eth, GDMA2_TX_ETHDROPCNT);
		fe_data->api_data.FE_TxCnt.txBroadcastCnt = airoha_fe_rr(eth, GDMA2_TX_ETHBCDCNT);
		fe_data->api_data.FE_TxCnt.txMulticastCnt = airoha_fe_rr(eth, GDMA2_TX_ETHMULTICASTCNT);
		fe_data->api_data.FE_TxCnt.txLess64Cnt = airoha_fe_rr(eth, GDMA2_TX_ETH_LESS64_CNT);
		fe_data->api_data.FE_TxCnt.txMore1518Cnt = airoha_fe_rr(eth, GDMA2_TX_ETH_MORE1518_CNT);

		gdm2_tx_high = airoha_fe_rr(eth, GDMA2_TX_ETH_64_CNT_H);
		fe_data->api_data.FE_TxCnt.txEq64Cnt = ((gdm2_tx_high << 32) + airoha_fe_rr(eth, GDMA2_TX_ETH_64_CNT));

		gdm2_tx_high = airoha_fe_rr(eth, GDMA2_TX_ETH_65_TO_127_CNT_H);
		fe_data->api_data.FE_TxCnt.txFrom65To127Cnt = ((gdm2_tx_high << 32) + airoha_fe_rr(eth, GDMA2_TX_ETH_65_TO_127_CNT));

		gdm2_tx_high = airoha_fe_rr(eth, GDMA2_TX_ETH_128_TO_255_CNT_H);
		fe_data->api_data.FE_TxCnt.txFrom128To255Cnt = ((gdm2_tx_high << 32) + airoha_fe_rr(eth, GDMA2_TX_ETH_128_TO_255_CNT));

		gdm2_tx_high = airoha_fe_rr(eth, GDMA2_TX_ETH_256_TO_511_CNT_H);
		fe_data->api_data.FE_TxCnt.txFrom256To511Cnt = ((gdm2_tx_high << 32) + airoha_fe_rr(eth, GDMA2_TX_ETH_256_TO_511_CNT));

		gdm2_tx_high = airoha_fe_rr(eth, GDMA2_TX_ETH_512_TO_1023_CNT_H);
		fe_data->api_data.FE_TxCnt.txFrom512To1023Cnt = ((gdm2_tx_high << 32) + airoha_fe_rr(eth, GDMA2_TX_ETH_512_TO_1023_CNT));

		gdm2_tx_high = airoha_fe_rr(eth, GDMA2_TX_ETH_1024_TO_1518_CNT_H);
		fe_data->api_data.FE_TxCnt.txFrom1024To1518Cnt = ((gdm2_tx_high << 32) + airoha_fe_rr(eth, GDMA2_TX_ETH_1024_TO_1518_CNT));
	}
	else
	{
		fe_data->api_data.FE_TxCnt.txFrameCnt = airoha_fe_rr(eth, GDMA2_TX_ETHCNT);
		fe_data->api_data.FE_TxCnt.txFrameLen = (gdm2_tx_len_high<<32) + airoha_fe_rr(eth, GDMA2_TX_ETHLENCNT);
		fe_data->api_data.FE_TxCnt.txDropCnt = airoha_fe_rr(eth, GDMA2_TX_ETHDROPCNT);
		fe_data->api_data.FE_TxCnt.txBroadcastCnt = airoha_fe_rr(eth, GDMA2_TX_ETHBCDCNT);
		fe_data->api_data.FE_TxCnt.txMulticastCnt = airoha_fe_rr(eth, GDMA2_TX_ETHMULTICASTCNT);
		fe_data->api_data.FE_TxCnt.txLess64Cnt = airoha_fe_rr(eth, GDMA2_TX_ETH_LESS64_CNT);
		fe_data->api_data.FE_TxCnt.txMore1518Cnt = airoha_fe_rr(eth, GDMA2_TX_ETH_MORE1518_CNT);
		fe_data->api_data.FE_TxCnt.txEq64Cnt = airoha_fe_rr(eth, GDMA2_TX_ETH_64_CNT);
		fe_data->api_data.FE_TxCnt.txFrom65To127Cnt = airoha_fe_rr(eth, GDMA2_TX_ETH_65_TO_127_CNT);
		fe_data->api_data.FE_TxCnt.txFrom128To255Cnt = airoha_fe_rr(eth, GDMA2_TX_ETH_128_TO_255_CNT);
		fe_data->api_data.FE_TxCnt.txFrom256To511Cnt = airoha_fe_rr(eth, GDMA2_TX_ETH_256_TO_511_CNT);
		fe_data->api_data.FE_TxCnt.txFrom512To1023Cnt = airoha_fe_rr(eth, GDMA2_TX_ETH_512_TO_1023_CNT);
		fe_data->api_data.FE_TxCnt.txFrom1024To1518Cnt = airoha_fe_rr(eth, GDMA2_TX_ETH_1024_TO_1518_CNT);
	}

	return 0;
}




int mbi_hang_unlock_by_terminate(FE_Gdma_Sel_t idx)
{
	struct airoha_eth *eth = glb_eth;
    unsigned int reg=0, value=0, cnt=0, mbi_ok_flag=0;
    
    if (idx == FE_GDM_SEL_GDMA1) {
        reg = GDMA1_CHN_RLS;
    } else if(idx == FE_GDM_SEL_GDMA2) {
        reg = GDMA2_CHN_RLS;
    } else {
        return -1;
    }

    cnt = mbi_ok_flag = 0;
    do {
        if((airoha_fe_rr(eth, reg) & (1 << MBI_TX_BUSY_OFFSET)) == 0) {
            mbi_ok_flag = 1;
            break;
        }
    } while((cnt++)<=10);
    if(mbi_ok_flag == 0) {
        value = airoha_fe_rr(eth, reg);
        airoha_fe_wr(eth, reg, value | (1<<MBI_TX_TERMINATE_OFFSET) );
        cnt = 0;
        do {
            if((airoha_fe_rr(eth, reg) & (1 << MBI_TX_BUSY_OFFSET)) == 0)
                break;
        } while((cnt++)<=10);
        if(cnt>10) {
            printk("Error: MBI TX Hang issue Terminate Fail!\n");
            return -1;
        }
        airoha_fe_wr(eth, reg, value & (~(1<<MBI_TX_TERMINATE_OFFSET)) );
    }
    
    cnt = mbi_ok_flag = 0;
    do {
        if((airoha_fe_rr(eth, reg) & (1 << MBI_RX_BUSY_OFFSET)) == 0) {
            mbi_ok_flag = 1;
            break;
        }
    } while((cnt++)<=10);
    if(mbi_ok_flag == 0) {
        value = airoha_fe_rr(eth, reg);
        airoha_fe_wr(eth, reg, value | (1<<MBI_RX_TERMINATE_OFFSET) );
        cnt = 0;
        do {
            if((airoha_fe_rr(eth, reg) & (1 << MBI_RX_BUSY_OFFSET)) == 0)
                break;
        } while((cnt++)<=10);
        if(cnt>10) {
            printk("Error: MBI RX Hang issue Terminate Fail!\n");
            return -1;
        }
        airoha_fe_wr(eth, reg, value & (~(1<<MBI_RX_TERMINATE_OFFSET)) );
    }
    
    return 0;
}

static int fe_channel_drop(void)
{
	struct airoha_eth *eth = glb_eth;
	unsigned int txreg,rxreg,txchn,rxchn;
	unsigned int hwreg,hw;
	unsigned int gdma_lpbk_cfg_reg, gdma_fwd_cfg_reg ,qdma_glb_cfg_reg,gdm2_tx_cha_vld_reg;
	unsigned int gdma_lpbk_cfg,gdma_fwd_cfg,qdma_glb_cfg;
	//QDMA_TxBufCtrl_T oldtxbuff,newTxbuff;
	unsigned int reg, reg2;
	//int ret;
	unsigned int value = 0;
	#ifdef TCSUPPORT_CPU_EN7580
	FE_Gdma_Sel_t gdm_sel = FE_GDM_SEL_GDMA2;
	#endif
		
	gdma_lpbk_cfg_reg	= GDMA2_LPBP_CFG;
	gdma_fwd_cfg_reg	= GDMA2_FWD_CFG;
	qdma_glb_cfg_reg	= QDMA_GLB_CFG;
	gdm2_tx_cha_vld_reg = GDMA2_TX_CHN_VLD;
	
	txreg = GDMA2_TXCHN_EN;
	rxreg = GDMA2_RXCHN_EN; 
	hwreg = CDMA2_HWF_CHN_EN;	
	reg = GDMA2_CHN_RLS;
	reg2= GDMA2_TX_CHN_VLD;
	
	txchn = airoha_fe_rr(eth, txreg);
	rxchn = airoha_fe_rr(eth, rxreg);
	hw = airoha_fe_rr(eth, hwreg);	

	#ifdef TCSUPPORT_CPU_EN7580
	mbi_hang_unlock_by_terminate(gdm_sel);
	#endif
	
	/*************diable Cdm2/Gdm2 rx and enable Gdm2 tx*********/	  
	airoha_fe_wr(eth, txreg,0xffff);
	airoha_fe_wr(eth, rxreg,0);
	airoha_fe_wr(eth, hwreg,0);

	/****************set gdm2 loop back*****************/
	gdma_lpbk_cfg = airoha_fe_rr(eth, gdma_lpbk_cfg_reg);
	gdma_fwd_cfg = airoha_fe_rr(eth, gdma_fwd_cfg_reg);
	qdma_glb_cfg = airoha_fe_rr(eth, qdma_glb_cfg_reg);
	
	airoha_fe_wr(eth, gdma_lpbk_cfg_reg,0x4007d003);
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
	airoha_fe_wr(eth, gdma_fwd_cfg_reg,0x03f1ffff);
#else
    airoha_fe_wr(eth, gdma_fwd_cfg_reg,0x03f17777);
#endif
	airoha_fe_wr(eth, qdma_glb_cfg_reg,qdma_glb_cfg | 1<<17);
		
	
	while((airoha_fe_rr(eth, gdm2_tx_cha_vld_reg) != 0) && (value++<300)){
		mdelay(1);
	}
			
	airoha_fe_wr(eth,gdma_lpbk_cfg_reg,gdma_lpbk_cfg);
	airoha_fe_wr(eth,gdma_fwd_cfg_reg,gdma_fwd_cfg);	
	airoha_fe_wr(eth,qdma_glb_cfg_reg,qdma_glb_cfg);

	airoha_fe_wr(eth, txreg,txchn);
	airoha_fe_wr(eth,rxreg,rxchn);
	airoha_fe_wr(eth,hwreg,hw);
	
	return 0;

}


static int fe_channel_retire_one(FE_Gdma_Sel_t idx,uint chn)
{
	struct airoha_eth *eth = glb_eth;

	unsigned int txreg=0, rxreg=0, txchn=0, rxchn=0;
	unsigned int reg3=0;

	unsigned int reg=0, reg2=0, value=0, value1 = 0;
	unsigned int rlsCnt=0;
	int ret = 0;

	if (idx == FE_GDM_SEL_GDMA1){
		txreg = GDMA1_TXCHN_EN;
		rxreg = GDMA1_RXCHN_EN;
		reg = GDMA1_CHN_RLS;
		reg2 = GDMA1_TX_CHN_VLD;
		reg3 = QDMA1_CHN_VLD_BASE;
	}else if(idx == FE_GDM_SEL_GDMA2){
		txreg = GDMA2_TXCHN_EN;
		rxreg = GDMA2_RXCHN_EN;
		reg = GDMA2_CHN_RLS;
		reg2 = GDMA2_TX_CHN_VLD;
		reg3 = QDMA2_CHN_VLD_BASE;
	}else if(idx == FE_GDM_SEL_GDMA3){
        txreg = GDMA3_TXCHN_EN;
        rxreg = GDMA3_RXCHN_EN;
        reg = GDMA3_CHN_RLS;
        reg2= GDMA3_RX_CHN_VLD;
		if(HWF_QDMA_SEL_SUPPORT&&FeGetHwfQdmaSelGdm3(eth))
			reg3 = QDMA2_CHN_VLD_BASE;
		else
			reg3 = QDMA1_CHN_VLD_BASE;
	}else if(idx == FE_GDM_SEL_GDMA4){
        txreg = GDMA4_TXCHN_EN;
        rxreg = GDMA4_RXCHN_EN;
        reg = GDMA4_CHN_RLS;
        reg2= GDMA4_RX_CHN_VLD;
		if(HWF_QDMA_SEL_SUPPORT&&FeGetHwfQdmaSelGdm4(eth))
			reg3 = QDMA2_CHN_VLD_BASE;
		else
			reg3 = QDMA1_CHN_VLD_BASE;
    }else{
		return -1;
	}
	reg3 += (chn>>2)<<2;
	txchn = airoha_fe_rr(eth,txreg);
	rxchn = airoha_fe_rr(eth,rxreg);
	airoha_fe_wr(eth, rxreg, 1<<chn);
	airoha_fe_wr(eth, txreg, 1<<chn);
        
	chn &= 0x1f;
	value = (chn << GDMA_CHN_RLS_CHN_OFFSET) | (1 << GDMA_CHN_RLS_EN_OFFSET);
	airoha_fe_wr(eth, reg, value);

	if(reg3 == QDMA2_CHN_VLD_BASE)
	{
		value1 = airoha_qdma_rr(&eth->qdma[1],QDMA_CHN_VLD_OFFSET) & (0xFF<<((chn&0x3)<<3));
	}
	else
	{		
		value1 = airoha_qdma_rr(&eth->qdma[0],QDMA_CHN_VLD_OFFSET) & (0xFF<<((chn&0x3)<<3));
	}
	
retry:
	
	mdelay(1);
    
	rlsCnt++;

	if (rlsCnt < GDMA_CHN_RLS_TIMEOUT)
	{
		for(value = 0; value < 2; value++)
		{
			if ( ((airoha_fe_rr(eth,reg) & (1 << GDMA_CHN_RLS_STAT_OFFSET)) == 0)
				|| ((airoha_fe_rr(eth,reg2) & (1 << chn)) != 0)
				|| (value1 != 0) ) {
				goto retry;
			}
		}
	} else {
		ret = -1;
	}
    
	airoha_fe_wr(eth, reg, 0);
	airoha_fe_wr(eth, txreg, txchn);
	airoha_fe_wr(eth, rxreg, rxchn); 

	return ret;
}

#define FE_PROBE_H          (PSE_BASE + 0x34)
#define GDMA1_RX_CHN_VLD     (GDM1_BASE + 0x74)
#define GDMA2_RX_CHN_VLD     (GDM2_BASE + 0x74)

static int fe_channel_retire(struct ecnt_fe_data *fe_data)
{
#if !defined(TCSUPPORT_CPU_EN7580)
	unsigned int txreg=0, rxreg=0, txchn=0, rxchn=0;
	unsigned int hw=0;
	QDMA_TxBufCtrl_T oldtxbuff,newTxbuff;
#else
	unsigned int qdmaChnlEn[8];
#endif
	struct airoha_eth *eth = glb_eth;
	
	unsigned int hwreg=0, i=0, j=0;
	unsigned int reg=0, reg2=0;
	int ret=0;

	FE_Gdma_Sel_t gdm_sel = fe_data->gdm_sel;
	uint channel = fe_data->channel;

#ifdef TCSUPPORT_CPU_EN7580
    if (gdm_sel == FE_GDM_SEL_GDMA1){
        hwreg = QDMA1_CHN_EN_BASE;
        reg = GDMA1_CHN_RLS;
        reg2 = GDMA1_RX_CHN_VLD;
    }else if(gdm_sel == FE_GDM_SEL_GDMA2){
        hwreg = QDMA2_CHN_EN_BASE;
        reg = GDMA2_CHN_RLS;
        reg2= GDMA2_RX_CHN_VLD;
	}else if(gdm_sel == FE_GDM_SEL_GDMA3){
		if(HWF_QDMA_SEL_SUPPORT&&FeGetHwfQdmaSelGdm3(eth))
			hwreg = QDMA2_CHN_EN_BASE;
		else
			hwreg = QDMA1_CHN_EN_BASE;
        reg = GDMA3_CHN_RLS;
        reg2= GDMA3_RX_CHN_VLD;
	}else if(gdm_sel == FE_GDM_SEL_GDMA4){
		if(HWF_QDMA_SEL_SUPPORT&&FeGetHwfQdmaSelGdm4(eth))
			hwreg = QDMA2_CHN_EN_BASE;
		else
			hwreg = QDMA1_CHN_EN_BASE;
        reg = GDMA4_CHN_RLS;
        reg2= GDMA4_RX_CHN_VLD;
    }else{
        return -1;
    }
    
    for(i=0; i<8; i++) {
        qdmaChnlEn[i] = airoha_fe_rr(eth,hwreg+(i<<2));
        airoha_fe_wr(eth, hwreg+(i<<2), 0);
    }
    
     
	mbi_hang_unlock_by_terminate(gdm_sel);

    mdelay(1);

    for(i=0; i<channel; i++) {
        ret = fe_channel_retire_one(gdm_sel,i);
        if(ret == -1) {
            printk("fe_channel_retire_one(%d/%d): timeout (%08x) ,(%08x) \n", i, gdm_sel, airoha_fe_rr(eth,reg), airoha_fe_rr(eth,reg2));
        }
    }
    
    for(j=0; j<10; j++) {
        if (airoha_fe_rr(eth,reg2) == 0) {
            break;
        }
    }
    if(j==10) {
        printk("rx channel vld is error: rx_chnl_vld:%08x\n", airoha_fe_rr(eth,reg2));
    }
    
    for(i=0; i<8; i++) {
        airoha_fe_wr(eth, hwreg+(i<<2), qdmaChnlEn[i]);
    }
#else
	if (gdm_sel == FE_GDM_SEL_GDMA1){
		txreg = GDMA1_TXCHN_EN;
		rxreg = GDMA1_RXCHN_EN;
		hwreg = CDMA1_HWF_CHN_EN;
		reg = GDMA1_CHN_RLS;
		reg2 = GDMA1_TX_CHN_VLD;
	}else if(gdm_sel == FE_GDM_SEL_GDMA2){
		txreg = GDMA2_TXCHN_EN;
		rxreg = GDMA2_RXCHN_EN;	
		hwreg = CDMA2_HWF_CHN_EN;	
		reg = GDMA2_CHN_RLS;
		reg2= GDMA2_TX_CHN_VLD;
	}else{
		return -1;
	}	

	txchn = airoha_fe_rr(eth,txreg);
	
	rxchn = airoha_fe_rr(eth,rxreg);
	
	hw = airoha_fe_rr(eth,hwreg);
	
	QDMA_API_GET_TXBUF_THRESHOLD(&oldtxbuff);

	airoha_fe_wr(eth,hwreg,0);

	newTxbuff.mode = QDMA_ENABLE;
	newTxbuff.chnThreshold = 1;
	newTxbuff.totalThreshold = 0x40;
	QDMA_API_SET_TXBUF_THRESHOLD(&newTxbuff);

	airoha_fe_wr(eth,rxreg,0);
	
	airoha_fe_wr(eth,txreg,0);
	
	mdelay(1);

	j = 0;
	while(j < 2)
	{
		for(i = 0; i < channel; i++)
		{
			ret = fe_channel_retire_one(gdm_sel,i);
			if(ret == -1 && j == 1) {
				printk("fe_channel_retire_one(%d/%d): timeout (%08x) ,(%08x) ,(%08x) \n",i,gdm_sel,airoha_fe_rr(eth,reg),airoha_fe_rr(eth,reg2),airoha_fe_rr(eth,FE_PROBE_H));
			}
		}
		j++;
	}
	
	airoha_fe_wr(eth,txreg,txchn);
	
	airoha_fe_wr(eth,rxreg,rxchn);
	
	QDMA_API_SET_TXBUF_THRESHOLD(&oldtxbuff);

	airoha_fe_wr(eth,hwreg,hw);
#endif

	return 0;
}


/*
    used when wan link is down,and called by Wan MAC Driver. 
    if channel_retire is 1, this function will retire each channel twice by two loops. First loop retire the  channels queued in PSE buffer,
    these channels will be retired done at first loop. Second loop will given the chance to those channels not queued in PSE buffer but
    queued in QDMA TX queue. All Channels will be retired done at second loop;
    esle, this function will do channel drop. the packet queued in qdma will be sent to gdma2, and set the force port as drop.
    the default set channel_retire is 0, and this function will call channel drop
*/
int fe_api_set_channel_retire_all(struct ecnt_fe_data *fe_data)
{
    if (isEN751221 && fe_api_pse_iq_abnormal())
    {
        printk("Info: fe_api_set_channel_retire_all ingored due to PSE IQ resource abnormal\n");
        return 0;
    }

	atomic_set(&qdma_stop_flag, 1);
	if(channel_retire != CHANNEL_RETIRE)
		fe_channel_drop();
	else 
		fe_channel_retire(fe_data);
	atomic_set(&qdma_stop_flag, 0);

	return 0;
}



int fe_api_set_channel_retire_one(struct ecnt_fe_data *fe_data)
{
	struct airoha_eth *eth = glb_eth;
    unsigned int hwreg=0;
	FE_Gdma_Sel_t gdm_sel = fe_data->gdm_sel;
	uint channel = fe_data->channel;

    if (isEN751221 && fe_api_pse_iq_abnormal())
    {
        printk("Info: fe_api_set_channel_retire_one ingored due to PSE IQ resource abnormal\n");
        return 0;
    }

    if (gdm_sel == FE_GDM_SEL_GDMA1){
        hwreg = QDMA1_CHN_EN_BASE;
    }else if(gdm_sel == FE_GDM_SEL_GDMA2){
        hwreg = QDMA2_CHN_EN_BASE;		
	}else if(gdm_sel == FE_GDM_SEL_GDMA3){
		if(HWF_QDMA_SEL_SUPPORT&&FeGetHwfQdmaSelGdm3(eth))
			hwreg = QDMA2_CHN_EN_BASE;
		else
			hwreg = QDMA1_CHN_EN_BASE;
	}else if(gdm_sel == FE_GDM_SEL_GDMA4){
		if(HWF_QDMA_SEL_SUPPORT&&FeGetHwfQdmaSelGdm4(eth))
			hwreg = QDMA2_CHN_EN_BASE;
		else
			hwreg = QDMA1_CHN_EN_BASE;
    }else{
        return -1;
    }

    mbi_hang_unlock_by_terminate(gdm_sel);
    fe_channel_retire_one(gdm_sel, channel);

	return 0;
}


static inline unsigned int pse_get_oq_rsv(unsigned int port, unsigned int queue)
{
	struct airoha_eth *eth = glb_eth;
	unsigned int tmp_val; 

	//IO_SREG(PSE_QUEUE_CFG_WR, (port<<PSE_CFG_PORT_ID_SHIFT)|(queue<<PSE_CFG_QUEUE_ID_SHIFT));
	
	airoha_fe_wr(eth, PSE_QUEUE_CFG_WR, (port<<PSE_CFG_PORT_ID_SHIFT)|(queue<<PSE_CFG_QUEUE_ID_SHIFT));
	
	//tmp_val = IO_GMASK(PSE_QUEUE_CFG_VAL, PSE_CFG_OQ_RSV_MASK, PSE_CFG_OQ_RSV_SHIFT);
	
	tmp_val = ((airoha_fe_rr(eth, PSE_QUEUE_CFG_VAL) & PSE_CFG_OQ_RSV_MASK) >> PSE_CFG_OQ_RSV_SHIFT);
	

	return tmp_val;
}

	
static inline unsigned int pse_set_oq_rsv(unsigned int port, unsigned int queue, unsigned int val)
{
	struct airoha_eth *eth = glb_eth;
	//IO_SMASK(PSE_QUEUE_CFG_VAL, PSE_CFG_OQ_RSV_MASK, PSE_CFG_OQ_RSV_SHIFT, val);
	airoha_fe_rmw(eth, PSE_QUEUE_CFG_VAL, PSE_CFG_OQ_RSV_MASK, (val << PSE_CFG_OQ_RSV_SHIFT));
	
	//IO_SREG(PSE_QUEUE_CFG_WR, (port<<PSE_CFG_PORT_ID_SHIFT)|(queue<<PSE_CFG_QUEUE_ID_SHIFT)|PSE_CFG_WR_EN|PSE_CFG_OQRSV_SEL);
	airoha_fe_wr(eth, PSE_QUEUE_CFG_WR, (port<<PSE_CFG_PORT_ID_SHIFT)|(queue<<PSE_CFG_QUEUE_ID_SHIFT)|PSE_CFG_WR_EN|PSE_CFG_OQRSV_SEL);

	return 0;
}


void fe_set_per_oq_rsv(uint port,uint oq,uint val)
{
#if defined(TCSUPPORT_CPU_EN7581)
	struct airoha_eth *eth = glb_eth;
    uint ori_rsv = pse_get_oq_rsv(port,oq);
    uint tmp = 0;
    uint pse_port_oq_num[PSE_PORT_NUM] = {PSE_PORT0_QUEUE_NUM,PSE_PORT1_QUEUE_NUM,\
        PSE_PORT2_QUEUE_NUM,PSE_PORT3_QUEUE_NUM,PSE_PORT4_QUEUE_NUM,PSE_PORT5_QUEUE_NUM,\
        PSE_PORT6_QUEUE_NUM,PSE_PORT7_QUEUE_NUM,PSE_PORT8_QUEUE_NUM,PSE_PORT9_QUEUE_NUM,\
        PSE_PORT10_QUEUE_NUM};

    if(port >= PSE_PORT_NUM)
        return;

    if(oq >= pse_port_oq_num[port])
        return;
    
    pse_set_oq_rsv(port,oq,val);

    /*modify all rsv*/
    tmp = GET_PSE_ALL_RSV(eth)-ori_rsv+val;
    SET_PSE_ALL_RSV(eth,tmp);

#if defined(TCSUPPORT_CPU_AN7583)
	/*modify hthd*/
    tmp = GET_PSE_FQ_LITMI(eth)-GET_PSE_ALL_RSV(eth)-0x100;
    SET_PSE_SHARED_USED_HTHD(eth,tmp);

    /*modify mthd&lthd,mthd = hthd - 0x100,lthd = mthd*0.75*/
    tmp = tmp-0x100;
    SET_PSE_SHARED_USED_MTHD(eth,tmp);
    tmp = tmp*3/4;
    SET_PSE_SHARED_USED_LTHD(eth,tmp);

#else
    /*modify hthd*/
    tmp = GET_PSE_FQ_LITMI(eth)-GET_PSE_ALL_RSV(eth)-0x20;
    SET_PSE_SHARED_USED_HTHD(eth,tmp);

    /*modify mthd&lthd,mthd = all_share - 0x20,lthd = mthd*0.75*/
    tmp = GET_PSE_FQ_LITMI(eth)-GET_PSE_ALL_RSV(eth)-0x100;
    SET_PSE_SHARED_USED_MTHD(eth,tmp);
    tmp = tmp*3/4;
    SET_PSE_SHARED_USED_LTHD(eth,tmp);
#endif
#endif
    
    return;
}




int fe_api_pse_oq_rsv_en(struct ecnt_fe_data *fe_data)
{
#ifdef TCSUPPORT_CPU_EN7581
	unsigned int port = fe_data->api_data.fe_oq_rsv_en.port;
    unsigned int channel = fe_data->api_data.fe_oq_rsv_en.channel;
    unsigned int enable = fe_data->api_data.fe_oq_rsv_en.enable;

    /*keep P2 oq0~oq5 rsv*/
    if((FE_DP_GDM2 == port)&&(channel >= 0)&&(channel <= 5))
            return 0;
    
    if(enable)
        fe_set_per_oq_rsv(port,channel,PSE_RSV_PAGE_DEFAULT);
    else
        fe_set_per_oq_rsv(port,channel,0); 
    
#endif

	return 0;
}


// Function definitions
int fe_api_set_mac_addr(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_crc_strip(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_padding(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_ext_tpid(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_ext_tpid(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_fw_cfg(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_fw_cfg(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_drop_udp_chksum_err_enable(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_drop_tcp_chksum_err_enable(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_drop_ip_chksum_err_enable(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_drop_crc_err_enable(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_drop_runt_enable(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_drop_long_enable(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_vlan_check(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_ok_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_rx_err_crc_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_rx_drop_err_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_ok_byte_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_tx_get_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_tx_drop_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_time_stamp(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_time_stamp(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_ins_vlan_tpid(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_vlan_enable(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_black_list(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_L2U_key(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_ac_group_pkt_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_ac_group_byte_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_clear_ac_group_pkt_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_clear_ac_group_byte_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_meter_group(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_meter_group(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_gdm_pcp_coding(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_cdm_pcp_coding(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_vip_enable(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_eth_frame_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_eth_err_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_cdm_rx_red_drop_mode(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_cdm_rx_red_drop_mode(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_tx_rate(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_rxuc_rate(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_rxbc_rate(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_rxmc_rate(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_rxoc_rate(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_add_vip_ether(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_add_vip_ppp(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_add_vip_ip(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_add_vip_tcp(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_add_vip_udp(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_del_vip_ether(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_del_vip_ppp(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_del_vip_ip(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_del_vip_tcp(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_del_vip_udp(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_add_l2lu_vlan_dscp(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_add_l2lu_vlan_trfc(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_del_l2lu_vlan_dscp(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_del_l2lu_vlan_trfc(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_add_traffic_class(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_del_traffic_class(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_tls_cfg(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_tls_forwad(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_do_fe_reset(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_mac_addr_7516(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_wan_port_7516(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_loopback_enable(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_loopback_mode(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_unknown_mul_pkt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_meter_ratelimit(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_meter_ratelimit(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_meter_idx(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_acnt1_idx(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_acnt0_idx(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_init_resource_manage(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_deinit_resource_manage(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_rx_ratelimit_rule(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_rx_ratelimit_mode(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_meter_ctl_by_olt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_flow_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_clear_flow_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_acnt0_mode(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_acnt1_mode(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_acnt0_mode(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_acnt1_mode(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_meter_enable(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_dev_mac_index(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_pse_oq_threshold(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_acnt2_idx(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_acnt2_mode(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_wan_itf_index(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_glo_rate_byte(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_pppoe_info(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_pppoe_info_clean(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_tx_traffic(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_rx_traffic(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_tx_rate(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_rx_rate(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_tx_octets(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_rx_octets(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_rx_discard_counter(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_tx_discard_counter(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_rx_error_counter(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_tx_error_counter(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_add_dev_to_total_account(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_add_stb_src_ip(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_del_stb_src_ip(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_ratelimit_for_pkt_formate(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_mc_vlan_global(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_mc_vlan_global(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_mc_vlan_table_cfg(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_mc_vlan_table_cfg(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_mc_vlan_action_cfg(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_mc_vlan_action_cfg(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_mc_vlan_clear_all(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_rx_mac_filter(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_rx_mac_filter_rate(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_xfi_link_change(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_rx_ratelimit_mode(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_hsgmii_rx_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_hsgmii_tx_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_aewan_fwdfq(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_aewan_ifcdisable(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_gdm2_sptag_for_loopback(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_rx_rate(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_tunnel_cfg(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_gdm_sptag_for_extswitch(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_hsgmii_rx_port_ratelimit(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_mbi_arb_rst(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_rmbi_frag(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_chn_rls(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_tmbi_frag(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_gdma_enable(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_gdma_disable(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_get_pse_drop_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_qbi_fttr_chn_disable(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_force_slow_enable(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_force_slow_duty(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_vip_rxq_selection(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_vip_for_tcp_speedtest(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_check_chn_rls(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_dev_stat_ratelimit_mode(struct ecnt_fe_data *fe_data) {
    return 0;
}

int fe_api_set_clr_cnt(struct ecnt_fe_data *fe_data) {
    return 0;
}




void airoha_pon_ppe_init_upd_mem(const u8 *addr)
{
	u32 val;
	struct airoha_eth *eth = glb_eth;
	
	val = (addr[2] << 24) | (addr[3] << 16) | (addr[4] <<8) | addr[5];
	airoha_fe_wr(eth, REG_UPDMEM_DATA(0), val);
	airoha_fe_wr(eth, REG_UPDMEM_CTRL(0),
		     FIELD_PREP(PPE_UPDMEM_ADDR_MASK,SMAC_PON_IDX) |
		     PPE_UPDMEM_WR_MASK | PPE_UPDMEM_REQ_MASK);
	val = (addr[0] << 8) | addr[1];
	airoha_fe_wr(eth, REG_UPDMEM_DATA(0), val);
	airoha_fe_wr(eth, REG_UPDMEM_CTRL(0),
		     FIELD_PREP(PPE_UPDMEM_ADDR_MASK, SMAC_PON_IDX) |
		     FIELD_PREP(PPE_UPDMEM_OFFSET_MASK, 1) |
		     PPE_UPDMEM_WR_MASK | PPE_UPDMEM_REQ_MASK);
	UpdateShrinkTable(SMAC_PON_IDX,addr);
}
void airoha_pon_set_macaddr(const u8 *addr)
{
	u32 val, reg;
	struct airoha_eth *eth = glb_eth;

	reg = REG_FE_WAN_MAC_H;
	val = (addr[0] << 16) | (addr[1] << 8) | addr[2];
	airoha_fe_wr(eth, reg, val);

	val = (addr[3] << 16) | (addr[4] << 8) | addr[5];
	airoha_fe_wr(eth, REG_FE_MAC_LMIN(reg), val);
	airoha_fe_wr(eth, REG_FE_MAC_LMAX(reg), val);
	airoha_pon_ppe_init_upd_mem(addr);
}
EXPORT_SYMBOL(airoha_pon_set_macaddr);

/*
static ssize_t qdma_bm_rx_dma_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char val_string[64] ;
	uint cmd=0 ,i=0;
	struct ECNT_QDMA_Data qdma_data;
	struct ecnt_fe_data fe_data;
	
	int len = 0;

    if (count >= 64)
    {
        len = 63;
    }
    else 
    {
        len = count;
    }
	
	if (copy_from_user(val_string, buf, len))
		return -EFAULT ;

	sscanf(val_string, "%d", &cmd) ;

	if(cmd == 1) {
		qdma_data.qdma_private.mode = 1 ;
		qdma_rx_dma_mode(&qdma_data);
		
	}
	if(cmd == 2) {
		qdma_data.qdma_private.mode = 0 ;
		qdma_rx_dma_mode(&qdma_data);
		qdma_tx_dma_mode(&qdma_data);
	}
	else if(cmd == 3)
	{
		qdma_data.qdma_private.mode = 1 ;
		qdma_tx_dma_mode(&qdma_data);
	}
	else if(cmd == 4)
	{
		qdma_data.qdma_private.lbMode = 1 ;
		qdma_loopback_mode(&qdma_data);
	}
	else if(cmd == 5)
	{
		qdma_data.qdma_private.lbMode = 0 ;
		qdma_loopback_mode(&qdma_data);
	}
	else if(cmd == 6)
	{
		qdma_enable_rxpkt_int(&qdma_data);
	}
	else if(cmd == 7)
	{
		qdma_disable_rxpkt_int(&qdma_data);
	}
	else if(cmd == 8)
	{
		fe_data.cdm_sel = 1;
		fe_data.channel = 1;
		fe_data.api_data.enable = 0;
		fe_set_hwfwd_channel(&fe_data);
	}
	else if(cmd == 9)
	{
		QDMA_ChannelStatus_T chnlCloseStatusSet = {0};
		chnlCloseStatusSet.chnlIdx = 1;
		chnlCloseStatusSet.chnlStatus = 0x0;
		qdma_data.qdma_private.chnlCloseStatusSet = &chnlCloseStatusSet ;
		qdma_set_channel_close_status(&qdma_data);
	}
	else if(cmd == 10)
	{
		
		fe_data.api_data.enable = 0;
		// disable all tx channel 
		for(i = 0; i < 32; i++) {
			fe_data.gdm_sel = FE_GDM_SEL_GDMA2;
			fe_data.txrx_sel = FE_GDM_SEL_TX;
			fe_data.channel = i;
			fe_api_set_channel_enable(&fe_data);
		}
		// disable all rx channel 
		for(i = 0; i < 16; i++) {
			fe_data.gdm_sel = FE_GDM_SEL_GDMA2;
			fe_data.txrx_sel = FE_GDM_SEL_RX;
			fe_data.channel = i;
			fe_api_set_channel_enable(&fe_data);
		}
	}
	else if(cmd == 11)
	{
		fe_data.api_data.enable = 1;
		// disable all tx channel
		for(i = 0; i < 32; i++) {
			fe_data.gdm_sel = FE_GDM_SEL_GDMA2;
			fe_data.txrx_sel = FE_GDM_SEL_TX;
			fe_data.channel = i;
			fe_api_set_channel_enable(&fe_data);
		}
		// disable all rx channel 
		for(i = 0; i < 16; i++) {
			fe_data.gdm_sel = FE_GDM_SEL_GDMA2;
			fe_data.txrx_sel = FE_GDM_SEL_RX;
			fe_data.channel = i;
			fe_api_set_channel_enable(&fe_data);
		}
		
	}
	else if(cmd == 12)
	{
		fe_data.api_data.enable = 0;
		// disable all tx channel 
		for(i = 0; i < 32; i++) {
			fe_data.gdm_sel = FE_GDM_SEL_GDMA1;
			fe_data.txrx_sel = FE_GDM_SEL_TX;
			fe_data.channel = i;
			fe_api_set_channel_enable(&fe_data);
		}
		// disable all rx channel
		for(i = 0; i < 16; i++) {
			fe_data.gdm_sel = FE_GDM_SEL_GDMA1;
			fe_data.txrx_sel = FE_GDM_SEL_RX;
			fe_data.channel = i;
			fe_api_set_channel_enable(&fe_data);
		}
		
	}
	else if(cmd == 13)
	{
		fe_data.api_data.enable = 1;
		// disable all tx channel 
		for(i = 0; i < 32; i++) {
			fe_data.gdm_sel = FE_GDM_SEL_GDMA1;
			fe_data.txrx_sel = FE_GDM_SEL_TX;
			fe_data.channel = i;
			fe_api_set_channel_enable(&fe_data);
		}
		// disable all rx channel 
		for(i = 0; i < 16; i++) {
			fe_data.gdm_sel = FE_GDM_SEL_GDMA1;
			fe_data.txrx_sel = FE_GDM_SEL_RX;
			fe_data.channel = i;
			fe_api_set_channel_enable(&fe_data);
		}
		
	}
	else if(cmd == 14)
	{
		fe_data.gdm_sel = FE_GDM_SEL_GDMA1;
		fe_data.channel = 1;
		fe_data.api_data.link_mode = 0;
		
		fe_api_set_channel_retire(&fe_data);
		
		fe_data.gdm_sel = FE_GDM_SEL_GDMA2;
		fe_data.channel = 1;
		fe_data.api_data.link_mode = 0;
		
		fe_api_set_channel_retire(&fe_data);
	}
	else if(cmd == 15)
	{
		fe_data.gdm_sel = FE_GDM_SEL_GDMA1;
		fe_data.channel = 1;
		fe_data.api_data.link_mode = 1;
		
		fe_api_set_channel_retire(&fe_data);
		
		fe_data.gdm_sel = FE_GDM_SEL_GDMA2;
		fe_data.channel = 1;
		fe_data.api_data.link_mode = 1;
		
		fe_api_set_channel_retire(&fe_data);
	}
	
    return len;
}


static const struct proc_ops proc_qdma_bm_rx_dma_fops = {
	.proc_write	= qdma_bm_rx_dma_write_proc,
};

int eth_proc_init()
{
    struct proc_dir_entry *eth_proc=NULL;
	
	eth_proc = proc_create("proc_qdma_bm_rx_dma", 0, NULL, &proc_qdma_bm_rx_dma_fops);
	
	return 0;
	
}
EXPORT_SYMBOL(eth_proc_init);


int eth_proc_exit()
{    
    remove_proc_entry("proc_qdma_bm_rx_dma", 0);
	return 0;
}
EXPORT_SYMBOL(eth_proc_exit);
*/
#ifdef QDMA_LAN
#define ECNT_QDMA ECNT_QDMA_LAN
#define driver_qdma_hook_name "driver_qdma_lan_hook"
#else
#define ECNT_QDMA ECNT_QDMA_WAN
#define driver_qdma_hook_name "driver_qdma_wan_hook"
#endif

__IMEM ecnt_ret_val ecnt_qdma_hook(struct ecnt_data *in_data)
{
	struct ECNT_QDMA_Data *qdma_data = (struct ECNT_QDMA_Data *)in_data ;
	//ulong flags=0 ;	
	
	if(qdma_data->function_id >= QDMA_FUNCTION_MAX_NUM) {
		QDMA_ERR("qdma_data->function_id is %d, exceed max number: %d", qdma_data->function_id, QDMA_FUNCTION_MAX_NUM);
		return ECNT_HOOK_ERROR;
	}
	if(qdma_operation[qdma_data->function_id] == NULL){
		QDMA_MSG(QDMA_DBG_ST, "ecnt_qdma_hook: NULL, function_id=0x%x\n", qdma_data->function_id);
		return ECNT_CONTINUE;
	}else{
		qdma_data->retValue = qdma_operation[qdma_data->function_id](qdma_data);
	}
	
	return ECNT_CONTINUE;
}

ecnt_ret_val ecnt_fe_api_hook(struct ecnt_data *in_data)
{
	struct ecnt_fe_data *fe_data = (struct ecnt_fe_data *)in_data ;
	/* ulong flags = 0 ; */
	
	if(fe_data->function_id >= FE_FUNCTION_MAX_NUM) {
		printk("fe_data->function_id is %d, exceed max number: %d", fe_data->function_id, FE_FUNCTION_MAX_NUM);
		return ECNT_HOOK_ERROR;
	}

	/* spin_lock_irqsave(&hookFuncLock[fe_data->function_id], flags) ; */
	fe_data->retValue = fe_operation[fe_data->function_id](fe_data) ;
	/* spin_unlock_irqrestore(&hookFuncLock[fe_data->function_id], flags) ; */
	
	return ECNT_CONTINUE;
}


#define CR_FE_PPE_PHY_RANGE		(0x4000)
#define CR_FE_PPE_PHY_END		(CR_FE_PPE_PHY_BASE + CR_FE_PPE_PHY_RANGE)
#define CR_FE_PPE_PHY_BASE		(0x1fb50000)
u32 get_fe_ppe_data(u32 reg)
{
    return readl(glb_eth->fe_regs + reg);
}

void set_fe_ppe_data(u32 reg, u32 val)
{
    writel(val, glb_eth->fe_regs + reg); 
}

u32 get_frame_engine_data(u32 reg)
{
	u32 reg_phy = 0;
	u32 reg_offset = 0;

	/* translate addr to physical addr */
	if( reg > 0xa0000000)
		reg_phy = (reg & 0x1fffffff);
	else
		reg_phy = reg;
	
	reg_offset = reg_phy % 4;
	if(reg_offset != 0){
		printk("\nDatapath(%s) get reg error, reg=0x%08X\n", __func__, reg);
		return 0;
	}
	
	if( (CR_FE_PPE_PHY_BASE <= reg_phy) && (reg_phy < CR_FE_PPE_PHY_END) )
		return get_fe_ppe_data(reg_phy - CR_FE_PPE_PHY_BASE);
	else
		printk("\nDatapath(%s) get reg error, reg=0x%08X\n", __func__, reg);

	return 0;
}
EXPORT_SYMBOL(get_frame_engine_data);
void set_frame_engine_data(u32 reg, u32 val)
{
	u32 reg_phy = 0;
	u32 reg_offset = 0;

	/* translate addr to physical addr */
	if( reg > 0xa0000000)
		reg_phy = (reg & 0x1fffffff);
	else
		reg_phy = reg;

	reg_offset = reg_phy % 4;
	if(reg_offset != 0){
		printk("\nDatapath(%s) set reg error, reg=0x%08X\n", __func__, reg);
		return ;
	}

	if( (CR_FE_PPE_PHY_BASE <= reg_phy) && (reg_phy < CR_FE_PPE_PHY_END) )
		set_fe_ppe_data(reg_phy - CR_FE_PPE_PHY_BASE, val); 
	else
		printk("\nDatapath(%s) set reg error, reg=0x%08X\n", __func__, reg);

}
EXPORT_SYMBOL(set_frame_engine_data);

struct ecnt_hook_ops ecnt_driver_qdma_op = {
	.name = driver_qdma_hook_name,
	.hookfn = ecnt_qdma_hook,
	.maintype = ECNT_QDMA,
	.is_execute = 1,
	.subtype = ECNT_DRIVER_API,
	.priority = 1
};


struct ecnt_hook_ops ecnt_fe_api_op = {
	.name = "fe_api_hook",
	.is_execute = 1,
	.hookfn = ecnt_fe_api_hook,
	.maintype = ECNT_FE,
	.subtype = ECNT_FE_API,
	.priority = 1
};

static inline int airoha_ppe_is_vlan_proto(u16 etype)
{
	return (etype == htons(0x8100) || etype == htons(0x88a8)
		|| etype == htons(0x9100) || etype == htons(0x884c));
}

static void airoha_ppe_foe_get_vlan_info
	(struct sk_buff *skb, u16 *vn, u16 *vid1, u16 *vid2)
{
	u16 vl = 0;
	u16 tmp = *(u16 *)(skb->data+12);

	if(airoha_ppe_is_vlan_proto(tmp)){
		*vid1 = ntohs(*(u16 *)(skb->data+14));
		vl++;
		tmp = *(u16 *)(skb->data+16);
		if(airoha_ppe_is_vlan_proto(tmp)){
			*vid2 = ntohs(*(u16 *)(skb->data+18));
			vl++;
		}
	}
	*vn = vl;
}

static void airoha_ppe_foe_get_vlan_vpm
	(struct sk_buff *skb, u16 *vpm)
{
	u16 tmp = *(u16 *)(skb->data+12);

	if(airoha_ppe_is_vlan_proto(tmp)){
		if(tmp == htons(0x88a8))
			*vpm = 2;
		else
			*vpm = 1;
	}
	else
	{
		*vpm = 0;
	}
}

static void airoha_ppe_foe_get_pppoe_info
	(struct sk_buff *skb, u16 vl, u16 *pppid)
{
	u16 tmp;
	struct pppoe_hdr *ppph = NULL;

	tmp = *(u16 *)(skb->data+12 + vl*4);
	if(tmp == htons(ETH_P_PPP_SES)){
		ppph = pppoe_hdr(skb);
		if(!ppph)
			return;
		*pppid = ntohs(ppph->sid);
	}
}

struct hwnat_shrink_table shnkTbl[UPDMEM_NUM];
int timeOutVal=3000;	//30s
void UpdateShrinkTable(int index,const u8 *addr){	
	memcpy(&shnkTbl[index].smac[0], addr, ETH_ALEN);
	shnkTbl[index].valid[PPE_UPDMEM_SEL_SMAC] = 1;
	return;
}

void FoeSetEntrySrcMac(uint8_t * Dst, u32 * Src_hi, u16 * Src_lo)
{
	*Src_hi = (Dst[6] << 24) | (Dst[7] << 16) | (Dst[8] << 8) | Dst[9];
    *Src_lo = (Dst[10] << 8) | Dst[11];
}

void FoeSetEntryMac(uint8_t * Dst, u32 * Src_hi, u16 * Src_lo)
{
	*Src_hi = (Dst[0] << 24) | (Dst[1] << 16) | (Dst[2] << 8) | Dst[3];
    *Src_lo = (Dst[4] << 8) | Dst[5];
}

int cmpMacInfo(uint8_t* Dst, uint8_t* Src)
{
	if((Dst[0] == Src[6]) && (Dst[1] == Src[7])
	&& (Dst[2] == Src[8]) && (Dst[3] == Src[9])
	&& (Dst[4] == Src[10]) && (Dst[5] == Src[11])) {
		return HWNAT_SUCCESS;
	} else {
		return HWNAT_FAIL;
	}
}
static void set_ppe_entry_smac_index (struct airoha_foe_entry *foe_entry, unsigned int type, unsigned char value) {
    if(0x10 == value)/*workround for hw remove keep smac bit issue,now smac idx 0xf means keep smac*/
    {
		foe_entry->ipv6.l2.src_mac_hi = (foe_entry->ipv6.l2.src_mac_hi & ~AIROHA_FOE_MAC_SMAC_ID) | 
										FIELD_PREP(AIROHA_FOE_MAC_SMAC_ID, 0xf);
    }
    else
    {
		foe_entry->ipv6.l2.src_mac_hi = (foe_entry->ipv6.l2.src_mac_hi & ~AIROHA_FOE_MAC_SMAC_ID) | 
										FIELD_PREP(AIROHA_FOE_MAC_SMAC_ID, value);
    }
    return;
}

static int airoha_is_bridge_packet(struct net_device *dev,unsigned char* src_addr)
{
	const unsigned char* dev_addr= dev->dev_addr;
	if( ether_addr_equal_64bits(dev_addr,src_addr))
		return false;
	else 
		return true;
}

int airoha_set_ppe_mac(struct airoha_foe_entry *foe_entry, struct net_device *dev, char* src_mac, char*dst_mac, u16 pppid)
{
	struct airoha_foe_mac_info_common *l2;
	int type, index = 0;
	struct hwnat_shrink_field shrinkField;
	type = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, foe_entry->ib1);
	
	switch (type) {
		case PPE_PKT_TYPE_IPV4_ROUTE:
		case PPE_PKT_TYPE_IPV4_HNAPT:
			l2 = &foe_entry->ipv4.l2.common;			
			break;
		case PPE_PKT_TYPE_BRIDGE:
			l2 = &foe_entry->bridge32.l2;
			break;
		case PPE_PKT_TYPE_IPV6_ROUTE_3T:
		case PPE_PKT_TYPE_IPV6_ROUTE_5T:
		case PPE_PKT_TYPE_IPV6_6RD:
			l2 = &foe_entry->ipv6.l2;
			break;		
		case PPE_PKT_TYPE_IPV4_DSLITE:
			l2 = &foe_entry->dslite.l2.common;
			break;
			
		default:
			break;
	}
	l2->dest_mac_hi = get_unaligned_be32(dst_mac);
	l2->dest_mac_lo = get_unaligned_be16(dst_mac + 4);
	if (type <= PPE_PKT_TYPE_IPV4_DSLITE) {
		struct airoha_foe_mac_info *mac_info;
		mac_info = (struct airoha_foe_mac_info *)l2;
		l2->src_mac_hi = get_unaligned_be32(src_mac);
		mac_info->src_mac_lo = get_unaligned_be16(src_mac + 4);
		if(pppid)
			mac_info->pppoe_id = pppid;
	} else {
		if(airoha_is_bridge_packet(dev,src_mac) == true) {
			set_ppe_entry_smac_index(foe_entry, PPE_PKT_TYPE_IPV6_ROUTE_5T, 0x10);
		} else {
			memcpy(shrinkField.smac, src_mac, ETH_ALEN);

			index = find_and_update_shrink_table(PPE_UPDMEM_SEL_SMAC, &shrinkField);
			if(index != -1) {
				set_ppe_entry_smac_index(foe_entry, PPE_PKT_TYPE_IPV6_ROUTE_5T, index);
			} else {
				printk("smac: find and update shrink table failed!\n");
			}
		}
		if(pppid)
			l2->src_mac_hi |=  FIELD_PREP(AIROHA_FOE_MAC_PPPOE_ID, pppid);
		
	}	
	AIROHA_LOG(AIROHA_DEBUG_LEVEL_WARN, "%s:%d  dev:%s / type:%d / is_bridge:%d  /smac_id:%d\n",
	__func__,__LINE__,dev->name, type,airoha_is_bridge_packet(dev,src_mac),index);
	return 0;
}

u8 get_dscp_from_skb(struct sk_buff *skb, int type)
{
	struct iphdr *iph;
	struct ipv6hdr *ip6h;
	u32 offset = 0;
	u8 tos = 0;
	
	switch (type) {
		case PPE_PKT_TYPE_IPV4_ROUTE:
		case PPE_PKT_TYPE_IPV4_HNAPT:
		case PPE_PKT_TYPE_IPV4_DSLITE:
			iph = (struct iphdr *)(skb_network_header(skb) + offset);
			tos = iph->tos;				
			break;
	
		case PPE_PKT_TYPE_IPV6_ROUTE_3T:
		case PPE_PKT_TYPE_IPV6_ROUTE_5T:
		case PPE_PKT_TYPE_IPV6_6RD:
			ip6h = (struct ipv6hdr *)(skb_network_header(skb) + offset);
			tos = ipv6_get_dsfield(ip6h);	
			break;

		default:
			break;
	}

	return tos;
}

int airoha_get_lan_id(struct net_device *dev)
{
	if(dev == NULL)
		return 0;
	
	if((dev->name[0] == 'l') && (dev->name[1] == 'a') && (dev->name[2] == 'n'))
	{
		if(dev->name[3] == '1')
			return LAN1;
		else if(dev->name[3] == '2')
			return LAN2;
		else if(dev->name[3] == '3')
			return LAN3;
		else if(dev->name[3] == '4')
			return LAN4;
		
	}
	if((dev->name[0] == 'e') && (dev->name[1] == 't') && (dev->name[2] == 'h') && (dev->name[3] != '1') && (dev->name[3] != '0') )
		return ETH2;
	
	return 0;
}

static void u32_array_to_ip6_be(const u32 *src, __be32 *dst)
{
    int i;
    for (i = 0; i < 4; i++)
        dst[i] = ntohl(src[i]);
}

static void delete_conntrack_by_tuple6(const u32 *src_ip, const u32 *dst_ip,
                                       __be16 src_port, __be16 dst_port, u8 l4proto)
{
    struct nf_conntrack_tuple tuple;
    struct nf_conntrack_tuple_hash *thash;
    struct nf_conn *ct;

    memset(&tuple, 0, sizeof(tuple));
    tuple.src.l3num = AF_INET6;

    u32_array_to_ip6_be(src_ip, tuple.src.u3.ip6);
    u32_array_to_ip6_be(dst_ip, tuple.dst.u3.ip6);

    tuple.src.u.all = src_port;
    tuple.dst.u.all = dst_port;
    tuple.dst.protonum = l4proto;

    thash = nf_conntrack_find_get(&init_net, &nf_ct_zone_dflt, &tuple);
    if (thash) {
        ct = nf_ct_tuplehash_to_ctrack(thash);
        nf_ct_delete(ct, 0, 0);
        nf_ct_put(ct);
    } else {
        AIROHA_LOG(AIROHA_DEBUG_LEVEL_ERR, "No conntrack found for this tuple\n");
    }
}


static void delete_conntrack_by_tuple(__be32 src_ip, __be32 dst_ip, __be16 src_port, __be16 dst_port, u8 l4proto)
{
	struct nf_conntrack_tuple tuple;
    struct nf_conn *ct;
	struct nf_conntrack_tuple_hash *thash;
	
    memset(&tuple, 0, sizeof(tuple));
    tuple.src.l3num = AF_INET;
    tuple.src.u3.ip = src_ip;
    tuple.dst.u3.ip = dst_ip;
    tuple.src.u.all = src_port; // already in network order
    tuple.dst.u.all = dst_port; // already in network order
	tuple.dst.protonum = l4proto; // TCP or UDP, etc

    thash = nf_conntrack_find_get(&init_net, &nf_ct_zone_dflt, &tuple);
    if (thash) {
        ct = nf_ct_tuplehash_to_ctrack(thash);
        nf_ct_delete(ct, 0, 0);
        nf_ct_put(ct);
        
    } else {
		AIROHA_LOG(AIROHA_DEBUG_LEVEL_ERR, "No conntrack found for this tuple\n");
    }
	
}

void airoha_fe_core_reset(struct airoha_eth *eth)
{
	airoha_fe_rmw(eth, FE_RESET_GLO, REG_FE_CORE_RESET, 
				FIELD_PREP(REG_FE_CORE_RESET, 1));
	return;
}

void airoha_flow_table_entries_lan(struct rhashtable *flow_table, struct net_device *dev)
{
    struct airoha_flow_table_entry *e;
    struct rhashtable_iter iter;
    int ret;
    int lan_idx = airoha_get_lan_id(dev);
    
    rhashtable_walk_enter(flow_table, &iter);
    ret = rhashtable_walk_start_check(&iter);
    if (ret)
        goto out;

    while ((e = rhashtable_walk_next(&iter)) && !IS_ERR(e)) {
        if (lan_idx == e->e_magic || lan_idx == e->ingress_dev_idx) {
            int type = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, e->data.ib1);
			u32 state = FIELD_GET(AIROHA_FOE_IB1_BIND_STATE, e->data.ib1);
			u8 is_udp = FIELD_GET(AIROHA_FOE_IB1_BIND_UDP, e->data.ib1);
    		u8 l4proto = is_udp ? IPPROTO_UDP : IPPROTO_TCP;

			if (state != AIROHA_FOE_STATE_BIND)
				continue;

			AIROHA_LOG(AIROHA_DEBUG_LEVEL_ERR, "airoha_flow_table_entries_lan found flow entry: cookie=%lx ingress_dev_idx=%u, e->magic=%d and lan_idx=%d\n",
                   e->cookie, e->ingress_dev_idx, e->e_magic, lan_idx);
			
			if (type == PPE_PKT_TYPE_IPV4_HNAPT || type == PPE_PKT_TYPE_IPV4_ROUTE) {
                struct airoha_foe_ipv4_tuple *tuple = &e->data.ipv4.orig_tuple;
				
                delete_conntrack_by_tuple(ntohl(tuple->src_ip), ntohl(tuple->dest_ip),
                                         ntohs(tuple->src_port), ntohs(tuple->dest_port),l4proto);
            } else if (type == PPE_PKT_TYPE_IPV4_DSLITE) {
                struct airoha_foe_ipv4_tuple *tuple = &e->data.dslite.ip4;
                delete_conntrack_by_tuple(ntohl(tuple->src_ip), ntohl(tuple->dest_ip),
                                         ntohs(tuple->src_port), ntohs(tuple->dest_port),l4proto);
            } else if (type == PPE_PKT_TYPE_IPV6_ROUTE_3T ||
                       type == PPE_PKT_TYPE_IPV6_ROUTE_5T ||
                       type == PPE_PKT_TYPE_IPV6_6RD) {
                struct airoha_foe_ipv6 *tuple6 = &e->data.ipv6;
				delete_conntrack_by_tuple6(tuple6->src_ip, tuple6->dest_ip,
                                           ntohs(tuple6->src_port), ntohs(tuple6->dest_port),l4proto);
            } else {
				AIROHA_LOG(AIROHA_DEBUG_LEVEL_INFO, "Unsupported packet type: %d\n", type);
                continue;
            }
        }
    }

    rhashtable_walk_stop(&iter);
out:
    rhashtable_walk_exit(&iter);
}

static u32 airoha_ppe_get_timestamp(struct airoha_ppe *ppe)
{
	u16 timestamp = airoha_fe_rr(ppe->eth, REG_FE_FOE_TS);

	return FIELD_GET(AIROHA_FOE_IB1_BIND_TIMESTAMP, timestamp);
}

int airoha_is_pon_point_to_point_mode(struct sk_buff *skb)
{
	//0: not point to point mode, 1: is point to point mode
	if(airoha_pon_is_sfu_point_to_point_mode_hook){
		return airoha_pon_is_sfu_point_to_point_mode_hook(skb);
	}
	
	return 0;
}


int packet_hash_collision_check(struct sk_buff * skb, struct airoha_foe_entry *foe_entry, u16 vlan_num)
{
	struct iphdr *iph = NULL;
	struct ipv6hdr *iph6 = NULL;
	__be16 etype = skb->protocol;
	struct in6_addr foe_sip = {0}, foe_dip = {0};

	//skip vlan
	etype = *(unsigned short*)(skb->data + 12 + vlan_num*4);
	
	if (etype == htons(ETH_P_IP))
	{
		//check sip/dip
		iph = (struct iphdr*)(skb->data + 12 + vlan_num*4 + 2);
		if(IS_IPV4_GRP(foe_entry))
		{
			if((ntohl(iph->saddr) != foe_entry->ipv4.orig_tuple.src_ip) || (ntohl(iph->daddr) != foe_entry->ipv4.orig_tuple.dest_ip))
			{
				AIROHA_LOG(AIROHA_DEBUG_LEVEL_DBG,"packet_hash_collision_check: has hash collision!\n");
				return -1;
			}
		}
	}
	else if (etype == htons(ETH_P_IPV6))
	{
		// IPv6 packet
		iph6 = (struct ipv6hdr*)(skb->data + 12 + vlan_num*4 + 2);
		if (IS_IPV6_GRP(foe_entry))
		{
			// Check IPv6 5-tuple route
			foe_sip.s6_addr32[0] = htonl(foe_entry->ipv6.src_ip[0]);
			foe_sip.s6_addr32[1] = htonl(foe_entry->ipv6.src_ip[1]);
			foe_sip.s6_addr32[2] = htonl(foe_entry->ipv6.src_ip[2]);
			foe_sip.s6_addr32[3] = htonl(foe_entry->ipv6.src_ip[3]);
			foe_dip.s6_addr32[0] = htonl(foe_entry->ipv6.dest_ip[0]);
			foe_dip.s6_addr32[1] = htonl(foe_entry->ipv6.dest_ip[1]);
			foe_dip.s6_addr32[2] = htonl(foe_entry->ipv6.dest_ip[2]);
			foe_dip.s6_addr32[3] = htonl(foe_entry->ipv6.dest_ip[3]);
			if (memcmp(&iph6->saddr, &foe_sip, sizeof(struct in6_addr)) != 0 ||
				memcmp(&iph6->daddr, &foe_dip, sizeof(struct in6_addr)) != 0)
			{
				AIROHA_LOG(AIROHA_DEBUG_LEVEL_DBG,"packet_hash_collision_check: has hash collision IPV6!\n");
				return -1;
			}
		}
	}
	
	return 0;
	
}


int airoha_check_flood_packet(struct sk_buff *skb, u16 vid, u32 tag)
{
	struct net_bridge *br;
	struct net_device *dev = NULL, *tmp_dev = NULL;
	struct net_bridge_port *p;
	struct net_bridge_fdb_entry *f;
	unsigned char *addr;
	int port_idx = 1;

	if (!skb || !skb->dev){
        return 0;
	}
	tmp_dev = skb->dev;

	if (netdev_uses_dsa(tmp_dev))
	{
		port_idx = ffs(tag)-1;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 12, 0)
		tmp_dev = dsa_conduit_find_user(tmp_dev, 0, port_idx);
#else
		tmp_dev = dsa_master_find_slave(tmp_dev, 0, port_idx);
#endif
		if(!tmp_dev)
			return 0;

	}

	p = br_port_get_rcu(tmp_dev);
	if (!p || !p->dev){
		return 0;
	}
	br = p->br;

	if (!br){
        return 0;
	}

    addr = skb->data;
	
	rcu_read_lock();
	f = br_fdb_find_rcu(br, addr, vid);
	if (f && f->dst){
		dev = f->dst->dev;
	}	
	rcu_read_unlock();

	if (dev && dev == tmp_dev){
		return 1;
	}

	return 0;
}

int airoha_is_pon_sfu_mode(void)
{
	return airoha_pon_onu_type == 1;
}
EXPORT_SYMBOL(airoha_is_pon_sfu_mode);

static inline int airoha_is_local_out(struct sk_buff *skb)
{
	return skb->inner_protocol == PPE_MAGIC_LOCAL_OUT;
}

void airoha_ppe_general_bind(struct airoha_ppe *ppe, struct airoha_foe_entry *hwe, 
	struct sk_buff *skb, struct port_info *pinfo, u8 fport)
{
	u16 vn = 0, vid1 = 0, vid2 = 0, vpm = 0;	
	int dscp=0, type, fast = 0;
	u32 data, ib1;
	u32 meter_idx_by_gemport = 0x0;
    u32 meter_id_to_use = 0x7F;
	struct airoha_eth *eth = glb_eth;
	struct airoha_foe_mac_info_common *l2;
	u32 ts = airoha_ppe_get_timestamp(ppe);	
	u32 hash = FOE_ENTRY_NUM(skb);

	if(FIELD_GET(AIROHA_FOE_IB1_BIND_STATE, hwe->ib1) == AIROHA_FOE_STATE_BIND)
		return;

	airoha_ppe_foe_get_vlan_info(skb, &vn, &vid1, &vid2);
	airoha_ppe_foe_get_vlan_vpm(skb, &vpm);

	if(packet_hash_collision_check(skb, hwe, vn) < 0)
		return;

	type = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, hwe->ib1);
	dscp = get_dscp_from_skb(skb, type);
			
	AIROHA_LOG(AIROHA_DEBUG_LEVEL_ERR, "Packet type:%d vid1 %d vid2 %d\n"
					,type,vid1,vid2);
					
	if (eth->meter_enable) {
		switch (fport) {
			case FE_PSE_PORT_GDM2:
				meter_idx_by_gemport = pinfo->tsid;
				break;
			case FE_PSE_PORT_GDM1:
			case FE_PSE_PORT_GDM3:
			case FE_PSE_PORT_GDM4:
			default:
				meter_idx_by_gemport = (skb->mark & AIROHA_SKB_MARK_MASK_FOR_GEMPORT_RATELIMIT) >> AIROHA_SKB_MARK_SHIFT_FOR_GEMPORT_RATELIMIT;  //meter_id_mask from (31,26) meter_is shift is 26.
				break;
		}
	
		if(meter_idx_by_gemport <= AIROHA_MAX_IDX_FOR_GEMPORT_RATELIMIT && meter_idx_by_gemport >= AIROHA_MIN_IDX_FOR_GEMPORT_RATELIMIT)
				meter_id_to_use = meter_idx_by_gemport + AIROHA_NUM_RX_RING;
	}
					
	data = FIELD_PREP(AIROHA_FOE_CHANNEL, pinfo->channel) |
   			FIELD_PREP(AIROHA_FOE_QID, 
				((AIROHA_NUM_QOS_QUEUES - 1) - ((pinfo->txq) % AIROHA_NUM_QOS_QUEUES))) |
   			FIELD_PREP(AIROHA_FOE_SHAPER_ID, meter_id_to_use);

	ib1 = hwe->ib1;
	ib1 &= ~(AIROHA_FOE_IB1_BIND_VLAN_LAYER | AIROHA_FOE_IB1_BIND_VPM |
				AIROHA_FOE_IB1_BIND_PPPOE | AIROHA_FOE_IB1_BIND_TIMESTAMP |
				AIROHA_FOE_IB1_BIND_STATE);
	ib1 |= FIELD_PREP(AIROHA_FOE_IB1_BIND_VLAN_LAYER, vn) | 
					FIELD_PREP(AIROHA_FOE_IB1_BIND_VPM, vpm) |
					FIELD_PREP(AIROHA_FOE_IB1_BIND_TIMESTAMP, ts) | 
					FIELD_PREP(AIROHA_FOE_IB1_BIND_STATE, AIROHA_FOE_STATE_BIND);
	
	switch (fport){
		case FE_PSE_PORT_GDM1:
			if (ppe->eth->qdma_init.lan_fastpath == 1){
				fast = AIROHA_FOE_IB2_FAST_PATH;
			}
			break;

		case FE_PSE_PORT_GDM2:
			if (ppe->eth->qdma_init.wan_fastpath == 1 || pinfo->fast == 1){
				fast = AIROHA_FOE_IB2_FAST_PATH;
			}
			break;

		case FE_PSE_PORT_GDM3:
		case FE_PSE_PORT_GDM4:
			if (ppe->eth->qdma_init.xsi_ether_fastpath == 1 || pinfo->fast == 1){
				fast = AIROHA_FOE_IB2_FAST_PATH;
			}
			break;

		default:
			break;
	}
	
	switch (type) {
		case PPE_PKT_TYPE_IPV4_HNAPT:
			hwe->ipv4.new_tuple.src_port = hwe->ipv4.orig_tuple.src_port;
			hwe->ipv4.new_tuple.dest_port = hwe->ipv4.orig_tuple.dest_port;
			fallthrough;
		case PPE_PKT_TYPE_IPV4_ROUTE:
			hwe->ipv4.data = data;
			l2 = &hwe->ipv4.l2.common;
			hwe->ipv4.ib2 = 0;
			hwe->ipv4.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, pinfo->nbq);	
			hwe->ipv4.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_DSCP, dscp);	
			hwe->ipv4.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, fport) |
			       AIROHA_FOE_IB2_PSE_QOS | fast;
			hwe->ipv4.new_tuple.src_ip = hwe->ipv4.orig_tuple.src_ip;
			hwe->ipv4.new_tuple.dest_ip = hwe->ipv4.orig_tuple.dest_ip;
			break;
		
		case PPE_PKT_TYPE_BRIDGE:
			hwe->bridge32.data = data;
			l2 = &hwe->bridge32.l2;
			hwe->bridge32.ib2 = 0;
			hwe->bridge32.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, pinfo->nbq);	
			hwe->bridge32.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, fport) |
		       AIROHA_FOE_IB2_PSE_QOS | fast;		
			break;
		
		case PPE_PKT_TYPE_IPV6_ROUTE_3T:
		case PPE_PKT_TYPE_IPV6_ROUTE_5T:
			hwe->ipv6.data = data;
			l2 = &hwe->ipv6.l2;
			hwe->ipv6.ib2 = 0;
			hwe->ipv6.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, pinfo->nbq);	
			hwe->ipv6.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_DSCP, dscp);
			hwe->ipv6.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, fport) |
		       AIROHA_FOE_IB2_PSE_QOS | fast;				
			break;

		default:
			break;
	}

	l2->etype = pinfo->stag;
	l2->vlan1 = vid1;
	l2->vlan2 = vid2;
	
	airoha_set_ppe_mac(hwe,skb->dev,skb->data+6,skb->data,0);

	hwe->ib1 = ib1;

	if (hash < PPE_SRAM_NUM_ENTRIES) {
		airoha_ppe_foe_commit_entry_ptr(ppe, hwe, hash,0);
	}
	
	return;
}

void airoha_ppe_pon_sfu_bind(struct airoha_ppe *ppe, struct airoha_foe_entry *hwe, 
	struct sk_buff *skb, struct port_info *pinfo, u8 fport)
{
	if (!airoha_is_pon_point_to_point_mode(skb) && !packet_is_transparent_mode) 
	{
		//if not point to ponit mode, need to check mac table in kernel
	    if (!airoha_check_flood_packet(skb, 0, pinfo->stag)) {
	        return;
	    }
	}

	return airoha_ppe_general_bind(ppe, hwe, skb, pinfo, fport);
}

void FoeGetEntryDstMac(u8 * Dst, u32 Dst_hi, u16 Dst_lo)
{
	Dst[0] = ((Dst_hi&0xff000000) >> 24);
	Dst[1] = ((Dst_hi&0xff0000) >> 16);
	Dst[2] = ((Dst_hi&0xff00) >> 8);
	Dst[3] = (Dst_hi&0xff) ;
	Dst[4] = ((Dst_lo&0xff00) >> 8);
	Dst[5] = (Dst_lo&0xff);
}

static inline int is_ipv4_multicast(u32 ip)
{
    return (ip >= 0xE0000000 && ip <= 0xEFFFFFFF);
}

static inline int is_ipv6_multicast(u32 addr)
{
    return ((addr >> 24) & 0xFF) == 0xFF;
}

/*Using return value to judge
0:is not multicast entry
1:is multicast entry and is for ipv4
2:is multicast entry and is for ipv6
*/
int ppe_is_multicast_entry(struct airoha_foe_entry *hwe)
{
	unsigned char dst_mac[ETH_ALEN] = {0};

	if(hwe == NULL)
	{
		return 0;
	}

	if (FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, hwe->ib1) < PPE_PKT_TYPE_BRIDGE )
	{
		if(is_ipv4_multicast(hwe->ipv4.orig_tuple.dest_ip))
		{
			return 1;
		}
	}
	else if (FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, hwe->ib1) == PPE_PKT_TYPE_BRIDGE)
	{
		FoeGetEntryDstMac(dst_mac, hwe->bridge.dest_mac_hi, hwe->bridge.dest_mac_lo);
		if(is_multicast_ether_addr(dst_mac)) {
			return 1;
		} else {
			return 0;
		}
	}
	else if((FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, hwe->ib1) == PPE_PKT_TYPE_IPV6_ROUTE_3T)||
		(FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, hwe->ib1) == PPE_PKT_TYPE_IPV6_ROUTE_5T))
	{
		if (is_ipv6_multicast(hwe->ipv6.dest_ip[0]))
			return 2;
	}

	return 0;
}

void airoha_ppe_foe_flow_update_eth_offload(struct airoha_ppe *ppe, 
	struct sk_buff *skb, struct port_info *pinfo, u8 fport)
{
	u32 hash = FOE_ENTRY_NUM(skb);
	int type;
	struct airoha_flow_table_entry *e;
	struct airoha_foe_entry *hwe;
	struct hlist_node *n;
	u32 index;
	int dscp=0;
	
	AIROHA_LOG(AIROHA_DEBUG_LEVEL_INFO, "hash at qdma_lan_tx: %u\n", hash);
	if (hash >= PPE_SRAM_NUM_ENTRIES || !skb->l4_hash || skb->sw_hash != false) {
		AIROHA_LOG(AIROHA_DEBUG_LEVEL_INFO, "Incorrect hash:%u\n", hash);
        return;
    }

	if(arht_multicast_handler_for_sfu(skb))
	{
		return;
	}
	
	spin_lock_bh(&ppe_lock);
	hwe = airoha_ppe_foe_get_entry_locked(ppe, hash);
	if (!hwe)
	{
		goto unlock;
	}

    if(airoha_is_local_out(skb))
    {
        pinfo->fast = 1;
        airoha_ppe_general_bind(ppe, hwe, skb, pinfo, fport);
        goto unlock;
    }

	if(airoha_is_pon_sfu_mode()){
		airoha_ppe_pon_sfu_bind(ppe, hwe, skb, pinfo, fport);
		goto unlock;
	}
	
	index = airoha_ppe_foe_get_entry_hash(hwe);
	hlist_for_each_entry_safe(e, n, &ppe->foe_flow[index], list) {
		if (airoha_ppe_foe_compare_entry(e, hwe)) {
			if(e->tx_modified){
				goto unlock;
			}
			type = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, e->data.ib1);			
			dscp = get_dscp_from_skb(skb, type);
			
			switch (type) {
				case PPE_PKT_TYPE_IPV4_ROUTE:
				case PPE_PKT_TYPE_IPV4_HNAPT:
					e->data.ipv4.ib2 &= ~AIROHA_FOE_IB2_DSCP;
					e->data.ipv4.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_DSCP, dscp);	
					break;
				
				case PPE_PKT_TYPE_BRIDGE:
					break;
				
				case PPE_PKT_TYPE_IPV6_ROUTE_3T:
				case PPE_PKT_TYPE_IPV6_ROUTE_5T:
				case PPE_PKT_TYPE_IPV6_6RD:
					e->data.ipv6.ib2 &= ~AIROHA_FOE_IB2_DSCP;
					e->data.ipv6.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_DSCP, dscp);
					break;
					
				case PPE_PKT_TYPE_IPV4_DSLITE:
					e->data.dslite.ib2 &= ~AIROHA_FOE_IB2_DSCP;
					e->data.dslite.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_DSCP, dscp);
					break;
					
				default:
					break;
			}
			e->tx_modified = true;
			AIROHA_LOG(AIROHA_DEBUG_LEVEL_ERR, "Eth upstream hash %u set modified_bit %u at tx\n"
					, hash, e->tx_modified);
			break;
		}
	}
unlock:
	spin_unlock_bh(&ppe_lock);
}

void airoha_ppe_foe_flow_update_pon_offload
	(struct airoha_ppe *ppe, struct sk_buff *skb, struct port_info *pinfo)
{
	u32 hash = FOE_ENTRY_NUM(skb);
	int type;
	struct airoha_flow_table_entry *e;
	struct airoha_foe_entry *hwe;
	struct hlist_node *n;
	struct airoha_foe_mac_info_common *l2;
	u32 index, data;
	u16 vn = 0, vid1 = 0, vid2 = 0, pppid = 0;	
	struct ethhdr* eth = NULL;
	int dscp=0;
	
	AIROHA_LOG(AIROHA_DEBUG_LEVEL_INFO, "hash at qdma_wan_tx: %u\n", hash);
	if (hash >= PPE_SRAM_NUM_ENTRIES || !skb->l4_hash || skb->sw_hash != false) {
		AIROHA_LOG(AIROHA_DEBUG_LEVEL_INFO, "Incorrect hash:%u\n", hash);
        return;
    }
	
	spin_lock_bh(&ppe_lock);
	hwe = airoha_ppe_foe_get_entry_locked(ppe, hash);
	if (!hwe)
	{
		goto unlock;
	}

    if(airoha_is_local_out(skb))
    {
        pinfo->fast = 1;
        airoha_ppe_general_bind(ppe, hwe, skb, pinfo, 2);
        goto unlock;
    }

	if(airoha_is_pon_sfu_mode()){
		airoha_ppe_pon_sfu_bind(ppe, hwe, skb, pinfo, 2);
		goto unlock;
	}
		
	index = airoha_ppe_foe_get_entry_hash(hwe);
	hlist_for_each_entry_safe(e, n, &ppe->foe_flow[index], list) {
		if (airoha_ppe_foe_compare_entry(e, hwe)) {
			if(e->tx_modified){
				goto unlock;
			}
			airoha_ppe_foe_get_vlan_info(skb, &vn, &vid1, &vid2);
			airoha_ppe_foe_get_pppoe_info(skb, vn, &pppid);

			type = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, e->data.ib1);
			
			dscp = get_dscp_from_skb(skb, type);
			
			AIROHA_LOG(AIROHA_DEBUG_LEVEL_ERR, "Packet type:%d vid1 %d vid2 %d pppid %d\n"
					,type,vid1,vid2,pppid);
			data = FIELD_PREP(AIROHA_FOE_CHANNEL, pinfo->channel) |
	       			FIELD_PREP(AIROHA_FOE_QID, 
						((AIROHA_NUM_QOS_QUEUES - 1) - ((pinfo->txq) % AIROHA_NUM_QOS_QUEUES))) |
	       			FIELD_PREP(AIROHA_FOE_SHAPER_ID, 0x7F);

			e->data.ib1 |= FIELD_PREP(AIROHA_FOE_IB1_BIND_VLAN_LAYER, !!vn) | 
							FIELD_PREP(AIROHA_FOE_IB1_BIND_VPM, !!vn) | 
							FIELD_PREP(AIROHA_FOE_IB1_BIND_PPPOE, !!pppid);
			switch (type) {
				case PPE_PKT_TYPE_IPV4_ROUTE:
				case PPE_PKT_TYPE_IPV4_HNAPT:
					e->data.ipv4.data = data;
					l2 = &e->data.ipv4.l2.common;
					e->data.ipv4.ib2 &= ~AIROHA_FOE_IB2_NBQ;
					e->data.ipv4.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, pinfo->channel);	
					e->data.ipv4.ib2 &= ~AIROHA_FOE_IB2_DSCP;
					e->data.ipv4.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_DSCP, dscp);	
					break;
				
				case PPE_PKT_TYPE_BRIDGE:
					e->data.bridge.data = data;
					l2 = &e->data.bridge.l2.common;
					e->data.bridge.ib2 &= ~AIROHA_FOE_IB2_NBQ;
					e->data.bridge.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, pinfo->channel);	
					break;
				
				case PPE_PKT_TYPE_IPV6_ROUTE_3T:
				case PPE_PKT_TYPE_IPV6_ROUTE_5T:
				case PPE_PKT_TYPE_IPV6_6RD:
					e->data.ipv6.data = data;
					l2 = &e->data.ipv6.l2;
					e->data.ipv6.ib2 &= ~AIROHA_FOE_IB2_NBQ;
					e->data.ipv6.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, pinfo->channel);	
					e->data.ipv6.ib2 &= ~AIROHA_FOE_IB2_DSCP;
					e->data.ipv6.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_DSCP, dscp);
					break;
					
				case PPE_PKT_TYPE_IPV4_DSLITE:
					e->data.dslite.data = data;		
					l2 = &e->data.dslite.l2.common;
					e->data.dslite.ib2 &= ~AIROHA_FOE_IB2_NBQ;
					e->data.dslite.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, pinfo->channel);
					e->data.dslite.ib2 &= ~AIROHA_FOE_IB2_DSCP;
					e->data.dslite.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_DSCP, dscp);
					break;
					
				default:
					break;
			}

			l2->etype = pinfo->stag;
			l2->vlan1 = vid1;
			l2->vlan2 = vid2;
			
			/*Set smac value when mac tx based on bridge /route mode */
			eth = (struct ethhdr *)skb_mac_header(skb);
			
			airoha_set_ppe_mac(&e->data,skb->dev,eth->h_source,eth->h_dest,pppid);
			
			e->tx_modified = true;
			AIROHA_LOG(AIROHA_DEBUG_LEVEL_ERR, "PON upstream hash %u set modified_bit %u at tx\n"
					, hash, e->tx_modified);
			
			break;
		}
	}
unlock:
	spin_unlock_bh(&ppe_lock);
}

static int isPriorityPkt(struct sk_buff *skb)
{
	ushort etherType=0;
	unchar ipVerLen=0;
	unchar ipProtocol=0;
	unchar tcpFlags=0;
	ushort pppProtocol=0;
	unchar ipv6_protocol=0, ipv6_type=0;
	ushort dport=0, sport=0;
	unchar *cp = NULL;
#define ICMPV6_ROUTE_SOL 133
#define ICMPV6_ROUTE_ADV 134
#define ICMPV6_NEIGH_SOL 135
#define ICMPV6_NEIGH_ADV 136

	cp = skb->data;
	cp += 12;
	/* get ether type */
	etherType = *(ushort *) cp;
	/* skip ether type */
	cp += 2;

	/*parse if vlan exists*/
	if (etherType == htons(0x8100)) {
		/*skip 802.1q tag field*/
		cp += 2;
		/*re-parse ether type*/
		etherType = *(ushort *) cp;
		/* skip ether type */
		cp += 2;
	}
    	/*parse if vlan exists*/
	if (etherType == htons(0x8100)) {
		/*skip 802.1q tag field*/
		cp += 2;
		/*re-parse ether type*/
		etherType = *(ushort *) cp;
		/* skip ether type */
		cp += 2;
	}

	/*check whether PPP packets*/
	if (etherType == htons(0x8864)) {
		/* skip pppoe head */
		cp += 6; 					/* 6: PPPoE header 2: PPP protocol */
		/* get ppp protocol */
		pppProtocol = *(ushort *) cp;
		/* check if LCP protocol and ipcpv6 protocol */
		if ((pppProtocol == htons(0xc021)) || (pppProtocol == htons(0x8021)) || (pppProtocol == htons(0x8057)) 
			|| (pppProtocol == htons(0xc223)) || (pppProtocol == htons(0xc057))) {
			return 1;
		/* check if IPv6 protocol */
		} else if (pppProtocol == htons(0x0057)) {
			cp += 2;
			cp += 6;
			/* get ip protocol */
			ipProtocol = *(unchar*)cp;
			ipVerLen = 0;
			cp += 34;
         
			if (ipProtocol == 0x3a) {
    				ipv6_type = *(unchar*)cp;

			}

			goto ipv6_header;
		/* check if IP protocol */
		} else if (pppProtocol != htons(0x0021)) {
			return 0;
		}
		/* skip ppp protocol */
		cp += 2; 					/* 6: PPPoE header 2: PPP protocol */
	} else if (etherType == htons(0x8863)) {
		return 1;
	/*check whether arp packet*/	
	} else if (etherType == htons(0x0806)) {
		return 1;	
	} 
	else if (etherType == htons(0x86dd)) {
		cp += 6;
         
		ipv6_protocol = *(unchar*)cp;
		cp += 34;
		if (ipv6_protocol == 0x3a) {
			ipv6_type = *(unchar*)cp;
		}
		/* get ip protocol */
		ipProtocol = ipv6_protocol;
		if(ipProtocol == 0x11) /* udp + dns */
		{
			if(  *(ushort *)(cp + 2) == htons(0x0035) )
				return 1;
		}
		ipVerLen = 0;
		goto ipv6_header;
	}
	else {
		/* check if ip packet */
		if (etherType != htons(0x0800)) {
			return 0;
		}
	}

	pppProtocol = *(ushort *) cp;
	/* check if LCP protocol, for pppoa control packet */
	if (pppProtocol == htons(0xc021)) {
		return 1;
	} else if(pppProtocol == htons(0x0021)) {
		cp += 2; 					/* 6: PPPoE header 2: PPP protocol */
	}

	/* check if it is a ipv4 packet */
	ipVerLen = *cp;
	if ((ipVerLen & 0xf0) != 0x40) {
		return 0;
	}

	/* get ip protocol */
	ipProtocol = *(cp + 9);

	if(ipProtocol == 0x11) /* udp + dns */
	{
		ushort sport = *(ushort *)(cp + ((ipVerLen & 0x0f) << 2));
		ushort dport = *(ushort *)(cp + ((ipVerLen & 0x0f) << 2) + 2);

		if ((sport == htons(68) && dport == htons(67)) || // DHCP Discover/Request
			(sport == htons(67) && dport == htons(68)))   // DHCP Offer/Ack
		{
			return 1;
		}
		if( *(ushort *)(cp + ((ipVerLen & 0x0f) << 2) + 2) == htons(0x0035) )
			return 1;
	}

    if((ipProtocol == 2) || (ipProtocol == 1))
        return 1;

ipv6_header:

	if (ipv6_type ==ICMPV6_ROUTE_SOL || 
		ipv6_type ==ICMPV6_ROUTE_ADV ||
		ipv6_type ==ICMPV6_NEIGH_SOL ||
		ipv6_type ==ICMPV6_NEIGH_ADV) {
		return 1;
	}

	if( ipProtocol == 17 )//udp
	{
		/*get source port and dest port*/
		sport = *(ushort *)(cp + ((ipVerLen & 0x0f) << 2) );
		dport = *(ushort *)(cp + ((ipVerLen & 0x0f) << 2) + 2);
		
		if(sport ==  htons(0x0223) && dport== htons(0x0222) ){
			//printk("lalala dhcpv6 adver or reply to first queue.\n"); 
			return 1;		
		}
		if(sport ==  htons(0x0222) && dport ==  htons(0x0223) ){
			//printk("lalala dhcpv6 request or soclit to first queue.\n"); 
			return 1;
		}
	}

	/* check if TCP protocol */
	if (ipProtocol != 6) {
		return 0;
	}

	/* align to TCP header */
	cp += (ipVerLen & 0x0f) << 2;
	/* get TCP flags */
	tcpFlags = *(cp + 13);
	
	/* check if TCP fin/syn/reset */
	if (((tcpFlags & 0x01) == 0x01) || ((tcpFlags & 0x02) == 0x02) || ((tcpFlags & 0x04) == 0x04)) {
		return 1;
	}

	return 0;
}

int airoha_eth_transmit_packet(struct sk_buff *skb, u32 txmsg0, u32 txmsg1, struct port_info *pinfo){
	struct net_device *dev = glb_eth->ports[0]->dev;
	struct airoha_gdm_port *port = netdev_priv(dev);
	struct airoha_qdma *qdma = port->qdma;
	u32 nr_frags, len;
	struct netdev_queue *txq;
	struct airoha_queue *q;
	
	void *data;
	int i, qid;
	u16 index;
	//u8 fport;
	skb->dev = dev;
	
	qid = skb_get_queue_mapping(skb) % ARRAY_SIZE(qdma->q_tx);
	//tag = DP_SPEED_UP;//use in sptag for pingpong stream

	txmsg0 = FIELD_PREP(QDMA_ETH_TXMSG_TCO_MASK, 1) |
		   FIELD_PREP(QDMA_ETH_TXMSG_CHAN_MASK,
			  qid / AIROHA_NUM_QOS_QUEUES) |
		   FIELD_PREP(QDMA_ETH_TXMSG_QUEUE_MASK,
			  qid % AIROHA_NUM_QOS_QUEUES) ;
	
	if (skb->ip_summed == CHECKSUM_PARTIAL)
		txmsg0 |= FIELD_PREP(QDMA_ETH_TXMSG_TCO_MASK, 1) |
			FIELD_PREP(QDMA_ETH_TXMSG_UCO_MASK, 1) |
			FIELD_PREP(QDMA_ETH_TXMSG_ICO_MASK, 1);

	/* TSO: fill MSS info in tcp checksum field */
	if (skb_is_gso(skb)) {
		if (skb_cow_head(skb, 0))
			goto error;

		if (skb_shinfo(skb)->gso_type & (SKB_GSO_TCPV4 |
						 SKB_GSO_TCPV6)) {
			__be16 csum = cpu_to_be16(skb_shinfo(skb)->gso_size);

			tcp_hdr(skb)->check = (__force __sum16)csum;
			txmsg0 |= FIELD_PREP(QDMA_ETH_TXMSG_TSO_MASK, 1) |
					FIELD_PREP(QDMA_ETH_TXMSG_UCO_MASK, 1) |
					FIELD_PREP(QDMA_ETH_TXMSG_ICO_MASK, 1);
		}
	} 
	
	//fport = FE_PSE_PORT_PPE1;
	
//	msg1 =0x7f4007ff;
	q = &qdma->q_tx[qid];
	if (WARN_ON_ONCE(!q->ndesc))
		goto error;

	spin_lock_irq(&q->lock);

	txq = netdev_get_tx_queue(dev, qid);
	nr_frags = 1 + skb_shinfo(skb)->nr_frags;

	if (q->queued + nr_frags > q->ndesc) {
		/* not enough space in the queue */
		netif_tx_stop_queue(txq);
		spin_unlock_irq(&q->lock);
		return NETDEV_TX_BUSY;
	}

	len = skb_headlen(skb);
	data = skb->data;
	index = q->head;

	for (i = 0; i < nr_frags; i++) {
		struct airoha_qdma_desc *desc = &q->desc[index];
		struct airoha_queue_entry *e = &q->entry[index];
		skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
		dma_addr_t addr;
		u32 val;
		addr = dma_map_single(dev->dev.parent, data, len,
					  DMA_TO_DEVICE);
		if (unlikely(dma_mapping_error(dev->dev.parent, addr)))
			goto error_unmap;

		index = (index + 1) % q->ndesc;

		val = FIELD_PREP(QDMA_DESC_LEN_MASK, len);
		if (i < nr_frags - 1){
			val |= FIELD_PREP(QDMA_DESC_MORE_MASK, 1);
		}
		WRITE_ONCE(desc->ctrl, cpu_to_le32(val));
		WRITE_ONCE(desc->addr, cpu_to_le32(addr));

		val = FIELD_PREP(QDMA_DESC_NEXT_ID_MASK, index);
		WRITE_ONCE(desc->data, cpu_to_le32(val));
		WRITE_ONCE(desc->msg0, cpu_to_le32(txmsg0));
		WRITE_ONCE(desc->msg1, cpu_to_le32(txmsg1));

		e->skb = i ? NULL : skb;
		e->dma_addr = addr;
		e->dma_len = len;

		data = skb_frag_address(frag);
		len = skb_frag_size(frag);
	}
	

	q->head = index;
	q->queued += i;

	skb_tx_timestamp(skb);
	netdev_tx_sent_queue(txq, skb->len);

	if (netif_xmit_stopped(txq) || !netdev_xmit_more())
		airoha_qdma_rmw(qdma, REG_TX_CPU_IDX(qid),
				TX_RING_CPU_IDX_MASK,
				FIELD_PREP(TX_RING_CPU_IDX_MASK, q->head));

	if (q->ndesc - q->queued < q->free_thr)
		netif_tx_stop_queue(txq);

	spin_unlock_irq(&q->lock);

	return NETDEV_TX_OK;

error_unmap:
	for (i--; i >= 0; i--) {
		index = (q->head + i) % q->ndesc;
		dma_unmap_single(dev->dev.parent, q->entry[index].dma_addr,
				 q->entry[index].dma_len, DMA_TO_DEVICE);
	}

	spin_unlock_irq(&q->lock);
error:
	dev_kfree_skb_any(skb);
	dev->stats.tx_dropped++;

	return NETDEV_TX_OK;
}
EXPORT_SYMBOL(airoha_eth_transmit_packet);
void airoha_qdma_lan_tx(struct sk_buff *skb,u32 tag,u8 fport,int channel,int qid)
{
	struct port_info pinfo = {0};
	pinfo.stag = tag;
	pinfo.channel = channel;
	pinfo.txq = qid;
	pinfo.nbq = channel;

	if (!glb_eth || !glb_eth->chip)
        return;

	for (int i = SERDES_PCIE0_IDX; i < SERDES_MAX_IDX; i++) {
        if (fport == glb_eth->chip->fport[i]) {
            pinfo.channel = glb_eth->chip->chnl[i];
            pinfo.nbq = glb_eth->chip->nbq[i];
            break;
        }
    }
	
	airoha_ppe_foe_flow_update_eth_offload(glb_eth->ppe, skb, &pinfo, fport);
}

/* qdma_wan Tx API */
int qdma_wan_tx(struct sk_buff *skb, u32 msg0, u32 msg1, struct port_info *pinfo)
{
	struct skb_shared_info *sinfo = skb_shinfo(skb);
	
	struct airoha_gdm_port *port = glb_eth->ports[1];
	
	struct net_device *dev = port->dev;
	struct airoha_foe_entry *hwe;
	u32  len = skb_headlen(skb);
	
	struct airoha_qdma *qdma = &glb_eth->qdma[1];
	
	u32 nr_frags = 1 + sinfo->nr_frags;
	struct netdev_queue *txq;
	struct airoha_queue *q;
	void *data;
	int i, qid;
	u16 index;
	u32 msg2 = 0;

	//qid = pinfo->channel;
	
	msg0 |= FIELD_PREP(QDMA_ETH_TXMSG_TCO_MASK, 1);
	if(isPriorityPkt(skb)){
		msg0 |= FIELD_PREP(QDMA_ETH_TXMSG_QUEUE_MASK, 7);
		skb_set_queue_mapping(skb, 7);
	}
	qid = skb_get_queue_mapping(skb);
	
	msg1 |= FIELD_PREP(QDMA_ETH_TXMSG_METER_MASK, 0x7f);
	msg2 |= FIELD_PREP(QDMA_ETH_TXMSG_ACNT_G2_MASK, 0x7f)|
			FIELD_PREP(QDMA_ETH_TXMSG_METER_G1_MASK, 0x1f)|
			FIELD_PREP(QDMA_ETH_TXMSG_METER_G2_MASK, 0xf)|
			FIELD_PREP(QDMA_ETH_TXMSG_SW_UDF, 0xffff);
	/* TSO: fill MSS info in tcp checksum field */
	if (skb_is_gso(skb)) {
		if (skb_cow_head(skb, 0))
			goto error;
                   if (skb_shinfo(skb)->gso_type & (SKB_GSO_TCPV4 |
						 SKB_GSO_TCPV6)) {
			__be16 csum = cpu_to_be16(skb_shinfo(skb)->gso_size);

			tcp_hdr(skb)->check = (__force __sum16)csum;
			msg0 |= FIELD_PREP(QDMA_ETH_TXMSG_TSO_MASK, 1) |
					FIELD_PREP(QDMA_ETH_TXMSG_UCO_MASK, 1) |
					FIELD_PREP(QDMA_ETH_TXMSG_ICO_MASK, 1);
		}
	}	

	q = &qdma->q_tx[qid];
	if (WARN_ON_ONCE(!q->ndesc))
		goto error;

	airoha_ppe_foe_flow_update_pon_offload(glb_eth->ppe, skb, pinfo);
	if(airoha_tunnel_hook_tx)
		airoha_tunnel_hook_tx(skb,glb_eth->ppe,pinfo);
	spin_lock_irq(&q->lock);
	if(skb->mark == DP_SPEED_UP)
	{
		hwe = airoha_ppe_foe_get_entry_locked(glb_eth->ppe, FOE_ENTRY_NUM(skb));
		speedtest_tx_offload(skb,hwe,glb_eth->ppe,pinfo);
	}

	txq = netdev_get_tx_queue(dev, qid);
	if (q->queued + nr_frags > q->ndesc) {
		/* not enough space in the queue */
		netif_tx_stop_queue(txq);
		spin_unlock_irq(&q->lock);
		return -1;
	}

	data = skb->data;
	index = q->head;

	for (i = 0; i < nr_frags; i++) {
		struct airoha_qdma_desc *desc = &q->desc[index];
		struct airoha_queue_entry *e = &q->entry[index];
		skb_frag_t *frag = &sinfo->frags[i];
		dma_addr_t addr;
		u32 val;

		addr = dma_map_single(dev->dev.parent, data, len,
				      DMA_TO_DEVICE);
		if (unlikely(dma_mapping_error(dev->dev.parent, addr)))
			goto error_unmap;

		index = (index + 1) % q->ndesc;

		val = FIELD_PREP(QDMA_DESC_LEN_MASK, len);
		if (i < nr_frags - 1)
			val |= FIELD_PREP(QDMA_DESC_MORE_MASK, 1);
		WRITE_ONCE(desc->ctrl, cpu_to_le32(val));
		WRITE_ONCE(desc->addr, cpu_to_le32(addr));
		val = FIELD_PREP(QDMA_DESC_NEXT_ID_MASK, index);
		WRITE_ONCE(desc->data, cpu_to_le32(val));
		WRITE_ONCE(desc->msg0, cpu_to_le32(msg0));
		WRITE_ONCE(desc->msg1, cpu_to_le32(msg1));
		WRITE_ONCE(desc->msg2, cpu_to_le32(msg2));

		e->skb = i ? NULL : skb;
		e->dma_addr = addr;
		e->dma_len = len;

		data = skb_frag_address(frag);
		len = skb_frag_size(frag);
	}

	q->head = index;
	q->queued += i;

	skb_tx_timestamp(skb);
	netdev_tx_sent_queue(txq, skb->len);

	if (netif_xmit_stopped(txq) || !netdev_xmit_more())
		airoha_qdma_rmw(qdma, REG_TX_CPU_IDX(qid),
				TX_RING_CPU_IDX_MASK,
				FIELD_PREP(TX_RING_CPU_IDX_MASK, q->head));

	if (q->ndesc - q->queued < q->free_thr)
		netif_tx_stop_queue(txq);

	spin_unlock_irq(&q->lock);

	return 0;

error_unmap:
	for (i--; i >= 0; i--) {
		index = (q->head + i) % q->ndesc;
		dma_unmap_single(dev->dev.parent, q->entry[index].dma_addr,
				 q->entry[index].dma_len, DMA_TO_DEVICE);
	}

	spin_unlock_irq(&q->lock);
error:
	dev_kfree_skb_any(skb);
	dev->stats.tx_dropped++;

	return 0;
}
EXPORT_SYMBOL(qdma_wan_tx);

void npu_get_entry_bind(struct sk_buff *skb, u32 hash, u32 reason)
{
	AIROHA_LOG(AIROHA_DEBUG_LEVEL_INFO, "Hash: %u\n", hash);
	if (hash < PPE_SRAM_NUM_ENTRIES)
		skb_set_hash(skb, hash,
				 PKT_HASH_TYPE_L4);

	AIROHA_LOG(AIROHA_DEBUG_LEVEL_INFO, "CPU_REASON: %u\n", reason);
	if (reason == PPE_CPU_REASON_HIT_UNBIND_RATE_REACHED){
		struct airoha_flow_table_entry *flow_e = airoha_flow_table_entry_get_by_hash(glb_eth->ppe, hash);
		if(flow_e)
			flow_e->ingress_dev_idx = WIFI_DEV;			
		airoha_ppe_check_skb(&glb_eth->ppe->dev, skb, hash, false);
	}
}

int airoha_ppe_is_wifi2G_dev(struct net_device *dev)
{
    return ((dev != NULL) && \
			(strncmp(dev->name,"phy0.0", 6) == 0));
}

void airoha_ppe_foe_flow_update_wifi_npu_offload(struct airoha_ppe *ppe, struct sk_buff *skb, struct port_info *pinfo)
{
	u32 hash = FOE_ENTRY_NUM(skb);
	int type;
	struct airoha_flow_table_entry *e;
	struct airoha_foe_entry *hwe;
	struct hlist_node *n;
//	struct airoha_foe_bridge br = {};
	unsigned int dscp = 0;
	unsigned int dscp_tmp = 0;
	u32 index, data;
	AIROHA_LOG(AIROHA_DEBUG_LEVEL_INFO, "hash at wifi_tx: %u\n", hash);
	if (hash >= PPE_SRAM_NUM_ENTRIES || !skb->l4_hash || skb->sw_hash != false) {
		AIROHA_LOG(AIROHA_DEBUG_LEVEL_INFO, "Incorrect hash:%u\n", hash);
        return;
    }
	
	spin_lock_bh(&ppe_lock);
	hwe = airoha_ppe_foe_get_entry_locked(ppe, hash);
	if (!hwe)
	{
		goto unlock;
	}
	
	type = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, hwe->ib1);
	AIROHA_LOG(AIROHA_DEBUG_LEVEL_INFO, "Packet type:%d\n", type);
	index = airoha_ppe_foe_get_entry_hash(hwe);
	
	if(airoha_ppe_is_wifi2G_dev(skb->dev)){		
		pinfo->nbq = 0;
	}
	else{		
		pinfo->nbq = 1;
	}
	switch (skb->protocol){
	case htons(ETH_P_IP):
		dscp_tmp = ipv4_get_dsfield(ip_hdr(skb)) & 0xfc;
		dscp = dscp_tmp >> 2;
		break;
	case htons(ETH_P_IPV6):
		dscp_tmp = ipv6_get_dsfield(ipv6_hdr(skb)) & 0xfc;
		dscp = dscp_tmp >> 2;
		break;
	default:
		break;
	}	
	hlist_for_each_entry_safe(e, n, &ppe->foe_flow[index], list) {
		if (airoha_ppe_foe_compare_entry(e, hwe)) {
			data = FIELD_PREP(AIROHA_FOE_ACTDP, pinfo->udf) |
	       			FIELD_PREP(AIROHA_FOE_SHAPER_ID, pinfo->tsid);
			
			switch (type) {
				case PPE_PKT_TYPE_IPV4_ROUTE:
				case PPE_PKT_TYPE_IPV4_HNAPT:
					e->data.ipv4.data = data;
					e->data.ipv4.l2.common.etype = pinfo->stag;
					e->data.ipv4.ib2 &= ~AIROHA_FOE_IB2_PSE_QOS;					
					e->data.ipv4.ib2 &= ~AIROHA_FOE_IB2_NBQ;
					e->data.ipv4.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, pinfo->nbq);	
					if(dscp)
						e->data.ipv4.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_DSCP, dscp);					
					e->data.ipv4.ib2 &= ~AIROHA_FOE_IB2_PSE_PORT;
					e->data.ipv4.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_CDM4);		//TDMA
					break;
				
				case PPE_PKT_TYPE_BRIDGE:
					e->data.bridge.data = data;
					e->data.bridge.l2.common.etype = pinfo->stag;
					e->data.bridge.ib2 &= ~AIROHA_FOE_IB2_PSE_QOS;					
					e->data.bridge.ib2 &= ~AIROHA_FOE_IB2_NBQ;
					e->data.bridge.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, pinfo->nbq);
					if(dscp)
						e->data.bridge.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_DSCP, dscp);					
					e->data.bridge.ib2 &= ~AIROHA_FOE_IB2_PSE_PORT;
					e->data.bridge.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_CDM4);					
					break;
				
				case PPE_PKT_TYPE_IPV6_ROUTE_3T:
				case PPE_PKT_TYPE_IPV6_ROUTE_5T:
				case PPE_PKT_TYPE_IPV6_6RD:
					e->data.ipv6.data = data;
					e->data.ipv6.l2.etype = pinfo->stag;
					e->data.ipv6.ib2 &= ~AIROHA_FOE_IB2_PSE_QOS;					
					e->data.ipv6.ib2 &= ~AIROHA_FOE_IB2_NBQ;
					e->data.ipv6.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, pinfo->nbq);
					if(dscp)
						e->data.ipv6.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_DSCP, dscp);					
					e->data.ipv6.ib2 &= ~AIROHA_FOE_IB2_PSE_PORT;
					e->data.ipv6.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_CDM4);					
					break;
					
				case PPE_PKT_TYPE_IPV4_DSLITE:
					e->data.dslite.data = data;		
					e->data.dslite.l2.common.etype = pinfo->stag;
					e->data.dslite.ib2 &= ~AIROHA_FOE_IB2_PSE_QOS;					
					e->data.dslite.ib2 &= ~AIROHA_FOE_IB2_NBQ;
					e->data.dslite.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, pinfo->nbq);	
					if(dscp)
						e->data.dslite.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_DSCP, dscp);
					e->data.dslite.ib2 &= ~AIROHA_FOE_IB2_PSE_PORT;
					e->data.dslite.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_CDM4);					
					break;
					
				default:
					break;
			}			
			break;
		}
	}
	
#if 0
	/*---START------ For Bridge Case, it should be get from L2_flows not foe_flow ---------*/
	airoha_ppe_foe_set_bridge_addrs(&br, eth_hdr(skb));
	e = rhashtable_lookup_fast(&ppe->l2_flows, &br,
				   airoha_l2_flow_table_params);
	if(e){
			data = FIELD_PREP(AIROHA_FOE_ACTDP, pinfo->udf) |
	       			FIELD_PREP(AIROHA_FOE_SHAPER_ID, pinfo->tsid);
			
			switch (type) {
				case PPE_PKT_TYPE_IPV4_ROUTE:
				case PPE_PKT_TYPE_IPV4_HNAPT:
					e->data.ipv4.data = data;
					e->data.ipv4.l2.common.etype = pinfo->stag;
					e->data.ipv4.ib2 &= ~AIROHA_FOE_IB2_PSE_QOS;					
					e->data.ipv4.ib2 &= ~AIROHA_FOE_IB2_NBQ;
					e->data.ipv4.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, pinfo->nbq);	
					e->data.ipv4.ib2 &= ~AIROHA_FOE_IB2_PSE_PORT;
					e->data.ipv4.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_CDM4);		//TDMA
					
					AIROHA_LOG(AIROHA_DEBUG_LEVEL_ERR, "Packet type:%d, hash at wifi_tx: %u, stag = %x, info2 = %x, data = %x\n", 
								type, hash, pinfo->stag, e->data.ipv4.ib2, e->data.ipv4.data);
					break;
				
				case PPE_PKT_TYPE_BRIDGE:
					e->data.bridge.data = data;
					e->data.bridge.l2.common.etype = pinfo->stag;
					e->data.bridge.ib2 &= ~AIROHA_FOE_IB2_PSE_QOS;					
					e->data.bridge.ib2 &= ~AIROHA_FOE_IB2_NBQ;
					e->data.bridge.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, pinfo->nbq);
					e->data.bridge.ib2 &= ~AIROHA_FOE_IB2_PSE_PORT;
					e->data.bridge.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_CDM4);					
					break;
				
				case PPE_PKT_TYPE_IPV6_ROUTE_3T:
				case PPE_PKT_TYPE_IPV6_ROUTE_5T:
				case PPE_PKT_TYPE_IPV6_6RD:
					e->data.ipv6.data = data;
					e->data.ipv6.l2.etype = pinfo->stag;
					e->data.ipv6.ib2 &= ~AIROHA_FOE_IB2_PSE_QOS;					
					e->data.ipv6.ib2 &= ~AIROHA_FOE_IB2_NBQ;
					e->data.ipv6.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, pinfo->nbq);	
					e->data.ipv6.ib2 &= ~AIROHA_FOE_IB2_PSE_PORT;
					e->data.ipv6.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_CDM4);					
					break;
					
				case PPE_PKT_TYPE_IPV4_DSLITE:
					e->data.dslite.data = data;		
					e->data.dslite.l2.common.etype = pinfo->stag;
					e->data.dslite.ib2 &= ~AIROHA_FOE_IB2_PSE_QOS;					
					e->data.dslite.ib2 &= ~AIROHA_FOE_IB2_NBQ;
					e->data.dslite.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, pinfo->nbq);	
					e->data.dslite.ib2 &= ~AIROHA_FOE_IB2_PSE_PORT;
					e->data.dslite.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_CDM4);					
					break;
					
				default:
					break;
			}			
			e->tx_modified_bit = 1; 
		}else{
		AIROHA_LOG(AIROHA_DEBUG_LEVEL_WARN, "wifi tx hash:%d Can't find Flow table and L2 Table \n",hash);
	}
	/*----END----- For Bridge Case, it should be get from L2_flows not foe_flow ---------*/
#endif						   
unlock:
	spin_unlock_bh(&ppe_lock);
}

int airoha_get_ppe_entry_state(unsigned int foe_index)
{
	if(glb_eth == NULL)
		return 0;
	
	struct airoha_ppe *ppe = glb_eth->ppe;
	struct airoha_foe_entry *hwe = NULL;
	
	hwe = airoha_ppe_foe_get_entry(ppe, foe_index);

	if(hwe == NULL)
		return 0;
	
	return FIELD_GET(AIROHA_FOE_IB1_BIND_STATE, hwe->ib1);
}
EXPORT_SYMBOL(airoha_get_ppe_entry_state);

#define PPE_MULTICAST_STATE_HANDLER_NUM (sizeof(state_handler)/sizeof(PPE_MULTICAST_FWD_STATE_HANDLER))
#define MULTICAST_GSW_EXIST(port_mask)		(port_mask & 0xF)
#define MULTICAST_HSGMII_EXIST(port_mask)	(port_mask & 0xF0)
#define MULTICAST_WLAN_EXIST(port_mask)		(port_mask & 0x80000000)

int (*arht_multicast_hwnat_data_handler_hook)(struct airoha_foe_entry *hwe, struct sk_buff* skb) = NULL;
EXPORT_SYMBOL(arht_multicast_hwnat_data_handler_hook);
int (*arht_xpon_igmp_get_fwd_ports_hook)(struct br_ip* addr,struct net_device* ports[],int nport) = NULL;
EXPORT_SYMBOL(arht_xpon_igmp_get_fwd_ports_hook);
int (*arht_multicast_hwnat_set_valid_hook)(unsigned int foe_index) = NULL;
EXPORT_SYMBOL(arht_multicast_hwnat_set_valid_hook);
int (*arht_multicast_hwnat_get_valid_hook)(unsigned int foe_index) = NULL;
EXPORT_SYMBOL(arht_multicast_hwnat_get_valid_hook);
int (*arht_soe_offload_get_valid_hook)(unsigned int foe_index) = NULL;
EXPORT_SYMBOL(arht_soe_offload_get_valid_hook);
int (*ra_sw_nat_hook_set_soe_info) (struct sk_buff * skb, unsigned char sa_index, unsigned char hop0, unsigned char hop1, unsigned char hop2) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_set_soe_info);
int (*arht_multicast_list_add_hook)(struct airoha_foe_entry *hwe, struct sk_buff* skb);
EXPORT_SYMBOL(arht_multicast_list_add_hook);


typedef struct
{
	unsigned int fwd_state;
	int (*handler_func)(struct airoha_ppe *ppe, struct airoha_foe_entry *hwe, unsigned int foe_index, unsigned int port_mask, int priority);
}PPE_MULTICAST_FWD_STATE_HANDLER;


static PPE_MULTICAST_FWD_STATE_HANDLER state_handler[]=
{
	{PPE_MULTICAST_FORWARD_STATE_LAN_ONLY,arht_multicast_hwnat_state_handler_lan_only},
	{PPE_MULTICAST_FORWARD_STATE_WLAN_ONLY,arht_multicast_hwnat_state_handler_wlan_only},
	{PPE_MULTICAST_FORWARD_STATE_LAN_WLAN,arht_multicast_hwnat_state_handler_lan_wlan},
	//{PPE_MULTICAST_FORWARD_STATE_LAN_XSI,ecnt_multicast_hwnat_state_handler_lan_xsi},
	{PPE_MULTICAST_FORWARD_STATE_XSI_ONLY,arht_multicast_hwnat_state_handler_xsi_only},
	//{PPE_MULTICAST_FORWARD_STATE_XSI_WLAN,ecnt_multicast_hwnat_state_handler_xsi_wlan},	
	{PPE_MULTICAST_FORWARD_STATE_LAN_HSGMII_1toN,arht_multicast_hwnat_state_handler_lan_hsgmii_1toN},
	{PPE_MULTICAST_FORWARD_STATE_UNKNOWN,arht_multicast_hwnat_state_handler_unknown},
};

int arht_ppe_multicast_handler(struct airoha_ppe *ppe, struct sk_buff* skb)
{
	unsigned int foe_index = FOE_ENTRY_NUM(skb);

	if(foe_index >= PPE_SRAM_NUM_ENTRIES || !skb->l4_hash || skb->sw_hash != false)
		return 0;
	
	if(arht_multicast_hwnat_data_handler_hook)
	{
		struct airoha_foe_entry *hwe = NULL;
		spin_lock_bh(&ppe_lock);
		hwe = airoha_ppe_foe_get_entry_locked(ppe, foe_index);
		spin_unlock_bh(&ppe_lock);

		arht_multicast_hwnat_data_handler_hook(hwe, skb);
	}
	return 0;
}

int arht_multicast_handler_for_sfu(struct sk_buff* skb)
{
	struct airoha_foe_entry *hwe = NULL;
	u32 hash = FOE_ENTRY_NUM(skb);

	if(glb_eth == NULL)
		return 0;
	
	if(!airoha_is_pon_sfu_mode())
		return 0;
	
	spin_lock_bh(&ppe_lock);
	hwe = airoha_ppe_foe_get_entry_locked(glb_eth->ppe, hash);
	
	if(ppe_is_multicast_entry(hwe)){
		spin_unlock_bh(&ppe_lock);
		arht_ppe_multicast_handler(glb_eth->ppe, skb);
		return 1;
	}
	spin_unlock_bh(&ppe_lock);
	return 0;
}
EXPORT_SYMBOL(arht_multicast_handler_for_sfu);

int arht_ppe_multicast_set_valid(struct sk_buff* skb)
{
	unsigned int foe_index = FOE_ENTRY_NUM(skb);

	if(arht_multicast_hwnat_set_valid_hook)
	{
		if(foe_index < PPE_SRAM_NUM_ENTRIES)
		{
			arht_multicast_hwnat_set_valid_hook(foe_index);
		}
	}
	return 0;
}

int arht_ppe_multicast_get_valid(unsigned int foe_index)
{
	if(arht_multicast_hwnat_get_valid_hook)
	{
		return arht_multicast_hwnat_get_valid_hook(foe_index);
	}
	return 0;
}

unsigned int arht_muliticast_get_forward_state(unsigned int port_mask, unsigned int local)
{
	if(MULTICAST_WLAN_EXIST(port_mask))
	{
		if(MULTICAST_GSW_EXIST(port_mask) || MULTICAST_HSGMII_EXIST(port_mask))
		{
			return PPE_MULTICAST_FORWARD_STATE_LAN_WLAN;
		}
		else
		{
			return PPE_MULTICAST_FORWARD_STATE_WLAN_ONLY;
		}
	}
	if(MULTICAST_HSGMII_EXIST(port_mask))
	{
		if(MULTICAST_GSW_EXIST(port_mask))
		{
			return PPE_MULTICAST_FORWARD_STATE_LAN_HSGMII_1toN;
		}
		else
		{
			return PPE_MULTICAST_FORWARD_STATE_XSI_ONLY;
		}
	}
	if(MULTICAST_GSW_EXIST(port_mask))
	{
		return PPE_MULTICAST_FORWARD_STATE_LAN_ONLY;
	}
	
	//if(port_mask == 0)
	//	return PPE_MULTICAST_FORWARD_STATE_UNKNOWN;
	
	return PPE_MULTICAST_FORWARD_STATE_UNKNOWN;
}

/* clear all fields in foe_entry besides info1 and old info */
void clear_foe_entry(struct airoha_foe_entry *hwe)
{
	int type = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, hwe->ib1);
	int offset = sizeof(hwe->ib1);

	if (type == PPE_PKT_TYPE_BRIDGE) {
		offset += offsetof(struct airoha_foe_bridge, data);
	} else if (type >= PPE_PKT_TYPE_IPV6_ROUTE_3T) {
		offset += offsetof(struct airoha_foe_ipv6, data);
	} else {
		offset += offsetof(struct airoha_foe_ipv4, ib2);
	}
	memset((char *)hwe + offset, 0, PPE_ENTRY_SIZE - offset);

	return;
}
EXPORT_SYMBOL(clear_foe_entry);

int setMcastDstMacv4(unsigned char *mac_hdr, struct airoha_foe_entry *foe_entry)
{
	mac_hdr[0] = 0x01;
	mac_hdr[1] = 0x00;
	mac_hdr[2] = 0x5e;
	mac_hdr[3] = ((foe_entry->ipv4.orig_tuple.dest_ip & 0xff0000) >> 16);
	mac_hdr[4] = ((foe_entry->ipv4.orig_tuple.dest_ip & 0xff00) >> 8);
	mac_hdr[5] = (foe_entry->ipv4.orig_tuple.dest_ip & 0xff);

	return 0;
}

int setMcastDstMacv6(unsigned char *mac_hdr, struct airoha_foe_entry *foe_entry)
{
	mac_hdr[0] = 0x33;
	mac_hdr[1] = 0x33;
	mac_hdr[2] = ((foe_entry->ipv6.dest_ip[3] & 0xff000000) >> 24);
	mac_hdr[3] = ((foe_entry->ipv6.dest_ip[3] & 0xff0000) >> 16);
	mac_hdr[4] = ((foe_entry->ipv6.dest_ip[3] & 0xff00) >> 8);
	mac_hdr[5] = (foe_entry->ipv6.dest_ip[3] & 0xff);

	return 0;
}

static void ppe_set_vlan_info(struct airoha_foe_entry *hwe, struct sk_buff *skb)
{
	u16 vn = 0, vid1 = 0, vid2 = 0;	
	u32 ib1;
	
	if(FIELD_GET(AIROHA_FOE_IB1_BIND_STATE, hwe->ib1) == AIROHA_FOE_STATE_BIND){
		return;
	}
	
	airoha_ppe_foe_get_vlan_info(skb, &vn, &vid1, &vid2);

	ib1 = hwe->ib1;
	ib1 &= ~(AIROHA_FOE_IB1_BIND_VLAN_LAYER | AIROHA_FOE_IB1_BIND_VPM);
	ib1 |= FIELD_PREP(AIROHA_FOE_IB1_BIND_VLAN_LAYER, vn) | 
					FIELD_PREP(AIROHA_FOE_IB1_BIND_VPM, !!vn);
	hwe->ib1 = ib1;

	if (IS_IPV4_GRP(hwe)) {
		hwe->ipv4.l2.common.vlan1 = vid1;
		hwe->ipv4.l2.common.vlan2 = vid2;
	}
	else if(IS_IPV6_GRP(hwe)){
		hwe->ipv6.l2.vlan1 = vid1;
		hwe->ipv6.l2.vlan2 = vid2;
	}
	else if(IS_L2_RRIDGE(hwe)){
		hwe->bridge.l2.common.vlan1 = vid1;
		hwe->bridge.l2.common.vlan2 = vid2;
	}

	return;
	
}

int32_t PpeFillInL2Info(struct sk_buff * skb, struct airoha_foe_entry *foe_entry)
{
	unsigned char *mac_hdr;
    //int pkt_type = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, foe_entry->ib1);
	//if this entry is already in binding state, skip it 
	if (FIELD_GET(AIROHA_FOE_IB1_BIND_STATE, foe_entry->ib1) == AIROHA_FOE_STATE_BIND) {
		return 1;
	}

    skb_reset_mac_header(skb);
	mac_hdr = skb_mac_header(skb);
    
	/* Set VLAN Info - VLAN1/VLAN2 */
	/* Set Layer2 Info - DMAC, SMAC */
	if (IS_IPV4_GRP(foe_entry)) {
		setMcastDstMacv4(mac_hdr,foe_entry);
		if(FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, foe_entry->ib1) == PPE_PKT_TYPE_IPV4_DSLITE) { //DS-Lite WAN->LAN
            FoeSetEntrySrcMac(mac_hdr, &(foe_entry->dslite.l2.common.src_mac_hi),&(foe_entry->dslite.l2.src_mac_lo));
            FoeSetEntryMac(mac_hdr,&(foe_entry->dslite.l2.common.dest_mac_hi),&(foe_entry->dslite.l2.common.dest_mac_lo));
		}else { //IPv4 WAN<->LAN
            FoeSetEntryMac(mac_hdr, &(foe_entry->ipv4.l2.common.dest_mac_hi), &(foe_entry->ipv4.l2.common.dest_mac_lo));
            FoeSetEntrySrcMac(mac_hdr, &(foe_entry->ipv4.l2.common.src_mac_hi), &(foe_entry->ipv4.l2.src_mac_lo));
		}
		if(airoha_is_pon_sfu_mode()){
			ppe_set_vlan_info(foe_entry,skb);
		}
	}
	else if(IS_IPV6_GRP(foe_entry)){
		setMcastDstMacv6(mac_hdr,foe_entry);
		// if smac doesn't change,then smac_idx[4]=1; else, smac_idx[3:0] =0~15
		set_ppe_entry_smac_index(foe_entry, PPE_PKT_TYPE_IPV6_ROUTE_5T, 0x10);
		
        FoeSetEntryMac(mac_hdr,&(foe_entry->ipv6.l2.dest_mac_hi), &(foe_entry->ipv6.l2.dest_mac_lo));

		if(airoha_is_pon_sfu_mode()){
			ppe_set_vlan_info(foe_entry,skb);
		}
	}

	return 0;
}

int32_t PpeFillInL3Info(struct sk_buff * skb, struct airoha_foe_entry *foe_entry)
{
	/* IPv4 */
	if (IS_IPV4_GRP(foe_entry)) {
		foe_entry->ipv4.new_tuple.src_ip = foe_entry->ipv4.orig_tuple.src_ip;
		foe_entry->ipv4.new_tuple.dest_ip = foe_entry->ipv4.orig_tuple.dest_ip;
	}
	else {
		//do nothing
		return 1;
	}

	return 0;
}

int arht_set_multicast_hwnat_info(struct sk_buff* skb, struct airoha_foe_entry *hwe)
{
	spin_lock_bh(&ppe_lock);
	/* Clear all fields in foe_entry besides info1 and old info */
	clear_foe_entry(hwe);
	
	/* Set Layer2 Info */
	if (PpeFillInL2Info(skb, hwe)) {
		spin_unlock_bh(&ppe_lock);
		return 0;
	}

	/* Set Layer3 Info */
	if (PpeFillInL3Info(skb, hwe)) {
		spin_unlock_bh(&ppe_lock);
		return 0;
	}
	spin_unlock_bh(&ppe_lock);

	return 1;
}
EXPORT_SYMBOL(arht_set_multicast_hwnat_info);

int arht_multicast_hwnat_state_handler(struct airoha_foe_entry *hwe, unsigned int foe_index,unsigned int port_mask,unsigned int local,int priority)
{
	unsigned int fwd_state = arht_muliticast_get_forward_state(port_mask, local);
	int i = 0;
    struct airoha_ppe *ppe = glb_eth->ppe;

	
	for(i = 0;i < PPE_MULTICAST_STATE_HANDLER_NUM;i++)
	{
		if(fwd_state == state_handler[i].fwd_state)
			return state_handler[i].handler_func(ppe, hwe, foe_index, port_mask,priority);
	}

	return 0;
}
EXPORT_SYMBOL(arht_multicast_hwnat_state_handler);

int arht_multicast_get_channel_by_stag(unsigned int stag_dp)
{
	int i, channel = 1, max_speed = 0;
	
	for (i = 0; i < 4; i++) {
		if(stag_dp & BIT(i+1)) {
			if(max_speed <= gsw_speed[i]) {
				max_speed = gsw_speed[i];
				channel = i+1;
			}
			if(max_speed == 1000) {
				break;
			}
		}
	}
	return channel ;
}

int arht_multicast_hwnat_state_handler_lan_only(struct airoha_ppe *ppe, struct airoha_foe_entry *hwe, unsigned int foe_index, unsigned int  port_mask, int priority)
{
	unsigned int stag_dp = 0;
	int i = 0, channel = 1;
	u32 data, val;
	int type;

	spin_lock_bh(&ppe_lock);

	hwe->ib1 &= ~AIROHA_FOE_IB1_BIND_STATE;
	hwe->ib1 |= FIELD_PREP(AIROHA_FOE_IB1_BIND_STATE, AIROHA_FOE_STATE_BIND);
				
	for(i = 0;i < 4;i++)
	{
		if(port_mask&(1<<i))
		{
			stag_dp |= (1<<(i+1));
		}
	}
	
	channel = arht_multicast_get_channel_by_stag(stag_dp);
	val = FIELD_PREP(AIROHA_FOE_IB2_PORT_AG, 0x1f) |
		  FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_GDM1) |
			  AIROHA_FOE_IB2_PSE_QOS | FIELD_PREP(AIROHA_FOE_IB2_NBQ, channel);
	
	type = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, hwe->ib1);
	data = FIELD_PREP(AIROHA_FOE_CHANNEL, channel) |
	       			FIELD_PREP(AIROHA_FOE_SHAPER_ID, 0x7F);
	switch (type) {
		case PPE_PKT_TYPE_IPV4_ROUTE:
		case PPE_PKT_TYPE_IPV4_HNAPT:
			hwe->ipv4.data = data;
			hwe->ipv4.ib2 = val;
			hwe->ipv4.l2.common.etype = stag_dp;
			break;
		
		case PPE_PKT_TYPE_BRIDGE:
			hwe->bridge.data = data;
			hwe->bridge.ib2 = val;
			hwe->bridge.l2.common.etype = stag_dp;
			break;
		
		case PPE_PKT_TYPE_IPV6_ROUTE_3T:
		case PPE_PKT_TYPE_IPV6_ROUTE_5T:
		case PPE_PKT_TYPE_IPV6_6RD:
			hwe->ipv6.data = data;
			hwe->ipv6.ib2 = val;
			hwe->ipv6.l2.etype = stag_dp;
			break;
			
		case PPE_PKT_TYPE_IPV4_DSLITE:
			hwe->dslite.data = data;
			hwe->dslite.ib2 = val;
			hwe->dslite.l2.common.etype = stag_dp;
			break;
			
		default:
			break;
	}
	if(arht_ppe_multicast_get_valid(foe_index) == HWNAT_MCAST_VALID){
		airoha_ppe_foe_commit_entry_ptr(ppe, hwe, foe_index,1);
	}
	spin_unlock_bh(&ppe_lock);
	
	return 0;
}

//not support, set invalid hwnat rule, make packet send to cpu
int arht_multicast_hwnat_state_handler_wlan_only(struct airoha_ppe *ppe, struct airoha_foe_entry *hwe, unsigned int foe_index, unsigned int  port_mask, int priority)
{
	
	airoha_ppe_delete_entry(ppe,hwe,foe_index);
	
	return 0;
}

//not support, set invalid hwnat rule, make packet send to cpu
int arht_multicast_hwnat_state_handler_lan_wlan(struct airoha_ppe *ppe, struct airoha_foe_entry *hwe, unsigned int foe_index, unsigned int  port_mask, int priority)
{
	airoha_ppe_delete_entry(ppe,hwe,foe_index);
#if 0
	unsigned int stag_dp = 0;
	int i = 0, channel = 1;
	u32 data, val, index;
	int type;
	struct airoha_flow_table_entry *e;

	hwe->ib1 &= ~AIROHA_FOE_IB1_BIND_STATE;
	hwe->ib1 |= FIELD_PREP(AIROHA_FOE_IB1_BIND_STATE, AIROHA_FOE_STATE_BIND);
				
	for(i = 0;i < 4;i++)
	{
		if(port_mask&(1<<i))
		{
			stag_dp |= (1<<(i+1));
		}
	}

	channel = arht_multicast_get_channel_by_stag(stag_dp);
	val = FIELD_PREP(AIROHA_FOE_IB2_PORT_AG, 0x1f) |
		  FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_GDM1) |
		  AIROHA_FOE_IB2_PSE_QOS | AIROHA_FOE_IB2_MULTICAST |
		  FIELD_PREP(AIROHA_FOE_IB2_NBQ, channel);
	
	type = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, hwe->ib1);
	data = FIELD_PREP(AIROHA_FOE_CHANNEL, channel) |
	       			FIELD_PREP(AIROHA_FOE_SHAPER_ID, 0x7F);
	switch (type) {
		case PPE_PKT_TYPE_IPV4_ROUTE:
		case PPE_PKT_TYPE_IPV4_HNAPT:
			hwe->ipv4.data = data;
			hwe->ipv4.ib2 = val;
			hwe->ipv4.l2.common.etype = stag_dp;
			break;
		
		case PPE_PKT_TYPE_BRIDGE:
			hwe->bridge.data = data;
			hwe->bridge.ib2 = val;
			hwe->bridge.l2.common.etype = stag_dp;
			break;
		
		case PPE_PKT_TYPE_IPV6_ROUTE_3T:
		case PPE_PKT_TYPE_IPV6_ROUTE_5T:
		case PPE_PKT_TYPE_IPV6_6RD:
			hwe->ipv6.data = data;
			hwe->ipv6.ib2 = val;
			hwe->ipv6.l2.etype = stag_dp;
			break;
			
		case PPE_PKT_TYPE_IPV4_DSLITE:
			hwe->dslite.data = data;
			hwe->dslite.ib2 = val;
			hwe->dslite.l2.common.etype = stag_dp;
			break;
			
		default:
			break;
	}

	airoha_ppe_foe_commit_entry(ppe, hwe, foe_index);

	index = airoha_ppe_foe_get_entry_hash(hwe);
								
	e = kzalloc(sizeof(*e), GFP_KERNEL);
	if (!e)
		return -ENOMEM;
	
	memcpy(&e->data, hwe, sizeof(e->data));
	hlist_add_head(&e->list, &ppe->foe_flow[index]);
	
#endif
	
	return 0;
}

//not support, set invalid hwnat rule, make packet send to cpu
int arht_multicast_hwnat_state_handler_xsi_only(struct airoha_ppe *ppe, struct airoha_foe_entry *hwe, unsigned int foe_index, unsigned int  port_mask, int priority)
{
	int channel = 1;
	u32 data, val;
	int type=0, nbq=0;
	struct airoha_eth *eth = glb_eth;
	
	spin_lock_bh(&ppe_lock);
	hwe->ib1 &= ~AIROHA_FOE_IB1_BIND_STATE;
	hwe->ib1 |= FIELD_PREP(AIROHA_FOE_IB1_BIND_STATE, AIROHA_FOE_STATE_BIND);
	
	type = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, hwe->ib1);


	if(port_mask & IS_ETH_SERDES)
	{
		val = FIELD_PREP(AIROHA_FOE_IB2_PORT_AG, 0x1f) | AIROHA_FOE_IB2_PSE_QOS | 
		  	FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, eth->chip->fport[SERDES_ETH_IDX]);
		
		nbq = eth->chip->nbq[SERDES_ETH_IDX];
		channel =eth->chip->chnl[SERDES_ETH_IDX];
	}
		
	if(port_mask & IS_USB_SERDES)
	{
		val = FIELD_PREP(AIROHA_FOE_IB2_PORT_AG, 0x1f) | AIROHA_FOE_IB2_PSE_QOS | 
		  	FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, eth->chip->fport[SERDES_USB_IDX]);
		nbq = eth->chip->nbq[SERDES_USB_IDX];
		channel = eth->chip->chnl[SERDES_USB_IDX];
	}

	if(port_mask & IS_PCIE0_SERDES)
	{
		val = FIELD_PREP(AIROHA_FOE_IB2_PORT_AG, 0x1f) | AIROHA_FOE_IB2_PSE_QOS | 
		  	FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, eth->chip->fport[SERDES_PCIE0_IDX]);
		nbq = eth->chip->nbq[SERDES_PCIE0_IDX];
		channel = eth->chip->chnl[SERDES_PCIE0_IDX];
	}

	if(port_mask & IS_PCIE1_SERDES)
	{
		val = FIELD_PREP(AIROHA_FOE_IB2_PORT_AG, 0x1f) | AIROHA_FOE_IB2_PSE_QOS | 
		  	FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, eth->chip->fport[SERDES_PCIE1_IDX]);
		nbq = eth->chip->nbq[SERDES_PCIE1_IDX];
		channel = eth->chip->chnl[SERDES_PCIE1_IDX];
	}
		
	data = FIELD_PREP(AIROHA_FOE_CHANNEL, channel) |
		   FIELD_PREP(AIROHA_FOE_QID, 
				((AIROHA_NUM_QOS_QUEUES - 1) - (priority % AIROHA_NUM_QOS_QUEUES))) |
	       FIELD_PREP(AIROHA_FOE_SHAPER_ID, 0x7F) ;
	val |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, nbq);

	switch (type) {
		case PPE_PKT_TYPE_IPV4_ROUTE:
		case PPE_PKT_TYPE_IPV4_HNAPT:
			hwe->ipv4.ib2 = val;
			hwe->ipv4.data = data;
			hwe->ipv4.l2.common.etype = 0;
			break;
		
		case PPE_PKT_TYPE_BRIDGE:
			hwe->bridge.ib2 = val;
			hwe->bridge.data = data;
			hwe->bridge.l2.common.etype = 0;
			break;
		
		case PPE_PKT_TYPE_IPV6_ROUTE_3T:
		case PPE_PKT_TYPE_IPV6_ROUTE_5T:
		case PPE_PKT_TYPE_IPV6_6RD:
			hwe->ipv6.ib2 = val;
			hwe->ipv6.data = data;
			hwe->ipv6.l2.etype = 0;
			break;
			
		case PPE_PKT_TYPE_IPV4_DSLITE:
			hwe->dslite.ib2 = val;
			hwe->dslite.data = data;
			hwe->dslite.l2.common.etype = 0;
			break;
			
		default:
			break;
	}

	if(arht_ppe_multicast_get_valid(foe_index) == HWNAT_MCAST_VALID){
		airoha_ppe_foe_commit_entry_ptr(ppe, hwe, foe_index,1);
	}
	
	spin_unlock_bh(&ppe_lock);
	
	return 0;
}

int arht_multicast_hwnat_state_handler_lan_hsgmii_1toN(struct airoha_ppe *ppe, struct airoha_foe_entry *hwe, unsigned int foe_index, unsigned int  port_mask, int priority)
{
	unsigned int stag_dp = 0;
	int i = 0, channel = 0, nbq = 0;
	u32 data, val;
	int type;

	spin_lock_bh(&ppe_lock);

	hwe->ib1 &= ~AIROHA_FOE_IB1_BIND_STATE;
	hwe->ib1 |= FIELD_PREP(AIROHA_FOE_IB1_BIND_STATE, AIROHA_FOE_STATE_BIND);
				
	for(i = 0;i < 4;i++)
	{
		if(port_mask&(1<<i))
		{
			stag_dp |= (1<<(i+1));
		}
	}

	if(port_mask & IS_ETH_SERDES){
		nbq |= (1<<2);
	}
	
	if(port_mask & IS_USB_SERDES){
		nbq |= (1<<1);
	}

	if(port_mask & IS_PCIE0_SERDES){
		channel |= (1<<4);
	}
	
	if(port_mask & IS_PCIE1_SERDES){
		nbq |= (1<<0);
	}
		
	val = FIELD_PREP(AIROHA_FOE_IB2_PORT_AG, 0x1f) |
		  FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_MCST) | 
		  AIROHA_FOE_IB2_PSE_QOS | FIELD_PREP(AIROHA_FOE_IB2_NBQ, nbq);
	
	type = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, hwe->ib1);
	data = FIELD_PREP(AIROHA_FOE_CHANNEL, channel) |
					FIELD_PREP(AIROHA_FOE_SHAPER_ID, 0x7F);
	switch (type) {
		case PPE_PKT_TYPE_IPV4_ROUTE:
		case PPE_PKT_TYPE_IPV4_HNAPT:
			hwe->ipv4.data = data;
			hwe->ipv4.ib2 = val;
			hwe->ipv4.l2.common.etype = stag_dp;
			break;
		
		case PPE_PKT_TYPE_BRIDGE:
			hwe->bridge.data = data;
			hwe->bridge.ib2 = val;
			hwe->bridge.l2.common.etype = stag_dp;
			break;
		
		case PPE_PKT_TYPE_IPV6_ROUTE_3T:
		case PPE_PKT_TYPE_IPV6_ROUTE_5T:
		case PPE_PKT_TYPE_IPV6_6RD:
			hwe->ipv6.data = data;
			hwe->ipv6.ib2 = val;
			hwe->ipv6.l2.etype = stag_dp;
			break;
			
		case PPE_PKT_TYPE_IPV4_DSLITE:
			hwe->dslite.data = data;
			hwe->dslite.ib2 = val;
			hwe->dslite.l2.common.etype = stag_dp;
			break;
			
		default:
			break;
	}

	if(arht_ppe_multicast_get_valid(foe_index) == HWNAT_MCAST_VALID){
		airoha_ppe_foe_commit_entry_ptr(ppe, hwe, foe_index,1);
	}
	spin_unlock_bh(&ppe_lock);

	return 0;
	
}


int arht_multicast_hwnat_state_handler_unknown(struct airoha_ppe *ppe, struct airoha_foe_entry *hwe, unsigned int foe_index, unsigned int  port_mask, int priority)
{
	u32 val;
	int type;
	
	spin_lock_bh(&ppe_lock);
	hwe->ib1 &= ~AIROHA_FOE_IB1_BIND_STATE;
	hwe->ib1 |= FIELD_PREP(AIROHA_FOE_IB1_BIND_STATE, AIROHA_FOE_STATE_BIND);


	val = FIELD_PREP(AIROHA_FOE_IB2_PORT_AG, 0x1f) |
		  FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_DROP);
	
	type = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, hwe->ib1);

	switch (type) {
		case PPE_PKT_TYPE_IPV4_ROUTE:
		case PPE_PKT_TYPE_IPV4_HNAPT:
			hwe->ipv4.ib2 = val;
			break;
		
		case PPE_PKT_TYPE_BRIDGE:
			hwe->bridge.ib2 = val;
			break;
		
		case PPE_PKT_TYPE_IPV6_ROUTE_3T:
		case PPE_PKT_TYPE_IPV6_ROUTE_5T:
		case PPE_PKT_TYPE_IPV6_6RD:
			hwe->ipv6.ib2 = val;
			break;
			
		case PPE_PKT_TYPE_IPV4_DSLITE:
			hwe->dslite.ib2 = val;
			break;
			
		default:
			break;
	}

	airoha_ppe_foe_commit_entry_ptr(ppe, hwe, foe_index,1);
	spin_unlock_bh(&ppe_lock);
	
	return 0;
}

int arht_multicast_hwnat_clean(unsigned int foe_index)
{
	struct airoha_foe_entry *hwe;
	if(glb_eth == NULL)
		return 0;
	
	spin_lock_bh(&ppe_lock);
	hwe = airoha_ppe_foe_get_entry_locked(glb_eth->ppe, foe_index);
	if(hwe == NULL)
		return 0;
	
	airoha_ppe_delete_entry(glb_eth->ppe,hwe,foe_index);
	spin_unlock_bh(&ppe_lock);

	return 0;
}
EXPORT_SYMBOL(arht_multicast_hwnat_clean);

int arht_multicast_hwnat_list_update(MULTICAST_HWNATENTRY_t* entry, unsigned int update_mode,unsigned int port_mask,unsigned int local)
{
	int find_flag = 0;
	unsigned int foe_index = 0;
	struct airoha_ppe *ppe = glb_eth->ppe;
	struct airoha_foe_entry *hwe;

	
    if ((PPE_MULTICAST_HWNATENTRY_STATE_UNBIND == entry->state)
        || (entry->port_mask == port_mask))
	{
		return -1;
	}
	else
	{
		foe_index = entry->foe_index;
		entry->port_mask  = port_mask;
		local = entry->local;

		if (0 == port_mask)
			entry->state = PPE_MULTICAST_HWNATENTRY_STATE_DROP;
		else
			entry->state = PPE_MULTICAST_HWNATENTRY_STATE_BINDED;
		find_flag = 1;
		hwe = airoha_ppe_foe_get_entry(ppe, foe_index);
		arht_multicast_hwnat_state_handler(hwe,foe_index,port_mask,local,0);

	}

	if(find_flag)
		return 0;

	return -1;
}
EXPORT_SYMBOL(arht_multicast_hwnat_list_update);

int arht_multicast_hwnat_list_update_lan(MULTICAST_HWNATENTRY_t* entry, unsigned int update_mode,unsigned int port_mask,unsigned int local)
{
	int find_flag = 0;
	unsigned int foe_index = 0;
	struct airoha_ppe *ppe = glb_eth->ppe;
	struct airoha_foe_entry *hwe;

	
    if ((PPE_MULTICAST_HWNATENTRY_STATE_UNBIND == entry->state)
        || (entry->port_mask == port_mask))
	{
		return -1;
	}
	else
	{
		foe_index = entry->foe_index;
		entry->port_mask  = port_mask;
		local = entry->local;

		if (0 == port_mask)
			entry->state = PPE_MULTICAST_HWNATENTRY_STATE_DROP;
		else
			entry->state = PPE_MULTICAST_HWNATENTRY_STATE_BINDED;
		find_flag = 1;
		hwe = airoha_ppe_foe_get_entry(ppe, foe_index);
		if (IS_IPV4_GRP(hwe)) {
			hwe->ipv4.new_tuple.src_ip = hwe->ipv4.orig_tuple.src_ip;
			hwe->ipv4.new_tuple.dest_ip = hwe->ipv4.orig_tuple.dest_ip;
		}

		arht_multicast_hwnat_state_handler(hwe,foe_index,port_mask,local,0);

	}

	if(find_flag)
		return 0;

	return -1;
}
EXPORT_SYMBOL(arht_multicast_hwnat_list_update_lan);

static int ppeChecConfigDone(uint reg, uint doneBit)
{
	int RETRY = 10;
	volatile uint regValue = 0 ;
	
	while(RETRY--) {
		regValue = airoha_fe_rr(glb_eth, reg);
		
		if(regValue & doneBit) {
			break ;
		}
	}
	if(RETRY < 0) {
		return -ETIME ;
	}

	return 0;
}

int ppeSetUpdateMemCtrl(unsigned int select, unsigned int addr, unsigned int offset)
{
	uint val=0 ;
	
	val = (PPE_UPDMEM_REQ | PPE_UPDMEM_WR 
		| ((addr<<PPE_UPDMEM_ADDR_SHIFT)&PPE_UPDMEM_ADDR_MASK)
		| ((offset<<PPE_UPDMEM_OFST_SHIFT)&PPE_UPDMEM_OFST_MASK)
		| ((select<<PPE_UPDMEM_SEL_SHIFT)&PPE_UPDMEM_SEL_MASK)) ;
	airoha_fe_wr(glb_eth, REG_UPDMEM_CTRL(0), val);

	if(ppeChecConfigDone(REG_UPDMEM_CTRL(0), PPE_UPDMEM_ACK) < 0) {
		printk("Timeout for set ppe update sram control configuration.\n") ;
		return -ETIME ;
	}
	
	return 0 ;
}
void ppeSetUpdMemData(unsigned int mac_data)
{
	airoha_fe_wr(glb_eth, REG_UPDMEM_CTRL(0), mac_data);
	return;
}

int ppeSetShrinkField(int select, int index, struct hwnat_shrink_field *shrinkFieldPtr)
{
	int i=0;
	unsigned int mac_data=0;
	
	if(select==PPE_UPDMEM_SEL_SMAC) {
		for(i=0; i<2; i++) {

			if(i==0)
				mac_data = (shrinkFieldPtr->smac[2]<<24) | (shrinkFieldPtr->smac[3]<<16) | (shrinkFieldPtr->smac[4]<<8) | shrinkFieldPtr->smac[5];
			else
				mac_data = (shrinkFieldPtr->smac[0]<<8) | shrinkFieldPtr->smac[1];
			ppeSetUpdMemData(mac_data);
			ppeSetUpdateMemCtrl(select, index, i);
		}
	} else if(select==PPE_UPDMEM_SEL_IPv4) {
		for(i=0; i<UPDMEM_IPV4_LINE; i++) {
			ppeSetUpdMemData(shrinkFieldPtr->eg_ipv4[i]);
			ppeSetUpdateMemCtrl(select, index, i);
		}
	} else if(select==PPE_UPDMEM_SEL_IPv6) {
		for(i=0; i<UPDMEM_IPV6_LINE; i++) {
			ppeSetUpdMemData(shrinkFieldPtr->eg_ipv6[i]);
			if(i<=3)
				ppeSetUpdateMemCtrl(select, index, 3-i);
			else 
				ppeSetUpdateMemCtrl(select, index, 11-i);
		}
	}
	
	return 0;
}

int find_and_update_shrink_table(int select, struct hwnat_shrink_field *shrinkFieldPtr)
{
	int index=0, invldIdx=UPDMEM_NUM;

	// find matched entry
	for(index=0; index<UPDMEM_NUM; index++) {
		if(shnkTbl[index].valid[select] == 1) {
			switch(select) {
				case PPE_UPDMEM_SEL_SMAC:
					if(memcmp(&shnkTbl[index].smac[0], &shrinkFieldPtr->smac[0], UPDMEM_SMAC_CNT) == 0) {
						goto success;
					}
					break;
					
				case PPE_UPDMEM_SEL_IPv4:
					if(memcmp(&shnkTbl[index].eg_ipv4[0], &shrinkFieldPtr->eg_ipv4[0], UPDMEM_IPV4_LINE*4) == 0) {
						goto success;
					}
					break;
					
				case PPE_UPDMEM_SEL_IPv6:
					if(memcmp(&shnkTbl[index].eg_ipv6[0], &shrinkFieldPtr->eg_ipv6[0], UPDMEM_IPV6_LINE*4) == 0) {
						goto success;
					}
					break;
					
				default:
					break;
			}
				
			// if current entry is time out, invalid this entry
//			if(time_after(jiffies, shnkTbl[index].timestamp[select]+timeOutVal)) {
//				shnkTbl[index].valid[select] = 0;
//			}
		}
		
		if((shnkTbl[index].valid[select] == 0) && (index<invldIdx))
			invldIdx = index;
	}

	// update entry if has position
	if(invldIdx==UPDMEM_NUM) {
		index=-1;
		goto fail; 
	} else {
		index = invldIdx;
		switch(select) {
			case PPE_UPDMEM_SEL_SMAC:
				memcpy(&shnkTbl[index].smac[0], &shrinkFieldPtr->smac[0], UPDMEM_SMAC_CNT);
				ppeSetShrinkField(select, index, shrinkFieldPtr);				
				break;
				
			case PPE_UPDMEM_SEL_IPv4:
				memcpy(&shnkTbl[index].eg_ipv4[0], &shrinkFieldPtr->eg_ipv4[0], UPDMEM_IPV4_LINE*4);
				ppeSetShrinkField(select, index, shrinkFieldPtr);
				break;
				
			case PPE_UPDMEM_SEL_IPv6:
				memcpy(&shnkTbl[index].eg_ipv6[0], &shrinkFieldPtr->eg_ipv6[0], UPDMEM_IPV6_LINE*4);
				ppeSetShrinkField(select, index, shrinkFieldPtr);
				break;
				
			default:
				break;
		}
	}
	
success:
	shnkTbl[index].valid[select] = 1;
	shnkTbl[index].timestamp[select] = jiffies;
	
fail:
	return index;
}

void PpeClearEntryInfo(struct airoha_foe_entry *foe_entry)
{
	char *foe_entry_point = (char *)foe_entry;

	if (FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, foe_entry->ib1) == PPE_PKT_TYPE_BRIDGE)
	{	
		memset(foe_entry_point+PPE_CLEAR_OFFSET4, 0, SIZE_OF_FOE_ENTRY-PPE_CLEAR_OFFSET4);
	}
	else if (FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, foe_entry->ib1) == PPE_PKT_TYPE_IPV4_DSLITE)
	{	
		memset(foe_entry_point+PPE_CLEAR_OFFSET1, 0, SIZE_OF_FOE_ENTRY-PPE_CLEAR_OFFSET4);
	}
	else if ((FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, foe_entry->ib1) == PPE_PKT_TYPE_IPV4_HNAPT) ||
		(FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, foe_entry->ib1) == PPE_PKT_TYPE_IPV4_ROUTE) ||
		(FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, foe_entry->ib1) == PPE_PKT_TYPE_BRIDGE))
	{	
		memset(foe_entry_point+PPE_CLEAR_OFFSET1, 0, SIZE_OF_FOE_ENTRY-PPE_CLEAR_OFFSET1);
	}
	else if((FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, foe_entry->ib1) == PPE_PKT_TYPE_IPV6_ROUTE_3T) ||
		(FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, foe_entry->ib1) == PPE_PKT_TYPE_IPV6_ROUTE_5T))
	{
		memset(foe_entry_point+PPE_CLEAR_OFFSET2, 0, SIZE_OF_FOE_ENTRY-PPE_CLEAR_OFFSET2);
	}
	else if((FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, foe_entry->ib1) == PPE_PKT_TYPE_IPV4_DSLITE) ||
		(FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, foe_entry->ib1) == PPE_PKT_TYPE_IPV6_6RD))
	{
        memset(foe_entry_point+PPE_CLEAR_OFFSET3, 0, SIZE_OF_FOE_ENTRY-PPE_CLEAR_OFFSET3);
	}
	else
	{
		printk("HNAT: unknow packet type\n");
	}
}


int32_t FillSpeedtestEntryInfo(struct sk_buff * skb, struct airoha_foe_entry *foe_entry)
{
	int index=0;
	struct hwnat_shrink_field shrinkField;

	struct ethhdr *eth = NULL;
	struct vlan_hdr *vh = NULL;
	struct pppoe_hdr *peh= NULL;

	uint16_t eth_type;
	uint16_t vlan=0,pppoe_sid=0,eth_type_origin=0;
	
	skb_reset_mac_header(skb);

	eth = (struct ethhdr *)skb->data;
	eth_type_origin =ntohs(eth->h_proto);
	eth_type=eth->h_proto;
	skb->data += ETH_HLEN;
	if(eth->h_proto== htons(0x8100))
	{
		vh = (struct vlan_hdr *)(skb->data);
		eth_type =vh->h_vlan_encapsulated_proto;
		skb->data += VLAN_HLEN;
		foe_entry->ib1 = (foe_entry->ib1 & ~(AIROHA_FOE_IB1_BIND_VPM | AIROHA_FOE_IB1_BIND_VLAN_LAYER)) | 
								FIELD_PREP(AIROHA_FOE_IB1_BIND_VLAN_LAYER, 1) | 
								FIELD_PREP(AIROHA_FOE_IB1_BIND_VPM, 1) ;
		vlan= ntohs(vh->h_vlan_TCI);
	}

	if(eth_type == htons(ETH_P_PPP_SES)){

		peh = (struct pppoe_hdr *)(skb->data);
		if (peh->ver != 1 || peh->type != 1)
		return 1;
		pppoe_sid=ntohs(peh->sid);
		skb->data += 8;
		foe_entry->ib1 = (foe_entry->ib1 & ~AIROHA_FOE_IB1_BIND_PPPOE) | 
									FIELD_PREP(AIROHA_FOE_IB1_BIND_PPPOE, 1);
	}


	skb_reset_network_header(skb);
	PpeClearEntryInfo(foe_entry);
	if ((eth_type == htons(ETH_P_IP)) || (eth_type == htons(ETH_P_PPP_SES)
		&& peh->tag[0].tag_type == htons(PPP_IP))) 
	/* Set VLAN Info - VLAN1/VLAN2 */
	/* Set Layer2 Info - DMAC, SMAC */
 {
 			
            FoeSetEntryMac(eth->h_dest,&(foe_entry->ipv4.l2.common.dest_mac_hi),&(foe_entry->ipv4.l2.common.dest_mac_lo));
            FoeSetEntryMac(eth->h_source,&(foe_entry->ipv4.l2.common.src_mac_hi),&(foe_entry->ipv4.l2.src_mac_lo));
			foe_entry->ipv4.l2.common.vlan1 = vlan;
			foe_entry->ipv4.l2.pppoe_id= pppoe_sid;
			if(eth->h_proto== htons(0x8100))
			foe_entry->ipv4.l2.common.etype = eth_type_origin;

			foe_entry->ipv4.new_tuple.src_ip = foe_entry->ipv4.orig_tuple.src_ip;
			foe_entry->ipv4.new_tuple.dest_ip = foe_entry->ipv4.orig_tuple.dest_ip;

			foe_entry->ipv4.new_tuple.src_port= foe_entry->ipv4.orig_tuple.src_port;
			foe_entry->ipv4.new_tuple.dest_port= foe_entry->ipv4.orig_tuple.dest_port;
			foe_entry->ib1 = (foe_entry->ib1 & ~AIROHA_FOE_IB1_BIND_UDP) | 
										FIELD_PREP(AIROHA_FOE_IB1_BIND_UDP, TCP);
			
		
	} 
	else {
			// if smac doesn't change,then smac_idx[4]=1; else, smac_idx[3:0] =0~15
			if(cmpMacInfo(eth->h_source, skb->data) == HWNAT_SUCCESS) {
				set_ppe_entry_smac_index(foe_entry, PPE_PKT_TYPE_IPV6_ROUTE_5T, 0x10);
			} else {
				memcpy(shrinkField.smac, eth->h_source, ETH_ALEN);

				index = find_and_update_shrink_table(PPE_UPDMEM_SEL_SMAC, &shrinkField);
				if(index != -1) {
					set_ppe_entry_smac_index(foe_entry, PPE_PKT_TYPE_IPV6_ROUTE_5T, index);
				} else {
					printk("smac: find and update shrink table failed!\n");
				}
			}
            FoeSetEntryMac(eth->h_dest,&(foe_entry->ipv6.l2.dest_mac_hi), &(foe_entry->ipv6.l2.dest_mac_lo));

			foe_entry->ipv6.l2.vlan1 = vlan;
			foe_entry->ipv6.l2.src_mac_hi = (foe_entry->ipv6.l2.src_mac_hi & ~AIROHA_FOE_MAC_PPPOE_ID) | 
										FIELD_PREP(AIROHA_FOE_MAC_PPPOE_ID, pppoe_sid);
			if(eth->h_proto== htons(0x8100))
			foe_entry->ipv6.l2.etype = eth_type_origin;
	}

	return 0;
}


int isValidPpeEntry(struct sk_buff *skb, struct airoha_foe_entry *foe_entry)
{
	unsigned char dmac[ETH_ALEN];
	memset(dmac, 0, sizeof(dmac));

	if (IS_IPV4_GRP(foe_entry)) {
		if ((foe_entry->ipv4.orig_tuple.src_ip== 0) && (foe_entry->ipv4.orig_tuple.dest_ip== 0))
			return 0;
	} else if (IS_IPV6_GRP(foe_entry)) {
		if ((foe_entry->ipv6.src_ip[0] == 0) && (foe_entry->ipv6.src_ip[1] == 0)
			&& (foe_entry->ipv6.src_ip[2] == 0) && (foe_entry->ipv6.src_ip[3] == 0)
			&& (foe_entry->ipv6.dest_ip[0] == 0) && (foe_entry->ipv6.dest_ip[1] == 0)
			&& (foe_entry->ipv6.dest_ip[2] == 0) && (foe_entry->ipv6.dest_ip[3] == 0))
			return 0;
	} else if (IS_L2_RRIDGE(foe_entry)) {
		//do nothing

	} else {
		return 1;
	}

	return 1;
}

void SetSpeedtestPortInfo(struct airoha_foe_entry * foe_entry, struct airoha_ppe *ppe,struct port_info *pinfo)
{

  
    u32 channel = 0, qdata=0, val=0, priority=0;
	foe_entry->ib1 = (foe_entry->ib1 & ~(AIROHA_FOE_IB1_BIND_STATE)) | 
					FIELD_PREP(AIROHA_FOE_IB1_BIND_STATE, AIROHA_FOE_STATE_BIND);

	val = FIELD_PREP(AIROHA_FOE_IB2_PORT_AG, 0x1f) |
		  FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_GDM2) | 
		  FIELD_PREP(AIROHA_FOE_IB2_NBQ, pinfo->channel) | 
		  AIROHA_FOE_IB2_PSE_QOS | 
		  AIROHA_FOE_IB2_FAST_PATH;
	channel = pinfo->channel;

	qdata = FIELD_PREP(AIROHA_FOE_CHANNEL, channel) |
	       FIELD_PREP(AIROHA_FOE_QID, priority) |
	       FIELD_PREP(AIROHA_FOE_SHAPER_ID, 0x7f);

	if (IS_IPV4_GRP(foe_entry)) 
	{
		foe_entry->ipv4.l2.common.etype = pinfo->stag;
		foe_entry->ipv4.data = qdata;
		foe_entry->ipv4.ib2 = val;
	}else  
	{
		foe_entry->ipv6.l2.etype = pinfo->stag;
		foe_entry->ipv6.data = qdata;
		foe_entry->ipv6.ib2 = val;
	}

	return ;

}
int speedtest_tx_offload(struct sk_buff * skb, struct airoha_foe_entry *foe_entry,struct airoha_ppe *ppe,struct port_info *pinfo)
{
	int ret = 0;
	u32 foe_entry_idx= 0;

	foe_entry_idx = skb->hash & AIROHA_PPE_ENTRY_MASK;
	
	/* get start fill entry for each layer */
	FillSpeedtestEntryInfo(skb, foe_entry);

	if(!isValidPpeEntry(skb, foe_entry)) {
		skb->hash = 0;
		ret = 1;
		skb->data = skb_mac_header(skb);
		return ret;
	}

	/* Set force port info */
	SetSpeedtestPortInfo(foe_entry,ppe,pinfo);

	airoha_ppe_foe_commit_entry_ptr(ppe,foe_entry,foe_entry_idx,1);		
	ret = 1;
	skb->data = skb_mac_header(skb);
	return ret;
}

int airoha_eth_fast_tx(struct sk_buff *skb, int channel){
	struct net_device *dev = glb_eth->ports[0]->dev;
	struct airoha_gdm_port *port = netdev_priv(dev);
	struct airoha_qdma *qdma = port->qdma;
	u32 nr_frags, msg0, msg1, len;
	struct netdev_queue *txq;
	struct airoha_queue *q;
	
	void *data;
	int i, qid;
	u16 index,tag;
	skb->dev = dev;
	
	skb_set_queue_mapping(skb, 7);

	qid = skb_get_queue_mapping(skb) % ARRAY_SIZE(qdma->q_tx);

    if(skb->inner_protocol == PPE_MAGIC_LOCAL_OUT){
        tag = PPE_MAGIC_LOCAL_OUT;
    }else{
	tag = DP_SPEED_UP;//use in sptag for pingpong stream
    }

	msg0 = FIELD_PREP(QDMA_ETH_TXMSG_TCO_MASK, 1) |
		   FIELD_PREP(QDMA_ETH_TXMSG_CHAN_MASK,
			  channel) |
		   FIELD_PREP(QDMA_ETH_TXMSG_QUEUE_MASK,
			  qid % AIROHA_NUM_QOS_QUEUES) |
			  FIELD_PREP(QDMA_ETH_TXMSG_SP_TAG_MASK, tag);
	
	if (skb->ip_summed == CHECKSUM_PARTIAL)
		msg0 |= FIELD_PREP(QDMA_ETH_TXMSG_TCO_MASK, 1) |
			FIELD_PREP(QDMA_ETH_TXMSG_UCO_MASK, 1) |
			FIELD_PREP(QDMA_ETH_TXMSG_ICO_MASK, 1);

	/* TSO: fill MSS info in tcp checksum field */
	if (skb_is_gso(skb)) {
		if (skb_cow_head(skb, 0))
			goto error;

		if (skb_shinfo(skb)->gso_type & (SKB_GSO_TCPV4 |
						 SKB_GSO_TCPV6)) {
			__be16 csum = cpu_to_be16(skb_shinfo(skb)->gso_size);

			tcp_hdr(skb)->check = (__force __sum16)csum;
			msg0 |= FIELD_PREP(QDMA_ETH_TXMSG_TSO_MASK, 1) |
					FIELD_PREP(QDMA_ETH_TXMSG_UCO_MASK, 1) |
					FIELD_PREP(QDMA_ETH_TXMSG_ICO_MASK, 1);
		}
	} 
	
	msg1 =0x7f4007ff;
	q = &qdma->q_tx[qid];
	if (WARN_ON_ONCE(!q->ndesc))
		goto error;

	spin_lock_irq(&q->lock);

	txq = netdev_get_tx_queue(dev, qid);
	nr_frags = 1 + skb_shinfo(skb)->nr_frags;

	if (q->queued + nr_frags > q->ndesc) {
		/* not enough space in the queue */
		spin_unlock_irq(&q->lock);
		goto error;
	}

	len = skb_headlen(skb);
	data = skb->data;
	index = q->head;

	for (i = 0; i < nr_frags; i++) {
		struct airoha_qdma_desc *desc = &q->desc[index];
		struct airoha_queue_entry *e = &q->entry[index];
		skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
		dma_addr_t addr;
		u32 val;
		addr = dma_map_single(dev->dev.parent, data, len,
					  DMA_TO_DEVICE);
		if (unlikely(dma_mapping_error(dev->dev.parent, addr)))
			goto error_unmap;

		index = (index + 1) % q->ndesc;

		val = FIELD_PREP(QDMA_DESC_LEN_MASK, len);
		if (i < nr_frags - 1){
			val |= FIELD_PREP(QDMA_DESC_MORE_MASK, 1);
		}
		WRITE_ONCE(desc->ctrl, cpu_to_le32(val));
		WRITE_ONCE(desc->addr, cpu_to_le32(addr));

		val = FIELD_PREP(QDMA_DESC_NEXT_ID_MASK, index);
		WRITE_ONCE(desc->data, cpu_to_le32(val));
		WRITE_ONCE(desc->msg0, cpu_to_le32(msg0));
		WRITE_ONCE(desc->msg1, cpu_to_le32(msg1));

		e->skb = i ? NULL : skb;
		e->dma_addr = addr;
		e->dma_len = len;

		data = skb_frag_address(frag);
		len = skb_frag_size(frag);
	}
	

	q->head = index;
	q->queued += i;

	netdev_tx_sent_queue(txq, skb->len);
	airoha_qdma_rmw(qdma, REG_TX_CPU_IDX(qid),
			TX_RING_CPU_IDX_MASK,
			FIELD_PREP(TX_RING_CPU_IDX_MASK, q->head));

	spin_unlock_irq(&q->lock);

	return NETDEV_TX_OK;

error_unmap:
	for (i--; i >= 0; i--) {
		index = (q->head + i) % q->ndesc;
		dma_unmap_single(dev->dev.parent, q->entry[index].dma_addr,
				 q->entry[index].dma_len, DMA_TO_DEVICE);
	}

	spin_unlock_irq(&q->lock);
error:
	dev_kfree_skb_any(skb);
	dev->stats.tx_dropped++;

	return NETDEV_TX_OK;
}

int SendToPpe(struct sk_buff * skb){
	
	unsigned int ip_ver = 0;
	unsigned char tmp_dst_mac[] = {0x0e,0x69,0x10,0x13,0x4d,0x2d};
	unsigned char tmp_src_mac[] = {0x00,0x00,0x00,0xff,0xee,0xdd};
	unsigned int paddingLength = 0;
	unsigned int skbLenTmp = 0;
	struct sk_buff * skb2 = NULL;
	struct iphdr *iph = NULL;
	iph = (struct iphdr *)skb->data;
 	ip_ver = iph->version;

	/* 1. add mac */
	skb = skb_unshare(skb, GFP_ATOMIC);
	if (!skb) {
		return 0;
	}

	if(skb_headroom(skb) <ETH_HLEN)
	{
		struct sk_buff *skb_tmp = skb_realloc_headroom(skb,14);
		dev_kfree_skb(skb);
		if(skb_tmp == NULL){
			return 0;
		}
		skb = skb_tmp;
	}
	
	skb_push(skb, 14);

	/* 2. fill in layer2 information */
	memcpy(skb->data, tmp_dst_mac, 6);
	memcpy(skb->data + 6, tmp_src_mac, 6);    


	if (4 == ip_ver){
		*(u16 *)(skb->data+12) = htons(ETH_P_IP);
	}else if (6 == ip_ver){
		*(u16 *)(skb->data+12) = htons(ETH_P_IPV6);
	}

	if (skb->len < 64)
	{
		paddingLength = 64 - skb->len;
		skbLenTmp = skb->len;
	
		if(skb_tailroom(skb) < paddingLength){
			skb2 = skb_copy_expand(skb, skb_headroom(skb), paddingLength, GFP_ATOMIC);
			if (skb2) {
				kfree_skb(skb);
				skb = skb2;
				skb_put(skb, paddingLength);
			}
		}
		else {
			skb_put(skb, paddingLength);
		}
		memset(skb->data + skbLenTmp, 0, paddingLength);
	}
	/* 3.redirect to PPE, send packet to PPE directly */
	airoha_eth_fast_tx(skb, 7);

	return 1;
}

int arht_ppe_soe_offload_get_valid(struct sk_buff* skb)
{
	unsigned int foe_index = FOE_ENTRY_NUM(skb);

	if(arht_soe_offload_get_valid_hook)
	{
		if(foe_index < PPE_SRAM_NUM_ENTRIES)
		{
			return arht_soe_offload_get_valid_hook(foe_index);
		}
	}
	return 0;
}

int airoha_ppe_drop_packet_handler(struct sk_buff *skb)
{
	u32 hash = FOE_ENTRY_NUM(skb);
	struct airoha_foe_entry *hwe;
	struct airoha_ppe *ppe;
	u32 val;
	int type, ret = 0;

	if(glb_eth == NULL)
		return 0;
	ppe = glb_eth->ppe;

	if(!airoha_is_pon_sfu_mode()){
		return 0;
	}

	if (hash >= PPE_SRAM_NUM_ENTRIES || !skb->l4_hash || skb->sw_hash != false) {
        return 0;
    }

	spin_lock_bh(&ppe_lock);
	hwe = airoha_ppe_foe_get_entry_locked(ppe, hash);

	if (!hwe)
	{
		ret = 0;
		goto unlock;
	}

	if(ppe_is_multicast_entry(hwe))
	{
		if(arht_multicast_list_add_hook)
			arht_multicast_list_add_hook(hwe,skb);
	}

	hwe->ib1 &= ~AIROHA_FOE_IB1_BIND_STATE;
	hwe->ib1 |= FIELD_PREP(AIROHA_FOE_IB1_BIND_STATE, AIROHA_FOE_STATE_BIND);


	val = FIELD_PREP(AIROHA_FOE_IB2_PORT_AG, 0x1f) |
		  FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_DROP);
	
	type = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, hwe->ib1);

	switch (type) {
		case PPE_PKT_TYPE_IPV4_ROUTE:
		case PPE_PKT_TYPE_IPV4_HNAPT:
			hwe->ipv4.ib2 = val;
			break;
		
		case PPE_PKT_TYPE_BRIDGE:
			hwe->bridge.ib2 = val;
			break;
		
		case PPE_PKT_TYPE_IPV6_ROUTE_3T:
		case PPE_PKT_TYPE_IPV6_ROUTE_5T:
		case PPE_PKT_TYPE_IPV6_6RD:
			hwe->ipv6.ib2 = val;
			break;
			
		case PPE_PKT_TYPE_IPV4_DSLITE:
			hwe->dslite.ib2 = val;
			break;
			
		default:
			break;
	}
	ret = 1;

unlock:	
	airoha_ppe_foe_commit_entry_ptr(ppe, hwe, hash,1);
	spin_unlock_bh(&ppe_lock);

	return ret;

}

void airoha_ppe_hook_init(void)
{
	rcu_assign_pointer(ra_sw_nat_hook_drop_packet, airoha_ppe_drop_packet_handler);
	return ;
}

void airoha_ppe_hook_exit(void)
{
	rcu_assign_pointer(ra_sw_nat_hook_drop_packet, NULL);
	return ;
}

int airoha_qdma_get_trtcm_param(struct airoha_qdma *qdma, int channel,
				       u32 addr, enum trtcm_param_type param,
				       enum trtcm_mode_type mode,
				       u32 *val_low, u32 *val_high)
{
	u32 idx = QDMA_METER_IDX(channel), group = QDMA_METER_GROUP(channel);
	u32 val, config = FIELD_PREP(TRTCM_PARAM_TYPE_MASK, param) |
			  FIELD_PREP(TRTCM_METER_GROUP_MASK, group) |
			  FIELD_PREP(TRTCM_PARAM_INDEX_MASK, idx) |
			  FIELD_PREP(TRTCM_PARAM_RATE_TYPE_MASK, mode);

	airoha_qdma_wr(qdma, REG_TRTCM_CFG_PARAM(addr), config);
	if (read_poll_timeout(airoha_qdma_rr, val,
			      val & TRTCM_PARAM_RW_DONE_MASK,
			      USEC_PER_MSEC, 10 * USEC_PER_MSEC, true,
			      qdma, REG_TRTCM_CFG_PARAM(addr)))
		return -ETIMEDOUT;

	*val_low = airoha_qdma_rr(qdma, REG_TRTCM_DATA_LOW(addr));
	if (val_high)
		*val_high = airoha_qdma_rr(qdma, REG_TRTCM_DATA_HIGH(addr));

	return 0;
}

static int airoha_qdma_set_trtcm_param(struct airoha_qdma *qdma, int channel,
				       u32 addr, enum trtcm_param_type param,
				       enum trtcm_mode_type mode, u32 val)
{
	uint valueLo_tmp = 0;
	uint valueHi_tmp = 0;
	int retry_num = 100;
	u32 idx = QDMA_METER_IDX(channel), group = QDMA_METER_GROUP(channel);
	u32 config = TRTCM_PARAM_RW_MASK |
		     FIELD_PREP(TRTCM_PARAM_TYPE_MASK, param) |
		     FIELD_PREP(TRTCM_METER_GROUP_MASK, group) |
		     FIELD_PREP(TRTCM_PARAM_INDEX_MASK, idx) |
		     FIELD_PREP(TRTCM_PARAM_RATE_TYPE_MASK, mode);
			 
	do{	
		airoha_qdma_wr(qdma, REG_TRTCM_DATA_LOW(addr), val);
		wmb();
		airoha_qdma_wr(qdma, REG_TRTCM_CFG_PARAM(addr), config);
	
		airoha_qdma_get_trtcm_param(qdma, channel, addr, param, mode, &valueLo_tmp, &valueHi_tmp);
		if(val == valueLo_tmp)
			break;
	}while(--retry_num); //check if write successfully

	return read_poll_timeout(airoha_qdma_rr, val,
				 val & TRTCM_PARAM_RW_DONE_MASK,
				 USEC_PER_MSEC, 10 * USEC_PER_MSEC, true,
				 qdma, REG_TRTCM_CFG_PARAM(addr));
}

static int airoha_qdma_set_trtcm_config(struct airoha_qdma *qdma, int channel,
					u32 addr, enum trtcm_mode_type mode,
					bool enable, u32 enable_mask)
{
	u32 val;

	if (airoha_qdma_get_trtcm_param(qdma, channel, addr, TRTCM_MISC_MODE,
					mode, &val, NULL))
		return -EINVAL;

	val = enable ? val | enable_mask : val & ~enable_mask;

	return airoha_qdma_set_trtcm_param(qdma, channel, addr, TRTCM_MISC_MODE,
					   mode, val);
}

static int airoha_qdma_set_trtcm_token_bucket(struct airoha_qdma *qdma,
					      int channel, u32 addr,
					      enum trtcm_mode_type mode,
					      u32 rate_val, u32 bucket_size)
{
	u32 val, config, tick, unit, rate, rate_frac, rl_mode_cfg, bucket_unit;
	int err;

	if (airoha_qdma_get_trtcm_param(qdma, channel, addr, TRTCM_MISC_MODE,
					mode, &config, NULL))
		return -EINVAL;

	val = airoha_qdma_rr(qdma, addr);
	tick = FIELD_GET(INGRESS_FAST_TICK_MASK, val);
	if (config & TRTCM_TICK_SEL)
		tick *= FIELD_GET(INGRESS_SLOW_TICK_RATIO_MASK, val);
	if (!tick)
		return -EINVAL;

	unit = (config & TRTCM_PKT_MODE) ? 1000000 / tick : 8000 / tick;
	if (!unit)
		return -EINVAL;

	rate = rate_val / unit;
	rate_frac = rate_val % unit;
	rate_frac = FIELD_PREP(TRTCM_TOKEN_RATE_MASK, rate_frac) / unit;
	rate = FIELD_PREP(TRTCM_TOKEN_RATE_MASK, rate) |
	       FIELD_PREP(TRTCM_TOKEN_RATE_FRACTION_MASK, rate_frac);

	err = airoha_qdma_set_trtcm_param(qdma, channel, addr,
					  TRTCM_TOKEN_RATE_MODE, mode, rate);
	if (err)
		return err;

	val = max_t(u32, bucket_size, MIN_TOKEN_SIZE);
	/* val = min_t(u32, __fls(val), MAX_TOKEN_SIZE_OFFSET); */
    err = airoha_qdma_get_trtcm_param(qdma, channel, REG_EGRESS_TRTCM_CFG, TRTCM_MISC_MODE, mode, &rl_mode_cfg, NULL);
    if(err){
        return err;
    }

    if (TRTCM_PKT_MODE == ((rl_mode_cfg & TRTCM_PKT_MODE) >> 1))
    {
		bucket_unit = trtcmBucketPacketUnit[EGRESS_TRTCM];
	}else{
		bucket_unit = trtcmBucketByteUnit[EGRESS_TRTCM];
	}

	val = biSearchGetBucketSizeShift(val, 0, 17, bucket_unit);
	val = min_t(u32, val, MAX_TOKEN_SIZE_OFFSET);

	return airoha_qdma_set_trtcm_param(qdma, channel, addr,
					   TRTCM_BUCKETSIZE_SHIFT_MODE,
					   mode, val);
}

int airoha_qdma_set_tx_rate_limit(struct airoha_gdm_port *port,
					 int channel, u32 rate,
					 u32 bucket_size)
{
	int i, err;

	for (i = 0; i <= TRTCM_PEAK_MODE; i++) {
		err = airoha_qdma_set_trtcm_config(port->qdma, channel,
						   REG_EGRESS_TRTCM_CFG, i,
						   !!rate, TRTCM_METER_MODE);
		if (err)
			return err;

		err = airoha_qdma_set_trtcm_token_bucket(port->qdma, channel,
							 REG_EGRESS_TRTCM_CFG,
							 i, rate, bucket_size);
		if (err)
			return err;
	}

	return 0;
}

int qdmalan_airoha_qdma_get_trtcm_param(struct airoha_qdma *qdma, int channel,
				       u32 addr, enum trtcm_param_type param,
				       enum trtcm_mode_type mode,
				       u32 *val_low, u32 *val_high)
{
	u32 idx = QDMA_METER_IDX(channel), group = QDMA_METER_GROUP(channel);
	u32 config = FIELD_PREP(TRTCM_PARAM_TYPE_MASK, param) |
			  FIELD_PREP(TRTCM_METER_GROUP_MASK, group) |
			  FIELD_PREP(TRTCM_PARAM_INDEX_MASK, idx) |
			  FIELD_PREP(TRTCM_PARAM_RATE_TYPE_MASK, mode);

	airoha_qdma_wr(qdma, REG_TRTCM_CFG_PARAM(addr), config);

	*val_low = airoha_qdma_rr(qdma, REG_TRTCM_DATA_LOW(addr));
	if (val_high)
		*val_high = airoha_qdma_rr(qdma, REG_TRTCM_DATA_HIGH(addr));

	return 0;
}

static int qdmalan_airoha_qdma_set_trtcm_param(struct airoha_qdma *qdma, int channel,
				       u32 addr, enum trtcm_param_type param,
				       enum trtcm_mode_type mode, u32 val)
{
	uint valueLo_tmp = 0;
	uint valueHi_tmp = 0;
	int retry_num = 100;
	u32 idx = QDMA_METER_IDX(channel), group = QDMA_METER_GROUP(channel);
	u32 config = TRTCM_PARAM_RW_MASK |
		     FIELD_PREP(TRTCM_PARAM_TYPE_MASK, param) |
		     FIELD_PREP(TRTCM_METER_GROUP_MASK, group) |
		     FIELD_PREP(TRTCM_PARAM_INDEX_MASK, idx) |
		     FIELD_PREP(TRTCM_PARAM_RATE_TYPE_MASK, mode);
			 
	do{	
		airoha_qdma_wr(qdma, REG_TRTCM_DATA_LOW(addr), val);
		wmb();
		airoha_qdma_wr(qdma, REG_TRTCM_CFG_PARAM(addr), config);
	
		qdmalan_airoha_qdma_get_trtcm_param(qdma, channel, addr, param, mode, &valueLo_tmp, &valueHi_tmp);
		if(val == valueLo_tmp)
			break;
	}while(--retry_num); //check if write successfully

	
	return 0;
}

static int qdmalan_airoha_qdma_set_trtcm_config(struct airoha_qdma *qdma, int channel,
					u32 addr, enum trtcm_mode_type mode,
					bool enable, u32 enable_mask)
{
	u32 val;

	if (qdmalan_airoha_qdma_get_trtcm_param(qdma, channel, addr, TRTCM_MISC_MODE,
					mode, &val, NULL))
		return -EINVAL;

	val = enable ? val | enable_mask : val & ~enable_mask;

	return qdmalan_airoha_qdma_set_trtcm_param(qdma, channel, addr, TRTCM_MISC_MODE,
					   mode, val);
}

static int qdmalan_airoha_qdma_set_trtcm_token_bucket(struct airoha_qdma *qdma,
					      int channel, u32 addr,
					      enum trtcm_mode_type mode,
					      u32 rate_val, u32 bucket_size)
{
	u32 val, config, tick, unit, rate, rate_frac, rl_mode_cfg, bucket_unit;
	int err;

	if (qdmalan_airoha_qdma_get_trtcm_param(qdma, channel, addr, TRTCM_MISC_MODE,
					mode, &config, NULL))
		return -EINVAL;

	val = airoha_qdma_rr(qdma, addr);
	tick = FIELD_GET(INGRESS_FAST_TICK_MASK, val);
	if (config & TRTCM_TICK_SEL)
		tick *= FIELD_GET(INGRESS_SLOW_TICK_RATIO_MASK, val);
	if (!tick)
		return -EINVAL;

	unit = (config & TRTCM_PKT_MODE) ? 1000000 / tick : 8000 / tick;
	if (!unit)
		return -EINVAL;

	rate = rate_val / unit;
	rate_frac = rate_val % unit;
	rate_frac = FIELD_PREP(TRTCM_TOKEN_RATE_MASK, rate_frac) / unit;
	rate = FIELD_PREP(TRTCM_TOKEN_RATE_MASK, rate) |
	       FIELD_PREP(TRTCM_TOKEN_RATE_FRACTION_MASK, rate_frac);

	err = qdmalan_airoha_qdma_set_trtcm_param(qdma, channel, addr,
					  TRTCM_TOKEN_RATE_MODE, mode, rate);
	if (err)
		return err;

	val = max_t(u32, bucket_size, MIN_TOKEN_SIZE);
	/* val = min_t(u32, __fls(val), MAX_TOKEN_SIZE_OFFSET); */
    err = qdmalan_airoha_qdma_get_trtcm_param(qdma, channel, REG_EGRESS_TRTCM_CFG, TRTCM_MISC_MODE, mode, &rl_mode_cfg, NULL);
    if(err){
        return err;
    }

    if (TRTCM_PKT_MODE == ((rl_mode_cfg & TRTCM_PKT_MODE) >> 1))
    {
		bucket_unit = trtcmBucketPacketUnit[EGRESS_TRTCM];
	}else{
		bucket_unit = trtcmBucketByteUnit[EGRESS_TRTCM];
	}

	val = biSearchGetBucketSizeShift(val, 0, 17, bucket_unit);
	val = min_t(u32, val, MAX_TOKEN_SIZE_OFFSET);

	return qdmalan_airoha_qdma_set_trtcm_param(qdma, channel, addr,
					   TRTCM_BUCKETSIZE_SHIFT_MODE,
					   mode, val);
}

int qdma_set_qdmalan_tx_ratelimit(QDMA_TxRateLimitSet_T *txRateLimitPtr)
{
	int i, err;
	struct airoha_qdma *qdma = &glb_eth->qdma[0];
	int channel = txRateLimitPtr->chnlIdx;
	int rate = txRateLimitPtr->rateLimitValue;

	for (i = 0; i <= TRTCM_PEAK_MODE; i++) {
		err = qdmalan_airoha_qdma_set_trtcm_config(qdma, channel,
						   REG_EGRESS_TRTCM_CFG, i,
						   !!rate, TRTCM_METER_MODE);
		if (err)
			return err;

		err = qdmalan_airoha_qdma_set_trtcm_token_bucket(qdma, channel,
							 REG_EGRESS_TRTCM_CFG,
							 i, rate, 4096000);
		if (err)
			return err;
	}

	return 0;
}
EXPORT_SYMBOL(qdma_set_qdmalan_tx_ratelimit);


static struct delayed_work  airoha_eth_monitor_workqueue;
u32 gsw_speed[4] = {0};
bool eth_lastlinks[ARHT_ETH_PORT_MAX] = {0};
static int airoha_fe_gdm_rls(struct airoha_gdm_port *port)
{
	/*Only support gdm4 channel release for 7581 */
	if (port->id != 4)
		return -EINVAL;
	
	int ret;
	u32 val = 0;
	if(glb_eth->fe_regs){
		
		airoha_fe_rmw(glb_eth, REG_GDM4_CHN_RLS, REG_GDM4_TX_CHN_ID|REG_GDM4_TX_CHN_EN, 
					FIELD_PREP(REG_GDM4_TX_CHN_ID, 0)|FIELD_PREP(REG_GDM4_TX_CHN_EN, 1));
					
		ret = read_poll_timeout(airoha_fe_rr, val,
					val & REG_GDM4_TX_CHN_DN,
					USEC_PER_MSEC, 100 * USEC_PER_MSEC,
					false, glb_eth, REG_GDM4_CHN_RLS);	
					
		airoha_fe_rmw(glb_eth, REG_GDM4_CHN_RLS, REG_GDM4_TX_CHN_ID|REG_GDM4_TX_CHN_EN, 
					FIELD_PREP(REG_GDM4_TX_CHN_ID, 0)|FIELD_PREP(REG_GDM4_TX_CHN_EN, 0));			
	}
	return ret;
}

static void airoha_gdma_start_TxRx_channel(struct airoha_gdm_port *port)
{
	if(glb_eth->fe_regs){
		airoha_fe_wr(glb_eth, REG_GDM_TXCHN_EN(port->id), 0xffffffff);
		airoha_fe_wr(glb_eth, REG_GDM_RXCHN_EN(port->id), 0xffff);
	}	
}

static void airoha_gdma_stop_TxRx_channel(struct airoha_gdm_port *port)
{
	if(glb_eth->fe_regs){
		airoha_fe_wr(glb_eth, REG_GDM_TXCHN_EN(port->id), 0x0);
		airoha_fe_wr(glb_eth, REG_GDM_RXCHN_EN(port->id), 0x0);
	}
}

static void airoha_xsi_mac_start(struct airoha_gdm_port *port)
{
	/*Note: Start xsi mac mbi mpi by sequence*/
	if(port->xfi_mac){
		regmap_clear_bits(port->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_GIB_CFG,
					AIROHA_PCS_XFI_RXMBI_STOP);
		msleep(1);			
		regmap_clear_bits(port->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_GIB_CFG,
					AIROHA_PCS_XFI_RXMPI_STOP);	
		regmap_clear_bits(port->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_GIB_CFG,
					AIROHA_PCS_XFI_TXMPI_STOP);			
		regmap_clear_bits(port->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_GIB_CFG,
					AIROHA_PCS_XFI_TXMBI_STOP);

	}	
}

static void airoha_xsi_mac_stop(struct airoha_gdm_port *port)
{
	
	/*Note: Stop xsi mac mbi mpi by sequence*/
	if(port->xfi_mac){
		regmap_set_bits(port->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_GIB_CFG,
					AIROHA_PCS_XFI_RXMPI_STOP);
		regmap_set_bits(port->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_GIB_CFG,
					AIROHA_PCS_XFI_RXMBI_STOP);
		regmap_set_bits(port->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_GIB_CFG,
					AIROHA_PCS_XFI_TXMBI_STOP);
		msleep(1);
		regmap_set_bits(port->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_GIB_CFG,
					AIROHA_PCS_XFI_TXMPI_STOP);
	}
}

static void airoha_xsi_mac_reset(struct airoha_gdm_port *port)
{
	if(port->xfi_mac){
		regmap_clear_bits(port->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_LOGIC_RST,
				  AIROHA_PCS_XFI_MAC_LOGIC_RST);
		regmap_set_bits(port->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_LOGIC_RST,
				AIROHA_PCS_XFI_MAC_LOGIC_RST);
	}		
}
static void airoha_qdma_start_channel(struct airoha_gdm_port *port,uint channel)
{
	
	int  i = 0;
	if(port->qdma){
		for (i = 0; i < AIROHA_NUM_QOS_QUEUES; i++)
			airoha_qdma_clear(port->qdma, REG_QUEUE_CLOSE_CFG(channel),
					  TXQ_DISABLE_CHAN_QUEUE_MASK(channel, i));	
	}
}

static void airoha_qdma_stop_channel(struct airoha_gdm_port *port,uint channel)
{
	int  i = 0;
	if(port->qdma){
		for (i = 0; i < AIROHA_NUM_QOS_QUEUES; i++)
			airoha_qdma_set(port->qdma, REG_QUEUE_CLOSE_CFG(channel),
					  TXQ_DISABLE_CHAN_QUEUE_MASK(channel, i));
	}
}

static u32 airoha_eth_gsw_get_link_rate(struct net_device *eth_dev)
{
	struct ethtool_link_ksettings ecmd;
	u32 speed;
	memset(&ecmd,0,sizeof(ecmd));
	eth_dev->ethtool_ops->get_link_ksettings(eth_dev,&ecmd);
	speed = ecmd.base.speed; 
	return speed;
}
static void airoha_eth_gsw_link_rate_update(struct airoha_gdm_port *port, int idx, u32 speed)
{
	if(glb_eth){
		/*Set tx qdma channel ratelimit based on link rate of lan port*/
		airoha_qdma_set_tx_rate_limit(port, idx+1, speed*1000, 4096);
	}else{
		printk("%s:glb_eth = NULL \n",__func__);
	}
	
}	
static void airoha_eth_mbi_hang_unlock_by_arbit_reset(struct airoha_gdm_port *port)
{
    int ret, val = 0;
	if(glb_eth->fe_regs){
		/*Tx terminate*/
		ret = read_poll_timeout(airoha_fe_rr, val,
					!(val & REG_GDM4_MBI_TX_BUSY),
					USEC_PER_MSEC, 100 * USEC_PER_MSEC,
					false, glb_eth, REG_GDM4_CHN_RLS);	
						
		if(ret){
			airoha_fe_rmw(glb_eth, FE_RESET_GLO, GDM4_MBI_ARB_TX_RST, 
						FIELD_PREP(GDM4_MBI_ARB_TX_RST, 1));
						
			ret = read_poll_timeout(airoha_fe_rr, val,
					!(val & REG_GDM4_MBI_TX_BUSY),
					USEC_PER_MSEC, 100 * USEC_PER_MSEC,
					false, glb_eth, REG_GDM4_CHN_RLS);		
				
			if(ret)
				printk(" Error: %s MBI TX Hang issue Terminate Fail! \n",__func__);
			
			airoha_fe_rmw(glb_eth, FE_RESET_GLO, GDM4_MBI_ARB_TX_RST, 
						FIELD_PREP(GDM4_MBI_ARB_TX_RST, 1));
			
			ret = read_poll_timeout(airoha_fe_rr, val,
					!(val & REG_GDM4_MBI_TX_BUSY),
					USEC_PER_MSEC, 100 * USEC_PER_MSEC,
					false, glb_eth, REG_GDM4_CHN_RLS);
			if(ret)
				printk(" Error2: %s MBI TX Hang issue Terminate Fail! \n",__func__);		
		
		}
		
		
		/*Rx terminate*/
		ret = read_poll_timeout(airoha_fe_rr, val,
					!(val & REG_GDM4_MBI_RX_BUSY),
					USEC_PER_MSEC, 100 * USEC_PER_MSEC,
					false, glb_eth, REG_GDM4_CHN_RLS);	
		
		if(ret){
			airoha_fe_rmw(glb_eth, FE_RESET_GLO, GDM4_MBI_ARB_RX_RST, 
						FIELD_PREP(GDM4_MBI_ARB_RX_RST, 1));
						
			ret = read_poll_timeout(airoha_fe_rr, val,
					!(val & REG_GDM4_MBI_RX_BUSY),
					USEC_PER_MSEC, 100 * USEC_PER_MSEC,
					false, glb_eth, REG_GDM4_CHN_RLS);	
				
			if(ret)
				printk(" Error: %s MBI RX Hang issue Terminate Fail! \n",__func__);
			
			airoha_fe_rmw(glb_eth, FE_RESET_GLO, GDM4_MBI_ARB_RX_RST, 
						FIELD_PREP(GDM4_MBI_ARB_RX_RST, 1));
						
			ret = read_poll_timeout(airoha_fe_rr, val,
					!(val & REG_GDM4_MBI_RX_BUSY),
					USEC_PER_MSEC, 100 * USEC_PER_MSEC,
					false, glb_eth, REG_GDM4_CHN_RLS);	
			if(ret)
				printk(" Error2: %s MBI RX Hang issue Terminate Fail! \n",__func__);		
					
		}
	}	
	
}
static void airoha_eth_monitor_link_up_protection(struct airoha_gdm_port *port, struct net_device *eth_dev, int channel)
{
		/*Start QDMA channel & MAC when Link Up*/
		airoha_gdma_start_TxRx_channel(port);
		
		if (port->id != 1){
			airoha_xsi_mac_start(port);
		}
		
		airoha_qdma_start_channel(port,channel);
		
		printk("\n [ARIOHA_ETH_MONITOR] Itf - %s Link Up Check Success! \n",eth_dev->name);
}

static void airoha_eth_monitor_link_down_protection(struct airoha_gdm_port *port, struct net_device *eth_dev, int channel)
{
	int ret = 0;
	if (glb_eth){
		airoha_flow_table_entries_lan(&glb_eth->flow_table,eth_dev);
		
		airoha_qdma_stop_channel(port,channel);
		
		if (port->id != 1){	
			/*Rleasing Process for Serdes Port*/
			airoha_xsi_mac_stop(port);
			msleep(1);
			airoha_gdma_stop_TxRx_channel(port);
			
			if (ra_sw_nat_hook_clean_table)
				ra_sw_nat_hook_clean_table();

			if(glb_eth->chip->set_channel_retire){
				ret = glb_eth->chip->set_channel_retire(glb_eth,port);
			}
			else{
				ret = airoha_fe_gdm_rls(port);
				airoha_eth_mbi_hang_unlock_by_arbit_reset(port);
				airoha_xsi_mac_reset(port);
			}
			if(ret < 0 )
				printk("\n [ARIOHA_ETH_MONITOR] Itf - %s Link Down GDM%d Rls Fail! \n",eth_dev->name,port->id);
			else
				printk("\n [ARIOHA_ETH_MONITOR] Itf - %s Link Down GDM%d Rls Success! \n",eth_dev->name,port->id);
			
			//msleep(1);
			//airoha_gdma_start_TxRx_channel(port);
			
		}
		//for gsw
		else
		{
			if (ra_sw_nat_hook_clean_table)
				ra_sw_nat_hook_clean_table();
		}
	}	
}

void airoha_eth_monitor_gsw_process(struct airoha_gdm_port *port)
{
	struct net_device *eth_dev = NULL;
	int i = 0;
	EphyMonitor();
	for(i=0; i<AIROHA_MAX_GSW_LAN_PORTS; i++)
	{
		char ifname[8] = {0};
		snprintf(ifname,sizeof(ifname),"lan%d",i+1);
		eth_dev = dev_get_by_name(&init_net,ifname);
		
		if(eth_dev && netif_running(eth_dev) && netif_device_present(eth_dev))
		{					
			rtnl_lock();
			
			/*1. Detect Link Rate*/
			if(eth_dev->ethtool_ops &&eth_dev->ethtool_ops->get_link_ksettings)
			{
				u32 speed;				
				speed = airoha_eth_gsw_get_link_rate(eth_dev);
				if(speed != SPEED_UNKNOWN && speed != gsw_speed[i]){ 
					airoha_eth_gsw_link_rate_update(port,i,speed);
					printk("%s: %s LinkRate changed for %d to %u!\n",__func__,eth_dev->name,gsw_speed[i],speed);
					gsw_speed[i] = speed;
				}
			}
			
			/*2. Detect Link Status*/
			     /*Link down to up*/
			if(!eth_lastlinks[i]&& eth_dev->ethtool_ops->get_link(eth_dev)){
				
				airoha_eth_monitor_link_up_protection(port, eth_dev, i+1);
				eth_lastlinks[i] = 1;
				
			     /* Link up to down*/	
			}else if (eth_lastlinks[i]&& !eth_dev->ethtool_ops->get_link(eth_dev)){
				
				airoha_eth_monitor_link_down_protection(port, eth_dev, i+1);	
				eth_lastlinks[i] = 0;
			}
			rtnl_unlock();	
			dev_put(eth_dev);
		}	
	}	
}

void airoha_eth_monitor_serdes_process(struct airoha_gdm_port *port)
{
	struct net_device *eth_dev = port->dev;
	/*Detect Link Status*/ 
	/*Link down to up*/
	if (!eth_dev){
		printk("Error ! %s: Invaild eth dev port->id:%d\n",__func__,port->id);
		return;
	}
	
	if (!eth_lastlinks[ARHT_ETH_PORT_2] && (netif_running(eth_dev)&& netif_carrier_ok(eth_dev))){
		if (glb_eth){
			if (port && port->qdma){
				airoha_eth_monitor_link_up_protection(port, eth_dev, HSGMII_LAN_ETH_CHNL);
			}
		}
		eth_lastlinks[ARHT_ETH_PORT_2] = 1;
				
				/* Link up to down*/
	}else if (eth_lastlinks[ARHT_ETH_PORT_2] && (!netif_running(eth_dev) || !netif_carrier_ok(eth_dev))){	
		if (glb_eth){
			if (port && port->qdma){
				/*Stop QDMA channel & MAC when Link Down*/
				airoha_eth_monitor_link_down_protection(port, eth_dev, HSGMII_LAN_ETH_CHNL);
			}
			
		}
		eth_lastlinks[ARHT_ETH_PORT_2] = 0;
	}
}

void airoha_eth_monitor(struct work_struct *work)
{
	int p;
	
	//fe_oq_stat_monitor();
	for(p = 0; p < ARRAY_SIZE(glb_eth->ports); p++)
	{
		if(!glb_eth->chip->support_eth_monitor(p))
			continue;
		
		struct airoha_gdm_port *port = glb_eth->ports[p];
		struct net_device *eth_dev = NULL;
		
		eth_dev = port->dev;
		
		if(netdev_uses_dsa(eth_dev)){
			/*Detect eth0 - lan1~4*/
			airoha_eth_monitor_gsw_process(port);
		}else{
			/*Detect eth2*/
			airoha_eth_monitor_serdes_process(port);
		}
		
		
	}
	
	if(glb_eth->work_started)
		schedule_delayed_work(&airoha_eth_monitor_workqueue,msecs_to_jiffies(125));
}

void airoha_eth_monitor_workqueue_init(struct airoha_eth *eth)
{
	if (!eth)
		return;

	if(eth->chip->support_eth_monitor){
		if(!eth->work_started)
		{
			eth->work_started = true;
			printk("[AIROHA_ETH] ETH State Monitor Active \n");
			INIT_DELAYED_WORK(&airoha_eth_monitor_workqueue, airoha_eth_monitor);
			schedule_delayed_work(&airoha_eth_monitor_workqueue,msecs_to_jiffies(50000));
		}
	}
}

void airoha_eth_monitor_workqueue_exit(struct airoha_eth *eth)
{
	if (!eth)
		return;

	if(eth->chip->support_eth_monitor){
		if(eth->work_started)
		{
			eth->work_started = false;
			cancel_delayed_work_sync(&airoha_eth_monitor_workqueue);	
		}
	}
}

int airoha_hwnat_clean_lan_entry_by_fport_and_channel(struct airoha_gdm_port *port, int channel)
{
	
	u8 fport, hwe_fport = 0;
	struct airoha_foe_entry *hwe;
	unsigned int type, hwe_chnl = 0;
	int sram_entries_num = (port->qdma->eth->airoha_chip_id == ID_EN7581) ? 
		PPE_SRAM_NUM_ENTRIES : PPE1_SRAM_NUM_ENTRIES;

	fport = port->qdma->eth->chip->get_fe_fport(NULL, port->dev, port->id);

	spin_lock_bh(&ppe_lock);
	for (int i = 0; i < sram_entries_num; i++)
	{		
		hwe = airoha_ppe_foe_get_entry_locked(port->qdma->eth->ppe, i);

		if(FIELD_GET(AIROHA_FOE_IB1_BIND_STATE, hwe->ib1) != AIROHA_FOE_STATE_BIND)
			continue;
		type = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, hwe->ib1);
		
		switch (type) {
			case PPE_PKT_TYPE_IPV4_HNAPT:
			case PPE_PKT_TYPE_IPV4_ROUTE:
				hwe_fport = FIELD_GET(AIROHA_FOE_IB2_PSE_PORT,hwe->ipv4.ib2);
				hwe_chnl = FIELD_GET(AIROHA_FOE_CHANNEL,hwe->ipv4.data);
				break;
			
			case PPE_PKT_TYPE_BRIDGE:
				hwe_fport = FIELD_GET(AIROHA_FOE_IB2_PSE_PORT,hwe->bridge32.ib2);
				hwe_chnl = FIELD_GET(AIROHA_FOE_CHANNEL,hwe->bridge32.data);
				break;
			
			case PPE_PKT_TYPE_IPV6_ROUTE_3T:
			case PPE_PKT_TYPE_IPV6_ROUTE_5T:
				hwe_fport = FIELD_GET(AIROHA_FOE_IB2_PSE_PORT,hwe->ipv6.ib2);
				hwe_chnl = FIELD_GET(AIROHA_FOE_CHANNEL,hwe->ipv6.data);
				break;
	
			default:
				break;
		}

		if((channel == hwe_chnl)&& (fport == hwe_fport))
		{			
			memset(hwe, 0, sizeof(*hwe));
			airoha_ppe_foe_commit_entry_ptr(port->qdma->eth->ppe, hwe, i,0);
		}
		
	}
	spin_unlock_bh(&ppe_lock);
	return 0;
	
}

static int airoha_check_pon_rx_hook(void *pMsg,struct sk_buff *skb, uint pktLen)
{
	/*
	PON RX process hook 
	Ruturn: if pwan_cb_rx_hook return success iterate to next packet otherwise it go to eth rx
	*/
	if(pwan_cb_rx_hook){
		pwan_cb_rx_hook(pMsg,QDMA_RX_DSCP_MSG_LENS,skb,pktLen);
	}else{
		return PON_RX_FAIL;
	}

	return PON_RX_SUCCESS;
}

static int airoha_force_to_cpu_handler(struct sk_buff *skb, unsigned short stag)
{
	if(stag == PPE_UDF_LOCAL_IN && likely(arht_force_to_cpu_hook))
	{
		arht_force_to_cpu_hook(skb, stag);
	}else{
		dev_kfree_skb(skb);
	}

	return 0;
}

int airoha_receive_hook(struct sk_buff *skb, struct airoha_qdma_desc *desc, int p, int len, u32 sptag)
{
	int sport, crsn, VirIfIdx = 0;
	struct pwan_msg pMsg;

	pMsg.msg0 = le32_to_cpu(desc->msg0);
	pMsg.msg1 = le32_to_cpu(desc->msg1);
		
	crsn = FIELD_GET(QDMA_ETH_RXMSG_CRSN_MASK, pMsg.msg1);
	sport = FIELD_GET(AIROHA_RXD4_SPORT, pMsg.msg1);
	VirIfIdx = FIELD_GET(QDMA_ETH_RXMSG_SPTAG_PINGPONG, pMsg.msg0);
	AIROHA_LOG(AIROHA_DEBUG_LEVEL_INFO, "sport: %d, VirIfIdx: %d",sport,VirIfIdx);

	if(crsn == 0x16){
		airoha_force_to_cpu_handler(skb, VirIfIdx);
		return 0;
	}
	
	if(sport == FE_PSE_PORT_CDM1){
		if(wan_speed_test_pinpong_handle_hook && (VirIfIdx == DP_SPEED_UP)){
			u32 hash = FIELD_GET(AIROHA_RXD4_FOE_ENTRY, pMsg.msg1);
			skb->mark = DP_SPEED_UP;
			skb_set_hash(skb, hash,PKT_HASH_TYPE_L4);
			wan_speed_test_pinpong_handle_hook(skb);
			return 0;
		}

		if(local_out_pingpong_hook && (VirIfIdx == PPE_MAGIC_LOCAL_OUT)){
			local_out_pingpong_hook(skb);
			return 0;
		}
	}

	pMsg.msg2 = le32_to_cpu(desc->msg2);
	pMsg.msg3 = le32_to_cpu(desc->msg3);
			
	if(p==1 && (airoha_check_pon_rx_hook((void *)&pMsg,skb,len)==PON_RX_SUCCESS)){
		return 0;
	}
	
	if(wan_speed_test_hook && wan_speed_test_hook(skb)==SPEED_TEST_SUCCESS)
	{
		return 0;			
	}

	if(airoha_pon_sfu_point_to_point_transmit_hook)
	{
		if(airoha_pon_sfu_point_to_point_transmit_hook(skb, sptag))
			return 0;
	}
	
	return 1;
}

int ppe_entry_is_valid(u32 foe_entry_idx, int ring_idx)
{
	struct airoha_foe_entry *hwe = NULL;
    int ptype = 0, cur_state = 0, cur_nbq = 0;

	hwe = airoha_ppe_foe_get_entry_locked(glb_eth->ppe, foe_entry_idx);;
	if(hwe == NULL){
		return -1;
	}

    ptype = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, hwe->ib1);
    cur_state = FIELD_GET(AIROHA_FOE_IB1_BIND_STATE, hwe->ib1);

    if (PPE_PKT_TYPE_IPV4_HNAPT == ptype){
        cur_nbq = FIELD_GET(AIROHA_FOE_IB2_NBQ, hwe->ipv4.ib2);
    }else if (PPE_PKT_TYPE_IPV6_ROUTE_5T == ptype){
        cur_nbq = FIELD_GET(AIROHA_FOE_IB2_NBQ, hwe->ipv6.ib2);
    }else{
        return -1;
    }
    
    if((cur_state == AIROHA_FOE_STATE_INVALID) || (cur_nbq != (ring_idx <= LRO_RING_END ? ring_idx : ring_idx - LRO_RING_NUM)))
        return 1;
    else
        return 0;
}

int airoha_get_free_lro_ring(u32 foe_entry_idx)
{
	int i = 0;

	/* check if the entry has been binded. */
	for(i = 0; i < 2 * LRO_RING_NUM; i++)
	{
		if(lro_ring_reserve[i][0] == LRO_RING_RESERVED)
		{
			if(ppe_entry_is_valid(lro_ring_reserve[i][1], (LRO_RING_START + i)))     /* current ppe entry has aged out. */
			{
				lro_ring_reserve[i][1] = foe_entry_idx;
				return (LRO_RING_START + i);
			}
		}

		if((lro_ring_reserve[i][0] == LRO_RING_RESERVED) && (lro_ring_reserve[i][1] == foe_entry_idx))
		{
			return (LRO_RING_START + i);
		}
	}

	/* scan for free Rx ring. */
	for(i = 0; i < 2 * LRO_RING_NUM; i++)
	{
		if(lro_ring_reserve[i][0] == LRO_RING_FREE)
		{
			lro_ring_reserve[i][1] = foe_entry_idx;
			lro_ring_reserve[i][0] = LRO_RING_RESERVED;
			return (LRO_RING_START + i);
		}
	}

	return 1;
}

static int arht_general_offload_bind_ring(unsigned short type, u32 foe_entry_idx)
{
	int ring = 1;
	
	switch (type){
		case PPE_UDF_LOCAL_IN:
			ring = airoha_get_free_lro_ring(foe_entry_idx);
			break;

		default:
			break;
	}

	return ring;
}

/******************************************************************************
 Descriptor: This is for hwnat binding, in case of no NAT/VLAN/pppoe opteration.
 Input Args:	
 Ret Value: 
******************************************************************************/
int arht_general_offload_bind(struct sk_buff * skb, unsigned short type)
{
	int ret = 0, ptype = 0;
	u32 foe_entry_idx= 0;
	struct airoha_foe_entry *hwe;
	u32 data;
	int ring;

	foe_entry_idx = skb->hash;
	if (foe_entry_idx >= PPE_SRAM_NUM_ENTRIES || !skb->l4_hash || skb->sw_hash != false)
		return ret;
	
	spin_lock_bh(&ppe_lock);
	hwe = airoha_ppe_foe_get_entry_locked(glb_eth->ppe, foe_entry_idx);
	if (!hwe)
		goto unlock;

	if(FIELD_GET(AIROHA_FOE_IB1_BIND_STATE, hwe->ib1) == AIROHA_FOE_STATE_BIND)
		goto unlock;

	ptype = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, hwe->ib1);
	data = FIELD_PREP(AIROHA_FOE_SHAPER_ID, 0x7F);

	ring = arht_general_offload_bind_ring(type, foe_entry_idx);
	switch (ptype) {
		case PPE_PKT_TYPE_IPV4_HNAPT:
			hwe->ipv4.new_tuple.src_port = hwe->ipv4.orig_tuple.src_port;
			hwe->ipv4.new_tuple.dest_port = hwe->ipv4.orig_tuple.dest_port;
			hwe->ipv4.data = data;
			hwe->ipv4.ib2 = 0;
			if(ring <= LRO_RING_END){
			    hwe->ipv4.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, ring);	
			    hwe->ipv4.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_CDM1);
			}else{
			    hwe->ipv4.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, ring - LRO_RING_NUM);
			    hwe->ipv4.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_CDM2);
			}
			hwe->ipv4.new_tuple.src_ip = hwe->ipv4.orig_tuple.src_ip;
			hwe->ipv4.new_tuple.dest_ip = hwe->ipv4.orig_tuple.dest_ip;
			hwe->ipv4.l2.common.etype = type;
			break;
		
		case PPE_PKT_TYPE_IPV6_ROUTE_5T:
			hwe->ipv6.data = data;
			hwe->ipv6.ib2 = 0;	
			if(ring <= LRO_RING_END){
			    hwe->ipv6.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, ring);	
			    hwe->ipv6.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_CDM1);
			}else{
			    hwe->ipv6.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_NBQ, ring - LRO_RING_NUM);	
			    hwe->ipv6.ib2 |= FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, FE_PSE_PORT_CDM2);
			}
			hwe->ipv6.l2.etype = type;
			break;

		default:
			break;
	}

	if(skb_mac_header_was_set(skb)){
		unsigned char *mac_h = skb_mac_header(skb);
		airoha_set_ppe_mac(hwe,skb->dev,mac_h+6,mac_h,0);
	}

	airoha_ppe_foe_commit_entry_ptr(glb_eth->ppe, hwe, foe_entry_idx, 0);

unlock:	
	spin_unlock_bh(&ppe_lock);

	return ret;
}
EXPORT_SYMBOL(arht_general_offload_bind);

struct dst_entry *arht_gen_dst_clone(struct dst_entry *dst)
{
	return dst_clone(dst);
}
EXPORT_SYMBOL(arht_gen_dst_clone);
