/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

#include <linux/phy.h>
#include <linux/regmap.h>
#include <linux/mfd/syscon.h>

#ifndef _TCETHERPHY__DEF_H_
#define _TCETHERPHY__DEF_H_
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
//#include <ecnt_hook/ecnt_hook_ether.h>
#include "tcetherphy_hook.h"
#include "tcetherphy.h"
#include <linux/bitfield.h>
#include <linux/phy.h>


// variables for mii registers
typedef struct 
{
    u8 main_reset; 		// 15
    u8 force_speed; 		// 13
    u8 autoneg_enable; 	// 12
    u8 powerdown; 		// 11
    u8 force_duplex; 	// 8
} tcphy_mr0_reg_t;

typedef struct 
{
    u16 value; 				// 15:0
    //u8 autoneg_complete; 	// 5
    bool link_status; 			// 2
    bool link_status_prev;
} tcphy_mr1_reg_t;

typedef struct 
{
    u8 selector_field; 	// 4:0
    u8 able100F; 		// 8
    u8 able100H; 		// 7
    u8 able10F; 			// 6
    u8 able10H; 			// 5
} tcphy_mr4_reg_t; // use for mr4 & mr5

typedef struct 
{
    u8 selector_field; 	// 4:0
    u8 able100F; 		// 8
    u8 able100H; 		// 7
    u8 able10F; 			// 6
    u8 able10H; 			// 5
    u8 LPNextAble;		// 15
} tcphy_mr5_reg_t; // use for mr4 & mr5

typedef struct 
{
    //u8 parallel_detect_fault; 	// 4
    u8 lp_np_able; 				// 3
    //u8 np_able; 				// 2
    //u8 lch_page_rx; 			// 1
    u8 lp_autoneg_able; 			// 0
} tcphy_mr6_reg_t; // use for mr6

typedef struct 
{
    //u8  slicer_err_thd; 	// 15:11
    u16 err_over_cnt;   		// 10:0
    u16 err_over_cnt_prev; 	// 10:0
} tcphy_l0r25_reg_t;

typedef struct 
{
    //u8  slicer_err_thd; 	// 15:11
    u16 err_over_cnt;   		// 10:0
    u16 err_over_cnt_prev; 	// 10:0
} mtphy_errovcnt_reg_t;

typedef struct 
{
    u8 lch_sig_detect;		// 15
    u8 lch_rx_linkpulse;		// 14
    u8 lch_linkup_100;		// 13
    u8 lch_linkup_10;		// 12
    u8 lch_linkup_mdi;		// 11
    u8 lch_linkup_mdix;		// 10
    u8 lch_descr_lock;		// 9
    u8 mdix_status;			// 5
    u8 tx_amp_save;			// 4:3
    u8 final_duplex;			// 2
    u8 final_speed;			// 1
    u8 final_link;   		// 0
} tcphy_l0r28_reg_t;

typedef struct 
{
    u8 lch_SignalDetect;			// 15
    u8 lch_LinkPulse;			// 14
    u8 lch_DescramblerLock1000;	// 13
	u8 lch_DescramblerLock100;	// 12
    u8 lch_LinkStatus1000_OK;	// 11
	u8 lch_LinkStatus100_OK;		// 10
	u8 lch_LinkStatus10_OK;		// 9 
	u8 lch_MrPageRx;				// 8
	u8 lch_MrAutonegComplete;	// 7
  	u8 da_mdix;					// 6
  	u8 FullDuplexEnable;			// 5
  	u8 MSConfig1000;				// 4
  	u8 final_speed_1000;			// 3 
  	u8 final_speed_100;			// 2
  	u8 final_speed_10;			// 1
} tcphy_1ErA2_reg_t;

typedef struct 
{
    u8 lp_eee_10g;
    u8 lp_eee_1000;
    u8 lp_eee_100;  
} tcphy_l3r18_reg_t;

typedef struct 
{
    u8 lp_eee_10g;
    u8 lp_eee_1000;
    u8 lp_eee_100;  
} tcphy_7r3D_reg_t;

#define EPHY_ADDR_P0   9
#define debug_flag	1

#ifdef LINUX_OS
#define pause(x)					mdelay(x)
#endif //LINUX_OS

#ifdef TCPHY_SUPPORT
// type for register settings
typedef struct cfg_data_s
{
    u32 	reg_num;
    u32 	val;
}cfg_data_t;
typedef struct cfg_cL45data_s
{
    u32 	dev_num;
    u32 	reg_num;
    u32 	val;
}cfg_cl45data_t;
typedef struct cfg_trdata_s
{
    char 	reg_typ[10];
    u32 	reg_num;
    u32 	val;
}cfg_trdata_t;

#endif

//#ifdef LINUX_OS 

#define EN75xxGe_PORTNUM 				4		
#define EN75xxGe_PHY_INIT_LDATA_LEN 	0
#define EN75xxGe_PHY_INIT_PERPDATA_LEN 	0
#define EN75xxGe_INIT_SET_NUM 			1
#define EN75xxGe_PHY_INIT_SET_NUM 		1

#define EN75xxGe_PHY_INIT_CL45_GDATA_LEN 	0 	
#define EN75xxGe_PHY_INIT_CL45_LDATA_LEN 	0	 
#define EN75xxGe_PHY_INIT_CL45_PERPDATA_LEN 0
#define EN75xxGe_PHY_INIT_CL45_SET_NUM 		1

#define EN75xxGe_PHY_INIT_TR_LDATA_LEN 		1
#define EN75xxGe_PHY_INIT_TR_PERPDATA_LEN 	1
#define EN75xxGe_PHY_INIT_TR_SET_NUM 		1

typedef struct en75xxGe_cfg_data_s
{
	char 		name[10];
	cfg_data_t 	ldata[EN75xxGe_PHY_INIT_LDATA_LEN];
	cfg_data_t 	perpdata[EN75xxGe_PHY_INIT_PERPDATA_LEN];			// per port register setting	
}en75xxGe_cfg_data_t;

typedef struct en75xxGe_cfg_cl45data_s
{
	cfg_cl45data_t gdata[EN75xxGe_PHY_INIT_CL45_GDATA_LEN];
	cfg_cl45data_t ldata[EN75xxGe_PHY_INIT_CL45_LDATA_LEN];
	cfg_cl45data_t perpdata[EN75xxGe_PHY_INIT_CL45_PERPDATA_LEN];	// per port register setting	
}en75xxGe_cfg_cl45data_t;

typedef struct en75xxGe_cfg_trdata_s
{
	cfg_trdata_t ldata[EN75xxGe_PHY_INIT_TR_LDATA_LEN];
	cfg_trdata_t perpdata[EN75xxGe_PHY_INIT_PERPDATA_LEN];			// per port register setting	
}en75xxGe_cfg_trdata_t;


typedef	struct {
	char *name;		
	int	(*func)(int argc,char *argv[],void *p);
	int	flags;	
	int	argcmin;
	char *argc_errmsg;	
} cmds_t;
//#endif

#define EPHY_ID_2031		0x9400
#define	EPHY_ID_2101mb		0x9401
#define	EPHY_ID_2104mc		0x9402
#define	EPHY_ID_2104sd		0x9403
#define	EPHY_ID_2101me		0x9404
#define	EPHY_ID_2102me		0x9405	
#define	EPHY_ID_2104me		0x9406
#define	EPHY_ID_2101mf		0x9407
#define	EPHY_ID_2105sg		0x9408
#define	EPHY_ID_2101mi		0x940a
#define	EPHY_ID_2105mj		0x940b
#define	EPHY_ID_2105sk		0x940c
#define	EPHY_ID_2101mm		0x940e
#define	EPHY_ID_7530Ge_7512Fe	0x9412
#define	EPHY_ID_7552Ge		0x0400
#define	EPHY_ID_7583		0x0420

// for tcXXXX_link_state
#define ST_LINK_DOWN 	0
#define ST_LINK_DOWN2UP 1
#define ST_LINK_UP 		2
#define ST_LINK_UP2DOWN 3

#define tcPhyVer_2031     	0  	// 9400, LEH
#define tcPhyVer_2101mb   	1  	// 9401, LEM
#define tcPhyVer_2104mc   	2  	// 9402, tc2206
#define tcPhyVer_2104sd   	3  	// 9403, tc2104sd
#define tcPhyVer_2101me   	4  	// 9404, 62UE
#define tcPhyVer_2102me   	5  	// 9405, tc3182
#define tcPhyVer_2104me   	6  	// 9406, tc2206F
#define tcPhyVer_2101mf   	7  	// 9407, FPGA(MF)
#define tcPhyVer_2105sg   	8  	// 9408, TC6501
#define tcPhyVer_2101mi   	10 	// 940a, RT63260
#define tcPhyVer_2105mj   	11 	// 940b, RT63365
#define tcPhyVer_2105sk   	12 	// 940c, TC6508
#define mtPhyVer_7530     	13 	// 9412, MT7530
#define tcPhyVer_2101mm   	14 	// 940e, MT7502 
#define tcPhyVer_7552Ge		27	// 0400
#define tcPhyVer_7583		28	// 0400

#define  all_port_start 9
#define  all_port_end 12

#define EEE_pairA_ON_pairBCD_OFF 1
#define idle_mode_power_saving 1

#ifdef TCPHY_SUPPORT
#define ProbeZfgain  					0
#define ProbeAgccode 					1
#define ProbeBoosten 					2
#define ProbeSnr     					3
#define ProbeDcoff   					4
#define ProbeAdcoff  					5
#define ProbeAdcSign 					6
#define Probe_VgaState 					7
#define Probe_MSE 						8
#define Probe_MSE_ALL 					9
#define Probe_MSE_Sum 					10
#define Probe_Slicer_Err_Over_Sum 		11
#define Probe_Slicer_Err_Over_Sum_ALL 	12
#define Probe_MSE_Slicer_err_thres 		13

#define TCPHYDISP1 if(tcPhy_disp_level>=1) printk
#define TCPHYDISP2 if(tcPhy_disp_level>=2) printk

#if defined(TCPHY_DEBUG) || defined(MTPHY_DEBUG)
#define TCPHYDISP3 if(tcPhy_disp_level>=3) printk
#define TCPHYDISP4 if(tcPhy_disp_level>=4) printk
#define TCPHYDISP5 if(tcPhy_disp_level>=5) printk
#define TCPHYDISP6 if(tcPhy_disp_level>=6) printk
#else
#define TCPHYDISP3 if(0) printk
#define TCPHYDISP4 if(0) printk
#define TCPHYDISP5 if(0) printk
#define TCPHYDISP6 if(0) printk
#endif
#define SkewDISP1 if(mtSkewCal_disp_level>=1) printk
#endif

#define SMIREG_PWDN_BIT         (1 << 11)


//#define tcMiiStationRead(phy_add, phy_reg) ETHER_MDIO_READ(phy_add, phy_reg)
//#define tcMiiStationWrite(phy_add, phy_reg, phy_data) ETHER_MDIO_WRITE(phy_add, phy_reg, phy_data)

extern u8   tcSWVer_REG; 
extern u8   tcSWVer_FRAMEWORK; 
extern u8   tcSWVer_CMD; 
extern u8   tcSWVer_FUNC; 
extern u8   tcSWVer_PATCH; 
extern u8   tcSWVer_COMMON; 



extern u8	ephy_addr_base;
extern u8    tcPhyVer;
extern u8    tcPhy_disp_level;
extern uint     mtSkewCal_disp_level;
extern unsigned int slt_not_excute_loop ;
extern unsigned int mdi_resister;
extern unsigned int transformer_status;

u32 	tcPhyReadReg(u8 port_num,u8 reg_num);
void 	tcPhyWriteReg(u8 port_num,u8 reg_num,u32 reg_data);
u32 	tcPhyReadLReg(u8 port_num,u8 page_num,u8 reg_num);
void 	tcPhyWriteLReg(u8 port_num,u8 page_num,u8 reg_num,u32 reg_data);
u32 	tcPhyReadGReg(u8 port_num,u8 page_num,u8 reg_num);
void 	tcPhyWriteGReg(u8 port_num,u8 page_num,u8 reg_num,u32 reg_data);  
//#ifdef LINUX_OS 
u32 	mtPhyReadGReg(u32 port_num, u32 dev_num, u32 reg_num);
u32 	mtPhyReadReg(u8 port_num, u8 reg_num);
int 	mtPhyMiiWrite_TrDbg(u8 phyaddr, char *type, u32 reg ,u32 val, u8 ch_num);
int32_t 	mtPhyMiiRead_TrDbg(u8 phyaddr, char *type, u32 reg , u8 ch_num);
//void 	mtPhyChkVal (void);
void 	mtMiiRegWrite(int port_num, int reg_num, int reg_data);
u32 	mtMiiRegRead(u8 port_num,u8 reg_num);
void    mtRegWrite_CL22(u32 port_num,u32 dev_num,u32 reg_num,u32 reg_data);
u32  mtRegRead_CL22(u32 port_num,u32 dev_num,u32 reg_num);
void 	mtEMiiRegWrite(u32 port_num, u32 dev_num, u32 reg_num, u32 reg_data);
u32 	mtEMiiRegRead(u32 port_num, u32 dev_num, u32 reg_num);
unsigned int buck_pbus_read( int phy, unsigned int register_address);
void buck_pbus_write( int phy, unsigned int register_address,unsigned int value);
void toKenRingWrite(unsigned int phy, unsigned int ch, unsigned int node, unsigned int data_addr,unsigned int value);
unsigned int toKenRingRead(unsigned int phy, unsigned int ch, unsigned int node, unsigned int data_addr);
//#endif


int32_t en75xxGePhyReadProbe(u8 port_num, u8 mode, u8 ch_num);

int tcPhyVerLookUp(u16 ephy_addr);
int ePhyInit(u16 ephy_addr);
int tcPhyInit(void);
void    normalinit(void);


void normalinit(void);
void GECal_tx_offset(u8 phyaddr, u32 delay);

void doGePhyALLAnalogCal_R45(u8 phyaddr);

u16 checked_atoi(char *val);

/*
#define VPint			*(volatile u32 *)
static inline u32 regRead32(u32 reg)		\
{						  	\
	return (u32) VPint(reg);			  	\
}		
static inline void regWrite32(u32 reg, u32 vlaue)	\
{                                                	\
     VPint(reg) = vlaue;                      	\
}
*/

#endif /* _TCETHERPHY__DEF_H_ */
