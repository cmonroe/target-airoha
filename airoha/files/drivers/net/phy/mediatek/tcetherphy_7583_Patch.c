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
#endif
#include <asm/io.h>
//#include <asm/tc3162/tc3162.h>
//#include <asm/tc3162/ledcetrl.h>
//#include <asm/tc3162/TCIfSetQuery_os.h>
//#include <asm/tc3162/cmdparse.h>
#include <linux/mii.h>
//#include <ecnt_hook/ecnt_hook_ephy.h>
//#include "tcetherphy_hook.h"
#include "tcetherphy.h"
#include "tcetherphy_define.h"

u8    tcSWVer_PATCH =2; 

u8    phychkval_portnum = 0;
u8    phychkval_flag = 0;			// variable for doPhyChkVal()


#if 0
static u8 	power_on_finish_flag = 0;  	//dai121019 indicate boot end
#endif
u8   slt_excuting =0;
u8 	sw_patch_flag = 1;
u8   inital_finish_flag = 0;
u8   disable_force_master_flag = 1; //wschung add for disable force master

extern void mtMiiRegWrite(int port_num, int reg_num, int reg_data);
extern u32 mtMiiRegRead(u8 port_num,u8 reg_num);
extern void mtEMiiRegWrite(u32 port_num, u32 dev_num, u32 reg_num, u32 reg_data);;
extern u32 mtEMiiRegRead(u32 port_num, u32 dev_num, u32 reg_num);;
extern int tcMiiStationWrite(u32 phy_addr, u32 phy_reg, u32 phy_data);
extern int tcMiiStationRead(u32 phy_addr, u32 phy_reg);
extern void toKenRingWrite(unsigned int phy, unsigned int ch, unsigned int node, unsigned int data_addr,unsigned int value);
extern unsigned int toKenRingRead(unsigned int phy, unsigned int ch, unsigned int node, unsigned int data_addr);

static u8 	Skew_update_flag = 0;
static u8	force_giga_master_counter_p9=0,force_giga_master_counter_p10=0,force_giga_master_counter_p11=0,force_giga_master_counter_p12=0;
static int mean_square_error_counter_p9=0,mean_square_error_counter_p10=0,mean_square_error_counter_p11=0,mean_square_error_counter_p12=0;
static int ephy_monitor_continue_p9_counter=0,ephy_monitor_continue_p10_counter=0,ephy_monitor_continue_p11_counter=0,ephy_monitor_continue_p12_counter=0;
static int downshift_2pair_capture_counter_p9_counter=0,downshift_2pair_capture_counter_p10_counter=0,downshift_2pair_capture_counter_p11_counter=0,downshift_2pair_capture_counter_p12_counter=0;
static int force_slave_enable_p9=0,force_slave_enable_p10=0,force_slave_enable_p11=0,force_slave_enable_p12=0;
static u8    force_giga_master_mode_p9=0,force_giga_master_mode_p10=0,force_giga_master_mode_p11=0,force_giga_master_mode_p12=0;

//downshift
static int downshift_to_force_giga_p9=0,downshift_to_force_giga_p10=0,downshift_to_force_giga_p11=0,downshift_to_force_giga_p12=0;
static int loopback_haed_start_p9=0,loopback_haed_start_p10=0,loopback_haed_start_p11=0,loopback_haed_start_p12=0;
unsigned int netgear_downshift_100M_start_p9,netgear_downshift_100M_start_p10,netgear_downshift_100M_start_p11,netgear_downshift_100M_start_p12;
unsigned int netgear_downshift_100M_end_p9,netgear_downshift_100M_end_p10,netgear_downshift_100M_end_p11,netgear_downshift_100M_end_p12;
unsigned int netgear_downshift_100M_counter_p9,netgear_downshift_100M_counter_p10,netgear_downshift_100M_counter_p11,netgear_downshift_100M_counter_p12;
unsigned int netgear_downshift_100M_last_p9,netgear_downshift_100M_last_p10,netgear_downshift_100M_last_p11,netgear_downshift_100M_last_p12;
unsigned int netgear_downshift_100M_into_p9,netgear_downshift_100M_into_p10,netgear_downshift_100M_into_p11,netgear_downshift_100M_into_p12;

unsigned int  reltek_downshift_100M_counter_p9=0,reltek_downshift_100M_counter_p10=0,reltek_downshift_100M_counter_p11=0,reltek_downshift_100M_counter_p12=0;
unsigned int  reltek_downshift_100M_into_p9=0,reltek_downshift_100M_into_p10=0,reltek_downshift_100M_into_p11=0,reltek_downshift_100M_into_p12=0;
unsigned int prefer_master_kepp_link_p9=0,prefer_master_kepp_link_p10=0,prefer_master_kepp_link_p11=0,prefer_master_kepp_link_p12=0;
unsigned int netgear_downshift_100M_flag_p9 = 0, netgear_downshift_100M_flag_p10 = 0, netgear_downshift_100M_flag_p11 = 0, netgear_downshift_100M_flag_p12 = 0;

static u8 netgear_downshift_100M=1, debugfs_cmd_init = 0;
//

extern u8 loopback_head_downshift_100M;
extern tcphy_1ErA2_reg_t 	mr1E_A2;
extern tcphy_mr1_reg_t 		Nmr1[TCPHY_PORTNUM];
extern u8 mtphy_link_state[MTPHY_PORTNUM];
extern u8 link_state_polling;
extern int ephy_monitor_p9_stop_polling;
extern int ephy_monitor_p10_stop_polling;
extern int ephy_monitor_p11_stop_polling;
extern int ephy_monitor_p12_stop_polling;
extern int ephy_monitor_all_counter;
extern u8 phy_downshift_force_giga_enable;
extern u8 sw_down_shift_en;
extern int sw_down_shift_p9;
extern int sw_down_shift_p10;
extern int sw_down_shift_p11;
extern int sw_down_shift_p12;
extern int ephy_monitor_p9_curren_counter;
extern int ephy_monitor_p10_curren_counter;
extern int ephy_monitor_p11_curren_counter;
extern int ephy_monitor_p12_curren_counter;
extern u8 	GECal_Rext_flag;
extern u8    tcPhyInitFlag;
extern u8    command_calbration_enable;

extern void ephy_timer_init(void);
extern int auto_select_transformer(unsigned int phy);
//extern void tcephydbgcmd(void);
extern int ephy_debugfs_init(void);

#define HWTRAP_RESISTER 1
#define RGS_ECC_SEL 0x1FA20258

//==========================IOT WA Check Flag=======================================
unsigned int workaround_flag_p9 = 0, workaround_flag_p10 = 0, workaround_flag_p11 = 0, workaround_flag_p12 = 0;
unsigned int workaround_linkdown_flag_p9 = 0, workaround_linkdown_flag_p10 = 0, workaround_linkdown_flag_p11 = 0, workaround_linkdown_flag_p12 = 0;
//==========================IOT WA Check Flag=======================================

//===============LDPS MARCO====================
#define LDPS_RX_RCV_NLP 0xAB
#define LDPS_RX_NON_RCV_NLP 0xCD
#define BIT0 0x0001
#define BIT1 0x0002
#define BIT2 0x0004
#define BIT3 0x0008
#define BIT4 0x0010
#define BIT5 0x0020
#define BIT6 0x0040
#define BIT7 0x0080
#define BIT8 0x0100
#define BIT9 0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000

u8 LDPS_function_en = 1;
extern u8 testmode_excuted_flag;
static u8 LDPS_rx_status = 0, LDPS_nlp_sd_mask_flag = 0, LDPS_function_skip_flag = 0;
static u32 LDPS_RX_IDLE_CNT = 0, LDPS_RX_BUSY_CNT = 0;
int LDPS_Framework_cnt = 0;
tcphy_mr1_reg_t LDPS_check_status[TCPHY_PORTNUM];

//u8 phy_downshift_force_giga_enable = 0;
//u8 testmode_excuted_flag = 0;


void LDPS_main(void);
void LDPS_Framework(void);
void LDPS_TX_normal(u32 port);
void LDPS_TX_idle(u32 port);
u8 check_RX_interrupt(u32 port);
void LDPS_clear_nlp_det_int(u32 port);
void LDPS_Exit_Linkpulse(u32 port);
void LDPS_Gen_Linkpulse(u32 port);
void ephy_dis_LDPS(void);

//===============LDPS MARCO END====================

void PreCalSet(void)
{
	u16 ephy_addr;
	u32 value;
//Sim  0x1FB5FC14 bit28 =0

/*    value =regRead32(0x1FB5FC14);
        value &= 0xEFFFFFFF;
        regWrite32(0x1FB5FC14,value);    
*/
    value =readl( ioremap(0x1FB5FC14,4));
    value &= 0xEFFFFFFF;
    writel(value, ioremap(0x1FB5FC14,4)); 
	
    msleep(100);	
	//power on ETH p0 p1 p2 p3 
    writel(0x00001040, ioremap(0x1FB5FC24,4));  
    writel(0xc0800000, ioremap(0x1FB5FC20,4));  
    writel(0x00001040, ioremap(0x1FB5FC24,4));  
    writel(0xc1800000, ioremap(0x1FB5FC20,4));  
    writel(0x00001040, ioremap(0x1FB5FC24,4));  
    writel(0xc2800000, ioremap(0x1FB5FC20,4)); 
    writel(0x00001040, ioremap(0x1FB5FC24,4));  
    writel(0xc3800000, ioremap(0x1FB5FC20,4)); 
//Read HWtrap if 0R or 5R apply on GBE
#if (HWTRAP_RESISTER ==1)

    value =readl( ioremap(RGS_ECC_SEL,4));
	//bit23
    if (value &= 0x00800000)
		mdi_resister = 5;
	else
		mdi_resister = 0;
#endif	
   
	for(ephy_addr = all_port_start; ephy_addr <= all_port_end; ephy_addr++)
		{
        //Sim  CL22 reg0 bit11 =0
          value = tcMiiStationRead(ephy_addr, 0x0);
          value &=0xFFFFF7FF; 
          tcMiiStationWrite(ephy_addr, 0x0, value);
       
					//RX setting
			//mtEMiiRegWrite(ephy_addr, 0x1e, 0x3c, 0x0000);
			//mtEMiiRegWrite(ephy_addr, 0x1e, 0x3d, 0x0000);
			//mtEMiiRegWrite(ephy_addr, 0x1e, 0x3e, 0x0000);
			//mtEMiiRegWrite(ephy_addr, 0x1e, 0xc6, 0x53aa);

             //mtEMiiRegWrite(ephy_addr, 0x1e, 0x5c, 0x6666);
			//if(define_TXMR_or_discrete_TXMR == phy_5R_discrete) { mtEMiiRegWrite(ephy_addr, 0x1e, 0x5c, 0x6666);}
			//if(define_TXMR_or_discrete_TXMR == phy_0R_discrete) { mtEMiiRegWrite(ephy_addr, 0x1e, 0x5c, 0x6666);}
			//if(define_TXMR_or_discrete_TXMR == phy_5R_TXMR) { mtEMiiRegWrite(ephy_addr, 0x1e, 0x5c, 0x7777);}
			//if(define_TXMR_or_discrete_TXMR == phy_0R_TXMR) { mtEMiiRegWrite(ephy_addr, 0x1e, 0x5c, 0x7777);}
					
			
					
					
    	}



}

/*
int phyTxAmpcomp(int show_or_comp,unsigned int phy, unsigned int TXMR_or_discrete)
{ 	
	int	P9_tx_amp_hbt_cha_comp,P9_tx_amp_hbt_chb_comp;
	int	P10_tx_amp_hbt_cha_comp,P10_tx_amp_hbt_chb_comp;
	int	P11_tx_amp_hbt_cha_comp,P11_tx_amp_hbt_chb_comp;
	int	P12_tx_amp_hbt_cha_comp,P12_tx_amp_hbt_chb_comp;	
	int	P9_tx_amp_tbt_cha_comp,P9_tx_amp_tbt_chb_comp;
	int	P10_tx_amp_tbt_cha_comp,P10_tx_amp_tbt_chb_comp;
	int	P11_tx_amp_tbt_cha_comp,P11_tx_amp_tbt_chb_comp;
	int	P12_tx_amp_tbt_cha_comp,P12_tx_amp_tbt_chb_comp;
	int	P9_tx_amp_tst_cha_comp,P9_tx_amp_tst_chb_comp,P9_tx_amp_tst_chc_comp,P9_tx_amp_tst_chd_comp;
	int	P10_tx_amp_tst_cha_comp,P10_tx_amp_tst_chb_comp,P10_tx_amp_tst_chc_comp,P10_tx_amp_tst_chd_comp;
	int	P11_tx_amp_tst_cha_comp,P11_tx_amp_tst_chb_comp,P11_tx_amp_tst_chc_comp,P11_tx_amp_tst_chd_comp;
	int	P12_tx_amp_tst_cha_comp,P12_tx_amp_tst_chb_comp,P12_tx_amp_tst_chc_comp,P12_tx_amp_tst_chd_comp;
	int P9_tx_amp_gbe_cha_comp,P9_tx_amp_gbe_chb_comp,P9_tx_amp_gbe_chc_comp,P9_tx_amp_gbe_chd_comp;
	int	P10_tx_amp_gbe_cha_comp,P10_tx_amp_gbe_chb_comp,P10_tx_amp_gbe_chc_comp,P10_tx_amp_gbe_chd_comp;
	int	P11_tx_amp_gbe_cha_comp,P11_tx_amp_gbe_chb_comp,P11_tx_amp_gbe_chc_comp,P11_tx_amp_gbe_chd_comp;
	int	P12_tx_amp_gbe_cha_comp,P12_tx_amp_gbe_chb_comp,P12_tx_amp_gbe_chc_comp,P12_tx_amp_gbe_chd_comp;
	
	int txamp_low_limit,txamp_high_limit;
	int r50_low_limit,r50_high_limit; 

	unsigned int  reg_1e_12_read,reg_1e_16_read,reg_1e_17_read,reg_1e_18_read,reg_1e_19_read,reg_1e_20_read,reg_1e_21_read,reg_1e_22_read,reg_1e_174_read,reg_1e_175_read;
    unsigned int  reg_1e_12_write,reg_1e_16_write,reg_1e_17_write,reg_1e_18_write,reg_1e_19_write,reg_1e_20_write,reg_1e_21_write,reg_1e_22_write,reg_1e_174_write,reg_1e_175_write;
	int  reg_1e_12_header,reg_1e_16_header,reg_1e_17_header,reg_1e_18_header,reg_1e_19_header,reg_1e_21_header,reg_1e_174_header,reg_1e_175_header;
	int  reg_1e_12_end,reg_1e_16_end,reg_1e_17_end,reg_1e_18_end,reg_1e_20_end,reg_1e_22_end,reg_1e_174_end,reg_1e_175_end;
	int  status_flag;

	int	P9_r50_cha_comp,P9_r50_chb_comp,P9_r50_chc_comp,P9_r50_chd_comp;
	int	P10_r50_cha_comp,P10_r50_chb_comp,P10_r50_chc_comp,P10_r50_chd_comp;
	int	P11_r50_cha_comp,P11_r50_chb_comp,P11_r50_chc_comp,P11_r50_chd_comp;
	int	P12_r50_cha_comp,P12_r50_chb_comp,P12_r50_chc_comp,P12_r50_chd_comp;

	
	//TXMR_or_discrete=0-->discrete TXMR_or_discrete=1-->TXMR
	P9_tx_amp_hbt_cha_comp=0;P9_tx_amp_hbt_chb_comp=0;
	P10_tx_amp_hbt_cha_comp=0;P10_tx_amp_hbt_chb_comp=0;
	P11_tx_amp_hbt_cha_comp=0;P11_tx_amp_hbt_chb_comp=0;
	P12_tx_amp_hbt_cha_comp=0;P12_tx_amp_hbt_chb_comp=0;	
	P9_tx_amp_tbt_cha_comp=0;P9_tx_amp_tbt_chb_comp=0;
	P10_tx_amp_tbt_cha_comp=0;P10_tx_amp_tbt_chb_comp=0;
	P11_tx_amp_tbt_cha_comp=0;P11_tx_amp_tbt_chb_comp=0;
	P12_tx_amp_tbt_cha_comp=0;P12_tx_amp_tbt_chb_comp=0;
	P9_tx_amp_tst_cha_comp=0;P9_tx_amp_tst_chb_comp=0;P9_tx_amp_tst_chc_comp=0;P9_tx_amp_tst_chd_comp=0;
	P10_tx_amp_tst_cha_comp=0;P10_tx_amp_tst_chb_comp=0;P10_tx_amp_tst_chc_comp=0;P10_tx_amp_tst_chd_comp=0;
	P11_tx_amp_tst_cha_comp=0;P11_tx_amp_tst_chb_comp=0;P11_tx_amp_tst_chc_comp=0;P11_tx_amp_tst_chd_comp=0;
	P12_tx_amp_tst_cha_comp=0;P12_tx_amp_tst_chb_comp=0;P12_tx_amp_tst_chc_comp=0;P12_tx_amp_tst_chd_comp=0;
	P9_tx_amp_gbe_cha_comp=0;P9_tx_amp_gbe_chb_comp=0;P9_tx_amp_gbe_chc_comp=0;P9_tx_amp_gbe_chd_comp=0;
	P10_tx_amp_gbe_cha_comp=0;P10_tx_amp_gbe_chb_comp=0;P10_tx_amp_gbe_chc_comp=0;P10_tx_amp_gbe_chd_comp=0;
	P11_tx_amp_gbe_cha_comp=0;P11_tx_amp_gbe_chb_comp=0;P11_tx_amp_gbe_chc_comp=0;P11_tx_amp_gbe_chd_comp=0;
	P12_tx_amp_gbe_cha_comp=0;P12_tx_amp_gbe_chb_comp=0;P12_tx_amp_gbe_chc_comp=0;P12_tx_amp_gbe_chd_comp=0;

	txamp_low_limit=-3;txamp_high_limit=0x42;
    r50_low_limit=-8;r50_high_limit=0x88; 

	reg_1e_12_read=0;reg_1e_16_read=0;reg_1e_17_read=0;reg_1e_18_read=0;reg_1e_19_read=0;reg_1e_20_read=0;reg_1e_21_read=0;reg_1e_22_read=0;reg_1e_174_read=0;reg_1e_175_read=0;
    reg_1e_12_write=0;reg_1e_16_write=0;reg_1e_17_write=0;reg_1e_18_write=0;reg_1e_19_write=0;reg_1e_20_write=0;reg_1e_21_write=0;reg_1e_22_write=0;reg_1e_174_write=0;reg_1e_175_write=0;
	reg_1e_12_header=0;reg_1e_16_header=0;reg_1e_17_header=0;reg_1e_18_header=0;reg_1e_19_header=0;reg_1e_21_header=0;reg_1e_174_header=0;reg_1e_175_header=0;
	reg_1e_12_end=0;reg_1e_16_end=0;reg_1e_17_end=0;reg_1e_18_end=0;reg_1e_20_end=0;reg_1e_22_end=0;reg_1e_174_end=0;reg_1e_175_end=0;
	status_flag=0;

	P9_r50_cha_comp=0;P9_r50_chb_comp=0;P9_r50_chc_comp=0;P9_r50_chd_comp=0;
	P10_r50_cha_comp=0;P10_r50_chb_comp=0;P10_r50_chc_comp=0;P10_r50_chd_comp=0;
	P11_r50_cha_comp=0;P11_r50_chb_comp=0;P11_r50_chc_comp=0;P11_r50_chd_comp=0;
	P12_r50_cha_comp=0;P12_r50_chb_comp=0;P12_r50_chc_comp=0;P12_r50_chd_comp=0;

	if((TXMR_or_discrete == 0)&& (mdi_resister==5)) 
	{	
	if(phy == 9) 
	{
		    // test mode TX amp setting
			P9_tx_amp_tst_cha_comp=0;P9_tx_amp_tst_chb_comp=0;P9_tx_amp_tst_chc_comp=0;P9_tx_amp_tst_chd_comp=0;
			// Giga TX amp setting
			P9_tx_amp_gbe_cha_comp=0;P9_tx_amp_gbe_chb_comp=0;P9_tx_amp_gbe_chc_comp=0;P9_tx_amp_gbe_chd_comp=0;
			// 100M TX amp setting
			P9_tx_amp_hbt_cha_comp=0;P9_tx_amp_hbt_chb_comp=0;
			// 10M TX amp setting
			P9_tx_amp_tbt_cha_comp=0;P9_tx_amp_tbt_chb_comp=0;
			// R50 setting
			P9_r50_cha_comp=0;P9_r50_chb_comp=0;P9_r50_chc_comp=0;P9_r50_chd_comp=0;
	}
	
	if(phy == 10) 
	{
		    // test mode TX amp setting
			P10_tx_amp_tst_cha_comp=0;P10_tx_amp_tst_chb_comp=0;P10_tx_amp_tst_chc_comp=0;P10_tx_amp_tst_chd_comp=0;
			// Giga TX amp setting
			P10_tx_amp_gbe_cha_comp=0;P10_tx_amp_gbe_chb_comp=0;P10_tx_amp_gbe_chc_comp=0;P10_tx_amp_gbe_chd_comp=0;
			// 100M TX amp setting
			P10_tx_amp_hbt_cha_comp=0;P10_tx_amp_hbt_chb_comp=0;
			// 10M TX amp setting
			P10_tx_amp_tbt_cha_comp=0;P10_tx_amp_tbt_chb_comp=0;
			// R50 setting
			P10_r50_cha_comp=0;P10_r50_chb_comp=0;P10_r50_chc_comp=0;P10_r50_chd_comp=0;
	}
	if(phy == 11) 
	{
		    // test mode TX amp setting
			P11_tx_amp_tst_cha_comp=0;P11_tx_amp_tst_chb_comp=0;P11_tx_amp_tst_chc_comp=0;P11_tx_amp_tst_chd_comp=0;
			// Giga TX amp setting
			P11_tx_amp_gbe_cha_comp=0;P11_tx_amp_gbe_chb_comp=0;P11_tx_amp_gbe_chc_comp=0;P11_tx_amp_gbe_chd_comp=0;
			// 100M TX amp setting
			P11_tx_amp_hbt_cha_comp=0;P11_tx_amp_hbt_chb_comp=0;
			// 10M TX amp setting
			P11_tx_amp_tbt_cha_comp=0;P11_tx_amp_tbt_chb_comp=0;
			// R50 setting
			P11_r50_cha_comp=0;P11_r50_chb_comp=0;P11_r50_chc_comp=0;P11_r50_chd_comp=0;
	}
	if(phy == 12) 
	{
		    // test mode TX amp setting
			P12_tx_amp_tst_cha_comp=0;P12_tx_amp_tst_chb_comp=0;P12_tx_amp_tst_chc_comp=0;P12_tx_amp_tst_chd_comp=0;
			// Giga TX amp setting
			P12_tx_amp_gbe_cha_comp=0;P12_tx_amp_gbe_chb_comp=0;P12_tx_amp_gbe_chc_comp=0;P12_tx_amp_gbe_chd_comp=0;
			// 100M TX amp setting
			P12_tx_amp_hbt_cha_comp=0;P12_tx_amp_hbt_chb_comp=0;
			// 10M TX amp setting
			P12_tx_amp_tbt_cha_comp=0;P12_tx_amp_tbt_chb_comp=0;
			// R50 setting
			P12_r50_cha_comp=0;P12_r50_chb_comp=0;P12_r50_chc_comp=0;P12_r50_chd_comp=0;
	}
	}


    //TXMR_or_discrete=0-->discrete TXMR_or_discrete=1-->TXMR
	if((TXMR_or_discrete == 1)&& (mdi_resister==5)) 
	{	
	if(phy == 9) 
	{
		    // test mode TX amp setting
			P9_tx_amp_tst_cha_comp=0;P9_tx_amp_tst_chb_comp=-0;P9_tx_amp_tst_chc_comp=0;P9_tx_amp_tst_chd_comp=0;
			// Giga TX amp setting
			P9_tx_amp_gbe_cha_comp=0;P9_tx_amp_gbe_chb_comp=-0;P9_tx_amp_gbe_chc_comp=0;P9_tx_amp_gbe_chd_comp=0;
			// 100M TX amp setting
			P9_tx_amp_hbt_cha_comp=0;P9_tx_amp_hbt_chb_comp=0;
			// 10M TX amp setting
			P9_tx_amp_tbt_cha_comp=0;P9_tx_amp_tbt_chb_comp=0;
			// R50 setting
			P9_r50_cha_comp=0;P9_r50_chb_comp=0;P9_r50_chc_comp=0;P9_r50_chd_comp=0;
	}
	
	if(phy == 10) 
	{
		    // test mode TX amp setting
			P10_tx_amp_tst_cha_comp=0;P10_tx_amp_tst_chb_comp=0;P10_tx_amp_tst_chc_comp=0;P10_tx_amp_tst_chd_comp=0;
			// Giga TX amp setting
			P10_tx_amp_gbe_cha_comp=0;P10_tx_amp_gbe_chb_comp=0;P10_tx_amp_gbe_chc_comp=0;P10_tx_amp_gbe_chd_comp=0;
			// 100M TX amp setting
			P10_tx_amp_hbt_cha_comp=0;P10_tx_amp_hbt_chb_comp=0;
			// 10M TX amp setting
			P10_tx_amp_tbt_cha_comp=0;P10_tx_amp_tbt_chb_comp=0;
			// R50 setting
			P10_r50_cha_comp=0;P10_r50_chb_comp=0;P10_r50_chc_comp=0;P10_r50_chd_comp=0;
	}
	if(phy == 11) 
	{
		    // test mode TX amp setting
			P11_tx_amp_tst_cha_comp=0;P11_tx_amp_tst_chb_comp=0;P11_tx_amp_tst_chc_comp=0;P11_tx_amp_tst_chd_comp=0;
			// Giga TX amp setting
			P11_tx_amp_gbe_cha_comp=0;P11_tx_amp_gbe_chb_comp=0;P11_tx_amp_gbe_chc_comp=0;P11_tx_amp_gbe_chd_comp=0;
			// 100M TX amp setting
			P11_tx_amp_hbt_cha_comp=0;P11_tx_amp_hbt_chb_comp=0;
			// 10M TX amp setting
			P11_tx_amp_tbt_cha_comp=0;P11_tx_amp_tbt_chb_comp=0;
			// R50 setting
			P11_r50_cha_comp=0;P11_r50_chb_comp=0;P11_r50_chc_comp=0;P11_r50_chd_comp=0;
	}
	if(phy == 12) 
	{
		    // test mode TX amp setting
			P12_tx_amp_tst_cha_comp=0;P12_tx_amp_tst_chb_comp=0;P12_tx_amp_tst_chc_comp=0;P12_tx_amp_tst_chd_comp=0;
			// Giga TX amp setting
			P12_tx_amp_gbe_cha_comp=0;P12_tx_amp_gbe_chb_comp=0;P12_tx_amp_gbe_chc_comp=0;P12_tx_amp_gbe_chd_comp=0;
			// 100M TX amp setting
			P12_tx_amp_hbt_cha_comp=0;P12_tx_amp_hbt_chb_comp=0;
			// 10M TX amp setting
			P12_tx_amp_tbt_cha_comp=0;P12_tx_amp_tbt_chb_comp=0;
			// R50 setting
			P12_r50_cha_comp=0;P12_r50_chb_comp=0;P12_r50_chc_comp=0;P12_r50_chd_comp=0;
	}
	}

  
	if(show_or_comp == 0)
	{
	if(phy == 9) 
	{
			printk("port9A  tst= %d " , P9_tx_amp_tst_cha_comp);
			printk("port9B  tst= %d " , P9_tx_amp_tst_chb_comp);
			printk("port9C  tst= %d " , P9_tx_amp_tst_chc_comp);
			printk("port9D  tst= %d \n\r" , P9_tx_amp_tst_chd_comp);
			printk("port9A  gbe= %d " , P9_tx_amp_gbe_cha_comp);
			printk("port9B  gbe= %d " , P9_tx_amp_gbe_chb_comp);
			printk("port9C  gbe= %d " , P9_tx_amp_gbe_chc_comp);
			printk("port9D  gbe= %d \n\r" , P9_tx_amp_gbe_chd_comp);
			printk("port9A  hbt= %d " , P9_tx_amp_hbt_cha_comp);
			printk("port9B  hbt= %d " , P9_tx_amp_hbt_chb_comp);
			printk("port9A  tbt= %d " , P9_tx_amp_tbt_cha_comp);
			printk("port9B  tbt= %d \n\r" , P9_tx_amp_tbt_chb_comp);
			printk("port9A  R50= %d   " , P9_r50_cha_comp);
			printk("port9B  R50= %d   " , P9_r50_chb_comp);
			printk("port9C  R50= %d   " , P9_r50_chc_comp);
			printk("port9D  R50= %d \n\r" , P9_r50_chd_comp);
	}
	if(phy == 10) 
	{
			printk("port10A tst= %d " , P10_tx_amp_tst_cha_comp);
			printk("port10B tst= %d " , P10_tx_amp_tst_chb_comp);
			printk("port10C tst= %d " , P10_tx_amp_tst_chc_comp);
			printk("port10D tst= %d \n\r" , P10_tx_amp_tst_chd_comp);
			printk("port10A gbe= %d " , P10_tx_amp_gbe_cha_comp);
			printk("port10B gbe= %d " , P10_tx_amp_gbe_chb_comp);
			printk("port10C gbe= %d " , P10_tx_amp_gbe_chc_comp);
			printk("port10D gbe= %d \n\r" , P10_tx_amp_gbe_chd_comp);
			printk("port10A hbt= %d " , P10_tx_amp_hbt_cha_comp);
			printk("port10B hbt= %d " , P10_tx_amp_hbt_chb_comp);
			printk("port10A tbt= %d " , P10_tx_amp_tbt_cha_comp);
			printk("port10B tbt= %d \n\r" , P10_tx_amp_tbt_chb_comp);
			printk("port10A R50= %d   " , P10_r50_cha_comp);
			printk("port10B R50= %d   " , P10_r50_chb_comp);
			printk("port10C R50= %d   " , P10_r50_chc_comp);
			printk("port10D R50= %d \n\r" , P10_r50_chd_comp);
	}
	if(phy == 11) 
	{			
			printk("port11A tst= %d " , P11_tx_amp_tst_cha_comp);
			printk("port11B tst= %d " , P11_tx_amp_tst_chb_comp);
			printk("port11C tst= %d " , P11_tx_amp_tst_chc_comp);
			printk("port11D tst= %d \n\r" , P11_tx_amp_tst_chd_comp);
			printk("port11A gbe= %d " , P11_tx_amp_gbe_cha_comp);
			printk("port11B gbe= %d " , P11_tx_amp_gbe_chb_comp);
			printk("port11C gbe= %d " , P11_tx_amp_gbe_chc_comp);
			printk("port11D gbe= %d \n\r" , P11_tx_amp_gbe_chd_comp);
			printk("port11A hbt= %d " , P11_tx_amp_hbt_cha_comp);
			printk("port11B hbt= %d " , P11_tx_amp_hbt_chb_comp);
			printk("port11A tbt= %d " , P11_tx_amp_tbt_cha_comp);
			printk("port11B tbt= %d \n\r" , P11_tx_amp_tbt_chb_comp);
			printk("port11A R50= %d   " , P11_r50_cha_comp);
			printk("port11B R50= %d   " , P11_r50_chb_comp);
			printk("port11C R50= %d   " , P11_r50_chc_comp);
			printk("port11D R50= %d \n\r" , P11_r50_chd_comp);
	}
			
	if(phy == 12) 
	{
			printk("port12A tst= %d " , P12_tx_amp_tst_cha_comp);
			printk("port12B tst= %d " , P12_tx_amp_tst_chb_comp);
			printk("port12C tst= %d " , P12_tx_amp_tst_chc_comp);
			printk("port12D tst= %d \n\r" , P12_tx_amp_tst_chd_comp);
			printk("port12A gbe= %d " , P12_tx_amp_gbe_cha_comp);
			printk("port12B gbe= %d " , P12_tx_amp_gbe_chb_comp);
			printk("port12C gbe= %d " , P12_tx_amp_gbe_chc_comp);
			printk("port12D gbe= %d \n\r" , P12_tx_amp_gbe_chd_comp);
			printk("port12A hbt= %d " , P12_tx_amp_hbt_cha_comp);
			printk("port12B hbt= %d " , P12_tx_amp_hbt_chb_comp);
			printk("port12A tbt= %d " , P12_tx_amp_tbt_cha_comp);
			printk("port12B tbt= %d \n\r" , P12_tx_amp_tbt_chb_comp);
			printk("port12A R50= %d   " , P12_r50_cha_comp);
			printk("port12B R50= %d   " , P12_r50_chb_comp);
			printk("port12C R50= %d   " , P12_r50_chc_comp);
			printk("port12D R50= %d \n\r" , P12_r50_chd_comp);
	}
            return 0;			
	
	}

			//read out 
	reg_1e_12_read = mtEMiiRegRead(phy, 0x1e, 0x12);
	reg_1e_16_read = mtEMiiRegRead(phy, 0x1e, 0x16);
	reg_1e_17_read = mtEMiiRegRead(phy, 0x1e, 0x17);
	reg_1e_18_read = mtEMiiRegRead(phy, 0x1e, 0x18);
	reg_1e_19_read = mtEMiiRegRead(phy, 0x1e, 0x19);
	reg_1e_20_read = mtEMiiRegRead(phy, 0x1e, 0x20);
	reg_1e_21_read = mtEMiiRegRead(phy, 0x1e, 0x21);
	reg_1e_22_read = mtEMiiRegRead(phy, 0x1e, 0x22);
	reg_1e_174_read = mtEMiiRegRead(phy, 0x1e, 0x174);
	reg_1e_175_read = mtEMiiRegRead(phy, 0x1e, 0x175);
	printk("port = %d \r\n" , phy);
	printk("1e_12 = 0x%x " , reg_1e_12_read);
	printk("1e_16 = 0x%x " , reg_1e_16_read);
	printk("1e_17 = 0x%x " , reg_1e_17_read);
	printk("1e_18 = 0x%x " , reg_1e_18_read);
	printk("1e_19 = 0x%x " , reg_1e_19_read);
	printk("1e_20 = 0x%x " , reg_1e_20_read);
	printk("1e_21 = 0x%x " , reg_1e_21_read);
	printk("1e_22 = 0x%x \r\n" , reg_1e_22_read);
	printk("1e_174 = 0x%x " , reg_1e_174_read);
	printk("1e_175 = 0x%x \r\n" , reg_1e_175_read);
	if(phy == 9)
	{				
                status_flag=0;
				reg_1e_12_header= ((reg_1e_12_read&0xfc00)/1024)+P9_tx_amp_gbe_cha_comp;
				reg_1e_17_header= ((reg_1e_17_read&0x3f00)/256)+P9_tx_amp_gbe_chb_comp;			
				reg_1e_19_header= ((reg_1e_19_read&0x3f00)/256)+P9_tx_amp_gbe_chc_comp;					
				reg_1e_21_header= ((reg_1e_21_read&0x3f00)/256)+P9_tx_amp_gbe_chd_comp;
				if((reg_1e_12_header < txamp_low_limit)||(reg_1e_12_header > txamp_high_limit)) status_flag=1;
				if((reg_1e_17_header < txamp_low_limit)||(reg_1e_17_header > txamp_high_limit)) status_flag=1;
				if((reg_1e_19_header < txamp_low_limit)||(reg_1e_19_header > txamp_high_limit)) status_flag=1;
				if((reg_1e_21_header < txamp_low_limit)||(reg_1e_21_header > txamp_high_limit)) status_flag=1;
				if(status_flag == 1) {reg_1e_12_header=0x20;reg_1e_17_header=0x20;reg_1e_19_header=0x20;reg_1e_21_header=0x20;}
				else {
				  if(reg_1e_12_header < 0) {reg_1e_12_header=0;}
				  if(reg_1e_12_header > 0x3f)	{reg_1e_12_header=0x3f;}
				  if(reg_1e_17_header < 0) {reg_1e_17_header=0;}
				  if(reg_1e_17_header > 0x3f)	{reg_1e_17_header=0x3f;}
				  if(reg_1e_19_header < 0) {reg_1e_19_header=0;}
				  if(reg_1e_19_header > 0x3f)	{reg_1e_19_header=0x3f;}
				  if(reg_1e_21_header < 0) {reg_1e_21_header=0;}
				  if(reg_1e_21_header > 0x3f)	{reg_1e_21_header=0x3f;}
				}
				status_flag=0;
				reg_1e_16_end= (reg_1e_16_read&0x003f)+P9_tx_amp_tst_cha_comp;
				reg_1e_18_end= (reg_1e_18_read&0x003f)+P9_tx_amp_tst_chb_comp;
				reg_1e_20_end= (reg_1e_20_read&0x003f)+P9_tx_amp_tst_chc_comp;
				reg_1e_22_end= (reg_1e_22_read&0x003f)+P9_tx_amp_tst_chd_comp;						
				if((reg_1e_16_end < txamp_low_limit)||(reg_1e_16_end > txamp_high_limit)) status_flag=1;
				if((reg_1e_18_end < txamp_low_limit)||(reg_1e_18_end > txamp_high_limit)) status_flag=1;
				if((reg_1e_20_end < txamp_low_limit)||(reg_1e_20_end > txamp_high_limit)) status_flag=1;
				if((reg_1e_22_end < txamp_low_limit)||(reg_1e_22_end > txamp_high_limit)) status_flag=1;
				if(status_flag == 1) {reg_1e_16_end=0x20;reg_1e_18_end=0x20;reg_1e_20_end=0x20;reg_1e_22_end=0x20;}
				else {
				  if(reg_1e_16_end < 0) {reg_1e_16_end=0;}
				  if(reg_1e_16_end > 0x3f)	{reg_1e_16_end=0x3f;}
				  if(reg_1e_18_end < 0) {reg_1e_18_end=0;}
				  if(reg_1e_18_end > 0x3f)	{reg_1e_18_end=0x3f;}
				  if(reg_1e_20_end < 0) {reg_1e_20_end=0;}
				  if(reg_1e_20_end > 0x3f)	{reg_1e_20_end=0x3f;}
				  if(reg_1e_22_end < 0) {reg_1e_22_end=0;}
				  if(reg_1e_22_end > 0x3f)	{reg_1e_22_end=0x3f;}
				}
				status_flag=0;
				reg_1e_16_header= ((reg_1e_16_read&0xfc00)/1024)+P9_tx_amp_hbt_cha_comp;
				reg_1e_18_header= ((reg_1e_18_read&0x3f00)/256)+P9_tx_amp_hbt_chb_comp;
				if((reg_1e_16_header < txamp_low_limit)||(reg_1e_16_header > txamp_high_limit)) status_flag=1;
				if((reg_1e_18_header < txamp_low_limit)||(reg_1e_18_header > txamp_high_limit)) status_flag=1;
				if(status_flag == 1) {reg_1e_16_header=0x20;reg_1e_18_header=0x20;}
				else {
				  if(reg_1e_16_header < 0) {reg_1e_16_header=0;}
				  if(reg_1e_16_header > 0x3f)	{reg_1e_16_header=0x3f;}
				  if(reg_1e_18_header < 0) {reg_1e_18_header=0;}
				  if(reg_1e_18_header > 0x3f)	{reg_1e_18_header=0x3f;}
				}
				status_flag=0;
				reg_1e_12_end= (reg_1e_12_read&0x003f)+P9_tx_amp_tbt_cha_comp;
				reg_1e_17_end= (reg_1e_17_read&0x003f)+P9_tx_amp_tbt_chb_comp;
				if((reg_1e_12_end < txamp_low_limit)||(reg_1e_12_end > txamp_high_limit)) status_flag=1;
				if((reg_1e_17_end < txamp_low_limit)||(reg_1e_17_end > txamp_high_limit)) status_flag=1;
				if(status_flag == 1) {reg_1e_12_end=0x20;reg_1e_17_end=0x20;}
				else {
				  if(reg_1e_12_end < 0) {reg_1e_12_end=0;}
				  if(reg_1e_12_end > 0x3f)	{reg_1e_12_end=0x3f;}
				  if(reg_1e_17_end < 0) {reg_1e_17_end=0;}
				  if(reg_1e_17_end > 0x3f)	{reg_1e_17_end=0x3f;}
				}
                reg_1e_12_write=(reg_1e_12_header*1024)+reg_1e_12_end;
				reg_1e_16_write=(reg_1e_16_header*1024)+reg_1e_16_end;
				reg_1e_17_write=(reg_1e_17_header*256)+reg_1e_17_end;
				reg_1e_18_write=(reg_1e_18_header*256)+reg_1e_18_end;
				reg_1e_19_write=(reg_1e_19_read&0x00ff)+(reg_1e_19_header*256);
				reg_1e_20_write=(reg_1e_20_read&0xff00)+reg_1e_20_end;
				reg_1e_21_write=(reg_1e_21_read&0x00ff)+(reg_1e_21_header*256);
				reg_1e_22_write=(reg_1e_22_read&0xff00)+reg_1e_22_end;
				//r50
				status_flag=0;
				reg_1e_174_header= ((reg_1e_174_read&0x7f00)/256)+P9_r50_cha_comp;
				reg_1e_174_end= (reg_1e_174_read&0x007f)+P9_r50_chb_comp;
				reg_1e_175_header= ((reg_1e_175_read&0x7f00)/256)+P9_r50_chc_comp;
				reg_1e_175_end= (reg_1e_175_read&0x007f)+P9_r50_chd_comp;
				if((reg_1e_174_header < r50_low_limit)||(reg_1e_174_header >r50_high_limit)) status_flag=1;
				if((reg_1e_174_end < r50_low_limit)||(reg_1e_174_end >r50_high_limit)) status_flag=1;
				if((reg_1e_175_header < r50_low_limit)||(reg_1e_175_header >r50_high_limit)) status_flag=1;
				if((reg_1e_175_end < r50_low_limit)||(reg_1e_175_end >r50_high_limit)) status_flag=1;	
				if(status_flag == 1) {	reg_1e_174_header=0x34;reg_1e_174_end=0x34;reg_1e_175_header=0x34;reg_1e_175_end=0x34;}
                else 
				{
					if(reg_1e_174_header< 0) { reg_1e_174_header=0;}
					if(reg_1e_174_header > 0x7f) { reg_1e_174_header=0x7f;}
					if(reg_1e_174_end< 0) { reg_1e_174_end=0;}
					if(reg_1e_174_end > 0x7f) { reg_1e_174_end=0x7f;}
					if(reg_1e_175_header< 0) { reg_1e_175_header=0;}
					if(reg_1e_175_header > 0x7f) { reg_1e_175_header=0x7f;}
					if(reg_1e_175_end< 0) { reg_1e_175_end=0;}
					if(reg_1e_175_end > 0x7f) { reg_1e_175_end=0x7f;}					
				}
				reg_1e_175_write=0x8080+(reg_1e_175_header*256)+reg_1e_175_end;
				reg_1e_174_write=0x8080+(reg_1e_174_header*256)+reg_1e_174_end;
	}
	//calculate phy 10 
	if(phy == 10)
	{
                status_flag=0;
				reg_1e_12_header= ((reg_1e_12_read&0xfc00)/1024)+P10_tx_amp_gbe_cha_comp;
				reg_1e_17_header= ((reg_1e_17_read&0x3f00)/256)+P10_tx_amp_gbe_chb_comp;
				reg_1e_19_header= ((reg_1e_19_read&0x3f00)/256)+P10_tx_amp_gbe_chc_comp;
				reg_1e_21_header= ((reg_1e_21_read&0x3f00)/256)+P10_tx_amp_gbe_chd_comp;
				if((reg_1e_12_header < txamp_low_limit)||(reg_1e_12_header > txamp_high_limit)) status_flag=1;
				if((reg_1e_17_header < txamp_low_limit)||(reg_1e_17_header > txamp_high_limit)) status_flag=1;
				if((reg_1e_19_header < txamp_low_limit)||(reg_1e_19_header > txamp_high_limit)) status_flag=1;
				if((reg_1e_21_header < txamp_low_limit)||(reg_1e_21_header > txamp_high_limit)) status_flag=1;
				if(status_flag == 1) {reg_1e_12_header=0x20;reg_1e_17_header=0x20;reg_1e_19_header=0x20;reg_1e_21_header=0x20;}
				else {
				  if(reg_1e_12_header < 0) {reg_1e_12_header=0;}
				  if(reg_1e_12_header > 0x3f)	{reg_1e_12_header=0x3f;}
				  if(reg_1e_17_header < 0) {reg_1e_17_header=0;}
				  if(reg_1e_17_header > 0x3f)	{reg_1e_17_header=0x3f;}
				  if(reg_1e_19_header < 0) {reg_1e_19_header=0;}
				  if(reg_1e_19_header > 0x3f)	{reg_1e_19_header=0x3f;}
				  if(reg_1e_21_header < 0) {reg_1e_21_header=0;}
				  if(reg_1e_21_header > 0x3f)	{reg_1e_21_header=0x3f;}
				}
				status_flag=0;
				reg_1e_16_end= (reg_1e_16_read&0x003f)+P10_tx_amp_tst_cha_comp;
				reg_1e_18_end= (reg_1e_18_read&0x003f)+P10_tx_amp_tst_chb_comp;
				reg_1e_20_end= (reg_1e_20_read&0x003f)+P10_tx_amp_tst_chc_comp;
				reg_1e_22_end= (reg_1e_22_read&0x003f)+P10_tx_amp_tst_chd_comp;
				if((reg_1e_16_end < txamp_low_limit)||(reg_1e_16_end > txamp_high_limit)) status_flag=1;
				if((reg_1e_18_end < txamp_low_limit)||(reg_1e_18_end > txamp_high_limit)) status_flag=1;
				if((reg_1e_20_end < txamp_low_limit)||(reg_1e_20_end > txamp_high_limit)) status_flag=1;
				if((reg_1e_22_end < txamp_low_limit)||(reg_1e_22_end > txamp_high_limit)) status_flag=1;
				if(status_flag == 1) {reg_1e_16_end=0x20;reg_1e_18_end=0x20;reg_1e_20_end=0x20;reg_1e_22_end=0x20;}
				else {
				  if(reg_1e_16_end < 0) {reg_1e_16_end=0;}
				  if(reg_1e_16_end > 0x3f)	{reg_1e_16_end=0x3f;}
				  if(reg_1e_18_end < 0) {reg_1e_18_end=0;}
				  if(reg_1e_18_end > 0x3f)	{reg_1e_18_end=0x3f;}
				  if(reg_1e_20_end < 0) {reg_1e_20_end=0;}
				  if(reg_1e_20_end > 0x3f)	{reg_1e_20_end=0x3f;}
				  if(reg_1e_22_end < 0) {reg_1e_22_end=0;}
				  if(reg_1e_22_end > 0x3f)	{reg_1e_22_end=0x3f;}
				}
				status_flag=0;
				reg_1e_16_header= ((reg_1e_16_read&0xfc00)/1024)+P10_tx_amp_hbt_cha_comp;
				reg_1e_18_header= ((reg_1e_18_read&0x3f00)/256)+P10_tx_amp_hbt_chb_comp;
				if((reg_1e_16_header < txamp_low_limit)||(reg_1e_16_header > txamp_high_limit)) status_flag=1;
				if((reg_1e_18_header < txamp_low_limit)||(reg_1e_18_header > txamp_high_limit)) status_flag=1;
				if(status_flag==1) {reg_1e_16_header=0x20;reg_1e_18_header=0x20;}
				else {
				  if(reg_1e_16_header < 0) {reg_1e_16_header=0;}
				  if(reg_1e_16_header > 0x3f)	{reg_1e_16_header=0x3f;}
				  if(reg_1e_18_header < 0) {reg_1e_18_header=0;}
				  if(reg_1e_18_header > 0x3f)	{reg_1e_18_header=0x3f;}	
				}
				status_flag=0;
				reg_1e_12_end= (reg_1e_12_read&0x003f)+P10_tx_amp_tbt_cha_comp;
				reg_1e_17_end= (reg_1e_17_read&0x003f)+P10_tx_amp_tbt_chb_comp;
				if((reg_1e_12_end < txamp_low_limit)||(reg_1e_12_end > txamp_high_limit)) status_flag=1;
				if((reg_1e_17_end < txamp_low_limit)||(reg_1e_17_end > txamp_high_limit)) status_flag=1;
				if(status_flag == 1) {reg_1e_12_end=0x20;reg_1e_17_end=0x20;}
				else {
				  if(reg_1e_12_end < 0) {reg_1e_12_end=0;}
				  if(reg_1e_12_end > 0x3f)	{reg_1e_12_end=0x3f;}
				  if(reg_1e_17_end < 0) {reg_1e_17_end=0;}
				  if(reg_1e_17_end > 0x3f)	{reg_1e_17_end=0x3f;}
				}
                reg_1e_12_write=(reg_1e_12_header*1024)+reg_1e_12_end;
				reg_1e_16_write=(reg_1e_16_header*1024)+reg_1e_16_end;
				reg_1e_17_write=(reg_1e_17_header*256)+reg_1e_17_end;
				reg_1e_18_write=(reg_1e_18_header*256)+reg_1e_18_end;
				reg_1e_19_write=(reg_1e_19_read&0x00ff)+(reg_1e_19_header*256);
				reg_1e_20_write=(reg_1e_20_read&0xff00)+reg_1e_20_end;
				reg_1e_21_write=(reg_1e_21_read&0x00ff)+(reg_1e_21_header*256);
				reg_1e_22_write=(reg_1e_22_read&0xff00)+reg_1e_22_end;
				//r50
				status_flag=0;
				reg_1e_174_header= ((reg_1e_174_read&0x7f00)/256)+P10_r50_cha_comp;
				reg_1e_174_end= (reg_1e_174_read&0x007f)+P10_r50_chb_comp;
				reg_1e_175_header= ((reg_1e_175_read&0x7f00)/256)+P10_r50_chc_comp;
				reg_1e_175_end= (reg_1e_175_read&0x007f)+P10_r50_chd_comp;
				if((reg_1e_174_header < r50_low_limit)||(reg_1e_174_header >r50_high_limit)) status_flag=1;
				if((reg_1e_174_end < r50_low_limit)||(reg_1e_174_end >r50_high_limit)) status_flag=1;
				if((reg_1e_175_header < r50_low_limit)||(reg_1e_175_header >r50_high_limit)) status_flag=1;
				if((reg_1e_175_end < r50_low_limit)||(reg_1e_175_end >r50_high_limit)) status_flag=1;	
				if(status_flag == 1) {	reg_1e_174_header=0x34;reg_1e_174_end=0x34;reg_1e_175_header=0x34;reg_1e_175_end=0x34;}
                else 
				{
					if(reg_1e_174_header< 0) { reg_1e_174_header=0;}
					if(reg_1e_174_header > 0x7f) { reg_1e_174_header=0x7f;}
					if(reg_1e_174_end< 0) { reg_1e_174_end=0;}
					if(reg_1e_174_end > 0x7f) { reg_1e_174_end=0x7f;}
					if(reg_1e_175_header< 0) { reg_1e_175_header=0;}
					if(reg_1e_175_header > 0x7f) { reg_1e_175_header=0x7f;}
					if(reg_1e_175_end< 0) { reg_1e_175_end=0;}
					if(reg_1e_175_end > 0x7f) { reg_1e_175_end=0x7f;}					
				}
				reg_1e_175_write=0x8080+(reg_1e_175_header*256)+reg_1e_175_end;
				reg_1e_174_write=0x8080+(reg_1e_174_header*256)+reg_1e_174_end;
	}
    //calculate phy 11
	if(phy == 11)
	{
                status_flag=0;
				reg_1e_12_header= ((reg_1e_12_read&0xfc00)/1024)+P11_tx_amp_gbe_cha_comp;
				reg_1e_17_header= ((reg_1e_17_read&0x3f00)/256)+P11_tx_amp_gbe_chb_comp;
				reg_1e_19_header= ((reg_1e_19_read&0x3f00)/256)+P11_tx_amp_gbe_chc_comp;
				reg_1e_21_header= ((reg_1e_21_read&0x3f00)/256)+P11_tx_amp_gbe_chd_comp;
				if((reg_1e_12_header < txamp_low_limit)||(reg_1e_12_header > txamp_high_limit)) status_flag=1;
				if((reg_1e_17_header < txamp_low_limit)||(reg_1e_17_header > txamp_high_limit)) status_flag=1;
				if((reg_1e_19_header < txamp_low_limit)||(reg_1e_19_header > txamp_high_limit)) status_flag=1;
				if((reg_1e_21_header < txamp_low_limit)||(reg_1e_21_header > txamp_high_limit)) status_flag=1;
				if(status_flag == 1) {reg_1e_12_header=0x20;reg_1e_17_header=0x20;reg_1e_19_header=0x20;reg_1e_21_header=0x20;}
				else {
				  if(reg_1e_12_header < 0) {reg_1e_12_header=0;}
				  if(reg_1e_12_header > 0x3f)	{reg_1e_12_header=0x3f;}
				  if(reg_1e_17_header < 0) {reg_1e_17_header=0;}
				  if(reg_1e_17_header > 0x3f)	{reg_1e_17_header=0x3f;}
				  if(reg_1e_19_header < 0) {reg_1e_19_header=0;}
				  if(reg_1e_19_header > 0x3f)	{reg_1e_19_header=0x3f;}
				  if(reg_1e_21_header < 0) {reg_1e_21_header=0;}
				  if(reg_1e_21_header > 0x3f)	{reg_1e_21_header=0x3f;}
				}
				status_flag=0;
				reg_1e_16_end= (reg_1e_16_read&0x003f)+P11_tx_amp_tst_cha_comp;
				reg_1e_18_end= (reg_1e_18_read&0x003f)+P11_tx_amp_tst_chb_comp;
				reg_1e_20_end= (reg_1e_20_read&0x003f)+P11_tx_amp_tst_chc_comp;
				reg_1e_22_end= (reg_1e_22_read&0x003f)+P11_tx_amp_tst_chd_comp;
				if((reg_1e_16_end < txamp_low_limit)||(reg_1e_16_end > txamp_high_limit)) status_flag=1;
				if((reg_1e_18_end < txamp_low_limit)||(reg_1e_18_end > txamp_high_limit)) status_flag=1;
				if((reg_1e_20_end < txamp_low_limit)||(reg_1e_20_end > txamp_high_limit)) status_flag=1;
				if((reg_1e_22_end < txamp_low_limit)||(reg_1e_22_end > txamp_high_limit)) status_flag=1;
				if(status_flag==1) {reg_1e_16_end=0x20;reg_1e_18_end=0x20;reg_1e_20_end=0x20;reg_1e_22_end=0x20;}
				else {
				if(reg_1e_16_end < 0) {reg_1e_16_end=0;}
				if(reg_1e_16_end > 0x3f)	{reg_1e_16_end=0x3f;}
				if(reg_1e_18_end < 0) {reg_1e_18_end=0;}
				if(reg_1e_18_end > 0x3f)	{reg_1e_18_end=0x3f;}
				if(reg_1e_20_end < 0) {reg_1e_20_end=0;}
				if(reg_1e_20_end > 0x3f)	{reg_1e_20_end=0x3f;}
				if(reg_1e_22_end < 0) {reg_1e_22_end=0;}
				if(reg_1e_22_end > 0x3f)	{reg_1e_22_end=0x3f;}
				}
				status_flag=0;
				reg_1e_16_header= ((reg_1e_16_read&0xfc00)/1024)+P11_tx_amp_hbt_cha_comp;
				reg_1e_18_header= ((reg_1e_18_read&0x3f00)/256)+P11_tx_amp_hbt_chb_comp;
				if((reg_1e_16_header < txamp_low_limit)||(reg_1e_16_header > txamp_high_limit)) status_flag=1;
				if((reg_1e_18_header < txamp_low_limit)||(reg_1e_18_header > txamp_high_limit)) status_flag=1;
				if(status_flag==1) {reg_1e_16_header=0x20;reg_1e_18_header=0x20;}
				else {
				if(reg_1e_16_header < 0) {reg_1e_16_header=0;}
				if(reg_1e_16_header > 0x3f)	{reg_1e_16_header=0x3f;}
				if(reg_1e_18_header < 0) {reg_1e_18_header=0;}
				if(reg_1e_18_header > 0x3f)	{reg_1e_18_header=0x3f;}
				}
				status_flag=0;
				reg_1e_12_end= (reg_1e_12_read&0x003f)+P11_tx_amp_tbt_cha_comp;
				reg_1e_17_end= (reg_1e_17_read&0x003f)+P11_tx_amp_tbt_chb_comp;
				if((reg_1e_12_end < txamp_low_limit)||(reg_1e_12_end > txamp_high_limit)) status_flag=1;
				if((reg_1e_17_end < txamp_low_limit)||(reg_1e_17_end > txamp_high_limit)) status_flag=1;
				if(status_flag == 1) {reg_1e_12_end=0x20;reg_1e_17_end=0x20;}
				else {
				if(reg_1e_12_end < 0) {reg_1e_12_end=0;}
				if(reg_1e_12_end > 0x3f)	{reg_1e_12_end=0x3f;}
				if(reg_1e_17_end < 0) {reg_1e_17_end=0;}
				if(reg_1e_17_end > 0x3f)	{reg_1e_17_end=0x3f;}
				}
                reg_1e_12_write=(reg_1e_12_header*1024)+reg_1e_12_end;
				reg_1e_16_write=(reg_1e_16_header*1024)+reg_1e_16_end;
				reg_1e_17_write=(reg_1e_17_header*256)+reg_1e_17_end;
				reg_1e_18_write=(reg_1e_18_header*256)+reg_1e_18_end;
				reg_1e_19_write=(reg_1e_19_read&0x00ff)+(reg_1e_19_header*256);
				reg_1e_20_write=(reg_1e_20_read&0xff00)+reg_1e_20_end;
				reg_1e_21_write=(reg_1e_21_read&0x00ff)+(reg_1e_21_header*256);
				reg_1e_22_write=(reg_1e_22_read&0xff00)+reg_1e_22_end;
				//r50
				status_flag=0;
				reg_1e_174_header= ((reg_1e_174_read&0x7f00)/256)+P11_r50_cha_comp;
				reg_1e_174_end= (reg_1e_174_read&0x007f)+P11_r50_chb_comp;
				reg_1e_175_header= ((reg_1e_175_read&0x7f00)/256)+P11_r50_chc_comp;
				reg_1e_175_end= (reg_1e_175_read&0x007f)+P11_r50_chd_comp;
				if((reg_1e_174_header < r50_low_limit)||(reg_1e_174_header >r50_high_limit)) status_flag=1;
				if((reg_1e_174_end < r50_low_limit)||(reg_1e_174_end >r50_high_limit)) status_flag=1;
				if((reg_1e_175_header < r50_low_limit)||(reg_1e_175_header >r50_high_limit)) status_flag=1;
				if((reg_1e_175_end < r50_low_limit)||(reg_1e_175_end >r50_high_limit)) status_flag=1;
				if(status_flag == 1) {	reg_1e_174_header=0x34;reg_1e_174_end=0x34;reg_1e_175_header=0x34;reg_1e_175_end=0x34;}
                else 
				{
					if(reg_1e_174_header< 0) { reg_1e_174_header=0;}
					if(reg_1e_174_header > 0x7f) { reg_1e_174_header=0x7f;}
					if(reg_1e_174_end< 0) { reg_1e_174_end=0;}
					if(reg_1e_174_end > 0x7f) { reg_1e_174_end=0x7f;}
					if(reg_1e_175_header< 0) { reg_1e_175_header=0;}
					if(reg_1e_175_header > 0x7f) { reg_1e_175_header=0x7f;}
					if(reg_1e_175_end< 0) { reg_1e_175_end=0;}
					if(reg_1e_175_end > 0x7f) { reg_1e_175_end=0x7f;}					
				}
				reg_1e_175_write=0x8080+(reg_1e_175_header*256)+reg_1e_175_end;
				reg_1e_174_write=0x8080+(reg_1e_174_header*256)+reg_1e_174_end;
	}
    //calculate phy 12
	if(phy == 12)
	{
                status_flag=0;
				reg_1e_12_header= ((reg_1e_12_read&0xfc00)/1024)+P12_tx_amp_gbe_cha_comp;
				reg_1e_17_header= ((reg_1e_17_read&0x3f00)/256)+P12_tx_amp_gbe_chb_comp;
				reg_1e_19_header= ((reg_1e_19_read&0x3f00)/256)+P12_tx_amp_gbe_chc_comp;
				reg_1e_21_header= ((reg_1e_21_read&0x3f00)/256)+P12_tx_amp_gbe_chd_comp;
				if((reg_1e_12_header < txamp_low_limit)||(reg_1e_12_header > txamp_high_limit)) status_flag=1;
				if((reg_1e_17_header < txamp_low_limit)||(reg_1e_17_header > txamp_high_limit)) status_flag=1;
				if((reg_1e_19_header < txamp_low_limit)||(reg_1e_19_header > txamp_high_limit)) status_flag=1;
				if((reg_1e_21_header < txamp_low_limit)||(reg_1e_21_header > txamp_high_limit)) status_flag=1;
				if(status_flag==1) {reg_1e_12_header=0x20;reg_1e_17_header=0x20;reg_1e_19_header=0x20;reg_1e_21_header=0x20;}
				else {
				if(reg_1e_12_header < 0) {reg_1e_12_header=0;}
				if(reg_1e_12_header > 0x3f)	{reg_1e_12_header=0x3f;}
				if(reg_1e_17_header < 0) {reg_1e_17_header=0;}
				if(reg_1e_17_header > 0x3f)	{reg_1e_17_header=0x3f;}
				if(reg_1e_19_header < 0) {reg_1e_19_header=0;}
				if(reg_1e_19_header > 0x3f)	{reg_1e_19_header=0x3f;}
				if(reg_1e_21_header < 0) {reg_1e_21_header=0;}
				if(reg_1e_21_header > 0x3f)	{reg_1e_21_header=0x3f;}
				}
				status_flag=0;
				reg_1e_16_end= (reg_1e_16_read&0x003f)+P12_tx_amp_tst_cha_comp;
				reg_1e_18_end= (reg_1e_18_read&0x003f)+P12_tx_amp_tst_chb_comp;
				reg_1e_20_end= (reg_1e_20_read&0x003f)+P12_tx_amp_tst_chc_comp;
				reg_1e_22_end= (reg_1e_22_read&0x003f)+P12_tx_amp_tst_chd_comp;
				if((reg_1e_16_end < txamp_low_limit)||(reg_1e_16_end > txamp_high_limit)) status_flag=1;
				if((reg_1e_18_end < txamp_low_limit)||(reg_1e_18_end > txamp_high_limit)) status_flag=1;
				if((reg_1e_20_end < txamp_low_limit)||(reg_1e_20_end > txamp_high_limit)) status_flag=1;
				if((reg_1e_22_end < txamp_low_limit)||(reg_1e_22_end > txamp_high_limit)) status_flag=1;
				if(status_flag==1) {reg_1e_16_end=0x20;reg_1e_18_end=0x20;reg_1e_20_end=0x20;reg_1e_22_end=0x20;}
				else {
				if(reg_1e_16_end < 0) {reg_1e_16_end=0;}
				if(reg_1e_16_end > 0x3f)	{reg_1e_16_end=0x3f;}
				if(reg_1e_18_end < 0) {reg_1e_18_end=0;}
				if(reg_1e_18_end > 0x3f)	{reg_1e_18_end=0x3f;}
				if(reg_1e_20_end < 0) {reg_1e_20_end=0;}
				if(reg_1e_20_end > 0x3f)	{reg_1e_20_end=0x3f;}
				if(reg_1e_22_end < 0) {reg_1e_22_end=0;}
				if(reg_1e_22_end > 0x3f)	{reg_1e_22_end=0x3f;}
				}
				status_flag=0;
				reg_1e_16_header= ((reg_1e_16_read&0xfc00)/1024)+P12_tx_amp_hbt_cha_comp;
				reg_1e_18_header= ((reg_1e_18_read&0x3f00)/256)+P12_tx_amp_hbt_chb_comp;
				if((reg_1e_16_header < txamp_low_limit)||(reg_1e_16_header > txamp_high_limit)) status_flag=1;
				if((reg_1e_18_header < txamp_low_limit)||(reg_1e_18_header > txamp_high_limit)) status_flag=1;
				if(status_flag==1) {reg_1e_16_header=0x20;reg_1e_18_header=0x20;}
				else {
				if(reg_1e_16_header < 0) {reg_1e_16_header=0;}
				if(reg_1e_16_header > 0x3f)	{reg_1e_16_header=0x3f;}
				if(reg_1e_18_header < 0) {reg_1e_18_header=0;}
				if(reg_1e_18_header > 0x3f)	{reg_1e_18_header=0x3f;}
				}
				status_flag=0;
				reg_1e_12_end= (reg_1e_12_read&0x003f)+P12_tx_amp_tbt_cha_comp;
				reg_1e_17_end= (reg_1e_17_read&0x003f)+P12_tx_amp_tbt_chb_comp;
				if((reg_1e_12_end < txamp_low_limit)||(reg_1e_12_end > txamp_high_limit)) status_flag=1;
				if((reg_1e_17_end < txamp_low_limit)||(reg_1e_17_end > txamp_high_limit)) status_flag=1;
				if(status_flag == 1) {reg_1e_12_end=0x20;reg_1e_17_end=0x20;}
				else {
				if(reg_1e_12_end < 0) {reg_1e_12_end=0;}
				if(reg_1e_12_end > 0x3f)	{reg_1e_12_end=0x3f;}
				if(reg_1e_17_end < 0) {reg_1e_17_end=0;}
				if(reg_1e_17_end > 0x3f)	{reg_1e_17_end=0x3f;}
				}
                reg_1e_12_write=(reg_1e_12_header*1024)+reg_1e_12_end;
				reg_1e_16_write=(reg_1e_16_header*1024)+reg_1e_16_end;
				reg_1e_17_write=(reg_1e_17_header*256)+reg_1e_17_end;
				reg_1e_18_write=(reg_1e_18_header*256)+reg_1e_18_end;
				reg_1e_19_write=(reg_1e_19_read&0x00ff)+(reg_1e_19_header*256);
				reg_1e_20_write=(reg_1e_20_read&0xff00)+reg_1e_20_end;
				reg_1e_21_write=(reg_1e_21_read&0x00ff)+(reg_1e_21_header*256);
				reg_1e_22_write=(reg_1e_22_read&0xff00)+reg_1e_22_end;
				//r50
				status_flag=0;
				reg_1e_174_header= ((reg_1e_174_read&0x7f00)/256)+P12_r50_cha_comp;
				reg_1e_174_end= (reg_1e_174_read&0x007f)+P12_r50_chb_comp;
				reg_1e_175_header= ((reg_1e_175_read&0x7f00)/256)+P12_r50_chc_comp;
				reg_1e_175_end= (reg_1e_175_read&0x007f)+P12_r50_chd_comp;
				if((reg_1e_174_header < r50_low_limit)||(reg_1e_174_header >r50_high_limit)) status_flag=1;
				if((reg_1e_174_end < r50_low_limit)||(reg_1e_174_end >r50_high_limit)) status_flag=1;
				if((reg_1e_175_header < r50_low_limit)||(reg_1e_175_header >r50_high_limit)) status_flag=1;
				if((reg_1e_175_end < r50_low_limit)||(reg_1e_175_end >r50_high_limit)) status_flag=1;	
				if(status_flag == 1) {	reg_1e_174_header=0x34;reg_1e_174_end=0x34;reg_1e_175_header=0x34;reg_1e_175_end=0x34;}
                else 
				{
					if(reg_1e_174_header< 0) { reg_1e_174_header=0;}
					if(reg_1e_174_header > 0x7f) { reg_1e_174_header=0x7f;}
					if(reg_1e_174_end< 0) { reg_1e_174_end=0;}
					if(reg_1e_174_end > 0x7f) { reg_1e_174_end=0x7f;}
					if(reg_1e_175_header< 0) { reg_1e_175_header=0;}
					if(reg_1e_175_header > 0x7f) { reg_1e_175_header=0x7f;}
					if(reg_1e_175_end< 0) { reg_1e_175_end=0;}
					if(reg_1e_175_end > 0x7f) { reg_1e_175_end=0x7f;}					
				}
				reg_1e_175_write=0x8080+(reg_1e_175_header*256)+reg_1e_175_end;
				reg_1e_174_write=0x8080+(reg_1e_174_header*256)+reg_1e_174_end;
	}	
    printk("1e_12 = 0x%x " , reg_1e_12_write);
    printk("1e_16 = 0x%x " , reg_1e_16_write);
    printk("1e_17 = 0x%x " , reg_1e_17_write);
    printk("1e_18 = 0x%x " , reg_1e_18_write);
    printk("1e_19 = 0x%x " , reg_1e_19_write);
    printk("1e_20 = 0x%x " , reg_1e_20_write);
    printk("1e_21 = 0x%x " , reg_1e_21_write);
	printk("1e_22 = 0x%x \r\n" , reg_1e_22_write);
	printk("1e_174 = 0x%x " , reg_1e_174_write);
	printk("1e_175 = 0x%x \r\n" , reg_1e_175_write);

	//write back reg
	mtEMiiRegWrite(phy, 0x1e, 0x0012, reg_1e_12_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0016, reg_1e_16_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0017, reg_1e_17_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0018, reg_1e_18_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0019, reg_1e_19_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0020, reg_1e_20_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0021, reg_1e_21_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0022, reg_1e_22_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0174, reg_1e_174_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0175, reg_1e_175_write);
	//TXMR_or_discrete=0-->discrete TXMR_or_discrete=1-->TXMR
	if((TXMR_or_discrete == 0)&& (mdi_resister==5)) 
	{	
                
	}
	if((TXMR_or_discrete == 1)&& (mdi_resister==5)) 
	{	
				
				//mtEMiiRegWrite(phy, 0x1e, 0x0176, 0x5500);
				//mtEMiiRegWrite(phy, 0x1e, 0x0177, 0x0055);
	}
	return 0;		
}


*/

const int	AMP_TBL[4][4][16] =
{
 //transformer_type type 0
 //(TXMR_or_discrete == 0)&& (mdi_resister==5)) 
 {
  {         //phy9 
   -3,-3,-3,-3, //tx_amp_tst_cha_comp tx_amp_tst_chb_comp tx_amp_tst_chc_comp tx_amp_tst_chd_comp 
   0,0,0,0, //tx_amp_gbe_cha_comp tx_amp_gbe_chb_comp tx_amp_gbe_chc_comp tx_amp_gbe_chd_comp
   4,4,     //tx_amp_hbt_cha_comp tx_amp_hbt_chb_comp
   0,0,     //tx_amp_tbt_cha_comp tx_amp_tbt_chb_comp
   0,0,0,0  //r50_cha_comp r50_chb_comp r50_chc_comp r50_chd_comp
  },
  {         //phy10   
   -3,-3,-3,-3, //tx_amp_tst_cha_comp tx_amp_tst_chb_comp tx_amp_tst_chc_comp tx_amp_tst_chd_comp 
   0,0,0,0, //tx_amp_gbe_cha_comp tx_amp_gbe_chb_comp tx_amp_gbe_chc_comp tx_amp_gbe_chd_comp
   4,4,     //tx_amp_hbt_cha_comp tx_amp_hbt_chb_comp
   0,0,     //tx_amp_tbt_cha_comp tx_amp_tbt_chb_comp
   0,0,0,0  //r50_cha_comp r50_chb_comp r50_chc_comp r50_chd_comp
  },
  {         //phy11   
   -3,-3,-3,-3, //tx_amp_tst_cha_comp tx_amp_tst_chb_comp tx_amp_tst_chc_comp tx_amp_tst_chd_comp 
   0,0,0,0, //tx_amp_gbe_cha_comp tx_amp_gbe_chb_comp tx_amp_gbe_chc_comp tx_amp_gbe_chd_comp
   4,4,     //tx_amp_hbt_cha_comp tx_amp_hbt_chb_comp
   0,0,     //tx_amp_tbt_cha_comp tx_amp_tbt_chb_comp
   0,0,0,0  //r50_cha_comp r50_chb_comp r50_chc_comp r50_chd_comp
  },
  {
            //phy12
   -3,-3,-3,-3, //tx_amp_tst_cha_comp tx_amp_tst_chb_comp tx_amp_tst_chc_comp tx_amp_tst_chd_comp 
   0,0,0,0, //tx_amp_gbe_cha_comp tx_amp_gbe_chb_comp tx_amp_gbe_chc_comp tx_amp_gbe_chd_comp
   4,4,     //tx_amp_hbt_cha_comp tx_amp_hbt_chb_comp
   0,0,     //tx_amp_tbt_cha_comp tx_amp_tbt_chb_comp
   0,0,0,0  //r50_cha_comp r50_chb_comp r50_chc_comp r50_chd_comp
  }
 },

//transformer_type type 1
//(TXMR_or_discrete == 1)&& (mdi_resister==5)) 
 {
  {         //phy9 
   -3,-3,-3,-3, //tx_amp_tst_cha_comp tx_amp_tst_chb_comp tx_amp_tst_chc_comp tx_amp_tst_chd_comp 
   0,0,0,0, //tx_amp_gbe_cha_comp tx_amp_gbe_chb_comp tx_amp_gbe_chc_comp tx_amp_gbe_chd_comp
   4,4,     //tx_amp_hbt_cha_comp tx_amp_hbt_chb_comp
   0,0,     //tx_amp_tbt_cha_comp tx_amp_tbt_chb_comp
   -4,-4,-2,-3  //r50_cha_comp r50_chb_comp r50_chc_comp r50_chd_comp
  },
  {         //phy10   
   -3,-3,-3,-3, //tx_amp_tst_cha_comp tx_amp_tst_chb_comp tx_amp_tst_chc_comp tx_amp_tst_chd_comp 
   0,0,0,0, //tx_amp_gbe_cha_comp tx_amp_gbe_chb_comp tx_amp_gbe_chc_comp tx_amp_gbe_chd_comp
   4,4,     //tx_amp_hbt_cha_comp tx_amp_hbt_chb_comp
   0,0,     //tx_amp_tbt_cha_comp tx_amp_tbt_chb_comp
   -4,-4,-2,-3  //r50_cha_comp r50_chb_comp r50_chc_comp r50_chd_comp
  },
  {         //phy11   
   -3,-3,-3,-3, //tx_amp_tst_cha_comp tx_amp_tst_chb_comp tx_amp_tst_chc_comp tx_amp_tst_chd_comp 
   0,0,0,0, //tx_amp_gbe_cha_comp tx_amp_gbe_chb_comp tx_amp_gbe_chc_comp tx_amp_gbe_chd_comp
   4,4,     //tx_amp_hbt_cha_comp tx_amp_hbt_chb_comp
   0,0,     //tx_amp_tbt_cha_comp tx_amp_tbt_chb_comp
   -4,-4,-2,-2  //r50_cha_comp r50_chb_comp r50_chc_comp r50_chd_comp
  },
  {
            //phy12
   -3,-3,-3,-3, //tx_amp_tst_cha_comp tx_amp_tst_chb_comp tx_amp_tst_chc_comp tx_amp_tst_chd_comp 
   0,0,0,0, //tx_amp_gbe_cha_comp tx_amp_gbe_chb_comp tx_amp_gbe_chc_comp tx_amp_gbe_chd_comp
   4,4,     //tx_amp_hbt_cha_comp tx_amp_hbt_chb_comp
   0,0,     //tx_amp_tbt_cha_comp tx_amp_tbt_chb_comp
   -4,-4,-2,-2  //r50_cha_comp r50_chb_comp r50_chc_comp r50_chd_comp
  }
 },
 
//transformer_type type 2
//(TXMR_or_discrete == 0)&& (mdi_resister==0)) 
 {
  {         //phy9 
   -3,-3,-3,-3, //tx_amp_tst_cha_comp tx_amp_tst_chb_comp tx_amp_tst_chc_comp tx_amp_tst_chd_comp 
   0,0,0,0, //tx_amp_gbe_cha_comp tx_amp_gbe_chb_comp tx_amp_gbe_chc_comp tx_amp_gbe_chd_comp
   4,4,     //tx_amp_hbt_cha_comp tx_amp_hbt_chb_comp
   0,0,     //tx_amp_tbt_cha_comp tx_amp_tbt_chb_comp
   4,4,6,4  //r50_cha_comp r50_chb_comp r50_chc_comp r50_chd_comp
  },
  {         //phy10   
   -3,-3,-3,-3, //tx_amp_tst_cha_comp tx_amp_tst_chb_comp tx_amp_tst_chc_comp tx_amp_tst_chd_comp 
   0,0,0,0, //tx_amp_gbe_cha_comp tx_amp_gbe_chb_comp tx_amp_gbe_chc_comp tx_amp_gbe_chd_comp
   4,4,     //tx_amp_hbt_cha_comp tx_amp_hbt_chb_comp
   0,0,     //tx_amp_tbt_cha_comp tx_amp_tbt_chb_comp
   4,4,6,4  //r50_cha_comp r50_chb_comp r50_chc_comp r50_chd_comp
  },
  {         //phy11   
   -3,-3,-3,-3, //tx_amp_tst_cha_comp tx_amp_tst_chb_comp tx_amp_tst_chc_comp tx_amp_tst_chd_comp 
   0,0,0,0, //tx_amp_gbe_cha_comp tx_amp_gbe_chb_comp tx_amp_gbe_chc_comp tx_amp_gbe_chd_comp
   4,4,     //tx_amp_hbt_cha_comp tx_amp_hbt_chb_comp
   0,0,     //tx_amp_tbt_cha_comp tx_amp_tbt_chb_comp
   4,4,6,4  //r50_cha_comp r50_chb_comp r50_chc_comp r50_chd_comp
  },
  {
            //phy12
   -3,-3,-3,-3, //tx_amp_tst_cha_comp tx_amp_tst_chb_comp tx_amp_tst_chc_comp tx_amp_tst_chd_comp 
   0,0,0,0, //tx_amp_gbe_cha_comp tx_amp_gbe_chb_comp tx_amp_gbe_chc_comp tx_amp_gbe_chd_comp
   4,4,     //tx_amp_hbt_cha_comp tx_amp_hbt_chb_comp
   0,0,     //tx_amp_tbt_cha_comp tx_amp_tbt_chb_comp
   4,4,6,4  //r50_cha_comp r50_chb_comp r50_chc_comp r50_chd_comp
  }
 }, 
//transformer_type type 3
//(TXMR_or_discrete == 1)&& (mdi_resister==0)) 
 {
  {         //phy9 
   -3,-3,-3,-3, //tx_amp_tst_cha_comp tx_amp_tst_chb_comp tx_amp_tst_chc_comp tx_amp_tst_chd_comp 
   0,0,0,0, //tx_amp_gbe_cha_comp tx_amp_gbe_chb_comp tx_amp_gbe_chc_comp tx_amp_gbe_chd_comp
   4,4,     //tx_amp_hbt_cha_comp tx_amp_hbt_chb_comp
   0,0,     //tx_amp_tbt_cha_comp tx_amp_tbt_chb_comp
   4,4,5,4  //r50_cha_comp r50_chb_comp r50_chc_comp r50_chd_comp
  },
  {         //phy10   
   -3,-3,-3,-3, //tx_amp_tst_cha_comp tx_amp_tst_chb_comp tx_amp_tst_chc_comp tx_amp_tst_chd_comp 
   0,0,0,0, //tx_amp_gbe_cha_comp tx_amp_gbe_chb_comp tx_amp_gbe_chc_comp tx_amp_gbe_chd_comp
   4,4,     //tx_amp_hbt_cha_comp tx_amp_hbt_chb_comp
   0,0,     //tx_amp_tbt_cha_comp tx_amp_tbt_chb_comp
   4,4,5,4  //r50_cha_comp r50_chb_comp r50_chc_comp r50_chd_comp
  },
  {         //phy11   
   -3,-3,-3,-3, //tx_amp_tst_cha_comp tx_amp_tst_chb_comp tx_amp_tst_chc_comp tx_amp_tst_chd_comp 
   0,0,0,0, //tx_amp_gbe_cha_comp tx_amp_gbe_chb_comp tx_amp_gbe_chc_comp tx_amp_gbe_chd_comp
   4,4,     //tx_amp_hbt_cha_comp tx_amp_hbt_chb_comp
   0,0,     //tx_amp_tbt_cha_comp tx_amp_tbt_chb_comp
   0,0,3,4  //r50_cha_comp r50_chb_comp r50_chc_comp r50_chd_comp
  },
  {
            //phy12
   -3,-3,-3,-3, //tx_amp_tst_cha_comp tx_amp_tst_chb_comp tx_amp_tst_chc_comp tx_amp_tst_chd_comp 
   0,0,0,0, //tx_amp_gbe_cha_comp tx_amp_gbe_chb_comp tx_amp_gbe_chc_comp tx_amp_gbe_chd_comp
   4,4,     //tx_amp_hbt_cha_comp tx_amp_hbt_chb_comp
   0,0,     //tx_amp_tbt_cha_comp tx_amp_tbt_chb_comp
   0,0,4,0  //r50_cha_comp r50_chb_comp r50_chc_comp r50_chd_comp
  }
 }
};

int phyTxAmpcomp(int show_or_comp,unsigned int phy, unsigned int TXMR_or_discrete)
{ 	
	int transformer_type,count;	
	int txamp_low_limit,txamp_high_limit;
	int r50_low_limit,r50_high_limit; 

	unsigned int  reg_1e_12_read,reg_1e_16_read,reg_1e_17_read,reg_1e_18_read,reg_1e_19_read,reg_1e_20_read,reg_1e_21_read,reg_1e_22_read,reg_1e_174_read,reg_1e_175_read;
    unsigned int  reg_1e_12_write,reg_1e_16_write,reg_1e_17_write,reg_1e_18_write,reg_1e_19_write,reg_1e_20_write,reg_1e_21_write,reg_1e_22_write,reg_1e_174_write,reg_1e_175_write;
	int  reg_1e_12_header,reg_1e_16_header,reg_1e_17_header,reg_1e_18_header,reg_1e_19_header,reg_1e_21_header,reg_1e_174_header,reg_1e_175_header;
	int  reg_1e_12_end,reg_1e_16_end,reg_1e_17_end,reg_1e_18_end,reg_1e_20_end,reg_1e_22_end,reg_1e_174_end,reg_1e_175_end;
	int  status_flag;
	//TXMR_or_discrete=0-->discrete TXMR_or_discrete=1-->TXMR

	txamp_low_limit=-3;txamp_high_limit=0x42;
   r50_low_limit=-8;r50_high_limit=0x88; 

	reg_1e_12_read=0;reg_1e_16_read=0;reg_1e_17_read=0;reg_1e_18_read=0;reg_1e_19_read=0;reg_1e_20_read=0;reg_1e_21_read=0;reg_1e_22_read=0;reg_1e_174_read=0;reg_1e_175_read=0;
    reg_1e_12_write=0;reg_1e_16_write=0;reg_1e_17_write=0;reg_1e_18_write=0;reg_1e_19_write=0;reg_1e_20_write=0;reg_1e_21_write=0;reg_1e_22_write=0;reg_1e_174_write=0;reg_1e_175_write=0;
	reg_1e_12_header=0;reg_1e_16_header=0;reg_1e_17_header=0;reg_1e_18_header=0;reg_1e_19_header=0;reg_1e_21_header=0;reg_1e_174_header=0;reg_1e_175_header=0;
	reg_1e_12_end=0;reg_1e_16_end=0;reg_1e_17_end=0;reg_1e_18_end=0;reg_1e_20_end=0;reg_1e_22_end=0;reg_1e_174_end=0;reg_1e_175_end=0;
	status_flag=0;

    //TXMR_or_discrete=0-->discrete TXMR_or_discrete=1-->TXMR
	transformer_type =0;
	if((TXMR_or_discrete == 0)&& (mdi_resister==5)) 
	{
	   transformer_type =0;
	}
	if((TXMR_or_discrete == 1)&& (mdi_resister==5)) 
	{	
	  transformer_type  =1;
	}
	if((TXMR_or_discrete == 0)&& (mdi_resister==0)) 
	{	
	  transformer_type	=2;
	}	
	if((TXMR_or_discrete == 1)&& (mdi_resister==0)) 
	{	
	  transformer_type	=3;
	}
	printk("TX AMP comp tble transformer_type=%d,Phy port =%d \r\n",transformer_type,phy);
	printk("TX AMP comp table:\r\n");
	for (count =0; count <16;count++)
	{
  	  printk("(%d)",AMP_TBL[transformer_type][phy-9][count]);	  
	}
	printk("\r\n");
	//read out 
	reg_1e_12_read = mtEMiiRegRead(phy, 0x1e, 0x12);
	reg_1e_16_read = mtEMiiRegRead(phy, 0x1e, 0x16);
	reg_1e_17_read = mtEMiiRegRead(phy, 0x1e, 0x17);
	reg_1e_18_read = mtEMiiRegRead(phy, 0x1e, 0x18);
	reg_1e_19_read = mtEMiiRegRead(phy, 0x1e, 0x19);
	reg_1e_20_read = mtEMiiRegRead(phy, 0x1e, 0x20);
	reg_1e_21_read = mtEMiiRegRead(phy, 0x1e, 0x21);
	reg_1e_22_read = mtEMiiRegRead(phy, 0x1e, 0x22);
	reg_1e_174_read = mtEMiiRegRead(phy, 0x1e, 0x174);
	reg_1e_175_read = mtEMiiRegRead(phy, 0x1e, 0x175);
	//printk("port = %d \r\n" , phy);
	printk("1e_12 = 0x%x " , reg_1e_12_read);
	printk("1e_16 = 0x%x " , reg_1e_16_read);
	printk("1e_17 = 0x%x " , reg_1e_17_read);
	printk("1e_18 = 0x%x " , reg_1e_18_read);
	printk("1e_19 = 0x%x " , reg_1e_19_read);
	printk("1e_20 = 0x%x " , reg_1e_20_read);
	printk("1e_21 = 0x%x " , reg_1e_21_read);
	printk("1e_22 = 0x%x \r\n" , reg_1e_22_read);
	printk("1e_174 = 0x%x " , reg_1e_174_read);
	printk("1e_175 = 0x%x \r\n" , reg_1e_175_read);
			
    status_flag=0;
	//gbe
	reg_1e_12_header= ((reg_1e_12_read&0xfc00)/1024)+AMP_TBL[transformer_type][phy-9][4];
	reg_1e_17_header= ((reg_1e_17_read&0x3f00)/256)+AMP_TBL[transformer_type][phy-9][5];			
	reg_1e_19_header= ((reg_1e_19_read&0x3f00)/256)+AMP_TBL[transformer_type][phy-9][6];				
	reg_1e_21_header= ((reg_1e_21_read&0x3f00)/256)+AMP_TBL[transformer_type][phy-9][7];
	if((reg_1e_12_header < txamp_low_limit)||(reg_1e_12_header > txamp_high_limit)) status_flag=1;
	if((reg_1e_17_header < txamp_low_limit)||(reg_1e_17_header > txamp_high_limit)) status_flag=1;
	if((reg_1e_19_header < txamp_low_limit)||(reg_1e_19_header > txamp_high_limit)) status_flag=1;
	if((reg_1e_21_header < txamp_low_limit)||(reg_1e_21_header > txamp_high_limit)) status_flag=1;
	if(status_flag == 1) {reg_1e_12_header=0x20;reg_1e_17_header=0x20;reg_1e_19_header=0x20;reg_1e_21_header=0x20;}
	else {
	  if(reg_1e_12_header < 0) {reg_1e_12_header=0;}
	  if(reg_1e_12_header > 0x3f)	{reg_1e_12_header=0x3f;}
	  if(reg_1e_17_header < 0) {reg_1e_17_header=0;}
	  if(reg_1e_17_header > 0x3f)	{reg_1e_17_header=0x3f;}
	  if(reg_1e_19_header < 0) {reg_1e_19_header=0;}
	  if(reg_1e_19_header > 0x3f)	{reg_1e_19_header=0x3f;}
	  if(reg_1e_21_header < 0) {reg_1e_21_header=0;}
	  if(reg_1e_21_header > 0x3f)	{reg_1e_21_header=0x3f;}
	}
	status_flag=0;
	//tst
	reg_1e_16_end= (reg_1e_16_read&0x003f)+AMP_TBL[transformer_type][phy-9][0];
	reg_1e_18_end= (reg_1e_18_read&0x003f)+AMP_TBL[transformer_type][phy-9][1];
	reg_1e_20_end= (reg_1e_20_read&0x003f)+AMP_TBL[transformer_type][phy-9][2];
	reg_1e_22_end= (reg_1e_22_read&0x003f)+AMP_TBL[transformer_type][phy-9][3];						
	if((reg_1e_16_end < txamp_low_limit)||(reg_1e_16_end > txamp_high_limit)) status_flag=1;
	if((reg_1e_18_end < txamp_low_limit)||(reg_1e_18_end > txamp_high_limit)) status_flag=1;
	if((reg_1e_20_end < txamp_low_limit)||(reg_1e_20_end > txamp_high_limit)) status_flag=1;
	if((reg_1e_22_end < txamp_low_limit)||(reg_1e_22_end > txamp_high_limit)) status_flag=1;
	if(status_flag == 1) {reg_1e_16_end=0x20;reg_1e_18_end=0x20;reg_1e_20_end=0x20;reg_1e_22_end=0x20;}
	else {
	  if(reg_1e_16_end < 0) {reg_1e_16_end=0;}
	  if(reg_1e_16_end > 0x3f)	{reg_1e_16_end=0x3f;}
	  if(reg_1e_18_end < 0) {reg_1e_18_end=0;}
	  if(reg_1e_18_end > 0x3f)	{reg_1e_18_end=0x3f;}
	  if(reg_1e_20_end < 0) {reg_1e_20_end=0;}
	  if(reg_1e_20_end > 0x3f)	{reg_1e_20_end=0x3f;}
	  if(reg_1e_22_end < 0) {reg_1e_22_end=0;}
	  if(reg_1e_22_end > 0x3f)	{reg_1e_22_end=0x3f;}
	}
	status_flag=0;
	//hbt
	reg_1e_16_header= ((reg_1e_16_read&0xfc00)/1024)+AMP_TBL[transformer_type][phy-9][8];	
	reg_1e_18_header= ((reg_1e_18_read&0x3f00)/256)+AMP_TBL[transformer_type][phy-9][9];	
	if((reg_1e_16_header < txamp_low_limit)||(reg_1e_16_header > txamp_high_limit)) status_flag=1;
	if((reg_1e_18_header < txamp_low_limit)||(reg_1e_18_header > txamp_high_limit)) status_flag=1;
	if(status_flag == 1) {reg_1e_16_header=0x20;reg_1e_18_header=0x20;}
	else {
	  if(reg_1e_16_header < 0) {reg_1e_16_header=0;}
	  if(reg_1e_16_header > 0x3f)	{reg_1e_16_header=0x3f;}
	  if(reg_1e_18_header < 0) {reg_1e_18_header=0;}
	  if(reg_1e_18_header > 0x3f)	{reg_1e_18_header=0x3f;}
	}
	status_flag=0;
	//tbt
	reg_1e_12_end= (reg_1e_12_read&0x003f)+AMP_TBL[transformer_type][phy-9][10];
	reg_1e_17_end= (reg_1e_17_read&0x003f)+AMP_TBL[transformer_type][phy-9][11];
	if((reg_1e_12_end < txamp_low_limit)||(reg_1e_12_end > txamp_high_limit)) status_flag=1;
	if((reg_1e_17_end < txamp_low_limit)||(reg_1e_17_end > txamp_high_limit)) status_flag=1;
	if(status_flag == 1) {reg_1e_12_end=0x20;reg_1e_17_end=0x20;}
	else {
	  if(reg_1e_12_end < 0) {reg_1e_12_end=0;}
	  if(reg_1e_12_end > 0x3f)	{reg_1e_12_end=0x3f;}
	  if(reg_1e_17_end < 0) {reg_1e_17_end=0;}
	  if(reg_1e_17_end > 0x3f)	{reg_1e_17_end=0x3f;}
	}
   reg_1e_12_write=(reg_1e_12_header*1024)+reg_1e_12_end;
	reg_1e_16_write=(reg_1e_16_header*1024)+reg_1e_16_end;
	reg_1e_17_write=(reg_1e_17_header*256)+reg_1e_17_end;
	reg_1e_18_write=(reg_1e_18_header*256)+reg_1e_18_end;
	reg_1e_19_write=(reg_1e_19_read&0x00ff)+(reg_1e_19_header*256);
	reg_1e_20_write=(reg_1e_20_read&0xff00)+reg_1e_20_end;
	reg_1e_21_write=(reg_1e_21_read&0x00ff)+(reg_1e_21_header*256);
	reg_1e_22_write=(reg_1e_22_read&0xff00)+reg_1e_22_end;
	//r50
	status_flag=0;
	//r50_ch
	reg_1e_174_header= ((reg_1e_174_read&0x7f00)/256)+AMP_TBL[transformer_type][phy-9][12];
	reg_1e_174_end= (reg_1e_174_read&0x007f)+AMP_TBL[transformer_type][phy-9][13];;
	reg_1e_175_header= ((reg_1e_175_read&0x7f00)/256)+AMP_TBL[transformer_type][phy-9][14];;
	reg_1e_175_end= (reg_1e_175_read&0x007f)+AMP_TBL[transformer_type][phy-9][15];;
	if((reg_1e_174_header < r50_low_limit)||(reg_1e_174_header >r50_high_limit)) status_flag=1;
	if((reg_1e_174_end < r50_low_limit)||(reg_1e_174_end >r50_high_limit)) status_flag=1;
	if((reg_1e_175_header < r50_low_limit)||(reg_1e_175_header >r50_high_limit)) status_flag=1;
	if((reg_1e_175_end < r50_low_limit)||(reg_1e_175_end >r50_high_limit)) status_flag=1;	
	if(status_flag == 1) {	reg_1e_174_header=0x34;reg_1e_174_end=0x34;reg_1e_175_header=0x34;reg_1e_175_end=0x34;}
   else 
	{
		if(reg_1e_174_header< 0) { reg_1e_174_header=0;}
		if(reg_1e_174_header > 0x7f) { reg_1e_174_header=0x7f;}
		if(reg_1e_174_end< 0) { reg_1e_174_end=0;}
		if(reg_1e_174_end > 0x7f) { reg_1e_174_end=0x7f;}
		if(reg_1e_175_header< 0) { reg_1e_175_header=0;}
		if(reg_1e_175_header > 0x7f) { reg_1e_175_header=0x7f;}
		if(reg_1e_175_end< 0) { reg_1e_175_end=0;}
		if(reg_1e_175_end > 0x7f) { reg_1e_175_end=0x7f;}					
	}
	reg_1e_175_write=0x8080+(reg_1e_175_header*256)+reg_1e_175_end;
	reg_1e_174_write=0x8080+(reg_1e_174_header*256)+reg_1e_174_end;

   printk("1e_12 = 0x%x " , reg_1e_12_write);
   printk("1e_16 = 0x%x " , reg_1e_16_write);
   printk("1e_17 = 0x%x " , reg_1e_17_write);
   printk("1e_18 = 0x%x " , reg_1e_18_write);
   printk("1e_19 = 0x%x " , reg_1e_19_write);
   printk("1e_20 = 0x%x " , reg_1e_20_write);
   printk("1e_21 = 0x%x " , reg_1e_21_write);
	printk("1e_22 = 0x%x \r\n" , reg_1e_22_write);
	printk("1e_174 = 0x%x " , reg_1e_174_write);
	printk("1e_175 = 0x%x \r\n" , reg_1e_175_write);

	//write back reg
	mtEMiiRegWrite(phy, 0x1e, 0x0012, reg_1e_12_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0016, reg_1e_16_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0017, reg_1e_17_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0018, reg_1e_18_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0019, reg_1e_19_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0020, reg_1e_20_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0021, reg_1e_21_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0022, reg_1e_22_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0174, reg_1e_174_write);
	mtEMiiRegWrite(phy, 0x1e, 0x0175, reg_1e_175_write);
	//TXMR_or_discrete=0-->discrete TXMR_or_discrete=1-->TXMR
	if((TXMR_or_discrete == 0)&& (mdi_resister==5)) 
	{	
                
	}
	if((TXMR_or_discrete == 1)&& (mdi_resister==5)) 
	{	
				
				//mtEMiiRegWrite(phy, 0x1e, 0x0176, 0x5500);
				//mtEMiiRegWrite(phy, 0x1e, 0x0177, 0x0055);
	}	
	if((TXMR_or_discrete == 0)&& (mdi_resister==0)) 
	{	

	}
	
	if((TXMR_or_discrete == 1)&& (mdi_resister==0)) 
	{	

	}
	return 0;		
}


void    normalinit(void)
{
	u16 ephy_addr;
	unsigned int transformer_temp=0;


	
	
	for(ephy_addr = all_port_start; ephy_addr <= all_port_end; ephy_addr++)
		{
         if(ephy_addr == 9) 
		 	{
				transformer_temp=transformer_status&0x01;
				if (idle_mode_power_saving)
					{
						mtEMiiRegWrite(ephy_addr, 0x1f, 0x268, 0x07F1);
						mtEMiiRegWrite(ephy_addr, 0x1f, 0x703, 0x3111);
						mtEMiiRegWrite(ephy_addr, 0x1f, 0x271, 0x3C24);
						mtEMiiRegWrite(ephy_addr, 0x1f, 0x701, 0x1023);
					}
		 	}
         if(ephy_addr == 10) {transformer_temp=transformer_status&0x02;}
         if(ephy_addr == 11) {transformer_temp=transformer_status&0x04;}
         if(ephy_addr == 12) {transformer_temp=transformer_status&0x08;}
		//
	    if(transformer_temp == 0) 
		{	
		    if(mdi_resister == 5)
            {   
                printk(" 100M D5R\r\n "); 
				
				//RX setting for 5R_TXMR before AN setting
				mtEMiiRegWrite(ephy_addr, 0x1e, 0xe7, 0x6666);
				mtEMiiRegWrite(ephy_addr, 0x1e, 0xe9, 0x0003);
				mtEMiiRegWrite(ephy_addr, 0x1e, 0xfe, 0x0006);
				mtEMiiRegWrite(ephy_addr, 0x1e, 0xe6, 0x1111);
			    //100M sharp				
   			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x23, 0x0c86);
   			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x24, 0x0c86);
   			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x25, 0x0c86);
   			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x26, 0x0c86);				
			    //mtEMiiRegWrite(ephy_addr, 0x1e, 0x00, 0x0190);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x01, 0x01cb);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x02, 0x01c2);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x03, 0x0108);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x04, 0x0211);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x05, 0x0205);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x06, 0x0387);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x07, 0x03ce);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x08, 0x03c8);
			    //mtEMiiRegWrite(ephy_addr, 0x1e, 0x09, 0x0300);
			    //mtEMiiRegWrite(ephy_addr, 0x1e, 0x0a, 0x0008);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x0b, 0x0005);
				
			}
			if(mdi_resister == 0)
            {            
                printk(" 100M D0R\r\n "); 
			    //100M sharp
   			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x23, 0x0886);
   			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x24, 0x0886);
   			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x25, 0x0886);
   			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x26, 0x0886);							    
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x00, 0x0195);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x01, 0x01cb);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x02, 0x01c2);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x03, 0x0108);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x04, 0x0211);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x05, 0x0205);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x06, 0x0387);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x07, 0x03ce);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x08, 0x03c3); // WeiTing 240605 modified for TX Overshoot NEG
			    //mtEMiiRegWrite(ephy_addr, 0x1e, 0x09, 0x0300);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x0a, 0x0010);
			    mtEMiiRegWrite(ephy_addr, 0x1e, 0x0b, 0x0005);
				
				//mtEMiiRegWrite(ephy_addr, 0x1e, 0xe7, 0x2222);
				//mtEMiiRegWrite(ephy_addr, 0x1e, 0xe9, 0x0001);
				//mtEMiiRegWrite(ephy_addr, 0x1e, 0xfe, 0x0000);
			}
			
		}
		if(transformer_temp != 0) 			
		{	
			mtEMiiRegWrite(ephy_addr, 0x1e, 0x23, 0x0881);
			mtEMiiRegWrite(ephy_addr, 0x1e, 0x24, 0x0881);
			mtEMiiRegWrite(ephy_addr, 0x1e, 0x25, 0x0881);
			mtEMiiRegWrite(ephy_addr, 0x1e, 0x26, 0x0881);				
		
	       if(mdi_resister == 5)
            {
              printk(" 100M T5R\r\n "); 
			   //100M sharp added by Wei
			   mtEMiiRegWrite(ephy_addr, 0x1e, 0x05, 0x0205);				 					
			   //RX setting for 5R_TXMR before AN setting
			   //mtEMiiRegWrite(ephy_addr, 0x1e, 0xe7, 0x5555);
			   //mtEMiiRegWrite(ephy_addr, 0x1e, 0xe9, 0x0005);
			   //mtEMiiRegWrite(ephy_addr, 0x1e, 0xfe, 0x0004);
            }
			if(mdi_resister == 0)
			{
              printk(" 100M T0R\r\n "); 
			   //100M sharp
			   mtEMiiRegWrite(ephy_addr, 0x1e, 0x05, 0x0200);				  					
			   //mtEMiiRegWrite(ephy_addr, 0x1e, 0xe7, 0x2222);
			   //mtEMiiRegWrite(ephy_addr, 0x1e, 0xe9, 0x0001);
			   //mtEMiiRegWrite(ephy_addr, 0x1e, 0xfe, 0x0000);
			}
				
		}


					//Power Saving

					
			
					//Long Loop Reach setting 

					//led
					mtEMiiRegWrite(ephy_addr, 0x1f, 0x21, 0x800a);
					mtEMiiRegWrite(ephy_addr, 0x1f, 0x24, 0xc007);
					mtEMiiRegWrite(ephy_addr, 0x1f, 0x25, 0x003f);
					mtEMiiRegWrite(ephy_addr, 0x1f, 0x26, 0xc007);
					mtEMiiRegWrite(ephy_addr, 0x1f, 0x27, 0x0037);

					
					mtEMiiRegWrite(ephy_addr, 0x1e, 0x3e, 0xc000);
					mtEMiiRegWrite(ephy_addr, 0x1e, 0x3d, 0x0000);
					mtEMiiRegWrite(ephy_addr, 0x1e, 0x3c, 0xc000);
					//EEE setting
					mtEMiiRegWrite(ephy_addr, 0x7, 0x3c, 0x0000);
					//CL22 ephy_addr, 0x3, 0x14, 0x190);			
					mtRegWrite_CL22(ephy_addr, 0x3, 0x14, 0x190);

					tcMiiStationWrite(ephy_addr, 0x1f, 0x1);
				    tcMiiStationWrite(ephy_addr, 0x14, 0x3a18);
			    	tcMiiStationWrite(ephy_addr, 0x1f, 0x0);
					tcMiiStationWrite(ephy_addr, 0x9, 0x0600);
				    //tcMiiStationWrite(ephy_addr, 0x00, 0x1040);	
					//mtPhyMiiWrite_TrDbg(slt_not_excute_loop, "PMA", 0x12, 0x5e4d2a, 0);
					
					//10M setting
					mtEMiiRegWrite(ephy_addr, 0x1e, 0x1a3, 0x00d2);
					mtEMiiRegWrite(ephy_addr, 0x1e, 0x1a4, 0x010e);
					mtEMiiRegWrite(ephy_addr, 0x1f, 0x27b, 0x1177);
					mtEMiiRegWrite(ephy_addr, 0x1f, 0x27c, 0x0808);

					

					#if defined(TCSUPPORT_AUTOBENCH)
					//LV SLT pass set
					#endif					
					


					// EEE more power saving 240527 WeiTing
						
					if (EEE_pairA_ON_pairBCD_OFF)
					{
						mtEMiiRegWrite(ephy_addr, 0x1e, 0x630, 0x006E);
					}

					// idle_mode_power_saving

					

					
    	}
}

// ************************************************************************
// 		tcPhy initial: reset, load default register setting, restat AN
//		[in] *mac_p
// ************************************************************************

int tcPhyInit_patch(void)
{
	u16	ephy_addr, port_num,get_transformer;	
//	if (isFPGA) 
//	{	//xyyou add for 7505 FPGA phy addr1 and addr2 reset 			
//    	tcMiiStationWrite(1, MII_BMCR, BMCR_RESET); 	
//    	pause(10);			
//    	tcMiiStationWrite(1, MII_BMCR, BMCR_ANRESTART | BMCR_ANENABLE); 
//    	pause(10);			
//    	tcMiiStationWrite(2, MII_BMCR, BMCR_RESET); 	
//    	pause(10);			
//    	tcMiiStationWrite(2, MII_BMCR, BMCR_ANRESTART | BMCR_ANENABLE); 
//	}
//	else
//	{
		GECal_Rext_flag=0;
		ephy_addr_base = 9;
		PreCalSet();
		for(ephy_addr = EPHY_ADDR_P0; ephy_addr <= 12; ephy_addr++)
		{
    		tcPhyVerLookUp(ephy_addr);
			tcPhyInitFlag = 1; //for all
    		if(tcPhyInitFlag == 1)  // is ECNT's ephy
    		{
    			port_num = ephy_addr - ephy_addr_base;
    			TCPHYDISP1(", port_num = %d, ephy_addr = %d, ephy_addr_base = 0x%x \r\n", port_num, ephy_addr, ephy_addr_base);

    			//if(port_num >= 0)
    			{
    				ePhyInit(ephy_addr);
					get_transformer=auto_select_transformer(ephy_addr);					
					doGePhyALLAnalogCal_R45(ephy_addr); //FW cal
					phyTxAmpcomp(1,ephy_addr,get_transformer);
		            tcMiiStationWrite(ephy_addr, 0x00, 0x0800);	
    			}	
     		}	
    	}
		normalinit();

//	}
	
//	tcephydbgcmd();
//    ephy_timer_init();
	inital_finish_flag = 1;

	return 0;
}	

int monitor_phy_init_flag(void)
{
	return inital_finish_flag;
}

// Software Patch for TrendChip's ethernet PHY
void tcPhyChkVal(unsigned int phyAddr)
{ 	
	    u32 	val;
		//u32 	phyAddr = phychkval_portnum;
        u32 	mr02,mr03;
        u8 	mr_anen, mr_dplx, mr_speed;
        u8 	mr_an_capable;
        u8 	mr_lp_an_capable;
        u8 	mr_lp_anen;
		u8 	mr01_link;	
        u32 	reg3, r15_temp;
        u8 	lr_linkup, lr_speed, lr_dplx, lr_mdix;
		// show phy_id
	
		tcMiiStationWrite(phyAddr, 31, 0);   				// g0r31 = 0x0
		val = tcMiiStationRead(phyAddr, 2);
		mr02 = val&(0xffff);
		val = tcMiiStationRead(phyAddr, 3); 
		mr03 = val&(0xffff);
		reg3 = tcMiiStationRead(phyAddr, 3); 
		printk(" tcPhy ID: %x %x\r\n", mr02, mr03); 
	
		val = tcMiiStationRead(phyAddr, 0);
		mr_anen = (val>>12)&0x01;
		mr_dplx = (val>>8)&0x01;
		mr_speed = ((val>>13)|(val>>5))&0x03;				// 00:10,01:100,10:1000
		
		// reg4h
		val = tcMiiStationRead(phyAddr, 0x04);
		mr_an_capable = (val>>5)&0x0F; 						// 100F,100H,10F,10H

		// reg9h
		val = tcMiiStationRead(phyAddr, 0x09);
		mr_an_capable = mr_an_capable | ((val>>4)&0x30);	// 1000F,1000H

		// reg5h
		val = tcMiiStationRead(phyAddr, 0x05);
		mr_lp_an_capable = (val>>5)&0x0F; 					// 100F,100H,10F,10H
		
		// regAh
		val = tcMiiStationRead(phyAddr, 0x0A);
		mr_lp_an_capable = mr_lp_an_capable | ((val>>6)&0x30); 		// 1000F,1000H

		// reg6h
		val = tcMiiStationRead(phyAddr, 0x06);
		mr_lp_anen = (val)&0x01;
   
		// reg1h 											// allen_20130926 from biker
		val = tcMiiStationRead(phyAddr, 0x01);
		mr01_link = (val>>2)&0x01;
		//printk(" [1] reg1 = 0x%x, mr01_link = %d \n", val, mr01_link);
		
		val = tcMiiStationRead(phyAddr, 0x01);
		mr01_link = (val>>2)&0x01;
		//printk(" [2] reg1 = 0x%x, mr01_link = %d \n", val, mr01_link);
	
		val = tcMiiStationRead(phyAddr, 0x01);
		mr01_link = (val>>2)&0x01;
		printk(" [3] reg1 = 0x%x, mr01_link = %d, tcPhyVer = 0x%x \r\n", val, mr01_link, tcPhyVer);
    	//printk(" [3] phychkval_portnum = %d, phyAddr = %d, port_num = %d \r\n", phychkval_portnum, phyAddr, port_num);

		r15_temp = tcMiiStationRead(phyAddr, 0xf);
		if((reg3 == EPHY_ID_7583))
		{									
			val = mtPhyReadGReg(phyAddr,0x1E,0xA2);
    
    		//printk(" tcPhyChkVal= 0x%04lx",val);
  
			lr_linkup = (val>>9)&0x07;		//dev1e_A2[9]:LinkStatus10_OK,dev1e_A2[10]:LinkStatus100_OK,dev1e_A2[11]:LinkStatus1000_OK
			lr_speed = (val>>1)&0x07; 		//dev1e_A2[1]:final_speed_10,,dev1e_A2[2]:dev1e_A2[2]:final_speed_100,dev1e_A2[3]:final_speed_1000
			lr_dplx = (val>>5)&0x01;
			lr_mdix = (val>>6)&0x01;
		}    
		else
		{ 
			// l0reg28
			val = tcPhyReadLReg(phyAddr,0, 28);
			//printk(" tcPhyChkVal= 0x%04lx",val);
			
			lr_linkup = (val)&0x01;
			lr_speed = (val>>1)&0x01;
			lr_dplx = (val>>2)&0x01;
			lr_mdix = (val>>5)&0x01;
		}
			
		printk(" TcPhy mode:");	
		if(mr_anen)
		{ 	// Auto-neg
			printk(" AN-(%s,%s,%s,%s,%s,%s)\r\n",
	    	((mr_an_capable>>5)&0x01)?"1000F":"",
	    	((mr_an_capable>>4)&0x01)?"1000H":"",
	    	((mr_an_capable>>3)&0x01)?"100F":"",
	    	((mr_an_capable>>2)&0x01)?"100H":"",     
	    	((mr_an_capable>>1)&0x01)?"10F":"",      
	    	((mr_an_capable>>0)&0x01)?"10H":"");  
			
//	    	printk(" AN-(");
//	    	printk(" %s,",((mr_an_capable>>5)&0x01)?"1000F":"");
//	    	printk(" %s,",((mr_an_capable>>4)&0x01)?"1000H":"");
//	    	printk(" %s,",((mr_an_capable>>3)&0x01)?"100F":"");
//	    	printk(" %s,",((mr_an_capable>>2)&0x01)?"100H":"");     
//	    	printk(" %s,",((mr_an_capable>>1)&0x01)?"10F":"");      
//	    	printk(" %s)\r\n",((mr_an_capable>>0)&0x01)?"10H":"");  
		}
		else if((reg3 == EPHY_ID_7583))
		{ 	// Force-speed
			if(mr_speed==0x00)
	 		{
		 		printk(" Force-%d%s\r\n",10,(mr_dplx?"F":"H"));
			}
			else if(mr_speed==0x01)
			{
				printk(" Force-%d%s\r\n",100,(mr_dplx?"F":"H"));
	 		}
			else if(mr_speed==0x02)
			{
				printk(" Force-%d%s\r\n",1000,(mr_dplx?"F":"H"));
	 		}
    	}
	    else 
	 	{
	 		printk(" Force-%d%s\r\n",(mr_speed?100:10),(mr_dplx?"F":"H")); 	
        }
		 
		if(mr01_link == 0)  // link down
		{ 	// allen_20130926
			printk(" *** Link is down!\r\n");

            // dev1E_RA2 message
            if(mr1E_A2.lch_SignalDetect || mr1E_A2.lch_LinkPulse
                || mr1E_A2.final_speed_1000 || mr1E_A2.final_speed_100|| mr1E_A2.final_speed_10)
            {
				TCPHYDISP4(" tcphy[%d]: ",phyAddr);

				if(mr1E_A2.lch_SignalDetect) 
					TCPHYDISP4("SigDet ");
				if(mr1E_A2.lch_LinkPulse)
					TCPHYDISP4("RxLkp ");
				if(mr1E_A2.final_speed_1000)
				{					
					if(mr1E_A2.MSConfig1000)
					{
						TCPHYDISP4("Up1000 Master ");
					}
					else
					{
						TCPHYDISP4("Up1000 Slave ");
					}
				}

				if(mr1E_A2.final_speed_100)
					TCPHYDISP4("Up100 ");
				if(mr1E_A2.final_speed_10)
					TCPHYDISP4("Up10 ");
				if(mr1E_A2.da_mdix)
				{
					TCPHYDISP4("UpMdix ");
				}
				else
				{
					TCPHYDISP4("UpMdi ");
				}
				TCPHYDISP4("\r\n");
            }

        }
		else 
		{ 	// link-up
			if((reg3==EPHY_ID_7583)) 
			{
				if(lr_speed == 0x04)
				{
					printk(" TCphy is link-up at 1000 %s ",(mr1E_A2.MSConfig1000?"Master":"Slave"));
				}
				else if(lr_speed == 0x02)
				{
					printk(" TCphy is link-up at 100 %s ",(lr_dplx?"F":"H"));
				}
				else if(lr_speed == 0x01)
				{	
					printk(" TCphy is link-up at 10 %s ",(lr_dplx?"F":"H"));
				}
	      	}
			else
			{
				printk(" TCphy is link-up at %d%s ",(lr_speed?100:10),(lr_dplx?"F":"H"));	
			}
           
	   		if(mr_lp_anen)
			{
				printk(" Link-partner supports AN-(%s,%s,%s,%s,%s,%s)\r\n",
				((mr_an_capable>>5)&0x01)?"1000F":"",
				((mr_an_capable>>4)&0x01)?"1000H":"",
				((mr_an_capable>>3)&0x01)?"100F":"",
				((mr_an_capable>>2)&0x01)?"100H":"",     
				((mr_an_capable>>1)&0x01)?"10F":"",      
				((mr_an_capable>>0)&0x01)?"10H":"");
								
//                printk(", Link-partner supports AN-(");
//                printk(" %s,",((mr_lp_an_capable>>5)&0x01)?"1000F":"");
//                printk(" %s,",((mr_lp_an_capable>>4)&0x01)?"1000H":"");
//                printk(" %s,",((mr_lp_an_capable>>3)&0x01)?"100F":"");
//                printk(" %s,",((mr_lp_an_capable>>2)&0x01)?"100H":"");  
//                printk(" %s,",((mr_lp_an_capable>>1)&0x01)?"10F":"");       
//                printk(" %s),  ",((mr_lp_an_capable>>0)&0x01)?"10H":"");   
            }
	    	else 
			{
                printk(" Link-partner operates in Force mode.\r\n");
            }

            printk(" %s,",(lr_mdix?"mdix":"mdi")); 
            
			if(lr_speed >= 0x02)
			{	// 100Mbps/1000Mbps

				if((reg3 == EPHY_ID_7583))
				{
					#if debug_flag
					val=en75xxGePhyReadProbe(phyAddr, Probe_VgaState, 0);
					printk("7583 VgaStateA, B, C, D = 0x%x, 0x%x, 0x%x, 0x%x\r\n",((val>>4) & 0x1F),((val>>9) & 0x1F),((val>>14) & 0x1F),((val>>19) & 0x1F));
				
					val=en75xxGePhyReadProbe(phyAddr,Probe_MSE,0);
					printk("7583 lch_mse_mdcA =0x%x\r\n",val);
					val=en75xxGePhyReadProbe(phyAddr,Probe_MSE,1);
					printk("7583 lch_mse_mdcB =0x%x\r\n",val);
					val=en75xxGePhyReadProbe(phyAddr,Probe_MSE,2);
					printk("7583 lch_mse_mdcC =0x%x\r\n",val);
					val=en75xxGePhyReadProbe(phyAddr,Probe_MSE,3);
					printk("7583 lch_mse_mdcD =0x%x\r\n",val);
			
					val=en75xxGePhyReadProbe(phyAddr,Probe_MSE_Slicer_err_thres,0);
					printk("slicer_err_thres =0x%x\r\n",val);
					val=en75xxGePhyReadProbe(phyAddr,Probe_Slicer_Err_Over_Sum,0);
					printk("ProbeSlicerErrOverSumA =0x%x\r\n",val);
					val=en75xxGePhyReadProbe(phyAddr,Probe_Slicer_Err_Over_Sum,1);
					printk("ProbeSlicerErrOverSumB =0x%x\r\n",val);
					val=en75xxGePhyReadProbe(phyAddr,Probe_Slicer_Err_Over_Sum,2);
					printk("ProbeSlicerErrOverSumC =0x%x\r\n",val);
					val=en75xxGePhyReadProbe(phyAddr,Probe_Slicer_Err_Over_Sum,3);
					printk("ProbeSlicerErrOverSumD =0x%x\r\n",val);
					#endif
				}
			}
		}   

}

extern en75xxGe_cfg_data_t en75xxGe_cfg[];
int doPhySwVer(int argc, char *argv[], void *p)
{
	char main_ver[64] = "AN7583.05.03";
	char date[64] = "20250826";
	unsigned int transformer_ver;
	printk("Ver : ");

    switch (tcPhyVer) 
	{    
		case tcPhyVer_7583: 
			printk("%s, \r\n", en75xxGe_cfg[0].name);
		 	printk("tcPhyVer_7583, tcPhyVer=0x%02X, Date=%s \r\n", tcPhyVer, date);
   		 	printk("-tcSWVer_REG=0x%02X \r\n", tcSWVer_REG);
   		 	printk("-tcSWVer_FrameWork=0x%02X \r\n", tcSWVer_FRAMEWORK);
   		 	printk("-tcSWVer_CMD=0x%02X \r\n", tcSWVer_CMD);
  		 	printk("-tcSWVer_FUNC=0x%02X \r\n", tcSWVer_FUNC);
   		 	printk("-tcSWVer_PATCH=0x%02X \r\n", tcSWVer_PATCH);
   		 	printk("-tcSWVer_COMMON=0x%02X \r\n", tcSWVer_COMMON);			
			break;	

        default: 
	    	printk("Unknown PHY, tcPhyVer=0x%02X \r\n", tcPhyVer);
    }

printk("Main Version = %s \r\n", main_ver);
printk("%dR_TXMR \r\n",mdi_resister);

transformer_ver=transformer_status&0x01;
if(transformer_ver != 0) { printk("port 9_%dR_TXMR \r\n",mdi_resister);} else { printk("port 9_%dR_discrete_TXMR \r\n",mdi_resister);}
    
transformer_ver=transformer_status&0x02;
if(transformer_ver != 0) { printk("port 10_%dR_TXMR \r\n",mdi_resister);} else { printk("port 10_%dR_discrete_TXMR \r\n",mdi_resister);}
    
transformer_ver=transformer_status&0x04;
if(transformer_ver != 0) { printk("port 11_%dR_TXMR \r\n",mdi_resister);} else { printk("port 11_%dR_discrete_TXMR \r\n",mdi_resister);}
    
transformer_ver=transformer_status&0x08;
if(transformer_ver != 0) { printk("port 12_%dR_TXMR \r\n",mdi_resister);} else { printk("port 12_%dR_discrete_TXMR \r\n",mdi_resister);}    
return 0;
} 


int sw_ability_setting(int argc, char *argv[], void *p)
{
	unsigned int sw_ability,switch_status;
	if((argc <=2)||(argc >=4)) 
	{
	   	printk("sw_ability_set <SDS|FS|SLT> <on|ff>\r\n");
	}
	else	
	{
		sw_ability=checked_atoi(argv[1]);
    	switch_status = checked_atoi(argv[2]);
		if(sw_ability==1) 
		{
			if(switch_status==1) { sw_down_shift_en=1;printk("software downshift enable\r\n");} else { sw_down_shift_en=0;printk("software downshift disable\r\n");}
		}
		if(sw_ability==2) 
		{
			if(switch_status==1) { phy_downshift_force_giga_enable=1;printk("giga downshift force slave enable\r\n");} else { phy_downshift_force_giga_enable=0;printk("giga downshift force slave disable\r\n");}
		}
		if(sw_ability==3) 
		{
			if(switch_status==1) { slt_excuting=1;printk("slt excuting\r\n");} else { slt_excuting=0;printk("slt not excuting\r\n");}
		}
		if(sw_ability==4) 
		{	
			if( switch_status ==1 ) { loopback_head_downshift_100M=1;printk("loopback_head_downshift on\r\n");} else { loopback_head_downshift_100M=0;printk("loopback_head_downshift off\r\n");}
		}
		if(sw_ability==5) 
		{	
			if( switch_status ==1 ) { link_state_polling=1;printk("link status print on\r\n");} else { link_state_polling=0;printk("link status print off\r\n");}
		}
		if(sw_ability==6) 
		{	
			if( switch_status ==1 ) { inital_finish_flag=1;printk("phy monitor on\n");} else { inital_finish_flag=0;printk("phy monitor off\r\n");}
		}
		if(sw_ability==7) 
		{	
			if( switch_status ==1 ) { command_calbration_enable=1;printk("command calibration on\n\r");} else { command_calbration_enable=0;printk("command calibration off\r\n");}
		}
		if(sw_ability == 8)
		{
			if(switch_status == 1) { netgear_downshift_100M = 1; printk("netgear downshift on\n\r"); } else { netgear_downshift_100M = 0; printk("netgear downshift off\r\n"); }
		}
		if(sw_ability == 9)
		{
			if(switch_status == 1) { LDPS_function_en = 1; printk("LDPS function on\n\r"); } else { LDPS_function_en = 0; printk("LDPS function off\r\n"); }
		}
	}
    return 0;
}



int doReadAllCalData(int argc, char *argv[], void *p)
{
	u16 do_cal_loop,show_temp;
	unsigned int  reg_1e_12_read,reg_1e_16_read,reg_1e_17_read,reg_1e_18_read,reg_1e_19_read,reg_1e_20_read,reg_1e_21_read,reg_1e_22_read,reg_1e_174_read,reg_1e_175_read;
	

for(do_cal_loop = 9; do_cal_loop<=12; do_cal_loop ++)
	{
    if(do_cal_loop == 9 ) {show_temp=transformer_status&0x01;}
    if(do_cal_loop == 10 ) {show_temp=transformer_status&0x02;}
    if(do_cal_loop == 11 ) {show_temp=transformer_status&0x04;}
    if(do_cal_loop == 12 ) {show_temp=transformer_status&0x08;}
    if(show_temp != 0 ) { phyTxAmpcomp(0,do_cal_loop,1);} else { phyTxAmpcomp(0,do_cal_loop,0);}
	reg_1e_12_read = mtEMiiRegRead(do_cal_loop, 0x1e, 0x12);
	reg_1e_16_read = mtEMiiRegRead(do_cal_loop, 0x1e, 0x16);
	reg_1e_17_read = mtEMiiRegRead(do_cal_loop, 0x1e, 0x17);
	reg_1e_18_read = mtEMiiRegRead(do_cal_loop, 0x1e, 0x18);
	reg_1e_19_read = mtEMiiRegRead(do_cal_loop, 0x1e, 0x19);
	reg_1e_20_read = mtEMiiRegRead(do_cal_loop, 0x1e, 0x20);
	reg_1e_21_read = mtEMiiRegRead(do_cal_loop, 0x1e, 0x21);
	reg_1e_22_read = mtEMiiRegRead(do_cal_loop, 0x1e, 0x22);
	reg_1e_174_read = mtEMiiRegRead(do_cal_loop, 0x1e, 0x174);
	reg_1e_175_read = mtEMiiRegRead(do_cal_loop, 0x1e, 0x175);
	
	printk("Port %d \r\n",do_cal_loop);
	show_temp=reg_1e_16_read&0x003f;
	printk("tx_amp_tst_cha = 0x%x " ,show_temp);
	show_temp=reg_1e_18_read&0x003f;		
	printk("tx_amp_tst_chb = 0x%x " ,show_temp);
	show_temp=reg_1e_20_read&0x003f;	
	printk("tx_amp_tst_chc = 0x%x " ,show_temp);
	show_temp=reg_1e_22_read&0x003f;
	printk("tx_amp_tst_chd = 0x%x \n\r" ,show_temp);
	
	show_temp=(reg_1e_12_read&0xfc00)/1024;
	printk("tx_amp_gbe_cha = 0x%x " ,show_temp);
	show_temp=(reg_1e_17_read&0x3f00)/256;
	printk("tx_amp_gbe_chb = 0x%x " ,show_temp);
	show_temp=(reg_1e_19_read&0x3f00)/256;
	printk("tx_amp_gbe_chc = 0x%x " ,show_temp);
	show_temp=(reg_1e_21_read&0x3f00)/256;
	printk("tx_amp_gbe_chd = 0x%x \n\r" ,show_temp);
	
	show_temp=(reg_1e_16_read&0xfc00)/1024;	
	printk("tx_amp_hbt_cha = 0x%x " ,show_temp);
	show_temp=(reg_1e_18_read&0x3f00)/256;
	printk("tx_amp_hbt_chb = 0x%x " ,show_temp);
	
	show_temp=reg_1e_12_read&0x003f;	
	printk("tx_amp_tbt_cha = 0x%x " ,show_temp);
	show_temp=reg_1e_17_read&0x003f;
	printk("tx_amp_tbt_chb = 0x%x \n\r" ,show_temp);
	
	show_temp=(reg_1e_174_read&0x7f00)/256;	
	printk("R50_cha = 0x%x " ,show_temp);
	show_temp=reg_1e_174_read&0x007f;
	printk("R50_chb = 0x%x " ,show_temp);
	show_temp=(reg_1e_175_read&0x7f00)/256;	
	printk("R50_chc = 0x%x " ,show_temp);
	show_temp=reg_1e_175_read&0x007f;
	printk("R50_chd = 0x%x \n\r" ,show_temp);

	printk("  1f_200 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x200));
	printk("  1f_201 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x201));
    printk("  1f_202 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x202));
	printk("  1f_203 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x203));
	printk("  1f_204 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x204));
	printk("  1f_205 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x205));
    printk("  1f_206 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x206));
	printk("  1f_207 = 0x%x\n\r" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x207));			
	printk("  1f_208 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x208));
	printk("  1f_209 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x209));
    printk("  1f_20a = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x20a));
	printk("  1f_20b = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x20b));
	printk("  1f_20c = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x20c));
	printk("  1f_20d = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x20d));
    printk("  1f_20e = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x20e));
	printk("  1f_20f = 0x%x\n\r" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x20f));	

	printk("  1f_210 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x210));
	printk("  1f_211 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x211));
    printk("  1f_212 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x212));
	printk("  1f_213 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x213));
	printk("  1f_214 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x214));
	printk("  1f_215 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x215));
    printk("  1f_216 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x216));
	printk("  1f_217 = 0x%x\n\r" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x217));			
	printk("  1f_218 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x218));
	printk("  1f_219 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x219));
    printk("  1f_21a = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x21a));
	printk("  1f_21b = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x21b));
	printk("  1f_21c = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x21c));
	printk("  1f_21d = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x21d));
    printk("  1f_21e = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x21e));
	printk("  1f_21f = 0x%x\n\r" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x21f));

	printk("  1f_220 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x220));
	printk("  1f_221 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x221));
    printk("  1f_222 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x222));
	printk("  1f_223 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x223));
	printk("  1f_224 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x224));
	printk("  1f_225 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x225));
    printk("  1f_226 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x226));
	printk("  1f_227 = 0x%x\n\r" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x227));			
	printk("  1f_228 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x228));
	printk("  1f_229 = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x229));
    printk("  1f_22a = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x22a));
	printk("  1f_22b = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x22b));
	printk("  1f_22c = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x22c));
	printk("  1f_22d = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x22d));
    printk("  1f_22e = 0x%x" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x22e));
	printk("  1f_22f = 0x%x\n\r" , mtEMiiRegRead(do_cal_loop, 0x1f, 0x22f));

	// printk downshift status
	tcMiiStationWrite(do_cal_loop, 0x1f, 0x1);
	printk("  Ext_14 = 0x%x\n\r" , tcMiiStationRead(do_cal_loop, 0x14));
	tcMiiStationWrite(do_cal_loop, 0x1f, 0x0);
	
	}
	    if(sw_down_shift_en == 1 ) { printk("software downshift enable\r\n");} else { printk("software downshift disable\r\n");}
		if(slt_excuting == 1 ) { printk("slt_excuting\r\n");} else { printk("not slt_excuting\r\n");}
		if(phy_downshift_force_giga_enable == 1 ) { printk("giga downshift force slave enable\r\n");} else { printk("giga downshift force slave disable\r\n");}
	    if( loopback_head_downshift_100M ==1 ) { printk("loopback_head_downshift on\r\n");} else { printk("loopback_head_downshift off\r\n");}
	    if( link_state_polling ==1 ) { printk("link status print on\r\n");} else { printk("link status print off\r\n");}
	    if( inital_finish_flag ==1 ) { printk("phy monitor on\r\n");} else { printk("phy monitor off\r\n");}
		if( command_calbration_enable ==1 ) { printk("command calibration on\r\n");} else { printk("command calibration off\r\n");}	
		if (netgear_downshift_100M == 1) { printk("netgear downshift on\r\n"); } else { printk("netgear downshift off\r\n"); }
		if (LDPS_function_en == 1) { printk("LDPS function on\r\n"); } else { printk("LDPS function off\r\n"); }
        printk("\r\n");

	return 0; //Coverity #139525
}




void sw_downshift(u8 phyaddr)
{
		u32 	dev_1e_reg_9b, dev_1e_reg_9c,mean_square_error_times_limit=3,mean_square_error_value_limit=5,link_counte_limit=20,mean_squre_error_A,mean_squre_error_B,mean_squre_error_C,mean_squre_error_D,mean_squre_error_temp;

                dev_1e_reg_9b = mtEMiiRegRead(phyaddr,0x1E,0x9B);
				dev_1e_reg_9c = mtEMiiRegRead(phyaddr,0x1E,0x9C);
				mean_squre_error_A=(dev_1e_reg_9b/256)&0xff;
				mean_squre_error_B=dev_1e_reg_9b&0xff;
				mean_squre_error_C=(dev_1e_reg_9c/256)&0xff;
				mean_squre_error_D=dev_1e_reg_9c&0xff;
				/*
                printk("port%d A_mean_square_error=0x%x --" ,phyaddr,mean_squre_error_A);
                printk("port%d B_mean_square_error=0x%x --" ,phyaddr,mean_squre_error_B);
                printk("port%d C_mean_square_error=0x%x --" ,phyaddr,mean_squre_error_C);
                printk("port%d D_mean_square_error=0x%x \n\r" ,phyaddr,mean_squre_error_D);

                if(phyaddr ==9) {printk("port%d curren_counter=0x%x    --  " ,phyaddr,ephy_monitor_p9_curren_counter);}
				if(phyaddr ==10) {printk("port%d curren_counter=0x%x    --  " ,phyaddr,ephy_monitor_p10_curren_counter);}
                if(phyaddr ==11) {printk("port%d curren_counter=0x%x    --  " ,phyaddr,ephy_monitor_p11_curren_counter);}
                if(phyaddr ==12) {printk("port%d curren_counter=0x%x    --  " ,phyaddr,ephy_monitor_p12_curren_counter);}
				printk("ephy_monitor_all_counter=0x%x \n\r" ,ephy_monitor_all_counter);
				*/
				mean_squre_error_temp=ephy_monitor_all_counter-ephy_monitor_p9_curren_counter;
				if(mean_squre_error_temp ==1) { ephy_monitor_continue_p9_counter++;} else { ephy_monitor_continue_p9_counter=0;}
				mean_squre_error_temp=ephy_monitor_all_counter-ephy_monitor_p10_curren_counter;
				if(mean_squre_error_temp ==1) { ephy_monitor_continue_p10_counter++;} else { ephy_monitor_continue_p10_counter=0;}
				mean_squre_error_temp=ephy_monitor_all_counter-ephy_monitor_p11_curren_counter;
				if(mean_squre_error_temp ==1) { ephy_monitor_continue_p11_counter++;} else { ephy_monitor_continue_p11_counter=0;}
				mean_squre_error_temp=ephy_monitor_all_counter-ephy_monitor_p12_curren_counter;
				if(mean_squre_error_temp ==1) { ephy_monitor_continue_p12_counter++;} else { ephy_monitor_continue_p12_counter=0;}
		
				if(ephy_monitor_continue_p9_counter >= link_counte_limit) { ephy_monitor_continue_p9_counter=0;ephy_monitor_p9_stop_polling=1;mean_square_error_counter_p9=0;}
				if(ephy_monitor_continue_p10_counter >= link_counte_limit) { ephy_monitor_continue_p10_counter=0;ephy_monitor_p10_stop_polling=1;mean_square_error_counter_p10=0;}
				if(ephy_monitor_continue_p11_counter >= link_counte_limit) { ephy_monitor_continue_p11_counter=0;ephy_monitor_p11_stop_polling=1;mean_square_error_counter_p11=0;}
				if(ephy_monitor_continue_p12_counter >= link_counte_limit) { ephy_monitor_continue_p12_counter=0;ephy_monitor_p12_stop_polling=1;mean_square_error_counter_p12=0;}
				if(phyaddr ==9) {ephy_monitor_p9_curren_counter=ephy_monitor_all_counter;}
				if(phyaddr ==10) {ephy_monitor_p10_curren_counter=ephy_monitor_all_counter;}
				if(phyaddr ==11) {ephy_monitor_p11_curren_counter=ephy_monitor_all_counter;}
				if(phyaddr ==12) {ephy_monitor_p12_curren_counter=ephy_monitor_all_counter;}
			if((mean_squre_error_A >= mean_square_error_value_limit)||(mean_squre_error_B >= mean_square_error_value_limit)||(mean_squre_error_C >= mean_square_error_value_limit)||(mean_squre_error_D >= mean_square_error_value_limit)) 
			{
				if(phyaddr ==9) {mean_square_error_counter_p9++;}
				if(phyaddr ==10) {mean_square_error_counter_p10++;}
				if(phyaddr ==11) {mean_square_error_counter_p11++;}
				if(phyaddr ==12) {mean_square_error_counter_p12++;}
				if( mean_square_error_counter_p9 >= mean_square_error_times_limit) { tcMiiStationWrite(phyaddr, 0x9,0000);sw_down_shift_p9=1;printk("port9 sw down shift\n\r" );mean_square_error_counter_p9=0; workaround_flag_p9 = workaround_flag_p9 | BIT10;}
				if( mean_square_error_counter_p10 >= mean_square_error_times_limit) { tcMiiStationWrite(phyaddr, 0x9,0000);sw_down_shift_p10=1;printk("port10 sw down shift\n\r" );mean_square_error_counter_p10=0; workaround_flag_p10 = workaround_flag_p10 | BIT10;}
				if( mean_square_error_counter_p11 >= mean_square_error_times_limit) { tcMiiStationWrite(phyaddr, 0x9,0000);sw_down_shift_p11=1;printk("port11 sw down shift\n\r" );mean_square_error_counter_p11=0; workaround_flag_p11 = workaround_flag_p11 | BIT10;}
				if( mean_square_error_counter_p12 >= mean_square_error_times_limit) { tcMiiStationWrite(phyaddr, 0x9,0000);sw_down_shift_p12=1;printk("port12 sw down shift\n\r" );mean_square_error_counter_p12=0; workaround_flag_p12 = workaround_flag_p12 | BIT10;}
				tcMiiStationWrite(phyaddr, 0x0,0x1240);
				
			} 
			
}


void loopback_head_downshift_100M_main(unsigned int phyaddr,unsigned int reg1)
{
	unsigned int reg_temp1,reg_temp,downshift_2pair_limit=24;
			reg_temp1=reg1&0x0004;
		if((phyaddr ==9)&&(loopback_haed_start_p9==1)) { 
		if(reg_temp1==0) {
		     if(force_slave_enable_p9==0) {
		     reg_temp=ephy_monitor_all_counter-downshift_2pair_capture_counter_p9_counter;
			 if( reg_temp >downshift_2pair_limit ) { tcMiiStationWrite(phyaddr, 0x09, 0x0000);tcMiiStationWrite(phyaddr, 0x00, 0x1240);force_slave_enable_p9 =1;}
			 }} else {
			//port9 link
			 if( force_slave_enable_p9 ==1 ) { tcMiiStationWrite(phyaddr, 0x9 ,0x1e00);force_slave_enable_p9=0;}
		 	loopback_haed_start_p9=0;force_giga_master_counter_p9=0;force_giga_master_mode_p9=0;
			if(link_state_polling == 1){ printk("phy=%d loopback header downshift \n\r",phyaddr);}
		}}
		if((phyaddr ==10)&&(loopback_haed_start_p10==1)) { 
		if(reg_temp1==0) {
			if(force_slave_enable_p10==0) {
		     reg_temp=ephy_monitor_all_counter-downshift_2pair_capture_counter_p10_counter;
			 if( reg_temp >downshift_2pair_limit ) { tcMiiStationWrite(phyaddr, 0x09, 0x0000);tcMiiStationWrite(phyaddr, 0x00, 0x1240);force_slave_enable_p10 =1;}
			}} else {
			//port10 link
			 if(force_slave_enable_p10 ==1) { tcMiiStationWrite(phyaddr, 0x9 ,0x1e00);force_slave_enable_p10=0;}
			 loopback_haed_start_p10=0;force_giga_master_counter_p10=0;force_giga_master_mode_p10=0;
			 if(link_state_polling == 1){ printk("phy=%d loopback header downshift \n\r",phyaddr);}
		}}
		if((phyaddr ==11)&&(loopback_haed_start_p11==1)) { 
		if(reg_temp1==0) {
			if(force_slave_enable_p11==0) {
		     reg_temp=ephy_monitor_all_counter-downshift_2pair_capture_counter_p11_counter;
			 if( reg_temp >downshift_2pair_limit ) { tcMiiStationWrite(phyaddr, 0x09, 0x0000);tcMiiStationWrite(phyaddr, 0x00, 0x1240);force_slave_enable_p11 =1;}
			}} else {
			//port11 link
			 if( force_slave_enable_p11 ==1 ) { tcMiiStationWrite(phyaddr, 0x9 ,0x1e00);force_slave_enable_p11=0;}
			 loopback_haed_start_p11=0;force_giga_master_counter_p11=0;force_giga_master_mode_p11=0;
			 if(link_state_polling == 1){ printk("phy=%d loopback header downshift \n\r",phyaddr);}
		}}
		if((phyaddr ==12)&&(loopback_haed_start_p12==1)) { 
		if(reg_temp1==0) {
		if(force_slave_enable_p12==0) {
		     reg_temp=ephy_monitor_all_counter-downshift_2pair_capture_counter_p12_counter;
			 if( reg_temp >downshift_2pair_limit ) { tcMiiStationWrite(phyaddr, 0x09, 0x0000);tcMiiStationWrite(phyaddr, 0x00, 0x1240);force_slave_enable_p12 =1;}
		}} else {
			//port12 link
			 if( force_slave_enable_p12 ==1 ) { tcMiiStationWrite(phyaddr, 0x9 ,0x1e00);force_slave_enable_p12=0;}
			 loopback_haed_start_p12=0;force_giga_master_counter_p12=0;force_giga_master_mode_p12=0;
			 if(link_state_polling == 1){ printk("phy=%d loopback header downshift \n\r",phyaddr);}
		}}
	
				//loopback header downshift

}

void downshift_force_giga(unsigned int phyaddr,unsigned int dev_1e_reg_a2)
{
	unsigned int dev_1e_reg_a2_temp, page_0_reg_1 = 0, page_0_reg_1_temp = 0, force_master_limit = 0, force_slave_limit = 1;
	dev_1e_reg_a2_temp = dev_1e_reg_a2 &0x0004; //Final speed is 100M
	page_0_reg_1 = tcMiiStationRead(phyaddr, 0x01);
	page_0_reg_1_temp = (page_0_reg_1 & 0x0004); //Link status

	if(phyaddr == 9)
	{
		if (netgear_downshift_100M_flag_p9 == 0)
		{
			if ((page_0_reg_1_temp != 0) && (dev_1e_reg_a2_temp != 0) && (downshift_to_force_giga_p9 == force_master_limit))
			{
				tcMiiStationWrite(phyaddr, 0x9 ,0x1e00); //Force master
				tcMiiStationWrite(phyaddr, 0x0 ,0x1240); //Re-AN
				workaround_flag_p9 = workaround_flag_p9 | BIT0;
				if (link_state_polling == 1)
				{
					printk("phy=%d downshift to force giga master \n\r", phyaddr);
				}
				downshift_to_force_giga_p9 = downshift_to_force_giga_p9 + 1;
			}
			else if ((page_0_reg_1_temp != 0) && (dev_1e_reg_a2_temp != 0) && (downshift_to_force_giga_p9 == force_slave_limit))
			{
				tcMiiStationWrite(phyaddr, 0x9 ,0x1600); //Force slave
				tcMiiStationWrite(phyaddr, 0x0 ,0x1240); //Re-AN
				workaround_flag_p9 = workaround_flag_p9 | BIT1;
				if (link_state_polling == 1)
				{
					printk("phy=%d downshift to force giga slave \n\r", phyaddr);
				}
				downshift_to_force_giga_p9 = downshift_to_force_giga_p9 + 1;					
			}
			else if ((page_0_reg_1_temp != 0) && (dev_1e_reg_a2_temp != 0) && (downshift_to_force_giga_p9 > force_slave_limit))
			{
				if (link_state_polling == 1)
				{ 
					printk("phy=%d downshift to force giga over times, keep 100M \n\r",phyaddr);
				}					
			}
		}
		else
		{
			if ((link_state_polling == 1) && ((ephy_monitor_all_counter % 80) == 1))
			{ 
				printk("phy=%d already netgear downshift to 100M \n\r",phyaddr);
			}
		}
	}

	if(phyaddr == 10)
	{
		if (netgear_downshift_100M_flag_p10 == 0)
		{
			if ((page_0_reg_1_temp != 0) && (dev_1e_reg_a2_temp != 0) && (downshift_to_force_giga_p10 == force_master_limit))
			{
				tcMiiStationWrite(phyaddr, 0x9 ,0x1e00); //Force master
				tcMiiStationWrite(phyaddr, 0x0 ,0x1240); //Re-AN
				workaround_flag_p10 = workaround_flag_p10 | BIT0;
				if (link_state_polling == 1)
				{
					printk("phy=%d downshift to force giga master \n\r", phyaddr);
				}
				downshift_to_force_giga_p10 = downshift_to_force_giga_p10 + 1;
			}
			else if ((page_0_reg_1_temp != 0) && (dev_1e_reg_a2_temp != 0) && (downshift_to_force_giga_p10 == force_slave_limit))
			{
				tcMiiStationWrite(phyaddr, 0x9 ,0x1600); //Force slave
				tcMiiStationWrite(phyaddr, 0x0 ,0x1240); //Re-AN
				workaround_flag_p10 = workaround_flag_p10 | BIT1;
				if (link_state_polling == 1)
				{
					printk("phy=%d downshift to force giga slave \n\r", phyaddr);
				}
				downshift_to_force_giga_p10 = downshift_to_force_giga_p10 + 1;				
			}
			else if ((page_0_reg_1_temp != 0) && (dev_1e_reg_a2_temp != 0) && (downshift_to_force_giga_p10 > force_slave_limit))
			{
				if(link_state_polling == 1)
				{ 
					printk("phy=%d downshift to force giga over times, keep 100M \n\r",phyaddr);
				}					
			}
		}
		else
		{
			if ((link_state_polling == 1) && ((ephy_monitor_all_counter % 80) == 1))
			{ 
				printk("phy=%d already netgear downshift to 100M \n\r",phyaddr);
			}
		}
	}

	if(phyaddr == 11)
	{
		if (netgear_downshift_100M_flag_p11 == 0)
		{
			if ((page_0_reg_1_temp != 0) && (dev_1e_reg_a2_temp != 0) && (downshift_to_force_giga_p11 == force_master_limit))
			{
				tcMiiStationWrite(phyaddr, 0x9 ,0x1e00); //Force master
				tcMiiStationWrite(phyaddr, 0x0 ,0x1240); //Re-AN
				workaround_flag_p11 = workaround_flag_p11 | BIT0;
				if (link_state_polling == 1)
				{
					printk("phy=%d downshift to force giga master \n\r", phyaddr);
				}
				downshift_to_force_giga_p11 = downshift_to_force_giga_p11 + 1;
			}
			else if ((page_0_reg_1_temp != 0) && (dev_1e_reg_a2_temp != 0) && (downshift_to_force_giga_p11 == force_slave_limit))
			{
				tcMiiStationWrite(phyaddr, 0x9 ,0x1600); //Force slave
				tcMiiStationWrite(phyaddr, 0x0 ,0x1240); //Re-AN
				workaround_flag_p11 = workaround_flag_p11 | BIT1;
				if (link_state_polling == 1)
				{
					printk("phy=%d downshift to force giga slave \n\r", phyaddr);
				}
				downshift_to_force_giga_p11 = downshift_to_force_giga_p11 + 1;					
			}
			else if ((page_0_reg_1_temp != 0) && (dev_1e_reg_a2_temp != 0) && (downshift_to_force_giga_p11 > force_slave_limit))
			{
				if(link_state_polling == 1)
				{ 
					printk("phy=%d downshift to force giga over times, keep 100M \n\r",phyaddr);
				}					
			}
		}
		else
		{
			if ((link_state_polling == 1) && ((ephy_monitor_all_counter % 80) == 1))
			{ 
				printk("phy=%d already netgear downshift to 100M \n\r",phyaddr);
			}
		}
	}

	if(phyaddr == 12)
	{
		if (netgear_downshift_100M_flag_p12 == 0)
		{
			if ((page_0_reg_1_temp != 0) && (dev_1e_reg_a2_temp != 0) && (downshift_to_force_giga_p12 == force_master_limit))
			{
				tcMiiStationWrite(phyaddr, 0x9 ,0x1e00); //Force master
				tcMiiStationWrite(phyaddr, 0x0 ,0x1240); //Re-AN
				workaround_flag_p12 = workaround_flag_p12 | BIT0;
				if (link_state_polling == 1)
				{
					printk("phy=%d downshift to force giga master \n\r", phyaddr);
				}
				downshift_to_force_giga_p12 = downshift_to_force_giga_p12 + 1;
			}
			else if ((page_0_reg_1_temp != 0) && (dev_1e_reg_a2_temp != 0) && (downshift_to_force_giga_p12 == force_slave_limit))
			{
				tcMiiStationWrite(phyaddr, 0x9 ,0x1600); //Force slave
				tcMiiStationWrite(phyaddr, 0x0 ,0x1240); //Re-AN
				workaround_flag_p12 = workaround_flag_p12 | BIT1;
				if (link_state_polling == 1)
				{
					printk("phy=%d downshift to force giga slave \n\r", phyaddr);
				}
				downshift_to_force_giga_p12 = downshift_to_force_giga_p12 + 1;					
			}
			else if ((page_0_reg_1_temp != 0) && (dev_1e_reg_a2_temp != 0) && (downshift_to_force_giga_p12 > force_slave_limit))
			{
				if(link_state_polling == 1)
				{ 
					printk("phy=%d downshift to force giga over times, keep 100M \n\r",phyaddr);
				}					
			}
		}
		else
		{
			if ((link_state_polling == 1) && ((ephy_monitor_all_counter % 80) == 1))
			{ 
				printk("phy=%d already netgear downshift to 100M \n\r",phyaddr);
			}
		}
	}

	dev_1e_reg_a2_temp = (dev_1e_reg_a2 & 0x0008); //Final speed is 1000M

	if ((phyaddr == 9) && (downshift_to_force_giga_p9 > 0))
	{
		if ((page_0_reg_1_temp == 0) || (dev_1e_reg_a2_temp != 0)) { downshift_to_force_giga_p9 = 0; tcMiiStationWrite(phyaddr, 0x9 ,0x0600);}
	}
	if ((phyaddr == 10) && (downshift_to_force_giga_p10 > 0))
	{
		if ((page_0_reg_1_temp == 0) || (dev_1e_reg_a2_temp != 0)) { downshift_to_force_giga_p10 = 0; tcMiiStationWrite(phyaddr, 0x9 ,0x0600);}
	}
	if ((phyaddr == 11) && (downshift_to_force_giga_p11 > 0))
	{
		if ((page_0_reg_1_temp == 0) || (dev_1e_reg_a2_temp != 0)) { downshift_to_force_giga_p11 = 0; tcMiiStationWrite(phyaddr, 0x9 ,0x0600);}
	}
	if ((phyaddr == 12) && (downshift_to_force_giga_p12 > 0))
	{
		if ((page_0_reg_1_temp == 0) || (dev_1e_reg_a2_temp != 0)) { downshift_to_force_giga_p12 = 0; tcMiiStationWrite(phyaddr, 0x9 ,0x0600);}
	}
}

void netgear_downshift_100M_main(unsigned int phy,unsigned int reg1)
{
	unsigned int reg1_value,tkr_temp,netgear_downshift_100M_limit=4,slave_cnt=2,unlik_free=100,temp,prefer_master_kepp_link_all=12;
	reg1_value=reg1&0x04;
	tkr_temp=toKenRingRead(phy,0x0,0xf,0x1);
	tkr_temp=tkr_temp&0x0030;
	//printk("netgear_downshift_100M_main tkr_temp =%x \n\r",tkr_temp);
	if(reg1_value == 0)
    {
			if(phy==9)
			{
				if(netgear_downshift_100M_last_p9!=tkr_temp)
				{
					if(tkr_temp==0x0030)
		            {
					if(netgear_downshift_100M_start_p9==0){netgear_downshift_100M_start_p9=1;if(ephy_monitor_all_counter >=0xfffeffc4) {netgear_downshift_100M_end_p9=0;} else {netgear_downshift_100M_end_p9=ephy_monitor_all_counter;}}
					netgear_downshift_100M_counter_p9=netgear_downshift_100M_counter_p9+1;
					if(netgear_downshift_100M_counter_p9==slave_cnt) {tcMiiStationWrite(phy, 0x9,0x1600);tcMiiStationWrite(phy, 0x0,0x1240);workaround_flag_p9 = workaround_flag_p9 | BIT3; if(link_state_polling == 1){printk("phy=%d netgear downshift force slave retry \n\r", phy);}}
					if(netgear_downshift_100M_counter_p9>=netgear_downshift_100M_limit)
					{
						tcMiiStationWrite(phy, 0x9,0x0000);
						tcMiiStationWrite(phy, 0x0,0x1240);
						netgear_downshift_100M_start_p9=0;netgear_downshift_100M_end_p9=0;netgear_downshift_100M_counter_p9=0;
						netgear_downshift_100M_into_p9=1;
						netgear_downshift_100M_flag_p9 = 1;
						workaround_flag_p9 = workaround_flag_p9 | BIT4;
						if(link_state_polling==1){printk(" phy=%d netgear downshift\n\r",phy);}
					}}}
				netgear_downshift_100M_last_p9=tkr_temp;
				if(netgear_downshift_100M_start_p9==1)
				{
					temp=ephy_monitor_all_counter-netgear_downshift_100M_end_p9;
					if(temp>=unlik_free){netgear_downshift_100M_start_p9=0;netgear_downshift_100M_end_p9=0;netgear_downshift_100M_counter_p9=0;if(link_state_polling==1){printk(" phy=%d netgear time out\n\r",phy);} }
				    
				}	
			}		
		
			if(phy==10)
			{
				if(netgear_downshift_100M_last_p10!=tkr_temp)
				{
				
			        if(tkr_temp==0x0030)
		            {
					if(netgear_downshift_100M_start_p10==0){netgear_downshift_100M_start_p10=1;if(ephy_monitor_all_counter >=0xfffeffc4) {netgear_downshift_100M_end_p10=0;} else {netgear_downshift_100M_end_p10=ephy_monitor_all_counter;}}
					netgear_downshift_100M_counter_p10=netgear_downshift_100M_counter_p10+1;
					if(netgear_downshift_100M_counter_p10==slave_cnt) {tcMiiStationWrite(phy, 0x9,0x1600);tcMiiStationWrite(phy, 0x0,0x1240);workaround_flag_p10 = workaround_flag_p10 | BIT3; if(link_state_polling == 1){printk("phy=%d netgear downshift force slave retry \n\r", phy);}}
					if(netgear_downshift_100M_counter_p10>=netgear_downshift_100M_limit)
					{
						tcMiiStationWrite(phy, 0x9,0x0000);
						tcMiiStationWrite(phy, 0x0,0x1240);
						netgear_downshift_100M_start_p10=0;netgear_downshift_100M_end_p10=0;netgear_downshift_100M_counter_p10=0;
						netgear_downshift_100M_into_p10=1;
						netgear_downshift_100M_flag_p10 = 1;
						workaround_flag_p10 = workaround_flag_p10 | BIT4;
						if(link_state_polling==1){printk(" phy=%d netgear downshift\n\r",phy);}
					}}}
			     netgear_downshift_100M_last_p10=tkr_temp;
				 if(netgear_downshift_100M_start_p10==1)
				{
					temp=ephy_monitor_all_counter-netgear_downshift_100M_end_p10;
					if(temp>=unlik_free){netgear_downshift_100M_start_p10=0;netgear_downshift_100M_end_p10=0;netgear_downshift_100M_counter_p10=0; if(link_state_polling==1){printk(" phy=%d netgear time out\n\r",phy);}}
				    
				}
			}
		
		
			if(phy==11)
			{
				if(netgear_downshift_100M_last_p11!=tkr_temp)
				{
					if(tkr_temp==0x0030)
		            {
					if(netgear_downshift_100M_start_p11==0){netgear_downshift_100M_start_p11=1;if(ephy_monitor_all_counter >=0xfffeffc4) {netgear_downshift_100M_end_p11=0;} else {netgear_downshift_100M_end_p11=ephy_monitor_all_counter;}}
					netgear_downshift_100M_counter_p11=netgear_downshift_100M_counter_p11+1;
					if(netgear_downshift_100M_counter_p11==slave_cnt) {tcMiiStationWrite(phy, 0x9,0x1600);tcMiiStationWrite(phy, 0x0,0x1240);workaround_flag_p11 = workaround_flag_p11 | BIT3; if(link_state_polling == 1){printk("phy=%d netgear downshift force slave retry \n\r", phy);}}
					if(netgear_downshift_100M_counter_p11>=netgear_downshift_100M_limit)
					{
						tcMiiStationWrite(phy, 0x9,0x0000);
						tcMiiStationWrite(phy, 0x0,0x1240);
						netgear_downshift_100M_start_p11=0;netgear_downshift_100M_end_p11=0;netgear_downshift_100M_counter_p11=0;
						netgear_downshift_100M_into_p11=1;
						netgear_downshift_100M_flag_p11 = 1;
						workaround_flag_p11 = workaround_flag_p11 | BIT4;
						if(link_state_polling==1){printk(" phy=%d netgear downshift\n\r",phy);}
					}}}
				netgear_downshift_100M_last_p11=tkr_temp;
				if(netgear_downshift_100M_start_p11==1)
				{
					temp=ephy_monitor_all_counter-netgear_downshift_100M_end_p11;
					if(temp>=unlik_free){netgear_downshift_100M_start_p11=0;netgear_downshift_100M_end_p11=0;netgear_downshift_100M_counter_p11=0; if(link_state_polling==1){printk(" phy=%d netgear time out\n\r",phy);}}
				    
				}
			}
		
		
			if(phy==12)
			{
				if(netgear_downshift_100M_last_p12!=tkr_temp)
				{
				if(tkr_temp==0x0030)
		            {
					if(netgear_downshift_100M_start_p12==0){netgear_downshift_100M_start_p12=1;if(ephy_monitor_all_counter >=0xfffeffc4) {netgear_downshift_100M_end_p12=0;} else {netgear_downshift_100M_end_p12=ephy_monitor_all_counter;}}
					netgear_downshift_100M_counter_p12=netgear_downshift_100M_counter_p12+1;
					if(netgear_downshift_100M_counter_p12==slave_cnt) {tcMiiStationWrite(phy, 0x9,0x1600);tcMiiStationWrite(phy, 0x0,0x1240);workaround_flag_p12 = workaround_flag_p12 | BIT3; if(link_state_polling == 1){printk("phy=%d netgear downshift force slave retry \n\r", phy);}}
					if(netgear_downshift_100M_counter_p12>=netgear_downshift_100M_limit)
					{
						tcMiiStationWrite(phy, 0x9,0x0000);
						tcMiiStationWrite(phy, 0x0,0x1240);
						netgear_downshift_100M_start_p12=0;netgear_downshift_100M_end_p12=0;netgear_downshift_100M_counter_p12=0;
						netgear_downshift_100M_into_p12=1;
						netgear_downshift_100M_flag_p12 = 1;
						workaround_flag_p12 = workaround_flag_p12 | BIT4;
						if(link_state_polling==1){printk(" phy=%d netgear downshift\n\r",phy);}
					}}}
				netgear_downshift_100M_last_p12=tkr_temp;
				if(netgear_downshift_100M_start_p12==1)
				{
					temp=ephy_monitor_all_counter-netgear_downshift_100M_end_p12;
					if(temp>=unlik_free){netgear_downshift_100M_start_p12=0;netgear_downshift_100M_end_p12=0;netgear_downshift_100M_counter_p12=0; if(link_state_polling==1){printk(" phy=%d netgear time out\n\r",phy);}}
					
				}
			}
		
	} else {
		//link
		//wschung modify ...REG09 should go back to 0c0600
		if (disable_force_master_flag){			
			if (phy == 9) { if (prefer_master_kepp_link_p9 >= prefer_master_kepp_link_all) { if (netgear_downshift_100M_counter_p9 >= slave_cnt) { tcMiiStationWrite(phy, 0x9, 0x0600); } netgear_downshift_100M_start_p9 = 0; netgear_downshift_100M_end_p9 = 0; netgear_downshift_100M_counter_p9 = 0; if (netgear_downshift_100M_into_p9 == 1) { netgear_downshift_100M_into_p9 = 0; } } }
		  if(phy==10){ if( prefer_master_kepp_link_p10 >= prefer_master_kepp_link_all) { if (netgear_downshift_100M_counter_p10 >= slave_cnt) { tcMiiStationWrite(phy, 0x9, 0x0600); } netgear_downshift_100M_start_p10=0;netgear_downshift_100M_end_p10=0;netgear_downshift_100M_counter_p10=0; if(netgear_downshift_100M_into_p10==1){netgear_downshift_100M_into_p10=0;}}}
		  if(phy==11){ if( prefer_master_kepp_link_p11 >= prefer_master_kepp_link_all) { if (netgear_downshift_100M_counter_p11 >= slave_cnt) { tcMiiStationWrite(phy, 0x9, 0x0600); } netgear_downshift_100M_start_p11=0;netgear_downshift_100M_end_p11=0;netgear_downshift_100M_counter_p11=0; if(netgear_downshift_100M_into_p11==1){netgear_downshift_100M_into_p11=0;}}}
		  if(phy==12){ if( prefer_master_kepp_link_p12 >= prefer_master_kepp_link_all) { if (netgear_downshift_100M_counter_p12 >= slave_cnt) { tcMiiStationWrite(phy, 0x9, 0x0600); } netgear_downshift_100M_start_p12=0;netgear_downshift_100M_end_p12=0;netgear_downshift_100M_counter_p12=0; if(netgear_downshift_100M_into_p12==1){netgear_downshift_100M_into_p12=0; }}}
		} else {
  		  if(phy==9){ if( prefer_master_kepp_link_p9 >= prefer_master_kepp_link_all) { if (netgear_downshift_100M_counter_p9 >= slave_cnt) { tcMiiStationWrite(phy, 0x9, 0x1e00); } netgear_downshift_100M_start_p9=0;netgear_downshift_100M_end_p9=0;netgear_downshift_100M_counter_p9=0; if(netgear_downshift_100M_into_p9==1){netgear_downshift_100M_into_p9=0; }}}
		  if(phy==10){ if( prefer_master_kepp_link_p10 >= prefer_master_kepp_link_all) { if (netgear_downshift_100M_counter_p10 >= slave_cnt) { tcMiiStationWrite(phy, 0x9, 0x1e00); } netgear_downshift_100M_start_p10=0;netgear_downshift_100M_end_p10=0;netgear_downshift_100M_counter_p10=0; if(netgear_downshift_100M_into_p10==1){netgear_downshift_100M_into_p10=0;}}}
		  if(phy==11){ if( prefer_master_kepp_link_p11 >= prefer_master_kepp_link_all) { if (netgear_downshift_100M_counter_p11 >= slave_cnt) { tcMiiStationWrite(phy, 0x9, 0x1e00); } netgear_downshift_100M_start_p11=0;netgear_downshift_100M_end_p11=0;netgear_downshift_100M_counter_p11=0; if(netgear_downshift_100M_into_p11==1){netgear_downshift_100M_into_p11=0;}}}
		  if(phy==12){ if( prefer_master_kepp_link_p12 >= prefer_master_kepp_link_all) { if (netgear_downshift_100M_counter_p12 >= slave_cnt) { tcMiiStationWrite(phy, 0x9, 0x1e00); } netgear_downshift_100M_start_p12=0;netgear_downshift_100M_end_p12=0;netgear_downshift_100M_counter_p12=0; if(netgear_downshift_100M_into_p12==1){netgear_downshift_100M_into_p12=0; }}}
		}
		
	}
	

}


void reltek_downshift_100M_main(unsigned int phy,unsigned int reg1)
{
	//RTL8221B
	unsigned int reg1_value,tkr_temp,downshift_100M_limit=40;
	reg1_value=reg1&0x04;
	tkr_temp=toKenRingRead(phy,0x0,0xf,0x1);
	tkr_temp=tkr_temp&0xf000;
	//printk("reltek_downshift_100M_main tkr_temp =%x \n\r",tkr_temp);
	if(reg1_value == 0)
    {
		if(phy==9){
		if(tkr_temp == 0xa000)
		{
			reltek_downshift_100M_counter_p9=reltek_downshift_100M_counter_p9+1;
			if(reltek_downshift_100M_counter_p9>= downshift_100M_limit) 
			{ 
		      reltek_downshift_100M_into_p9=1;
			  reltek_downshift_100M_counter_p9=0;
			  tcMiiStationWrite(phy, 0x9,0x0000);
			  tcMiiStationWrite(phy, 0x0,0x1240);
			  netgear_downshift_100M_flag_p9 = 1;
			  workaround_flag_p9 = workaround_flag_p9 | BIT5;
			  if(link_state_polling==1){printk(" phy=%d reltek downshift\n\r",phy);}
			
			}
		} else { reltek_downshift_100M_counter_p9=0;}
		}
		
		if(phy==10){
		if(tkr_temp == 0xa000)
		{
			reltek_downshift_100M_counter_p10=reltek_downshift_100M_counter_p10+1;
			if(reltek_downshift_100M_counter_p10>= downshift_100M_limit) 
			{ 
		      reltek_downshift_100M_into_p10=1;
			  reltek_downshift_100M_counter_p10=0;
			  tcMiiStationWrite(phy, 0x9,0x0000);
			  tcMiiStationWrite(phy, 0x0,0x1240);
			  netgear_downshift_100M_flag_p10 = 1;
			  workaround_flag_p10 = workaround_flag_p10 | BIT5;
			  if(link_state_polling==1){printk(" phy=%d reltek downshift\n\r",phy);}
			
			}
		} else { reltek_downshift_100M_counter_p10=0;}
		}
		
		if(phy==11){
		if(tkr_temp == 0xa000)
		{
			reltek_downshift_100M_counter_p11=reltek_downshift_100M_counter_p11+1;
			if(reltek_downshift_100M_counter_p11>= downshift_100M_limit) 
			{ 
		      reltek_downshift_100M_into_p11=1;
			  reltek_downshift_100M_counter_p11=0;
			  tcMiiStationWrite(phy, 0x9,0x0000);
			  tcMiiStationWrite(phy, 0x0,0x1240);
			  netgear_downshift_100M_flag_p11 = 1;
			  workaround_flag_p11 = workaround_flag_p11 | BIT5;
			  if(link_state_polling==1){printk(" phy=%d reltek downshift\n\r",phy);}
			
			}
		} else { reltek_downshift_100M_counter_p11=0;}
		}
		
		if(phy==12){
		if(tkr_temp == 0xa000)
		{
			reltek_downshift_100M_counter_p12=reltek_downshift_100M_counter_p12+1;
			if(reltek_downshift_100M_counter_p12>= downshift_100M_limit) 
			{ 
		      reltek_downshift_100M_into_p12=1;
			  reltek_downshift_100M_counter_p12=0;
			  tcMiiStationWrite(phy, 0x9,0x0000);
			  tcMiiStationWrite(phy, 0x0,0x1240);
			  netgear_downshift_100M_flag_p12 = 1;
			  workaround_flag_p12 = workaround_flag_p12 | BIT5;
			  if(link_state_polling==1){printk(" phy=%d reltek downshift\n\r",phy);}
			
			}
		} else { reltek_downshift_100M_counter_p12=0;}
		}	
		
	}
	else 
	{
	
	 //wschung modify ...REG09 should go back to 0c0600
	 if (disable_force_master_flag){ 			 	
		if(phy==9){ reltek_downshift_100M_counter_p9=0;if(reltek_downshift_100M_into_p9==1){tcMiiStationWrite(phy, 0x9,0x0600);reltek_downshift_100M_into_p9=0;}}
		if(phy==10){ reltek_downshift_100M_counter_p10=0;if(reltek_downshift_100M_into_p10==1){tcMiiStationWrite(phy, 0x9,0x0600);reltek_downshift_100M_into_p10=0;}}
		if(phy==11){ reltek_downshift_100M_counter_p11=0;if(reltek_downshift_100M_into_p11==1){tcMiiStationWrite(phy, 0x9,0x0600);reltek_downshift_100M_into_p11=0;}}
		if(phy==12){ reltek_downshift_100M_counter_p12=0;if(reltek_downshift_100M_into_p12==1){tcMiiStationWrite(phy, 0x9,0x0600);reltek_downshift_100M_into_p12=0;}}
	 }else {	
		if(phy==9){ reltek_downshift_100M_counter_p9=0;if(reltek_downshift_100M_into_p9==1){tcMiiStationWrite(phy, 0x9,0x1e00);reltek_downshift_100M_into_p9=0;}}
		if(phy==10){ reltek_downshift_100M_counter_p10=0;if(reltek_downshift_100M_into_p10==1){tcMiiStationWrite(phy, 0x9,0x1e00);reltek_downshift_100M_into_p10=0;}}
		if(phy==11){ reltek_downshift_100M_counter_p11=0;if(reltek_downshift_100M_into_p11==1){tcMiiStationWrite(phy, 0x9,0x1e00);reltek_downshift_100M_into_p11=0;}}
		if(phy==12){ reltek_downshift_100M_counter_p12=0;if(reltek_downshift_100M_into_p12==1){tcMiiStationWrite(phy, 0x9,0x1e00);reltek_downshift_100M_into_p12=0;}}
 	 }
	}
}

//===============================LDPS Function=============================================
u8 check_RX_interrupt(u32 port)
{
	u32 cl45_1e_380 = 0;

	cl45_1e_380 = mtEMiiRegRead(port, 0x1E, 0x380);
	
	if ((cl45_1e_380 & (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5)) != 0)
	{
		return LDPS_RX_RCV_NLP;
	}
	else
	{
		return LDPS_RX_NON_RCV_NLP;
	}
}
void LDPS_Gen_Linkpulse(u32 port)
{
	u32 cl45_1e_342 = 0;

	cl45_1e_342 = mtEMiiRegRead(port, 0x1E, 0x342);
	cl45_1e_342 = (cl45_1e_342 | (BIT0 | BIT4));  //[0]:rg_bypass_nlp_gen; [4]:rg_nlp_gen_p
	mtEMiiRegWrite(port, 0x1E, 0x342, cl45_1e_342);

}

void LDPS_Exit_Linkpulse(u32 port)
{
	u32 cl45_1e_342 = 0;

	cl45_1e_342 = mtEMiiRegRead(port, 0x1E, 0x342);
	cl45_1e_342 = (cl45_1e_342 & 0xFFEE); //Clear bit 4 and bit 0
	mtEMiiRegWrite(port, 0x1E, 0x342, cl45_1e_342);

}

void LDPS_NLP_sd_mask_en(u32 port)
{
	u32 cl45_1e_342 = 0;
	cl45_1e_342 = mtEMiiRegRead(port, 0x1E, 0x342);
	cl45_1e_342 = (cl45_1e_342 | BIT8);
	mtEMiiRegWrite(port, 0x1E, 0x342, cl45_1e_342);
}

void LDPS_NLP_sd_mask_dis(u32 port)
{
	u32 cl45_1e_342 = 0;
	cl45_1e_342 = mtEMiiRegRead(port, 0x1E, 0x342);
	cl45_1e_342 = (cl45_1e_342 & 0xFEFF);  //clear bit 8
	mtEMiiRegWrite(port, 0x1E, 0x342, cl45_1e_342);
}

void LDPS_TX_normal(u32 port)
{
	mtEMiiRegWrite(port, 0x1E, 0x3E, 0xC000);
}

void LDPS_TX_idle(u32 port)
{
	mtEMiiRegWrite(port, 0x1E, 0x3E, 0xF8F8);
}

void LDPS_clear_nlp_det_int(u32 port)
{
	u32 cl45_1e_380 = 0;

	cl45_1e_380 = mtEMiiRegRead(port, 0x1E, 0x380);

	if ((cl45_1e_380 & BIT0) != 0)
	{
		mtEMiiRegWrite(port, 0x1E, 0x380, BIT0);    //w1c nlp_rec_done_int
	}

	if ((cl45_1e_380 & BIT1) != 0)
	{
		mtEMiiRegWrite(port, 0x1E, 0x380, BIT1);    //w1c nlp_signal_detect_int
	}

	if ((cl45_1e_380 & BIT2) != 0)
	{
		mtEMiiRegWrite(port, 0x1E, 0x380, BIT2);    //w1c nlp_adc_detect_int
	}

	if ((cl45_1e_380 & BIT3) != 0)
	{
		mtEMiiRegWrite(port, 0x1E, 0x380, BIT3);    //w1c nlp_rsv_int
	}

	if ((cl45_1e_380 & BIT4) != 0)
	{
		mtEMiiRegWrite(port, 0x1E, 0x380, BIT4);    //w1c nlp_energy0_detect_int
	}
	if ((cl45_1e_380 & BIT5) != 0)
	{
		mtEMiiRegWrite(port, 0x1E, 0x380, BIT5);    //w1c nlp_energy1_detect_int
	}
}

void LDPS_clear_nlp_transfer_int(u32 port)
{
	u32 cl45_1e_380 = 0;

	cl45_1e_380 = mtEMiiRegRead(port, 0x1E, 0x380);

	if ((cl45_1e_380 & BIT6) != 0)
	{
		mtEMiiRegWrite(port, 0x1E, 0x380, BIT6);    //w1c nlp_transfer_complete_int
	}
}

void LDPS_Chk_status(void)
{
	u32 phyaddr = 0, gphy_base = 9, pn = 0, page1_val = 0;
	for (phyaddr = gphy_base; phyaddr < (gphy_base + EN75xxGe_PORTNUM); phyaddr++)
	{
		pn = phyaddr - gphy_base;
		page1_val = tcMiiStationRead(phyaddr, 1);
		LDPS_check_status[pn].link_status_prev = LDPS_check_status[pn].link_status;
		LDPS_check_status[pn].link_status = (page1_val >> 2) & 0x00000001;
	}

	for (phyaddr = gphy_base; phyaddr < (gphy_base + EN75xxGe_PORTNUM); phyaddr++)
	{
		pn = phyaddr - gphy_base;
		if (LDPS_check_status[pn].link_status_prev && !LDPS_check_status[pn].link_status) //UP2DOWN to clear RX detect
		{
			LDPS_check_status[pn].value = 0; //Clear continue NLP counter
			LDPS_clear_nlp_det_int(phyaddr);
			LDPS_RX_BUSY_CNT = 0;
			if (link_state_polling == 1)
			{
				printk("phy=%d link up to down, LDPS clear RX detector\n\r", phyaddr);
			}
		}
	}
}

void LDPS_main(void)
{
	u32 phyaddr = 0, gphy_base = 9, pn = 0;
	u8 ldps_active_cnt = 20, nlp_continue_threshold = 2;

	for(phyaddr = gphy_base; phyaddr < (gphy_base+EN75xxGe_PORTNUM); phyaddr++) 
	{
		pn = phyaddr - gphy_base;
		LDPS_rx_status = check_RX_interrupt(phyaddr);

		if (LDPS_rx_status == LDPS_RX_RCV_NLP) 
		{
			LDPS_RX_IDLE_CNT = 0;
			LDPS_check_status[pn].value = LDPS_check_status[pn].value + 1;
			LDPS_clear_nlp_det_int(phyaddr);
			if (link_state_polling == 1)
			{
				printk("phyaddr = %d received %d nlp continously \r\n", phyaddr, LDPS_check_status[pn].value);
			}
		}

		//RX received 2 NLP contineously => Confirm Link partner attached
		if ((LDPS_check_status[pn].value >= nlp_continue_threshold) || (LDPS_check_status[pn].link_status == 1))
		{
			LDPS_rx_status = LDPS_RX_RCV_NLP;
			break;
		}
		else
		{
			LDPS_rx_status = LDPS_RX_NON_RCV_NLP;
		}
	}
	

	if (LDPS_rx_status == LDPS_RX_RCV_NLP)
	{
		LDPS_RX_IDLE_CNT = 0;
		LDPS_RX_BUSY_CNT += 1;
		for(phyaddr = gphy_base; phyaddr < (gphy_base+EN75xxGe_PORTNUM); phyaddr++) 
		{
			LDPS_TX_normal(phyaddr);
			LDPS_Exit_Linkpulse(phyaddr);
		}

		
		if (LDPS_RX_BUSY_CNT >= 0xFFFF)
		{
			LDPS_RX_BUSY_CNT = 0xFFFF;
		}
		
	}
	else if (LDPS_rx_status == LDPS_RX_NON_RCV_NLP)
	{
		LDPS_RX_BUSY_CNT = 0;
		LDPS_RX_IDLE_CNT += 1;

		if (LDPS_RX_IDLE_CNT > ldps_active_cnt)  //RX detector idle over N seconds
		{
			for (phyaddr = gphy_base; phyaddr < (gphy_base + EN75xxGe_PORTNUM); phyaddr++)
			{
				pn = phyaddr - gphy_base;
				LDPS_check_status[pn].value = 0;           //Clear continue NLP counter
				LDPS_TX_normal(phyaddr);
				LDPS_clear_nlp_transfer_int(phyaddr);      // clear nlp_transfer_complete_int
				LDPS_Gen_Linkpulse(phyaddr);               // transfer nlp
				LDPS_clear_nlp_det_int(phyaddr);
			}
			mdelay(1);
			for (phyaddr = gphy_base; phyaddr < (gphy_base + EN75xxGe_PORTNUM); phyaddr++)
			{
					LDPS_TX_idle(phyaddr);
			}
		}

		if (LDPS_RX_IDLE_CNT >= 0xFFFF)
		{
			LDPS_RX_IDLE_CNT = 0xFFFF;
		}
	}

	LDPS_Chk_status();   //Check link status to clear RX detector or not

}

void LDPS_Framework(void)
{
	u32 	gphy_base = 9, phyaddr = 9, LDPS_Framework_counter = 4;

	if (LDPS_function_en == 1)   //LDPS switch
	{
		if ((LDPS_Framework_cnt >= LDPS_Framework_counter) && (LDPS_nlp_sd_mask_flag == 0))
		{
			for (phyaddr = gphy_base; phyaddr < (gphy_base + EN75xxGe_PORTNUM); phyaddr++)
			{
				LDPS_NLP_sd_mask_dis(phyaddr);  //MASK RX Detect 50 cycle in the begining
			}

			LDPS_nlp_sd_mask_flag = 1;
		}
		else if (LDPS_Framework_cnt < LDPS_Framework_counter)
		{
			for (phyaddr = gphy_base; phyaddr < (gphy_base + EN75xxGe_PORTNUM); phyaddr++)
			{
				LDPS_clear_nlp_det_int(phyaddr);
				LDPS_NLP_sd_mask_en(phyaddr);
			}
		}

		LDPS_main();
		LDPS_function_skip_flag = 0;  //This flag to prevent set normal setting every cycle
	}
	else
	{
		if (LDPS_function_skip_flag == 0)
		{
			for (phyaddr = gphy_base; phyaddr < (gphy_base + EN75xxGe_PORTNUM); phyaddr++)
			{
				if (testmode_excuted_flag == 0)
				{
					LDPS_TX_normal(phyaddr);
				}
				else
				{
					//do nothing
				}
				LDPS_Exit_Linkpulse(phyaddr);
				LDPS_NLP_sd_mask_dis(phyaddr);
			}

			LDPS_function_skip_flag = 1;
		}
		else
		{
			//do nothing
		}
	}

	LDPS_Framework_cnt = LDPS_Framework_cnt + 1;

	if (LDPS_Framework_cnt >= 0xFFFF0000)
	{
		LDPS_Framework_cnt = 0xFFFF0000;
	}
}

void ephy_dis_LDPS(void)
{
	LDPS_function_en = 0;
	LDPS_function_skip_flag = 0;
}

//===============================LDPS Function End=============================================

void en75xxGePhySwPatch(void)
{
	u32 	reg0, reg1, reg9, reg10,reg_temp,reg_9_temp,reg_10_temp,reg_temp1,master_times=1,auto_times=3,prefer_master_kepp_link_all=12;
	u32 	dev_1e_reg_a2,dev_1e_reg_a2_temp,power_down;	
	u32 	pn = 0, gphy_base = 9, phyaddr = 9, phy_base = 9;

	#if 0
    if(power_on_finish_flag == 0)
    {
    	if((tcMiiStationRead(12, 0x09)&0xE000) == 0xE000) // ? 
      	{
        	power_on_finish_flag = 1;
        	printk("\n Power On Process Finish \n");
        	//tcMiiStationWrite(12, 0x09, 0x0700);
      	}
    }
    #endif
	
    for(phyaddr = gphy_base; phyaddr < (gphy_base+EN75xxGe_PORTNUM); phyaddr++) 
	{
		pn = phyaddr - phy_base;
        mtPhyReadReg(pn,1);
        reg0 = tcMiiStationRead(phyaddr, 0x0);
		power_down=reg0&0x0800;
		reg1 = tcMiiStationRead(phyaddr, 0x1);
		reg9 = tcMiiStationRead(phyaddr, 0x9);
		reg10 = tcMiiStationRead(phyaddr, 0xa);
		dev_1e_reg_a2 = mtEMiiRegRead(phyaddr,0x1E,0xa2);
		reg_9_temp=reg9;
		reg_10_temp=reg10;
        if( !Nmr1[pn].link_status_prev && !Nmr1[pn].link_status )
		{
            mtphy_link_state[pn] = ST_LINK_DOWN;

        }
        else if( !Nmr1[pn].link_status_prev && Nmr1[pn].link_status )
		{
            mtphy_link_state[pn] = ST_LINK_DOWN2UP;
			if((link_state_polling == 1)||(slt_excuting==1)){ 
			reg_temp=dev_1e_reg_a2&0x002e;            
			if(reg_temp ==0x22) {printk("phy=%d link 10M full duplex",phyaddr);}
			if(reg_temp ==0x02) {printk("phy=%d link 10M half duplex",phyaddr);}
			if(reg_temp ==0x24) {printk("phy=%d link 100M full duplex",phyaddr);}
			if(reg_temp ==0x04) {printk("phy=%d link 100M half duplex",phyaddr);}
			if(reg_temp ==0x28) {printk("phy=%d link 1000M full duplex",phyaddr);reg_temp=dev_1e_reg_a2&0x0010;if(reg_temp ==0x10) {printk(" Master");} else {printk(" Slave");}}
			if(reg_temp ==0x08) {printk("phy=%d link 1000M ",phyaddr);reg_temp=dev_1e_reg_a2&0x0010;if(reg_temp ==0x10) {printk(" Master");} else {printk(" Slave");}}
			printk("\n\r");			
			printk("phy=%d link down to up\n\r",phyaddr);}
        }
        else if( Nmr1[pn].link_status_prev && !Nmr1[pn].link_status )
		{
            mtphy_link_state[pn]= ST_LINK_UP2DOWN;
			if( sw_down_shift_en ==1 )
			{
			if((ephy_monitor_p9_stop_polling==1)&&(phyaddr ==9)) { ephy_monitor_p9_stop_polling=0;}
			if((ephy_monitor_p10_stop_polling==1)&&(phyaddr ==10)) { ephy_monitor_p10_stop_polling=0;}
			if((ephy_monitor_p11_stop_polling==1)&&(phyaddr ==11)) { ephy_monitor_p11_stop_polling=0;}
			if((ephy_monitor_p12_stop_polling==1)&&(phyaddr ==12)) { ephy_monitor_p12_stop_polling=0;}		
			}
			if(phyaddr ==9) {prefer_master_kepp_link_p9=0; netgear_downshift_100M_flag_p9 = 0; workaround_linkdown_flag_p9 = workaround_flag_p9; workaround_flag_p9 = 0;}
			if(phyaddr ==10) {prefer_master_kepp_link_p10=0; netgear_downshift_100M_flag_p10 = 0; workaround_linkdown_flag_p10 = workaround_flag_p10; workaround_flag_p10 = 0;}
			if(phyaddr ==11) {prefer_master_kepp_link_p11=0; netgear_downshift_100M_flag_p11 = 0; workaround_linkdown_flag_p11 = workaround_flag_p11; workaround_flag_p11 = 0;}
			if(phyaddr ==12) {prefer_master_kepp_link_p12=0; netgear_downshift_100M_flag_p12 = 0; workaround_linkdown_flag_p12 = workaround_flag_p12; workaround_flag_p12 = 0;}
			if((link_state_polling == 1)||(slt_excuting==1)){ printk("phy=%d link up to down\n\r",phyaddr);}
       }
        else 
		{ 	//if( Nmr1[pn].link_status_prev && Nmr1[pn].link_status ){
            mtphy_link_state[pn] = ST_LINK_UP;
			if( slt_excuting==0 ) {
		    if( phyaddr ==9 ) { 
			prefer_master_kepp_link_p9=prefer_master_kepp_link_p9+1;
			if( prefer_master_kepp_link_p9 >= prefer_master_kepp_link_all) { force_giga_master_mode_p9=0;force_giga_master_counter_p9=0;}}
			if( phyaddr ==10 ) { 
			prefer_master_kepp_link_p10=prefer_master_kepp_link_p10+1;
			if( prefer_master_kepp_link_p10 >= prefer_master_kepp_link_all) { force_giga_master_mode_p10=0;force_giga_master_counter_p10=0;}}
			if( phyaddr ==11 ) { 
			prefer_master_kepp_link_p11=prefer_master_kepp_link_p11+1;
			if( prefer_master_kepp_link_p11 >= prefer_master_kepp_link_all) { force_giga_master_mode_p11=0;force_giga_master_counter_p11=0;}}
			if( phyaddr ==12 ) { 
			prefer_master_kepp_link_p12=prefer_master_kepp_link_p12+1;
			if( prefer_master_kepp_link_p12 >= prefer_master_kepp_link_all) { force_giga_master_mode_p12=0;force_giga_master_counter_p12=0;}}
            // set giga force master				
			//wschung modify ...
			if (!disable_force_master_flag)
			{
			  	reg_temp=reg_9_temp&0x0200;        
			  	if( reg_temp !=0 ) 		    
			  	{									
					reg_temp=reg_9_temp&0x1800;            
					if( reg_temp !=0x1800 ) 			
					{					
				 		reg_temp=(reg_9_temp&0xe7ff)|0x1800;			
				  		tcMiiStationWrite(phyaddr, 0x09, reg_temp); 	// set giga force master			
					}
			 	}
			}

			if (((reg9 & 0x0200) == 0x0200) && ((reg0 & 0x1000) == 0x1000))
			{
				if (phy_downshift_force_giga_enable == 1) { downshift_force_giga(phyaddr, dev_1e_reg_a2); }
			}
		
			//check mean square error
			if( sw_down_shift_en ==1 ) { 
			if((sw_down_shift_p9==1)&&(phyaddr ==9)) { sw_down_shift_p9=0;tcMiiStationWrite(phyaddr, 0x09, 0x1e00);}
			if((sw_down_shift_p10==1)&&(phyaddr ==10)) { sw_down_shift_p10=0;tcMiiStationWrite(phyaddr, 0x09, 0x1e00);}
			if((sw_down_shift_p11==1)&&(phyaddr ==11)) { sw_down_shift_p11=0;tcMiiStationWrite(phyaddr, 0x09, 0x1e00);}
			if((sw_down_shift_p12==1)&&(phyaddr ==12)) { sw_down_shift_p12=0;tcMiiStationWrite(phyaddr, 0x09, 0x1e00);}

			dev_1e_reg_a2_temp = dev_1e_reg_a2 &0x0008;
			if(dev_1e_reg_a2_temp ==8)
			{
                //printk("port %d mean_square_error_counter_p10=%d --" ,phyaddr,mean_square_error_counter_p10);			
				if((ephy_monitor_p9_stop_polling==0)&&(phyaddr ==9)) { sw_downshift(phyaddr);}
				if((ephy_monitor_p10_stop_polling==0)&&(phyaddr ==10)) { sw_downshift(phyaddr);}
				if((ephy_monitor_p11_stop_polling==0)&&(phyaddr ==11)) { sw_downshift(phyaddr);}
				if((ephy_monitor_p12_stop_polling==0)&&(phyaddr ==12)) { sw_downshift(phyaddr);}
			}
			}}
        }
		reg_temp=reg_9_temp&0x0200;		
		reg_temp1=reg1&0x0004;
		if (slt_excuting==0 ) {	
		//This block is for AN force master mode
		if((reg_temp != 0)&&(reg_temp1 == 0)) 								// check 1000Mbps capbility
		{
			//reg_temp=reg_9_temp&0x1000;
			reg_temp1=reg_10_temp&0x8000;
			if( phyaddr ==9 ) {	
		        
			if((reg_temp1!=0)&&(force_giga_master_mode_p9==1))	// local phy is slave && collision
			{
				reg_temp=(reg_9_temp&0xe7ff)|0x0600;
			    tcMiiStationWrite(phyaddr, 0x09, reg_temp); 	// set giga force slave
				force_giga_master_counter_p9=force_giga_master_counter_p9+1;
				workaround_flag_p9 = workaround_flag_p9 | BIT8;
			}
			if((reg_temp1!=0)&&(force_giga_master_mode_p9==1)&&(force_giga_master_counter_p9>=auto_times))	// local phy is slave && collision
			{
				reg_temp=(reg_9_temp&0xe7ff)|0x1800;
			    tcMiiStationWrite(phyaddr, 0x09, reg_temp); 	// set giga auto master slave
				force_giga_master_counter_p9=0;
				force_giga_master_mode_p9++;
				workaround_flag_p9 = workaround_flag_p9 | BIT9;

			}
			if((reg_temp1!=0)&&(force_giga_master_mode_p9==0))	// local phy is slave && collision
			{
				reg_temp=(reg_9_temp&0xe7ff)|0x1800;
			    tcMiiStationWrite(phyaddr, 0x09, reg_temp); 	// set giga force master
				force_giga_master_counter_p9=force_giga_master_counter_p9+1;
				workaround_flag_p9 = workaround_flag_p9 | BIT6;
			}
			if((reg_temp1!=0)&&(force_giga_master_mode_p9==0)&&(force_giga_master_counter_p9>=master_times))	// local phy is slave && collision
			{
				reg_temp=(reg_9_temp&0xe7ff)|0x0600;
			    tcMiiStationWrite(phyaddr, 0x09, reg_temp); 	// set giga force slave
				force_giga_master_counter_p9=0;
				force_giga_master_mode_p9++;
				workaround_flag_p9 = workaround_flag_p9 | BIT7;
				if( (loopback_head_downshift_100M ==1)&&(loopback_haed_start_p9==0) ) { downshift_2pair_capture_counter_p9_counter=ephy_monitor_all_counter;loopback_haed_start_p9=1;}

			}
			if((reg_temp1!=0)&&(force_giga_master_mode_p9==2))	// local phy is slave && collision
			{
				force_giga_master_mode_p9=0;
			}	

			}
			
            if( phyaddr ==10 ) {

			if((reg_temp1!=0)&&(force_giga_master_mode_p10==1))	// local phy is slave && collision
			{
				reg_temp=(reg_9_temp&0xe7ff)|0x0600;
			    tcMiiStationWrite(phyaddr, 0x09, reg_temp); 	// set giga force slave
				force_giga_master_counter_p10=force_giga_master_counter_p10+1;
				workaround_flag_p10 = workaround_flag_p10 | BIT8;
			}
			if((reg_temp1!=0)&&(force_giga_master_mode_p10==1)&&(force_giga_master_counter_p10>=auto_times))	// local phy is slave && collision
			{
				reg_temp=(reg_9_temp&0xe7ff)|0x1800;
			    tcMiiStationWrite(phyaddr, 0x09, reg_temp); 	// set giga auto master slave
				force_giga_master_counter_p10=0;
				force_giga_master_mode_p10++;
				workaround_flag_p10 = workaround_flag_p10 | BIT9;
			}
			if((reg_temp1!=0)&&(force_giga_master_mode_p10==0))	// local phy is slave && collision
			{
				reg_temp=(reg_9_temp&0xe7ff)|0x1800;
			    tcMiiStationWrite(phyaddr, 0x09, reg_temp); 	// set giga force master
				force_giga_master_counter_p10=force_giga_master_counter_p10+1;
				workaround_flag_p10 = workaround_flag_p10 | BIT6;
			}
			if((reg_temp1!=0)&&(force_giga_master_mode_p10==0)&&(force_giga_master_counter_p10>=master_times))	// local phy is slave && collision
			{
				reg_temp=(reg_9_temp&0xe7ff)|0x0600;
			    tcMiiStationWrite(phyaddr, 0x09, reg_temp); 	// set giga force slave
				force_giga_master_counter_p10=0;
				force_giga_master_mode_p10++;
				workaround_flag_p10 = workaround_flag_p10 | BIT7;
				if( (loopback_head_downshift_100M ==1 )&&(loopback_haed_start_p10==0)) { downshift_2pair_capture_counter_p10_counter=ephy_monitor_all_counter;loopback_haed_start_p10=1;}

			}
			if((reg_temp1!=0)&&(force_giga_master_mode_p10==2))	// local phy is slave && collision
			{
				force_giga_master_mode_p10=0;
			}
			}
			
            if( phyaddr ==11 ) {

	
			if((reg_temp1!=0)&&(force_giga_master_mode_p11==1))	// local phy is slave && collision
			{
				reg_temp=(reg_9_temp&0xe7ff)|0x0600;
			    tcMiiStationWrite(phyaddr, 0x09, reg_temp); 	// set giga force slave
				force_giga_master_counter_p11=force_giga_master_counter_p11+1;
				workaround_flag_p11 = workaround_flag_p11 | BIT8;
			}
			if((reg_temp1!=0)&&(force_giga_master_mode_p11==1)&&(force_giga_master_counter_p11>=auto_times))	// local phy is slave && collision
			{
				reg_temp=(reg_9_temp&0xe7ff)|0x1800;
			    tcMiiStationWrite(phyaddr, 0x09, reg_temp); 	// set giga auto master slave
				force_giga_master_counter_p11=0;
				force_giga_master_mode_p11++;
				workaround_flag_p11 = workaround_flag_p11 | BIT9;
			}
			if((reg_temp1!=0)&&(force_giga_master_mode_p11==0))	// local phy is slave && collision
			{
				reg_temp=(reg_9_temp&0xe7ff)|0x1800;
			    tcMiiStationWrite(phyaddr, 0x09, reg_temp); 	// set giga force master
				force_giga_master_counter_p11=force_giga_master_counter_p11+1;
				workaround_flag_p11 = workaround_flag_p11 | BIT6;
			}
			if((reg_temp1!=0)&&(force_giga_master_mode_p11==0)&&(force_giga_master_counter_p11>=master_times))	// local phy is slave && collision
			{
				reg_temp=(reg_9_temp&0xe7ff)|0x0600;
			    tcMiiStationWrite(phyaddr, 0x09, reg_temp); 	// set giga force slave
				force_giga_master_counter_p11=0;
				force_giga_master_mode_p11++;
				workaround_flag_p11 = workaround_flag_p11 | BIT7;
				if((loopback_head_downshift_100M ==1)&&(loopback_haed_start_p11==0)) { downshift_2pair_capture_counter_p11_counter=ephy_monitor_all_counter;loopback_haed_start_p11=1;}

			}
			if((reg_temp1!=0)&&(force_giga_master_mode_p11==2))	// local phy is slave && collision
			{
				force_giga_master_mode_p11=0;
			}
			}
			
            if( phyaddr ==12 ) {	

	
			if((reg_temp1!=0)&&(force_giga_master_mode_p12==1))	// local phy is slave && collision
			{
				reg_temp=(reg_9_temp&0xe7ff)|0x0600;
			    tcMiiStationWrite(phyaddr, 0x09, reg_temp); 	// set giga force slave
				force_giga_master_counter_p12=force_giga_master_counter_p12+1;
				workaround_flag_p12 = workaround_flag_p12 | BIT8;
			}
			if((reg_temp1!=0)&&(force_giga_master_mode_p12==1)&&(force_giga_master_counter_p12>=auto_times))	// local phy is slave && collision
			{
				reg_temp=(reg_9_temp&0xe7ff)|0x1800;
			    tcMiiStationWrite(phyaddr, 0x09, reg_temp); 	// set giga auto master slave
				force_giga_master_counter_p12=0;
				force_giga_master_mode_p12++;
				workaround_flag_p12 = workaround_flag_p12 | BIT9;
			}
			if((reg_temp1!=0)&&(force_giga_master_mode_p12==0))	// local phy is slave && collision
			{
				reg_temp=(reg_9_temp&0xe7ff)|0x1800;
			    tcMiiStationWrite(phyaddr, 0x09, reg_temp); 	// set giga force master
				force_giga_master_counter_p12=force_giga_master_counter_p12+1;
				workaround_flag_p12 = workaround_flag_p12 | BIT6;
			}
			if((reg_temp1!=0)&&(force_giga_master_mode_p12==0)&&(force_giga_master_counter_p12>=master_times))	// local phy is slave && collision
			{
				reg_temp=(reg_9_temp&0xe7ff)|0x0600;
			    tcMiiStationWrite(phyaddr, 0x09, reg_temp); 	// set giga force slave
				force_giga_master_counter_p12=0;
				force_giga_master_mode_p12++;
				workaround_flag_p12 = workaround_flag_p12 | BIT7;
                if((loopback_head_downshift_100M ==1)&&(loopback_haed_start_p12==0)) { downshift_2pair_capture_counter_p12_counter=ephy_monitor_all_counter;loopback_haed_start_p12=1;}
			}
			}
			if((reg_temp1!=0)&&(force_giga_master_mode_p12==2))	// local phy is slave && collision
			{
				force_giga_master_mode_p12=0;
			}

		}  		

		}//if ((slt_excuting==0 ) && (!disable_force_master_flag) )
		
		//force downshift
		 if( loopback_head_downshift_100M ==1 ) { loopback_head_downshift_100M_main(phyaddr,reg_temp1);}   
		 if( (power_down==0)&&(netgear_downshift_100M ==1) ) { netgear_downshift_100M_main(phyaddr,reg1); reltek_downshift_100M_main(phyaddr,reg1);}

        //mtPhyReadReg(phyaddr,5);
        //mtPhyReadGReg(phyaddr,0x1E,0xA2);
		//mtPhyReadGReg(phyaddr,0x7,0x3d);
      
                        /*
        
		if(((reg9>>9)&0x1) == 1) 								// check 1000Mbps capbility
		{
			if((((reg10>>15)&0x1)==1)&&(((reg9>>12)&0x1)==1))	// local phy is manual mode && collision
			{
				tcMiiStationWrite(phyaddr, 0x09, 0x0600); 		// disable manual 
				//en75xxGePhyDisSlaveMode_flag[pn] = 0;
			}

            #if 0 
			if((mtphy_link_state[pn] == ST_LINK_UP) && (en75xxGePhyDisSlaveMode_flag[pn] == 0))
			{
				tcMiiStationWrite(phyaddr, 0x09, 0x1e00); // keep default master 
				//en75xxGePhyDisSlaveMode_flag[pn] = 1;
			}
			#endif
                        */
		

		#if 0
		#ifdef MTPHY_DEBUG
		// display message
		en75xxGePhySwDispMessage(pn);
		#endif

		#ifdef MTPHY_DEBUG
		if(sw_ErrOverMonitor_flag)
			en75xxGePhyUpdateErrOverSum(pn);
		if(sw_ErrOverMonitor_flag) 
			en75xxGePhySwErrOverMonitor(pn); // call after en75xxFEUpdateErrOverSum()
		#endif
		#endif
    }
	ephy_monitor_all_counter++;
	if(ephy_monitor_all_counter >=0xffff0000) { ephy_monitor_all_counter=0;ephy_monitor_p9_curren_counter=0;ephy_monitor_p10_curren_counter=0;ephy_monitor_p11_curren_counter=0;ephy_monitor_p12_curren_counter=0;}

}


int tcPhySwPatch(void)
{
    if(!sw_patch_flag)
    {
       	Skew_update_flag=0;
    	return 0;
    }
	#ifdef PHYPART_DEBUG_SW_PATCH
	printk("PhyPart debug: tcPhySwPatch() in  \r\n");
	#endif
    switch (tcPhyVer) 
	{  
		case tcPhyVer_7583: 
			LDPS_Framework();
			en75xxGePhySwPatch();  				
			break;
    }
	#ifdef PHYPART_DEBUG_SW_PATCH
	printk("PhyPart debug: tcPhySwPatch() out  \r\n");
	#endif
    return 0;
}

void EphyMonitor(void)
{

	if(debugfs_cmd_init == 0)
	{
		printk("*************************Debug********************************** \r\n");
		ephy_debugfs_init();
		debugfs_cmd_init = 1;
		tcPhyVerLookUp(9);
	}
	//{
		
		/* add your monitor code here*/
		//tcPhyChkVal();
		tcPhySwPatch();
		//printk(" ephy monitor \r\n");
	//}
}


