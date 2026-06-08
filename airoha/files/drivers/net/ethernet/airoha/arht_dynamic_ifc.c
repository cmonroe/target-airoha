/*
 *  General Airoha dynamic ifc driver
 *
 *  Copyright (C) 2025 
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/skbuff.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <net/inet_hashtables.h>
#include <net/protocol.h>
#include <net/dst.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <arht_hook/ecnt_hook_gen_offload.h>
#include "airoha_eth.h"
#include "arht_dynamic_ifc.h"

/************************************************************************************************
*				   E X T E R N A L     D A T A	 D E C L A R A T I O N S
*************************************************************************************************
*/
extern struct airoha_eth *glb_eth;


/************************************************************************************************
*				   E X T E R N A L     F U N C T I O N	 D E C L A R A T I O N S
*************************************************************************************************
*/
extern int (*offload_eth_fast_tx_hook)(struct sk_buff *skb, int channel);
extern int (*local_out_pingpong_hook)(struct sk_buff*);
extern struct dst_entry *arht_gen_dst_clone(struct dst_entry *dst);
extern u32 get_frame_engine_data(u32 reg);
extern void set_frame_engine_data(u32 reg, u32 val);
extern int (*arht_hook_get_crsn) (struct sk_buff * skb);
extern int arht_skip_copy_kprobe_enable(void);
extern void arht_skip_copy_kprobe_disable(void);
extern int (*ra_sw_nat_hook_clean_entry_by_port)(u16 src_port, u16 dest_port);
/************************************************************************************************
*				   
*************************************************************************************************
*/
static int ifc_enable = 0;
static int ifc_ring_reserve[IFC_LRO_RING_NUM];

static u32 lro_agg_num_orig_cdm1 = 0;
static u32 lro_agg_num_orig_cdm2 = 0;
static bool lro_agg_num_saved = false;

static dynamic_ifc dynamic_ifc_ety[MAX_DYNAMIC_IFC_NUM];

char dynamic_ifc_apps[MAX_DYNAMIC_IFC_APP_NUM][32] = {0};
static int dynamic_ifc_app_num = 0;

DEFINE_SPINLOCK(dynamic_ifc_lock);

static int ifc_get_free_lro_ring(int entry_idx)
{
    int i;

    for (i = 0; i < IFC_LRO_RING_NUM; i++) 
	{
        if (ifc_ring_reserve[i] == entry_idx){
            return (IFC_LRO_RING_START + i);
        }

        if (ifc_ring_reserve[i] != IFC_RING_SLOT_FREE) 
		{
            int prev = ifc_ring_reserve[i];
            if (prev >= MAX_DYNAMIC_IFC_NUM ||
                !dynamic_ifc_ety[prev].valid ||
                !dynamic_ifc_ety[prev].ifc_valid) 
            {
                ifc_ring_reserve[i] = entry_idx;
                return (IFC_LRO_RING_START + i);
            }
        }
    }

    for (i = 0; i < IFC_LRO_RING_NUM; i++) 
	{
        if (ifc_ring_reserve[i] == IFC_RING_SLOT_FREE) 
		{
            ifc_ring_reserve[i] = entry_idx;
            return (IFC_LRO_RING_START + i);
        }
    }

    return IFC_DEFAULT_RING;
}

/* Release ring reservation */
static void ifc_release_lro_ring(int entry_idx)
{
    int i;

    for (i = 0; i < IFC_LRO_RING_NUM; i++) 
	{
        if (ifc_ring_reserve[i] == entry_idx) 
		{
            ifc_ring_reserve[i] = IFC_RING_SLOT_FREE;
            return;
        }
    }
}

/* Create IFC rule for a data session. First 4 data sessions get ring 12~15, rest get ring 1 (default). */
static int dynamic_ifc_add_ifc(dynamic_ifc *lo)
{
	struct ecnt_ifc_param ifc_param;

	if (!ifc_enable || lo->ifc_valid){
		return 0;
	}

	memset(&ifc_param, 0, sizeof(struct ecnt_ifc_param));

	/* DPORT = local port (iperf3 server side) */
	ifc_param.field[0] = DPORT;
	ifc_param.mask[0]  = 0xFFFF;
	ifc_param.key[0]   = ntohs(lo->lport);

	/* SPORT = remote port (iperf3 client side) */
	ifc_param.field[1] = SPORT;
	ifc_param.mask[1]  = 0xFFFF;
	ifc_param.key[1]   = ntohs(lo->rport);

	lo->ifc_index = IFC_API_SET_LUT_RULE_AUTO(&ifc_param);
	if (lo->ifc_index == 0 || lo->ifc_index == ECNT_HOOK_ERROR) 
	{
		pr_warn("dynamic_ifc: IFC add failed lport=%u rport=%u\n", ntohs(lo->lport), ntohs(lo->rport));
		lo->ifc_index = 0;
		return -1;
	}

	lo->ring_id = ifc_get_free_lro_ring(lo - dynamic_ifc_ety);

	/* actIdx=14 (ACT_ForceCPU): force to CPU, specify ring */
	IFC_API_SET_ACTION(lo->ifc_index, 14, IFC_ENABLE, 1, lo->ring_id, 0, 0);

	lo->ifc_valid = 1;

	pr_info("dynamic_ifc: IFC added idx=%u lport=%u rport=%u -> ring %d\n",
		lo->ifc_index, ntohs(lo->lport), ntohs(lo->rport), lo->ring_id);

	return 0;
}

/* Delete IFC rule for a session. */
static void dynamic_ifc_del_ifc(dynamic_ifc *lo)
{
	struct ecnt_ifc_param ifc_param;

	if (!lo->ifc_valid || lo->ifc_index == 0){
		return;
	}

	memset(&ifc_param, 0, sizeof(struct ecnt_ifc_param));
	ifc_param.field[0]   = DPORT;
	ifc_param.command[0] = 0;
	ifc_param.mask[0]    = 0xFFFF;
	ifc_param.key[0]     = ntohs(lo->lport);
	ifc_param.field[1]   = SPORT;
	ifc_param.command[1] = 0;
	ifc_param.mask[1]    = 0xFFFF;
	ifc_param.key[1]     = ntohs(lo->rport);

	IFC_API_DEL_LUT_RULE_AUTO(&ifc_param);

	pr_info("dynamic_ifc: IFC deleted idx=%u lport=%u rport=%u ring=%d\n",
		lo->ifc_index, ntohs(lo->lport), ntohs(lo->rport), lo->ring_id);

	lo->ifc_index = 0;
	lo->ifc_valid = 0;
	lo->ring_id = 0;

	ifc_release_lro_ring(lo - dynamic_ifc_ety);
}

static void lro_save_and_set_agg_num(int agg_num)
{
	u32 val;

	if (lro_agg_num_saved) {
		return;
	}

	val = get_frame_engine_data(PHY_REG_CDM1_LRO_LIMIT);
	lro_agg_num_orig_cdm1 = (val & CDM_LRO_AGG_NUM_MASK_VAL) >> CDM_LRO_AGG_NUM_SHIFT;

	val = get_frame_engine_data(PHY_REG_CDM2_LRO_LIMIT);
	lro_agg_num_orig_cdm2 = (val & CDM_LRO_AGG_NUM_MASK_VAL) >> CDM_LRO_AGG_NUM_SHIFT;

	lro_agg_num_saved = true;

	val = get_frame_engine_data(PHY_REG_CDM1_LRO_LIMIT);
	val &= ~CDM_LRO_AGG_NUM_MASK_VAL;
	val |= ((u32)agg_num << CDM_LRO_AGG_NUM_SHIFT) & CDM_LRO_AGG_NUM_MASK_VAL;
	set_frame_engine_data(PHY_REG_CDM1_LRO_LIMIT, val);

	val = get_frame_engine_data(PHY_REG_CDM2_LRO_LIMIT);
	val &= ~CDM_LRO_AGG_NUM_MASK_VAL;
	val |= ((u32)agg_num << CDM_LRO_AGG_NUM_SHIFT) & CDM_LRO_AGG_NUM_MASK_VAL;
	set_frame_engine_data(PHY_REG_CDM2_LRO_LIMIT, val);
}

static void lro_restore_agg_num(void)
{
	u32 val;
	
	if (!lro_agg_num_saved) {
		return;
	}

	val = get_frame_engine_data(PHY_REG_CDM1_LRO_LIMIT);
	val &= ~CDM_LRO_AGG_NUM_MASK_VAL;
	val |= (lro_agg_num_orig_cdm1 << CDM_LRO_AGG_NUM_SHIFT) & CDM_LRO_AGG_NUM_MASK_VAL;
	set_frame_engine_data(PHY_REG_CDM1_LRO_LIMIT, val);

	val = get_frame_engine_data(PHY_REG_CDM2_LRO_LIMIT);
	val &= ~CDM_LRO_AGG_NUM_MASK_VAL;
	val |= (lro_agg_num_orig_cdm2 << CDM_LRO_AGG_NUM_SHIFT) & CDM_LRO_AGG_NUM_MASK_VAL;
	set_frame_engine_data(PHY_REG_CDM2_LRO_LIMIT, val);

	lro_agg_num_saved = false;
}

static int is_dynamic_ifc_app(char * name)
{
    int i;

    for (i = 0; i < MAX_DYNAMIC_IFC_APP_NUM; i++)
    {
        if (!strcmp(dynamic_ifc_apps[i], name))
            return 1;
    }

    return 0;
}

static int dynamic_ifc_app_list_add(char * name)
{
    int i;

	if(is_dynamic_ifc_app(name))
		return 1;
	
    for (i = 0; i < MAX_DYNAMIC_IFC_APP_NUM; i++)
    {
		if(dynamic_ifc_apps[i][0] == '\0')
        {
            strncpy(dynamic_ifc_apps[i], name, 31);
        	dynamic_ifc_apps[i][31] = '\0';
			dynamic_ifc_app_num++;
			return 1;
        }
    }

    return 0;
}

static int dynamic_ifc_app_list_del(char * name)
{
    int i;

    for (i = 0; i < MAX_DYNAMIC_IFC_APP_NUM; i++)
    {
        if (!strcmp(dynamic_ifc_apps[i], name))
            break;
    }

    if (i < MAX_DYNAMIC_IFC_APP_NUM)
    {
        dynamic_ifc_apps[i][0] = '\0';
		dynamic_ifc_app_num--;
        return 1;
    }

    return 0;
}

static dynamic_ifc* find_dynamic_ifc_ety_by_port(unsigned short lport, unsigned short rport)
{
	int i;

	for(i=0; i<MAX_DYNAMIC_IFC_NUM; i++)
	{
		if(dynamic_ifc_ety[i].valid == 0 || dynamic_ifc_ety[i].lport != lport || dynamic_ifc_ety[i].rport != rport)
		{
			continue;
		}
		return &dynamic_ifc_ety[i];
	}
	
	return NULL;
}

static void dynamic_ifc_timeout(struct timer_list *arg)
{
	dynamic_ifc *e = from_timer(e, arg, age_timer);
	int i;
	bool has_active_session = false;

	spin_lock_bh(&dynamic_ifc_lock);
	if(time_before(jiffies, e->last_tx+EXPIRE_TIME)) 
	{
		mod_timer(&e->age_timer, jiffies+EXPIRE_TIME);
		spin_unlock_bh(&dynamic_ifc_lock);
		return;
	}

	del_timer(&e->age_timer);

	if (e->ifc_valid) {
		dynamic_ifc_del_ifc(e);
		e->last_tx = jiffies;
	    mod_timer(&e->age_timer, jiffies + EXPIRE_TIME);
		spin_unlock_bh(&dynamic_ifc_lock);
	    return;
	}

	if (e->sk) {
		WRITE_ONCE(e->sk->sk_mark, 0);
		e->sk = NULL;
	}

	e->valid = 0;

	for (i = 0; i < MAX_DYNAMIC_IFC_NUM; i++) 
	{
		if (dynamic_ifc_ety[i].valid) {
			has_active_session = true;
			break;
		}
	}
	if (!has_active_session) {
		lro_restore_agg_num();
	}

	spin_unlock_bh(&dynamic_ifc_lock);

	return;
}

static void add_dynamic_ifc_ety(struct sk_buff *skb, struct tcphdr *th)
{
	int i;
	dynamic_ifc *lo;

	spin_lock_bh(&dynamic_ifc_lock);
	for(i = 0; i < MAX_DYNAMIC_IFC_NUM; i++)
	{
		lo = &dynamic_ifc_ety[i];
		if(!lo->valid){
			lo->lport= th->source;
			lo->rport= th->dest;
			lo->tx_dst = skb_dst(skb);
			lo->last_tx = jiffies;
			lo->valid = 1;
			
			lo->sk = skb->sk;
			lo->skip_copy = 0;

			lro_save_and_set_agg_num(LRO_AGG_NUM);

			timer_setup(&lo->age_timer, dynamic_ifc_timeout, 0);
			mod_timer(&lo->age_timer,jiffies + EXPIRE_TIME);	
			break;
		}
	}
	spin_unlock_bh(&dynamic_ifc_lock);

	return;
}

static struct tcphdr *get_tcp_header(struct sk_buff *skb, unsigned short protocol)
{
	if(protocol == htons(ETH_P_IPV6)){	
		struct ipv6hdr *ipv6h = ipv6_hdr(skb);
		if(ipv6h && ipv6h->nexthdr == IPPROTO_TCP){
			return (struct tcphdr*)((unsigned char*)ipv6h + 40);
		}
	}
	else if(protocol == htons(ETH_P_IP)){
		struct iphdr *iph = ip_hdr(skb);
		if(iph && iph->protocol == IPPROTO_TCP){
			return (struct tcphdr*)((unsigned char*)iph + (iph->ihl<<2));
		}
	}

	return NULL;
}

static dynamic_ifc* get_dynamic_ifc_ety(struct sk_buff *skb, unsigned short protocol, int out)
{
	struct tcphdr *th = get_tcp_header(skb, protocol);

	if(!th){
		return NULL;
	}

	if(out){
		return find_dynamic_ifc_ety_by_port(th->source,th->dest);
	}
	
	return find_dynamic_ifc_ety_by_port(th->dest,th->source);
}

static int local_out_pingpong(struct sk_buff *skb)
{
	dynamic_ifc* lo;
	const struct ethhdr *eth = (struct ethhdr *)skb_mac_header(skb);
	
	skb_reset_network_header(skb);
	lo = get_dynamic_ifc_ety(skb, eth->h_proto, 1);
	if(unlikely(!lo)){
		dev_kfree_skb(skb);
		return 0;
	}

	skb->inner_protocol = PPE_MAGIC_LOCAL_OUT;
	skb_dst_set(skb, arht_gen_dst_clone(lo->tx_dst));
	skb_dst(skb)->output(dev_net(skb_dst(skb)->dev), skb->sk, skb);

	return 0;
}

static unsigned int arht_dynamic_ifc_localin(void *priv, struct sk_buff *skb,
		const struct nf_hook_state *state)
{
	dynamic_ifc* lo = get_dynamic_ifc_ety(skb, skb->protocol, 0);

	if(!lo){
		return NF_ACCEPT;
	}

	lo->rx_dst = skb_dst(skb);
	lo->hash = FOE_ENTRY_NUM(skb);

	if (ifc_enable && !lo->ifc_valid) 
	{
		if (((skb->hash & PPE_CPU_MASK) >> PPE_CPU_REASON_BIT) != PPE_CPU_REASON_HIT_UNBIND_RATE_REACHED)
		{
			return NF_ACCEPT;
		}

		if (!lo->skip_copy && lo->sk) {
			lo->skip_copy = 1;
			WRITE_ONCE(lo->sk->sk_mark, SK_MARK_LOCAL_OFFLOAD);
		}
		dynamic_ifc_add_ifc(lo);
	}

	return NF_ACCEPT;
}

static uint8_t* airoha_get_macaddr(void)
{
    static uint8_t addr[6];
    u32 val, reg;

    reg = REG_FE_LAN_MAC_H;

    // Read the high part of the MAC address
	val = get_frame_engine_data(reg);
    addr[0] = (val >> 16) & 0xFF;
    addr[1] = (val >> 8) & 0xFF;
    addr[2] = val & 0xFF;

    // Read the low part of the MAC address
    val = get_frame_engine_data(REG_FE_MAC_LMIN(reg));
    addr[3] = (val >> 16) & 0xFF;
    addr[4] = (val >> 8) & 0xFF;
    addr[5] = val & 0xFF;
	
    return addr;
}

static unsigned int arht_dynamic_ifc_localout(void *priv, struct sk_buff *skb,
		const struct nf_hook_state *state)
{
	dynamic_ifc* lo;
	struct tcphdr *th;

	if(dynamic_ifc_app_num == 0){
		return NF_ACCEPT;
	}

	th = get_tcp_header(skb, skb->protocol);
	if(!th){
		return NF_ACCEPT;
	}
	
	lo = find_dynamic_ifc_ety_by_port(th->source,th->dest);
	if(lo){
		lo->last_tx = jiffies;
		if(likely(offload_eth_fast_tx_hook))
		{
			skb->inner_protocol = PPE_MAGIC_LOCAL_OUT;
			skb_push(skb, ETH_HLEN);
			memcpy(skb->data, airoha_get_macaddr(), ETH_ALEN);
			*(unsigned short *)(skb->data+12) = skb->protocol;
			offload_eth_fast_tx_hook(skb, 7);

			return NF_STOLEN;
		}
		return NF_ACCEPT;
	}
	else if(is_dynamic_ifc_app(current->comm)){
		add_dynamic_ifc_ety(skb, th);
	}

	return NF_ACCEPT;
}

static struct nf_hook_ops ipv4_local_in_hook = {
	.hook = arht_dynamic_ifc_localin,
	.pf = NFPROTO_IPV4,
	.hooknum = NF_INET_LOCAL_IN,
	.priority = NF_IP_PRI_FIRST,	
};

static struct nf_hook_ops ipv4_local_out_hook = {
	.hook = arht_dynamic_ifc_localout,
	.pf = NFPROTO_IPV4,
	.hooknum = NF_INET_LOCAL_OUT,
	.priority = NF_IP_PRI_FIRST,	
};

static struct nf_hook_ops ipv6_local_in_hook = {
	.hook = arht_dynamic_ifc_localin,
	.pf = NFPROTO_IPV6,
	.hooknum = NF_INET_LOCAL_IN,
	.priority = NF_IP6_PRI_FIRST,	
};

static struct nf_hook_ops ipv6_local_out_hook = {
	.hook = arht_dynamic_ifc_localout,
	.pf = NFPROTO_IPV6,
	.hooknum = NF_INET_LOCAL_OUT,
	.priority = NF_IP6_PRI_FIRST,	
};

static ssize_t dynamic_ifc_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len = 0, i;
	char pb[1024];
	dynamic_ifc *lo;

	if (*ppos > 0){
		return 0;
	}

	len += sprintf(pb+len,"Apps: ");
    for (i = 0; i < MAX_DYNAMIC_IFC_APP_NUM; i++)
    {
        len += sprintf(pb+len, "%s ", dynamic_ifc_apps[i]);
    }

	len += sprintf(pb+len,"\nentry: \n");
	for(i=0; i<MAX_DYNAMIC_IFC_NUM; i++)
	{
		lo = &dynamic_ifc_ety[i];
		if(!lo->valid)
			continue;
		len += sprintf(pb+len, "\t[%d] lport:%d rport:%d hash:%d time:%lums\n", 
			i, ntohs(lo->lport), ntohs(lo->rport), lo->hash, (jiffies-lo->last_tx)*10);
	}	
	len += sprintf(pb+len,"\n");

	if (count > len){
		count = len;
	}
	
	if (copy_to_user(buf, pb, count)){
		return -EFAULT;
	}
	
	*ppos += count;
	return count;	
}

static ssize_t dynamic_ifc_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char str[64] = {0};
	char cmd[32] = {0};
	char name[32] = {0};
	int val = 0, i;

	if (count > sizeof(str) - 1)
		return -EINVAL;

	if (copy_from_user(str, buf, count))
		return -EFAULT;

	str[count] = '\0';

	if (sscanf(str, "%s", cmd) < 1)
		return -EINVAL;

	if (!strcmp(cmd, "ifc")) {
		/* "ifc 1" or "ifc 0" - enable/disable IFC rule creation */
		if (sscanf(str, "%s %d", cmd, &val) == 2) {
			spin_lock_bh(&dynamic_ifc_lock);
			if (val) {
				ifc_enable = val;
			} else {
				ifc_enable = 0;
				/* Delete all existing IFC rules */
				for (i = 0; i < MAX_DYNAMIC_IFC_NUM; i++) {
					if (dynamic_ifc_ety[i].valid)
						dynamic_ifc_del_ifc(&dynamic_ifc_ety[i]);
				}
			}
			spin_unlock_bh(&dynamic_ifc_lock);
			pr_info("dynamic_ifc: ifc_enable = %d\n", ifc_enable);
		}
	} else {
		/* Original format: "1 iperf3" or "0 iperf3" - add/del app */
		val = 0;
		if (sscanf(str, "%d %s", &val, name) >= 2) {
			if (val)
				dynamic_ifc_app_list_add(name);
			else
				dynamic_ifc_app_list_del(name);
		}
	}
	
	return count;
}

static const struct proc_ops dynamic_ifc_fops = {
	.proc_read = dynamic_ifc_read_proc,
	.proc_write = dynamic_ifc_write_proc,
};

static int __init arht_dynamic_ifc_init(void)
{	
	int i;
	struct proc_dir_entry *proc;

	nf_register_net_hook(&init_net, &ipv4_local_in_hook);
	nf_register_net_hook(&init_net, &ipv4_local_out_hook);
	nf_register_net_hook(&init_net, &ipv6_local_in_hook);
	nf_register_net_hook(&init_net, &ipv6_local_out_hook);

	proc = proc_create("dynamic_ifc", 0, NULL, &dynamic_ifc_fops);

	rcu_assign_pointer(local_out_pingpong_hook, local_out_pingpong);
	arht_skip_copy_kprobe_enable();

	for (i = 0; i < IFC_LRO_RING_NUM; i++){
		ifc_ring_reserve[i] = IFC_RING_SLOT_FREE;
	}

	return 0;
}

static void __exit arht_dynamic_ifc_exit(void)
{
	int i;
	dynamic_ifc *lo;

	rcu_assign_pointer(local_out_pingpong_hook, NULL);

	arht_skip_copy_kprobe_disable();
	spin_lock_bh(&dynamic_ifc_lock);
	for (i = 0; i < MAX_DYNAMIC_IFC_NUM; i++) 
	{
		lo = &dynamic_ifc_ety[i];

		if (lo->valid)
			dynamic_ifc_del_ifc(&dynamic_ifc_ety[i]);

		if (lo->valid && lo->sk) {
			WRITE_ONCE(lo->sk->sk_mark, 0);
			lo->sk = NULL;
		}
	}
	spin_unlock_bh(&dynamic_ifc_lock);
		
	nf_unregister_net_hook(&init_net, &ipv4_local_in_hook);
	nf_unregister_net_hook(&init_net, &ipv4_local_out_hook);
	nf_unregister_net_hook(&init_net, &ipv6_local_in_hook);
	nf_unregister_net_hook(&init_net, &ipv6_local_out_hook);

	remove_proc_entry("dynamic_ifc", NULL);

	return;
}

late_initcall(arht_dynamic_ifc_init);


