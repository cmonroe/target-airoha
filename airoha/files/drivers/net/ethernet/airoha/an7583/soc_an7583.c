/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <linux/regmap.h>

#include "../airoha_regs.h"
#include "../airoha_eth.h"
#include "../eth_sal.h"
#include "../arht_dp_api.h"

/************************************************************************
*                  D  E F I N E S   &&   C O N S T A N T S
*************************************************************************
*/
#define GET_XSI_GDM(eth,serdes_id)            ((eth)->chip->gdm[(serdes_id)])
#define GET_XSI_CHANNEL(eth,serdes_id)        ((eth)->chip->chnl[(serdes_id)])
#define GET_XSI_RETIRE_CHANNEL(eth,serdes_id) ((eth)->chip->retire_channel[(serdes_id)])

#define XSI_GLB_CFG (0x0)
#define XSI_GLB_CFG_F_IPG_NUM_SHIFT (10)
#define XSI_GLB_CFG_F_IPG_NUM_MASK (0xFC00)
#define XSI_GLB_CFG_F_TX_FRAG_LEN_SHIFT (17)
#define XSI_GLB_CFG_F_TX_FRAG_LEN_MASK (0x3E0000)
#define XSI_GLB_CFG_F_RX_FRAG_LEN_SHIFT (22)
#define XSI_GLB_CFG_F_RX_FRAG_LEN_MASK (0x7C00000)
#define XSI_GLB_CFG_F_RX_FC_EN (1<<4)
#define XSI_GLB_CFG_F_TX_FC_EN (1<<5)

/************************************************************************
*                  S T A T I C   F U N C T I O N S
*************************************************************************
*/
static u32 an7583_get_fe_fport(struct sk_buff *skb, struct net_device *dev, u32 portid)
{
	u8 fport;
	fport = (portid == AIROHA_GDM3_IDX) ? FE_PSE_PORT_GDM3 : portid;
	return fport;
}

static int an7583_get_ports_id_by_desc(struct airoha_eth *eth, struct airoha_qdma_desc *desc)
{
	u32 port, sport, msg1 = le32_to_cpu(desc->msg1);

	sport = FIELD_GET(QDMA_ETH_RXMSG_SPORT_MASK, msg1);
	switch (sport) {
	case 0x16:
		port = AIROHA_PORTS_GDM3_ID; /* AN7583, ETH on GDM3 */
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

static int an7583_get_vip_port(int portid)
{
	u32 vip_port;
	switch (portid) {
	case AIROHA_GDM3_IDX:
		vip_port = XSI_ETH_VIP_PORT_MASK;
		break;
	case AIROHA_GDM4_IDX:
		vip_port = XSI_PCIE0_VIP_PORT_MASK;
		break;
	default:
		return 0;
	}
	return vip_port;
}


static void an7583_set_ppe_dft_cport(struct airoha_eth *eth, int port_id, int qdma_id)
{
	u8 dft_port;
	u32 shift = port_id * 4;
	
	if(qdma_id == 0)
		dft_port = FE_PSE_PORT_CDM1;
	else
		dft_port = FE_PSE_PORT_CDM2;
	airoha_fe_rmw(eth,REG_PPE1_DFT_CPORT_0, 0xF << shift, dft_port << shift);
}

static void an7583_get_gdm_loopback_info(int portid, u32 *chan, u32 *pse_port)
{
	
	*chan = (portid == AIROHA_GDM3_IDX) ? 3 : 0;
	*pse_port = (portid == AIROHA_GDM3_IDX) ? FE_PSE_PORT_GDM3 : FE_PSE_PORT_GDM4;
}

void an7583_set_sp_dft_cport(struct airoha_eth *eth, int portid)
{
    unsigned char srcport_idx = eth->chip->dma_srcport[portid];

    switch(portid){
    case AIROHA_GDM3_IDX:
		airoha_fe_rmw(eth, REG_SRC_PORT_FC_MAP6,
			      FC_ID_OF_SRC_PORT24_MASK,
			      FIELD_PREP(FC_ID_OF_SRC_PORT24_MASK, 2));
		airoha_fe_rmw(eth, REG_FE_WAN_PORT,
			      WAN1_EN_MASK | WAN1_MASK | WAN0_MASK,
			      FIELD_PREP(WAN0_MASK, srcport_idx));
		airoha_fe_rmw(eth, REG_SP_DFT_CPORT(srcport_idx >> 3),
			      SP_CPORT_ETH_MASK,
			      FIELD_PREP(SP_CPORT_ETH_MASK, FE_PSE_PORT_CDM2));
        break;
	default:
		break;
    }
}

static int an7583_support_eth_monitor(int p)
{
	/*gdm2 and gdm4 not detect, skip*/
	if(p == AIROHA_PORTS_GDM2_ID || p == AIROHA_PORTS_GDM4_ID)
		return 0;
	return 1;
}

static inline int xsi_check_index_valid(uint hsgmii_index)
{
	if(hsgmii_index >= SERDES_MAX_IDX)
		return 0;
	else 
		return 1 ;

}

static int xsi_mac_set_ipg(struct airoha_gdm_port *port, uint hsgmii_index, uint ipg)
{
	if(0 == xsi_check_index_valid(hsgmii_index))
		return -1;

	if(port->xfi_mac){
		regmap_update_bits(port->xfi_mac, XSI_GLB_CFG,XSI_GLB_CFG_F_IPG_NUM_MASK,ipg << XSI_GLB_CFG_F_IPG_NUM_SHIFT);
	}
	return 0;
}

int xsi_mac_set_tx_rx_frag_len(struct airoha_gdm_port *port, uint hsgmii_index, uint frag_len)
{
	if(0 == xsi_check_index_valid(hsgmii_index))
		return -1;
	
	if(port->xfi_mac){
		regmap_update_bits(port->xfi_mac, XSI_GLB_CFG,XSI_GLB_CFG_F_TX_FRAG_LEN_MASK,frag_len << XSI_GLB_CFG_F_TX_FRAG_LEN_SHIFT);
		regmap_update_bits(port->xfi_mac, XSI_GLB_CFG,XSI_GLB_CFG_F_RX_FRAG_LEN_MASK,frag_len << XSI_GLB_CFG_F_RX_FRAG_LEN_SHIFT);
	}
	return 0;
}

int xsi_mac_set_tx_rx_fc_en(struct airoha_gdm_port *port, uint hsgmii_index)
{
	if(0 == xsi_check_index_valid(hsgmii_index))
		return -1;
	
	if(port->xfi_mac){
		regmap_set_bits(port->xfi_mac, XSI_GLB_CFG,XSI_GLB_CFG_F_TX_FC_EN);
		regmap_set_bits(port->xfi_mac, XSI_GLB_CFG,XSI_GLB_CFG_F_RX_FC_EN);
	}
	return 0;
}

static int an7583_channel_retire(struct airoha_eth *eth, struct airoha_gdm_port *port)
{

	int ret = 0;
	struct ecnt_fe_data fe_data = {0};
	FE_Gdma_Sel_t gdm_idx;
	u8 retire_channle, hsgmii_index;

	/*Only support gdm3 channel retire for 7583 */
	if(port->id != AIROHA_GDM3_IDX){
		return -1;
	}
	hsgmii_index = SERDES_ETH_IDX;
	
	gdm_idx = GET_XSI_GDM(eth,hsgmii_index);
	retire_channle = GET_XSI_RETIRE_CHANNEL(eth,hsgmii_index);

	fe_data.gdm_sel = gdm_idx;
	fe_data.channel = retire_channle;
	fe_api_set_channel_retire_one(&fe_data);
	mdelay(1);

	xsi_mac_set_ipg(port,hsgmii_index, 10);

	//usxgmii mode disable tx_rx frag & use fe frag
	xsi_mac_set_tx_rx_frag_len(port,hsgmii_index,4);
	
	xsi_mac_set_tx_rx_fc_en(port,hsgmii_index);
	return ret;
}

void an7583_qdma_regs_setting(struct airoha_qdma *qdma, struct airoha_eth *eth)
{
	int mask = 0;
	int val = 0;
	
	int id = qdma - &eth->qdma[0];

	//set qdma multi issue
	mask = GLOBAL_CFG_RD_BYPASS_WR_MASK | GLOBAL_CFG_MAX_ISSUE_NUM_MASK;
	val = GLOBAL_CFG_RD_BYPASS_WR_MASK | FIELD_PREP(GLOBAL_CFG_MAX_ISSUE_NUM_MASK, 3);
	airoha_qdma_rmw(qdma, REG_QDMA_GLOBAL_CFG,mask,val);

	if(id == 0)
	{
		//set qdma egress ratemeter cfg
		mask = (EGRESS_RATE_METER_EQ_RATE_EN_MASK | EGRESS_RATE_METER_WINDOW_SZ_MASK | EGRESS_RATE_METER_TIMESLICE_MASK);
		val = (FIELD_PREP(EGRESS_RATE_METER_WINDOW_SZ_MASK, 1) | FIELD_PREP(EGRESS_RATE_METER_TIMESLICE_MASK, 0x740));
		airoha_qdma_rmw(qdma, REG_EGRESS_RATE_METER_CFG,mask,val);
	}

	mask = (TXQ_CNGST_TXQ_TOTAL_MAX_THR_MASK | TXQ_CNGST_TXQ_TOTAL_MIN_THR_MASK);
	val = (FIELD_PREP(TXQ_CNGST_TXQ_TOTAL_MAX_THR_MASK, BUFF_FAST_TOTAL_MAX_THRH) 
		| FIELD_PREP(TXQ_CNGST_TXQ_TOTAL_MIN_THR_MASK, BUFF_FAST_TOTAL_MIN_THRH));
	airoha_qdma_rmw(qdma, REG_QDMA_TXQ_TOTAL_FAST_THR,mask,val);
	
	return;
}

void an7583_set_qbi_fttr_chn_disable(struct airoha_eth *eth)
{
    airoha_fe_wr(eth, REG_QBI_FTTR_CHANNEL_CFG, 0);

    return;
}

void an7583_fe_regs_setting(struct airoha_eth *eth)
{
	//set cdm2 faq 
	airoha_fe_wr(eth, REG_FAQ_CFG(1),0x07e6);
	airoha_fe_wr(eth, REG_FAQTHR_CFG(1),0xc40003f0);
	airoha_fe_set(eth, REG_FAQ_CFG(1), FAQ_EN_MASK);	
		
	return;
}

static const char * const an7583_xsi_rsts_names[] = {
	"hsi0-mac",
	"hsi1-mac",
	"xfp-mac",
};

const struct airoha_eth_chip chip_an7583_info = {
    .id = ID_AN7583,
    .ppe_num=1,
    .dma_srcport = {
        [SERDES_PCIE0_IDX] = 0x18,
        [SERDES_PCIE1_IDX] = 0x18,
        [SERDES_ETH_IDX]   = 0x16,
        [SERDES_USB_IDX]   = 0x19,
    },
	.fport = {
		[SERDES_PCIE0_IDX] = FE_PSE_PORT_GDM4,
        [SERDES_PCIE1_IDX] = FE_PSE_PORT_GDM4,
        [SERDES_ETH_IDX]   = FE_PSE_PORT_GDM3,
        [SERDES_USB_IDX]   = FE_PSE_PORT_GDM4,
	},
	.nbq = {
		[SERDES_PCIE0_IDX] = 0,
        [SERDES_PCIE1_IDX] = 0,
        [SERDES_ETH_IDX]   = 0,
        [SERDES_USB_IDX]   = 1,
	},
	.chnl = {
		[SERDES_PCIE0_IDX] = 10,
        [SERDES_PCIE1_IDX] = 11,
        [SERDES_ETH_IDX]   = 13,
        [SERDES_USB_IDX]   = 12,
	},
	.retire_channel = {
		[SERDES_PCIE0_IDX] = 0,
        [SERDES_PCIE1_IDX] = 0,
        [SERDES_ETH_IDX]   = 0,
        [SERDES_USB_IDX]   = 1,
	},
	.gdm = {
		[SERDES_PCIE0_IDX] = FE_GDM_SEL_GDMA4,
        [SERDES_PCIE1_IDX] = FE_GDM_SEL_GDMA4,
        [SERDES_ETH_IDX]   = FE_GDM_SEL_GDMA3,
        [SERDES_USB_IDX]   = FE_GDM_SEL_GDMA4,
	},
    .xsi_rsts_names = an7583_xsi_rsts_names,
    .num_xsi_rsts = ARRAY_SIZE(an7583_xsi_rsts_names),
    .get_fe_fport = an7583_get_fe_fport,
    .get_ports_id = an7583_get_ports_id_by_desc,
    .get_vip_port = an7583_get_vip_port,
    .set_ppe_dft_cport = an7583_set_ppe_dft_cport,
    .get_gdm_lpbk_info = an7583_get_gdm_loopback_info,
    .set_sp_dft_cport = an7583_set_sp_dft_cport,
	.support_eth_monitor = an7583_support_eth_monitor,
	.set_channel_retire = an7583_channel_retire,
	.set_qdma_regs = an7583_qdma_regs_setting,
	.set_qbi_fttr_chn = an7583_set_qbi_fttr_chn_disable,
	.set_fe_regs = an7583_fe_regs_setting,
};