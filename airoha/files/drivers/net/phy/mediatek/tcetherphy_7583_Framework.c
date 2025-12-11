/* //BBN_Linux/DEV/main/tclinux_phoenix/modules/private/tcphy/tcetherphy_7523.c */
/************************************************************************
*                E X T E R N A L   R E F E R E N C E S  (1)
**************************************************************************/

#include <linux/phy.h>
#include <linux/regmap.h>
#include <linux/mfd/syscon.h>

#ifdef LINUX_OS
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <asm/io.h>
//#include <asm/tc3162/tc3162.h>
//#include <asm/tc3162/ledcetrl.h>
//#include <asm/tc3162/TCIfSetQuery_os.h>
//#include <asm/tc3162/cmdparse.h>
#include <linux/mii.h>
//#include <../../../../../../../../../source/linux-arht-sdk/include/ecnt_hook/ecnt_hook_ephy.h>
//#include <../../../../../../../../../source/linux-arht-sdk/include/ecnt_hook/ecnt_hook.h>
#include "tcetherphy_hook.h"
#include "tcetherphy.h"
#include "tcetherphy_define.h"



#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_AUTOBENCH) 
#include <ecnt_hook/ecnt_hook.h>
#endif

#endif //LINUX_OS

extern void mtMiiRegWrite(int port_num, int reg_num, int reg_data);
extern u32 mtMiiRegRead(u8 port_num,u8 reg_num);
extern void mtEMiiRegWrite(u32 port_num, u32 dev_num, u32 reg_num, u32 reg_data);;
extern u32 mtEMiiRegRead(u32 port_num, u32 dev_num, u32 reg_num);;
extern int tcMiiStationWrite(u32 phy_addr, u32 phy_reg, u32 phy_data);
extern int tcMiiStationRead(u32 phy_addr, u32 phy_reg);
extern void toKenRingWrite(unsigned int phy, unsigned int ch, unsigned int node, unsigned int data_addr,unsigned int value);
extern unsigned int toKenRingRead(unsigned int phy, unsigned int ch, unsigned int node, unsigned int data_addr);

u8   tcSWVer_FRAMEWORK =2; 
extern u8 inital_finish_flag;

//struct ecnt_hook_ops ecnt_driver_ephy_op = {  // JasonG_7512
//    .name = "driver_ephy_hook",
//    .hookfn = ecnt_ephy_hook,
//    .maintype = ECNT_ETHER_PHY,
//    .is_execute = 1,
//    .subtype = ECNT_DRIVER_API,
//    .priority = 1
//};

/************************************************************************
*                        Data for EPHY  (3)
**************************************************************************/
#ifdef TCPHY_SUPPORT
int eco_rev = 0x00;
u8  tcPhyInitFlag = 0;

static u8  tcPhyFlag = 0;
static u8  tcPhyPortNum = 4; 		// set value in tcPhyVerLookUp
static u8  tcphy_link_state[TCPHY_PORTNUM];

#endif 			// TCPHY_SUPPORT

/************************************************************************
*            Variables for EPHY  (4)
**************************************************************************/
//*** TCPHY registers ***//
#ifdef TCPHY_SUPPORT

static u8 	current_idx = 0; 				// default 0, Do NOT change 

#endif 			// TCPHY_SUPPORT

#ifdef TCPHY_SUPPORT
void tcephydbgcmd(void);

#ifdef LINUX_OS
#ifdef TCPHY_SUPPORT
//extern int    	subcmd(const cmds_t tab[], int argc, char *argv[], void *p);
//extern int    	cmd_register(cmds_t *cmds_p);
//extern int doEtherPhydbg(int argc, char *argv[], void *p);
#endif
#endif //LINUX_OS

#endif

#ifdef TCPHY_SUPPORT
// variables for mii registers

// for multiple phy support
extern tcphy_mr1_reg_t 		Nmr1[TCPHY_PORTNUM];


#endif
extern u8    inital_finish_flag;
extern u8    slt_excuting;



/************************************************************************
*
*            Common Functions for EPHY  (5)
*
**************************************************************************/
extern  int tcPhyInit_patch(void);
int 	ePhyInit(u16 ephy_addr);



#if 1
/************************************************************************
*
*            Variables and Functions for 7523 GE PHY (ID=0x???)
*
**************************************************************************/
#ifdef LINUX_OS 
//const u32 en75xxGe_page_sel_addr = 31;
en75xxGe_cfg_data_t en75xxGe_cfg[EN75xxGe_PHY_INIT_SET_NUM]=
{
	{
		{"Ge_1.0"}, 
    	//local data	
     	{ 	
       		//{0x09, 0x0600},
       		//{0x1f, 0x0001},
       		//{0x14, 0x3a04},
       		//{0x1f, 0x0000}
     	},
     	
    	//per-port data 
	    { 
	 	}
    },
};

en75xxGe_cfg_cl45data_t en75xxGe_cfg_cl45[EN75xxGe_PHY_INIT_CL45_SET_NUM]=
{
    {	//globle data
     	{
			
			//{0x1f, 0x0044, 0x00a0},					// pair delay (digital)
			//{0x1f, 0x027c, 0x0808},					// 10 base Tx
			
			//{0x1f, 0x027b, 0x1177},					// disable 10 base-Te
			//{0x1f, 0x0417, 0x7775},
			
			//{0x1f, 0x0024, 0xc007},					// gphy led0
			//{0x1f, 0x0025, 0x003f},					// gphy led0  // 0x003f
			//{0x1f, 0x0026, 0xc007},					// gphy led1
			//{0x1f, 0x0027, 0x003f},					// gphy led1
			//{0x1f, 0x0021, 0x800a}					// gphy led
			
     	},
     	
     	//local data
     	{
			//{0x1e,0x0000,0x0190},					// 0->1, middle
			//{0x1e,0x0001,0x01c8},					// 0->1, ov1
			//{0x1e,0x0002,0x01c2},					// 0->1, ov2, z2p_ovs
			//{0x1e,0x0003,0x0100},					// 1->0, middle
			//{0x1e,0x0004,0x020d},					// 1->0, ov1
			//{0x1e,0x0005,0x020a},					// 1->0, ov2, p2z_ovs

			//{0x1e,0x0006,0x038f},					// 0->-1, middle
			//{0x1e,0x0007,0x03c8},					// 0->-1, ov1
			//{0x1e,0x0008,0x03c4},					// 0->-1, ov2, z2n_ovs
			//{0x1e,0x0009,0x0300},					// -1->0, middle
			//{0x1e,0x000a,0x0008},					// -1->0, ov1
			//{0x1e,0x000b,0x0002},					// -1->0, ov2, n2z_ovs
            //{0x1e,0x0011,0x0f00},
			
			//{0x1e,0x0013,0x0000},
			//{0x1e,0x0014,0x0000},					// pair delay (afe)
			//{0x1e,0x0044,0x0000},
			//{0x1e,0x0176,0x6600},					//tm4_gain
			//{0x1e,0x0177,0x0066},					//tm4_gain

			//{0x1e,0x0041,0x3333},					// hvga_bias : enhance rx voltage
			//{0x1e,0x0040,0x0000},					// gain down (dB); 0x00:0dB, 0x01:4dB  // allen_20180917 no rx gain down for IOT performance
			//{0x1e,0x0201,0x4000},					// txvld disable, 1e_201[7:0]=0, disable voltage mode when 100_EEE 
			//{0x1e,0x003d,0x0000},					// RX vbuffer bypass pwd
			//{0x1e,0x0198,0x0001},					// allen_20180530, TX vld bypass pwd
			//{0x1e,0x003e,0x0000},					// allen_20180530, TX vld bypass pwd (pair A/B for EEE)
			//{0x1e,0x023c,0x0a20},					// allen_20180530, EEE wake up cnt from 0x14 to 0x20

			//{0x1e,0x01a3,0x00d2},					// for 10M TP-IDL	// allen_20180815
			//{0x1e,0x01a4,0x010e},					// for 10M TP-IDL	// allen_20180815
			
			
			//{0x1e,0x0123,0xffff},
			//{0x1e,0x0147,0x0000},
			//{0x1e,0x0236,0x0020},
				
			//{0x1e,0x0190,0x0110},
			//{0x1e,0x0191,0x4444},					// hvga_rsel 	3b'100
			//{0x1e,0x00a6,0x0350},

			//{0x1e,0x00e7,0x5555},
            //{0x1e,0x00e9,0x0001},
            //{0x1e,0x00fe,0x0000},
            //{0x1e,0x00e6,0x1111},

			//{0x1e,0x0023,0x0885},
			//{0x1e,0x0024,0x0885},
			//{0x1e,0x0025,0x0885},
			//{0x1e,0x0026,0x0885},

			//{0x07,0x003c,0x0006}	
	     },

     	//per-port data
     	{
	 	}
    }, 
};
en75xxGe_cfg_trdata_t en75xxGe_cfg_tr[EN75xxGe_PHY_INIT_TR_SET_NUM]=
{
    {	//local data
     	{ 
			//{"PMA",0x03,0x082422}
     	},

     	//per-port data
     	{ 	
     		//{"PMA",0x03,0x082422}	
	 	}
    }, 
};

void 		en75xxGePhyCfgLoad(u8 idx);
void 		en75xxGePhyGRCfgCheck(void);
void 		en75xxGePhyLRCfgCheck(u8 port_num);
void 		en75xxGePhyCfgCheck(void);

			
#endif

//static struct timer_list ephy_timer;
////static void my_ephy_monitor(unsigned long data)
//static void my_ephy_monitor(struct timer_list *t)    
//{
//
//	/* Schedule for the next time */
//    /* add your code here */
//	if(slt_excuting == 0)
//	{
//		printk("monitor value xxxxxxx \r\n");
//		for(slt_not_excute_loop = all_port_start; slt_not_excute_loop <= all_port_end; slt_not_excute_loop++)
//	{			
//	                tcMiiStationWrite(slt_not_excute_loop, 0x04, 0x0de1);
//	                tcMiiStationWrite(slt_not_excute_loop, 0x00, 0x1240);
//					//mtPhyMiiWrite_TrDbg(slt_not_excute_loop, "PMA", 0x12, 0x5e4d2a, 0);				
//	}
//	}
//	//inital_finish_flag = 1;
//}
/*
void ephy_timer_init(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)
	init_timer(&ephy_timer);
	ephy_timer.expires = jiffies + msecs_to_jiffies(10000);
    ephy_timer.function = my_ephy_monitor;
    ephy_timer.data = 0;
#else
	timer_setup(&ephy_timer, &my_ephy_monitor, 0);
	ephy_timer.expires = jiffies + msecs_to_jiffies(10000);
#endif
    add_timer(&ephy_timer);
}
*/

void en75xxGePhyCfgLoad(u8 idx)
{
	int pn, i;
	u16 phyAddr, phyAddr_base = 9;
	current_idx = idx;

    // global registers
    for( pn=0; pn < EN75xxGe_PORTNUM; pn++)		
	{
		phyAddr = phyAddr_base + pn;
	
		TCPHYDISP3("\n 7523Ge, phyaddr= (%d,%d) \n", phyAddr_base, phyAddr);
		if (EN75xxGe_PHY_INIT_CL45_GDATA_LEN >0)
		{
		  for( i=0; i<EN75xxGe_PHY_INIT_CL45_GDATA_LEN; i++ )
		  {         
			 mtEMiiRegWrite(phyAddr, en75xxGe_cfg_cl45[current_idx].gdata[i].dev_num, en75xxGe_cfg_cl45[current_idx].gdata[i].reg_num, en75xxGe_cfg_cl45[current_idx].gdata[i].val);
		  }
		}
		if (EN75xxGe_PHY_INIT_CL45_LDATA_LEN >0)
		{
		  for( i=0; i<EN75xxGe_PHY_INIT_CL45_LDATA_LEN; i++ )
		  {         
			mtEMiiRegWrite(phyAddr, en75xxGe_cfg_cl45[current_idx].ldata[i].dev_num, en75xxGe_cfg_cl45[current_idx].ldata[i].reg_num, en75xxGe_cfg_cl45[current_idx].ldata[i].val);
		  }
		}
		if (EN75xxGe_PHY_INIT_LDATA_LEN > 0)
		{
		  for( i=0; i<EN75xxGe_PHY_INIT_LDATA_LEN; i++ )
		  {         
			tcMiiStationWrite(phyAddr, en75xxGe_cfg[current_idx].ldata[i].reg_num, en75xxGe_cfg[current_idx].ldata[i].val);
		  }
		}
		if (EN75xxGe_PHY_INIT_TR_LDATA_LEN >0)
		{
		  for( i=0; i<EN75xxGe_PHY_INIT_TR_LDATA_LEN; i++ )
		  {         
			mtPhyMiiWrite_TrDbg( phyAddr, en75xxGe_cfg_tr[current_idx].ldata[i].reg_typ, en75xxGe_cfg_tr[current_idx].ldata[i].reg_num, en75xxGe_cfg_tr[current_idx].ldata[i].val,0);
		  }
		}
	}
	
	
    TCPHYDISP4("mtphy: CfgLoad %s\r\n",  en75xxGe_cfg[current_idx].name);
}
#endif  


/************************************************************************
*                       API for EPHY (TCPHY_SUPPORT)
**************************************************************************/
// function declaration for TCEPHYDBG commands
#ifdef TCPHY_SUPPORT



/*** public, Called by tc3162l2mac.c ***/
int tcPhyPortNumGet(void)
{
	if(tcPhyVer != 99)
		return tcPhyPortNum;
	else
		return 0;
}

// ************************************************************************
// 		set tcPhyFlag & tcPhyVer
//		[in] ephy_addr
// ************************************************************************
//int tcPhyVerLookUp(macAdapter_t *mac_p)
int tcPhyVerLookUp(u16 ephy_addr)
{
    u32 rval;
	u16 r15_temp;

	ephy_addr_base = 0;


	TCPHYDISP3(" [tcPhyVerLookUp] (%d)...in \r\n", ephy_addr);
	
	r15_temp = tcMiiStationRead(ephy_addr, 15);
	//printk(" [tcPhyVerLookUp] (%d)...in...in...in...in...in, r15=0x%x \r\n", ephy_addr, r15_temp);

    rval = tcMiiStationRead(ephy_addr, 3); 				// phy revision id
	if(rval == 0xffff)
	{
		#ifdef LINUX_OS 
		rval = tcMiiStationRead(ephy_addr, 3); 			// phy revision id
		#endif
	}
    rval &= 0xffff;
	printk("%s %d %X \n", __func__, __LINE__, rval);  	// JasonG_7512
    if(rval == EPHY_ID_2031)
	{
        tcPhyVer = tcPhyVer_2031;         
        tcPhyPortNum = 1;
        TCPHYDISP1("TC2031, ");
    }
    #if debug_flag
	else if(rval == EPHY_ID_2101mb)
    {
        tcPhyVer = tcPhyVer_2101mb;
        tcPhyPortNum = 1;
        TCPHYDISP1("TC2101MB, ");
    }
	else if(rval == EPHY_ID_2104mc)
	{
        tcPhyVer = tcPhyVer_2104mc;
        tcPhyPortNum = 4;
        TCPHYDISP1("TC2104MC, ");
    }
	else if(rval == EPHY_ID_2104sd)
	{
        tcPhyVer = tcPhyVer_2104sd;
        tcPhyPortNum = 4;
        TCPHYDISP1("TC2104SD, ");
    }
	else if (rval == EPHY_ID_2101me)
	{
        tcPhyVer = tcPhyVer_2101me;
        tcPhyPortNum = 1;
        TCPHYDISP1("TC2101ME, ");
	}
	else if(rval == EPHY_ID_2102me)
	{
        tcPhyVer = tcPhyVer_2102me;
        tcPhyPortNum = 1;
        TCPHYDISP1("TC2102ME, ");	
	}
	else if (rval == EPHY_ID_2104me)
	{
        tcPhyVer = tcPhyVer_2104me;
        tcPhyPortNum = 4;
        TCPHYDISP1("TC2104ME, ");
	}
	else if(rval == EPHY_ID_2101mf)
	{
        tcPhyVer = tcPhyVer_2101mf;
        tcPhyPortNum = 1;
        TCPHYDISP1("TC2101MF, ");
    }
	else if(rval == EPHY_ID_2105sg)
	{
        tcPhyVer = tcPhyVer_2105sg;
        tcPhyPortNum = 5;
        TCPHYDISP1("TC2105SG, ");
	}
	else if(rval == EPHY_ID_2101mi)
	{
        tcPhyVer = tcPhyVer_2101mi;
        tcPhyPortNum = 1;
        TCPHYDISP1("TC2101MI, ");	
   	}
	else if(rval == EPHY_ID_2105mj)
	{
        tcPhyVer = tcPhyVer_2105mj;
        tcPhyPortNum = 5;
        TCPHYDISP1("TC2105MJ, "); 
    }
	else if(rval == EPHY_ID_2105sk)
	{
        tcPhyVer = tcPhyVer_2105sk;
        tcPhyPortNum = 5;
        TCPHYDISP1("TC2105SK, ");
    }
	else if(rval == EPHY_ID_2101mm)
	{
        tcPhyVer = tcPhyVer_2101mm;
		tcPhyPortNum = 1;
		TCPHYDISP1("TC2101MM, ");
   	}
    #endif
    
    else if(rval == EPHY_ID_7583)  
	{
        tcPhyVer = tcPhyVer_7583;		
        tcPhyPortNum = 4;
        TCPHYDISP1(" 7583 , \n");
        tcPhyFlag = 1;
        tcPhyInitFlag = 1;
    }
    else 
	{
        TCPHYDISP3(" unknown PHYID: %x, \n", rval);        
        tcPhyInitFlag = 0;
    }

	eco_rev = tcMiiStationRead(ephy_addr, 31);

	eco_rev &= (0x0f);
	
	TCPHYDISP3(" [tcPhyVerLookUp] (%d)(%d)...out	 \r\n", ephy_addr, tcPhyVer);
	//printk(" [tcPhyVerLookUp] (%d)(%d)...out...out...out...out...out	 \r\n", ephy_addr, tcPhyVer);

	if(ephy_addr_base == 0) // allen_20140822
	{
			if((tcPhyVer == tcPhyVer_7583))	
				ephy_addr_base += EPHY_ADDR_P0;

	}
	
    return 0;
}

//static void tcPhyDeinit(void)  // JasonG_7512
//{
//	inital_finish_flag = 0;
////    ecnt_unregister_hook(&ecnt_driver_ephy_op);
//}

int ePhyInit(u16 ephy_addr)
{
    int i=0;
	//u32 reg_value; //CML_20130226_1

	TCPHYDISP1(" [tcPhyInit] in ");

	switch(tcPhyVer) 
	{			    
		case tcPhyVer_7583: 		// EN7581 Ge
			if(ephy_addr_base == 0)
				ephy_addr_base += EPHY_ADDR_P0;
			break;
	}

              
    //tcMiiStationWrite(ephy_addr, PHY_CONTROL_REG, PHY_RESET);
    //tcMiiStationWrite(ephy_addr, 0x0, 0x8000);

    switch(tcPhyVer) 
	{
		#ifdef LINUX_OS 			// allen_20130926

		case tcPhyVer_7583: 		// AN7552 Ge
			en75xxGePhyCfgLoad(0);   
			break;
		#endif
    }

	#if 0
    // always boot-up with AN-enable
    //for(i=start_addr; i<start_addr+tcPhyPortNum; i++ )
	//{            
        //tcMiiStationWrite(i, PHY_CONTROL_REG, MIIDR_AUTO_NEGOTIATE );
		// rewrite to avoid changing H/W default setting //CML_20130226_1
		#ifdef LINUX_OS // allen_20130926
		reg_value = tcMiiStationRead(ephy_addr, PHY_CONTROL_REG);
		reg_value |= MIIDR_AUTO_NEGOTIATE;
        tcMiiStationWrite(ephy_addr, PHY_CONTROL_REG, reg_value );
		#else
		tcMiiStationWrite(ephy_addr, PHY_CONTROL_REG, MIIDR_AUTO_NEGOTIATE );
		#endif
    //}
    #endif
    //tcMiiStationWrite(ephy_addr, 0x0, 0x1200);

    // tcphy_link_state init.
    for(i=0; i<tcPhyPortNum; i++)
    {
        tcphy_link_state[i] = ST_LINK_DOWN;
    }
	TCPHYDISP1(" [tcPhyInit] out ");
	TCPHYDISP1(", r31 = (%d, 0x%x) \r\n", ephy_addr, tcMiiStationRead(ephy_addr, 31));
	

	
    return 0;
}


#if 0
// tcPhy initial: reset, load default register setting, restat AN
int tcPhyPortInit(u8 port_num)
{
	u32 reg_value; // CML_20130226_1
	
    #ifndef LINUX_OS
    //tcMiiStationWrite(port_num, PHY_CONTROL_REG, PHY_RESET);
	#endif

    switch(tcPhyVer) 
	{     
		//case tcPhyVer_7512Fe: 		// mt7512 FE
		//case tcPhyVer_7522Fe: 		// mt7512 FE
		//	mt7512FECfgLoad(0, DO_PER_PORT, port_num);
		//	break;
    }

    // always boot-up with AN-enable         
    //tcMiiStationWrite(port_num, PHY_CONTROL_REG, MIIDR_AUTO_NEGOTIATE );
    // rewrite to avoid changing H/W default setting //CML_20130226_1
	//tcMiiStationWrite(port_num, PHY_CONTROL_REG, MIIDR_AUTO_NEGOTIATE );
	reg_value = tcMiiStationRead(port_num, PHY_CONTROL_REG);
	reg_value |= MIIDR_AUTO_NEGOTIATE;
	tcMiiStationWrite(port_num, PHY_CONTROL_REG, reg_value);
	
    // tcphy_link_state init.
    tcphy_link_state[port_num]=ST_LINK_DOWN;
    return 0;
}
#endif

// return LP0Reg1 PHY Status Registers Value
u8 getTcPhyFlag(void)
{
    return tcPhyFlag;
}
#if 0
u32 getTcPhyStatusReg(int phy_add)
{
	// no called if (force_link_flag==1)
	/*
	#ifdef PHYPART_DEBUG
	printk("PhyPart debug: getTcPhyStatusReg() in \r\n");
	#endif
	*/
    if(!sw_patch_flag)//if sw patch off, mac get PHY status via MDIO directly.
    {
		/*    
		#ifdef PHYPART_DEBUG
		printk("PhyPart debug: getTcPhyStatusReg() out \r\n");
		#endif
		*/
		return ( tcMiiStationRead(phy_add, PHY_STATUS_REG) );
    }
    else//if sw patch on, mac get the PHY status that patch intercept. 
    {
	    /*
		#ifdef PHYPART_DEBUG
		printk("PhyPart debug: getTcPhyStatusReg() out \r\n");
		#endif
		*/
		return (Nmr1[0].value); //mr1_link_status_reg;
	}
	return 0;
}
#endif

u32 getTcPhyRMCAPReg(int phy_add)
{
	#if 0
	#ifdef TC2101MI_SUPPORT
	if ((tcPhyVer==tcPhyVer_2101mi) && sw_patch_flag)
		return tc2101mi_reg5_val;
	else
	#endif
	#endif	
	//return ( tcMiiStationRead(phy_add, PHY_REMOTE_CAP_REG));	
	return 0;
}
#if 0
u16 getTcPhyNStatusReg(u8 port_num)
{
    u16 phyAddr;

    if(!sw_patch_flag) 
	{
        phyAddr = ephy_addr_base + port_num;
        return ( tcMiiStationRead(phyAddr, PHY_STATUS_REG) ); 
    }
    else 
	{
        return (Nmr1[port_num].value);
    }
    return 0;
}
#endif

bool getTcPhyEsdDetectFlag(void)
{
	return 0;
}
	

 
#ifdef LINUX_OS
/*=========================================================================
**   tcephydbgcmd: register rootcommand of "ether ". 
**           call: call by femac.c/mainInit(), tc3262_gmac_init
**=========================================================================*/
//void tcephydbgcmd(void)
//{
//	cmds_t tcephydbg_cmd;
//
//	tcephydbg_cmd.name = "tce";
//	tcephydbg_cmd.func = doEtherPhydbg;
//	tcephydbg_cmd.flags = 0x12;
//	tcephydbg_cmd.argcmin = 0;
//	tcephydbg_cmd.argc_errmsg = NULL;
//	cmd_register(&tcephydbg_cmd);
//} /*end tcephydbgcmd*/
//
//void tcephydbgcmd1(void)
//{
//	cmds_t tcephydbg_cmd;
//
//	tcephydbg_cmd.name = "tce1";
//	tcephydbg_cmd.func = doEtherPhydbg;
//	tcephydbg_cmd.flags = 0x12;
//	tcephydbg_cmd.argcmin = 0;
//	tcephydbg_cmd.argc_errmsg = NULL;
//	cmd_register(&tcephydbg_cmd);
//} /*end tcephydbgcmd*/
#endif

#endif //TCPHY_SUPPORT


/************************************************************************
*         API functions body for EPHY (TCPHY_SUPPORT)
**************************************************************************/
#ifdef TCPHY_SUPPORT

/************************************************************************
*                       API functions body for EPHY
**************************************************************************/


#endif // TCPHY_DEBUG
//int tcPhyInit(void)
//{
//	printk("!!!!!!!!!!!!!!!!!!!!!!ecnt_check!!!!!!!!!!!!!!!!!!!!!\n");
//	dump_stack();
//  if(ecnt_register_hook(&ecnt_driver_ephy_op)) 
//  {
//		printk("ecnt_driver_ephy_op register fail\n");
//		return -ENODEV ;
//  }  // JasonG_7512
//	return tcPhyInit_patch();
//}
//module_init(tcPhyInit);
//module_exit(tcPhyDeinit);  // JasonG_7512
//MODULE_LICENSE("Proprietary");
MODULE_LICENSE("GPL");
