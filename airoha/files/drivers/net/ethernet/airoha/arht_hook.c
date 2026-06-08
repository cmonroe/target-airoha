/*
 *  General Airoha Hook driver
 *
 *  Copyright (C) 2024 Zhengping Zhang <nbd@nbd.name>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <net/protocol.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/wait.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/if.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <asm/cacheflush.h>
#include <net/net_namespace.h>
#include <net/sock.h>
#include <arht_hook/arht_hook.h>

#include <linux/arht_kernel.h>

struct list_head ecnt_hooks[ECNT_NUM_MAINTYPE][ECNT_MAX_SUBTYPE];
EXPORT_SYMBOL(ecnt_hooks);

static DEFINE_SPINLOCK(ecnt_hook_lock);
unsigned int hook_id = 0;

static inline ecnt_ret_val ecnt_iterate(struct list_head *head, struct ecnt_hook_ops **elemp, struct ecnt_data *in_data)
{
	ecnt_ret_val verdict = ECNT_CONTINUE;

	/*
	 * The caller must not block between calls to this
	 * function because of risk of continuing from deleted element.
	 */
	//list_for_each_continue_rcu(*i, head) {
	list_for_each_entry_continue_rcu((*elemp), head, list){
		if((*elemp)->is_execute){
			verdict = (*elemp)->hookfn(in_data);
			if (verdict == ECNT_RETURN_DROP) {
					(*elemp)->info.drop_num++;
					return verdict;
			}
			if(verdict == ECNT_RETURN)
				return verdict;
		}
	}

	return verdict;
}


__IMEM ecnt_ret_val __ECNT_HOOK(unsigned int maintype, unsigned int subtype,struct ecnt_data *in_data)
{
	ecnt_ret_val ret;
	struct ecnt_hook_ops *elem;
	struct list_head* ptr = &ecnt_hooks[maintype][subtype];
	
	if((maintype >= ECNT_NUM_MAINTYPE) || (subtype >= ECNT_MAX_SUBTYPE)){
		printk("__ECNT_HOOK fail, max maintype %d, max subtype %d\n", ECNT_NUM_MAINTYPE, ECNT_MAX_SUBTYPE);
		return ECNT_HOOK_ERROR;
	}
	if (list_empty(&ecnt_hooks[maintype][subtype])){
		return ECNT_HOOK_ERROR;
	}
	
	/* We may already have this, but read-locks nest anyway */
	rcu_read_lock();

	elem = list_entry_rcu(ptr, struct ecnt_hook_ops, list);	
	/* We may already have this, but read-locks nest anyway */
	ret = ecnt_iterate(&ecnt_hooks[maintype][subtype], &elem, in_data);
	rcu_read_unlock();
	
	return ret;
}
EXPORT_SYMBOL(__ECNT_HOOK);

__IMEM ecnt_ret_val __ARHT_HOOK(unsigned int maintype, unsigned int subtype,struct ecnt_data *in_data)
{
	return __ECNT_HOOK(maintype, subtype, in_data);
}
EXPORT_SYMBOL(__ARHT_HOOK);

int set_ecnt_hookfn_execute_or_not(unsigned int maintype, unsigned int subtype, unsigned int hook_id, unsigned int is_execute)
{
	//struct list_head *pos;
	struct list_head* ptr = &ecnt_hooks[maintype][subtype];
	
	if((maintype >= ECNT_NUM_MAINTYPE) || (subtype >= ECNT_MAX_SUBTYPE)){
		printk("set_ecnt_hookfn_execute_or_not fail, max maintype %d, max subtype %d\n", ECNT_NUM_MAINTYPE, ECNT_MAX_SUBTYPE);
		return 0;
	}
	if(list_empty(&ecnt_hooks[maintype][subtype]))
		return 0;
	
	struct ecnt_hook_ops *elem;
	rcu_read_lock();
	elem = list_entry_rcu(ptr, struct ecnt_hook_ops, list);
	//list_for_each_continue_rcu(pos, &ecnt_hooks[maintype][subtype]){
	list_for_each_entry_continue_rcu(elem, &ecnt_hooks[maintype][subtype], list){
		if(elem->hook_id == hook_id){
			elem->is_execute = is_execute;
			rcu_read_unlock();
			return 1;
		}
	}
	rcu_read_unlock();

	return 0;
}
EXPORT_SYMBOL(set_ecnt_hookfn_execute_or_not);

int get_ecnt_hookfn(unsigned int maintype, unsigned int subtype){
	struct list_head *pos;
	struct ecnt_hook_ops *elem;
	int index = 1;
	
	if((maintype >= ECNT_NUM_MAINTYPE) || (subtype >= ECNT_MAX_SUBTYPE)){
		printk("get_ecnt_hookfn fail, max maintype %d, max subtype %d\n", ECNT_NUM_MAINTYPE, ECNT_MAX_SUBTYPE);
		return 0;
	}
	if(list_empty(&ecnt_hooks[maintype][subtype])){
		printk("maintype = %d, subtype=%d, 0 hook functions\n",maintype, subtype);
		return 0;
	}
	
	printk("maintype = %d, subtype=%d\n",maintype, subtype);
	printk("index\t[id]\tis_exe\tpri\tdropnum\tname\n");
	rcu_read_lock();
	
	list_for_each(pos, &ecnt_hooks[maintype][subtype]){
		elem = (struct ecnt_hook_ops *)pos;
		printk("%d.\t[%d]\t%d\t%d\t%d\t%s\n", 
			index++, elem->hook_id, elem->is_execute, elem->priority,elem->info.drop_num,elem->name);
			
	}
	rcu_read_unlock();
	
	return 1;
}
EXPORT_SYMBOL(get_ecnt_hookfn);

int show_all_ecnt_hookfn(void){
	int maintype, subtype;
	struct list_head *pos;
	struct ecnt_hook_ops *elem;
	int index = 1;

	printk("index\t[main-sub]\t[id]\tis_exe\tpri\tdropnum\tname\n");
	rcu_read_lock();
	for(maintype = 0; maintype < ECNT_NUM_MAINTYPE; maintype++){
		for(subtype = 0; subtype < ECNT_MAX_SUBTYPE; subtype++){
			list_for_each(pos, &ecnt_hooks[maintype][subtype]){
				elem = (struct ecnt_hook_ops *)pos;
				printk("%d.\t[%d-%d]\t\t[%d]\t%d\t%d\t%d\t%s\n", 
					index++, maintype, subtype,elem->hook_id, elem->is_execute, elem->priority,elem->info.drop_num,elem->name);
			}
		}
	}
	rcu_read_unlock();
	
	return 1;
}
EXPORT_SYMBOL(show_all_ecnt_hookfn);

int ecnt_register_hook(struct ecnt_hook_ops *reg)
{
	struct ecnt_hook_ops *elem;
	if(!reg){
		printk("ecnt_register_hook fail, reg is NULL\n");
		return ECNT_REGISTER_FAIL;
	}
	if((reg->maintype >= ECNT_NUM_MAINTYPE) || (reg->subtype >= ECNT_MAX_SUBTYPE)){
		printk("ecnt_register_hook fail, maintype = %d, subtype=%d, out of range\n", reg->maintype, reg->subtype);
		return ECNT_REGISTER_FAIL;
	}
	if(reg->list.next!= NULL){
		printk("ecnt_register_hook fail, %s already registered\n", reg->name);
		return ECNT_REGISTER_FAIL;
	}
	spin_lock(&ecnt_hook_lock);
	if(hook_id >= 0xFFFFFFFF){
		spin_unlock(&ecnt_hook_lock);
		printk("ecnt_register_hook fail, hook_id out of range\n");
		return ECNT_REGISTER_FAIL;
	}
	list_for_each_entry(elem, &ecnt_hooks[reg->maintype][reg->subtype], list) {
		if (reg->priority < elem->priority){
			break;
		}
	}
	reg->hook_id = ++hook_id;
	reg->info.drop_num = 0;
	list_add_rcu(&reg->list, elem->list.prev);
	spin_unlock(&ecnt_hook_lock);

	return ECNT_REGISTER_SUCCESS;
}
EXPORT_SYMBOL(ecnt_register_hook);

void ecnt_unregister_hook(struct ecnt_hook_ops *reg)
{
	if(!reg){
		printk("ecnt_unregister_hook fail, reg is NULL\n");
		return;
	}
	if(reg->list.prev == LIST_POISON2){
		printk("%s already unregistered\n", reg->name);
		return;
	}
	spin_lock(&ecnt_hook_lock);
	list_del_rcu(&reg->list);
	spin_unlock(&ecnt_hook_lock);
	synchronize_rcu();
}
EXPORT_SYMBOL(ecnt_unregister_hook);

int ecnt_ops_unregister(unsigned int maintype, unsigned int subtype, unsigned int hook_id)
{
	struct ecnt_hook_ops *elem;
	struct list_head* ptr = &ecnt_hooks[maintype][subtype];
	if((maintype >= ECNT_NUM_MAINTYPE) || (subtype >= ECNT_MAX_SUBTYPE)){
		printk("set_ecnt_hookfn_execute_or_not fail, max maintype %d, max subtype %d\n", ECNT_NUM_MAINTYPE, ECNT_MAX_SUBTYPE);
		return 0;
	}
	if(list_empty(&ecnt_hooks[maintype][subtype]))
		return 0;
	
	rcu_read_lock();	
	elem = list_entry_rcu(ptr, struct ecnt_hook_ops, list);
	//list_for_each_continue_rcu(pos, &ecnt_hooks[maintype][subtype]){
	list_for_each_entry_continue_rcu(elem, &ecnt_hooks[maintype][subtype], list){
		if(elem->hook_id == hook_id){
			ecnt_unregister_hook(elem);
			rcu_read_unlock();
			return 1;
		}
	}
	rcu_read_unlock();

	return 0;
}
EXPORT_SYMBOL(ecnt_ops_unregister);

int ecnt_register_hooks(struct ecnt_hook_ops *reg, unsigned int n)
{
	unsigned int i;
	int err = 0;

	for (i = 0; i < n; i++) {
		err = ecnt_register_hook(&reg[i]);
		if (err)
			goto err;
	}
	return err;

err:
	if (i > 0)
		ecnt_unregister_hooks(reg, i);
	return err;
}
EXPORT_SYMBOL(ecnt_register_hooks);

void ecnt_unregister_hooks(struct ecnt_hook_ops *reg, unsigned int n)
{
	unsigned int i;

	for (i = 0; i < n; i++)
		ecnt_unregister_hook(&reg[i]);
}
EXPORT_SYMBOL(ecnt_unregister_hooks);


// void ecnt_dcache_inv(void *addr, size_t size)
// {
// 	dcache_inval_poc((unsigned long)addr, (unsigned long)addr + size);
// }
// EXPORT_SYMBOL(ecnt_dcache_inv);
// 
// void ecnt_dcache_wback_inv(void *addr, size_t size)
// {
// 	dcache_clean_inval_poc((unsigned long)addr, (unsigned long)addr + size);
// }
// EXPORT_SYMBOL(ecnt_dcache_wback_inv);

// int (*airoha_ethwan_rx_hook)(struct sk_buff *skb) = NULL;
// EXPORT_SYMBOL(airoha_ethwan_rx_hook);


static int __init arht_hook_init(void)
{
	int i, h;
	
	printk("arht-hook: module init");
	for (i = 0; i < ARRAY_SIZE(ecnt_hooks); i++){
		for (h = 0; h < ECNT_MAX_SUBTYPE; h++)
			INIT_LIST_HEAD(&ecnt_hooks[i][h]);
	}
	return 0;
}

static void __exit arht_hook_exit(void)
{
	int i, h;
	
	printk("arht-hook: module exit");
	for (i = 0; i < ARRAY_SIZE(ecnt_hooks); i++){
		for (h = 0; h < ECNT_MAX_SUBTYPE; h++)
			INIT_LIST_HEAD(&ecnt_hooks[i][h]);
	}
}

#if 0
module_init(arht_hook_init);
module_exit(arht_hook_exit);

MODULE_AUTHOR("Zhengping Zhang <nbd@nbd.name>");
MODULE_DESCRIPTION("General Airoha Hook driver");
MODULE_LICENSE("GPL v2");
#endif

core_initcall(arht_hook_init);


