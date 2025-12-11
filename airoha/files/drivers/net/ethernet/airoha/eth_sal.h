#ifndef __ETH_SAL_H_
#define __ETH_SAL_H_

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include "arht_dp_api.h"

enum airoha_gdm_index_t {
	AIROHA_PORTS_GDM1_ID = 0,
	AIROHA_PORTS_GDM2_ID = 1,
	AIROHA_PORTS_GDM3_ID = 2,
	AIROHA_PORTS_GDM4_ID = 3,
};

enum airoha_serdes_idx{
	SERDES_PCIE0_IDX = 0,
	SERDES_PCIE1_IDX = 1,
	SERDES_USB_IDX   = 2,
	SERDES_AE_IDX    = 3,
	SERDES_ETH_IDX   = 4,
	SERDES_MAX_IDX,
	SERDES_INVALID_IDX = SERDES_MAX_IDX,
};

struct hsgmii_srcport {
    unsigned char pcie0_srcport;
    unsigned char pcie1_srcport;
	unsigned char eth_srcport;
	unsigned char usb_srcport;
};

struct airoha_eth_chip {
	unsigned char id;
	unsigned int ppe_num;
	unsigned char dma_srcport[SERDES_MAX_IDX];
	int fport[SERDES_MAX_IDX];
	int nbq[SERDES_MAX_IDX];
	int chnl[SERDES_MAX_IDX];
	int retire_channel[SERDES_MAX_IDX];
	int gdm[SERDES_MAX_IDX];

	int num_xsi_rsts;
	const char * const *xsi_rsts_names;

	u32 (*get_fe_fport)(struct sk_buff *skb,struct net_device *dev,u32 portid);
	int (*get_ports_id)(struct airoha_eth *eth, struct airoha_qdma_desc *desc);
	int (*get_vip_port)(int portid);
	void (*set_ppe_dft_cport)(struct airoha_eth *eth, int portid, int qdmaid);
	void (*get_gdm_lpbk_info)(int portid, u32 *chan, u32 *pse_port);
	void (*set_sp_dft_cport)(struct airoha_eth *eth, int portid);
	int (*support_eth_monitor)(int p);
	int (*set_channel_retire)(struct airoha_eth *eth, struct airoha_gdm_port *port);
	void (*set_qdma_regs)(struct airoha_qdma *qdma, struct airoha_eth *eth);
	void (*set_qbi_fttr_chn)(struct airoha_eth *eth);
	void (*set_fe_regs)(struct airoha_eth *eth);
};

bool airoha_check_eth_info(struct airoha_eth *eth);

#endif /* __ETH_SAL_H_ */
