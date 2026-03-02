// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024 AIROHA Inc
 */
#ifndef AIROHA_DP_API_H
#define AIROHA_DP_API_H

#include <linux/etherdevice.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/of_net.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/tcp.h>
#include <linux/u64_stats_sync.h>
#include <net/dsa.h>
#include <net/page_pool/helpers.h>
#include <uapi/linux/ppp_defs.h>
#include <linux/types.h>
#include <net/ipv6.h>
#include <linux/if_pppox.h>
#include <net/pkt_cls.h>
#include <linux/ip.h>

#include "arht_hook/arht_hook.h"
#include "arht_hook/ecnt_hook_qdma_type.h"
#include "arht_hook/ecnt_hook_fe_type.h"

#define TX_WRED_THR_NUM                         (5)
#define TX_WRED_PROBABILITY_NUM                 (4)
#define MAX_NAME_LENGTH             (16)

#define PER_CHNL_TICKSEL_NUM (2)


#define REG_QDMA_GLOBAL_CFG			0x0004
#define REG_QDMA_TXQ_TOTAL_FAST_THR			0x10d4
#define TXQ_CNGST_TXQ_TOTAL_MAX_THR_SHIFT			(16)
#define TXQ_CNGST_TXQ_TOTAL_MAX_THR_MASK			(0xFFFF<<TXQ_CNGST_TXQ_TOTAL_MAX_THR_SHIFT)
#define TXQ_CNGST_TXQ_TOTAL_MIN_THR_SHIFT			(0)
#define TXQ_CNGST_TXQ_TOTAL_MIN_THR_MASK			(0xFFFF<<TXQ_CNGST_TXQ_TOTAL_MIN_THR_SHIFT)

//fast totalmin equal to 8/10share_free
#define BUFF_FAST_TOTAL_MAX_THRH 0x2286
#define BUFF_FAST_TOTAL_MIN_THRH 0x2286

#define AIROHA_MAX_IDX_FOR_GEMPORT_RATELIMIT 31
#define AIROHA_MIN_IDX_FOR_GEMPORT_RATELIMIT 0
#define AIROHA_SKB_MARK_MASK_FOR_GEMPORT_RATELIMIT  GENMASK(31, 26)
#define AIROHA_SKB_MARK_SHIFT_FOR_GEMPORT_RATELIMIT  26


#if !defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_QDMA_UMAC_LOOPBACK 1
#else
#define SUPPORT_QDMA_UMAC_LOOPBACK 0
#endif

#define GLB_CFG_QDMA_LOOPBACK						(1<<16)
#define GLB_CFG_UMAC_LOOPBACK						(1<<17)

/*NO IMEM for EN7516/EN7580*/
#undef __IMEM
#define __IMEM

#define TXQ_DIS_CFG_REG_NUM				8

#define TXQ_DIS_QUEUE_CLOSE_OFFSET(chnl)                    ((chnl)&0xFC)
#define TXQ_DIS_CHANNEL_SHIFT(chnl)                         (((chnl)&0x03)*8)
#define TXQ_DIS_CHANNEL_MASK(chnl)                          (0xFF<<TXQ_DIS_CHANNEL_SHIFT(chnl))

#if defined(QDMA_LAN)
#define qdma_path "qdma_lan"
#else
#define qdma_path "qdma_wan"
#endif

#define CONFIG_DEBUG 1
#ifdef CONFIG_DEBUG
	#define QDMA_MSG(level, F, B...)	{ \
											if(gpQdmaPriv->devCfg.dbgLevel >= level) 	\
												printk("%s: %s [%d]: " F, qdma_path, strrchr(__FILE__, '/')+1, __LINE__, ##B) ; \
										}
	#define QDMA_ERR(F, B...)			printk("%s: %s [%d]: " F, qdma_path, strrchr(__FILE__, '/')+1, __LINE__, ##B)
	#define QDMA_LOG(F, B...)			printk("%s: %s [%d]: " F, qdma_path, strrchr(__FILE__, '/')+1, __LINE__, ##B)
#else
	#define QDMA_MSG(level, F, B...)	
	#define QDMA_ERR(F,B...)			
	#define QDMA_LOG(F,B...)			printk("%s: " F, qdma_path, ##B)
#endif

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_CHANNEL_CLOSE_STATUS 1
#else
#define SUPPORT_CHANNEL_CLOSE_STATUS 0
#endif

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_INT_ENABLE_NUM_2 1
#else
#define SUPPORT_INT_ENABLE_NUM_2 0
#endif

/*after 7528,wrr weight scale change to 1Byte from 16Byte*/
#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7528)
#define SUPPORT_WRR_WEIGHT_SCALE_1Byte 1
#else
#define SUPPORT_WRR_WEIGHT_SCALE_1Byte 0
#endif

#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7528)
#define SUPPORT_QUEUE_DISABLE 1
#else
#define SUPPORT_QUEUE_DISABLE 0
#endif

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_OAM_MODIFY_FP 1
#else
#define SUPPORT_OAM_MODIFY_FP 0
#endif

#if defined(TCSUPPORT_CPU_EN7517)
#define DBG_CNTR_FWD_CHNL_GROUP_NUM 		(12)
#else
#define DBG_CNTR_FWD_CHNL_GROUP_NUM 		(16)
#endif

/*default setting for INT1~4 bind TX&RX Ring info*/
#if defined(QDMA_LAN)
/*INT1*/
#define INT1_RX_RING_MASK					        (0x8187)        /*Rx_Ring_0/1/2/7/8/15 binded to INT1*/
#define INT1_TX_RING_MASK					        (0x3)           /*Tx_Ring_0~1 binded to INT1*/
/*INT2*/
#define INT2_RX_RING_MASK					        (0x418)          /*Rx_Ring_3/4/10 for reserve*/
#define INT2_TX_RING_MASK					        (0x0)
/*INT3*/
#define INT3_RX_RING_MASK					        (0x20)          /*Rx_Ring_5 for WIFI-2.4G,Ring_9 for left to right*/
#define INT3_TX_RING_MASK					        (0x0)
/*INT4*/
#define INT4_RX_RING_MASK					        (0x40)          /*Rx_Ring_6 for WIFI-5G*/
#define INT4_TX_RING_MASK					        (0x0)
#else
#if  defined(TCSUPPORT_CPU_EN7517)
/*INT1 for wan*/
#define INT1_RX_RING_MASK					        (0xff00)        /*Rx_Ring_8-15 binded to INT1*/
#define INT1_TX_RING_MASK					        (0x3)           /*Tx_Ring_0~1 binded to INT1*/
/*INT2 for lan*/
#define INT2_RX_RING_MASK					        (0x00ff)         /*Rx_Ring_0-7 binded to INT2*/
#define INT2_TX_RING_MASK					        (0x0)
/*INT3*/
#define INT3_RX_RING_MASK					        (0x0)
#define INT3_TX_RING_MASK					        (0x0)
/*INT4*/
#define INT4_RX_RING_MASK					        (0x0)
#define INT4_TX_RING_MASK					        (0x0)

#else
/*INT1*/
#define INT1_RX_RING_MASK					        (0x8187)        /*Rx_Ring_0/1/2/7/8/15 binded to INT1*/
#define INT1_TX_RING_MASK					        (0x3)           /*Tx_Ring_0~1 binded to INT1*/
/*INT2*/
#define INT2_RX_RING_MASK					        (0x478)          /*Rx_Ring_3/4/5/6 for reserve*/
#define INT2_TX_RING_MASK					        (0x0)
/*INT3*/
#define INT3_RX_RING_MASK					        (0x0)
#define INT3_TX_RING_MASK					        (0x0)
/*INT4*/
#define INT4_RX_RING_MASK					        (0x0)
#define INT4_TX_RING_MASK					        (0x0)
#endif
#endif




#define INT1_MASK_RX_DONE_LOW_16_RING				(int1_rx_ring&0x0000FFFF)
#define INT1_MASK_RX_DONE_HIGH_16_RING				(int1_rx_ring>>16)
#define INT1_MASK_RX_DONE							(int1_rx_ring)
#define INT1_MASK_RX0_DONE							((int1_rx_ring&0x1)<<1)
#define INT1_MASK_RX1_DONE							((int1_rx_ring&0x2)<<4)
#define INT1_MASK_RX_DONE_0_1						(INT1_MASK_RX0_DONE|INT1_MASK_RX1_DONE)
#define INT1_MASK_RX_DONE_2_15						((int1_rx_ring&0xFFFC)>>2)

static inline uint __get_csr_int_enable_register(int intNo, int enNo)
{
	if((enNo) <= 2)
	{
		return (0x0024 + ((intNo-1)<<3) + (enNo<<2));
	}
	else if((enNo) == 3)
	{
		return (0x0740 + ((intNo-1)<<5));
	}
	else if((enNo) == 4)
	{
		return (0x0744 + ((intNo-1)<<5));
	}
	else
	{
		return (0x0750 + ((intNo-1)<<5));
	}
	return 0;
}

#define QDMA_CSR_INT_EN(intNo,enNo)			__get_csr_int_enable_register(intNo,enNo)

#define INT_MASK_XPON_PHY							(1<<24)
#define INT_MASK_EPON_MAC							(1<<17)
#define INT_MASK_GPON_MAC							(1<<16)



#if defined(TCSUPPORT_CPU_EN7581)
#define TX_RING_NUM		(32)
#elif defined(TCSUPPORT_CPU_EN7580)
#define TX_RING_NUM		(8)
#else /*TCSUPPORT_CPU_EN7527 || TCSUPPORT_CPU_EN7516 and before */
#define TX_RING_NUM		(2)
#endif

#if defined(TCSUPPORT_CPU_EN7581)
#define RX_RING_NUM		(32)
#elif defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
#define RX_RING_NUM		(16)
#else
#define RX_RING_NUM		(2)
#endif

#define QDMA_DSCP_MSG_WORD_LENS			4

/***************************************
 fe resource manage variables
***************************************/
#define METER_MAX_NUM       127
#define NAME_LENGTH         16
#define METER_GROUP_NUM     4

#define CONFIG_FLOWCNT_MAX_DROUP_NUM            (3)
#define CONFIG_FLOWCNT_GRP0_MAX_IDX_NUM         (63)	/* ACNT_GRP[5:0] */
#define CONFIG_FLOWCNT_GRP1_MAX_IDX_NUM         (31)	/* ACNT_GRP[10:6] */
#define CONFIG_FLOWCNT_GRP2_MAX_IDX_NUM         (127)	/* MTR_GRP[6:0] */

#define CONFIG_QDMA_QUEUE                   8

#ifndef UINT32
#define UINT32
typedef uint32_t uint32; 		/* 32-bit unsigned integer      */
#endif

typedef  unsigned long long uint64; 



typedef struct {
	struct {
		unchar		isTxPolling			:1 ;
		unchar		isRxPolling			:1 ;
		unchar		isRxNapi			:1 ;
		unchar		isIsrRequest		:1 ;
		unchar 		resv1				:4 ;
	} flags ;
	unchar		txRecycleThreshold ;
	unchar		macType;
#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
	unchar		txQueueTrtcmScale ;
	unchar		gponTrtcmScale ;
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
#ifdef CONFIG_DEBUG
	unchar		dbgLevel ;
#endif /* CONFIG_DEBUG */
#ifdef CONFIG_SUPPORT_SELF_TEST
	unchar		txDscpDoneBit ;
	unchar		rxDscpDoneBit ;
	ushort		waitTxMaxNums ;
	ushort		countDown ;
	ushort		countDownRound ;
	unchar		rxMsgLens ;
#endif /* CONFIG_SUPPORT_SELF_TEST */	
	int (*bmRecvCallbackFunction)(void *, uint, struct sk_buff *, uint) ;
#if defined(TCSUPPORT_FTTDP_V2) && defined(QDMA_LAN)
    int (*bmRecvCallbackPtmLanFunction)(void *, uint, struct sk_buff *, uint) ;
#endif
	int (*bmEventCallbackFunction)(QDMA_EventType_t) ;
    int (*bmRecyPktCallbackFunction)(struct sk_buff *, uint) ;
	void (*bmGponMacIntHandler)(void) ;
	void (*bmEponMacIntHandler)(void) ;
	void (*bmXponPhyIntHandler)(void) ;
    /*multi-INTs extend*/
    int (*bmInt2RecvCallbackFunction)(void *, uint, struct sk_buff *, uint, QDMA_RxInfo_T *) ;
    int (*bmInt2EventCallbackFunction)(QDMA_EventType_t) ;
    int (*bmInt3RecvCallbackFunction)(void *, uint, struct sk_buff *, uint, QDMA_RxInfo_T *) ;
    int (*bmInt3EventCallbackFunction)(QDMA_EventType_t) ;
    int (*bmInt4RecvCallbackFunction)(void *, uint, struct sk_buff *, uint, QDMA_RxInfo_T *) ;
    int (*bmInt4EventCallbackFunction)(QDMA_EventType_t) ;
} BM_DevConfig_T ;

typedef struct {
	uint	intIrqcnt ;
	ushort 	IrqQueueAsynchronous ;
	ushort 	txIrqQueueIdxErrs ;
	uint	rxAllocFailErrs ;
	ushort	intIrqFull ;
	ushort	IrqErr ;
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
	uint	intIrq2cnt ;
	ushort 	Irq2QueueAsynchronous ;
	ushort 	txIrq2QueueIdxErrs ;
	ushort	intIrq2Full ;
	ushort	Irq2Err ;
#endif
	ushort	intNoLinkDscp ;
	ushort	intLowLinkDscp ;
	ushort	txPktsFreeCounts ;
	ushort	txPktsFreeErrs ;
	ushort	txMacLimitDropCnt ;
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_FORWARD_LEFT_TO_RIGHT)
	ushort  txForceRing0Cnt;
#endif/*TCSUPPORT_COMPILE*/
	ushort	apiIdxErrs;
	uint	txCounts[TX_RING_NUM] ;
	uint	txRecycleCounts[TX_RING_NUM] ;
	uint 	rxCounts[RX_RING_NUM] ;
	ushort 	txDscpIncorrect[TX_RING_NUM] ;
	ushort 	txDscpDoneErrs[TX_RING_NUM] ;
#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7528)
	ushort	txDscpDropErrs[TX_RING_NUM] ;
#endif
	ushort 	rxDscpIncorrect[RX_RING_NUM] ;
	ushort	rxPktErrs[RX_RING_NUM] ;
	ushort	noTxDscps[TX_RING_NUM] ;
	ushort 	noRxCbErrs[RX_RING_NUM] ;
	ushort	noRxDscps[RX_RING_NUM] ;
	ushort	rxAllocFailDropErrs[RX_RING_NUM] ;
	ushort	intRxCoherent[RX_RING_NUM] ;
	ushort	intTxCoherent[TX_RING_NUM] ;
	ushort	intNoRxBuff[RX_RING_NUM] ;
	ushort	intNoTxBuff[TX_RING_NUM] ;
	ushort	intNoRxDscp[RX_RING_NUM] ;
	ushort	intNoTxDscp[TX_RING_NUM] ;
	ushort 	rxBroadcastCounts[RX_RING_NUM] ;
	ushort 	rxUnknownUnicastCounts[RX_RING_NUM] ;
	ushort  rxMulticastCounts[RX_RING_NUM] ;
	uint	intRxDone[RX_RING_NUM] ;
	uint	intTxDone[TX_RING_NUM] ;
	uint 	intRxDscpLow[RX_RING_NUM] ;
	uint 	car_queue[CONFIG_QDMA_QUEUE] ;
	uint 	car_queue_drop[CONFIG_QDMA_QUEUE] ;
} BM_Counters_T ;

typedef struct {
    uint32                      MeterUseState[METER_GROUP_NUM] ;   /*global meter ctrl variable, bit map data*/
    ushort                      MaxMeterNum ;                            /*trtcm mode:63 ;ratelimit mode 127 */
    char                        MeterUseFunc[METER_MAX_NUM][NAME_LENGTH] ;        /* name of meter bind function*/
    char                        MeterGroupNum ;
}QDMA_Meter_Manage_T ;

typedef struct {
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
	uint64                      Acnt0UseState;
#else
    uint32                      Acnt0UseState;   /*global meter ctrl variable, bit map data*/
#endif
    char                        Acnt0UseFunc[CONFIG_FLOWCNT_GRP0_MAX_IDX_NUM][NAME_LENGTH] ;        /* name of acnt0 bind function*/
}QDMA_ACNT0_Manage_T ;

typedef struct {
    uint32                      Acnt1UseState;   /*global meter ctrl variable, bit map data*/
    char                        Acnt1UseFunc[CONFIG_FLOWCNT_GRP1_MAX_IDX_NUM][NAME_LENGTH] ;        /* name of acnt1 bind function*/
}QDMA_ACNT1_Manage_T ;

typedef struct {
    uint64                      Acnt2UseState0;   /*global meter ctrl variable, bit map data for account0~63*/
    uint64                      Acnt2UseState1;   /*global meter ctrl variable, bit map data for account64~127*/
    char                        Acnt2UseFunc[CONFIG_FLOWCNT_GRP2_MAX_IDX_NUM][NAME_LENGTH] ;        /* name of acnt2 bind function*/
}QDMA_ACNT2_Manage_T ;



typedef struct {
	struct cdev *pQdmaDev ;
} QDMA_Dev_T ;

typedef struct {
	uint 	resv1 ;
	struct {
#ifdef __BIG_ENDIAN
		uint done				: 1 ;
		uint drop_pkt			: 1 ;	/*drop_pkt for TX_Dscp, overflow for RX_Dscp*/
		uint nls				: 1 ;	/*not last packet, for scatter*/
		uint resv2				: 3 ;
		uint dei                : 1 ;
		uint no_drop            : 1 ;
		uint resv				: 8 ;
		uint pkt_len			: 16 ;
#else
		uint pkt_len			: 16 ;
		uint resv				: 8 ;
		uint no_drop            : 1 ;
		uint dei                : 1 ;
		uint resv2				: 3 ;
		uint nls				: 1 ;	/*not last packet, for scatter*/
		uint drop_pkt			: 1 ;	/*drop_pkt for TX_Dscp, overflow for RX_Dscp*/
		uint done				: 1 ;
#endif /* __BIG_ENDIAN */
	} ctrl ;
	uint 	pkt_addr ;
#ifdef __BIG_ENDIAN
	uint resv3					: 16 ;
	uint next_idx				: 16 ;
#else
	uint next_idx				: 16 ;
	uint resv3					: 16 ;
#endif /* __BIG_ENDIAN */
	uint msg[QDMA_DSCP_MSG_WORD_LENS] ;
} QDMA_DMA_DSCP_T ;

struct QDMA_DscpInfo_S {
	QDMA_DMA_DSCP_T				*dscpPtr ;
	uint						dscpIdx ;
#if 0
	void						*msgPtr ;
#ifdef CONFIG_SUPPORT_SELF_TEST
	void						*testMsgPtr ;
#endif
#endif
	struct sk_buff				*skb ;
	struct QDMA_DscpInfo_S		*next ;
} ;


typedef struct {
	uint						csrBaseAddr ;
	spinlock_t					txLock[TX_RING_NUM] ;       /* spin lock for Tx */
	spinlock_t					irqLock ;                   /* spin lock for IRQ */
	spinlock_t					txDoneLock ;                /* spin lock for tx transmit done */
#if defined(TCSUPPORT_CPU_EN7581)
	uint						txDscpNum[TX_RING_NUM] ;	/* Total TX DSCP number */
	uint						rxDscpNum[RX_RING_NUM] ;	/* Total RX DSCP number */
#else
	ushort						txDscpNum[TX_RING_NUM] ;    /* Total TX DSCP number */
	ushort						rxDscpNum[RX_RING_NUM] ;    /* Total RX DSCP number */
#endif
	uint						hwFwdDscpNum ;              /*EN7580: support 64K dscp at most*/
	uint						hwFwdDscpMaxNum ;
	ushort						irqDepth ;                  /* Max depth for IRQ queue */
	ushort						irq2Depth ;                 /* Max depth for IRQ2 queue */
	ushort						hwPktSize ;
#ifdef TCSUPPORT_CPU_ARMV8
	unsigned long				dscpInfoAddr ;              /* Start pointer for DSCP information node */
	unsigned long				txBaseAddr[TX_RING_NUM] ;
	unsigned long				rxBaseAddr[RX_RING_NUM] ;
	unsigned long				irqQueueAddr ;              /* IRQ queue address */
	unsigned long				irq2QueueAddr ;             /* IRQ2 queue address */
	unsigned long				hwFwdBaseAddr ;             /* Base address of the hardware forwarding */
	unsigned long				hwFwdBuffAddr ;             /* Base address of the hardware forwarding Buffer*/
	unsigned long				hwFwdPayloadSize ;          /* Payload size of the hardware forwarding Buffer*/
#else
	uint						dscpInfoAddr ;              /* Start pointer for DSCP information node */
	uint						txBaseAddr[TX_RING_NUM] ;
	uint						rxBaseAddr[RX_RING_NUM] ;
	uint						irqQueueAddr ;              /* IRQ queue address */
	uint						irq2QueueAddr ;             /* IRQ2 queue address */
	uint						hwFwdBaseAddr ;             /* Base address of the hardware forwarding */
	uint						hwFwdBuffAddr ;             /* Base address of the hardware forwarding Buffer*/
	uint						hwFwdPayloadSize ;          /* Payload size of the hardware forwarding Buffer*/
#endif
	struct QDMA_DscpInfo_S		*txHeadPtr[TX_RING_NUM] ;   /* Head node for unused tx desc. */
	struct QDMA_DscpInfo_S		*txTailPtr[TX_RING_NUM] ;   /* Tail node for unused tx desc. */
	struct QDMA_DscpInfo_S		*txUsingPtr[TX_RING_NUM] ;  /* TXDMA using DSCP node. */
	struct QDMA_DscpInfo_S		*rxStartPtr[RX_RING_NUM] ;  /* Start using node for rx desc. */
	struct QDMA_DscpInfo_S		*rxEndPtr[RX_RING_NUM] ;    /* End using node for rx desc. */
	struct QDMA_DscpInfo_S		*rxUsingPtr[RX_RING_NUM] ;  /* RXDMA using DSCP node. */
	struct tasklet_struct 		task ;
	BM_DevConfig_T				devCfg ;
	BM_Counters_T				counters ;
	struct timer_list			fwdCfg_timer ;
#ifdef TCSUPPORT_DRIVER_API
	QDMA_Dev_T 					qdmaDev ;
#endif
	QDMA_DbgCntrChnlGroup_t		dbgCntrType;
    QDMA_Meter_Manage_T         meterMgr ;
    QDMA_Meter_Manage_T         meter1Mgr ;
    QDMA_Meter_Manage_T         meter2Mgr ;
    QDMA_ACNT0_Manage_T         acnt0Mgr ;
    QDMA_ACNT1_Manage_T         acnt1Mgr ;
    QDMA_ACNT2_Manage_T         acnt2Mgr ;
} QDMA_Private_T ;



/*
struct port_info {
    unsigned long int tsid:8;
    unsigned long int channel:5;
    unsigned long int nbq:5;
    unsigned long int fast:1;
    unsigned long int txq:4;
    unsigned long int atm_pppoa:1;
    unsigned long int atm_ipoa:1;
    unsigned long int atm_vc_mux:1;
    unsigned long int eth_macSTagEn:1;
	unsigned long int eth_is_wan:1;
    unsigned long int ds_to_qdma:1;
    unsigned long int ds_need_offload:1;
    unsigned long int force_high_priority_ring:1;
	unsigned long int txq_is_valid:1;
    unsigned long int stag:16;
    unsigned long int magic:16;
	unsigned long int udf:8;//add for inode wifi 
};
*/


typedef union
{
	struct {
#ifdef __BIG_ENDIAN
		uint32 				:1;
		uint32 no_mic		:1;
		uint32 sp_tag		:16;
		uint32 longf 		:1;
		uint32 runtf		:1;
		uint32 crcer		:1;
		uint32 soe			:1;
		uint32 ptp			:1;
		uint32 oam			:1;
		uint32 channel		:5;
		uint32 hop_flags	:3;
#else
		uint32 hop_flags	:3;
		uint32 channel		:5;
		uint32 oam			:1;
		uint32 ptp			:1;
		uint32 soe			:1;
		uint32 crcer		:1;
		uint32 runtf		:1;
		uint32 longf 		:1;
		uint32 sp_tag		:16;
		uint32 no_mic		:1;
		uint32 				:1;
#endif
	}raw;
    uint32 word;
} rxMsgWord0_t;

typedef union
{
    struct {
#ifdef __BIG_ENDIAN
        uint32 dei			:1;
        uint32 ip6			:1;
        uint32 ip4			:1;
        uint32 ip4f         :1;
        uint32 l4vld        :1;
        uint32 l4f          :1;
        uint32 sport        :5;
        uint32 crsn         :5;
        uint32 ppe_entry    :16;
#else
        uint32 ppe_entry    :16;
        uint32 crsn         :5;
        uint32 sport        :5;
        uint32 l4f          :1;
        uint32 l4vld        :1;
        uint32 ip4f         :1;
        uint32 ip4			:1;
        uint32 ip6			:1;
        uint32 dei			:1;
#endif
    }raw;
    uint32 word;
} rxMsgWord1_t;

typedef union
{
	struct {
#ifdef __BIG_ENDIAN
		uint32 sw_udf		:8;
		uint32				:7;
		uint32 ifc_id		:9;
		uint32 ifc_hit		:1;
		uint32 l2_len		:7;
#else
		uint32 l2_len		:7;
		uint32 ifc_hit		:1;
		uint32 ifc_id		:9;
		uint32				:7;
		uint32 sw_udf		:8;
#endif
    }raw;
    uint32 word;
} rxMsgWord2_t;

typedef union
{
	uint32  resv;
    uint32  word;
} rxMsgWord3_t;

typedef struct rxMsgWord_s
{
    rxMsgWord0_t rxMsgW0;
    rxMsgWord1_t rxMsgW1;
    rxMsgWord2_t rxMsgW2;
    rxMsgWord3_t rxMsgW3;
}rxMsgWord_t;

typedef int (*qdma_api_op_t)(struct ECNT_QDMA_Data *qdma_data);

#define AIROHA_TSO_BIT		26
#define AIROHA_PPE_CPU_REASON_BIT		27
//#define AIROHA_PPE_CPU_REASON			(1<<AIROHA_PPE_CPU_REASON_BIT)
#define AIROHA_PPE_CPU_MASK				(0x1F<<AIROHA_PPE_CPU_REASON_BIT)
#define AIROHA_PPE_ENTRY_MASK			0xFFFF

#define VLAN_HLEN  4
#define ETH_ALEN   6
#define ETH_HLEN   14
#define PPE_CLEAR_OFFSET1 16
#define PPE_CLEAR_OFFSET2 40
#define PPE_CLEAR_OFFSET3 40
#define PPE_CLEAR_OFFSET4 24
#define SIZE_OF_FOE_ENTRY 80

#define SMACT_SIZE	(0x800)
#define UPDMEM_NUM	(16)
#define UPDMEM_SMAC_CNT		(6)
#define UPDMEM_IPV4_LINE	(2)
#define UPDMEM_IPV6_LINE	(8)

/* PPE_UPDMEM_CTRL */
#define PPE_UPDMEM_ACK						(1<<31)
#define PPE_UPDMEM_ADDR_SHIFT				(8)
#define PPE_UPDMEM_OFST_SHIFT				(4)
#define PPE_UPDMEM_OFST_MASK				(0xF<<PPE_UPDMEM_OFST_SHIFT)
#define PPE_UPDMEM_SEL_SMAC			(0)
#define PPE_UPDMEM_SEL_IPv6			(1)
#define PPE_UPDMEM_SEL_IPv4			(2)
#define PPE_UPDMEM_SEL_SHIFT				(2)
#define PPE_UPDMEM_CTRL_READ		(0)
#define PPE_UPDMEM_CTRL_WRITE		(1)
#define PPE_UPDMEM_WR						(1<<1)
#define PPE_UPDMEM_REQ						(1<<0)

#define FOE_INFO_LEN		    7

#define IS_IPV4_HNAPT(x)	((FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, (x)->ib1) == PPE_PKT_TYPE_IPV4_HNAPT) ? 1: 0)
#define IS_IPV4_HNAT(x)		((FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, (x)->ib1) == PPE_PKT_TYPE_IPV4_ROUTE) ? 1 : 0)
#define IS_L2_RRIDGE(x)	((FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, (x)->ib1) == PPE_PKT_TYPE_BRIDGE) ? 1 : 0)
#define IS_IPV4_DSLITE(x)	((FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, (x)->ib1) == PPE_PKT_TYPE_IPV4_DSLITE) ? 1 : 0)
#define IS_IPV6_3T_ROUTE(x)	((FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, (x)->ib1) == PPE_PKT_TYPE_IPV6_ROUTE_3T) ? 1 : 0)
#define IS_IPV6_5T_ROUTE(x)	((FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, (x)->ib1) == PPE_PKT_TYPE_IPV6_ROUTE_5T) ? 1 : 0)
#define IS_IPV6_6RD(x)		((FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, (x)->ib1) == PPE_PKT_TYPE_IPV6_6RD) ? 1: 0)
#define IS_IPV4_GRP(x)		(IS_IPV4_HNAPT(x) | IS_IPV4_HNAT(x) | IS_IPV4_DSLITE(x))
#define IS_IPV6_GRP(x)		(IS_IPV6_3T_ROUTE(x) | IS_IPV6_5T_ROUTE(x) | IS_IPV6_6RD(x))


enum hwnat_status {
        HWNAT_SUCCESS = 0,
        HWNAT_FAIL = 1,
        HWNAT_ENTRY_NOT_FOUND = 2
};

struct hwnat_shrink_field {
	//unsigned int smac[UPDMEM_SMAC_LINE];		// 0:smac[31~0]; 1:smac[48~32];
	unsigned char smac[UPDMEM_SMAC_CNT];		// 0:smac[31~0]; 1:smac[48~32];
	unsigned int eg_ipv4[UPDMEM_IPV4_LINE]; 	// 0:dipv4; 1:sipv4
	unsigned int eg_ipv6[UPDMEM_IPV6_LINE]; 	// 0:dipv6[31:0], ....., 7:sipv6[127:96]
};

struct hwnat_shrink_table {
	//unsigned int smac[UPDMEM_SMAC_LINE];		// 0:smac[31~0]; 1:smac[48~32];
	unsigned char smac[UPDMEM_SMAC_CNT];		// 0:smac[47:40], .... ,5:smac[7:0];
	unsigned int eg_ipv4[UPDMEM_IPV4_LINE]; 	// 0:dipv4; 1:sipv4
	unsigned int eg_ipv6[UPDMEM_IPV6_LINE]; 	// 0:dipv6[31:0], ....., 7:sipv6[127:96]
	unsigned int valid[3];
	unsigned long timestamp[3];
};


#define PPP_IP		0x21	/* Internet Protocol */
#define PPP_IPV6	0x57	/* Internet Protocol Version 6 */
enum FoeTblTcpUdp {
	TCP = 0,
	UDP = 1,
	ANY = 2
};

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

/******************************************************************************
 Descriptor:	It's used to init the QDMA software driver and hardware device.
 				This function must be called if the upper layer application wanna
 				use the QDMA to send/receive packets.
 Input Args:	The pointer of the QDMA_InitCfg_t
 Ret Value:		0: init successful otherwise failed.
******************************************************************************/
int qdma_init(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to deinit the QDMA software driver and hardware device.
 				This function must be called if the upper layer application wanna
 				transfer to another application.
 Input Args:	None
 Ret Value:		0: init successful otherwise failed.
******************************************************************************/
int qdma_deinit(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable the TXDMA, RXDMA and HWFWD mode
 Input Args:	arg1: TX DMA mode (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value: 	No
******************************************************************************/
int qdma_tx_dma_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable the TXDMA, RXDMA and HWFWD mode
 Input Args:	arg1: RX DMA mode (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value: 	No
******************************************************************************/
int qdma_rx_dma_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable the QDMA loopback mode
 Input Args:	arg1: loopback mode (QDMA_LOOPBACK_DISABLE/QDMA_LOOPBACK_QDMA/QDMA_LOOPBACK_UMAC)
 Ret Value: 	No
******************************************************************************/
int qdma_loopback_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to register the QDMA callback functions. The QDMA 
 				driver support several callback function type that is define 
 				in QDMA_CbType_t enum.
 Input Args:	arg1: callback function type that is define in QDMA_CbType_t enum.
 				arg2: the pointer of the callback function.
 Ret Value:		0: register successful otherwise failed.
******************************************************************************/
int qdma_register_callback_function(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Description:	It's used to unregister the QDMA callback functions.
 Input Args:	arg1: callback function type that is define in QDMA_CbType_t enum.
 Ret Value:		0: unregister successful otherwise failed.
******************************************************************************/
int qdma_unregister_callback_function(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_enable_rxpkt_int(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_disable_rxpkt_int(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_enable_rxpkt_int2(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_disable_rxpkt_int2(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_enable_rxpkt_int3(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_disable_rxpkt_int3(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_enable_rxpkt_int4(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_disable_rxpkt_int4(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_receive_packets(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_receive_packets_int2(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_receive_packets_int3(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_receive_packets_int4(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_transmit_packet(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_bm_transmit_packet_wifi_fast(struct ECNT_QDMA_Data *qdma_data);

/******************************************************************************
 Descriptor:	It's used to configure the TXQOS weight type and scale.
 Input Args:	arg1: setting the WRR weighting value is base on packet or byte
 					  (QDMA_TXQOS_WEIGHT_BY_PACKET/QDMA_TXQOS_WEIGHT_BY_BYTE)
 				arg2: setting the byte weighting scale(QDMA_TXQOS_WEIGHT_SCALE_64B
 				      /QDMA_TXQOS_WEIGHT_SCALE_16B). when the weigthing value 
 				      is base on packet, these argument is don't care.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_qos_weight(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the TXQOS weight type and scale.
 Input Args:	arg1: the pointer of the weight base value 
 				      (QDMA_TXQOS_WEIGHT_BY_PACKET/QDMA_TXQOS_WEIGHT_BY_BYTE)
 				arg2: the pointer of the weight scale value
 					  (QDMA_TXQOS_WEIGHT_SCALE_64B/QDMA_TXQOS_WEIGHT_SCALE_16B)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_qos_weight(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to configure the tx queue scheduler and queue weigth
 				for specific channel.
 Input Args:	The pointer of the tx qos scheduler struct. It includes:
 				- channel: specific the channel ID (0~15)
 				- qosType: The QoS type is define in QDMA_TxQosType_t enum.
 				- weight: The unit of WRR weight is packets.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_qos(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the tx queue scheduler and queue weigth
 				for specific channel.
 Input Args:	The pointer of the tx qos scheduler struct. It includes:
 				- channel: specific the channel ID (0~15)
 				- qosType: The QoS type is define in QDMA_TxQosType_t enum.
 				- weight: The unit of WRR weight is packets.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_qos(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set channel/queue limit threshold for mac driver.
 Input Args:	arg1: channel limit threshold
                     arg2: queue limit threshold
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_mac_limit_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get channel/queue limit threshold for mac driver.
 Input Args:	arg1: channel limit threshold
                     arg2: queue limit threshold
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_get_mac_limit_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get channel of all current DSCPs for mac driver.
 Input Args:	arg1: channel number
                     arg2: the id list of all channels
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_get_using_tx_dscp_channel(struct ECNT_QDMA_Data *qdma_data);

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
int qdma_set_txbuf_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the tx buffer threshold.
 Input Args:	The pointer of the tx buffer control struct. It includes:
 				- mode: Eanble/Disable tx buffer usage control
 				- chnThreshold: Per tx per channel block usage threshold.
 				- totalThreshold: Total tx block usage threshold.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_txbuf_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable QDMA pre-fetch function. Since the
                on-chip buffer resource is limited, to prevent from Head-Of-Line
                blocking issue, 'when' to retrieving packet from External DRAM 
                to on-chip buffer will be application dependent.
 Input Args:	arg1: Enable/disable TXDMA pre-fetch function (QDMA_ENABLE/QDMA_DISABLE)
 			arg2: Enable/disable TXDMA pre-fetch overdrag function (QDMA_ENABLE/QDMA_DISABLE)
 			arg3: set TXDMA pre-fetch overdrag count value
 Ret Value:	No return value
******************************************************************************/
int qdma_set_prefetch_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable or disable pktsize overhead function.
 Input Args:	arg1: mode means enable or disable.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_pktsize_overhead_en(struct ECNT_QDMA_Data *qdma_data) ;


/******************************************************************************
 Descriptor:	It's used to get pktsize overhead function is enable or disable.
 Input Args:	None
 Ret Value:	0: Disable, 1: Enable.
******************************************************************************/
int qdma_get_pktsize_overhead_en(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of pktsize overhead.
 Input Args:	arg1: pktsize overhead value
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_pktsize_overhead_value(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of pktsize overhead.
 Input Args:	None
 Ret Value:	pktsize overhead value.
******************************************************************************/
int qdma_get_pktsize_overhead_value(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of lmgr low threshold.
 Input Args:	arg1: lmgr low threshold value
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_lmgr_low_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of lmgr low threshold.
 Input Args:	None
 Ret Value:	lmgr low threshold value.
******************************************************************************/
int qdma_get_lmgr_low_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get lmgr status: free lmgr dscp, used lmgr dscp, 
 			and used total buffer usage.
 Input Args:	arg1: to get free lmgr dscp counter
 			arg2: to get used lmgr dscp counter
 			arg3: to get used total buffer usage
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_get_lmgr_status(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to clear all the qdma cpu counters.
 Input Args:	None
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_clear_cpu_counters(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump all the qdma cpu counters.
 Input Args:	None
 Ret Value:	No return value
******************************************************************************/
int qdma_dump_cpu_counters(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump all qdma related register values.
 Input Args:	None
 Ret Value:	No return value
******************************************************************************/
int qdma_dump_register_value(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump all cpu tx/rx dscp information.
 Input Args:	None
 Ret Value:	No return value
******************************************************************************/
int qdma_dump_descriptor_info(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump all cpu tx/rx dscp cnt info.
 Input Args:	None
 Ret Value:	No return value
******************************************************************************/
int qdma_dump_descriptor_cnt(struct ECNT_QDMA_Data *qdma_data);

/******************************************************************************
 Descriptor:	It's used to dump all IRQ information.
 Input Args:	None
 Ret Value:	No return value
******************************************************************************/
int qdma_dump_irq_info(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump all IRQ2 information.
 Input Args:	None
 Ret Value:	No return value
******************************************************************************/
int qdma_dump_irq2_info(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump all hwfwd dscp information.
 Input Args:	None
 Ret Value:	No return value
******************************************************************************/
int qdma_dump_hwfwd_info(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set dbg level for qdma debug.
 Input Args:	arg1: dbgLevel: 0 for ERROR; 1 for ST; 2 for WARN; 3 for MSG
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_dbg_level(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump tx/rx dma busy bit in several seconds.
 Input Args:	arg1: busyDuration means the seconds of dma busy bit print out. 
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_dump_dma_busy(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to update dynamic threshold per ms.
 Input Args:	NULL. 
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_config_trigger_test(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump the value of specific qdma register in several seconds.
 Input Args:	arg1: regOffset means the offset of specific qdma register. 
			arg2: pollingDuration means the seconds of polling time. 
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_dump_reg_polling(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set receive rx ring1 packets only: rx1:rx0 from 5:2 to 5:0.
 Input Args:	arg1: forceEn means enable/disable force receive rx1 function.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_force_receive_rx_ring1(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable or disable TX_DROP_EN.
 Input Args:	arg1: mode means enable or disable.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_drop_en(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of TX_DROP_EN.
 Ret Value:		0: Disable, 1: Enable.
******************************************************************************/
int qdma_get_tx_drop_en(struct ECNT_QDMA_Data *qdma_data) ;

/*--------------EN7580 New APIs start-------------------*/
int qdma_general_set_trtcm_cfg(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_get_trtcm_cfg(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_general_set_ratelimit_mode_cfg(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_get_ratelimit_mode_cfg(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_set_ratelimit_mode_value(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_get_ratelimit_mode_value(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_set_ratelimit_bucket_size(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_general_set_trtcm_mode_cfg(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_get_trtcm_mode_cfg(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_set_trtcm_mode_value(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_get_trtcm_mode_value(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_set_trtcm_bucket_size(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_set_flow_cntr_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_flow_cntr_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_flow_cntr_value(struct ECNT_QDMA_Data *qdma_data);
int qdma_clear_flow_cntr_value(struct ECNT_QDMA_Data *qdma_data);

int qdma_set_tx_wred_mode(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_tx_wred_mode(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_tx_wred_threshold(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_tx_wred_threshold(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_tx_wred_probability(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_tx_wred_probability(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_tx_wred_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_cpu_rx_red_probability(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_cpu_rx_red_probability(struct ECNT_QDMA_Data *qdma_data);

int qdma_set_channel_close_status(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_channel_close_status(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_channel_empty_status(struct ECNT_QDMA_Data *qdma_data);

int qdma_set_oam_modify_fp_en(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_oam_modify_fp_en(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_multicast_en(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_multicast_en(struct ECNT_QDMA_Data *qdma_data);

int qdma_allocate_meter(struct ECNT_QDMA_Data *qdma_data);
int qdma_free_meter(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_qos_flag(struct ECNT_QDMA_Data *qdma_data);
int qdma_allocate_acnt(struct ECNT_QDMA_Data *qdma_data);
int qdma_free_acnt(struct ECNT_QDMA_Data *qdma_data);

int qdma_dram_test_dma_config(struct ECNT_QDMA_Data *qdma_data);
int qdma_dram_test_dma_enable(struct ECNT_QDMA_Data *qdma_data);
int qdma_dram_test_is_rx_done(struct ECNT_QDMA_Data *qdma_data);
int qdma_dram_test_dma_skb_get(struct ECNT_QDMA_Data *qdma_data);

int qdma_set_sla_chnl_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_sla_chnl_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_qos_aging_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_qos_aging_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_per_queue_aging_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_per_queue_aging_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_multicast_sptag_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_multicast_sptag_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_multicast_fport_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_multicast_fport_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_txq_cngst_static_channel_en(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_txq_cngst_static_queue_ratio(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_hqos_en(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_dbg_cntr_rx_ring(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_multicast_1toN_cfg(struct ECNT_QDMA_Data *qdma_data);

int qdma_reset(struct ECNT_QDMA_Data *qdma_data);

/*--------------EN7580 New APIs end-------------------*/

int qdma_get_channel_cfg(struct ECNT_QDMA_Data *qdma_data);

/******************************************************************************
 Descriptor:	It's used to set the value of tx ratemeter config. It includes
 			timeDivisor and timeSlice, while tx_rateMeter_En always enable.
 Input Args:	arg1: The pointer of the  Tx RateMeter Config struct.
                     tx ratemeter default enable.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_ratemeter(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of tx ratemeter config. It includes
 			timeDivisor and timeSlice, while tx_rateMeter_En always enable.
 Input Args:	arg1: The pointer of the  Tx RateMeter Config struct. It includes
 				- timeDivisor: the divisor of timeSlice, which will affect datarate.
 				- timeSlice: ratemeter calculate duration.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_ratemeter(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get tx channel datarate.
 Input Args:	arg1: channel index
 Ret Value:		return tx datarate by channel, unit is bps
******************************************************************************/
int qdma_get_tx_channel_datarate(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable tx global ratelimit function. 
 Input Args:	No input arguments
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_enable_tx_ratelimit(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of tx rate limit config. It includes bucketScale and ticksel.
 Input Args:	arg1: The pointer of the  Tx Rate Limit Config struct.It includes
 			- txRateLimitUnit: unit is 1kbps, scope is (1kbps~65kbps). 0 means not change.
 			- txRateLimitBucketScale: ratemeter calculate duration.
 			- txRateLimitEn: default enable, ratelimit enable or not decide by chnlRateLimitEn.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_ratelimit_cfg(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of tx rate limit config. It includes
 			tx_rateLimit_En, bucketScale and ticksel.
 Input Args:	arg1: The pointer of the  Tx Rate Limit Config struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_ratelimit_cfg(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of tx rate limit parameter. It includes chnlRateLimitEn and PIR.
 Input Args:	arg1: The pointer of the TX RateLimit Parameter struct.It includes
 			- chnlRateLimitEn: means current channel ratelimit enable or disable.
 			- PIR: unit is kbps, scope is [1kbps~1Gbps]
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_ratelimit(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of tx rate limit parameter. It includes PIR and PBS.
 Input Args:	arg1: The pointer of the  TX RateLimit Parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_ratelimit(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of tx dba report parameter. It includes CIR, CBS, PIR and PBS.
 Input Args:	arg1: The pointer of the TX RateLimit Parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_dba_report(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of tx rate limit parameter. It includes PIR and PBS.
 Input Args:	arg1: The pointer of the  TX RateLimit Parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_dba_report(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable rx protect mode. 
 Input Args:	arg1: Enable/disable rx protect mode (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_rx_protect_en(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get rx protect mode:enable/disable.
 Input Args:	No input arguments
 Ret Value:		return rx protect mode (QDMA_ENABLE/QDMA_DISABLE)
******************************************************************************/
int qdma_get_rx_protect_en(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of rx low threshold. 
 Input Args:	arg1: The pointer of the RX Low Threshold struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_rx_low_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of rx low threshold. 
 Input Args:	arg1: The pointer of the RX Low Threshold struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_rx_low_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of rx rate limit config. It includes
 			rx_rateLimit_En, bucketScale and tickSel.
 Input Args:	arg1: The pointer of the RX Rate Limit config struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_rx_ratelimit_en(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of rx rate limit config. It includes
 			rx_rateLimit_En, bucketScale and tickSel.
 Input Args:	arg1: The pointer of the RX Rate Limit config struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_rx_ratelimit_pkt_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of rx rate limit config. It includes
 			rx_rateLimit_En, bucketScale and tickSel.
 Input Args:	arg1: The pointer of the RX Rate Limit config struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_rx_ratelimit_cfg(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of rx rate limit parameter. It includes PIR and PBS.
 Input Args:	arg1: The pointer of the RX RateLimit Parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_rx_ratelimit(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of rx rate limit parameter. It includes PIR and PBS.
 Input Args:	arg1: The pointer of the  RX RateLimit Parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_rx_ratelimit(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable tx queue DEI dropped function. 
 Input Args:	arg1: Enable/disable tx queue DEI function (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_dei_drop_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get tx queue DEI dropped mode. 
 Input Args:	No input arguments
 Ret Value:		return tx queue DEI mode (QDMA_ENABLE/QDMA_DISABLE)
******************************************************************************/
int qdma_get_txq_dei_drop_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable tx queue dynamic cngst mode. 
 Input Args:	arg1: Enable/disable tx queue dynamic cngst (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get tx queue dynamic cngst mode. 
 Input Args:	No input arguments
 Ret Value:		return tx queue dynamic cngst mode.  (QDMA_ENABLE/QDMA_DISABLE)
******************************************************************************/
int qdma_get_txq_cngst_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set tx queue dei threshold scale, scope is (1/2 ~ 1/16) 
 Input Args:	arg1: set tx queue dei threshold scale  (1/2 ~ 1/16)
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_dei_threshold_scale(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get tx queue dei threshold scale, scope is (1/2 ~ 1/16) 
 Input Args:	No input arguments
 Ret Value:	return tx queue dei threshold scale  (1/2 ~ 1/16)
******************************************************************************/
int qdma_get_txq_dei_threshold_scale(struct ECNT_QDMA_Data *qdma_data) ;

/* need modify later */
int qdma_set_txq_cngst_auto_config(struct ECNT_QDMA_Data *qdma_data) ;

/* need modify later */
int qdma_get_txq_cngst_auto_config(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of tx dynamic threshold. It includes 
 			total max threshold, total min Threshold, channel max threshold, 
 			channel min threshold, queue max threshold, queue min threshold.
 Input Args:	arg1: The pointer of the TX dynamic threshold struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_dynamic_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of tx dynamic threshold. It includes 
 			total max threshold, total min Threshold, channel max threshold, 
 			channel min threshold, queue max threshold, queue min threshold.
 Input Args:	arg1: The pointer of the TX dynamic threshold struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_txq_cngst_dynamic_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set txQ total max/min threshold, unit is byte.
 Input Args:	arg1: set txQ total max/min threshold,
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_total_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get txQ total max/min threshold, unit is byte.
 Input Args:	No input arguments
 Ret Value:	return txQ total max/min threshold,
******************************************************************************/
int qdma_get_txq_cngst_total_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set txQ channel  max/min threshold, unit is byte.
 Input Args:	arg1: set txQ channel max/min threshold,
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_channel_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get txQ channel  max/min threshold, unit is byte.
 Input Args:	No input arguments
 Ret Value:	return txQ channel max/min threshold,
******************************************************************************/
int qdma_get_txq_cngst_channel_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set txQ queue max/min threshold, unit is byte.
 Input Args:	arg1: set txQ queue max/min threshold,
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_queue_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get txQ queue max/min threshold, unit is byte.
 Input Args:	No input arguments
 Ret Value:	return txQ queue max/min threshold,
******************************************************************************/
int qdma_get_txq_cngst_queue_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set txQ peekrate parameters.
 Input Args:	arg1: set txQ peekrate parameters, it includes
 			- peekRateEn: peekrate enable or disable.
 			- peekRateMargin: scope is [0~3], means (0, 25%, 50%, 100%)
 			- peekRateDuration: unit is ms
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_peekrate_params(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get txQ peekrate parameters.
 Input Args:	No input arguments
 Ret Value:	return txQ peekrate parameters.
******************************************************************************/
int qdma_get_txq_peekrate_params(struct ECNT_QDMA_Data *qdma_data) ;


/******************************************************************************
 Descriptor:	It's used to set txQ static queue normal threshold.
 Input Args:	arg1: set txQ static queue normal threshold, it includes
 			- normalThrh[8]: unit is byte, scope is 256Byte~16MByte.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_static_queue_normal_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set txQ static queue dei threshold.
 Input Args:	arg1: set txQ static queue dei threshold, it includes
 			- deiThrh[8]: unit is byte, scope is 256Byte~16MByte.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_static_queue_dei_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get txQ dynamic cngst infomation.
 Input Args:	No input arguments
 Ret Value:	return txQ dynamic cngst infomation.
******************************************************************************/
int qdma_get_txq_cngst_dynamic_info(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get txQ static cngst infomation.
 Input Args:	No input arguments
 Ret Value:	return txQ static cngst infomation.
******************************************************************************/
int qdma_get_txq_cngst_static_info(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set txQ queue nonblocking or not.
 Input Args:	arg1: The pointer of the TXQ Cngst Queue Cfg struct.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_queue_nonblocking(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get txQ queue nonblocking or not.
 Input Args:	No input arguments
 Ret Value:	return txQ queue nonblocking or not.
******************************************************************************/
int qdma_get_txq_cngst_queue_nonblocking(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set txQ channel nonblocking or not.
 Input Args:	arg1: The pointer of the TXQ Cngst Chanel Cfg struct.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_channel_nonblocking(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get txQ channel nonblocking or not.
 Input Args:	No input arguments
 Ret Value:	return txQ channel nonblocking or not.
******************************************************************************/
int qdma_get_txq_cngst_channel_nonblocking(struct ECNT_QDMA_Data *qdma_data) ;


/******************************************************************************
 Descriptor:	It's used to set virtual channel mode.
 Input Args:	arg1: set virtual channel mode, it includes
 			- virChnlEn: virtual channel mapping mode enable or disable.
 			- virChnlMode: 16 Queue or 32 Queue.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_virtual_channel_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get virtual channel mode.
 Input Args:	No input arguments
 Ret Value:	return virtual channel mode.
******************************************************************************/
int qdma_get_virtual_channel_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set single physical channel qos value.
 Input Args:	arg1: set single physical channel qos value, it includes
 			- phyChnl: specific physical channel index, scope is (0~7) or (0~15).
 			- qosType: means SP / WRR / SPWRR3 / SPWRR2.
 			- queue[4]: means weight of 2 or 4 queue.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_virtual_channel_qos(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get single physical channel qos value.
 Input Args:	No input arguments
 Ret Value:	return single physical channel qos value.
******************************************************************************/
int qdma_get_virtual_channel_qos(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_clear_dbg_cntr_value_all(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_clear_and_set_dbg_cntr_channel_group(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_clear_and_set_dbg_cntr_queue_group(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_clear_and_set_dbg_cntr_ring_group(struct ECNT_QDMA_Data *qdma_data) ;
//added tmp by czw
int qdma_set_cntr_channel(struct ECNT_QDMA_Data *qdma_data);

int qdma_dump_cntr_channel(struct ECNT_QDMA_Data *qdma_data);

int qdma_get_dbg_cntr_all_queue_value(struct ECNT_QDMA_Data *qdma_data);

int qdma_dump_dbg_cntr_value(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump the tx qos type and queue weigth for all channel.
 Input Args:		chnlIdx: specific the channel ID (0~31) and chnl
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_dump_tx_qos(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_dump_virtual_channel_qos(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_dump_tx_ratelimit(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_dump_rx_ratelimit(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_dump_tx_dba_report(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_dump_txq_cngst(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_dump_info_all(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_set_tx_cngst_mode(struct ECNT_QDMA_Data *qdma_data);

int qdma_get_tx_cngst_mode(struct ECNT_QDMA_Data *qdma_data);

/******************************************************************************
 Descriptor:	It's used to read VIP info from fe. 
 Input Args:	None
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_read_vip_info(struct ECNT_QDMA_Data *qdma_data);
/******************************************************************************
 Descriptor:	It's used to set mac qos config ,include qos_flag, queuemask qos_wrr_info etc. 
 Input Args:	qdma_data
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_mac_qos_config(struct ECNT_QDMA_Data *qdma_data);

/******************************************************************************
 Descriptor:	It's used to set tx msg. 
 Input Args:	qdma_data
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_api_set_txmsg(struct ECNT_QDMA_Data *qdma_data);

/******************************************************************************
 Descriptor:	It's used to get rx msg. 
 Input Args:	qdma_data
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_api_get_rxmsg(struct ECNT_QDMA_Data *qdma_data);

/******************************************************************************
 Descriptor:	It's used to get rx msg for eth driver. 
 Input Args:	qdma_data
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_api_get_rxmsg_eth(struct ECNT_QDMA_Data *qdma_data);

int qdma_api_check_dscp_is_free(struct ECNT_QDMA_Data *qdma_data);

int qdma_api_set_downstream_qos_mode(struct ECNT_QDMA_Data *qdma_data);

int qdma_api_get_downstream_qos_mode(struct ECNT_QDMA_Data *qdma_data);





/************************************************************************
*                  FE Defines/MACROS
*************************************************************************
*/



/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
int fe_api_set_pkt_length(struct ecnt_fe_data *fe_data);

int fe_api_set_channel_enable(struct ecnt_fe_data *fe_data);
int fe_api_set_mac_addr(struct ecnt_fe_data *fe_data);
int fe_api_set_mac_addr_7516(struct ecnt_fe_data *fe_data);
int fe_api_set_wan_port_7516(struct ecnt_fe_data *fe_data);
int fe_get_hwfwd_channel(struct ecnt_fe_data *fe_data);
int fe_set_hwfwd_channel(struct ecnt_fe_data *fe_data);
int fe_api_set_channel_retire(struct ecnt_fe_data *fe_data);
int fe_api_set_crc_strip(struct ecnt_fe_data *fe_data);
int fe_api_set_padding(struct ecnt_fe_data *fe_data);
int fe_api_get_ext_tpid(struct ecnt_fe_data *fe_data);
int fe_api_set_ext_tpid(struct ecnt_fe_data *fe_data);
int fe_api_get_fw_cfg(struct ecnt_fe_data *fe_data);

int fe_api_set_fw_cfg(struct ecnt_fe_data *fe_data);
int fe_api_set_drop_udp_chksum_err_enable(struct ecnt_fe_data *fe_data);
int fe_api_set_drop_tcp_chksum_err_enable(struct ecnt_fe_data *fe_data);
int fe_api_set_drop_ip_chksum_err_enable(struct ecnt_fe_data *fe_data);
int fe_api_set_drop_runt_enable(struct ecnt_fe_data *fe_data);
int fe_api_set_drop_long_enable(struct ecnt_fe_data *fe_data);
int fe_api_set_drop_crc_err_enable(struct ecnt_fe_data *fe_data);
int fe_api_set_vlan_check(struct ecnt_fe_data *fe_data);
int fe_api_get_ok_cnt(struct ecnt_fe_data *fe_data);
int fe_api_get_rx_err_crc_cnt(struct ecnt_fe_data *fe_data);
int fe_api_get_rx_drop_fifo_cnt(struct ecnt_fe_data *fe_data);
int fe_api_get_rx_drop_err_cnt(struct ecnt_fe_data *fe_data);
int fe_api_get_ok_byte_cnt(struct ecnt_fe_data *fe_data);
int fe_api_get_tx_get_cnt(struct ecnt_fe_data *fe_data);
int fe_api_get_tx_drop_cnt(struct ecnt_fe_data *fe_data);
int fe_api_get_time_stamp(struct ecnt_fe_data *fe_data);
int fe_api_set_time_stamp(struct ecnt_fe_data *fe_data);
int fe_api_set_ins_vlan_tpid(struct ecnt_fe_data *fe_data);
int fe_api_set_vlan_enable(struct ecnt_fe_data *fe_data);
int fe_api_set_black_list(struct ecnt_fe_data *fe_data);
int fe_api_set_ether_type(struct ecnt_fe_data *fe_data);
int fe_api_set_L2U_key(struct ecnt_fe_data *fe_data);
int fe_api_get_ac_group_pkt_cnt(struct ecnt_fe_data *fe_data);
int fe_api_get_ac_group_byte_cnt(struct ecnt_fe_data *fe_data);
int fe_api_clear_ac_group_pkt_cnt(struct ecnt_fe_data *fe_data);
int fe_api_clear_ac_group_byte_cnt(struct ecnt_fe_data *fe_data);
int fe_api_set_meter_group(struct ecnt_fe_data *fe_data);
int fe_api_get_meter_group(struct ecnt_fe_data *fe_data);
int fe_api_set_gdm_pcp_coding(struct ecnt_fe_data *fe_data);
int fe_api_set_cdm_pcp_coding(struct ecnt_fe_data *fe_data);
int fe_api_set_vip_enable(struct ecnt_fe_data *fe_data);
int fe_api_get_eth_rx_cnt(struct ecnt_fe_data *fe_data);
int fe_api_get_eth_tx_cnt(struct ecnt_fe_data *fe_data);
int fe_api_get_eth_frame_cnt(struct ecnt_fe_data *fe_data);
int fe_api_get_eth_err_cnt(struct ecnt_fe_data *fe_data);
int fe_api_set_clear_mib(struct ecnt_fe_data *fe_data);
int fe_api_set_cdm_rx_red_drop_mode(struct ecnt_fe_data *fe_data);
int fe_api_get_cdm_rx_red_drop_mode(struct ecnt_fe_data *fe_data);
int fe_api_set_channel_retire_all(struct ecnt_fe_data *fe_data);
int fe_api_set_channel_retire_one(struct ecnt_fe_data *fe_data);
int fe_api_set_tx_rate(struct ecnt_fe_data *fe_data);
int fe_api_set_rx_rate(struct ecnt_fe_data *fe_data);
int fe_api_set_rxuc_rate(struct ecnt_fe_data *fe_data);
int fe_api_set_rxbc_rate(struct ecnt_fe_data *fe_data);
int fe_api_set_rxmc_rate(struct ecnt_fe_data *fe_data);
int fe_api_set_rxoc_rate(struct ecnt_fe_data *fe_data);
int fe_api_set_rx_mac_filter_rate(struct ecnt_fe_data *fe_data);
int fe_api_add_vip_ether(struct ecnt_fe_data *fe_data);
int fe_api_add_vip_ppp(struct ecnt_fe_data *fe_data);
int fe_api_add_vip_ip(struct ecnt_fe_data *fe_data);
int fe_api_add_vip_tcp(struct ecnt_fe_data *fe_data);
int fe_api_add_vip_udp(struct ecnt_fe_data *fe_data);
int fe_api_del_vip_ether(struct ecnt_fe_data *fe_data);
int fe_api_del_vip_ppp(struct ecnt_fe_data *fe_data);
int fe_api_del_vip_ip(struct ecnt_fe_data *fe_data);
int fe_api_del_vip_tcp(struct ecnt_fe_data *fe_data);
int fe_api_del_vip_udp(struct ecnt_fe_data *fe_data);
int fe_api_add_l2lu_vlan_dscp(struct ecnt_fe_data *fe_data);
int fe_api_add_l2lu_vlan_trfc(struct ecnt_fe_data *fe_data);
int fe_api_del_l2lu_vlan_dscp(struct ecnt_fe_data *fe_data);
int fe_api_del_l2lu_vlan_trfc(struct ecnt_fe_data *fe_data);
int fe_api_add_traffic_class(struct ecnt_fe_data * fe_data);
int fe_api_del_traffic_class(struct ecnt_fe_data * fe_data);
int fe_api_set_tx_favor_oam_enable(struct ecnt_fe_data *fe_data);
int fe_api_set_tls_cfg(struct ecnt_fe_data *fe_data);
int fe_api_do_fe_reset(struct ecnt_fe_data *fe_data);
int fe_api_set_loopback_enable(struct ecnt_fe_data *fe_data);
int fe_api_set_loopback_mode(struct ecnt_fe_data *fe_data);
int fe_api_get_unknown_mul_pkt(struct ecnt_fe_data *fe_data);
int fe_api_set_glo_rate_byte(struct ecnt_fe_data *fe_data);
int fe_api_get_pppoe_info(struct ecnt_fe_data *fe_data);
int fe_api_set_pppoe_info_clean(struct ecnt_fe_data *fe_data);
int fe_api_get_tx_traffic(struct ecnt_fe_data *fe_data);
int fe_api_get_rx_traffic(struct ecnt_fe_data *fe_data);
int fe_api_get_tx_rate(struct ecnt_fe_data *fe_data);
int fe_api_get_rx_rate(struct ecnt_fe_data *fe_data);
int fe_api_get_tx_octets(struct ecnt_fe_data *fe_data);
int fe_api_get_rx_octets(struct ecnt_fe_data *fe_data);
int fe_api_get_rx_discard_counter(struct ecnt_fe_data *fe_data);
int fe_api_get_tx_discard_counter(struct ecnt_fe_data *fe_data);
int fe_api_get_rx_error_counter(struct ecnt_fe_data *fe_data);
int fe_api_get_tx_error_counter(struct ecnt_fe_data *fe_data);
int fe_api_add_dev_to_total_account(struct ecnt_fe_data *fe_data);
int fe_api_add_stb_src_ip(struct ecnt_fe_data *fe_data);
int fe_api_del_stb_src_ip(struct ecnt_fe_data *fe_data);

int fe_api_set_rx_mac_filter(struct ecnt_fe_data *fe_data);

int fe_api_set_mc_vlan_global(struct ecnt_fe_data *fe_data);
int fe_api_get_mc_vlan_global(struct ecnt_fe_data *fe_data);
int fe_api_set_mc_vlan_table_cfg(struct ecnt_fe_data *fe_data);
int fe_api_get_mc_vlan_table_cfg(struct ecnt_fe_data *fe_data);
int fe_api_set_mc_vlan_action_cfg(struct ecnt_fe_data *fe_data);
int fe_api_get_mc_vlan_action_cfg(struct ecnt_fe_data *fe_data);
int fe_api_set_mc_vlan_clear_all(struct ecnt_fe_data *fe_data);
int fe_api_xfi_link_change(struct ecnt_fe_data *fe_data);
int fe_api_set_gdma_misc_config(struct ecnt_fe_data *fe_data);
int fe_api_get_hsgmii_rx_cnt(struct ecnt_fe_data *fe_data);
int fe_api_get_hsgmii_tx_cnt(struct ecnt_fe_data *fe_data);
int fe_api_set_clr_cnt(struct ecnt_fe_data *fe_data);
int fe_api_set_aewan_fwdfq(struct ecnt_fe_data *fe_data);
int fe_api_set_aewan_ifcdisable(struct ecnt_fe_data *fe_data);
int fe_api_set_gdm2_sptag_for_loopback(struct ecnt_fe_data *fe_data);
int fe_api_set_tunnel_cfg(struct ecnt_fe_data *fe_data);
int fe_api_set_gdm_sptag_for_extswitch(struct ecnt_fe_data *fe_data);
int fe_api_pse_oq_rsv_en(struct ecnt_fe_data *fe_data);
int fe_api_set_hsgmii_rx_port_ratelimit(struct ecnt_fe_data *fe_data);
int fe_api_set_mbi_arb_rst(struct ecnt_fe_data *fe_data);
int fe_api_set_rmbi_frag(struct ecnt_fe_data *fe_data);
int fe_api_get_chn_rls(struct ecnt_fe_data *fe_data);
int fe_api_set_tmbi_frag(struct ecnt_fe_data *fe_data);
int fe_api_set_gdma_enable(struct ecnt_fe_data *fe_data);
int fe_api_set_gdma_disable(struct ecnt_fe_data *fe_data);
int fe_api_set_chn_retire_action(struct ecnt_fe_data *fe_data);
int fe_api_set_chn_retire_done(struct ecnt_fe_data *fe_data);
int fe_api_set_qbi_fttr_chn_disable(struct ecnt_fe_data *fe_data);
int fe_api_set_force_slow_enable(struct ecnt_fe_data *fe_data);
int fe_api_set_force_slow_duty(struct ecnt_fe_data *fe_data);

int fe_ioctl_set_tls_cfg(unsigned char cmd,unsigned char tls_port,unsigned short tls_vlan);
int fe_api_tls_forwad(struct ecnt_fe_data *fe_data);
int fe_ioctl_set_trunk_mode_tls_cfg(unsigned char cmd,FE_Tls_info_t tls_info);

int fe_api_init_resource_manage(struct ecnt_fe_data *fe_data);
int fe_api_deinit_resource_manage(struct ecnt_fe_data *fe_data);
int fe_api_set_meter_ratelimit(struct ecnt_fe_data *fe_data);
int fe_api_get_meter_ratelimit(struct ecnt_fe_data *fe_data);
int fe_api_get_meter_idx(struct ecnt_fe_data *fe_data);
int fe_api_get_acnt2_idx(struct ecnt_fe_data *fe_data);
int fe_api_get_acnt1_idx(struct ecnt_fe_data *fe_data);
int fe_api_get_acnt0_idx(struct ecnt_fe_data *fe_data);
int fe_api_get_rx_ratelimit_mode(struct ecnt_fe_data *fe_data);
int fe_api_set_rx_ratelimit_rule(struct ecnt_fe_data *fe_data);
int fe_api_set_rx_ratelimit_mode(struct ecnt_fe_data *fe_data);
int fe_api_set_meter_ctl_by_olt(struct ecnt_fe_data *fe_data);
int fe_api_get_flow_cnt(struct ecnt_fe_data *fe_data);
int fe_api_get_acnt0_mode(struct ecnt_fe_data *fe_data);
int fe_api_get_acnt1_mode(struct ecnt_fe_data *fe_data);
int fe_api_set_acnt0_mode(struct ecnt_fe_data *fe_data);
int fe_api_set_acnt1_mode(struct ecnt_fe_data *fe_data);
int fe_api_set_acnt2_mode(struct ecnt_fe_data *fe_data);
int fe_api_get_meter_enable(struct ecnt_fe_data *fe_data);
int fe_api_clear_flow_cnt(struct ecnt_fe_data *fe_data);
int fe_api_get_dev_mac_index(struct ecnt_fe_data *fe_data);
int fe_api_set_pse_oq_threshold(struct ecnt_fe_data *fe_data);
int fe_api_get_wan_itf_index(struct ecnt_fe_data *fe_data);
int fe_api_set_ratelimit_for_pkt_formate(struct ecnt_fe_data *fe_data);

int fe_api_get_pse_drop_cnt(struct ecnt_fe_data *fe_data);
int fe_api_set_vip_rxq_selection(struct ecnt_fe_data *fe_data);
int fe_api_set_vip_for_tcp_speedtest(struct ecnt_fe_data *fe_data);
int fe_api_check_chn_rls(struct ecnt_fe_data *fe_data);
int fe_api_set_dev_stat_ratelimit_mode(struct ecnt_fe_data *fe_data);


typedef int (*fe_api_op_t)(struct ecnt_fe_data *fe_data);

struct airoha_eth;
struct airoha_gdm_port;
struct airoha_qdma;
enum trtcm_mode_type;
enum trtcm_param_type ;
void airoha_eth_monitor_workqueue_init(struct airoha_eth *eth);
void airoha_eth_monitor_workqueue_exit(struct airoha_eth *eth);
int airoha_qdma_set_tx_rate_limit(struct airoha_gdm_port *port,int channel, u32 rate,u32 bucket_size);
int airoha_qdma_get_trtcm_param(struct airoha_qdma *qdma, int channel, u32 addr, enum trtcm_param_type param,
				       enum trtcm_mode_type mode,u32 *val_low, u32 *val_high);

#define CONFIG_QDMA_CHANNEL		32
#define CONFIG_QDMA_QUEUE	8
#define QUEUE_ALL_NUM     (CONFIG_QDMA_CHANNEL*CONFIG_QDMA_QUEUE)

#define GENERAL_INGRESS_INDEX_MAX               (127)   /*EN7580: 128 ratelimit. or 64 trtcm*/
#define GENERAL_INGRESS_INDEX_MAX_GRP1               (31)   /*grp1: 32 ratelimit. or 16 trtcm*/
#define GENERAL_INGRESS_INDEX_MAX_GRP2               (15)   /*grp2: 16 ratelimit. or 8 trtcm*/
#define GENERAL_SLA_INDEX_MAX                                   (31)

#define TRTCM_MODE_MASK			BIT(30)
#define TRTCM_PARAM_CFG(trtcm_base)                (trtcm_base+0x4)
#define TRTCM_DATA_LO(trtcm_base)                  (trtcm_base+0x8)
#define TRTCM_DATA_HI(trtcm_base)                  (trtcm_base+0xc)
#define TRTCM_PARA_METER_GROUP_SHIFT    		(26)
#define TRTCM_PARA_IDX_RATE_TYPE_SHIFT  		(16)
#define TRTCM_PARA_IDX_RATE_TYPE_MASK   		(1<<TRTCM_PARA_IDX_RATE_TYPE_SHIFT)  //pir&pbs or cir&cbs
#define TRTCM_PARA_IDX_INDEX_SHIFT              (17)
#define TRTCM_PARA_IDX_INDEX_MASK               (0x3F<<TRTCM_PARA_IDX_INDEX_SHIFT)       //7580
#define TRTCM_PARA_TYPE_SHIFT                   (28)
#define TRTCM_PARA_TYPE_MASK                    (0x3<<TRTCM_PARA_TYPE_SHIFT)
#define TRTCM_PARA_RWCMD                                (1<<31)
#define TRTCM_PARA_RWCMD_DONE                   (1<<30)
#define RATELIMIT_PARA_IDX_INDEX_SHIFT          (16)
#define RATELIMIT_PARA_IDX_INDEX_MASK           (0xFF<<RATELIMIT_PARA_IDX_INDEX_SHIFT)   //7581
#define RATELIMIT_PARA_RWCMD                            (1<<31)
#define RATELIMIT_PARA_RWCMD_DONE                       (1<<30)
#define RATELIMIT_PARA_TYPE_SHIFT                       (28)
#define RATELIMIT_PARA_TYPE_MASK                        (0x3<<RATELIMIT_PARA_TYPE_SHIFT)
#define TRTCM_FAST_TICK_SHIFT                           (0)
#define RATELIMIT_BYTE_MODE_BUCKET_SHIFT                (10)
#define RATELIMIT_PKT_MODE_BUCKET_SHIFT                 (0)
#define TRTCM_FAST_TICK_MASK                            (0xFFFF<<TRTCM_FAST_TICK_SHIFT)
#define TRTCM_SLOW_TICKRATIO_SHIFT                      (16)
#define TRTCM_SLOW_TICKRATIO_MASK                       (0x3FFF<<TRTCM_SLOW_TICKRATIO_SHIFT)
#define METER_1K        (1000)
#define METER_1M        (METER_1K<<10)
#define TRTCM_TOKEN_RATE_INTEGER_SHIFT          (6)
#define TRTCM_TOKEN_RATE_INTEGER_MASK           (0x3FFFF<<TRTCM_TOKEN_RATE_INTEGER_SHIFT)

typedef struct
{
	unsigned int proto;/*ipv4 or ipv6*/
	unsigned int vlan_tag_num;
	unsigned short outer_tci;
	unsigned short inner_tci;
	unsigned char grp_addr[16];
	unsigned char src_addr[16];
	struct net_device* ori_dev;
	unsigned short br_vid;
}MULTICAST_INFO_t;

typedef struct
{
	struct list_head list;
	MULTICAST_INFO_t ppe_multicast_info;
	unsigned int foe_index;
	unsigned int port_mask;/*bit0-3 for eth0.1-eth0.4;bit16-23 for ra0-ra7;bit24-31 for rai0-rai7;other reserve*/
    unsigned int local;
	unsigned char state;
	struct timer_list age_timer;
	unsigned char valid;
	atomic_t deleted;
}MULTICAST_HWNATENTRY_t;


enum
{
	PPE_MULTICAST_HWNATENTRY_STATE_UNBIND=0,
	PPE_MULTICAST_HWNATENTRY_STATE_BINDED,
	PPE_MULTICAST_HWNATENTRY_STATE_DROP,
};

enum hwnat_mcast_status {
        HWNAT_MCAST_INVALID = 0,
        HWNAT_MCAST_VALID = 1
};


#define HWNAT_LAN_IF_MAXNUM	12
#define HWNAT_LAN_IF_BASE		0//0~7 is lan interface
#define HWNAT_OLT_IF_BASE		13
#define HWNAT_WLAN_IF_MAXNUM	16 //16 is max wifi interface
#define HWNAT_WLAN_IF_BASE		16//16  is base
#define HWNAT_LAN_IF_MASK		(0x3FFF)
#define HWNAT_USB_IF_BASE		15
#define HWNAT_USB_IF_NUM		1 
#define HWNAT_USB_IF_MASK		(0x1)
#define HWNAT_XSI_IF_BASE		14
#define HWNAT_XSI_IF_NUM		1 
#define HWNAT_XSI_IF_MASK		(0x1)

enum
{
	PPE_MULTICAST_FORWARD_STATE_LAN_ONLY = 0,
	PPE_MULTICAST_FORWARD_STATE_WLAN_ONLY,
	PPE_MULTICAST_FORWARD_STATE_LAN_WLAN,
	PPE_MULTICAST_FORWARD_STATE_LAN_XSI,
	PPE_MULTICAST_FORWARD_STATE_XSI_ONLY,
	PPE_MULTICAST_FORWARD_STATE_XSI_WLAN,
	PPE_MULTICAST_FORWARD_STATE_LAN_HSGMII_1toN,
	PPE_MULTICAST_FORWARD_STATE_UNKNOWN,
};

#define IS_ETH_SERDES 0x10
#define IS_USB_SERDES 0x20
#define IS_PCIE0_SERDES 0x40
#define IS_PCIE1_SERDES 0x80

//for AN7581
#define ETH_SERDES_FPORT FE_PSE_PORT_GDM4
#define USB_SERDES_FPORT FE_PSE_PORT_GDM4
#define PCIE0_SERDES_FPORT FE_PSE_PORT_GDM3
#define PCIE1_SERDES_FPORT FE_PSE_PORT_GDM3

//for AN7581
#define ETH_SERDES_NBQ 		0
#define USB_SERDES_NBQ 		1
#define PCIE0_SERDES_NBQ 	4
#define PCIE1_SERDES_NBQ 	5
#define ETH_SERDES_CHNL 	0
#define USB_SERDES_CHNL 	0
#define PCIE0_SERDES_CHNL 	0
#define PCIE1_SERDES_CHNL 	0



/* -----------------PSE Port Number Info ----------------- */

#define PSE_PORT_NUM                (11) /* Port-0 ~ Port-9 for normal use , Port-15 for free */
#define PSE_PORT0_QUEUE_NUM         (6)
#define PSE_PORT1_QUEUE_NUM         (6)
#define PSE_PORT2_QUEUE_NUM         (32)
#define PSE_PORT3_QUEUE_NUM         (6)
#define PSE_PORT4_QUEUE_NUM         (4)
#define PSE_PORT5_QUEUE_NUM         (6)


//FE CNT

#define PSE_SHARE_BUF_USED_MAX_MASK	(0x7fff)
#define REG_FE_PSE_BUF_USE_REC			 0x100 /* 30:16, SHARE_USED_CNT_MAX; 14:0, FQ_CNT_MIN */
#define PSE_SHARE_BUF_USED_CNT_SHIFT	(16)
#define PSE_SHARE_BUF_USED_CNT_MASK	(0x7fff)
#define PSE_SHARE_BUF_FREE_CNT_MASK	(0x7fff)
#define REG_FE_PSE_SHARE_BUF_STA		0x104
#define REG_FE_PSE_PORT_STA        		0x10C
#define REG_FE_PSE_OQ_PCNT				0x110
#define REG_FE_PSE_OQ_PCNT_REC			0x114 /* 29:16, OQ_CNT_MAX; 13:0, OQ_REAL_CNT_MAX */
#define REG_FE_PSE_FQFC_CFG_STA(i)		(0x118 + (i<<2))
#define REG_FE_PSE_FQFC_CFG_STA0		0x118
#define REG_FE_PSE_FQFC_CFG_STA1		0x11C
#define REG_FE_PSE_DROP_CNT(i)	   		(0x120 + (i<<2))
#define REG_FE_PSE_DROP_CNT_0      		0x120
#define REG_FE_PSE_DROP_CNT_1      		0x124
#define REG_FE_PSE_DROP_CNT_2      		0x128
#define REG_FE_PSE_DROP_CNT_3      		0x12C
#define REG_FE_PSE_DROP_CNT_4      		0x130
#define REG_FE_PSE_DROP_CNT_5      		0x134
#define REG_FE_PSE_DROP_CNT_6      		0x138
#define REG_FE_PSE_DROP_CNT_7      		0x13C
#define REG_FE_PSE_DROP_CNT_8      		0x140
#define REG_FE_PSE_DROP_CNT_9      		0x144
#define REG_FE_PSE_PORT_Q_USE_STA(i)	(0x150 + (i<<2))
#define REG_FE_PSE_PORT_Q_USE_STA0		0x150/* 30:16, P0_IQ_PCNT; 14:0, P0_OQ_PCNT */
#define REG_FE_PSE_PORT_Q_USE_STA1		0x154
#define REG_FE_PSE_PORT_Q_USE_STA2		0x158
#define REG_FE_PSE_PORT_Q_USE_STA3		0x15C
#define REG_FE_PSE_PORT_Q_USE_STA4		0x160
#define REG_FE_PSE_PORT_Q_USE_STA5		0x164
#define REG_FE_PSE_PORT_Q_USE_STA6		0x168
#define REG_FE_PSE_PORT_Q_USE_STA7		0x16C
#define REG_FE_PSE_PORT_Q_USE_STA8		0x170
#define REG_FE_PSE_PORT_Q_USE_STA9		0x174


#define CDM_CNT_BASE(_n)			\
	((_n) == 2 ? (CDM_BASE(1) + 0x100) : (CDM_BASE(0) + 0x100))
	
#define REG_FE_GDM_TX_GET_PKT_CNT(_n)		(GDM_BASE(_n) + 0x100)

/*CDM1*/
#define CDMA1_TX_OK_CNT             (CDM_CNT_BASE(1) + 0x80)
#define CDMA1_RXCPU_OK_CNT          (CDM_CNT_BASE(1) + 0x90)
#define CDMA1_RXHWF_OK_CNT          (CDM_CNT_BASE(1) + 0x94)

#define CDMA1_RXCPU_KA_CNT          (CDM_CNT_BASE(1) + 0x8c)
#define CDMA1_RXHWF_FAST_ALL_CNT    (CDM_CNT_BASE(1) + 0x98)
#define CDMA1_RXOQ5_OK_CNT			(CDM_CNT_BASE(1) + 0x9c) /* default for TSO */
#define CDMA1_RXCPU_DROP_CNT        (CDM_CNT_BASE(1) + 0xa0)
#define CDMA1_RXHWF_DROP_CNT        (CDM_CNT_BASE(1) + 0xa4)
#define CDMA1_RXHWF_FAST_DROP_CNT   (CDM_CNT_BASE(1) + 0xa8)
#define CDMA1_RXOQ5_DROP_CNT		(CDM_CNT_BASE(1) + 0xac) /* default for TSO */
#define CDMA1_RXCPU0_OK_CNT         (CDM_CNT_BASE(1) + 0xb0)
#define CDMA1_RXCPU1_OK_CNT         (CDM_CNT_BASE(1) + 0xb4)
#define CDMA1_RXCPU2_OK_CNT         (CDM_CNT_BASE(1) + 0xb8)
#define CDMA1_RXCPU3_OK_CNT         (CDM_CNT_BASE(1) + 0xbc)
#define CDMA1_RXCPU0_DROP_CNT       (CDM_CNT_BASE(1) + 0xd0)
#define CDMA1_RXCPU1_DROP_CNT       (CDM_CNT_BASE(1) + 0xd4)
#define CDMA1_RXCPU2_DROP_CNT       (CDM_CNT_BASE(1) + 0xd8)
#define CDMA1_RXCPU3_DROP_CNT       (CDM_CNT_BASE(1) + 0xdc)


/*CDM2*/

#define CDMA2_TX_OK_CNT             (CDM_CNT_BASE(2) + 0x80)
#define CDMA2_RXCPU_OK_CNT          (CDM_CNT_BASE(2) + 0x90)
#define CDMA2_RXHWF_OK_CNT          (CDM_CNT_BASE(2) + 0x94)
#define CDMA2_RXCPU_KA_CNT          (CDM_CNT_BASE(2) + 0x8c)
#define CDMA2_RXHWF_FAST_ALL_CNT    (CDM_CNT_BASE(2) + 0x98)
#define CDMA2_RXOQ5_OK_CNT			(CDM_CNT_BASE(2) + 0x9c) /* default for TSO */
#define CDMA2_RXCPU_DROP_CNT        (CDM_CNT_BASE(2) + 0xa0)
#define CDMA2_RXHWF_DROP_CNT        (CDM_CNT_BASE(2) + 0xa4)
#define CDMA2_RXHWF_FAST_DROP_CNT   (CDM_CNT_BASE(2) + 0xa8)
#define CDMA2_RXOQ5_DROP_CNT		(CDM_CNT_BASE(2) + 0xac) /* default for TSO */
#define CDMA2_RXCPU0_OK_CNT         (CDM_CNT_BASE(2) + 0xb0)
#define CDMA2_RXCPU1_OK_CNT         (CDM_CNT_BASE(2) + 0xb4)
#define CDMA2_RXCPU2_OK_CNT         (CDM_CNT_BASE(2) + 0xb8)
#define CDMA2_RXCPU3_OK_CNT         (CDM_CNT_BASE(2) + 0xbc)
#define CDMA2_RXCPU0_DROP_CNT       (CDM_CNT_BASE(2) + 0xd0)
#define CDMA2_RXCPU1_DROP_CNT       (CDM_CNT_BASE(2) + 0xd4)
#define CDMA2_RXCPU2_DROP_CNT       (CDM_CNT_BASE(2) + 0xd8)
#define CDMA2_RXCPU3_DROP_CNT       (CDM_CNT_BASE(2) + 0xdc)

#define TO_MULTICAST_OFFLOAD 1
#define TO_CPU 0

#endif /* AIROHA_DP_API_H */