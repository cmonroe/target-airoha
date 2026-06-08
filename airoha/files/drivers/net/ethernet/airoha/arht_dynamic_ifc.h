/*********************************************************************************
 * decleration and function prototype for dynamic ifc
 *
 * Copyright (C) 2025 Econet Technologies, Corp.
 * All Rights Reserved.
 *
 *********************************************************************************/
#ifndef ARHT_DYNAMIC_IFC_H_
#define	ARHT_DYNAMIC_IFC_H_

#include <arht_hook/ecnt_hook_ifc.h>
#include <arht_hook/ecnt_hook_ifc_type.h>

#define IFC_LRO_RING_START      12
#define IFC_LRO_RING_NUM        4
#define IFC_DEFAULT_RING	1
#define IFC_RING_SLOT_FREE  (-1)

#define SK_MARK_LOCAL_OFFLOAD	0xAE000001

#define LRO_AGG_NUM                22
#define PHY_REG_CDM1_LRO_LIMIT     0x1fb50484
#define PHY_REG_CDM2_LRO_LIMIT     0x1fb51484
#define CDM_LRO_AGG_NUM_MASK_VAL   0x00FF0000
#define CDM_LRO_AGG_NUM_SHIFT      16

#define PPE_CPU_REASON_BIT		27
#define PPE_CPU_MASK			(0x1F << PPE_CPU_REASON_BIT)

#define MAX_DYNAMIC_IFC_NUM	32
#define MAX_DYNAMIC_IFC_APP_NUM	16

#define EXPIRE_TIME		(1*HZ)

#define REG_FE_LAN_MAC_H		0x1fb50040
#define REG_FE_MAC_LMIN(_n)		((_n) + 0x04)

#define FOE_ENTRY_NUM(skb)		(skb_get_hash(skb) & 0xFFFF)

typedef struct{
	struct dst_entry *rx_dst;
	struct dst_entry *tx_dst;
	struct timer_list age_timer;
	struct sock *sk;
	unsigned short lport;
	unsigned short rport;
	unsigned short hash;
	unsigned long last_tx;
	unsigned int valid:1;
	unsigned int skip_copy:1;
	unsigned int ifc_valid:1;
	unsigned int resv:29;
	unsigned int ifc_index;
	int ring_id;
}dynamic_ifc;

#endif


