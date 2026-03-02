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
#include "arht_dp_api.h"
#include "airoha_eth.h"
#include "airoha_regs.h"

/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/
extern struct airoha_eth *glb_eth;
extern int airoha_dp_api_qdma_get_meter_value(struct airoha_qdma *qdma,uint meterIdx);
extern int airoha_dp_api_qdma_set_meter_value(struct airoha_qdma *qdma,uint meterIdx,uint value);
extern int packet_is_transparent_mode;

ssize_t CHECK_BUF(char *debugfs_buffer, int index, int buf_size)
{
	if(index >= buf_size -1)
	{
		buf_size*=2;
		debugfs_buffer=kmalloc(buf_size,GFP_KERNEL);
		
		if(!debugfs_buffer)
			return -ENOMEM;
		
	}
	return buf_size;
}
static ssize_t airoha_rxring_limit_value_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char input_str[64];
	char fuc_str[16];
	int ret = 0;
	unsigned int qdma_idx, ring_idx, value = 0;;
	struct airoha_eth *eth = glb_eth;

	if (count >= sizeof(input_str))
		return -EINVAL;
	if (copy_from_user(input_str, buf, count))
		return -EFAULT;
	input_str[count] = '\0';
	if(sscanf(input_str, "%s %u %u %u ",fuc_str, &qdma_idx, &ring_idx, &value) < 2){
		printk("\n Help: [set/get] [qdma_idx:0/1][ring_idx] [value] \n");
		return -EINVAL;
	}
	if(qdma_idx > 1 || ring_idx >= AIROHA_NUM_RX_RING)
		return -EINVAL;
	
	if(!strcmp(fuc_str,"set"))
		ret = airoha_dp_api_qdma_set_meter_value(&eth->qdma[qdma_idx],ring_idx,value);
	else if(!strcmp(fuc_str,"get")){
		ret = airoha_dp_api_qdma_get_meter_value(&eth->qdma[qdma_idx],ring_idx);
		printk("\n qdma idx:%u ring_idx:%u rxring_value:%u \n",qdma_idx,ring_idx,ret);
	}
	else{
		printk("\n Help: [set/get] [qdma_idx:0/1][ring_idx] [value] \n");
		return -EINVAL;
	}
	
	if(ret < 0)
		return -EINVAL;
		
	return count;
	
   
}

static ssize_t airoha_gemport_ratelimit_value_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char input_str[64];
	int ret = 0;
	char fuc_str[16];
	unsigned int dir, gemport_idx, meter_idx, value = 0;
	struct airoha_eth *eth = glb_eth;

	if (count >= sizeof(input_str))
		return -EINVAL;
	if (copy_from_user(input_str, buf, count))
		return -EFAULT;
	input_str[count] = '\0';
	if(sscanf(input_str, "%s %u %u %u ",fuc_str, &dir, &gemport_idx, &value) < 3){
		printk("\n Help: [set/get] [dir:0(DS)/1(US)][gemport_idx] [value] \n");
		return -EINVAL;
	}
	
  
	if (dir <= 1 && gemport_idx <= AIROHA_MAX_IDX_FOR_GEMPORT_RATELIMIT && gemport_idx >= AIROHA_MIN_IDX_FOR_GEMPORT_RATELIMIT)
		meter_idx += AIROHA_NUM_RX_RING;
	else{
		printk("\n Error: dir should be betweeen 0 - 1 Index should be betweeen 0 - 31 \n");
		return -EINVAL;
	}
	if(!strcmp(fuc_str,"set"))
		ret = airoha_dp_api_qdma_set_meter_value(&eth->qdma[dir],meter_idx,value);
	else if(!strcmp(fuc_str,"get")){
		ret = airoha_dp_api_qdma_get_meter_value(&eth->qdma[dir],meter_idx);
		printk("\n dir:%u gemport_idx:%u value:%u \n",dir,gemport_idx,ret);
		}
	else{
		printk("\n Help: [set/get] [dir:0/1][gemport_idx] [value] \n");
		return -EINVAL;
	}
	
	if(ret < 0)
		return -EINVAL;
		
	return count;
	
}


static ssize_t airoha_fe_debug_reg_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int index = 0;
    int i = 0;
	ssize_t buf_size=4086;
	ssize_t ret = 0;
	struct airoha_eth *eth = glb_eth;
	char *debugfs_buffer;
	debugfs_buffer=kmalloc(buf_size,GFP_KERNEL);
   
	if(!debugfs_buffer)
		return -ENOMEM;

    index += sprintf(debugfs_buffer+index, "PSE_DROP_CNT:\n");
    
    for(i = 0; i < PSE_PORT_NUM; i++) 
    {
        index += sprintf(debugfs_buffer+index, "P%i:0x%08x  ", i, airoha_fe_rr(eth,REG_FE_PSE_DROP_CNT(i)));
        buf_size=CHECK_BUF(debugfs_buffer,index,buf_size);
        if( (i%4) == 3 ){
            index += sprintf(debugfs_buffer+index, "\n");
            
        }
    }

    index += sprintf(debugfs_buffer+index, "\nPSE_IQ_CNT:    P0:0x%02x  P1:0x%02x  P2:0x%02x  P3:0x%02x  P4:0x%02x  P5:0x%02x\n"
        , ((airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA0)>>16) & 0x7fff), ((airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA1)>>16) & 0x7fff)
        , ((airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA2)>>16) & 0x7fff), ((airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA3)>>16) & 0x7fff)
        , ((airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA4)>>16) & 0x7fff), ((airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA5)>>16) & 0x7fff));
    buf_size=CHECK_BUF(debugfs_buffer,index,buf_size);
	
	index += sprintf(debugfs_buffer+index, "               P6:0x%02x  P7:0x%02x  P8:0x%02x  P9:0x%02x\n"
        , ((airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA6)>>16) & 0x7fff), ((airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA7)>>16) & 0x7fff)
        , ((airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA8)>>16) & 0x7fff), ((airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA9)>>16) & 0x7fff));
    
    index += sprintf(debugfs_buffer+index, "PSE_OQ_CNT:    P0:0x%02x  P1:0x%02x  P2:0x%02x  P3:0x%02x  P4:0x%02x  P5:0x%02x\n"
        , (airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA0) & 0x7fff), (airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA1) & 0x7fff)
        , (airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA2) & 0x7fff), (airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA3) & 0x7fff)
        , (airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA4) & 0x7fff), (airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA5) & 0x7fff));
    buf_size=CHECK_BUF(debugfs_buffer,index,buf_size);
	index += sprintf(debugfs_buffer+index, "               P6:0x%02x  P7:0x%02x  P8:0x%02x  P9:0x%02x\n"
        , (airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA6) & 0x7fff), (airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA7) & 0x7fff)
        , (airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA8) & 0x7fff), (airoha_fe_rr(eth,REG_FE_PSE_PORT_Q_USE_STA9) & 0x7fff));
    buf_size=CHECK_BUF(debugfs_buffer,index,buf_size);
    index += sprintf(debugfs_buffer+index, "PSE_SHARE_BUF:  SHARED_USED_CNT:0x%04x  SHARED_FREE_CNT:0x%04x\n"
        , ((airoha_fe_rr(eth,REG_FE_PSE_SHARE_BUF_STA)>>16) & 0x7fff), (airoha_fe_rr(eth,REG_FE_PSE_SHARE_BUF_STA) & 0x7fff));
    buf_size=CHECK_BUF(debugfs_buffer,index,buf_size);

//FE CNT //BROWN



	index += sprintf(debugfs_buffer+index, "\nCDMA1_TX_OK_CNT           (0x%08x) = 0x%08x\n", CDMA1_TX_OK_CNT, airoha_fe_rr(eth,CDMA1_TX_OK_CNT));
	
	index += sprintf(debugfs_buffer+index, "CDMA1_RXCPU_OK_CNT        (0x%08x) = 0x%08x\n", CDMA1_RXCPU_OK_CNT, airoha_fe_rr(eth,CDMA1_RXCPU_OK_CNT));
	
	index += sprintf(debugfs_buffer+index, "CDMA1_RXHWF_OK_CNT        (0x%08x) = 0x%08x\n", CDMA1_RXHWF_OK_CNT, airoha_fe_rr(eth,CDMA1_RXHWF_OK_CNT));
	

	index += sprintf(debugfs_buffer+index, "CDMA1_RXHWF_FAST_OK_CNT   (0x%08x) = 0x%08x\n", CDMA1_RXHWF_FAST_ALL_CNT, airoha_fe_rr(eth,CDMA1_RXHWF_FAST_ALL_CNT));
	

	index += sprintf(debugfs_buffer+index, "CDMA1_RXCPU_DROP_CNT      (0x%08x) = 0x%08x\n", CDMA1_RXCPU_DROP_CNT, airoha_fe_rr(eth,CDMA1_RXCPU_DROP_CNT));
	
	index += sprintf(debugfs_buffer+index, "CDMA1_RXHWF_DROP_CNT      (0x%08x) = 0x%08x\n", CDMA1_RXHWF_DROP_CNT, airoha_fe_rr(eth,CDMA1_RXHWF_DROP_CNT));
	buf_size=CHECK_BUF(debugfs_buffer,index,buf_size);

	index += sprintf(debugfs_buffer+index, "CDMA1_RXHWF_FAST_DROP_CNT (0x%08x) = 0x%08x\n", CDMA1_RXHWF_FAST_DROP_CNT, airoha_fe_rr(eth,CDMA1_RXHWF_FAST_DROP_CNT));
	



	index += sprintf(debugfs_buffer+index, "\nGDMA1_TX_GET_CNT          (0x%08x) = 0x%08x\n", REG_FE_GDM_TX_GET_PKT_CNT(1) , airoha_fe_rr(eth,REG_FE_GDM_TX_GET_PKT_CNT(1)));
	
	index += sprintf(debugfs_buffer+index, "GDMA1_TX_OK_CNT_L         (0x%08x) = 0x%08x\n", REG_FE_GDM_TX_OK_PKT_CNT_L(1) , airoha_fe_rr(eth,REG_FE_GDM_TX_OK_PKT_CNT_L(1)));
	
	index += sprintf(debugfs_buffer+index, "GDMA1_TX_OK_CNT_H         (0x%08x) = 0x%08x\n", REG_FE_GDM_TX_OK_PKT_CNT_H(1), airoha_fe_rr(eth,REG_FE_GDM_TX_OK_PKT_CNT_H(1)));
	buf_size=CHECK_BUF(debugfs_buffer,index,buf_size);

	index += sprintf(debugfs_buffer+index, "GDMA1_TX_DROP_CNT         (0x%08x) = 0x%08x\n", REG_FE_GDM_TX_ETH_DROP_CNT(1), airoha_fe_rr(eth,REG_FE_GDM_TX_ETH_DROP_CNT(1)));
	
	index += sprintf(debugfs_buffer+index, "GDMA1_RX_OK_CNT           (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_OK_PKT_CNT_L(1) , airoha_fe_rr(eth,REG_FE_GDM_RX_OK_PKT_CNT_L(1)));
	
	index += sprintf(debugfs_buffer+index, "GDMA1_RX_FC_DROP_CNT      (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_FC_DROP_CNT(1), airoha_fe_rr(eth,REG_FE_GDM_RX_FC_DROP_CNT(1)));
	
	index += sprintf(debugfs_buffer+index, "GDMA1_RX_RC_DROP_CNT      (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_RC_DROP_CNT(1), airoha_fe_rr(eth,REG_FE_GDM_RX_RC_DROP_CNT(1)));
	
	index += sprintf(debugfs_buffer+index, "GDMA1_RX_OVER_DROP_CNT    (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_OVERFLOW_DROP_CNT(1), airoha_fe_rr(eth,REG_FE_GDM_RX_OVERFLOW_DROP_CNT(1)));
	
	index += sprintf(debugfs_buffer+index, "GDMA1_RX_ERROR_DROP_CNT   (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_ERROR_DROP_CNT(1), airoha_fe_rr(eth,REG_FE_GDM_RX_ERROR_DROP_CNT(1)));
	
	buf_size=CHECK_BUF(debugfs_buffer,index,buf_size);
    
	index += sprintf(debugfs_buffer+index, "\nCDMA2_TX_OK_CNT           (0x%08x) = 0x%08x\n", CDMA2_TX_OK_CNT, airoha_fe_rr(eth,CDMA2_TX_OK_CNT));
	
	index += sprintf(debugfs_buffer+index, "CDMA2_RXCPU_OK_CNT        (0x%08x) = 0x%08x\n", CDMA2_RXCPU_OK_CNT, airoha_fe_rr(eth,CDMA2_RXCPU_OK_CNT));
	
	index += sprintf(debugfs_buffer+index, "CDMA2_RXHWF_OK_CNT        (0x%08x) = 0x%08x\n", CDMA2_RXHWF_OK_CNT, airoha_fe_rr(eth,CDMA2_RXHWF_OK_CNT));
	

	index += sprintf(debugfs_buffer+index, "CDMA2_RXHWF_FAST_OK_CNT   (0x%08x) = 0x%08x\n", CDMA2_RXHWF_FAST_ALL_CNT, airoha_fe_rr(eth,CDMA2_RXHWF_FAST_ALL_CNT));
	

	index += sprintf(debugfs_buffer+index, "CDMA2_RXCPU_DROP_CNT      (0x%08x) = 0x%08x\n", CDMA2_RXCPU_DROP_CNT, airoha_fe_rr(eth,CDMA2_RXCPU_DROP_CNT));
	
	index += sprintf(debugfs_buffer+index, "CDMA2_RXHWF_DROP_CNT      (0x%08x) = 0x%08x\n", CDMA2_RXHWF_DROP_CNT, airoha_fe_rr(eth,CDMA2_RXHWF_DROP_CNT));
	buf_size=CHECK_BUF(debugfs_buffer,index,buf_size);

	index += sprintf(debugfs_buffer+index, "CDMA2_RXHWF_FAST_DROP_CNT (0x%08x) = 0x%08x\n", CDMA2_RXHWF_FAST_DROP_CNT, airoha_fe_rr(eth,CDMA2_RXHWF_FAST_DROP_CNT));
	


//GDM2
	index += sprintf(debugfs_buffer+index, "\nGDMA2_TX_GET_CNT          (0x%08x) = 0x%08x\n", REG_FE_GDM_TX_GET_PKT_CNT(2) , airoha_fe_rr(eth,REG_FE_GDM_TX_GET_PKT_CNT(2)));
	
	index += sprintf(debugfs_buffer+index, "GDMA2_TX_OK_CNT_L         (0x%08x) = 0x%08x\n", REG_FE_GDM_TX_OK_PKT_CNT_L(2) , airoha_fe_rr(eth,REG_FE_GDM_TX_OK_PKT_CNT_L(2)));
	
	index += sprintf(debugfs_buffer+index, "GDMA2_TX_OK_CNT_H         (0x%08x) = 0x%08x\n", REG_FE_GDM_TX_OK_PKT_CNT_H(2), airoha_fe_rr(eth,REG_FE_GDM_TX_OK_PKT_CNT_H(2)));
	buf_size=CHECK_BUF(debugfs_buffer,index,buf_size);

	index += sprintf(debugfs_buffer+index, "GDMA2_TX_DROP_CNT         (0x%08x) = 0x%08x\n", REG_FE_GDM_TX_ETH_DROP_CNT(2), airoha_fe_rr(eth,REG_FE_GDM_TX_ETH_DROP_CNT(2)));
	
	index += sprintf(debugfs_buffer+index, "GDMA2_RX_OK_CNT           (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_OK_PKT_CNT_L(2) , airoha_fe_rr(eth,REG_FE_GDM_RX_OK_PKT_CNT_L(2)));
	
	index += sprintf(debugfs_buffer+index, "GDMA2_RX_FC_DROP_CNT      (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_FC_DROP_CNT(2), airoha_fe_rr(eth,REG_FE_GDM_RX_FC_DROP_CNT(2)));
	
	index += sprintf(debugfs_buffer+index, "GDMA2_RX_RC_DROP_CNT      (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_RC_DROP_CNT(2), airoha_fe_rr(eth,REG_FE_GDM_RX_RC_DROP_CNT(2)));
	
	index += sprintf(debugfs_buffer+index, "GDMA2_RX_OVER_DROP_CNT    (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_OVERFLOW_DROP_CNT(2), airoha_fe_rr(eth,REG_FE_GDM_RX_OVERFLOW_DROP_CNT(2)));
	
	index += sprintf(debugfs_buffer+index, "GDMA2_RX_ERROR_DROP_CNT   (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_ERROR_DROP_CNT(2), airoha_fe_rr(eth,REG_FE_GDM_RX_ERROR_DROP_CNT(2)));
	buf_size=CHECK_BUF(debugfs_buffer,index,buf_size);


//GDM3
		
	buf_size=CHECK_BUF(debugfs_buffer,index,buf_size);
	
	index += sprintf(debugfs_buffer+index, "\nGDMA3_TX_GET_CNT          (0x%08x) = 0x%08x\n", REG_FE_GDM_TX_GET_PKT_CNT(3) , airoha_fe_rr(eth,REG_FE_GDM_TX_GET_PKT_CNT(3)));
	
	index += sprintf(debugfs_buffer+index, "GDMA3_TX_OK_CNT_L         (0x%08x) = 0x%08x\n", REG_FE_GDM_TX_OK_PKT_CNT_L(3) , airoha_fe_rr(eth,REG_FE_GDM_TX_OK_PKT_CNT_L(3)));
	
	index += sprintf(debugfs_buffer+index, "GDMA3_TX_OK_CNT_H         (0x%08x) = 0x%08x\n", REG_FE_GDM_TX_OK_PKT_CNT_H(3), airoha_fe_rr(eth,REG_FE_GDM_TX_OK_PKT_CNT_H(3)));
	buf_size=CHECK_BUF(debugfs_buffer,index,buf_size);

	index += sprintf(debugfs_buffer+index, "GDMA3_TX_DROP_CNT         (0x%08x) = 0x%08x\n", REG_FE_GDM_TX_ETH_DROP_CNT(3), airoha_fe_rr(eth,REG_FE_GDM_TX_ETH_DROP_CNT(3)));
	
	index += sprintf(debugfs_buffer+index, "GDMA3_RX_OK_CNT           (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_OK_PKT_CNT_L(3) , airoha_fe_rr(eth,REG_FE_GDM_RX_OK_PKT_CNT_L(3)));
	
	index += sprintf(debugfs_buffer+index, "GDMA3_RX_FC_DROP_CNT      (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_FC_DROP_CNT(3), airoha_fe_rr(eth,REG_FE_GDM_RX_FC_DROP_CNT(3)));
	
	index += sprintf(debugfs_buffer+index, "GDMA3_RX_RC_DROP_CNT      (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_RC_DROP_CNT(3), airoha_fe_rr(eth,REG_FE_GDM_RX_RC_DROP_CNT(3)));
	
	index += sprintf(debugfs_buffer+index, "GDMA2_RX_OVER_DROP_CNT    (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_OVERFLOW_DROP_CNT(3), airoha_fe_rr(eth,REG_FE_GDM_RX_OVERFLOW_DROP_CNT(3)));
	
	index += sprintf(debugfs_buffer+index, "GDMA3_RX_ERROR_DROP_CNT   (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_ERROR_DROP_CNT(3), airoha_fe_rr(eth,REG_FE_GDM_RX_ERROR_DROP_CNT(3)));
	buf_size=CHECK_BUF(debugfs_buffer,index,buf_size);

//GDM4
	
	buf_size=CHECK_BUF(debugfs_buffer,index,buf_size);
	index += sprintf(debugfs_buffer+index, "\nGDMA4_TX_GET_CNT          (0x%08x) = 0x%08x\n", REG_FE_GDM_TX_GET_PKT_CNT(4) , airoha_fe_rr(eth,REG_FE_GDM_TX_GET_PKT_CNT(4)));
	
	index += sprintf(debugfs_buffer+index, "GDMA4_TX_OK_CNT_L         (0x%08x) = 0x%08x\n", REG_FE_GDM_TX_OK_PKT_CNT_L(4) , airoha_fe_rr(eth,REG_FE_GDM_TX_OK_PKT_CNT_L(4)));
	
	index += sprintf(debugfs_buffer+index, "GDMA4_TX_OK_CNT_H         (0x%08x) = 0x%08x\n", REG_FE_GDM_TX_OK_PKT_CNT_H(4), airoha_fe_rr(eth,REG_FE_GDM_TX_OK_PKT_CNT_H(4)));
	buf_size=CHECK_BUF(debugfs_buffer,index,buf_size);

	index += sprintf(debugfs_buffer+index, "GDMA4_TX_DROP_CNT         (0x%08x) = 0x%08x\n", REG_FE_GDM_TX_ETH_DROP_CNT(4), airoha_fe_rr(eth,REG_FE_GDM_TX_ETH_DROP_CNT(4)));
	
	index += sprintf(debugfs_buffer+index, "GDMA4_RX_OK_CNT           (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_OK_PKT_CNT_L(4) , airoha_fe_rr(eth,REG_FE_GDM_RX_OK_PKT_CNT_L(4)));
	
	index += sprintf(debugfs_buffer+index, "GDMA4_RX_FC_DROP_CNT      (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_FC_DROP_CNT(4), airoha_fe_rr(eth,REG_FE_GDM_RX_FC_DROP_CNT(4)));
	
	index += sprintf(debugfs_buffer+index, "GDMA4_RX_RC_DROP_CNT      (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_RC_DROP_CNT(4), airoha_fe_rr(eth,REG_FE_GDM_RX_RC_DROP_CNT(4)));
	
	index += sprintf(debugfs_buffer+index, "GDMA4_RX_OVER_DROP_CNT    (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_OVERFLOW_DROP_CNT(4), airoha_fe_rr(eth,REG_FE_GDM_RX_OVERFLOW_DROP_CNT(4)));
	
	index += sprintf(debugfs_buffer+index, "GDMA4_RX_ERROR_DROP_CNT   (0x%08x) = 0x%08x\n", REG_FE_GDM_RX_ERROR_DROP_CNT(4), airoha_fe_rr(eth,REG_FE_GDM_RX_ERROR_DROP_CNT(4)));
	
	
	ret = simple_read_from_buffer(buf, count, ppos, debugfs_buffer,index);
	kfree(debugfs_buffer);
	
    return ret;
}

static ssize_t airoha_packet_transparent_mode_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char input_str[64];
	char fuc_str[16];
	int ret = 0;

	if (count >= sizeof(input_str))
		return -EINVAL;
	if (copy_from_user(input_str, buf, count))
		return -EFAULT;
	input_str[count] = '\0';
	if(sscanf(input_str, "%s",fuc_str) < 1){
		printk("\n Help: echo [enable/disable] \n");
		return -EINVAL;
	}
	
	if(!strcmp(fuc_str,"enable")){
		packet_is_transparent_mode = 1;
		ret = 0;
		printk("\n packet_is_transparent_mode:%d \n",packet_is_transparent_mode);
	}
	else if(!strcmp(fuc_str,"disable")){
		packet_is_transparent_mode = 0;
		ret = 0;
		printk("\n packet_is_transparent_mode:%d \n",packet_is_transparent_mode);
	}
	else{
		printk("\n Help: [set/get] [qdma_idx:0/1][ring_idx] [value] \n");
		return -EINVAL;
	}
	
	if(ret < 0)
		return -EINVAL;
		
	return count;
	
   
}

static ssize_t airoha_packet_transparent_mode_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char level_str[64];
    snprintf(level_str, sizeof(level_str), "packet_is_transparent_mode enable:%d\n", packet_is_transparent_mode);
    return simple_read_from_buffer(buf, count, ppos, level_str, sizeof(level_str));
}

static void airoha_debugfs_print_tuple(struct seq_file *m,
					   void *src_addr, void *dest_addr,
					   u16 *src_port, u16 *dest_port,
					   bool ipv6)
{
	__be32 n_addr[IPV6_ADDR_WORDS];

	if (ipv6) {
		ipv6_addr_cpu_to_be32(n_addr, src_addr);
		seq_printf(m, "%pI6", n_addr);
	} else {
		seq_printf(m, "%pI4h", src_addr);
	}
	if (src_port)
		seq_printf(m, ":%d", *src_port);

	seq_puts(m, "->");

	if (ipv6) {
		ipv6_addr_cpu_to_be32(n_addr, dest_addr);
		seq_printf(m, "%pI6", n_addr);
	} else {
		seq_printf(m, "%pI4h", dest_addr);
	}
	if (dest_port)
		seq_printf(m, ":%d", *dest_port);
}

static int airoha_ppe_debugfs_foe_flow_show(struct seq_file *m, void *private)
{
	struct airoha_flow_table_entry *e;
	struct hlist_node *n;
	struct airoha_ppe *ppe = glb_eth->ppe;
	int i;

	static const char *const ppe_type_str[] = {
		[PPE_PKT_TYPE_IPV4_HNAPT] = "IPv4 5T",
		[PPE_PKT_TYPE_IPV4_ROUTE] = "IPv4 3T",
		[PPE_PKT_TYPE_BRIDGE] = "L2B",
		[PPE_PKT_TYPE_IPV4_DSLITE] = "DS-LITE",
		[PPE_PKT_TYPE_IPV6_ROUTE_3T] = "IPv6 3T",
		[PPE_PKT_TYPE_IPV6_ROUTE_5T] = "IPv6 5T",
		[PPE_PKT_TYPE_IPV6_6RD] = "6RD",
	};

	static const char *const ppe_magic[] = {
		[PON] = "PON",
		[ETH2] = "ETH2",
		[LAN1] = "LAN1",
		[LAN2] = "LAN2",
		[LAN3] = "LAN3",
		[LAN4] = "LAN4",
	};

	static const char *const ppe_type[] = {
		[FLOW_TYPE_L4] = "FLOW_TYPE_L4",
		[FLOW_TYPE_L2] = "FLOW_TYPE_L2",
		[FLOW_TYPE_L2_SUBFLOW] = "FLOW_TYPE_L2_SUBFLOW",
	};

	for (i = 0; i < PPE_NUM_ENTRIES; i++) 
	{
		hlist_for_each_entry_safe(e, n, &ppe->foe_flow[i], list) 
		{
			const char *type_str = "UNKNOWN";
			void *src_addr = NULL, *dest_addr = NULL;
			u16 *src_port = NULL, *dest_port = NULL;
			bool ipv6 = false;
			
			int type = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, e->data.ib1);
			if (type < ARRAY_SIZE(ppe_type_str) && ppe_type_str[type])
				type_str = ppe_type_str[type];
			seq_printf(m, "hash 0x%04x: %7s", e->hash, type_str);
			
			switch (type) {
    			case PPE_PKT_TYPE_IPV4_HNAPT:
    			case PPE_PKT_TYPE_IPV4_DSLITE:
    				src_port = &e->data.ipv4.orig_tuple.src_port;
    				dest_port = &e->data.ipv4.orig_tuple.dest_port;
    				fallthrough;
    			case PPE_PKT_TYPE_IPV4_ROUTE:
    				src_addr = &e->data.ipv4.orig_tuple.src_ip;
    				dest_addr = &e->data.ipv4.orig_tuple.dest_ip;
    				break;
    			case PPE_PKT_TYPE_IPV6_ROUTE_5T:
    				src_port = &e->data.ipv6.src_port;
    				dest_port = &e->data.ipv6.dest_port;
    				fallthrough;
    			case PPE_PKT_TYPE_IPV6_ROUTE_3T:
    			case PPE_PKT_TYPE_IPV6_6RD:
    				src_addr = &e->data.ipv6.src_ip;
    				dest_addr = &e->data.ipv6.dest_ip;
    				ipv6 = true;
    				break;
    			default:
    				break;
			}

			if (src_addr && dest_addr) {
				seq_puts(m, " orig=");
				airoha_debugfs_print_tuple(m, src_addr, dest_addr,
							       src_port, dest_port, ipv6);
			}

			switch (type) {
    			case PPE_PKT_TYPE_IPV4_HNAPT:
    			case PPE_PKT_TYPE_IPV4_DSLITE:
    				src_port = &e->data.ipv4.new_tuple.src_port;
    				dest_port = &e->data.ipv4.new_tuple.dest_port;
    				fallthrough;
    			case PPE_PKT_TYPE_IPV4_ROUTE:
    				src_addr = &e->data.ipv4.new_tuple.src_ip;
    				dest_addr = &e->data.ipv4.new_tuple.dest_ip;
    				seq_puts(m, " new=");
    				airoha_debugfs_print_tuple(m, src_addr, dest_addr,
    							       src_port, dest_port, ipv6);
    				break;
    			default:
    				break;
			}
			seq_printf(m, "  %s e_magic:%4s tx_modified:%d ingress_dev_idx:%u\n", 
				ppe_type[e->type], ppe_magic[e->e_magic], e->tx_modified, e->ingress_dev_idx);
		}
	}

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(airoha_ppe_debugfs_foe_flow);

static const struct file_operations airoha_dp_api_debugfs_rxring_limit_value_fops = {
    .write = airoha_rxring_limit_value_write,
};

static const struct file_operations airoha_dp_api_debugfs_gemport_ratelimit_value_fops = {
    .write = airoha_gemport_ratelimit_value_write,
};


static const struct file_operations airoha_dp_api_debugfs_fe_debug_reg_fops = {
    .read = airoha_fe_debug_reg_read,
};

static const struct file_operations airoha_dp_api_debugfs_packet_transparent_mode_fops = {
    .write = airoha_packet_transparent_mode_write,
	.read = airoha_packet_transparent_mode_read,
};

int airoha_dp_api_debugfs_init(struct airoha_eth *eth)
{
	struct dentry *root;

	root = debugfs_create_dir("dp_api", eth->debugfs_dir);
	debugfs_create_file("rxring_limit_value", 0644, root, eth,
			    &airoha_dp_api_debugfs_rxring_limit_value_fops);
	debugfs_create_file("gemport_ratelimit", 0644, root, eth,
			    &airoha_dp_api_debugfs_gemport_ratelimit_value_fops);					
	debugfs_create_file("fe_debug_reg", 0644, root, eth,
			    &airoha_dp_api_debugfs_fe_debug_reg_fops);		
	debugfs_create_file("packet_transparent_mode", 0644, root, NULL,
                &airoha_dp_api_debugfs_packet_transparent_mode_fops);				
    debugfs_create_file("foe_flow", 0444, root, eth,
                &airoha_ppe_debugfs_foe_flow_fops);

	return 0;
}

