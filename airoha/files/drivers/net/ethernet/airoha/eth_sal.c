#include "airoha_eth.h"
#include "eth_sal.h"

bool airoha_check_eth_info(struct airoha_eth *eth)
{
	if(!eth->chip->get_fe_fport){
		printk("get_fe_fport not defined.\n");
		return false;
	}
	if(!eth->chip->get_ports_id){
		printk("get_ports_id not defined.\n");
		return false;
	}
	if(!eth->chip->get_vip_port){
		printk("get_vip_port not defined.\n");
		return false;
	}
	if(!eth->chip->get_gdm_lpbk_info){
		printk("get_gdm_lpbk_info not defined.\n");
		return false;
	}
	return true;
}