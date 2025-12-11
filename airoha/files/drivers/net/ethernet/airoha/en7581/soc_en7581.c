#include "../airoha_regs.h"
#include "../airoha_eth.h"
#include "../eth_sal.h"

static u32 en7581_get_fe_fport(struct sk_buff *skb, struct net_device *dev, u32 portid)
{
	u8 fport;
	fport = (portid == AIROHA_GDM4_IDX) ? FE_PSE_PORT_GDM4 : portid;
	return fport;
}

static int en7581_get_ports_id_by_desc(struct airoha_eth *eth, struct airoha_qdma_desc *desc)
{
	u32 port, sport, msg1 = le32_to_cpu(desc->msg1);

	sport = FIELD_GET(QDMA_ETH_RXMSG_SPORT_MASK, msg1);
	switch (sport) {
	case 0x18:
		port = AIROHA_PORTS_GDM4_ID; /* AN7581, ETH on GDM4 */
		break;
	case 0x16:
		port = AIROHA_PORTS_GDM3_ID; /* GDM3 */
		break;	
	case 0x10 ... 0x14:
		port = AIROHA_PORTS_GDM1_ID;
		break;
	case 0x2 ... 0x4:
		port = sport - 1;
		break;
	case 0x0:
		port = 0; /* pingpong */
		break;
	default:
		return -EINVAL;
	}

	return port >= ARRAY_SIZE(eth->ports) ? -EINVAL : port;
}

static int en7581_get_vip_port(int portid)
{
	u32 vip_port;
	switch (portid) {
	case AIROHA_GDM3_IDX:
		vip_port = XSI_PCIE0_VIP_PORT_MASK;
		break;
	case AIROHA_GDM4_IDX:
		vip_port = XSI_ETH_VIP_PORT_MASK;
		break;
	default:
		return 0;
	}
	return vip_port;
}

static void en7581_get_gdm_loopback_info(int portid, u32 *chan, u32 *pse_port)
{
	*chan = (portid == AIROHA_GDM4_IDX) ? 4 : 0;
	*pse_port = (portid == AIROHA_GDM4_IDX) ? FE_PSE_PORT_GDM3 : FE_PSE_PORT_GDM4;
}

void en7581_set_sp_dft_cport(struct airoha_eth *eth, int portid)
{
    //unsigned char srcport_idx = eth->chip->dma_srcport[portid];
	
	switch(portid){
	case AIROHA_GDM3_IDX:
		/* FIXME: handle XSI_PCE1_PORT */
		airoha_fe_rmw(eth, REG_FE_WAN_PORT,
			      WAN1_EN_MASK | WAN1_MASK | WAN0_MASK,
			      FIELD_PREP(WAN0_MASK, HSGMII_LAN_PCIE0_SRCPORT));
		airoha_fe_rmw(eth,
			      REG_SP_DFT_CPORT(HSGMII_LAN_PCIE0_SRCPORT >> 3),
			      SP_CPORT_PCIE0_MASK,
			      FIELD_PREP(SP_CPORT_PCIE0_MASK,
					 FE_PSE_PORT_CDM2));
		break;
	case AIROHA_GDM4_IDX:
		airoha_fe_rmw(eth, REG_SRC_PORT_FC_MAP6,
			      FC_ID_OF_SRC_PORT24_MASK,
			      FIELD_PREP(FC_ID_OF_SRC_PORT24_MASK, 2));
		airoha_fe_rmw(eth, REG_FE_WAN_PORT,
			      WAN1_EN_MASK | WAN1_MASK | WAN0_MASK,
			      FIELD_PREP(WAN0_MASK, HSGMII_LAN_ETH_SRCPORT));
		airoha_fe_rmw(eth, REG_SP_DFT_CPORT(HSGMII_LAN_ETH_SRCPORT >> 3),
			      SP_CPORT_ETH_MASK,
			      FIELD_PREP(SP_CPORT_ETH_MASK, FE_PSE_PORT_CDM2));
		break;
	default:
		break;
	}
}

static int en7581_support_eth_monitor(int p)
{
	/*gdm2 and gdm3 not detect, skip*/
	if(p == AIROHA_PORTS_GDM2_ID || p == AIROHA_PORTS_GDM3_ID)
		return 0;
	return 1;
}

void an7581_qdma_regs_setting(struct airoha_qdma *qdma, struct airoha_eth *eth)
{
	return ;
}

void en7581_set_qbi_fttr_chn_disable(struct airoha_eth *eth)
{
	return;
}

void an7581_fe_regs_setting(struct airoha_eth *eth)
{
	return;
}

static const char * const en7581_xsi_rsts_names[] = {
	"hsi0-mac",
	"hsi1-mac",
	"hsi-mac",
	"xfp-mac",
};
 
const struct airoha_eth_chip chip_en7581_info = {
	.id = ID_EN7581,
	.ppe_num=2,
    .dma_srcport = {
        [SERDES_PCIE0_IDX] = 0x16,
        [SERDES_PCIE1_IDX] = 0x17,
        [SERDES_ETH_IDX]   = 0x18,
        [SERDES_USB_IDX]   = 0x19,
    },
	.fport = {
		[SERDES_PCIE0_IDX] = FE_PSE_PORT_GDM3,
        [SERDES_PCIE1_IDX] = FE_PSE_PORT_GDM3,
        [SERDES_ETH_IDX]   = FE_PSE_PORT_GDM4,
        [SERDES_USB_IDX]   = FE_PSE_PORT_GDM4,
	},
	.nbq = {
		[SERDES_PCIE0_IDX] = 4,
        [SERDES_PCIE1_IDX] = 5,
        [SERDES_ETH_IDX]   = 0,
        [SERDES_USB_IDX]   = 1,
	},
	.chnl = {
		[SERDES_PCIE0_IDX] = 10,
        [SERDES_PCIE1_IDX] = 11,
        [SERDES_ETH_IDX]   = 13,
        [SERDES_USB_IDX]   = 12,
	},   
	.xsi_rsts_names = en7581_xsi_rsts_names,
	.num_xsi_rsts = ARRAY_SIZE(en7581_xsi_rsts_names),
	.get_fe_fport = en7581_get_fe_fport,
	.get_ports_id = en7581_get_ports_id_by_desc,
	.get_vip_port = en7581_get_vip_port,
	.get_gdm_lpbk_info = en7581_get_gdm_loopback_info,
	.set_sp_dft_cport = en7581_set_sp_dft_cport,
	.support_eth_monitor = en7581_support_eth_monitor,
	.set_qdma_regs = an7581_qdma_regs_setting,
	.set_qbi_fttr_chn = en7581_set_qbi_fttr_chn_disable,
	.set_fe_regs = an7581_fe_regs_setting,
};