// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 AIROHA Inc
 * Author:  2024 AIROHA Inc
 */



#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

extern int doPhyChkVal(int argc, char *argv[], void *p);
extern int doPhySwVer(int argc, char *argv[], void *p);
extern int doPhyMiiRead(int argc, char *argv[], void *p);
extern int doPhyMiiWrite(int argc, char *argv[], void *p);
extern int doPhyMMDRead(int argc, char *argv[], void *p);
extern int doPhyMMDWrite(int argc, char *argv[], void *p);
extern int doPhyMiiRead_TrDbg(int argc, char *argv[], void *p);
extern int doPhyMiiWrite_TrDbg(int argc, char *argv[], void *p);
extern int doPhyGphyTestMode(int argc, char *argv[], void *p);
extern int doReadAllCalData(int argc, char *argv[], void *p);
extern int sw_ability_setting(int argc, char *argv[], void *p);
extern int doPhySwPatch(int argc, char *argv[], void *p);
extern int doPhyForceMode(int argc, char *argv[], void *p);
extern int doPhyForceEEE(int argc, char *argv[], void *p);
extern int doPhygetLinkRate(int argc, char *argv[], void *p);
extern int doPhygetDuplex(int argc, char *argv[], void *p);
extern int doPhyChkErrFlag(int argc, char *argv[], void *p);
extern int doPhyClrErrFlag(int argc, char *argv[], void *p);

/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/
#define BUF_SIZE 128
static struct dentry *ephy_dir, *ephy_file;

void call_function(int argc, char *argv[], void *p)
{
	mdelay(10);
	
	if ((strcmp(argv[0], "chk") == 0) || (strcmp(argv[0], "chkval") == 0))
	{
		doPhyChkVal(argc, argv, p);
	}
	else if ((strcmp(argv[0], "ver") == 0))
	{
		doPhySwVer(argc, argv, p);
	}
	else if ((strcmp(argv[0], "miir") == 0))
	{
		doPhyMiiRead(argc, argv, p);
	}
	else if ((strcmp(argv[0], "miiw") == 0))
	{
		doPhyMiiWrite(argc, argv, p);
	}
	else if ((strcmp(argv[0], "emiir") == 0))
	{
		doPhyMMDRead(argc, argv, p);
	}
	else if ((strcmp(argv[0], "emiiw") == 0))
	{
		doPhyMMDWrite(argc, argv, p);
	}
	else if ((strcmp(argv[0], "miir_trdbg") == 0))
	{
		doPhyMiiRead_TrDbg(argc, argv, p);
	}
	else if ((strcmp(argv[0], "miiw_trdbg") == 0))
	{
		doPhyMiiWrite_TrDbg(argc, argv, p);
	}
	else if ((strcmp(argv[0], "testmode") == 0))
	{
		doPhyGphyTestMode(argc, argv, p);
	}
	else if ((strcmp(argv[0], "cal_all_data") == 0))
	{
		doReadAllCalData(argc, argv, p);
	}
	else if ((strcmp(argv[0], "sw_ability") == 0))
	{
		sw_ability_setting(argc, argv, p);
	}
	else if ((strcmp(argv[0], "swpatch") == 0))
	{
		doPhySwPatch(argc, argv, p);
	}
	else if ((strcmp(argv[0], "forcemode") == 0))
	{
		doPhyForceMode(argc, argv, p);
	}
	else if ((strcasecmp(argv[0], "forceEEE") == 0))  //strcasecmp use to ignore input upper case/lower case letter 
	{
		doPhyForceEEE(argc, argv, p);
	}
	else if ((strcasecmp(argv[0], "getLinkRate") == 0))
	{
		doPhygetLinkRate(argc, argv, p);
	}
	else if ((strcasecmp(argv[0], "getDuplex") == 0)) 
	{
		doPhygetDuplex(argc, argv, p);
	}
	else if ((strcasecmp(argv[0], "err_flag") == 0)) 
	{
		doPhyChkErrFlag(argc, argv, p);
	}	
	else if ((strcasecmp(argv[0], "clr_err_flag") == 0)) 
	{
		doPhyClrErrFlag(argc, argv, p);
	}	
	else
	{
		printk("Command not support\r\n");
		printk("Support command [chk] [ver] [miir] [miiw] [emiir] [emiiw] [miir_trdbg] [miiw_trdbg] \n \
			   [testmode] [cal_all_data] [sw_ability] [swpatch] [forcemode] [forceEEE] [getLinkRate] [getDuplex] \r\n");
	}
	
}

static ssize_t ephy_debugfs_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
	
	char buf[BUF_SIZE];
	char *argv[8];
	int argc = 0;
	char *token, *cur;
	char local_buf[BUF_SIZE];
	void *p;

	if (count >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, ubuf, count))
		return -EFAULT;

	buf[count] = '\0';

	strlcpy(local_buf, buf, sizeof(local_buf));
	cur = local_buf;

	while ((token = strsep(&cur, " \n")) != NULL && argc < 8)
	{
		if (*token == '\0')
			continue;
		argv[argc++] = token;
	}

	/*
	printk("argc = %d\n", argc);
	for (int i = 0; i < argc; i++)
	{
		printk("argv[%d] = %s\n", i, argv[i]);
	}
	*/

	call_function(argc, argv, p);


	return count;
}

static const struct file_operations ephy_debugfs_fops = {
	.write = ephy_debugfs_write,
};

int ephy_debugfs_init(void)
{
	printk("EPHY debugfs Init\r\n");
	ephy_dir = debugfs_create_dir("ephy", NULL);
	if (!ephy_dir)
	{
		printk("Failed to create ephy debugfs directory\r\n");
		return -ENOMEM;
	}

	ephy_file = debugfs_create_file("tce", 0644, ephy_dir, NULL, &ephy_debugfs_fops);
	if (!ephy_file)
	{
		printk("Failed to create ephy debugfs file\r\n");
		return -ENOMEM;
	}

	return 0;
}


void ephy_debugfs_exit(void)
{
	debugfs_remove_recursive(ephy_dir);
	printk("EPHY debugfs unloaded\r\n");

}





