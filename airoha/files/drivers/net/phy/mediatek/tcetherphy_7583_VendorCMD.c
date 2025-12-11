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
//#include <ecnt_hook/ecnt_hook_ether.h>
//#include "tcetherphy_hook.h"
#include "tcetherphy.h"
#include "tcetherphy_define.h"

extern void mtMiiRegWrite(int port_num, int reg_num, int reg_data);
extern u32 mtMiiRegRead(u8 port_num,u8 reg_num);
extern void mtEMiiRegWrite(u32 port_num, u32 dev_num, u32 reg_num, u32 reg_data);;
extern u32 mtEMiiRegRead(u32 port_num, u32 dev_num, u32 reg_num);;
extern int tcMiiStationWrite(u32 phy_addr, u32 phy_reg, u32 phy_data);
extern int tcMiiStationRead(u32 phy_addr, u32 phy_reg);
extern void toKenRingWrite(unsigned int phy, unsigned int ch, unsigned int node, unsigned int data_addr,unsigned int value);
extern unsigned int toKenRingRead(unsigned int phy, unsigned int ch, unsigned int node, unsigned int data_addr);
extern int ephy_debugfs_init(void);

u8   tcSWVer_CMD = 2; 

#endif //LINUX_OS

#ifdef TCPHY_DEBUG


#ifndef LINUX_OS 					/*Linux platform is used sendicmp user-space to send icmp packet*/
typedef struct 
{
    ping_t  *ping_p;        		/* ping pointer         */
} pingEvt_t;
typedef struct 
{
    u32  trace:1;
    u32  echo:1;
    u32  active:1;
    u32  user:1;
    u32  checkCmd:1;
    u32  checkRsp:1;
} pingCmdFlag_t;
#define N_PELOG         20
typedef struct 
{
    mbuf_t* mbp;
    u16  seq1;
    u16  seq2;
    char    cc[4];
} peLog_t;
typedef struct 
{
    u16  index;
    u16  cnt;            		/* data drror counter       */
    peLog_t log[N_PELOG];
} peLogDcb_t;
typedef struct 
{
    pingCmdFlag_t flags;
    int actSocket;
    u32  pingCnt;
    u32  echoSeq;
    u16  option;         		/* ping data option     */
    u16  dataSeq;        		/* data sequence number     */
    char    dataChar;       		/* data character       */
    u8   checkInd;       		/* data check indication    */
    peLogDcb_t* pelog_p;        	/* data log dcb         */
} pingCmdCb_t;
#endif // ndef LINUX_OS
#endif



/************************************************************************
*                        Data for EPHY  (3)
**************************************************************************/
int ephy_monitor_p9_curren_counter=0,ephy_monitor_p10_curren_counter=0,ephy_monitor_p11_curren_counter=0,ephy_monitor_p12_curren_counter=0;
int sw_down_shift_p9=0,sw_down_shift_p10=0,sw_down_shift_p11=0,sw_down_shift_p12=0;
int ephy_monitor_all_counter=0;
int ephy_monitor_p9_stop_polling=0,ephy_monitor_p10_stop_polling=0,ephy_monitor_p11_stop_polling=0,ephy_monitor_p12_stop_polling=0;
u8    sw_down_shift_en=0,phy_downshift_force_giga_enable=1,loopback_head_downshift_100M=0;
u8    command_calbration_enable=0;
u8    link_state_polling = 0, testmode_excuted_flag = 0;
extern u8 LDPS_function_en;

#ifdef TCPHY_SUPPORT

u8 mtphy_link_state[MTPHY_PORTNUM];

// Auto, AN, 100F, 100H, 10F,  10H,
#ifdef TCPHY_1PORT
#define tcphy_speed_Auto  		0
#define tcphy_speed_ForceAN  	1
#define tcphy_speed_Force100F  	2
#define tcphy_speed_Force100H  	3
#define tcphy_speed_Force10F  	4
#define tcphy_speed_Force10H  	5
#endif

#ifdef TCPHY_1PORT
static u8 tcphy_speed = tcphy_speed_Auto;
#endif
#endif 			// TCPHY_SUPPORT

#if defined( TCPHY_DEBUG) || defined(MTPHY_DEBUG)
// variable for doPing()
static u32 PingReplyCnt = 0;
extern u8 phychkval_portnum;
// variable for doPhyForceLink()
static u8 force_link_flag = 0;
// variables for doErrMonitor() / tcPhyErrMonitor()

//static u16 runt_cnt = 0;
//static u16 tlcc_cnt = 0;
//static u16 crc_cnt = 0;
//static u16 long_cnt = 0;
//static u16 loss_cnt = 0;
//static u16 col_cnt = 0;
// variables for doPhyLoopback()
static volatile u8 phy_loopback_flag = 0;
static volatile u8 recv_ok_flag = 0;
static volatile u8 recv_err_flag = 0;
#ifndef LINUX_OS
static volatile u8 timeout_flag = 0;
#endif
static volatile u8 timeout_cnt = 0;
#endif

extern void macSend(u32 chanId, struct sk_buff *skb);


/************************************************************************
*            Variables for EPHY  (4)
**************************************************************************/
//*** TCPHY registers ***//
#ifdef TCPHY_SUPPORT
// variables for doPhyConfig()

extern u8 	sw_patch_flag;
extern u8 	GECal_Rext_flag;
extern u8    phychkval_flag;
extern u8 	GECal_flag;
extern u8    slt_excuting;
extern u8    inital_finish_flag;

extern unsigned int workaround_flag_p9, workaround_flag_p10, workaround_flag_p11, workaround_flag_p12;
extern unsigned int workaround_linkdown_flag_p9, workaround_linkdown_flag_p10, workaround_linkdown_flag_p11, workaround_linkdown_flag_p12;




static u16  default_fe_l3r17 = 0x0000;   	// set value in init setting
static u16  default_ge_dev7r3c = 0x0006; 	// set value in init setting

//static u8 	EEE_DSP_Freeze_flag = 0;
//static u8 	VBuffer_1000M_flag = 0;
//static u32 	Skew_port_cnt = 0;
//static u8 	sw_ErrOverMonitor_flag = 1; 	// default enable
#ifdef PERIOD_PRINT 							// V1.17
static u8 	period_print_flag = 1; 			// default enable
static u16 	period_print_cnt = 0; 			// counter
static u16 	period_print_threshold = 240; 	// print message every 0.25s*240=60s
#endif
//static u8 	cfg_Tx10AmpSave_flag = 1; 	// default enable
//static u8 	cfg_LDPS_flag = 1; 			// default enable
static u8 	sw_FixUp2DownFast_flag = 1;
#endif 			// TCPHY_SUPPORT

#ifdef TCPHY_SUPPORT
// variables for doPhyDispFlag()

#ifdef LINUX_OS
#ifdef TCPHY_SUPPORT
//extern int    	subcmd(const cmds_t tab[], int argc, char *argv[], void *p);
//extern int    	cmd_register(cmds_t *cmds_p);
#define	RAND_MAX		32767
#endif
#endif //LINUX_OS

#endif

#ifdef TCPHY_SUPPORT
// variables for mii registers


extern tcphy_mr0_reg_t 	mr0;
extern tcphy_mr5_reg_t 	mr5;
extern tcphy_1ErA2_reg_t 	mr1E_A2;
extern tcphy_7r3D_reg_t 	mr7_3D;
// for multiple phy support
extern tcphy_mr1_reg_t 		Nmr1[TCPHY_PORTNUM];
extern tcphy_l0r25_reg_t 	Nmr25[TCPHY_PORTNUM];

mtphy_errovcnt_reg_t 	ErovcntA[MTPHY_PORTNUM];
mtphy_errovcnt_reg_t 	ErovcntB[MTPHY_PORTNUM];
mtphy_errovcnt_reg_t 	ErovcntC[MTPHY_PORTNUM];
mtphy_errovcnt_reg_t 	ErovcntD[MTPHY_PORTNUM];
// define for tc???ReadProbe()

//bool tcphy_anen = ENABLE;
//bool tcphy_speed = ENABLE;
//bool tcphy_duplex = ENABLE;

// define for tc??LinkFailDetect()
#define TbtOrHbt 		0
#define HbtOnly 		1
#define TbtOnly 		2
#define TbtOrHbtOrGbt 	3
#endif


/************************************************************************
*
*            Common Functions for EPHY  (5)
*
**************************************************************************/
extern void tcPhyChkVal(unsigned int phyAddr);
extern int phyTxAmpcomp(int show_or_comp,unsigned int phy, unsigned int TXMR_or_discrete);
//extern int phyTxAmpcomp_new(int show_or_comp,unsigned int phy, unsigned int TXMR_or_discrete);

extern int auto_select_transformer(unsigned int phy);
extern void    PreCalSet(void);

#if 1
/************************************************************************
*
*            Variables and Functions for 7523 GE PHY (ID=0x???)
*
**************************************************************************/
#ifdef LINUX_OS 
static s32 EN75xxGePhy_erroversum_A = 0;
static s32 EN75xxGePhy_erroversum_B = 0;
static s32 EN75xxGePhy_erroversum_C = 0;
static s32 EN75xxGePhy_erroversum_D = 0;

extern void 		en75xxGePhyCfgLoad(u8 idx);


#ifdef MTPHY_DEBUG
//ESD detect
static s16 en75xxGePhy_Up2Down_cnt_gbt[5] = {0,0,0,0,0};
static s16 en75xxGePhy_Up2Down_cnt_hbt[5] = {0,0,0,0,0};
static u8 en75xxGePhy_Up2DownFastHbt_detect[5] = {0,0,0,0,0};
static u8 en75xxGePhy_Up2DownFastgbt_detect[5] = {0,0,0,0,0};

static bool 	en75xxGePhy_link_fail_detect_flag = 0;
//static u8 	en75xxGePhy_reset_needed_perport[5] = {0,0,0,0,0};
//static u8 	en75xxGePhy_reset_needed_flag = 0;
//static u8 	en75xxGePhy_esdphy_init_flag = 1; 		// default: enable

//Disable EEE if link fail
static s16 	en75xxGePhy_Up2Down_cnt_tbt[5] = {0,0,0,0,0};
static u8 	en75xxGePhy_Up2DownFastTbt_detect[5] = {0,0,0,0,0};
static u8 	en75xxGePhy_linkup_check_timer[5] = {0,0,0,0,0};
static bool 	en75xxGePhyDisEEEHappened[5] = {0,0,0,0,0};
static u8 	en75xxGePhy_linkup_check_cnt[5] = {0,0,0,0,0};
static u8 	en75xxGePhyDisNextPage_cnt[5] = {0,0,0,0,0};
static bool 	en75xxGePhyDisNextPage_flag[5] = {0,0,0,0,0};
static u8 	en75xxGePhyDisSlaveMode_cnt[5] = {0,0,0,0,0};  
static bool 	en75xxGePhyDisSlaveMode_flag[5] = {0,0,0,0,0};
s32 			en75xxGePhyReadProbe(u8 port_num, u8 mode, u8 ch_num);
s32 			en75xxGePhyReadCoeff(u8 port_num, u8 mode, u8 ch_num);
void 			en75xxGePhyDbgPorbeSet(u8 prb_num, u8  port_num, u16 flg_num, u16 clk_num );
s32 			en75xxGePhyDbgPorbeRead(u8 prb_num,u8  port_num);
#endif

#ifdef MTPHY_DEBUG 
s32 	en75xxGePhyReadErrOverSum(u8 port_num);
s32 	en75xxGePhyReadAdcSum(u8 port_num);
void 	en75xxGePhyDispProbe100(u8 port_num, bool long_msg);
void 	en75xxGePhySwDispMessage(u8 port_num);
u16 	en75xxGePhyReadSnrSum(u8 port_num, u16 cnt);
void 	en75xxGePhyUpdateErrOverSum(u8 port_num);
void 	en75xxGePhySwErrOverMonitor(u8 port_num);
void 	en75xxGePhyUp2DownFastgbt(u8 port_num);
void 	en75xxGePhyUp2DownFastHbt(u8 port_num);
void 	en75xxGePhyUp2DownFastTbt(u8 port_num);
bool 	en75xxGePhyLinkFailDetect (u8 port_num, u8 mode);		//0:10BT or 100BT, 1:100BT only, 2:10BT only,3:10BT or 100BT or 1000BT
void 	en75xxGePhyDisEEENeeded(u8 port_num);
void 	en75xxGePhyDisNextPageNeeded(u8 port_num);
#endif //TCPHY_DEBUG
		 
void 	doEphySetSLTMode(u8 phyaddr, u16 speed_mode); 			 
void 	doEphySetSETMode(u8 set_mode, u16 parameter);   // allen_20190709


#ifdef MTPHY_DEBUG
void en75xxGePhyDbgPorbeSet(u8 prb_num, u8  port_num, u16 flg_num, u16 clk_num)
{
	u16 	value=0;
	#define dbg_flg_en 1 
	#define dbg_DevAddr 0x1F
	#define dbg_Flg0Ctrl 0x15
	#define dbg_Flg1Ctrl 0x16
	#define dbg_ClkAddr 0x19
 
	if(prb_num == 2)
	{
		value = (port_num<<13)|(dbg_flg_en<<12)|(flg_num);
		TCPHYDISP1("Dbg Flag Register Set Value=%x\r\n", value);
		mtEMiiRegWrite(port_num,dbg_DevAddr, dbg_Flg0Ctrl, value);
		mtEMiiRegWrite(port_num,dbg_DevAddr, dbg_Flg1Ctrl, value);
	}
	else if(prb_num == 0)
	{
		value = (port_num<<13)|(dbg_flg_en<<12)|(flg_num);
		TCPHYDISP1("Dbg Flag Register Set Value=%x\r\n", value);
		mtEMiiRegWrite(port_num, dbg_DevAddr, dbg_Flg0Ctrl, value);
	}
	else if(prb_num == 1)
	{
		value = (port_num<<13)|(dbg_flg_en<<12)|(flg_num);
		TCPHYDISP1("Dbg Flag Register Set Value=%x\r\n", value);
		mtEMiiRegWrite(port_num, dbg_DevAddr, dbg_Flg1Ctrl, value);
	}

    if(clk_num == 0xff)
    {	//disable clk output
		value = mtEMiiRegRead( port_num, dbg_DevAddr, dbg_ClkAddr);
		value = value & 0xEF;
		mtEMiiRegWrite(port_num, dbg_DevAddr, dbg_ClkAddr, value);
    }
	else
	{	//enable clk output
		value = (dbg_flg_en<<12)|(clk_num);
		mtEMiiRegWrite(port_num, dbg_DevAddr, dbg_ClkAddr, value);
	}	
}

s32 en75xxGePhyDbgPorbeRead(u8 prb_num,u8  port_num)
{
	u16 	value=0;
	#define dbg_flg_en 1 
	#define dbg_DevAddr 0x1F
	#define dbg_Flg0Ctrl 0x15
	#define dbg_Flg1Ctrl 0x16
	#define dbg_ClkAddr 0x19
	#define dbg_monAddr 0x1A

	value = mtEMiiRegRead( port_num, dbg_DevAddr, dbg_monAddr);

	if(prb_num == 0)
		value = (value & 0x00FF);
	else if(prb_num == 1)
		value = (value>>8) & 0x00FF;
             	
	TCPHYDISP1("Dbg Flag[%d] Monitor Register Value=%x\r\n", prb_num,value);
	return(value);	       	
}

s32 en75xxGePhyReadProbe(u8 port_num, u8 mode, u8 ch_num)
{
    u32 val=0,val_A=0,val_B=0,val_C=0,val_D=0;
    u32 avg_val=0, avg_A=0,avg_B=0,avg_C=0,avg_D=0;
    u32 phyaddr = port_num;
    u16 i;
    #define avg_cnt 1000
    
    #if debug_flag
    tcMiiStationWrite(port_num, 31, 0);
    switch(mode)
    {
    	case Probe_VgaState:
    		val = mtPhyMiiRead_TrDbg(phyaddr,"PMA",0x38,0);
    		TCPHYDISP3("VgaStateA =0x%x\r\n",((val>>4) & 0x1F));
    		TCPHYDISP3("VgaStateB =0x%x\r\n",((val>>9) & 0x1F));
    		TCPHYDISP3("VgaStateC =0x%x\r\n",((val>>14) & 0x1F));
    		TCPHYDISP3("VgaStateD =0x%x\r\n",((val>>19) & 0x1F));
    		break;
    		
        case Probe_MSE:
          	switch(ch_num)
          	{
	        	case 0:		//Pair A
              		val = mtEMiiRegRead(phyaddr,0x1E,0x9B);
			  		TCPHYDISP1("XX0 0x1E,0x9B =0x%x\r\n",val);
              		val = (val>>8) & 0xFF;  	//V1.16
              		TCPHYDISP1("AA0 lch_mse_mdcA =0x%x\r\n",val);
              		break;
            	case 1:		//Pair B
              		val = mtEMiiRegRead(phyaddr,0x1E,0x9B);
			  		TCPHYDISP1("XX1 0x1E,0x9B =0x%x\r\n",val);
              		val = (val) & 0xFF;  	//V1.16
              		TCPHYDISP1("AA1 lch_mse_mdcB =0x%x\r\n",val);
              		break;
            	case 2:		//Pair C
              		val = mtEMiiRegRead(phyaddr,0x1E,0x9C);
			  		TCPHYDISP1("XX2 0x1E,0x9C =0x%x\r\n",val);
              		val = (val>>8) & 0xFF;  	//V1.16
              		TCPHYDISP1("AA2 lch_mse_mdcC =0x%x\r\n",val);
              		break;
            	case 3:		//Pair D
              		val = mtEMiiRegRead(phyaddr,0x1E,0x9C);
			  		TCPHYDISP1("XX3 0x1E,0x9C =0x%x\r\n",val);
              		val = (val) & 0xFF;  	//V1.16
              		TCPHYDISP1("AA3 lch_mse_mdcD =0x%x\r\n",val);
              		break;
            }
		    break;
		    
        case Probe_MSE_ALL:
			val=mtEMiiRegRead(phyaddr,0x1E,0x9B);
			val_A =(val>>8) & 0xFF;  //V1.16
			val_B =(val) & 0xFF;  //V1.16
			TCPHYDISP1("ALL 0x1E,0x9B =0x%x\r\n",val);		

			val=mtEMiiRegRead(phyaddr,0x1E,0x9C);

			val_C =(val>>8) & 0xFF;  //V1.16
			val_D =(val) & 0xFF;  //V1.16
			TCPHYDISP1("ALL 0x1E,0x9C =0x%x\r\n",val);		
			TCPHYDISP1("BB1 lch_mse_mdcA =0x%x\r\n",val_A);
			TCPHYDISP1("BB2 lch_mse_mdcB =0x%x\r\n",val_B);
			TCPHYDISP1("BB3 lch_mse_mdcC =0x%x\r\n",val_C);
			TCPHYDISP1("BB4 lch_mse_mdcD =0x%x\r\n",val_D);
			val=val_A+val_B+val_C+val_D;
            break;
            
        case Probe_MSE_Slicer_err_thres:
			val=mtEMiiRegRead(phyaddr,0x1E,0x9D);
			val=(val>>11) & 0x1F; 
			TCPHYDISP3("slicer_err_thres =0x%x\r\n",val);
			break;
			
        case Probe_MSE_Sum:
			for(i=0; i<avg_cnt; i++)
			{
				val = mtEMiiRegRead(phyaddr, 0x1E, 0x9B);
				val_A =(val>>8) & 0xFF;  //V1.16
				val_B =(val) & 0xFF;  //V1.16
				avg_A = avg_A + val_A;
				avg_B = avg_B + val_B;
            
				val = mtEMiiRegRead(phyaddr, 0x1E, 0x9C);
				val_C =(val>>8) & 0xFF;  //V1.16
				val_D =(val) & 0xFF;  //V1.16
				avg_C = avg_C + val_C;
				avg_D = avg_D + val_D;
             
				val = (mtEMiiRegRead(phyaddr, 0x1E, 0x9D)>>11)&0x1F;

				avg_val = avg_val + val;
			}
	        avg_A = avg_A/1000;
    	    avg_B = avg_B/1000;
          	avg_C = avg_C/1000;
          	avg_D = avg_D/1000;
          	avg_val = avg_val/1000;
          
          	val = avg_A + avg_B + avg_C + avg_D;
          
          	TCPHYDISP3("lch_mse_mdcA avg1000 =0x%x\r\n", avg_A);
          	TCPHYDISP3("lch_mse_mdcB avg1000 =0x%x\r\n", avg_B);
          	TCPHYDISP3("lch_mse_mdcC avg1000 =0x%x\r\n", avg_C);
          	TCPHYDISP3("lch_mse_mdcD avg1000 =0x%x\r\n", avg_D);
          	TCPHYDISP3("slicer_err_thres avg1000 =0x%x\r\n", avg_val);
          	break;
          	
        case Probe_Slicer_Err_Over_Sum:      
	    	switch(ch_num)
	        {
              	case 0:
              		val = mtEMiiRegRead(phyaddr, 0x1E, 0x9E);
              		ErovcntA[port_num].err_over_cnt = val;
              		TCPHYDISP3("ProbeSlicerErrOverSumA =0x%x\r\n", val);
              		break;
              		
            	case 1:
              		val = mtEMiiRegRead(phyaddr, 0x1E, 0x9F);
              		ErovcntB[port_num].err_over_cnt = val;
              		TCPHYDISP3("ProbeSlicerErrOverSumB =0x%x\r\n", val);
              		break;
              		
	            case 2:
              		val = mtEMiiRegRead(phyaddr, 0x1E, 0xA0);
              		ErovcntC[port_num].err_over_cnt = val;
              		TCPHYDISP3("ProbeSlicerErrOverSumC =0x%x\r\n", val);
              		break;
              		
            	case 3:
              		val = mtEMiiRegRead(phyaddr, 0x1E, 0xA1);
              		ErovcntD[port_num].err_over_cnt = val;
              		TCPHYDISP3("ProbeSlicerErrOverSumD =0x%x\r\n", val);
              		break;
        	}
          	break;
          		
        case Probe_Slicer_Err_Over_Sum_ALL:
              val_A = mtEMiiRegRead(phyaddr, 0x1E, 0x9E);
              TCPHYDISP3("ProbeSlicerErrOverSumA =0x%x\r\n", val_A);
              val_B = mtEMiiRegRead(phyaddr, 0x1E, 0x9F);
              TCPHYDISP3("ProbeSlicerErrOverSumB =0x%x\r\n", val_B);
              val_C = mtEMiiRegRead(phyaddr, 0x1E, 0xA0);
              TCPHYDISP3("ProbeSlicerErrOverSumC =0x%x\r\n", val_C);
              val_D = mtEMiiRegRead(phyaddr, 0x1E, 0xA1);
              TCPHYDISP3("ProbeSlicerErrOverSumD =0x%x\r\n", val_D);
              val = val_A + val_B + val_C + val_D;
              break;
  
        default:
            TCPHYDISP1("\r\ntcphy error: ReadProbe %d.\r\n",mode);
            break;
    }
    #endif
    return val;
}

void en75xxGePhySwDispMessage(u8 port_num)
{
	#ifdef PERIOD_PRINT 	//V1.17
    u32 value;
	#endif

    switch(mtphy_link_state[port_num]) 
    {
        case ST_LINK_DOWN: 
      		// printk("Link Satus is ST_LINK_DOWN!\r\n");
            break;
            
        case ST_LINK_DOWN2UP: 
      		//  printk("Link Satus is ST_LINK_DOWN2UP!\r\n");
			if(mr1E_A2.final_speed_1000)
			{
				TCPHYDISP1("mtphy[%d]:Link-up at 1000 %s.\r\n", port_num, (mr1E_A2.FullDuplexEnable ?"F":"H"));
			}
			else if (mr1E_A2.final_speed_100)
			{
				TCPHYDISP1("mtphy[%d]:Link-up at 100%s.\r\n", port_num, (mr1E_A2.FullDuplexEnable ?"F":"H"));
			}
			else if (mr1E_A2.final_speed_10)
			{
				TCPHYDISP1("mtphy[%d]:Link-up at 10%s.\r\n",port_num, (mr1E_A2.FullDuplexEnable ?"F":"H"));
			}
	
			if(mr1E_A2.final_speed_1000 ||mr1E_A2.final_speed_100 ||mr1E_A2.final_speed_10)
			{
				en75xxGePhyReadProbe(port_num, Probe_VgaState, 0);
				en75xxGePhyReadProbe(port_num, Probe_MSE_ALL, 0);
				en75xxGePhyReadProbe(port_num, Probe_MSE_Slicer_err_thres, 0);
				en75xxGePhyReadProbe(port_num, Probe_Slicer_Err_Over_Sum_ALL, 0);
			}
            break;
            
        case ST_LINK_UP: 
         	// printk("Link Satus is ST_LINK_UP!\r\n");
          	#ifdef PERIOD_PRINT //V1.17
            if (period_print_flag == 1)
            {
            	if (period_print_cnt<period_print_threshold)
              	{
                	period_print_cnt++;
              	}
			  	else
			  	{
                	period_print_cnt=0;
					value = mtPhyMiiRead_TrDbg(port_num, "PMA", 0x38 ,0);
					printk("port_num=0x%X,Value of PMA 0x38=0x%X\r\n",port_num,value);
					printk("VgaStateA=0x%X,VgaStateB=0x%X,VgaStateC=0x%X,VgaStateD=0x%X\r\n",(value&0x1F0)>>4,(value&0x3E00)>>9,(value&0x7C000)>>14,(value&0xF80000)>>19);
			  	}
            }
          	#endif
            break;
            
        case ST_LINK_UP2DOWN:
          	#ifdef PERIOD_PRINT //V1.17			
			period_print_cnt=0;
          	#endif
            TCPHYDISP1("mtcphy[%d]: Link-down!!!\r\n",port_num);
            break;
            
        default: printk("\r\nmtphy error: SwDispMessage error!\r\n");
    }
}

void en75xxGePhyUpdateErrOverSum(u8 port_num)
{
    u16 	err_over_cnt_prev_A; 			// to store previous err_over_cnt value
    u16 	err_over_cnt_prev_B; 			// to store previous err_over_cnt value
    u16 	err_over_cnt_prev_C; 			// to store previous err_over_cnt value
    u16 	err_over_cnt_prev_D; 			// to store previous err_over_cnt value
    s32 	val_A, val_B, val_C, val_D;
    
    // clear Nmr25[port_num].err_over_cnt in tc2104meDispProbe100() in ST_LINK_DOWN2UP
    if (((mr1E_A2.final_speed_1000== 1) ||(mr1E_A2.final_speed_100 == 1)) 	// 1000/100BaseTX
        && (mtphy_link_state[port_num] == ST_LINK_UP)) 
    {
    	err_over_cnt_prev_A = ErovcntA[port_num].err_over_cnt;
        err_over_cnt_prev_B = ErovcntB[port_num].err_over_cnt;
        err_over_cnt_prev_C = ErovcntC[port_num].err_over_cnt;
        err_over_cnt_prev_D = ErovcntD[port_num].err_over_cnt;
        
      	// en75xxGePhyReadProbe(port_num,Probe_MSE_Sum,0);
       	en75xxGePhyReadProbe(port_num,Probe_Slicer_Err_Over_Sum,0);		//Pair A
       	en75xxGePhyReadProbe(port_num,Probe_Slicer_Err_Over_Sum,1);		//Pair B
       	en75xxGePhyReadProbe(port_num,Probe_Slicer_Err_Over_Sum,2);		//Pair C
       	en75xxGePhyReadProbe(port_num,Probe_Slicer_Err_Over_Sum,3);		//Pair D
        val_A = ErovcntA[port_num].err_over_cnt - err_over_cnt_prev_A;
        val_B = ErovcntB[port_num].err_over_cnt - err_over_cnt_prev_B;
        val_C = ErovcntC[port_num].err_over_cnt - err_over_cnt_prev_C;
        val_D = ErovcntD[port_num].err_over_cnt - err_over_cnt_prev_D;
        #if debug_flag
        TCPHYDISP4("ProbeSlicerErrOverSumA =0x%x\r\n",val_A);
        TCPHYDISP4("ProbeSlicerErrOverSumB =0x%x\r\n",val_B);
        TCPHYDISP4("ProbeSlicerErrOverSumC =0x%x\r\n",val_C);
        TCPHYDISP4("ProbeSlicerErrOverSumD =0x%x\r\n",val_D);
        #endif
        
        if( val_A < 0 )
        {
        	val_A += 2048;
        }
        if( val_B < 0 )
        {
            val_B += 2048;
        }
        if( val_C < 0 )
        {
            val_C += 2048;
        }
        if( val_D < 0 )
        {
            val_D += 2048;
        }
        EN75xxGePhy_erroversum_A = val_A;
        EN75xxGePhy_erroversum_B = val_B;
        EN75xxGePhy_erroversum_C = val_C;
        EN75xxGePhy_erroversum_D = val_D;
    }
    else 
    {
        EN75xxGePhy_erroversum_A = -1;
        EN75xxGePhy_erroversum_B = -1;
        EN75xxGePhy_erroversum_C = -1;
        EN75xxGePhy_erroversum_D = -1;
    }
}

void en75xxGePhySwErrOverMonitor(u8 port_num)
{
    if (tcPhy_disp_level < 3) 
        return; 			// inactive
	#if debug_flag
    if (EN75xxGePhy_erroversum_A>0)
    {
        TCPHYDISP3("tcphy[%d]: ErrOver A=%d\r\n",port_num,EN75xxGePhy_erroversum_A);          
    }
    if (EN75xxGePhy_erroversum_B>0)
    {
        TCPHYDISP3("tcphy[%d]: ErrOver B=%d\r\n",port_num,EN75xxGePhy_erroversum_B);          
    }
    if (EN75xxGePhy_erroversum_C>0)
    {
        TCPHYDISP3("tcphy[%d]: ErrOver C =%d\r\n",port_num,EN75xxGePhy_erroversum_C);          
    }
    if (EN75xxGePhy_erroversum_D>0)
    {
        TCPHYDISP3("tcphy[%d]: ErrOver D=%d\r\n",port_num,EN75xxGePhy_erroversum_D);          
    }
    #endif
}

void en75xxGePhyUp2DownFastgbt(u8 port_num)
{
	const u8 en75xxGePhy_Up2Down_gbt_Thd = 10;
	const u8 en75xxGePhy_Up2Down_gbt_ub = 40;

	if(mr1E_A2.final_speed_1000 & !Nmr1[port_num].link_status) 
	{
		if(en75xxGePhy_Up2Down_cnt_gbt[port_num]<en75xxGePhy_Up2Down_gbt_ub) 
		{
			en75xxGePhy_Up2Down_cnt_gbt[port_num]+=3;
		}
    }
	else if(!mr1E_A2.final_speed_1000 & !Nmr1[port_num].link_status)
	{
		if(en75xxGePhy_Up2Down_cnt_gbt[port_num]>0) 
		{
			en75xxGePhy_Up2Down_cnt_gbt[port_num]--;
		}
	}
	else if(mr1E_A2.final_speed_1000 & Nmr1[port_num].link_status) 
	{	//adjust link-up time to clear counter
		en75xxGePhy_Up2Down_cnt_gbt[port_num]=0;
	}
	
	if(en75xxGePhy_Up2Down_cnt_gbt[port_num] > en75xxGePhy_Up2Down_gbt_Thd) 
	{
	    en75xxGePhy_Up2Down_cnt_gbt[port_num]=0;
		en75xxGePhy_Up2DownFastgbt_detect[port_num]=1;
		#ifdef TCPHY_DEBUG
		TCPHYDISP4("set 7523 GePhy_Up2DownFastgbt_detect[%d] to 1.\r\n",port_num); 
		#endif
	}
	else 
	{
		en75xxGePhy_Up2DownFastgbt_detect[port_num]=0;
	}  
}

void en75xxGePhyUp2DownFastHbt(u8 port_num)
{
	const u8 en75xxGePhy_Up2Down_hbt_Thd = 10;
	const u8 en75xxGePhy_Up2Down_hbt_ub = 40;

	if(mr1E_A2.final_speed_100 & !Nmr1[port_num].link_status) 
	{
		if(en75xxGePhy_Up2Down_cnt_hbt[port_num] < en75xxGePhy_Up2Down_hbt_ub) 
		{
			en75xxGePhy_Up2Down_cnt_hbt[port_num] += 3;
		}
    }
	else if(!mr1E_A2.final_speed_100 & !Nmr1[port_num].link_status) 
	{
		if(en75xxGePhy_Up2Down_cnt_hbt[port_num]>0) 
		{
			en75xxGePhy_Up2Down_cnt_hbt[port_num]--;
		}
	}
	else if(mr1E_A2.final_speed_100 & Nmr1[port_num].link_status) 
	{	//adjust link-up time to clear counter
		en75xxGePhy_Up2Down_cnt_hbt[port_num]=0;
	}
	
	if(en75xxGePhy_Up2Down_cnt_hbt[port_num]> en75xxGePhy_Up2Down_hbt_Thd) 
	{
	    en75xxGePhy_Up2Down_cnt_hbt[port_num]=0;
		en75xxGePhy_Up2DownFastHbt_detect[port_num]=1;
		#ifdef TCPHY_DEBUG
		TCPHYDISP4("set 7523 GePhy_Up2DownFastHbt_detect[%d] to 1.\r\n",port_num); 
		#endif
	}
	else 
	{
		en75xxGePhy_Up2DownFastHbt_detect[port_num]=0;
	}  
}

void en75xxGePhyUp2DownFastTbt(u8 port_num)
{
	const u8 en75xxGePhy_Up2Down_Tbt_Thd = 10;
	const u8 en75xxGePhy_Up2Down_Tbt_ub = 40;
	const u8 linkup_check_timer_done = 1;
	
    if(mr1E_A2.final_speed_10 & !Nmr1[port_num].link_status) 
	{
		en75xxGePhy_linkup_check_timer[port_num]=0;
		if(en75xxGePhy_Up2Down_cnt_tbt[port_num] < en75xxGePhy_Up2Down_Tbt_ub) 
		{
			en75xxGePhy_Up2Down_cnt_tbt[port_num]+=5;
		}
    }
	else if(!mr1E_A2.final_speed_10 & !Nmr1[port_num].link_status) 
	{
		en75xxGePhy_linkup_check_timer[port_num]=0;
		if(en75xxGePhy_Up2Down_cnt_tbt[port_num]>0) 
		{
			en75xxGePhy_Up2Down_cnt_tbt[port_num]--;
		}
	}
	else if(mr1E_A2.final_speed_10 & Nmr1[port_num].link_status ) 
	{
		if((en75xxGePhy_linkup_check_timer[port_num] == linkup_check_timer_done)) 
		{
			en75xxGePhy_Up2Down_cnt_tbt[port_num]=0;
			en75xxGePhy_linkup_check_timer[port_num]=0;
		}
		else if((en75xxGePhy_linkup_check_timer[port_num] < linkup_check_timer_done))
		{
			en75xxGePhy_linkup_check_timer[port_num]++;
		}
	}
	
	if(en75xxGePhy_Up2Down_cnt_tbt[port_num]> en75xxGePhy_Up2Down_Tbt_Thd) 
	{
	    en75xxGePhy_Up2Down_cnt_tbt[port_num]=0; 
		en75xxGePhy_Up2DownFastTbt_detect[port_num]=1;
		#ifdef TCPHY_DEBUG
		TCPHYDISP4("set 7523 GePhy_Up2DownFastTbt_detect[%d] to 1.\r\n",port_num); 
		#endif
	}
	else 
	{
		en75xxGePhy_Up2DownFastTbt_detect[port_num]=0;
	}	
}

bool en75xxGePhyLinkFailDetect(u8 port_num, u8 mode)
{
	switch(mode)
	{
		case TbtOrHbtOrGbt:
			if(sw_FixUp2DownFast_flag 
				&(en75xxGePhy_Up2DownFastTbt_detect[port_num]
				||en75xxGePhy_Up2DownFastHbt_detect[port_num]||en75xxGePhy_Up2DownFastgbt_detect[port_num])) 
			{
				en75xxGePhy_link_fail_detect_flag =1;
		        #ifdef TCPHY_DEBUG
		        TCPHYDISP3(" 7523 GePhy_link_fail_detect_flag(%d)=%d\r\n", port_num, en75xxGePhy_link_fail_detect_flag); 
		        #endif
			}
			else 
			{
				en75xxGePhy_link_fail_detect_flag =0;
			}	
			break;

		case HbtOnly:
			if(sw_FixUp2DownFast_flag & en75xxGePhy_Up2DownFastHbt_detect[port_num]) 
			{
				en75xxGePhy_link_fail_detect_flag =1;
		        #ifdef TCPHY_DEBUG
		        TCPHYDISP3("Hbt: 7523 GePhy_link_fail_detect_flag(%d)=%d\r\n", port_num, en75xxGePhy_link_fail_detect_flag); 
		        #endif
			}
			else 
			{
				en75xxGePhy_link_fail_detect_flag =0;
			}
			break;

		case TbtOnly:
			if(sw_FixUp2DownFast_flag & en75xxGePhy_Up2DownFastTbt_detect[port_num]) 
			{
				en75xxGePhy_link_fail_detect_flag =1;
		       	#ifdef TCPHY_DEBUG
		        TCPHYDISP3("Tbt: 7523 GePhy_link_fail_detect_flag(%d)=%d\r\n", port_num, en75xxGePhy_link_fail_detect_flag); 
		        #endif
			}
			else 
			{
				en75xxGePhy_link_fail_detect_flag =0;
			}
			break;

		default: 
			en75xxGePhy_link_fail_detect_flag =0;
	}
	return en75xxGePhy_link_fail_detect_flag;
}

void en75xxGePhyDisNextPageNeeded(u8 port_num)
{
	const u8 en75xxGePhyDisNextPage_cntdone = 5;
	const u8 en75xxGePhyDisSlaveMode_cntdone = 3; 
   
   	if((mtphy_link_state[port_num] == ST_LINK_UP)||(en75xxGePhyDisEEEHappened[port_num])) 
 	{
   		en75xxGePhyDisNextPage_cnt[port_num] = 0;
   	}
   	else if((mr5.LPNextAble) && !(mr7_3D.lp_eee_100||mr7_3D.lp_eee_1000||mr7_3D.lp_eee_10g)
   		&& (en75xxGePhyDisNextPage_cnt[port_num] <= en75xxGePhyDisNextPage_cntdone)) 
   	{
   		en75xxGePhyDisNextPage_cnt[port_num]++;
   	}

   	if(en75xxGePhyDisNextPage_cnt[port_num] > en75xxGePhyDisNextPage_cntdone) 
	{
   		en75xxGePhyDisNextPage_flag[port_num] = 1;
		en75xxGePhyDisNextPage_cnt[port_num] = 0;
   	}
   	else 
	{
   		en75xxGePhyDisNextPage_flag[port_num]=0;
   	}

	// === for gphy eee iot with realtek : default slave mode
	if((mtphy_link_state[port_num] != ST_LINK_UP) && (mtphy_link_state[port_num] != ST_LINK_DOWN) &&
		(en75xxGePhyDisSlaveMode_cnt[port_num] <= en75xxGePhyDisSlaveMode_cntdone))
	{
		en75xxGePhyDisSlaveMode_cnt[port_num]++;
	}

	if(en75xxGePhyDisSlaveMode_cnt[port_num] > en75xxGePhyDisSlaveMode_cntdone)
	{
		en75xxGePhyDisSlaveMode_flag[port_num] = 1;
		en75xxGePhyDisSlaveMode_cnt[port_num] = 0;
	}
	else
	{
		en75xxGePhyDisSlaveMode_flag[port_num] = 0;
	}
}

void en75xxGePhyDisEEENeeded(u8 port_num)
{
    const u8 en75xxGePhy_linkup_check_done = 1;
	u8 phy_base = 8, phyaddr;
	
	phyaddr = phy_base + port_num;
	
    if((mtphy_link_state[port_num] == ST_LINK_UP)
		&& (en75xxGePhy_linkup_check_cnt[port_num] < en75xxGePhy_linkup_check_done)) 
	{
		en75xxGePhy_linkup_check_cnt[port_num]++;
    }
	else if(mtphy_link_state[port_num] != ST_LINK_UP)
	{
		en75xxGePhy_linkup_check_cnt[port_num] = 0;
    }
	
    if(en75xxGePhyDisEEEHappened[port_num] && en75xxGePhy_linkup_check_cnt[port_num]==en75xxGePhy_linkup_check_done) 
	{	//extend link-up time
		mtEMiiRegWrite(phyaddr, 0x07, 0x3c, 0x06);
		en75xxGePhyDisEEEHappened[port_num]=0;
    }
	else if(en75xxGePhyLinkFailDetect(port_num,3) || en75xxGePhyDisNextPage_flag[port_num]) 
	{
		mtEMiiRegWrite(phyaddr, 0x07, 0x3c, 0x0000);	// disable next page
		//tcMiiStationWrite(0,0x00,0x1200);				// re-start AN
		en75xxGePhyDisEEEHappened[port_num]=1;
    }
	//TCPHYDISP3("7523 GePhy_linkup_check_cnt[%d]=%d\r\n", port_num, en75xxGePhy_linkup_check_cnt[port_num]);
	//TCPHYDISP3("7523 GePhyDisEEEHappened[%d]=%d\r\n", port_num, en75xxGePhyDisEEEHappened[port_num]);
}

#endif


#endif  
#endif



/************************************************************************
*                       API for EPHY (TCPHY_SUPPORT)
**************************************************************************/
// function declaration for TCEPHYDBG commands
#ifdef TCPHY_SUPPORT
int doPhyMiiRead (int argc, char *argv[], void *p);
int doPhyMiiWrite(int argc, char *argv[], void *p);

int doPhyGphyTestMode (int argc, char *argv[], void *p);
int extPhyGphyTestMode (int argc, char *argv[], void *p);
int settingANSpeed (int argc, char *argv[], void *p);
int settingloopback (int argc, char *argv[], void *p);
int settingFlowControl (int argc, char *argv[], void *p);
int getAllPortStatus (int argc, char *argv[], void *p);

#ifdef TCPHY_1PORT
static int doPhySpeed (int argc, char *argv[], void *p);
#endif
#ifdef TCPHY_DEBUG

//static int doPhyRegCheck (int argc, char *argv[], void *p);
#ifndef LINUX_OS
static int doPhyLoopback (int argc, char *argv[], void *p);
#endif


int doPhyChkVal (int argc, char *argv[], void *p);
#ifdef TC2031_DEBUG
static int doPhyErrOver (int argc, char *argv[], void *p);
#endif
#ifdef LINUX_OS // allen_20130926 : merge 7502 & 7510/20
#ifdef MTPHY_DEBUG

int doDbgPorbeSet(int argc, char *argv[], void *p);
int doPhyMiiRead_TrDbg(int argc, char *argv[], void *p);
int doPhyMiiWrite_TrDbg(int argc, char *argv[], void *p);
int buck_pbus_read_command(int argc, char *argv[], void *p);
int buck_pbus_write_command(int argc, char *argv[], void *p);
int doPhyChkVal (int argc, char *argv[], void *p);
int doPhyChkErrFlag (int argc, char *argv[], void *p);
int doPhyClrErrFlag (int argc, char *argv[], void *p);
int doPhyMMDRead(int argc, char *argv[], void *p);
int doPhyMMDWrite(int argc, char *argv[], void *p);



int doPhySwPatch (int argc, char *argv[], void *p);

int doPhyForceMode (int argc, char *argv[], void *p);
int doPhyBootloaderMode (int argc, char *argv[], void *p);
int doPhyForceEEE (int argc, char *argv[], void *p);
int doPhyForceGE (int argc, char *argv[], void *p);
int doPhyForceReset (int argc, char *argv[], void *p);
int doPhyrestartAN (int argc, char *argv[], void *p);
int doPhygetLinkRate (int argc, char *argv[], void *p);
int doPhygetDuplex (int argc, char *argv[], void *p);
int doPhygetAN (int argc, char *argv[], void *p);
int doPhyforcePWD (int argc, char *argv[], void *p);
int doPhygetLinkStatus (int argc, char *argv[], void *p);
int doPhyforcePause (int argc, char *argv[], void *p);
int doPhygetLoopback (int argc, char *argv[], void *p);
int doPhyforceLoopback (int argc, char *argv[], void *p);
int doPhygetLPCap (int argc, char *argv[], void *p);
int doPhygetCALReg (int argc, char *argv[], void *p);
int doPhyForceLED (int argc, char *argv[], void *p);
int recalibration(int argc, char *argv[], void *p);
int doPbusRead (int argc, char *argv[], void *p);
int doPbusWrite (int argc, char *argv[], void *p);

extern int doPhySwVer(int argc, char *argv[], void *p);
extern int sw_ability_setting(int argc, char *argv[], void *p);
extern int doReadAllCalData(int argc, char *argv[], void *p);

//static int doTrgmiiRxCal (void);

#endif
#endif
#endif // TCPHY_DEBUG
static const cmds_t ethertphyprint[] =
{
    {"ver",       		doPhySwVer,           	0x02, 0, NULL},
    {"miir",      		doPhyMiiRead,         	0x02, 0, NULL},
    {"miiw",      		doPhyMiiWrite,        	0x02, 0, NULL},
   	{"testmode",  		doPhyGphyTestMode,		0x02, 0, "tce testmode <speed> <phyaddr> <mode or channel>"},
	{"exttestmode",  		extPhyGphyTestMode,		0x02, 0, "tce exttestmode <speed> <phyaddr> <mode or channel>"},
	{"cal_all_data",  	doReadAllCalData,		0x02, 0, NULL},
	{"sw_ability",  sw_ability_setting,		0x02, 1, "sw_ability_set <SDS|FS> <on|ff>"},
	{"recal",  	recalibration,		0x02, 0, "recal <phyaddr> "},
	{"bpr",  	buck_pbus_read_command,		0x02, 0, "bpr <phyaddr> <register>"},
	{"bpw",  	buck_pbus_write_command,		0x02, 0, "bpw <phyaddr> <register> <value>"},
	#ifdef TCPHY_DEBUG
    {"swpatch",   		doPhySwPatch,       	0x02, 0, NULL},
	#ifdef LINUX_OS // allen_20130926 : merge 7502 & 7510/20
	#ifdef MTPHY_DEBUG 
	{"chkval",  		doPhyChkVal,  			0x02, 1, "chkval <phyaddr>"},  
	{"err_flag",        doPhyChkErrFlag,        0x02, 0, NULL},
	{"clr_err_flag",    doPhyClrErrFlag,        0x02, 0, NULL},
	{"emiir",			doPhyMMDRead,		  	0x02, 0, NULL},
	{"emiiw",			doPhyMMDWrite,		  	0x02, 0, NULL},
	{"dbgprb",	  		doDbgPorbeSet,			0x02, 0, NULL},
	{"miir_trdbg",		doPhyMiiRead_TrDbg,		0x02, 0, NULL},
	{"miiw_trdbg",  	doPhyMiiWrite_TrDbg,	0x02, 0, NULL},
	{"pbr",				doPbusRead,		  	    0x02, 0, NULL},
	{"pbw",				doPbusWrite,		  	    0x02, 0, NULL},
	{"setANSpeed", 		settingANSpeed,			0x02, 1, "setANSpeed <phyaddr> <speed> <dupex>"},	
	{"setLoopback", 	settingloopback,	    0x02, 1, "setLoopback <phyaddr> <mode>"},	
	{"flowControl", 	settingFlowControl,	    0x02, 1, "flowControl <phyaddr> <on|off>"},	
	{"PortStatus", 		getAllPortStatus,	    0x02, 0, "PortStatus"},
	{"forcemode",  		doPhyForceMode,  		0x02, 1, "forcemode <phyaddr> <an|force> <10|100|1000> <full|half>"}, 
	{"bootloadermode",  doPhyBootloaderMode,  	0x02, 1, "bootloadrmode <phyaddr> <on|off>"}, 
	{"forceEEE",  		doPhyForceEEE,  		0x02, 1, "forceEEE <phyaddr> <on|off|recover>"},
	{"forceGE",  		doPhyForceGE,  			0x02, 1, "forceGE <phyaddr> <multi|single> <master|slave>"},
	{"forceReset", 		doPhyForceReset,		0x02, 1, "forceReset <phyaddr> "},
	{"restartAN", 		doPhyrestartAN,			0x02, 1, "restartAN <phyaddr> "}, 
	{"getLinkRate", 	doPhygetLinkRate,		0x02, 1, "getLinkRate <phyaddr> "}, 
	{"getDuplex", 		doPhygetDuplex,			0x02, 1, "getDuplex <phyaddr> "},
	{"getAN", 			doPhygetAN,				0x02, 1, "getAN <phyaddr> "},
	{"forcePWD", 		doPhyforcePWD,			0x02, 1, "forcePWD <phyaddr> "},
	{"getLinkStatus", 	doPhygetLinkStatus,		0x02, 1, "getLinkStatus <phyaddr> "},
	{"forcePause", 		doPhyforcePause,		0x02, 1, "forcePause <phyaddr> <on|off> "},
	{"getLoopback", 	doPhygetLoopback,		0x02, 1, "getLoopback <phyaddr> "},
	{"forceLoopback", 	doPhyforceLoopback,		0x02, 1, "forceLoopback <phyaddr> "},
	{"getLPCap", 		doPhygetLPCap,			0x02, 1, "getLPCap <phyaddr> "},
	{"getCAL", 			doPhygetCALReg,			0x02, 1, "getCAL <phyaddr> "}, 
	{"forceLED", 		doPhyForceLED,			0x02, 1, "forceLED <phyaddr> <enable|disable>"}, 
	#endif
	#endif
	#endif // TCPHY_DEBUG
    {NULL,			NULL,					0,	0,	NULL},
};




// CI command for tcephydbg
//int doEtherPhydbg(int argc, char *argv[], void *p) 
//{
//    return subcmd(ethertphyprint, argc, argv, p);
//} 
 


#endif //TCPHY_SUPPORT
#ifdef TCPHY_DEBUG
u8 getTcPhyLookbackFlag(void)
{
    return phy_loopback_flag;
}
u8 getTcPhyForceLinkFlag(void)
{	
    return force_link_flag;
}
#endif //TCPHY_DEBUG

/************************************************************************
*         API functions body for EPHY (TCPHY_SUPPORT)
**************************************************************************/
#ifdef TCPHY_SUPPORT

u16 getMiiPage(char *page)
{
    // g0,g1,g2,g3,g4,g5 = 0x0000~0x5000
    // l0,l1,l2,l3 = 0x8000~0xb000
    if(stricmp(page, "g0") == 0)
    {
        return (0x0000);
    }
    else if(stricmp(page, "g1") == 0)
    {
        return (0x1000);
    }
    else if(stricmp(page, "g2") == 0)
    {
        return (0x2000);
    }
    else if(stricmp(page, "g3") == 0)
    {
        return (0x3000);
    }
	else if(stricmp(page, "g4") == 0)
	{
        return (0x4000);
    }
	else if(stricmp(page, "g5") == 0)
	{
        return (0x5000);
    }
	else if(stricmp(page, "g6") == 0)
	{
        return (0x6000);
    }
    else if(stricmp(page, "g7") == 0)
	{
        return (0x7000);
    }
    else if(stricmp(page, "l0") == 0)
    {
        return (0x8000);
    }
    else if(stricmp(page, "l1") == 0)
    {
        return (0x9000);
    }
    else if(stricmp(page, "l2") == 0)
    {
        return (0xa000);
    }
    else if(stricmp(page, "l3") == 0)
    {
        return (0xb000);
    }
    else if(stricmp(page, "l4") == 0)
    {
        return (0xc000);
    }
	else if(stricmp(page, "ext") == 0)
	{	//Extended Page Registers
	    return (0x0001);
	}
	else if(stricmp(page, "misc") == 0)
	{	//Misc Page Registers
	    return (0x0002);
	}
	else if(stricmp(page, "lpi") == 0)
	{	//LPI Page Registers
	    return (0x0003);
	}
	else if( stricmp(page, "test") == 0)
	{	//Test Page Registers
	    return (0x2a30);
	}
	else if(stricmp(page, "tr") == 0)
	{	//Token Ring Debug Registers
		return (0x52b5);
	}
    else 
    {
        printk("* Wrong PageNo(%s).\r\n",page);
	    printk("* MTGPHY PageNo=><g0|ext|misc|lpi|test|tr> \r\n");
	    printk("* Tcxxx PageNo=><g0|g1|g2|g3|g4|g5|l0|l1|l2|l3|l4> \r\n");
        return (0xFFFF);
    }
}

int	doPhyMiiRead(int argc, char *argv[], void *p)
{
    u16 	phyaddr=0;
    const 	u16 page_reg=31;
    u16 	page_val=0;
    u32 	reg=0;
    u32 	value;
    int i;

    //  argc:3
    //  tce miir all <PhyAddr> 
    //  tce miir <PhyAddr> <RegAddr>
    //  argc:4
    //  tce miir all <PhyAddr> <PageNo>
    //  tce miir <PhyAddr> <PageNo> <RegAddr>

    // get parameters
    if(argc==3)
    {
        if(stricmp(argv[1], "all") == 0)
        { 	// tce miir all <PhyAddr> 
            phyaddr = checked_atoi(argv[2]);
        }
        else
        { 	// tce miir <PhyAddr> <RegAddr>
            phyaddr = checked_atoi(argv[1]);
            reg = checked_atoi(argv[2]);
           	//sscanf(argv[2], "%x", &reg);
		   //printk("*1: PhyAddr=%d, RegAddr=0x%02lX\r\n", phyaddr, reg);	   
        }
    }
    else if(argc==4)
    {
        if(stricmp(argv[1], "all") == 0)
        { 	// tce miir all <PhyAddr> <PageNo>
            phyaddr = checked_atoi(argv[2]);
            page_val = getMiiPage(argv[3]);
        }
        else
        { 	// tce miir <PhyAddr> <PageNo> <RegAddr>
            phyaddr = checked_atoi(argv[1]);
            page_val = getMiiPage(argv[2]);
            reg = checked_atoi(argv[3]);
            //sscanf(argv[3], "%x", &reg);
        }
    }

    // check parameters
    if((argc==3 || argc==4) 
        && phyaddr<=31 && reg<=31 && page_val!=0xffff)
    {
        // set page
//        if(!isFPGA)
//		{	
        	if (argc==4)
        	{           
            	if(stricmp(argv[1], "all") == 0)
            	{ 	// multiple read
                	printk("* PageNo=%s ",argv[3]);
                	printk("\r\n");             
            	}
            	else
            	{
                	printk("* PageNo=%s ",argv[2]);
            	}
				#ifdef LINUX_OS // allen_20130926
				tcMiiStationWrite(phyaddr, page_reg, page_val);
				#else
				tcMiiStationWrite(phyaddr, page_reg, page_val);
				#endif	
        	}
//		}//if(!isFPGA)
		
        // read data
        if(stricmp(argv[1], "all") == 0)
		{	// multiple read
           	for( i=0; i<32; i++ )
			{
				#ifdef LINUX_OS // allen_20130926	
				value = tcMiiStationRead(phyaddr, i);
				#else
				value = tcMiiStationRead(phyaddr, i);
				#endif
                printk("[reg=%02d val=%04X]", i, value);
                if( (i+1) % 4 == 0 )
                    printk("\r\n");             
            }
        }
        else
		{
			#ifdef LINUX_OS // allen_20130926
			value = tcMiiStationRead(phyaddr, reg);
			#else
			value = tcMiiStationRead(phyaddr, reg);
			#endif
            printk("* PhyAddr=%d RegAddr=%02d value=%04X\r\n", phyaddr, reg, value);
        }       
        return 0;           
    }   
    else
    { 	// error message
        printk("Usage: miir all <PhyAddr> [PageNo]\r\n");
        printk("       miir <PhyAddr> <RegAddr>\r\n");
        printk("       miir <PhyAddr> <PageNo> <RegAddr>\r\n");
        return 0;           
    }
}

int doPhyMiiWrite (int argc, char *argv[], void *p)
{
    u32 	phyaddr=0;
    const 	u16 page_reg=31;
    u16 	page_val=0;
	u32 	reg=0;
    u32 	value=0;
	u16 	STBit=0;
	u16 	BFlen=0;
	u16 	BF,BFMsk=0;
	u32 	RValue =0;
	int 	i;
    //  tce miir <PhyAddr> <RegAddr> <Value>
    //  tce miir <PhyAddr> <PageNo> <RegAddr> <Value>
    // get parameters
    if(argc==4)
	{
        phyaddr = checked_atoi(argv[1]);
		reg = checked_atoi(argv[2]);
		//sscanf(argv[2], "%x", &reg);
        sscanf(argv[3], "%x", &value);
		//printk("value = %x", value);
		//printk("*1: argv[2]=%s, argv[3]=%s\r\n", argv[2], argv[3]);		
		//printk("*2: phyaddr=%d, Reg=0x%02lX, value=0x%04lX\r\n", phyaddr, reg , value);		
    }
    else if (argc==5)
    {
        phyaddr = checked_atoi(argv[1]);
        page_val = getMiiPage(argv[2]);
        reg = checked_atoi(argv[3]);
        //sscanf(argv[3], "%x", &reg);
        sscanf(argv[4], "%x", &value);
    }
	else if (argc==6)
	{
		if(stricmp(argv[0], "miiwb") == 0)
		{
			phyaddr = checked_atoi(argv[1]);
		    reg = checked_atoi(argv[2]);
			//sscanf(argv[2], "%x", &reg);
			STBit = checked_atoi(argv[3]);
			BFlen = checked_atoi(argv[4]);
			sscanf(argv[5], "%x", &value);
			printk("* Phyaddr=%d, RegAddr=%02d, STBit=%0d, BFlen=%0d, value=%04X\r\n", phyaddr, reg, STBit, BFlen, value);			
		}
	}
	else if (argc==7)
	{
		if(stricmp(argv[0], "miiwb") == 0)
		{
			phyaddr = checked_atoi(argv[1]);
			page_val = getMiiPage(argv[2]);
			reg = checked_atoi(argv[3]);
			//sscanf(argv[3], "%x", &reg);
			STBit = checked_atoi(argv[4]);
			BFlen = checked_atoi(argv[5]);
			sscanf(argv[6], "%x", &value);
			printk("* Phyaddr=%d, pageNo=%d, RegAddr=%02d, STBit=%0d, BFlen=%0d, value=%04X\r\n", phyaddr, page_val, reg, STBit, BFlen, value);			
		}
	}
		
    // check parameters and write
    if ((argc==4 || argc==5) 
        && (phyaddr<=31) && (reg<=31) && (page_val!=0xffff))
	{
        // set page
//        if(!isFPGA)
//		{
       		if (argc==5) 
			{
        	    printk("* PageNo=%s ",argv[2]);
				#ifdef LINUX_OS // allen_20130926			
				tcMiiStationWrite(phyaddr, page_reg, page_val);
				#else
				tcMiiStationWrite(phyaddr, page_reg, page_val);
				#endif	
        	}
//	   	}
        // write data
        printk("* Phyaddr=%d RegAddr=%02d value=%04X\r\n", phyaddr, reg, value);
		#ifdef LINUX_OS // allen_20130926	
		tcMiiStationWrite(phyaddr, reg, value);
		#else
		tcMiiStationWrite(phyaddr, reg, value);
		#endif
    }
	#ifdef LINUX_OS // allen_20130926
 	else if (((argc == 6) || (argc == 7))
		&& (phyaddr<=31) && (reg<=31) && (page_val != 0xffff))
	{
//		if(!isFPGA)
//		{
        	if (argc==7) 
			{
            	printk("* PageNo=%s ",argv[2]);
				tcMiiStationWrite(phyaddr, page_reg, page_val);
        	}
//		}
		
		if(stricmp(argv[0], "miiwb") == 0)
		{

			if (BFlen==0) return 1;
		 	for(i=0;i<BFlen;i++)
		 	{
				BF=1;
				if ((STBit+i)>31) break; //coverify check
			 	BF=BF<<(STBit+i);
			 	BFMsk = BFMsk | BF;
			  	//printk("1: BF=%d, BFMsk=%04x\r\n", BF, BFMsk);
			}
			BFMsk = ~BFMsk;
			//printk("2: BF=%d, BFMsk=%04x\r\n", BF, BFMsk); 
			if (STBit > 32) return 1;//coverify check
			value = (RValue & BFMsk) | (value<<STBit);
			//printk("* Phyaddr=%d, RegAddr=0x%02lX, OrgValue=0x%04lX, Modified value=0x%04lX\r\n", phyaddr, reg,  RValue, value);

			printk("* OrgValue=%04X,",RValue);
			tcMiiStationWrite(phyaddr, reg, value);
			RValue = tcMiiStationRead(phyaddr, reg);
			printk(" ModValue=%04X\r\n", RValue);
		}
    }
	#endif
    else 
	{	// error message
		printk("Usage: miiw <PhyAddr> <RegAddr> <RegVal>\r\n");
		printk("       miiw <PhyAddr> <PageNo> <RegAddr> <RegVal>\r\n");
		#ifdef LINUX_OS // allen_20130926	
		printk("       miiwb <PhyAddr> <RegAddr> <STBit> <BFLen> <BFVal> \r\n");	
		printk("       miiwb <PhyAddr> <PageNo> <RegAddr> <STBit> <BFLen> <BFVal> \r\n");	
		#endif
        return 0;
    }
    return 0;
}

int	doPbusRead(int argc, char *argv[], void *p)
{
	u32 phyaddr=0;
	u32 pbus_addr=0;
	u32 value=0;
	u32 high_byte=0,low_byte=0;

	if((argc <=2)|| (argc >=4))
	{	
		printk("	   CMD Error : tce pbr <PhyAddr> <pbus_addr>\r\n");
		
	}
	else	
	{
		phyaddr = checked_atoi(argv[1]);
		sscanf(argv[2], "%x", &pbus_addr);//hex
		tcMiiStationWrite(phyaddr, 0x1f, (pbus_addr >> 6));
		low_byte = tcMiiStationRead(phyaddr, ((pbus_addr >> 2)& 0xF));
		high_byte = tcMiiStationRead(phyaddr, 0x10);
		value=(65536*high_byte)+low_byte;
		printk(" Pbus read => phyaddr=%d, pbus_addr=0x%08X , value=0x%08X\r\n", phyaddr, pbus_addr, value);
		return 0;	   
	}

   return 0; 

}

int doPbusWrite (int argc, char *argv[], void *p)
{

	u32 phyaddr=0;
	u32 pbus_addr=0;
	u32 value=0;


	if((argc <=3)|| (argc >=5))
	{		

		printk("	   CMD Error : tce pbw <PhyAddr> <pbus_addr> <value>\r\n");

		
	}
	else	
	{
		phyaddr = checked_atoi(argv[1]);
		sscanf(argv[2], "%x", &pbus_addr);//hex
		sscanf(argv[3], "%x", &value);//hex
		tcMiiStationWrite(phyaddr, 0x1f, (pbus_addr >> 6));
		tcMiiStationWrite(phyaddr, ((pbus_addr >> 2)& 0xF), (value & 0xFFFF));
		tcMiiStationWrite(phyaddr, 0x10, (value >> 16));
		printk(" Pbus Write => phyaddr=%d, pbus_addr=0x%08X , value=0x%08X\r\n", phyaddr, pbus_addr, value);
		return 0;	   
	}

   return 0;

}

#ifdef TCPHY_1PORT
static int doPhySpeed (int argc, char *argv[], void *p)
{
    u8 port_num = 0;

    if(argc != 2 ) 
    {
        printk("Usage: Speed <Auto|AN|100F|100H|10F|10H|Disp>\r\n");
    }   
    else 
    {
        if(stricmp(argv[1], "Auto") == 0 ) 
        {
            tcphy_speed = tcphy_speed_Auto;
            //tcPhyWriteReg(port_num,0,0x1200);
        }
        else if(stricmp(argv[1], "AN") == 0 )
        {
            tcphy_speed = tcphy_speed_ForceAN;
            //tcPhyWriteReg(port_num,0,0x1200);
        }
        else if(stricmp(argv[1], "100F") == 0 ) 
        {
            tcphy_speed = tcphy_speed_Force100F;
            tcPhyWriteReg(port_num,0,0x2100);
        }
        else if(stricmp(argv[1], "100H") == 0 )
        {
            tcphy_speed = tcphy_speed_Force100H;
            tcPhyWriteReg(port_num,0,0x2000);
        }
        else if(stricmp(argv[1], "10F") == 0 ) 
        {
            tcphy_speed = tcphy_speed_Force10F;
            tcPhyWriteReg(port_num,0,0x0100);
        }
        else if(stricmp(argv[1], "10H") == 0 )
        {
            tcphy_speed = tcphy_speed_Force10H;
            tcPhyWriteReg(port_num,0,0x0000);
        }
        else 
        {
            printk("Current Speed mode: %s.\r\n",
                    (tcphy_speed==tcphy_speed_Auto)?"Auto":
                    (tcphy_speed==tcphy_speed_ForceAN)?"ForceAN":
                    (tcphy_speed==tcphy_speed_Force100F)?"Force100F":
                    (tcphy_speed==tcphy_speed_Force100H)?"Force100H":
                    (tcphy_speed==tcphy_speed_Force10F)?"Force10F":
                    (tcphy_speed==tcphy_speed_Force10H)?"Force10H":
                                                        "Unknown");
        }
    }
    return 0;
}
#endif

int buck_pbus_read_command(int argc, char *argv[], void *p)
{
	unsigned int reg_addr,phyaddr,data;
	
		if((argc <=2)||(argc >=4)) 
	{
	   	printk("bpr <phyaddr> <register>\r\n");
	}
	else
	{
		phyaddr = checked_atoi(argv[1]);
		sscanf(argv[2], "%x", &reg_addr);//hex
		data=buck_pbus_read( phyaddr, reg_addr);
		printk("phy=%d register=0x%x data=0x%x \r\n",phyaddr,reg_addr,data);
		
	}	
    return 0;
	
}
int buck_pbus_write_command(int argc, char *argv[], void *p)
{
	unsigned int reg_addr,phyaddr,value,data;
	
		if((argc <=3)||(argc >=5)) 
	{
	   	printk("bpw <phyaddr> <register> <value> \r\n");
	}
	else
	{
		phyaddr = checked_atoi(argv[1]);
		sscanf(argv[2], "%x", &reg_addr);//hex
		sscanf(argv[3], "%x", &value);//hex
		buck_pbus_write( phyaddr, reg_addr,value);
		data=buck_pbus_read( phyaddr, reg_addr);
		printk("phy=%d register=0x%x data=0x%x \r\n",phyaddr,reg_addr,data);		
	}	
    return 0;
	
}

int doPhyGphyTestMode (int argc, char *argv[], void *p)
{
	u16 phyaddr = 0, mode=0, pair=0, speed=0;
	//u32 reg3 = 0, r15_temp = 0,temp1,temp2;

	if((argc <4)||(argc >=6)) 
	{
	   	printk("Usage: testmode <speed> <phyaddr> <mode or channel> <pair>\r\n");
	}
	else	
	{
		LDPS_function_en = 0;
		testmode_excuted_flag = 1;
		speed=checked_atoi(argv[1]);
    	phyaddr = checked_atoi(argv[2]);
		mode=checked_atoi(argv[3]);
    	if(argc ==5) {pair=checked_atoi(argv[4]);}

			if(speed == 2500 )	
		  	{
			    buck_pbus_write(phyaddr,0x10204,0x0);
				mtEMiiRegWrite(phyaddr, 0x1e, 0x800c, 0x0008);
				mtEMiiRegWrite(phyaddr, 0x1e, 0x800d, 0x0000);
				mtEMiiRegWrite(phyaddr, 0x1e, 0x800e, 0x1100);
				mtEMiiRegWrite(phyaddr, 0x1e, 0x800f, 0x0001);

			 if(mode == 1 )
			  {
				  buck_pbus_write(phyaddr,0x30008,0x1000007);
				  buck_pbus_write(phyaddr,0x30200,0x8f601101);
				  buck_pbus_write(phyaddr,0x30004,0x112101);
				  printk("port[%d] into 2.5G testmode 1\r\n",phyaddr);					  
			  }
			 if(mode == 2)
			  {
				  buck_pbus_write(phyaddr,0x30200,0x8c611101);
				  buck_pbus_write(phyaddr,0x30004,0x122101);
				  buck_pbus_write(phyaddr,0x30200,0x8c601101);
				  printk("port[%d] into 2.5G testmode 2\r\n",phyaddr);	
				  
			  }		
			 if(mode == 3)
			  {
				  buck_pbus_write(phyaddr,0x30200,0x8c611101);
				  buck_pbus_write(phyaddr,0x30200,0x89611101);
				  buck_pbus_write(phyaddr,0x30004,0x132101);
				  buck_pbus_write(phyaddr,0x85024,0x0);
				  buck_pbus_write(phyaddr,0x30200,0x89601101);
				  printk("port[%d] into 2.5G testmode 3\r\n",phyaddr);	
				  
			  }
			 if(mode == 4)
			  {
				  buck_pbus_write(phyaddr,0x30200,0x8c611101);
				  if(pair == 1 ) 
				  {
					  buck_pbus_write(phyaddr,0x30004,0x142101);
					  printk("port[%d] into 2.5G testmode 4 tone 1\r\n",phyaddr);
				  }
				  if(pair == 2 ) 
				  {
					  buck_pbus_write(phyaddr,0x30004,0x242101);
					  printk("port[%d] into 2.5G testmode 4 tone 2\r\n",phyaddr);
				  }
				  if(pair == 3 ) 
				  {
					  buck_pbus_write(phyaddr,0x30004,0x442101);
					  printk("port[%d] into 2.5G testmode 4 tone 3\r\n",phyaddr);
				  }
				  if(pair == 4 ) 
				  {
					  buck_pbus_write(phyaddr,0x30004,0x542101);
					  printk("port[%d] into 2.5G testmode 4 tone 4\r\n",phyaddr);
				  }
				  if(pair == 5 ) 
				  {
					  buck_pbus_write(phyaddr,0x30004,0x642101);
					  printk("port[%d] into 2.5G testmode 4 tone 5\r\n",phyaddr);
				  }
				  buck_pbus_write(phyaddr,0x30200,0x8c601101);
				  buck_pbus_write(phyaddr,0x3089c,0x1ff);
					  
			  }		
			 if(mode == 5)
			  {
				  buck_pbus_write(phyaddr,0x30200,0x8c611101);
				  buck_pbus_write(phyaddr,0x30004,0x152101);
				  buck_pbus_write(phyaddr,0x30200,0x8c601101);
				  buck_pbus_write(phyaddr,0x30080,0xc000006);
				  buck_pbus_write(phyaddr,0x30898,0x1ff01d8);				  
				  printk("port[%d] into 2.5G testmode 5\r\n",phyaddr);					  
			  }	
			 if(mode == 6)
			  {
				  buck_pbus_write(phyaddr,0x30200,0x8c611101);
				  buck_pbus_write(phyaddr,0x30004,0x162101);
				  buck_pbus_write(phyaddr,0x30200,0x8c601101);
				  printk("port[%d] into 2.5G testmode 6\r\n",phyaddr);				  
			  }	
			  
			}
		  if(speed == 1000 )	{
			if(mode == 0 )
			{	//default
				tcMiiStationWrite(phyaddr, 9, 0x600);
				tcMiiStationWrite(phyaddr, 0x0, 0x1040);
            	mtEMiiRegWrite(phyaddr, 0x1e, 0x145, 0x1000);	
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x1a3, 0x00d2);
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x1a4, 0x010e);
				mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xc000);
				tcMiiStationWrite(phyaddr, 0x1f, 0x0001);
				tcMiiStationWrite(phyaddr, 0x1d, 0x0000);
				printk("Usage: default\r\n");
			}
			if(mode == 1 )
			{	//Gphy test mode1
				mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xc000);
				
				mtEMiiRegWrite(phyaddr, 0x1e, 0x0011, 0x0f00);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0200, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0201, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0202, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0203, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0204, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0205, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0206, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0207, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0208, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0209, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020a, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020b, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020c, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020d, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020e, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020f, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0210, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0211, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0212, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0213, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0214, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0215, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0216, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0217, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0218, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0219, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021a, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021b, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021c, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021d, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021e, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021f, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0220, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0221, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0222, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0223, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0224, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0225, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0226, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0227, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0228, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0229, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022a, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022b, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022c, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022d, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022e, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022f, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0230, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0231, 0x0020);
				
				tcMiiStationWrite(phyaddr, 9, 0x2700);
				printk("Usage:port[%d] into testmode1\r\n",phyaddr);
			}
			if(mode == 2 )
			{	//Gphy test mode2
				mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xc000);
				
				mtEMiiRegWrite(phyaddr, 0x1e, 0x0011, 0x0f00);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0200, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0201, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0202, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0203, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0204, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0205, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0206, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0207, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0208, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0209, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020a, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020b, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020c, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020d, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020e, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020f, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0210, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0211, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0212, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0213, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0214, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0215, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0216, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0217, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0218, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0219, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021a, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021b, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021c, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021d, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021e, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021f, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0220, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0221, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0222, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0223, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0224, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0225, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0226, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0227, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0228, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0229, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022a, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022b, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022c, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022d, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022e, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022f, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0230, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0231, 0x0020);				
		
				tcMiiStationWrite(phyaddr, 9, 0x4700);
				printk("Usage:port[%d] into testmode2\r\n",phyaddr);
			}
			if(mode == 3 )
			{	//Gphy test mode3

				mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xc000);

				mtEMiiRegWrite(phyaddr, 0x1e, 0x0011, 0x0f00);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0200, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0201, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0202, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0203, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0204, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0205, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0206, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0207, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0208, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0209, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020a, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020b, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020c, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020d, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020e, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020f, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0210, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0211, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0212, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0213, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0214, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0215, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0216, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0217, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0218, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0219, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021a, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021b, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021c, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021d, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021e, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021f, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0220, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0221, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0222, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0223, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0224, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0225, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0226, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0227, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0228, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0229, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022a, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022b, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022c, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022d, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022e, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022f, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0230, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0231, 0x0020);				
		
				tcMiiStationWrite(phyaddr, 9, 0x6700);
				printk("Usage:port[%d] into testmode3\r\n",phyaddr);
			}
			if(mode == 4 )
			{	//Gphy test mode4
				
			if(pair == 1 )
			{	//Gphy test mode4 pairA
				mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0x7070);
				printk("Usage:port[%d] into testmode4_pairA\r\n",phyaddr);
			}
			if(pair == 2)
			{	//Gphy test mode4 pairB
				mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xb0b0);
    			printk("Usage:port[%d] into testmode4_pairB\r\n",phyaddr);
			}
			if(pair == 3 )
			{	//Gphy test mode4 pairC
			   	mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xd0d0);
				printk("Usage:port[%d] into testmode4_pairC\r\n",phyaddr);
			}
			if(pair == 4 )
			{	//Gphy test mode4 pairD
				mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xe0e0);
				printk("Usage:port[%d] into testmode4_pairD\r\n",phyaddr);
			}

				mtEMiiRegWrite(phyaddr, 0x1e, 0x0011, 0x0f00);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0200, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0201, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0202, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0203, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0204, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0205, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0206, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0207, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0208, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0209, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020a, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020b, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020c, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020d, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020e, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x020f, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0210, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0211, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0212, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0213, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0214, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0215, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0216, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0217, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0218, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0219, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021a, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021b, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021c, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021d, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021e, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x021f, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0220, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0221, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0222, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0223, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0224, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0225, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0226, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0227, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0228, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0229, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022a, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022b, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022c, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022d, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022e, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x022f, 0x0020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0230, 0x2020);
				mtEMiiRegWrite(phyaddr, 0x1f, 0x0231, 0x0020);				
				tcMiiStationWrite(phyaddr, 9, 0x8700);
				//tcMiiStationWrite(phyaddr, 9, 0x8700);
				//printk("Usage:port[%d] into testmode4\r\n",phyaddr);
			}
		  }
		if(speed == 100 )	{
			//send pair A 100M MLT3
			if(mode == 1 ) 
			{
            	tcMiiStationWrite(phyaddr, 0x1f, 0x0000);
	            tcMiiStationWrite(phyaddr, 0x0, 0x8000);
	            udelay(100);
	            tcMiiStationWrite(phyaddr, 0x9, 0x0600);
             	tcMiiStationWrite(phyaddr, 0x0, 0x2100);
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x145, 0x5010);	
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x1a3, 0x00d2);
            	mtEMiiRegWrite(phyaddr, 0x1e, 0x1a4, 0x010e);
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x3e, 0xc000);  //disable power save mode for pair A &B	
	            printk("set p[%d] 100M pairA \r\n", phyaddr);	
			}
			//send pair B MLT3
			if(mode == 2 ) 
			{
	            tcMiiStationWrite(phyaddr, 0x1f, 0x0000);
	            tcMiiStationWrite(phyaddr, 0x0, 0x8000);
	            udelay(100);
	            tcMiiStationWrite(phyaddr, 0x9, 0x0600);
	            tcMiiStationWrite(phyaddr, 0x0, 0x2100);
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x145, 0x5018);	
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x1a3, 0x00d2);
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x1a4, 0x010e);
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x3e, 0xc000);  //disable power save mode for pair A &B					
	            printk("set p[%d] 100M pairB \r\n", phyaddr);					
			}
			
		 }
		if(speed == 10 )	{ 
			//send pair A NLP
			if(mode == 1 ) 
			{
	           tcMiiStationWrite(phyaddr, 0x1f, 0x0001);
	           tcMiiStationWrite(phyaddr, 0x1d, 0x0000);
	
	           tcMiiStationWrite(phyaddr, 0x1f, 0x0000);
	           tcMiiStationWrite(phyaddr, 0x0, 0x8000);

	           udelay(100);
	
	           //set force link 10M
	           tcMiiStationWrite(phyaddr, 0x0, 0x0100);
	           //set giga ability and switch normal mode
               tcMiiStationWrite(phyaddr, 0x9, 0x0600);
	           //desable 10 base TE
	           mtEMiiRegWrite(phyaddr, 0x1f, 0x27b, 0x1177);  
	           //disable auto MDIX and force MDI
	           mtEMiiRegWrite(phyaddr, 0x1e, 0x145, 0x5010);
               //return to normal
	           mtEMiiRegWrite(phyaddr, 0x1e, 0x1a3, 0x00d2);
	           mtEMiiRegWrite(phyaddr, 0x1e, 0x1a4, 0x010e);
	           mtEMiiRegWrite(phyaddr, 0x1e, 0x3e, 0xc000);  //disable power save mode for pair A &B

	           printk("set p[%d] 10M link pulse \r\n", phyaddr);				
			}		
			//send pair A random
			if(mode == 2 ) 
			{
	           //disable packet gen
	           tcMiiStationWrite(phyaddr, 0x1f, 0x0001);
	           tcMiiStationWrite(phyaddr, 0x1d, 0x0000);
	           //do phy software reset
	           tcMiiStationWrite(phyaddr, 0x1f, 0x0000);
	           tcMiiStationWrite(phyaddr, 0x0, 0x8000);

	           udelay(100);
	           //set force link 10M
	           tcMiiStationWrite(phyaddr, 0x0, 0x0100);
	           //set giga ability and switch normal mode
               tcMiiStationWrite(phyaddr, 0x9, 0x0600);
	
	           //desable 10 base TE
	           mtEMiiRegWrite(phyaddr, 0x1f, 0x27b, 0x1177);  
	           //disable auto MDIX and force MDI
	           mtEMiiRegWrite(phyaddr, 0x1e, 0x145, 0x5010);
	           //return to normal
	           mtEMiiRegWrite(phyaddr, 0x1e, 0x1a3, 0x00d2);
	           mtEMiiRegWrite(phyaddr, 0x1e, 0x1a4, 0x010e);
               //set random packet
	           tcMiiStationWrite(phyaddr, 0x1f, 0x0001);
	           tcMiiStationWrite(phyaddr, 0x1d, 0xf842);
               mtEMiiRegWrite(phyaddr, 0x1e, 0x3e, 0xc000);  //disable power save mode for pair A &B

	           printk("set p[%d] 10M random \r\n", phyaddr);
			}		
			//send pair A fixed
			if(mode == 3 ) 
			{
                //disable packet gen
	            tcMiiStationWrite(phyaddr, 0x1f, 0x0001);
	            tcMiiStationWrite(phyaddr, 0x1d, 0x0000);
	            //do phy software reset
	            tcMiiStationWrite(phyaddr, 0x1f, 0x0000);
	            tcMiiStationWrite(phyaddr, 0x0, 0x8000);
             	udelay(100);
	
	            //set force link 10M
	            tcMiiStationWrite(phyaddr, 0x0, 0x0100);
	            //set giga ability and switch normal mode
                tcMiiStationWrite(phyaddr, 0x9, 0x0200);
	            //desable 10 base TE
	            mtEMiiRegWrite(phyaddr, 0x1f, 0x27b, 0x1177);  
	            //disable auto MDIX and force MDI
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x145, 0x5010);
                //for harmonic
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x1a3, 0x0000);
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x1a4, 0x0000);

                //set fixed data packet  0x00
	            tcMiiStationWrite(phyaddr, 0x1f, 0x0001);
                tcMiiStationWrite(phyaddr, 0x1e, 0x0000);
	            tcMiiStationWrite(phyaddr, 0x1d, 0xf840);
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x3e, 0xc000);  //disable power save mode for pair A &B
            	printk("set p[%d] 10M fix \r\n", phyaddr);
			}					
		
		  
		}
		}
    return 0;
}
		
int extPhyGphyTestMode (int argc, char *argv[], void *p)
{
	u16 phyaddr = 0, mode=0, pair=0, speed=0;
    u32 temp1;
	//u32 reg3 = 0, r15_temp = 0,temp1,temp2;

	if((argc <4)||(argc >=6)) 
	{
	   	printk("Usage: testmode <speed> <phyaddr> <mode or channel> <pair>\r\n");
	}
	else	
	{
		speed=checked_atoi(argv[1]);
    	phyaddr = checked_atoi(argv[2]);
		mode=checked_atoi(argv[3]);
    	if(argc ==5) {pair=checked_atoi(argv[4]);}
			if(speed == 2500 )	
		  	{
			    buck_pbus_write(phyaddr,0x10204,0x0);
				mtEMiiRegWrite(phyaddr, 0x1e, 0x800c, 0x0008);
				mtEMiiRegWrite(phyaddr, 0x1e, 0x800d, 0x0000);
				mtEMiiRegWrite(phyaddr, 0x1e, 0x800e, 0x1100);
				mtEMiiRegWrite(phyaddr, 0x1e, 0x800f, 0x0001);

			 if(mode == 1 )
			  {
				  buck_pbus_write(phyaddr,0x30008,0x1000007);
				  buck_pbus_write(phyaddr,0x30200,0x8f601101);
				  buck_pbus_write(phyaddr,0x30004,0x112101);
				  printk("port[%d] into 2.5G testmode 1\r\n",phyaddr);					  
			  }
			 if(mode == 2)
			  {
				  buck_pbus_write(phyaddr,0x30200,0x8c611101);
				  buck_pbus_write(phyaddr,0x30004,0x122101);
				  buck_pbus_write(phyaddr,0x30200,0x8c601101);
				  printk("port[%d] into 2.5G testmode 2\r\n",phyaddr);	
				  
			  }		
			 if(mode == 3)
			  {
				  buck_pbus_write(phyaddr,0x30200,0x8c611101);
				  buck_pbus_write(phyaddr,0x30200,0x89611101);
				  buck_pbus_write(phyaddr,0x30004,0x132101);
				  buck_pbus_write(phyaddr,0x85024,0x0);
				  buck_pbus_write(phyaddr,0x30200,0x89601101);
				  printk("port[%d] into 2.5G testmode 3\r\n",phyaddr);	
				  
			  }
			 if(mode == 4)
			  {
				  buck_pbus_write(phyaddr,0x30200,0x8c611101);
				  if(pair == 1 ) 
				  {
					  buck_pbus_write(phyaddr,0x30004,0x142101);
					  printk("port[%d] into 2.5G testmode 4 tone 1\r\n",phyaddr);
				  }
				  if(pair == 2 ) 
				  {
					  buck_pbus_write(phyaddr,0x30004,0x242101);
					  printk("port[%d] into 2.5G testmode 4 tone 2\r\n",phyaddr);
				  }
				  if(pair == 3 ) 
				  {
					  buck_pbus_write(phyaddr,0x30004,0x442101);
					  printk("port[%d] into 2.5G testmode 4 tone 3\r\n",phyaddr);
				  }
				  if(pair == 4 ) 
				  {
					  buck_pbus_write(phyaddr,0x30004,0x542101);
					  printk("port[%d] into 2.5G testmode 4 tone 4\r\n",phyaddr);
				  }
				  if(pair == 5 ) 
				  {
					  buck_pbus_write(phyaddr,0x30004,0x642101);
					  printk("port[%d] into 2.5G testmode 4 tone 5\r\n",phyaddr);
				  }
				  buck_pbus_write(phyaddr,0x30200,0x8c601101);
				  buck_pbus_write(phyaddr,0x3089c,0x1ff);
					  
			  }		
			 if(mode == 5)
			  {
				  buck_pbus_write(phyaddr,0x30200,0x8c611101);
				  buck_pbus_write(phyaddr,0x30004,0x152101);
				  buck_pbus_write(phyaddr,0x30200,0x8c601101);
				  buck_pbus_write(phyaddr,0x30080,0xc000006);
				  buck_pbus_write(phyaddr,0x30898,0x1ff01d8);				  
				  printk("port[%d] into 2.5G testmode 5\r\n",phyaddr);					  
			  }	
			 if(mode == 6)
			  {
				  buck_pbus_write(phyaddr,0x30200,0x8c611101);
				  buck_pbus_write(phyaddr,0x30004,0x162101);
				  buck_pbus_write(phyaddr,0x30200,0x8c601101);
				  printk("port[%d] into 2.5G testmode 6\r\n",phyaddr);				  
			  }	
			  
			}	
		  if(speed == 1000 )	
		  	{
			if(mode == 0 )
			{	//default
				tcMiiStationWrite(phyaddr, 9, 0x600);
				tcMiiStationWrite(phyaddr, 0x0, 0x1040);
            	mtEMiiRegWrite(phyaddr, 0x1e, 0x145, 0x1000);	
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x1a3, 0x00d2);
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x1a4, 0x010e);
				mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xc000);				
	            tcMiiStationWrite(phyaddr, 0x1f, 0x0001);
	            tcMiiStationWrite(phyaddr, 0x1d, 0x0000);				
				printk("Usage: default\r\n");
			}
			if(mode == 1 )
			{	//Gphy test mode1

				mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xc000);
				tcMiiStationWrite(phyaddr, 9, 0x2700);
				printk("Usage:port[%d] into testmode1\r\n",phyaddr);
			}
			if(mode == 2 )
			{	//Gphy test mode2
                
				mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xc000);
				tcMiiStationWrite(phyaddr, 9, 0x4700);
				printk("Usage:port[%d] into testmode2\r\n",phyaddr);
			}
			if(mode == 3 )
			{	//Gphy test mode3
                
				mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xc000);
				tcMiiStationWrite(phyaddr, 9, 0x6700);
				printk("Usage:port[%d] into testmode3\r\n",phyaddr);
			}
			if(mode == 4 )
			{	//Gphy test mode4
				
			if(pair == 1 )
			{	//Gphy test mode4 pairA
				mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0x7070);
				printk("Usage:port[%d] into testmode4_pairA\r\n",phyaddr);
			}
			if(pair == 2)
			{	//Gphy test mode4 pairB
				mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xb0b0);
    			printk("Usage:port[%d] into testmode4_pairB\r\n",phyaddr);
			}
			if(pair == 3 )
			{	//Gphy test mode4 pairC
			   	mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xd0d0);
				printk("Usage:port[%d] into testmode4_pairC\r\n",phyaddr);
			}
			if(pair == 4 )
			{	//Gphy test mode4 pairD
				mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xe0e0);
				printk("Usage:port[%d] into testmode4_pairD\r\n",phyaddr);

				tcMiiStationWrite(phyaddr, 9, 0x8700);
				//tcMiiStationWrite(phyaddr, 9, 0x8700);
				//printk("Usage:port[%d] into testmode4\r\n",phyaddr);
				}
				}
		  	}

		if(speed == 100 )	
		{
			//send pair A 100M MLT3
			if(mode == 1 ) 
			{
            	tcMiiStationWrite(phyaddr, 0x1f, 0x0000);
			   temp1=tcMiiStationRead(phyaddr, 0x03);
			   if( temp1 != 0xa411 ) {tcMiiStationWrite(phyaddr, 0x0, 0x8000);}
	            udelay(100);
	            tcMiiStationWrite(phyaddr, 0x9, 0x0600);
             	tcMiiStationWrite(phyaddr, 0x0, 0x2100);
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x145, 0x5010);	
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x1a3, 0x00d2);
            	mtEMiiRegWrite(phyaddr, 0x1e, 0x1a4, 0x010e);
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x3e, 0xc000);  //disable power save mode for pair A &B	
	            printk("set p[%d] 100M pairA \r\n", phyaddr);	
			}
			//send pair B MLT3
			if(mode == 2 ) 
			{
	            tcMiiStationWrite(phyaddr, 0x1f, 0x0000);
			   temp1=tcMiiStationRead(phyaddr, 0x03);
			   if( temp1 != 0xa411 ) {tcMiiStationWrite(phyaddr, 0x0, 0x8000);}
	            udelay(100);
	            tcMiiStationWrite(phyaddr, 0x9, 0x0600);
	            tcMiiStationWrite(phyaddr, 0x0, 0x2100);
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x145, 0x5018);	
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x1a3, 0x00d2);
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x1a4, 0x010e);
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x3e, 0xc000);  //disable power save mode for pair A &B					
	            printk("set p[%d] 100M pairB \r\n", phyaddr);					
			}

		 }
		if(speed == 10 )	
		{ 
			//send pair A NLP
			if(mode == 1 ) 
			{
	           tcMiiStationWrite(phyaddr, 0x1f, 0x0001);
	           tcMiiStationWrite(phyaddr, 0x1d, 0x0000);

	           tcMiiStationWrite(phyaddr, 0x1f, 0x0000);
			   temp1=tcMiiStationRead(phyaddr, 0x03);
			   if( temp1 != 0xa411 ) {tcMiiStationWrite(phyaddr, 0x0, 0x8000);}
	           udelay(100);

	           //set force link 10M
	           tcMiiStationWrite(phyaddr, 0x0, 0x0100);
	           //set giga ability and switch normal mode
               tcMiiStationWrite(phyaddr, 0x9, 0x0600);
	           //desable 10 base TE
	           mtEMiiRegWrite(phyaddr, 0x1f, 0x27b, 0x1177);  
	           //disable auto MDIX and force MDI
	           mtEMiiRegWrite(phyaddr, 0x1e, 0x145, 0x5010);
               //return to normal
	           mtEMiiRegWrite(phyaddr, 0x1e, 0x1a3, 0x00d2);
	           mtEMiiRegWrite(phyaddr, 0x1e, 0x1a4, 0x010e);
	           mtEMiiRegWrite(phyaddr, 0x1e, 0x3e, 0xc000);  //disable power save mode for pair A &B

	           printk("set p[%d] 10M link pulse \r\n", phyaddr);				
			}		
			//send pair A random
			if(mode == 2 ) 
			{
	           //disable packet gen
	           tcMiiStationWrite(phyaddr, 0x1f, 0x0001);
	           tcMiiStationWrite(phyaddr, 0x1d, 0x0000);
	           //do phy software reset
	           tcMiiStationWrite(phyaddr, 0x1f, 0x0000);
			   temp1=tcMiiStationRead(phyaddr, 0x03);
			   if( temp1 != 0xa411 ) {tcMiiStationWrite(phyaddr, 0x0, 0x8000);}

	           udelay(100);
	           //set force link 10M
	           tcMiiStationWrite(phyaddr, 0x0, 0x0100);
	           //set giga ability and switch normal mode
               tcMiiStationWrite(phyaddr, 0x9, 0x0600);

	           //desable 10 base TE
	           mtEMiiRegWrite(phyaddr, 0x1f, 0x27b, 0x1177);  
	           //disable auto MDIX and force MDI
	           mtEMiiRegWrite(phyaddr, 0x1e, 0x145, 0x5010);
	           //return to normal
	           mtEMiiRegWrite(phyaddr, 0x1e, 0x1a3, 0x00d2);
	           mtEMiiRegWrite(phyaddr, 0x1e, 0x1a4, 0x010e);
               //set random packet
	           tcMiiStationWrite(phyaddr, 0x1f, 0x0001);
	           tcMiiStationWrite(phyaddr, 0x1d, 0xf842);
               mtEMiiRegWrite(phyaddr, 0x1e, 0x3e, 0xc000);  //disable power save mode for pair A &B

	           printk("set p[%d] 10M random \r\n", phyaddr);
			}		
			//send pair A fixed
			if(mode == 3 ) 
			{
                //disable packet gen
	            tcMiiStationWrite(phyaddr, 0x1f, 0x0001);
	            tcMiiStationWrite(phyaddr, 0x1d, 0x0000);
	            //do phy software reset
	            tcMiiStationWrite(phyaddr, 0x1f, 0x0000);
			   temp1=tcMiiStationRead(phyaddr, 0x03);
			   if( temp1 != 0xa411 ) {tcMiiStationWrite(phyaddr, 0x0, 0x8000);}
             	udelay(100);

	            //set force link 10M
	            tcMiiStationWrite(phyaddr, 0x0, 0x0100);
	            //set giga ability and switch normal mode
                tcMiiStationWrite(phyaddr, 0x9, 0x0200);
	            //desable 10 base TE
	            mtEMiiRegWrite(phyaddr, 0x1f, 0x27b, 0x1177);  
	            //disable auto MDIX and force MDI
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x145, 0x5010);
                //for harmonic
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x1a3, 0x0000);
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x1a4, 0x0000);

                //set fixed data packet  0x00
	            tcMiiStationWrite(phyaddr, 0x1f, 0x0001);
                tcMiiStationWrite(phyaddr, 0x1e, 0x0000);
	            tcMiiStationWrite(phyaddr, 0x1d, 0xf840);
	            mtEMiiRegWrite(phyaddr, 0x1e, 0x3e, 0xc000);  //disable power save mode for pair A &B
            	printk("set p[%d] 10M fix \r\n", phyaddr);
			}					


		}
		}
        return 0;
}


int recalibration(int argc, char *argv[], void *p)
{
	unsigned int phy_address,ephy_addr=9,get_transformer;
		if((argc <=1)||(argc >=3)) 
	{
	   	printk("tce recal 32 re-calibration75xx\r\n");
	    printk("tce recal phy address re-calibration75xx\r\n");
	}
	else
	{
		if(command_calbration_enable==1)
		{
		inital_finish_flag = 0;
		phy_address=checked_atoi(argv[1]);
		if(phy_address >=32) {
		GECal_Rext_flag=0;
		transformer_status=0;
		PreCalSet();
		for(ephy_addr = 9; ephy_addr <= 12; ephy_addr++)
		{
	    		tcPhyVerLookUp(ephy_addr);
  				ePhyInit(ephy_addr);
				get_transformer=auto_select_transformer(ephy_addr);								
		    	doGePhyALLAnalogCal_R45(ephy_addr);
				phyTxAmpcomp(1,ephy_addr,get_transformer);
				//phyTxAmpcomp_new(1,ephy_addr,get_transformer);
		    	tcMiiStationWrite(ephy_addr, 0x00, 0x1240);
	    }
		normalinit();
		inital_finish_flag = 1;
		return 0;}
		else 
		{
			doGePhyALLAnalogCal_R45(phy_address);
			inital_finish_flag = 1;
		}
		} else {  printk("open cal_switch\r\n");}
	}	
    return 0;
}

#endif // TCPHY_SUPPORT

/************************************************************************
*                       API functions body for EPHY
**************************************************************************/


#if defined(TCPHY_DEBUG)|| defined(MTPHY_DEBUG)
int doPhySwPatch (int argc, char *argv[], void *p)
{
	u8 pn;
	
    if(argc != 2 ) 
	{
        printk("Sw patch status: %s.\r\n", (sw_patch_flag? "on": "off"));
    }
    else if(stricmp(argv[1], "on") == 0 )  
	{
        sw_patch_flag = 1;      
        printk("Sw patch status: ON.\r\n");
    }
    else if(stricmp(argv[1], "off") == 0 ) 
	{
        sw_patch_flag = 0;      
        printk("Sw patch status: OFF.\r\n");
    }           
	else if(stricmp(argv[1], "dbg") == 0) // allen_20140822
	{
		printk("\n tcPhyVer = %d \n", tcPhyVer);

		pn = 4;
		printk("mtPhyReg case 1 :Nmr1[%d].value =0x%04x\r\n", pn, Nmr1[pn].value);
		printk("mtPhyReg case 1 :Nmr1[%d].link_status_prev =0x%04x\r\n", pn, Nmr1[pn].link_status_prev );
		printk("mtPhyReg case 1 :Nmr1[%d].link_status =0x%04x\r\n", pn, Nmr1[pn].link_status);
	}	
    else 
	{
        printk("Sw patch status: %s.\r\n", (sw_patch_flag? "on": "off"));
    }
    return 0;
}
#endif

#if defined(TCPHY_DEBUG)

// called by macRxRingProc() for doPhyLoopback()
#ifdef LINUX_OS
u8 phyMbpChk(struct sk_buff* oldMbp)
{
#else
u8 phyMbpChk(mbuf_t*oldMbp)
{
#endif
    if(phy_loopback_flag)
    {
        recv_ok_flag = 1;
        return 1;
    }
    
    if((oldMbp->data[12] == 8) && (oldMbp->data[13] == 0) )
    {	// ip packet
        if(oldMbp->data[34] == 0 && oldMbp->data[23] == 1)//Echo Ping Reply
        {
            if( oldMbp->data[50] == 'B' && oldMbp->data[51] == 'E' && 
                 oldMbp->data[52] == 'A' && oldMbp->data[53] == 'F' )
            {
                PingReplyCnt++;
                return 1;
            }
        }
    }   
    return 0;
}

// called by macRxRingProc() for doPhyLoopback()
#ifdef LINUX_OS
u8 phy_recv_err_check(struct sk_buff** oldMbp, u32 *frameSize)
{
	#ifdef TC2031_DEBUG
 	#if  !(defined(CONFIG_MIPS_TC3182) ||  defined(CONFIG_MIPS_TC3262)) ||  defined(TCSUPPORT_CPU_ARMV8)
	int i = 0;
	if(phy_loopback_flag)
	{
	#if 0  /*jason.gong allen*/
		if(rxDescrp->rdes0.word & (1<<30))
		{
			*frameSize = rxDescrp->rdes0.bits.rfl;
			*frameSize = (*frameSize > 60) ? *frameSize : 60;
		}
		else
		{
			*frameSize = rxDescrp->rdes0.bits.rfl;
		}
		*oldMbp = (struct sk_buff *)(rxDescrp->skb);
		skb_put(*oldMbp, *frameSize);
		dbg_plinel_1("Received packet have CRC error! length=", *frameSize);
		dbg_pline_1("\r\n");
		for(i=0; i<*frameSize; i++ )
			dbg_plineb_1(" ", (*oldMbp)->data[i]);
		dbg_pline_1("\r\n");
		recv_err_flag = 1;	
		#endif
	}
	#endif
	#endif
	return 0;
}
#else
u8 phy_recv_err_check(mbuf_t **oldMbp, macRxDescr_t *rxDescrp, u32 *frameSize)
{
    #ifdef TC2031_DEBUG
    int i = 0;
    if(phy_loopback_flag)
    {
    	if(rxDescrp->rdes0.word & (1<<30))
        {
        	*frameSize = rxDescrp->rdes0.bits.rfl;
            *frameSize = (*frameSize > 60) ? *frameSize : 60;
        }
        else
            *frameSize = rxDescrp->rdes0.bits.rfl;                      
        (volatile mbuf_t *)(*oldMbp) = (volatile mbuf_t *)(rxDescrp->mbuf);
        (*oldMbp)->cnt = *frameSize;
        dbg_plinel_1("Received packet have CRC error! length=", *frameSize);
        dbg_pline_1("\r\n");
        for( i=0; i<*frameSize; i++ )
            dbg_plineb_1(" ", (*oldMbp)->data[i]);
        dbg_pline_1("\r\n");
        recv_err_flag = 1;                      
    }
    #endif
    return 0;
}
#endif

#ifdef LINUX_OS
/*======================================================================================
**      function name: reverseLong
**      descriptions: Reverse the bytes ordering of the input value.
**      parameters:
**            ul: Specify the 4 bytes value that you want to reverse the ordering.
**      revision: Here 2008/08/24
========================================================================================*/
//static u32 reverseLong(u32 ul)
//{
//	u8 z[4];
//	z[3] = *((u8 *)&ul + 0);
//	z[2] = *((u8 *)&ul + 1);
//	z[1] = *((u8 *)&ul + 2);
//	z[0] = *((u8 *)&ul + 3);
//	return *((u32 *)z);
//}/*end reverseLong*/
/*======================================================================================
**      function name: scramble
**      descriptions: Scramble the input 32bits value.            
**      return:  After Scramble the value
**      call: reverseLong
**      revision: Here 2008/08/24
=========================================================================================*/
//static u32 scramble(u32 checkCode)
//{
//	u32 a[6];
//	a[1] = (checkCode & 0x0000001F) << 0x0C;
//	a[2] = (checkCode & 0x03E003E0) << 0x01;
//	a[3] = (checkCode & 0xF8000400) >> 0x0A;
//	a[4] = (checkCode & 0x0000F800) << 0x10;
//	a[5] = (checkCode & 0x041F0000) >> 0x0F;
//	checkCode = a[1] + a[2] + a[3] + a[4] + a[5];
//	/* ICQ's check code is little-endian. Change the endian style */
//	checkCode = reverseLong(checkCode);
//	return checkCode;
//}/*end scramble*/

/*==============================================================================
**      function name: rand
**      descriptions: Random value generation. 
**      return: Random value
**      call: timerVlrGet
**      revision: Here 2008/08/24
================================================================================*/
/*
static u32 rand(void)
{
	u32 vlr = 0;  //Coverity #547970
	timerVlrGet(1, vlr);
	scramble(vlr);
	return (vlr & RAND_MAX);
}
*/
#endif
#ifndef PURE_BRIDGE
#ifndef LINUX_OS
// called by doPing()
static int pingExec(cbuf_t  *cbp,           /* cbuf pointer         */
    u16  signature,      /* event function signature */
    void    *data_p,        /* data pointer         */
    u16  data_size       /* data size            */)
{
    pingEvt_t *ping_evt_p = (pingEvt_t *)data_p;
    pingtx( ping_evt_p->ping_p );
    return 0;
} /* pintExec */
// called by doPing()
static int pingKeyProc(int c)
{
    struct usock *up;
    extern pingCmdCb_t pingCmdCb;
    /* Ignore all but ^C */
    if(c != CTLC)
        return 0;
    if (pingCmdCb.actSocket == -1 || (up=itop(pingCmdCb.actSocket)) == NULL)
        return 0;
    printk("^C\n");
    malert(up->eventid,1);  /* zc 5/18/95 - change to mb */
    pingCmdCb.actSocket = -1;
    pingCmdCb.flags.active = 0;
    return 0;
} /* pingKeyProc */
#endif



#endif

#endif // TCPHY_DEBUG

/************************************************************************
*            debug functions body for EPHY (TCPHY_TESTDBG CI commands)
**************************************************************************/


#ifdef LINUX_OS 		// allen_20130926
int doDbgPorbeSet(int argc, char *argv[], void *p)
{
	u16 PortSel=0;
	u32 value=0;
	u32 DbgflgSel=0;
	u32 DbgClk=0;
	#define dbg_flg_en 1 
	#define dbg_DevAddr 0x1F
 	#define dbg_Flg0Ctrl 0x15
 	#define dbg_Flg1Ctrl 0x16
 	#define dbg_ClkAddr 0x19
	
   	if(argc==4)
	{
		PortSel = checked_atoi(argv[2]);
		sscanf(argv[3], "%x", &DbgflgSel);
		//DbgClk = checked_atoi(argv[3]);
		printk("* dbg_port_Sel=%d, dbg_flag_sel=0x%04X\r\n", PortSel, DbgflgSel);		
	}
   	else if(argc==5)
   	{
		PortSel = checked_atoi(argv[2]);
		sscanf(argv[3], "%x", &DbgflgSel);
		//DbgClk = checked_atoi(argv[3]);
		sscanf(argv[4], "%x", &DbgClk);
		printk("* dbg_port_Sel=%d, dbg_flag_sel=0x%04X, dbg_clk_sel=0x%02X\r\n", PortSel, DbgflgSel, DbgClk);		
	}

	if(((argc == 4) ||(argc == 5)) && (PortSel <= 31))
	{
		if(argc == 4)
		{
			if(stricmp(argv[1], "all") == 0)
			{
	           value = (PortSel<<13)|(dbg_flg_en<<12)|(DbgflgSel);
			   printk("Dbg Flag Register Set Value=%x\r\n", value);
			   mtEMiiRegWrite(PortSel, dbg_DevAddr, dbg_Flg0Ctrl, value);
			   mtEMiiRegWrite(PortSel, dbg_DevAddr, dbg_Flg1Ctrl, value);
			}
			else if(stricmp(argv[1], "0") == 0)
			{
				value = (PortSel<<13)|(dbg_flg_en<<12)|(DbgflgSel);
				printk("Dbg Flag Register Set Value=%x\r\n", value);
				mtEMiiRegWrite(PortSel, dbg_DevAddr, dbg_Flg0Ctrl, value);
			}
			else if(stricmp(argv[1], "1") == 0)
			{
				value = (PortSel<<13)|(dbg_flg_en<<12)|(DbgflgSel);
				printk("Dbg Flag Register Set Value=%x\r\n", value);
				mtEMiiRegWrite(PortSel, dbg_DevAddr, dbg_Flg1Ctrl, value);
			}
			DbgClk = mtEMiiRegRead(PortSel, dbg_DevAddr, dbg_ClkAddr);
			value = DbgClk & 0xEF;
			mtEMiiRegWrite(PortSel, dbg_DevAddr, dbg_ClkAddr, value);
		}
		else if((argc == 5) && (stricmp(argv[1], "1") == 0))
		{
	    	value = (PortSel<<13)|(dbg_flg_en<<12)|(DbgflgSel);
			mtEMiiRegWrite(PortSel, dbg_DevAddr, dbg_Flg1Ctrl, value);
			value = (dbg_flg_en<<12)|(DbgClk);
			mtEMiiRegWrite(PortSel, dbg_DevAddr, dbg_ClkAddr, value);    
	 	   	printk("Dbg Flag Register Set Value=%x\r\n", value);
		}
		else if((argc == 5) && (stricmp(argv[1], "all") == 0))
		{
	    	value = (PortSel<<13)|(dbg_flg_en<<12)|(DbgflgSel);
	    	printk("Dbg Flag Register Set Value=%x\r\n", value);
    		mtEMiiRegWrite(PortSel, dbg_DevAddr, dbg_Flg0Ctrl, value);
			mtEMiiRegWrite(PortSel, dbg_DevAddr, dbg_Flg1Ctrl, value);
			value = (dbg_flg_en<<12)|(DbgClk);
			mtEMiiRegWrite(PortSel, dbg_DevAddr, dbg_ClkAddr, value);
		}
	}
	else 
	{ 	// error message
		printk("Usage: dbgprb 1 <PortNo> <DbgflgSel> <Clk>\r\n");
		printk("       dbgprb 1 <PortNo> <DbgflgSel> \r\n");
		printk("       dbgprb 0 <PortNo> <DbgflgSel> \r\n");
		printk("       dbgprb all <PortNo> <DbgflgSel> <Clk>\r\n");
		return 0;
	}
	return 0;
}

int doPhyMiiRead_TrDbg(int argc, char *argv[], void *p)
{
    const u16 page_reg=31;
    const u32 Token_Ring_debug_reg=0x52B5;
    const u32 Token_Ring_Control_reg=0x10;
    const u32 Token_Ring_Low_data_reg=0x11;
    const u32 Token_Ring_High_data_reg=0x12;
    u16 phyaddr=0;
    u16 ch_addr=0;
    u32 node_addr=0;
    u32 data_addr=0;
    u32 value=0;
    u32 value_high=0;
    u32 value_low=0;
	int i=0;
	u32 start_addr=0;
	u32 end_addr=0;

    if(argc ==4) 
    {	// tce miir_trdgb <PhyAddr> <TrRegType> <DataAddr>
		phyaddr = checked_atoi(argv[1]);		// 0~31
		sscanf(argv[3], "%x", &data_addr);		// 6 bit HEX
        if(strcasecmp(argv[2], "DSPF") == 0)
        {	// DSP Filter Debug Node
            ch_addr = 0x02;
			node_addr = 0x0D;
	    }
		else if(strcasecmp(argv[2], "PMA") == 0)
		{ 	// PMA Debug Node 
			ch_addr = 0x01;
			node_addr = 0x0F;
		}
		else if(strcasecmp(argv[2], "TR") == 0)
		{ 	// Timing Recovery  Debug Node 
			ch_addr = 0x01;
			node_addr = 0x0D;
		}
		else if(strcasecmp(argv[2], "PCS") == 0)
		{ 	// R1000PCS Debug Node 
			ch_addr = 0x02;
			node_addr = 0x0F;
		}
		else if(strcasecmp(argv[2], "FFE_A") == 0)
		{ 	// FFE Debug Node 
			ch_addr = 0x00;
			node_addr = 0x04;
		}
		else if(strcasecmp(argv[2], "FFE_B") == 0)
		{ 	// FFE Debug Node 
			ch_addr = 0x01;
			node_addr = 0x04;
		}
		else if(strcasecmp(argv[2], "FFE_C") == 0)
		{ 	// FFE Debug Node 
			ch_addr = 0x02;
			node_addr = 0x04;
		}
		else if(strcasecmp(argv[2], "FFE_D") == 0)
		{ 	// FFE Debug Node 
			ch_addr = 0x03;
			node_addr = 0x04;
		}
		else if(strcasecmp(argv[2], "FFE_INDEP") == 0)
		{ 	// FFE Debug Node 
			ch_addr = 0x00;
			node_addr = 0x04;
		}
 		else if(strcasecmp(argv[2], "ECNC_A") == 0)
 		{ 	// EC/NC Debug Node 
			ch_addr = 0x00;
			node_addr = (data_addr&0xC0)>>6; 	// V1.17 for ECNC_0xE0 is per port
			#if debug_flag
			printk("* ECNC_A => Phyaddr=%d,TrRegTyp =%s,node_addr=0x%02X,data_addr=0x%04X ,ch_addr=0x%02X\r\n",phyaddr, argv[2],node_addr,data_addr, ch_addr);
			#endif
		}
 		else if(strcasecmp(argv[2], "ECNC_B") == 0)
 		{ 	// EC/NC Debug Node 
			ch_addr = 0x01;
			node_addr = (data_addr&0xC0)>>6; 	// V1.17 for ECNC_0xE0 is per port
			#if debug_flag
			printk("* ECNC_B => Phyaddr=%d,TrRegTyp =%s,node_addr=0x%02X,data_addr=0x%04X ,ch_addr=0x%02X\r\n",phyaddr, argv[2],node_addr,data_addr, ch_addr);
			#endif	
		}
 		else if(strcasecmp(argv[2], "ECNC_C") == 0)
 		{ 	// EC/NC Debug Node 
			ch_addr = 0x02;
			node_addr = (data_addr&0xC0)>>6; 	// V1.17 for ECNC_0xE0 is per port
			#if debug_flag
			printk("* ECNC_C => Phyaddr=%d,TrRegTyp =%s,node_addr=0x%02X,data_addr=0x%04X ,ch_addr=0x%02X\r\n",phyaddr, argv[2],node_addr,data_addr, ch_addr);
			#endif
		}
 		else if(strcasecmp(argv[2], "ECNC_D") == 0)
 		{ 	// EC/NC Debug Node 
			ch_addr = 0x03;
			node_addr = (data_addr&0xC0)>>6; 	// V1.17 for ECNC_0xE0 is per port
			#if debug_flag
			printk("* ECNC_D => Phyaddr=%d,TrRegTyp =%s,node_addr=0x%02X,data_addr=0x%04X ,ch_addr=0x%02X\r\n",phyaddr, argv[2],node_addr,data_addr, ch_addr);
			#endif
		}
 		else if(strcasecmp(argv[2], "ECNC_INDEP") == 0)
 		{ 	// EC/NC Debug Node 
			ch_addr=0x00;
			node_addr=(data_addr&0xC0)>>6;
			#if debug_flag
			printk("* ECNC_INDEP => Phyaddr=%d,TrRegTyp =%s,node_addr=0x%02X,data_addr=0x%04X ,ch_addr=0x%02X\r\n",phyaddr, argv[2],node_addr,data_addr, ch_addr);
			#endif
		}
		else if(strcasecmp(argv[2], "DFEDC_A") == 0)
		{ 	// DFETail/DC Debug Node 
			ch_addr=0x00;
			node_addr=0x05;
		}
		else if(strcasecmp(argv[2], "DFEDC_B") == 0)
		{ 	// DFETail/DC Debug Node 
			ch_addr=0x01;
			node_addr=0x05;
		}
		else if(strcasecmp(argv[2], "DFEDC_C") == 0)
		{ 	// DFETail/DC Debug Node 
			ch_addr=0x02;
			node_addr=0x05;
		}
		else if(strcasecmp(argv[2], "DFEDC_D") == 0)
		{ 	// DFETail/DC Debug Node 
			ch_addr=0x03;
			node_addr=0x05;
		}
		else if(strcasecmp(argv[2], "DFEDC_INDEP") == 0)
		{ 	// DFETail/DC Debug Node 
			ch_addr=0x00;
			node_addr=0x05;
		}
		else if(strcasecmp(argv[2], "DEC") == 0)
		{ 	// R1000DEC Debug Node 
			ch_addr=0x00; 
			node_addr=0x07;
		}
		else if(strcasecmp(argv[2], "CRC_A") == 0)
		{ 	// R1000CRC Debug Node 
			ch_addr=0x00;
			node_addr=0x06;
		}
		else if(strcasecmp(argv[2], "CRC_B") == 0)
		{ 	// R1000CRC Debug Node 
			ch_addr=0x01;
			node_addr=0x06;
		}
		else if(strcasecmp(argv[2], "CRC_C") == 0)
		{ 	// R1000CRC Debug Node 
			ch_addr=0x02;
			node_addr=0x06;
		}
		else if(strcasecmp(argv[2], "CRC_D") == 0)
		{ 	// R1000CRC Debug Node 
			ch_addr=0x03;
			node_addr=0x06;
		}
		else if(strcasecmp(argv[2], "AN") == 0)
		{ 	// Autoneg Debug Node 
			ch_addr=0x00; 
			node_addr=0x0F;
		}
		else if(strcasecmp(argv[2], "CMI") == 0)
		{ 	// CMI Debug Node 
			ch_addr=0x03; 
			node_addr=0x0F;
		}
		else if(strcasecmp(argv[2], "SUPV") == 0)
		{ 	// SUPV PHY  Debug Node 
			ch_addr=0x00; 
			node_addr=0x0D;
		}
		else
		{
			printk("	miir_trdgb <PhyAddr> <TrRegTyp>(DSPF|PMA|TR|PCS|FFE_x|ECNC_x|DFEDC_x|DEC|CRC_x|AN|CMI|SUPV) <DataAddr>\r\n");
			return 0;
		}
		
		if ((tcMiiStationRead(phyaddr, 0) & SMIREG_PWDN_BIT) != 0) //token ring can't access during power down
		{
			printk("Tokenring invalide !");
			return(0xFFFF);
		}
		data_addr = data_addr&0x3F;
		tcMiiStationWrite(phyaddr, page_reg, Token_Ring_debug_reg);
		tcMiiStationWrite(phyaddr, Token_Ring_Control_reg, (1<<15)|(1<<13)|(ch_addr<<11)|(node_addr<<7)|(data_addr<<1));
		//while(!(tcMiiStationRead(phyaddr, Token_Ring_Control_reg)&0x8000)); 			// data ready
		{
			value_low = tcMiiStationRead(phyaddr, Token_Ring_Low_data_reg);
			value_high = tcMiiStationRead(phyaddr, Token_Ring_High_data_reg);
			value=value_low+((value_high&0x00FF)<<16);
			#if debug_flag
			printk("* %s => Phyaddr=%d,TrRegTyp =%s,node_addr=0x%02X,data_addr=0x%04X ,value=0x%06X\r\n", argv[2], phyaddr, argv[2],node_addr,data_addr, value);
			#endif
		}
		tcMiiStationWrite(phyaddr, page_reg, 0x00);			// V1.11
		return 0;		
	}
	else if (argc==5)
	{	// tce miir_trdgb <PhyAddr> <CHAddr> <NodeAddr> <DataAddr>
        phyaddr = checked_atoi(argv[1]);		// 0~31
   	    ch_addr = checked_atoi(argv[2]);		// 0:A pair  /1:B pair  /2:C pair  /3:D pair 
		sscanf(argv[3], "%x", &node_addr);		// 4 bit HEX
		sscanf(argv[4], "%x", &data_addr);		// 6 bit HEX
		if ((tcMiiStationRead(phyaddr, 0) & SMIREG_PWDN_BIT) != 0) //token ring can't access during power down
		{
			printk("Tokenring invalide !");
			return(0xFFFF);
		}
		tcMiiStationWrite(phyaddr, page_reg, Token_Ring_debug_reg);
		tcMiiStationWrite(phyaddr, Token_Ring_Control_reg, (1<<15)|(1<<13)|(ch_addr<<11)|(node_addr<<7)|(data_addr<<1));
		//while(!(tcMiiStationRead(phyaddr, Token_Ring_Control_reg)&0x8000)); 			// data ready
		{
			value_low = tcMiiStationRead(phyaddr, Token_Ring_Low_data_reg);		
			value_high = tcMiiStationRead(phyaddr, Token_Ring_High_data_reg);			
			value = value_low + ((value_high&0x00FF)<<16);
			#if debug_flag
			printk("* miir_trdgb => Phyaddr=%d, ch_addr=%d,node_addr=0x%02X,data_addr=0x%04X ,value=0x%06X\r\n", phyaddr, ch_addr, node_addr, data_addr, value);
			#endif
		}
		tcMiiStationWrite(phyaddr, page_reg, 0x00);		// V1.11
		return 0;
    }
	else if(argc ==6) 
	{	//miir_trdgb all <PhyAddr> <TrRegTyp> <s_addr> <e_addr>
		phyaddr = checked_atoi(argv[2]);		// 0~31
		sscanf(argv[4], "%x", &start_addr);	// hex
		sscanf(argv[5], "%x", &end_addr);		// hex
		for(i=start_addr; i<=end_addr; i++)
		{
		    data_addr=i;
	        if(stricmp(argv[3], "DSPF") == 0)
	        {	// DSP Filter Debug Node
        	    ch_addr=0x02;
				node_addr=0x0D;
		    }
			else if(stricmp(argv[3], "PMA") == 0)
			{ 	// PMA Debug Node 
					ch_addr=0x01;
					node_addr=0x0F;
			}
			else if(stricmp(argv[3], "TR") == 0)
			{ 	// Timing Recovery  Debug Node 
				ch_addr=0x01;
				node_addr=0x0D;
			}
			else if(stricmp(argv[3], "PCS") == 0)
			{ 	// R1000PCS Debug Node 
				ch_addr=0x02;
				node_addr=0x0F;
			}
			else if(stricmp(argv[3], "FFE_A") == 0)
			{ 	// FFE Debug Node 
				ch_addr=0x00;
				node_addr=0x04;
			}
			else if(stricmp(argv[3], "FFE_B") == 0)
			{ 	// FFE Debug Node 
				ch_addr=0x01;
				node_addr=0x04;
			}
			else if(stricmp(argv[3], "FFE_C") == 0)
			{ 	// FFE Debug Node 
				ch_addr=0x02;
				node_addr=0x04;
			}
			else if(stricmp(argv[3], "FFE_D") == 0)
			{ 	// FFE Debug Node 
				ch_addr=0x03;
				node_addr=0x04;
			}
			else if(stricmp(argv[3], "FFE_INDEP") == 0)
			{ 	// FFE Debug Node 
				ch_addr=0x00;
				node_addr=0x04;
			}
 			else if(stricmp(argv[3], "ECNC_A") == 0)
 			{ 	// EC/NC Debug Node 
				ch_addr=0x00;
				node_addr=(data_addr&0x40)>>6;
			}
 			else if(stricmp(argv[3], "ECNC_B") == 0)
 			{ 	// EC/NC Debug Node 
				ch_addr=0x01;
				node_addr=(data_addr&0x40)>>6;
			}
 			else if(stricmp(argv[3], "ECNC_C") == 0)
 			{ 	// EC/NC Debug Node 
				ch_addr=0x02;
				node_addr=(data_addr&0x40)>>6;
			}
 			else if(stricmp(argv[3], "ECNC_D") == 0)
 			{ 	// EC/NC Debug Node 
				ch_addr=0x03;
				node_addr=(data_addr&0x40)>>6;
			}
 			else if(stricmp(argv[3], "ECNC_INDEP") == 0)
 			{ 	// EC/NC Debug Node 
				ch_addr=0x00;
				node_addr=(data_addr&0xC0)>>6;
			}
			else if(stricmp(argv[3], "DFEDC_A") == 0)
			{ 	// DFETail/DC Debug Node 
				ch_addr=0x00;
				node_addr=0x05;
			}
			else if(stricmp(argv[3], "DFEDC_B") == 0)
			{ 	// DFETail/DC Debug Node 
				ch_addr=0x01;
				node_addr=0x05;
			}
			else if(stricmp(argv[3], "DFEDC_C") == 0)
			{ 	// DFETail/DC Debug Node 
				ch_addr=0x02;
				node_addr=0x05;
			}
			else if(stricmp(argv[3], "DFEDC_D") == 0)
			{ 	// DFETail/DC Debug Node 
				ch_addr=0x03;
				node_addr=0x05;
			}
			else if(stricmp(argv[3], "DFEDC_INDEP") == 0)
			{ 	// DFETail/DC Debug Node 
				ch_addr=0x00;
				node_addr=0x05;
			}
			else if(stricmp(argv[3], "DEC") == 0)
			{ 	// R1000DEC Debug Node 
				ch_addr=0x00; 
				node_addr=0x07;
			}
			else if(stricmp(argv[3], "CRC_A") == 0)
			{ 	// R1000CRC Debug Node 
				ch_addr=0x00;
				node_addr=0x06;
			}
			else if(stricmp(argv[3], "CRC_B") == 0)
			{ 	// R1000CRC Debug Node 
				ch_addr=0x01;
				node_addr=0x06;
			}
			else if(stricmp(argv[3], "CRC_C") == 0)
			{ 	// R1000CRC Debug Node 
				ch_addr=0x02;
				node_addr=0x06;
			}
			else if(stricmp(argv[3], "CRC_D") == 0)
			{ 	// R1000CRC Debug Node 
				ch_addr=0x03;
				node_addr=0x06;
			}
			else if(stricmp(argv[3], "AN") == 0)
			{ 	// Autoneg Debug Node 
				ch_addr=0x00; 
				node_addr=0x0F;
			}
			else if(stricmp(argv[3], "CMI") == 0)
			{ 	// CMI Debug Node 
				ch_addr=0x03; 
				node_addr=0x0F;
			}
			else if(stricmp(argv[3], "SUPV") == 0)
			{ 	// SUPV PHY  Debug Node 
				ch_addr=0x00; 
				node_addr=0x0D;
			}
			else 
			{
				printk("	miir_trdgb all <PhyAddr> <TrRegTyp>(DSPF|PMA|TR|PCS|FFE_x|ECNC_x|DFEDC_x|DEC|CRC_x|AN|CMI|SUPV) <s_addr> <e_addr>\r\n");
				return 0;
			}
			
			if ((tcMiiStationRead(phyaddr, 0) & SMIREG_PWDN_BIT) != 0) //token ring can't access during power down
			{
				printk("Tokenring invalide !");
				return(0xFFFF);
			}
			data_addr=data_addr&0x3F;
			tcMiiStationWrite(phyaddr, page_reg, Token_Ring_debug_reg);
			tcMiiStationWrite(phyaddr, Token_Ring_Control_reg, (1<<15)|(1<<13)|(ch_addr<<11)|(node_addr<<7)|(data_addr<<1));
			//while(!(tcMiiStationRead(phyaddr, Token_Ring_Control_reg)&0x8000)); // data ready
			{
				value_low = tcMiiStationRead(phyaddr, Token_Ring_Low_data_reg);
				value_high = tcMiiStationRead(phyaddr, Token_Ring_High_data_reg);
				value = value_low + ((value_high&0x00FF)<<16);
				#if debug_flag
				printk("* %s => Phyaddr=%d,TrRegTyp =%s,node_addr=0x%02X,data_addr=0x%04X,value=0x%06X\r\n", argv[2], phyaddr, argv[2],node_addr, i, value);
				#endif
			}
		}
		tcMiiStationWrite(phyaddr, page_reg, 0x00);		// V1.11
		return 0;
    }
	else
	{
		#if debug_flag
        printk("       CMD Error : miir_trdgb <PhyAddr> <CHAddr> <NodeAddr> <DataAddr>\r\n");
        printk("                   miir_trdgb <PhyAddr> <TrRegTyp>(DSPF|PMA|TR|PCS|FFE_x|ECNC_x|DFEDC_x|DEC|CRC_x|AN|CMI|SUPV) <DataAddr>\r\n");
        printk("                   miir_trdgb all <PhyAddr> <TrRegTyp>(DSPF|PMA|TR|PCS|FFE_x|ECNC_x|DFEDC_x|DEC|CRC_x|AN|CMI|SUPV) <s_addr> <e_addr>\r\n");
        printk("                   <where x=A/B/C/D/INDEP>\r\n");
		#endif
        return 0;      
	}
}

int doPhyMiiWrite_TrDbg(int argc, char *argv[], void *p)
{
    const u16 page_reg=31;
    const u32 Token_Ring_debug_reg=0x52B5;
    const u32 Token_Ring_Control_reg=0x10;
    const u32 Token_Ring_Low_data_reg=0x11;
    const u32 Token_Ring_High_data_reg=0x12;
    u16 phyaddr=0;
    u16 ch_addr=0;
    u32 node_addr=0;
    u32 data_addr=0;
    u32 value=0;
    u32 value_high=0;
    u32 value_low=0;
	
	if(argc ==5)
	{	// tce miiw_trdgb <PhyAddr> <TrRegType> <DataAddr> <Value>	
		phyaddr = checked_atoi(argv[1]);		// 0~31
		sscanf(argv[3], "%x", &data_addr);		// 6 bit HEX
		if(strcasecmp(argv[2], "DSPF") == 0)
		{	// DSP Filter Debug Node
			ch_addr=0x02;
		   	node_addr=0x0D;
		}
		else if(strcasecmp(argv[2], "PMA") == 0)
		{ 	// PMA Debug Node 
			ch_addr=0x01;
			node_addr=0x0F;
		}
		else if(strcasecmp(argv[2], "TR") == 0)
		{ 	// Timing Recovery  Debug Node 
			ch_addr=0x01;
			node_addr=0x0D;
		}
		else if(strcasecmp(argv[2], "PCS") == 0)
		{ 	// R1000PCS Debug Node 
			ch_addr=0x02;
			node_addr=0x0F;
		}
		else if(strcasecmp(argv[2], "FFE_A") == 0)
		{ 	// FFE Debug Node 
			ch_addr=0x00;
			node_addr=0x04;
		}
		else if(strcasecmp(argv[2], "FFE_B") == 0)
		{ 	// FFE Debug Node 
			ch_addr=0x01;
			node_addr=0x04;
		}
		else if(strcasecmp(argv[2], "FFE_C") == 0)
		{ 	// FFE Debug Node 
			ch_addr=0x02;
			node_addr=0x04;
		}
		else if(strcasecmp(argv[2], "FFE_D") == 0)
		{ 	// FFE Debug Node 
			ch_addr=0x03;
			node_addr=0x04;
		}
		else if(strcasecmp(argv[2], "FFE_INDEP") == 0)
		{ 	// FFE Debug Node 
			ch_addr=0x00;
			node_addr=0x04;
		}
		else if(strcasecmp(argv[2], "ECNC_A") == 0)
		{ 	// EC/NC Debug Node 
			ch_addr=0x00;
			node_addr=(data_addr&0x40)>>6;
		}
		else if(strcasecmp(argv[2], "ECNC_B") == 0)
		{ 	// EC/NC Debug Node 
			ch_addr=0x01;
			node_addr=(data_addr&0x40)>>6;
		}
		else if(strcasecmp(argv[2], "ECNC_C") == 0)
		{ 	// EC/NC Debug Node 
			ch_addr=0x02;
			node_addr=(data_addr&0x40)>>6;
		}
		else if(strcasecmp(argv[2], "ECNC_D") == 0)
		{ 	// EC/NC Debug Node 
			ch_addr=0x03;
			node_addr=(data_addr&0x40)>>6;
		}
		else if(strcasecmp(argv[2], "ECNC_INDEP") == 0)
		{ 	// EC/NC Debug Node 
			ch_addr=0x00;
			node_addr=(data_addr&0xC0)>>6;
		}
		else if(strcasecmp(argv[2], "DFEDC_A") == 0)
		{ 	// DFETail/DC Debug Node 
			ch_addr=0x00;
			node_addr=0x05;
		}
		else if(strcasecmp(argv[2], "DFEDC_B") == 0)
		{ 	// DFETail/DC Debug Node 
			ch_addr=0x01;
			node_addr=0x05;
		}
		else if(strcasecmp(argv[2], "DFEDC_C") == 0)
		{ 	// DFETail/DC Debug Node 
			ch_addr=0x02;
			node_addr=0x05;
		}
		else if(strcasecmp(argv[2], "DFEDC_D") == 0)
		{ 	// DFETail/DC Debug Node 
			ch_addr=0x03;
			node_addr=0x05;
		}
		else if(strcasecmp(argv[2], "DFEDC_INDEP") == 0)
		{ 	// DFETail/DC Debug Node 
			ch_addr=0x00;
			node_addr=0x05;
		}
		else if(strcasecmp(argv[2], "DEC") == 0)
		{ 	// R1000DEC Debug Node 
			ch_addr=0x00; 
			node_addr=0x07;
		}
		else if(strcasecmp(argv[2], "CRC_A") == 0)
		{ 	// R1000CRC Debug Node 
			ch_addr=0x00;
			node_addr=0x06;
		}
		else if(strcasecmp(argv[2], "CRC_B") == 0)
		{ 	// R1000CRC Debug Node 
			ch_addr=0x01;
			node_addr=0x06;
		}
		else if(strcasecmp(argv[2], "CRC_C") == 0)
		{ 	// R1000CRC Debug Node 
			ch_addr=0x02;
			node_addr=0x06;
		}
		else if(strcasecmp(argv[2], "CRC_D") == 0)
		{ 	// R1000CRC Debug Node 
			ch_addr=0x03;
			node_addr=0x06;
		}
		else if(strcasecmp(argv[2], "AN") == 0)
		{ 	// Autoneg Debug Node 
			ch_addr=0x00; 
			node_addr=0x0F;
		}
		else if(strcasecmp(argv[2], "CMI") == 0)
		{ 	// CMI Debug Node 
			ch_addr=0x03; 
			node_addr=0x0F;
		}
		else if(strcasecmp(argv[2], "SUPV") == 0)
		{ 	// SUPV PHY  Debug Node 
			ch_addr=0x00; 
			node_addr=0x0D;
		}
		else 
		{
			printk("	miiw_trdgb <PhyAddr> <TrRegTyp>(DSPF|PMA|TR|PCS|FFE_x|ECNC_x|DFEDC_x|DEC|CRC_x|AN|CMI|SUPV) <DataAddr> <Value>\r\n");
			return 0;
		}
		
		if ((tcMiiStationRead(phyaddr, 0) & SMIREG_PWDN_BIT) != 0) //token ring can't access during power down
		{
			printk("Tokenring invalide !");
			return(0xFFFF);
		}
        data_addr=data_addr&0x3F;
		sscanf(argv[4], "%x", &value);			// 24 bit //HEX
		value_high=(0x00FF0000&value)>>16;
		value_low=(0x0000FFFF&value);

		tcMiiStationWrite(phyaddr, page_reg, Token_Ring_debug_reg);	   
		tcMiiStationWrite(phyaddr, Token_Ring_Low_data_reg, value_low);
		tcMiiStationWrite(phyaddr, Token_Ring_High_data_reg, value_high);
		tcMiiStationWrite(phyaddr, Token_Ring_Control_reg, (1<<15)|(0<<13)|(ch_addr<<11)|(node_addr<<7)|(data_addr<<1));
		//while(!(tcMiiStationRead(phyaddr, Token_Ring_Control_reg)&0x8000)); // data ready
	   	{
			printk("* %s => Phyaddr=%d, ch_addr=%d, node_addr=0x%02X, data_addr=0x%04X , value=0x%06X\r\n",argv[2], phyaddr, ch_addr, node_addr, data_addr, value);
		}
		tcMiiStationWrite(phyaddr, page_reg, 0x00);		// V1.11
		return 0;
	}
	else if (argc==6)
	{
	
        if ((tcMiiStationRead(phyaddr, 0) & SMIREG_PWDN_BIT) != 0) //token ring can't access during power down
	    {
		   printk("Tokenring invalide !");
    	   return(0xFFFF);
     	}
    	// tce miiw_trdgb <PhyAddr> <CHAddr> <NodeAddr> <DataAddr> <Value>
    	phyaddr = checked_atoi(argv[1]);		// 0~31
    	ch_addr = checked_atoi(argv[2]);		// 0:A pair  /1:B pair  /2:C pair  /3:D pair 
    	sscanf(argv[3], "%x", &node_addr);		// 4 bit //HEX
    	sscanf(argv[4], "%x", &data_addr);		// 6 bit //HEX
    	sscanf(argv[5], "%x", &value);			// 24 bit //HEX
        value_high=(0x00FF0000&value)>>16;
        value_low=(0x0000FFFF&value);
        tcMiiStationWrite(phyaddr, page_reg, Token_Ring_debug_reg);	
        tcMiiStationWrite(phyaddr, Token_Ring_Low_data_reg, value_low);
        tcMiiStationWrite(phyaddr, Token_Ring_High_data_reg, value_high);
        tcMiiStationWrite(phyaddr, Token_Ring_Control_reg, (1<<15)|(0<<13)|(ch_addr<<11)|(node_addr<<7)|(data_addr<<1));
        //while(!(tcMiiStationRead(phyaddr, Token_Ring_Control_reg)&0x8000)); // data ready
		{
			printk("* miiw_trdgb ready => Phyaddr=%d, ch_addr=%d, node_addr=0x%02X, data_addr=0x%04X , value=0x%06X\r\n", phyaddr, ch_addr, node_addr, data_addr, value);
		}
		tcMiiStationWrite(phyaddr, page_reg, 0x00);		// V1.11
		return 0;
	}
	else
	{
        printk("       CMD Error : miiw_trdgb <PhyAddr> <CHAddr> <NodeAddr> <DataAddr> <Value>\r\n");
        printk("                   miiw_trdgb <PhyAddr> <TrRegTyp>(DSPF|PMA|TR|PCS|FFE_x|ECNC_x|DFEDC_x|DEC|CRC_x|AN|CMI|SUPV) <DataAddr> <Value>\r\n");
        printk("                   <where x=A/B/C/D/INDEP>\r\n");
        return 0;      
	}
}




int doPhyForceMode(int argc, char *argv[], void *p) 	
{
	u16 phyaddr = 0;
	u32 reg3 = 0, r15_temp = 0;
	
	phyaddr = checked_atoi(argv[1]);
	reg3 = tcMiiStationRead(phyaddr, 3);
	r15_temp = tcMiiStationRead(phyaddr, 15);

	tcMiiStationWrite(phyaddr, 0x0, 0x1040);  // [12]:AN, [6]:MSB_0x10=giga	

	if(argc == 5)  // tce forcemode <phyaddr> <an|force> <10|100|1000> <full|half>
	{
		if(stricmp(argv[2], "an") == 0)
		{
			if ((stricmp(argv[3], "10") == 0)&&(stricmp(argv[4], "full") == 0))
	{
				tcMiiStationWrite(phyaddr, 0x04, 0x0041);
				tcMiiStationWrite(phyaddr, 0x09, 0x0000);
				printk("an 10 full  \r\n");
	}
			else if((stricmp(argv[3], "10") == 0)&&(stricmp(argv[4], "half") == 0))
	{
				tcMiiStationWrite(phyaddr, 0x04, 0x0021);
				tcMiiStationWrite(phyaddr, 0x09, 0x0000);
				printk("an 10 half  \r\n");
	}
			else if((stricmp(argv[3], "100") == 0)&&(stricmp(argv[4], "full") == 0))
	{
				tcMiiStationWrite(phyaddr, 0x04, 0x0101);
				tcMiiStationWrite(phyaddr, 0x09, 0x0000);
				printk("an 100 full  \r\n");
	}
			else if((stricmp(argv[3], "100") == 0)&&(stricmp(argv[4], "half") == 0))
	{
				tcMiiStationWrite(phyaddr, 0x04, 0x0081);
				tcMiiStationWrite(phyaddr, 0x09, 0x0000);
				printk("an 100 half  \r\n");
	}
			else if(stricmp(argv[3], "1000") == 0)
	{
				if(reg3==EPHY_ID_7583)
				{
					if(stricmp(argv[4], "full") == 0)
	{
						tcMiiStationWrite(phyaddr, 0x04, 0x0001);
						tcMiiStationWrite(phyaddr, 0x09, 0x0600);
						printk("an 1000 full  \r\n");
	}
					else if(stricmp(argv[4], "half") == 0)
	{    
						printk("not support 1000 half \r\n");
				 	}		 	
			 	}
				else
					printk("port[%d] not support 1000 mode. \r\n",phyaddr);	
	}

			//tcMiiStationWrite(phyaddr, 0x00, 0x1200);
			printk("back to AN \r\n");
		}
		else if(stricmp(argv[2], "force") == 0)
		{
			if ((stricmp(argv[3], "10") == 0)&&(stricmp(argv[4], "full") == 0))
			{
				tcMiiStationWrite(phyaddr, 0x04, 0x0041);
				tcMiiStationWrite(phyaddr, 0x09, 0x0000);
				printk("force(an) 10 full  \r\n");
			}
			else if((stricmp(argv[3], "10") == 0)&&(stricmp(argv[4], "half") == 0))
			{
				tcMiiStationWrite(phyaddr, 0x00, 0x000);
				printk("force 10 half  \r\n");
			}
			else if((stricmp(argv[3], "100") == 0)&&(stricmp(argv[4], "full") == 0))
			{
				tcMiiStationWrite(phyaddr, 0x04, 0x0101);
				tcMiiStationWrite(phyaddr, 0x09, 0x0000);
				printk("force(an) 100 full  \r\n");
			}
			else if((stricmp(argv[3], "100") == 0)&&(stricmp(argv[4], "half") == 0))
			{
				tcMiiStationWrite(phyaddr, 0x00, 0x2000);
				printk("force 100 half  \r\n");
			}
			else if(stricmp(argv[3], "1000") == 0)
			{
				if(reg3==EPHY_ID_7583)
				{
					if(stricmp(argv[4], "full") == 0)
					{
						tcMiiStationWrite(phyaddr, 0x04, 0x0001);
						tcMiiStationWrite(phyaddr, 0x09, 0x0600);
						printk("force(an) 1000 full  \r\n");
				 	}
					else if(stricmp(argv[4], "half") == 0)
					{
				 		printk("not support 1000 half \r\n");
				 	}		 	
			 	}
				else
					printk("port[%d] not support 1000 mode. \r\n",phyaddr);	
			}
		}
		else
		{
			phyaddr = 0xFFFF;
		}
	}
	else if(argc == 4) // tce forcemode <phyaddr> <an|force> <10|100|1000>
	{
		if(stricmp(argv[2], "an") == 0)
		{
			if(stricmp(argv[3], "10") == 0)
			{
				tcMiiStationWrite(phyaddr, 0x04, 0x0061);
				tcMiiStationWrite(phyaddr, 0x09, 0x0000);
			}
			else if(stricmp(argv[3], "100") == 0)
			{
				tcMiiStationWrite(phyaddr, 0x04, 0x01e1);
				tcMiiStationWrite(phyaddr, 0x09, 0x0000);
			}
			else if(stricmp(argv[3], "1000") == 0)
			{
				tcMiiStationWrite(phyaddr, 0x04, 0x01e1);
				tcMiiStationWrite(phyaddr, 0x09, 0x0600);
			}
			else
			{
				phyaddr = 0xFFFF;
			}
			tcMiiStationWrite(phyaddr, 0x00, 0x1200);
			printk("back to AN \r\n");
		}
		else if(stricmp(argv[2], "force") == 0)
		{
			if(stricmp(argv[3], "10") == 0)
			{
				tcMiiStationWrite(phyaddr, 0x00, 0x100);
				printk("force 10 half  \r\n");
			}
			else if(stricmp(argv[3], "100") == 0)
			{
				tcMiiStationWrite(phyaddr, 0x00, 0x2100);
				printk("force 100 half  \r\n");
			}
			else if((stricmp(argv[3], "1000") == 0)&&(reg3==EPHY_ID_7583))
			{
				//tcMiiStationWrite(phyaddr, 0x00, 0x40);
				printk("not support 1000 half  \r\n");
			}
			else
			{
				phyaddr= 0xFFFF;
			}
		}
		else
		{
			phyaddr= 0xFFFF;
		}
	}
	else if(argc == 3) // tce forcemode <phyaddr> <an|force> 
	{
		if(stricmp(argv[2], "an") == 0)
		{
			if((reg3==EPHY_ID_7583))
			{
				tcMiiStationWrite(phyaddr, 0x04, 0x01e1);
				tcMiiStationWrite(phyaddr, 0x09, 0x0600);
			}

			//tcMiiStationWrite(phyaddr, 0x00, 0x1200);
			printk("back to AN  \r\n");
		}
			else
			{
			phyaddr= 0xFFFF;
			}
		}
			else
			{
			phyaddr= 0xFFFF;
		}
	if(phyaddr == 0xFFFF)
		printk("usage: tce forcemode <phyaddr> <an|force> <10|100|1000> <full|half> \r\n");
		
	return 0;
}


int doPhyBootloaderMode(int argc, char *argv[], void *p) 	
{
	u16 phyaddr = 0;
	//u32 reg3 = 0, r15_temp = 0;
	
	phyaddr = checked_atoi(argv[1]);

	if(argc == 3) // tce bootloadermode <phyaddr> <on|off> 
	{
		if(stricmp(argv[2], "on") == 0)
		{
			tcMiiStationWrite(phyaddr, 0x9, 0x0000);        // disable Giga
	        mtEMiiRegWrite(phyaddr, 0x7, 0x003c, 0x0000);   // disable EEE
			printk("force bootloader mode on  \r\n");
		}
		else if(stricmp(argv[2], "off") == 0)
		{
			tcMiiStationWrite(phyaddr, 0x9, 0x0600);        // enable Giga
	        mtEMiiRegWrite(phyaddr, 0x7, 0x003c, 0x0006);   // enable EEE
			printk("force bootloader mode off  \r\n");
		}
		else
		{
			phyaddr= 0xFFFF;
		}
	}
	else
	{
		phyaddr= 0xFFFF;
	}
	if(phyaddr == 0xFFFF)
	{
		printk("usage: tce bootloadermode <phyaddr> <on|off>  \r\n");
	}	
	return 0;
}

void doEphySetSLTMode(u8 phyaddr, u16 speed_mode)
{
	//u32 reg3 = 0, r15_temp = 0;
/*			
    u16 ch_addr = 0;
    u32 node_addr = 0;
    const u16 page_reg = 31;
    const u32 Token_Ring_debug_reg = 0x52B5;
    const u32 Token_Ring_Control_reg = 0x10;
    const u32 Token_Ring_Low_data_reg = 0x11;
    const u32 Token_Ring_High_data_reg = 0x12;

    u32 data_addr = 0;
    u32 value = 0;
    u32 value_high = 0;
    u32 value_low = 0;
*/
    sw_down_shift_en=0;slt_excuting=1;inital_finish_flag = 1;

    	
	
    if(speed_mode == 10)
	{
        tcMiiStationWrite(phyaddr, 0x1f, 0x0000);
		tcMiiStationWrite(phyaddr, 0x09, 0x0000);
		tcMiiStationWrite(phyaddr, 0x04, 0x0c41);		
		tcMiiStationWrite(phyaddr, 0x00, 0x1240);
		printk(" phy %d 10M\r\n",phyaddr);
	}
	else if(speed_mode == 100)
	{
		tcMiiStationWrite(phyaddr, 0x1f, 0x0000);
		tcMiiStationWrite(phyaddr, 0x09, 0x0000);
		tcMiiStationWrite(phyaddr, 0x04, 0x0d01);
		tcMiiStationWrite(phyaddr, 0x00, 0x1240);
		printk(" phy %d 100M\r\n",phyaddr);
		
	}
	else if(speed_mode == 1000)
	{	
		tcMiiStationWrite(phyaddr, 0x1f, 0x0001);
		tcMiiStationWrite(phyaddr, 0x14, 0x3A04);
 /*
		tcMiiStationWrite(phyaddr, 0x1f, 0x0000);
		tcMiiStationWrite(phyaddr, 0x18, 0x0001);
		//printk(" reg24 = 0x%x \n", tcMiiStationRead(phyaddr, 0x18));
		
		ch_addr = 0x02;
		node_addr = 0x0F;
		data_addr = 1; 					// PCS_1
		//data_addr = data_addr&0x3F;
		// sscanf(argv[4], "%x", &value);			// 24 bit //HEX
		value = 0x28001;
		value_high = (0x00FF0000&value)>>16;
		value_low = (0x0000FFFF&value);
		tcMiiStationWrite(phyaddr, page_reg, Token_Ring_debug_reg);	   
		tcMiiStationWrite(phyaddr, Token_Ring_Low_data_reg, value_low);
		tcMiiStationWrite(phyaddr, Token_Ring_High_data_reg, value_high);
		tcMiiStationWrite(phyaddr, Token_Ring_Control_reg, (1<<15)|(0<<13)|(ch_addr<<11)|(node_addr<<7)|(data_addr<<1));
*/
		tcMiiStationWrite(phyaddr, 0x1f, 0x0000);
//		tcMiiStationWrite(phyaddr, 0x09, 0x1f00);
  		tcMiiStationWrite(phyaddr, 0x09, 0x0600);
		tcMiiStationWrite(phyaddr, 0x00, 0x1200);
		printk(" phy %d 1G\r\n",phyaddr);
	}
}	

void doEphySetSETMode(u8 set_mode, u16 parameter)    // allen_20190709
{
	u16	phyaddr=0;
	
	if(set_mode == 0)				// set LED 
	{
		mtEMiiRegWrite(12, 0x1f, 0x21, 0x800a);
		mtEMiiRegWrite(12, 0x1f, 0x25, 0x003f);
		mtEMiiRegWrite(12, 0x1f, 0x27, 0x003f);
	
		if(parameter == 0) 			// set for LED0 on/active, LED1 off
 		{
 			mtEMiiRegWrite(12, 0x1f, 0x24, 0xc007);
 			mtEMiiRegWrite(12, 0x1f, 0x26, 0x4007);
		}
		else if(parameter == 1)		// set for LED0 off, LED1 on/active
		{
			mtEMiiRegWrite(12, 0x1f, 0x24, 0x4007);
 			mtEMiiRegWrite(12, 0x1f, 0x26, 0xc007);
		}
		else if(parameter == 2)		// set for default, LED0/1 on/active
		{
			mtEMiiRegWrite(12, 0x1f, 0x24, 0xc007);
 			mtEMiiRegWrite(12, 0x1f, 0x26, 0xc007);
		}
	}
	else if(set_mode == 1)	// set Power Saving
	{
		
		// global register
		if(parameter == 0)	// default
		{
			mtEMiiRegWrite(9, 0x1f, 0x271, 0x7fff);
			mtEMiiRegWrite(9, 0x1f, 0x269, 0x444f);
		}
		else if(parameter == 1)	// for 110m
		{
			mtEMiiRegWrite(9, 0x1f, 0x271, 0x7e0e);
			//mtEMiiRegWrite(9, 0x1f, 0x272, 0x14ff);
			mtEMiiRegWrite(9, 0x1f, 0x269, 0x111f);
		}
		else if(parameter == 2)	
		{
			mtEMiiRegWrite(9, 0x1f, 0x271, 0x7c63);
			//mtEMiiRegWrite(9, 0x1f, 0x272, 0x14ff);
			mtEMiiRegWrite(9, 0x1f, 0x269, 0x000f);
		}
		else if(parameter == 3)	
		{
			mtEMiiRegWrite(9, 0x1f, 0x271, 0x7c42);
			//mtEMiiRegWrite(9, 0x1f, 0x272, 0x14ff);
			mtEMiiRegWrite(9, 0x1f, 0x269, 0x000f);
		}
	
		// local register
		for(phyaddr = 9; phyaddr<=12; phyaddr++)
		{
			tcMiiStationWrite(phyaddr, 0x0, 0x0800);
			tcMiiStationWrite(phyaddr, 0x9, 0x0600);
		
			if(parameter == 0)	// default
			{
				tcMiiStationWrite(phyaddr, 0x17, 0x00a0);
				mtEMiiRegWrite(phyaddr, 0x1e, 0x3e, 0xc000);
				mtEMiiRegWrite(phyaddr, 0x1e, 0x5c, 0x7777);
			}
			else if(parameter == 3)	
			{
				tcMiiStationWrite(phyaddr, 0x17, 0x00b0);
				mtEMiiRegWrite(phyaddr, 0x1e, 0x3e, 0x0000);
				mtEMiiRegWrite(phyaddr, 0x1e, 0x5c, 0x4444);
			}
			else						// for 110m
			{
				tcMiiStationWrite(phyaddr, 0x17, 0x00b0);
				mtEMiiRegWrite(phyaddr, 0x1e, 0x3e, 0x0000);
				mtEMiiRegWrite(phyaddr, 0x1e, 0x5c, 0x7777);
			}
			udelay(100);
			//tcMiiStationWrite(phyaddr, 0x0, 0x1200);
		}
	}
}


u32 getPhyPortMask(void)
{
//	if(isMT7520S)
//		return 0x1000;		// port12
//	else if( isMT7520 || isMT7525)
//		return 0x1e00;		// port9-12
//	else if(isMT7525G || isMT7520G)
//		return 0xf;			// port0-3
//	else
//	{
		printk("not found phy port\n");
		return 0;
//	}
}

#define PHY_EEE_ON 					1
#define PHY_EEE_OFF 				2
#define PHY_EEE_RECOVER 			3
void phyForceEEEAllOnOffRecover(int mode)
{
	int port_num;
	u32 reg3;
	u32 phy_port_mask = getPhyPortMask();
	for(port_num= 0 ; port_num<32; port_num++)
	{
		if(!(phy_port_mask & (1<<port_num)))
			continue;		
		reg3 = tcMiiStationRead(port_num, 3); 
		switch(mode)
		{
			case PHY_EEE_ON:
				if((reg3 == 0x9421)||(reg3 == 0x9412))
				{
					mtEMiiRegWrite(port_num, 0x7, 0x003c, 0x0006);
				}
				else
				{
					tcMiiStationWrite(port_num, 31, 0xb000); //  l3
					tcMiiStationWrite(port_num, 17, 0x0002); //  17 2 : enable FE EEE
				}
				printk("force port[%d] EEE on \r\n", port_num);					
				break;
			case PHY_EEE_OFF:
				if((reg3 == 0x9421)||(reg3 == 0x9412))
				{
					mtEMiiRegWrite(port_num, 0x7, 0x003c, 0x0000);
				}
				else
				{
					tcMiiStationWrite(port_num, 31, 0xb000); //  l3
					tcMiiStationWrite(port_num, 17, 0x0000); //  17 2 : enable FE EEE
				}
				printk("force port[%d] EEE off \r\n", port_num);
				break;
			case PHY_EEE_RECOVER:
				if((reg3 == 0x9421)||(reg3 == 0x9412))
				{
					mtEMiiRegWrite(port_num, 0x7, 0x003c, default_ge_dev7r3c);
				}
				else
				{
					tcMiiStationWrite(port_num, 31, 0xb000); //  l3
					tcMiiStationWrite(port_num, 17, default_fe_l3r17); //  17 2 : enable FE EEE
				}
				printk("recover port[%d] EEE to default \r\n", port_num);
				break;
			default:
				printk("not support mode %d",mode);
				break;
		}
	}
}
//static spinlock_t Phy_EEE_lock;
int doPhyForceEEE (int argc, char *argv[], void *p) 
{
	u16 phyaddr = 0;
	u32 reg3 = 0, r15_temp = 0;
	//spin_lock_bh(&Phy_EEE_lock);
	if(!strcmp(argv[1],"all"))
	{
		phyaddr = 0xffff;
	}
	else
	{
		phyaddr = checked_atoi(argv[1]);
		reg3 = tcMiiStationRead(phyaddr, 3); 
	}
	r15_temp = tcMiiStationRead(phyaddr, 0xf);
	if(argc == 3) 
	{
		if((stricmp(argv[2], "on") == 0))
		{
			if(phyaddr == 0xffff)
			{
				phyForceEEEAllOnOffRecover(PHY_EEE_ON);
				//spin_unlock_bh(&Phy_EEE_lock);
				return 0;
			}
			
			if((reg3 == EPHY_ID_7583))
			{
				mtEMiiRegWrite(phyaddr, 0x7, 0x003c, 0x0006);
			}
			else
			{
				tcMiiStationWrite(phyaddr, 31, 0xb000); //  l3
				tcMiiStationWrite(phyaddr, 17, 0x0002); //  17 2 : enable FE EEE
			}
			printk("force port[%d] EEE on \r\n", phyaddr);
		}
		else if((stricmp(argv[2], "off") == 0))
		{
		
			if(phyaddr == 0xffff)
			{
				phyForceEEEAllOnOffRecover(PHY_EEE_OFF);
				//spin_unlock_bh(&Phy_EEE_lock);
				return 0;
			}

			if((reg3 == EPHY_ID_7583))
			{
				mtEMiiRegWrite(phyaddr, 0x7, 0x003c, 0x0000);
			}
			else
			{
				tcMiiStationWrite(phyaddr, 31, 0xb000); //  l3
				tcMiiStationWrite(phyaddr, 17, 0x0000); //  17 2 : enable FE EEE
			}
			printk("force port[%d] EEE off \r\n", phyaddr);
		}
		else if((stricmp(argv[2], "recover") == 0))
		{
		
			if(phyaddr == 0xffff){
				phyForceEEEAllOnOffRecover(PHY_EEE_RECOVER);
				//spin_unlock_bh(&Phy_EEE_lock);
				return 0;
			}

			if((reg3==EPHY_ID_7583))
			{
				mtEMiiRegWrite(phyaddr, 0x7, 0x003c, default_ge_dev7r3c);
			}
			else
			{
				tcMiiStationWrite(phyaddr, 31, 0xb000); //  l3
				tcMiiStationWrite(phyaddr, 17, default_fe_l3r17); //  17 2 : enable FE EEE
			}
			printk("recover port[%d] EEE to default \r\n", phyaddr);
		}
		else
		{
			phyaddr = 0xFFFF;
		}
	}
	else
	{
		phyaddr = 0xFFFF;
	}
	if(phyaddr == 0xFFFF)
		printk("usage: tce forceEEE <phyaddr> <on|off|recover> \r\n");
    //spin_unlock_bh(&Phy_EEE_lock);
	return 0;
}

int doPhyForceGE (int argc, char *argv[], void *p) // allen_20131120
{
	u16 phyaddr = 0;
	u32 reg3 = 0, r15_temp = 0;
	
	phyaddr = checked_atoi(argv[1]);
	reg3 = tcMiiStationRead(phyaddr, 3); 
	r15_temp = tcMiiStationRead(phyaddr, 0xf);
	
	if((reg3 == EPHY_ID_7583))
	{
		if (argc == 4) 
		{
			if ((stricmp(argv[2], "multi") == 0))
			{
				if ((stricmp(argv[3], "master") == 0))
				{
					tcMiiStationWrite(phyaddr, 0x9, 0x1e00);
					printk("force port[%d] multi master on \r\n", phyaddr);
				}
				else if((stricmp(argv[3], "slave") == 0))
				{
					tcMiiStationWrite(phyaddr, 0x9, 0x1600);
					printk("force port[%d] multi slave on \r\n", phyaddr);
				}
			}
			else if((stricmp(argv[2], "single") == 0))
			{
				if ((stricmp(argv[3], "master") == 0))
				{
					tcMiiStationWrite(phyaddr, 0x9, 0x1a00);
					printk("force port[%d] single master on \r\n", phyaddr);
				}
				else if((stricmp(argv[3], "slave") == 0))
				{
					tcMiiStationWrite(phyaddr, 0x9, 0x1400);
					printk("force port[%d] single slave on \r\n", phyaddr);
				}
			}
			else
			{
				phyaddr = 0xFFFF;
			}			
		}
		else if(argc == 3) 
		{
			if((stricmp(argv[2], "multi") == 0))
			{
				tcMiiStationWrite(phyaddr, 0x9, 0x0600);
				printk("force port[%d] multi on \r\n", phyaddr);
			}
			else if((stricmp(argv[2], "single") == 0))
			{
				tcMiiStationWrite(phyaddr, 0x9, 0x0400);
				printk("force port[%d] single on \r\n", phyaddr);
			}
			else
			{
				phyaddr = 0xFFFF;
			}
		}
		else
		{
			phyaddr = 0xFFFF;
		}
	}
	else
	{
		phyaddr = 0xFFFF;
	}
	if(phyaddr == 0xFFFF)
		printk("usage: tce forceGE <phyaddr> <multi|single> <master|slave>\r\n");

	return 0;
}

int doPhyForceReset(int argc, char *argv[], void *p) 	// allen_20131120
{
	u16 phyaddr = 0;
	u32 reg3, r15_temp;

	phyaddr = checked_atoi(argv[1]);
	reg3 = tcMiiStationRead(phyaddr, 3); 
	r15_temp = tcMiiStationRead(phyaddr, 15); 
	
    if(reg3==EPHY_ID_7583)
	{
		tcPhyVer = tcPhyVer_7583;
	}
	
	
	if (argc == 2) 
	{
		tcMiiStationWrite(phyaddr, 0x0, 0x0800);
		pause(1000);
		tcMiiStationWrite(phyaddr, 0x0, 0x1040);
		printk("port[%d] reset \r\n", phyaddr);

		switch (tcPhyVer) 
		{  
			case tcPhyVer_7583:
				en75xxGePhyCfgLoad(0);			
			break;
		}
	}
	else
	{
		phyaddr = 0xFFFF;
	}
	if(phyaddr == 0xFFFF)
		printk("usage: tce forceReset <phyaddr> \r\n");
	return 0;
}

int doPhyrestartAN(int argc, char *argv[], void *p) 	// allen_20131213
{
	u16 phyaddr = 0;
	u32 reg3, reg0;

	phyaddr = checked_atoi(argv[1]);
	reg3 = tcMiiStationRead(phyaddr, 3); 
	reg0 = tcMiiStationRead(phyaddr, 0);
	reg0 = reg0 | 0x1200;
	if(argc == 2) 
	{
		tcMiiStationWrite(phyaddr, 0x0, reg0);
		printk("phyaddr[%d] restart AN \r\n", phyaddr);		
	}
	else
	{
		phyaddr = 0xFFFF;
	}

	if(phyaddr == 0xFFFF)
		printk("usage: tce restartAN <phyaddr> \r\n");
	return 0;
}

int doPhyChkVal(int argc, char *argv[], void *p)
{	// for tcPhyChkVal	
    unsigned int phychkval_portnum;
	phychkval_flag = 1;
	//printk(" [debug 0] (%d) \r\n", tcPhyVer);	
   	if(argc == 2) 
	{
				phychkval_portnum =(unsigned int) checked_atoi(argv[1]);
				if (phychkval_portnum !=0xffff)
				  tcPhyChkVal(phychkval_portnum);

    }
    return 0;
}

int doPhyChkErrFlag(int argc, char *argv[], void *p)
{
	unsigned int phychkerrflag_portnum;
   	if(argc == 2) 
	{ 	// set phy for tcPhyChkVal()
		phychkerrflag_portnum = checked_atoi(argv[1]);
	}
	else
	{
		printk("Too many element, Chk error flag command is tce err_flag 9/10/11/12 \r\n");
		return 0;
	}

	switch (phychkerrflag_portnum)
	{
		case 9:
			printk("Phy addr = %d, Downshift debug flag = 0x%x \r\n", phychkerrflag_portnum, workaround_flag_p9);
			printk("Phy addr = %d, Linkdwown debug flag = 0x%x \r\n", phychkerrflag_portnum, workaround_linkdown_flag_p9);
		break;

		case 10:
			printk("Phy addr = %d, Downshift debug flag = 0x%x \r\n", phychkerrflag_portnum, workaround_flag_p10);
			printk("Phy addr = %d, Linkdwown debug flag = 0x%x \r\n", phychkerrflag_portnum, workaround_linkdown_flag_p10);
		break;

		case 11:
			printk("Phy addr = %d, Downshift debug flag = 0x%x  \r\n", phychkerrflag_portnum, workaround_flag_p11);
			printk("Phy addr = %d, Linkdwown debug flag = 0x%x \r\n", phychkerrflag_portnum, workaround_linkdown_flag_p11);
		break;

		case 12:
			printk("Phy addr = %d, Downshift debug flag = 0x%x  \r\n", phychkerrflag_portnum, workaround_flag_p12);
			printk("Phy addr = %d, Linkdwown debug flag = 0x%x \r\n", phychkerrflag_portnum, workaround_linkdown_flag_p12);
		break;
	}
	
    return 0;
}

int doPhyClrErrFlag(int argc, char *argv[], void *p)
{
	unsigned int phychkerrflag_portnum;
   	if(argc == 2) 
	{ 	// set phy for tcPhyChkVal()
		phychkerrflag_portnum = checked_atoi(argv[1]);
	}
	else
	{
		printk("Too many element, Clear error flag command is tce clr_err_flag 9/10/11/12 \r\n");
		return 0;
	}

	switch (phychkerrflag_portnum)
	{
		case 9:
			workaround_flag_p9 = 0;
			printk("Clear Phyaddr %d workaround error flag\r\n", phychkerrflag_portnum);
		break;

		case 10:
			workaround_flag_p10 = 0;
			printk("Clear Phyaddr %d workaround error flag\r\n", phychkerrflag_portnum);
		break;

		case 11:
			workaround_flag_p11 = 0;
			printk("Clear Phyaddr %d workaround error flag\r\n", phychkerrflag_portnum);
		break;

		case 12:
			workaround_flag_p12 = 0;
			printk("Clear Phyaddr %d workaround error flag\r\n", phychkerrflag_portnum);
		break;
	}
	
    return 0;
}

int getAllPortStatus (int argc, char *argv[], void *p)
{	
	u16 loop,reg_04,reg_0a,reg_1d,dev7_reg3c,reg_01,status_temp;	

     
		//set all port         
	for(loop=all_port_start;loop <= all_port_end;loop++)	    
	{			   
		reg_01=tcMiiStationRead(loop, 1);		   
		reg_04=tcMiiStationRead(loop, 4);		   
		reg_0a=tcMiiStationRead(loop, 0x0a);				   
		reg_1d=tcMiiStationRead(loop, 0x1d);			   
		dev7_reg3c=mtEMiiRegRead(loop, 0x7, 0x3c);		   
		printk("port%d  \n\r",loop);		   
		reg_01=reg_01&0x0004;		   
	if(reg_01 !=0)		    
		{		      
			printk("link status=link up");		      
			status_temp=reg_1d&0x0018;		      
			reg_0a=reg_0a&0x4000;	          
			if(status_temp ==0x0000) { printk("  link speed=10M");}			      
			if(status_temp ==0x0008) { printk("  link speed=100M");}			      
			if(status_temp ==0x0010) { if(reg_0a ==0x4000) {printk("  link speed=1G Master");} else {printk("  link speed=1G Salve");}}		      
			status_temp=reg_1d&0x0020;		      
			if(status_temp ==0x0000) { printk("  Half_duplex");} else {printk("  Full_duplex");}		      
			reg_04=reg_04&0x0c00;		      
			if(reg_04 ==0x0000) {printk("  flowcontrol=disable");}		      
			if(reg_04 ==0x0c00) {printk("  flowcontrol=enable");}				      
			dev7_reg3c=dev7_reg3c&0x0006;		      
			if(dev7_reg3c ==0x0006) {printk("  EEE ability=100M and 1G\n\r");}		      
			if(dev7_reg3c ==0x0002) {printk("  EEE ability=only 100M\n\r");}		      
			if(dev7_reg3c ==0x0004) {printk("  EEE ability=only 1G\n\r");}		      
			if(dev7_reg3c ==0x0000) {printk("  EEE ability=disable\n\r");}				    
		}			   
	else		    
		{		      
			printk("link down \n\r");					    
		}	    
	}	
	printk("\n\r");
    return 0;
}

int settingFlowControl (int argc, char *argv[], void *p)
{	
	u16 phyaddr = 0,loop,reg_04,control;	
	if(argc !=3) 	
	{            
		printk(" flowControl <phyaddr> <on|off>\r\n");		  
		printk("phy=32 is all port\r\n");	
	}	
	else 	
	{	
		phyaddr = checked_atoi(argv[1]);        
		control = checked_atoi(argv[2]);	        
	if(phyaddr==32)	   
	{       //set all port         
	for(loop=all_port_start;loop <= all_port_end;loop++)	      
	{			    
		reg_04=tcMiiStationRead(loop, 4);		    
		if(control==1) {reg_04=reg_04|0x0c00;} 		    
		if(control==0) {reg_04=reg_04&0xf3ff;} 		    
		tcMiiStationWrite(loop, 0x04, reg_04);	      
		tcMiiStationWrite(loop, 0x00, 0x1240);	      
	}		
		if(control==1) { printk("all flowControl on\r\n");}		
		if(control==0) { printk("all flowControl off\r\n");}	   
	}	
	else	 
	{ 		
		reg_04=tcMiiStationRead(phyaddr, 4);		
		if(control==1) {reg_04=reg_04|0x0c00;}		
		if(control==0) {reg_04=reg_04&0xf3ff;}		
		tcMiiStationWrite(phyaddr, 0x04, reg_04); 		
		tcMiiStationWrite(phyaddr, 0x00, 0x1240);	      
		if(control==1) { printk("prot%d flowControl on\r\n",phyaddr);}		
		if(control==0) { printk("port%d flowControl off\r\n",phyaddr);}			 
	}
     printk("\n\r");	
	}
   return 0; 
}

int settingloopback (int argc, char *argv[], void *p)
{	
	u16 phyaddr = 0,loop,reg_00,mode,page1_reg1d,control;	
	if(argc !=4) 	
	{            
		printk("tce setLoopback <phyaddr> <mode> <on|off>\r\n");		  
		printk("phy=32 is all port\r\n");	
	}	
	else 	
	{	    
		phyaddr = checked_atoi(argv[1]);        
		mode = checked_atoi(argv[2]);        
		control = checked_atoi(argv[3]);	  
	if(phyaddr==32)	   
	{          //set all port         
	for(loop=all_port_start;loop <= all_port_end;loop++)	      
	{            
		if(mode==1) 
			{ 		    
				tcMiiStationWrite(loop, 0x1f, 0x0001);
				page1_reg1d=tcMiiStationRead(loop, 0x1a);	        
				if(control==1) {page1_reg1d=page1_reg1d|0x8000;printk("port %d MDI Loopback on\r\n",loop);} 		    
				if(control==0) {page1_reg1d=page1_reg1d&0x7fff;printk("port %d MDI Loopback off\r\n",loop);	}		        
				tcMiiStationWrite(loop, 0x1a, page1_reg1d);
				tcMiiStationWrite(loop, 0x1f, 0x0000);     
			}
		if(mode==0)		
			{ 		
				tcMiiStationWrite(loop, 0x1f, 0x0000);		    
				reg_00=tcMiiStationRead(loop, 0);	        
				if(control==1) {reg_00=reg_00|0x4000;printk("port %d Loopback on\r\n",loop);}		
				if(control==0) {reg_00=reg_00&0xbfff;printk("port %d Loopback off\r\n",loop);}		
				tcMiiStationWrite(loop, 0, reg_00);         
			}		
	}	   
	}	   
	else		   
	{         
		if(mode==1) 		 
		{   	  	 		
		    tcMiiStationWrite(phyaddr, 0x1f, 0x0001);         
		    page1_reg1d=tcMiiStationRead(phyaddr, 0x1a);	     
			if(control==1) {page1_reg1d=page1_reg1d|0x8000;printk("port %d MDI Loopback on\r\n",phyaddr);} 		 
			if(control==0) {page1_reg1d=page1_reg1d&0x7fff;printk("port %d MDI Loopback off\r\n",phyaddr);}		     
			tcMiiStationWrite(phyaddr, 0x1a, page1_reg1d);
			tcMiiStationWrite(phyaddr, 0x1f, 0x0000);			    
		}				 
		if(mode==0)		 
		{ 		  
		    tcMiiStationWrite(phyaddr, 0x1f, 0x0000);         
			reg_00=tcMiiStationRead(phyaddr, 0);	     
			if(control==1) {reg_00=reg_00|0x4000;printk("port %d Loopback on\r\n",phyaddr);}		  
			if(control==0) {reg_00=reg_00&0xbfff;printk("port %d Loopback off\r\n",phyaddr);}		  
  		    tcMiiStationWrite(phyaddr, 0, reg_00);          	   
		}		  
	}
        printk("\n\r");   	
	}	
    return 0;
}


static spinlock_t anspeed_lock;
int settingANSpeed (int argc, char *argv[], void *p)
{	
	u16 phyaddr = 0,loop,speed,duplex,reg_09,reg_04;	
	if(argc !=4) 	
	{            
		printk("tce setANSpeed <phyaddr> <speed> <duplex>\r\n");		  
		printk("phy=32 is all port      1G half is set all speed\r\n");	
	}	
	else 	
	{	   
		spin_lock_bh(&anspeed_lock);
		phyaddr = checked_atoi(argv[1]);        
		speed = checked_atoi(argv[2]);        
		duplex = checked_atoi(argv[3]);
		if(phyaddr==32)	
		{         
			//set all port
		for(loop=all_port_start;loop <= all_port_end;loop++)
		{
			reg_09=tcMiiStationRead(loop, 9);
			reg_04=tcMiiStationRead(loop, 4);
			if(speed == 1000 )
			{		  
				if(duplex == 1 )		
				{					  
					reg_09=reg_09|0x0200; 	  
					tcMiiStationWrite(loop, 9, reg_09);		  
					reg_04=reg_04&0xfe1f; 	  
					tcMiiStationWrite(loop, 4, reg_04);		   
					tcMiiStationWrite(loop, 0, 0x1340);		  
					printk("port%d 1G full duplex set\r\n",loop); 	
				}		
				else		
				{		   
					reg_09=reg_09|0x0200;		  
					tcMiiStationWrite(loop, 9, reg_09);		  
					reg_04=reg_04|0x01e0; 	  
					tcMiiStationWrite(loop, 4, reg_04);		   
					tcMiiStationWrite(loop, 0, 0x1340);		  
					printk("port%d all speed set \r\n",loop); 	
				}				  
			} 
			
			if(speed == 100 )			
			{			 
				reg_09=reg_09&0xfdff;		   
				tcMiiStationWrite(loop, 9, reg_09);
				
				if(duplex == 1 ) 
				{ 
					reg_04=reg_04&0xff1f;reg_04=reg_04|0x0100;printk("port%d 100M full duplex set\r\n",loop);
				} 
				else 
				{ 
					reg_04=reg_04&0xfe9f;reg_04=reg_04|0x0080;printk("port%d 100M Half duplex set\r\n",loop);
				} 	   
				tcMiiStationWrite(loop, 4, reg_04);		   
				tcMiiStationWrite(loop, 0, 0x1340);			  
			} 	
			
			if(speed == 10 )			  
			{ 		   
				reg_09=reg_09&0xfdff;		   
				tcMiiStationWrite(loop, 9, reg_09);	
				
				if(duplex == 1 ) 
				{ 
					reg_04=reg_04&0xfe5f;reg_04=reg_04|0x0040;printk("port%d 10M full duplex set\r\n",loop);
				} 
				else 
				{ 
					reg_04=reg_04&0xfe3f;reg_04=reg_04|0x0020;printk("port%d 10M Half duplex set\r\n",loop);
				}		   
				tcMiiStationWrite(loop, 4, reg_04);		   
				tcMiiStationWrite(loop, 0, 0x1340);			  
			} 		   
		}	
		}		
		else	
		{			  
			reg_09=tcMiiStationRead(phyaddr, 9);			   
			reg_04=tcMiiStationRead(phyaddr, 4); 		 
			switch (speed) 			 
			{					 
				case 1000: 			
				if(duplex == 1 )			 
				{				 
					reg_09=reg_09|0x0200;			   
					tcMiiStationWrite(phyaddr, 9, reg_09);			   
					reg_04=reg_04&0xfe1f;			   
					tcMiiStationWrite(phyaddr, 4, reg_04);			   
					tcMiiStationWrite(phyaddr, 0, 0x1340);				
					printk("port%d 1G full duplex set\r\n",phyaddr);						 
				}			
				else			 
				{				 
					reg_09=reg_09|0x0200;			   
					tcMiiStationWrite(phyaddr, 9, reg_09);			   
					reg_04=reg_04|0x01e0;			   
					tcMiiStationWrite(phyaddr, 4, reg_04);			   
					tcMiiStationWrite(phyaddr, 0, 0x1340);				
					printk("port%d 1G all speed set\r\n",phyaddr);						 
				}				  
				break;
				
				case 100:				  
					reg_09=reg_09&0xfdff; 		   
					tcMiiStationWrite(phyaddr, 9, reg_09);
					
				if(duplex == 1 ) 
				{ 
					reg_04=reg_04&0xff1f;reg_04=reg_04|0x0100;
					printk("port%d 100M full duplex set\r\n",phyaddr);
				} 
				else 
				{ 
					reg_04=reg_04&0xfe9f;reg_04=reg_04|0x0080;
					printk("port%d 100M half duplex set\r\n",phyaddr);
				}			   
					tcMiiStationWrite(phyaddr, 4, reg_04);			   
					tcMiiStationWrite(phyaddr, 0, 0x1340);						
				break;
				
				case 10:				 
					reg_09=reg_09&0xfdff;			   
					tcMiiStationWrite(phyaddr, 9, reg_09);
					
				if(duplex == 1 ) 
				{ 
					reg_04=reg_04&0xfe5f;reg_04=reg_04|0x0040;
					printk("port%d 10M full duplex set\r\n",phyaddr);
				} 
				else 
				{ 
					reg_04=reg_04&0xfe3f;
					reg_04=reg_04|0x0020;
					printk("port%d 10M Half duplex set\r\n",phyaddr);
				}			   
					tcMiiStationWrite(phyaddr, 4, reg_04);				   
					tcMiiStationWrite(phyaddr, 0, 0x1340);					   
				break;
				
				default:			   
					printk("tce setANSpeed <phyaddr> <speed> <duplex>\r\n"); 		   
					printk("phy=32 is all port	   1G half is set all speed\r\n");			   
				break;			  
			} 		
		}
		spin_unlock_bh(&anspeed_lock);
		printk("\n\r");
	}	
	return 0;			
}

		

int doPhygetLinkRate (int argc, char *argv[], void *p)
{
	u16 phyaddr = 0;
	u32 	val;
    u8 	mr_speed;

	phyaddr = checked_atoi(argv[1]);
	val = tcMiiStationRead(phyaddr, 0);
	mr_speed = ((val>>13)|(val>>5))&0x03;	// 00:10,01:100,10:1000
	
	if(mr_speed == 0x0)
		printk(" [%d] 10M \r\n", phyaddr);	
	else if(mr_speed == 0x1)
		printk(" [%d] 100M \r\n", phyaddr);
	else if(mr_speed == 0x2)
		printk(" [%d] 1000M \r\n", phyaddr);

	return mr_speed;	
}

int doPhygetDuplex (int argc, char *argv[], void *p)
{
	u16 phyaddr = 0, r10_temp, r5_temp, r1_temp;
	u32 	val;
    u8 	mr_dplx;

	phyaddr = checked_atoi(argv[1]);
	val = tcMiiStationRead(phyaddr, 0);
	mr_dplx = ((val>>8)&0x01);
	r10_temp = tcMiiStationRead(phyaddr, 10);
	r5_temp = tcMiiStationRead(phyaddr, 5);
	r1_temp = tcMiiStationRead(phyaddr, 1);
	#if debug_flag
	if(((r1_temp>>5)&0x1) == 1)
	{
		if((((r10_temp>>11)&0x1) == 1)||(((r5_temp>>8)&0x1) == 1)||(((r5_temp>>6)&0x1) == 1))
		{
			mr_dplx = 1;
		}
	}
	#endif
	if(mr_dplx == 0x0)
		printk(" [%d] half \r\n", phyaddr);	
	else if(mr_dplx == 0x1)
		printk(" [%d] full \r\n", phyaddr);
		
	return mr_dplx;	
}

int doPhygetAN (int argc, char *argv[], void *p)
{
	u16 phyaddr = 0;
	u32 	val;
    u8 	mr_anen;

	phyaddr = checked_atoi(argv[1]);
	val = tcMiiStationRead(phyaddr, 0);
    #if debug_flag
	mr_anen = ((val>>12)&0x01);
	#endif
	if(mr_anen == 0x0)
		printk(" [%d] AN disable \r\n", phyaddr);	
	else if(mr_anen == 0x1)
		printk(" [%d] AN enable \r\n", phyaddr);

	return mr_anen;	
}

int doPhyforcePWD (int argc, char *argv[], void *p)
{
	u16 phyaddr = 0;

	phyaddr = checked_atoi(argv[1]);
	printk(" force [%d] power down \r\n", phyaddr);
	tcMiiStationWrite(phyaddr, 0, 0x0800);

	return 0;	
}

int doPhygetLinkStatus (int argc, char *argv[], void *p)
{
	u16 phyaddr = 0;
	u32 	val;
    u8 	r1_link_status;

	phyaddr = checked_atoi(argv[1]);
	val = tcMiiStationRead(phyaddr, 1);
	#if debug_flag
	r1_link_status = (val>>2)&0x01;
	#endif
	if(r1_link_status == 0x0)
		printk(" [%d] link down \r\n", phyaddr);	
	else if(r1_link_status == 0x1)
		printk(" [%d] link up \r\n", phyaddr);

	return r1_link_status;	
}

int doPhyforcePause (int argc, char *argv[], void *p)
{	// pause: r4[11:10]
	u16 phyaddr = 0, r4_temp = 0;
	
	phyaddr = checked_atoi(argv[1]);
	if((stricmp(argv[2], "on") == 0))
	{
		r4_temp = tcMiiStationRead(phyaddr, 4);
		tcMiiStationWrite(phyaddr, 4, (r4_temp|0x0c00));
		printk(" force [%d] pause on \r\n", phyaddr);
	}
	else if((stricmp(argv[2], "off") == 0))
	{
		r4_temp = tcMiiStationRead(phyaddr, 4);
		tcMiiStationWrite(phyaddr, 4, (r4_temp&0xf3ff));
		printk(" force [%d] pause off \r\n", phyaddr);
	}

	return 0;	
}

int doPhygetLoopback (int argc, char *argv[], void *p)
{
	u16 phyaddr = 0;
	u32 	val;
    u8 	r0_loopback;

	phyaddr = checked_atoi(argv[1]);
	val = tcMiiStationRead(phyaddr, 0);
	#if debug_flag
	r0_loopback = (val>>14)&0x01;
	#endif
	if(r0_loopback == 0x0)
		printk(" [%d] loopback off \r\n", phyaddr);	
	else if(r0_loopback == 0x1)
		printk(" [%d] loopback on \r\n", phyaddr);

	return r0_loopback;	
}

int doPhyforceLoopback (int argc, char *argv[], void *p)
{
	u16 phyaddr = 0, r0_temp = 0;

	phyaddr = checked_atoi(argv[1]);
	r0_temp = tcMiiStationRead(phyaddr, 0);
	printk(" force [%d] loopback \r\n", phyaddr);
	tcMiiStationWrite(phyaddr, 0, (r0_temp|0x4000));

	return 0;
}

int doPhygetLPCap (int argc, char *argv[], void *p)
{
	u16 phyaddr = 0, r5_temp = 0, r10_temp = 0;

	phyaddr = checked_atoi(argv[1]);
	r5_temp = tcMiiStationRead(phyaddr, 5);
	r10_temp = tcMiiStationRead(phyaddr, 10);

	if(((r10_temp>>11)&0x1) == 1)
	{
		printk(" [%d] LPCap support 1000M \r\n", phyaddr);	
		return 2;    // 1000
	}	
	else
	{
		if(((r5_temp>>7)&0x3) != 0)
		{
			printk(" [%d] LPCap support 100M \r\n", phyaddr);	
			return 1;    // 100
		}
		else
		{
			return 0;	// 10 or not an up
		}
	}
}

int doPhygetCALReg (int argc, char *argv[], void *p)
{
	u16 	phyaddr = 0;
	u16	l2_30=0, l0_0=0, l0_26=0, g2_25=0, l3_25=0;
	u16	l0_30=0, l4_21=0, g4_21=0, l2_23=0;
	u16	g1_26=0, l4_23=0;
	u32  delay=20, cal_case=0;

	phyaddr = checked_atoi(argv[1]);

	if(argc == 4)
	{
		delay = checked_atoi(argv[2]);
		cal_case =  checked_atoi(argv[3]);
	}
	
#if 1

	if(cal_case == 1) // tx offset
	{
		GECal_flag = 0;
		while(GECal_flag == 0)
			GECal_tx_offset(phyaddr, delay);
	}
	else if(cal_case == 5) // GE tx amp
	{
		GECal_flag = 0;
		//while(GECal_flag == 0)
			//GECal_R50(phyaddr, delay);
	}
	else if(cal_case == 6) // GE tx amp
	{
		GECal_flag = 0;
		//while(GECal_flag == 0)
			//GECal_tx_amp(phyaddr, delay);
	}
	
	printk("l2_30=0x%x, l0_0 =0x%x, l0_26=0x%x, g2_25=0x%x, l3_25=0x%x, \r\n", l2_30, l0_0 , l0_26, g2_25, l3_25);
	printk("l0_30=0x%x, l4_21=0x%x, g4_21=0x%x, l2_23=0x%x,             \r\n", l0_30, l4_21, g4_21, l2_23);
	printk("g1_26=0x%x, l4_23=0x%x,                                     \r\n", g1_26, l4_23);
#endif
	
	return 0;
}

int doPhyForceLED(int argc, char *argv[], void *p) 	// allen_20160630 
{
	u16 phyaddr = 0;
	u32 reg3 = 0, r15_temp = 0;
	
	phyaddr = checked_atoi(argv[1]);
	reg3 = tcMiiStationRead(phyaddr, 3);
	r15_temp = tcMiiStationRead(phyaddr, 15);

	if(reg3 == EPHY_ID_7583) // 7512GE
	{
		tcPhyVer = tcPhyVer_7583;
	}

	//printk("(%d) tcPhyVer = %x \r\n", phyaddr, tcPhyVer);
	return 0;
}	





//** MTK120625 start,
static spinlock_t pbus_lock;
/* frank modify for rt62806 */
u32
gswPbusRead(u32 pbus_addr)
{
	u32 pbus_data;

	u32 phyaddr;
	u32 reg;
	u32 value;

	spin_lock_bh(&pbus_lock);

	phyaddr = 31;
	// 1. write high-bit page address
	reg = 31;
	value = (pbus_addr >> 6);
  	tcMiiStationWrite(phyaddr, reg, value);
	//mdelay(5);
	//DBG_printk("1. miiw phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);
	//printk("1. miiw phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);

	// 2. read low DWord
	reg = (pbus_addr>>2) & 0x000f;
	value = tcMiiStationRead(phyaddr, reg);
	//mdelay(5);
	//DBG_printk("2. miir phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);
	//printk("2. miir phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);
	pbus_data = value;

	// 3. read high DWord
	reg = 16;
		value = tcMiiStationRead(phyaddr, reg);
	//mdelay(5);
	//DBG_printk("3. miir phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);
	//printk("3. miir phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);

	pbus_data = (pbus_data) | (value<<16);

	//DBG_printk("# pbus read addr=0x%04x data=0x%04x_%04x\r\n", (pbus_addr&0xfffc), (pbus_data>>16), (pbus_data&0xffff));
	//printk("# pbus read addr=0x%04x data=0x%04x_%04x\r\n", (pbus_addr&0xfffc), (pbus_data>>16), (pbus_data&0xffff));
//	printk("gswPbusRead read data:\n");

//	printk("pbus_data: %x\n", pbus_data);

	spin_unlock_bh(&pbus_lock);
	return pbus_data;
} /* end frank modify for rt62806 */

/* frank modify for rt62806 */
int
gswPbusWrite(u32 pbus_addr, u32 pbus_data)
{
	u32 phyaddr;
	u32 reg;
	u32 value;

	spin_lock_bh(&pbus_lock);

	phyaddr = 31;

	// 1. write high-bit page address
	reg = 31;
	value = (pbus_addr >> 6);
	tcMiiStationWrite(phyaddr, reg, value);
	//mdelay(5);

	//printk("1. miiw phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);
	//DBG_printk("1. miiw phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);

	// 2. write low DWord
	reg = (pbus_addr>>2) & 0x000f;
	value = pbus_data & 0xffff;
	tcMiiStationWrite(phyaddr, reg, value);
	//mdelay(5);
	//printk("2. miiw phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);
	//DBG_printk("2. miiw phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);

	// 3. write high DWord
	reg = 16;
	value = (pbus_data>>16) & 0xffff;
	tcMiiStationWrite(phyaddr, reg, value);
	//mdelay(5);
	//printk("3. miiw phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);
	//DBG_printk("3. miiw phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);

	//DBG_printk("# pbus write addr=0x%04x data=0x%04x_%04x\r\n", (pbus_addr&0xfffc), (pbus_data>>16),(pbus_data&0xffff));
	spin_unlock_bh(&pbus_lock);
  	return 0;
} /* end frank modify for rt62806 */
/*___________________________________________________________________
**      function name: doTrgmiiRxCal
**      descriptions:
**       Let TRGMII interface in training mode. Calibrate optimal Rx delay for each        
**       Rx pin. After calibration, leave training mode.
**     
**      parameters:
**         
**      global:
**
**      return:
**         -1:fail
**         0:succuces
**
**      call:
**
**      revision:
**      1. YMC 2017/06/07
**___________________________________________________________________
*/
#define EXTERNAL 1
#define INTERNAL 0
#define SOC_TO_EXT 0
#define EXT_TO_SOC 1
#define HEAD_55    1
#define TAIL_55    2
#define GSW_BASE		0xBFB58000
//F U N C T I O N   D E F I N I T I O N S
//#define read_reg_word(reg) 		VPint(reg)
//#if debug_flag
//#define write_reg_word(reg, wdata) 	VPint(reg)=wdata
//#endif

#if 0
u32 macMT7530ReadReg2(u32 gsw_reg, u8 external)
{
    if(external){
        return gswPbusRead(gsw_reg);
    }else{
        return read_reg_word(GSW_BASE + gsw_reg);
    }
}

int macMT7530WriteReg2(u32 gsw_reg, u32 gsw_data, u8 external)
{
    if(external){
        return gswPbusWrite(gsw_reg, gsw_data);
    }else{
    #if debug_flag
        write_reg_word((GSW_BASE + gsw_reg), gsw_data);
    #endif    
        return 0;
    }
}

int ErrChk(u32 RegAddr, u8 RxSwitch){
	u32 ErrCnt, RegVal;

	ErrCnt = 0;

	RegVal = macMT7530ReadReg2(RegAddr, RxSwitch);
	
	RegVal = RegVal | 0x40000000;
	macMT7530WriteReg2(RegAddr, RegVal, RxSwitch);

	RegVal = RegVal & 0xBFFFFFFF;
	macMT7530WriteReg2(RegAddr, RegVal, RxSwitch);

	ErrCnt = macMT7530ReadReg2(RegAddr, RxSwitch);
	ErrCnt = (ErrCnt & 0x00000F00) >> 8;

	return ErrCnt;
}

typedef struct _RX_CH_Window
{
    int head_cur;
    int	tail_cur;
    int head_best;
    int tail_best;
} RX_CH_Window;

void TrgmiiRxCal_RxRst(void)
{
	u32 RxChannel, RxRegAddr, Regvalue;
	u8 RxSwitch=0;
	for(RxSwitch=INTERNAL;RxSwitch <= EXTERNAL;RxSwitch++){
		//set clk delay to 0.
		Regvalue = macMT7530ReadReg2(0x7A00, RxSwitch);
    	Regvalue = Regvalue & 0xFFFFFF80;
		macMT7530WriteReg2(0x7A00, Regvalue, RxSwitch);
		for(RxChannel = 0 ; RxChannel <= 4 ; RxChannel++){
			switch(RxChannel){
    	        case 0:
    	            RxRegAddr = 0x7A10;
    	            break;
    	        case 1:
    	            RxRegAddr = 0x7A18;
    	            break;	            
    	        case 2:
    	            RxRegAddr = 0x7A20;
    	            break;
    	        case 3:
    	            RxRegAddr = 0x7A28;
    	            break;
    	        default:
    	            RxRegAddr = 0x7A30;
    	            break;
    	    }
			//set RxD delay to 0.
			Regvalue = macMT7530ReadReg2(RxRegAddr, RxSwitch);
		    Regvalue = Regvalue & 0xFFFFFF80;
		    macMT7530WriteReg2(RxRegAddr, Regvalue, RxSwitch);
		}
	}
}
#endif

#if 0 //debug_flag
static int doTrgmiiRxCal(void)
{
	u32 RxChannel, DAC, RxRegAddr, TxRegAddrl, TxRegAddr2, Regvalue, ErrCnt, Val;
    u8 TxSwitch=0, RxSwitch=0, direction=0, state;
    u16 delay_tap=0;
	RX_CH_Window	RX_CH_Window_55[5];
	RxChannel = 0;
	DAC = 0;

	TrgmiiRxCal_RxRst();
	for(TxSwitch=INTERNAL;TxSwitch <= EXTERNAL;TxSwitch++)
	{
		//Tx training mode enable
		TxRegAddrl = 0x7A40;
		Regvalue = macMT7530ReadReg2(TxRegAddrl, TxSwitch);
		Regvalue = Regvalue | 0x80000000;
		macMT7530WriteReg2(TxRegAddrl, Regvalue, TxSwitch);
		for(RxChannel = 0 ; RxChannel <= 4 ; RxChannel++)
		{
			TxRegAddr2 = 0x7A50 + 0x8*RxChannel;
			//All Channel transmits 0xAA
			Regvalue = macMT7530ReadReg2(TxRegAddr2,TxSwitch);
			Regvalue = (Regvalue & 0xFFFFFF00) | 0xAA;
			macMT7530WriteReg2(TxRegAddr2, Regvalue, TxSwitch);
		}
	}
    for (direction=0;direction < 2;direction++){
        printk("Direction %s \n",direction?"EXT_TO_SOC":"SOC_TO_EXT");     
        if(direction == SOC_TO_EXT){
            TxSwitch = INTERNAL;
            RxSwitch = EXTERNAL;
        }else{
            TxSwitch = EXTERNAL;
            RxSwitch = INTERNAL;
        }
    	//Scan
    	for (RxChannel = 0 ; RxChannel <= 4 ; RxChannel++) {
    	    switch(RxChannel){
    	        case 0:
    	            RxRegAddr = 0x7A10;
    				TxRegAddr2 = 0x7A50;
    	            break;
    	        case 1:
    	            RxRegAddr = 0x7A18;
    				TxRegAddr2 = 0x7A58;
    	            break;	            
    	        case 2:
    	            RxRegAddr = 0x7A20;
    				TxRegAddr2 = 0x7A60;
    	            break;
    	        case 3:
    	            RxRegAddr = 0x7A28;
    				TxRegAddr2 = 0x7A68;
    	            break;
    	        default:
    	            RxRegAddr = 0x7A30;
    				TxRegAddr2 = 0x7A70;
    	            break;
    	    }
			//Test pattern=0x55 on test channel.
			Regvalue = macMT7530ReadReg2(TxRegAddr2,TxSwitch);
    		Regvalue = (Regvalue & 0xFFFFFF00) | 0x55;
    		macMT7530WriteReg2(TxRegAddr2, Regvalue, TxSwitch);
			
			//clear variable
			RX_CH_Window_55[RxChannel].head_cur=0;
			RX_CH_Window_55[RxChannel].head_best=0;
			RX_CH_Window_55[RxChannel].tail_cur=0;
			RX_CH_Window_55[RxChannel].tail_best=0;
			//delay RxD to find pass window
            state = HEAD_55;
            for (DAC = 0 ; DAC <= 63 ; DAC++) {
                ErrCnt = 0;
    			Regvalue = macMT7530ReadReg2(RxRegAddr, RxSwitch);
    			Regvalue = Regvalue & 0xFFFFFF80;
    			Regvalue = Regvalue | DAC;
    			macMT7530WriteReg2(RxRegAddr, Regvalue, RxSwitch);
    			ErrCnt = ErrChk(RxRegAddr, RxSwitch);
				Regvalue = macMT7530ReadReg2(RxRegAddr, RxSwitch);
    			Val = (Regvalue & 0x00FF0000) >> 16;
    			//printk("RxChannel=%2d, DAC=0x%2x, Val=0x%2x, ErrCnt=0x%2x \n", 
    			    //RxChannel, DAC, Val, ErrCnt);
			    switch(state){
			        case HEAD_55:
			            if((Val == 0x55) && (ErrCnt == 0)){
			                RX_CH_Window_55[RxChannel].head_cur=DAC;
			                state = TAIL_55;
			            }
			            break;
			        case TAIL_55:
			            if((Val == 0x55) && (ErrCnt == 0)){
			                RX_CH_Window_55[RxChannel].tail_cur=DAC;			                
			            }else{
			                state = HEAD_55;
			            }
			            break;
			        default:
			            break;
                }
				if ((RX_CH_Window_55[RxChannel].tail_cur - RX_CH_Window_55[RxChannel].head_cur) > 
					(RX_CH_Window_55[RxChannel].tail_best - RX_CH_Window_55[RxChannel].head_best))
				{
					RX_CH_Window_55[RxChannel].head_best = RX_CH_Window_55[RxChannel].head_cur;
					RX_CH_Window_55[RxChannel].tail_best = RX_CH_Window_55[RxChannel].tail_cur;
				}
				if(RX_CH_Window_55[RxChannel].head_best != 0 && RX_CH_Window_55[RxChannel].tail_best != 0){
					if((Val == 0xAA) && (ErrCnt == 0))
						break;
				}
    		}
			//revert Tx pattern to 0xAA
			Regvalue = macMT7530ReadReg2(TxRegAddr2,TxSwitch);
			Regvalue = (Regvalue & 0xFFFFFF00) | 0xAA;
			macMT7530WriteReg2(TxRegAddr2, Regvalue, TxSwitch);
    	}
		
		//find optimal delay per RxD
		for(RxChannel = 0 ; RxChannel <= 4 ; RxChannel++){
			switch(RxChannel){
    	        case 0:
    	            RxRegAddr = 0x7A10;
    	            break;
    	        case 1:
    	            RxRegAddr = 0x7A18;
    	            break;	            
    	        case 2:
    	            RxRegAddr = 0x7A20;
    	            break;
    	        case 3:
    	            RxRegAddr = 0x7A28;
    	            break;
    	        default:
    	            RxRegAddr = 0x7A30;
    	            break;
    	    }
			Regvalue = macMT7530ReadReg2(RxRegAddr, RxSwitch);
        	Regvalue = Regvalue & 0xFFFFFF80;
	    	if((RX_CH_Window_55[RxChannel].head_best < RX_CH_Window_55[RxChannel].tail_best)){	
				delay_tap = ((RX_CH_Window_55[RxChannel].head_best + RX_CH_Window_55[RxChannel].tail_best)>>1);
				#if debug_flag
				printk("RxChannel=%2d,delay windows 55: 0x%2X ~ 0x%2X, optimal delay: 0x%2X\n",
                    RxChannel, RX_CH_Window_55[RxChannel].head_best, RX_CH_Window_55[RxChannel].tail_best, delay_tap);
				#endif
	    	}else{
	    		   delay_tap = 0;
	    		   #if debug_flag
	    		   printk("RxChannel=%2d, delay windows fail, set to default \n",RxChannel);
				   #endif
	    	}
           Regvalue = Regvalue | delay_tap;
           macMT7530WriteReg2(RxRegAddr, Regvalue, RxSwitch);
    	}
    }
	for(TxSwitch=INTERNAL;TxSwitch <= EXTERNAL;TxSwitch++)
	{
	    //Tx training mode disable
	    TxRegAddrl = 0x7A40;
	    Regvalue = macMT7530ReadReg2(TxRegAddrl, TxSwitch);
	    Regvalue = Regvalue & ~0x80000000;
	    macMT7530WriteReg2(TxRegAddrl, Regvalue, TxSwitch);
	}
	
	return 0;
}
#endif


int doPhyMMDRead(int argc, char *argv[], void *p)
{
#if debug_flag

    #ifndef CL45_CMD_SUPPORT
    const u16 MMD_Control_register=0xD;
    const u16 MMD_addr_data_register=0xE;
  	const u16 page_reg=31;
    #endif
	u32 dev_addr=0;
	u32 phyaddr=0;
	u32 reg_addr=0;
	u32 start_addr=0;
	u32 end_addr=0;
	u32 value=0;
	int i=0;

	if (argc!=4 && argc!=6)
	{
		printk("Usage: emiir all <phyaddr> <devaddr> <s_addr> <e_addr>\r\n");
		printk("       emiir <phyaddr> <devaddr> <reg>\r\n");
		return 0;			
	}
	
	if (strcasecmp(argv[1], "all") == 0)
	{ 	/*Post Read*/
		phyaddr = checked_atoi(argv[2]);//0~31
		sscanf(argv[3], "%x", &dev_addr);//hex
		sscanf(argv[4], "%x", &start_addr);//hex
		sscanf(argv[5], "%x", &end_addr);//hex
		if((start_addr>65535)  ||(end_addr>65535))
		{
			printk("s_addr or e_addr must be less than 65536\r\n");
			return 0;
		}
		for(i=start_addr; i<=end_addr; i++)
		{
			#ifdef CL45_CMD_SUPPORT
			value = mtEMiiRegRead(phyaddr, dev_addr, i);
			printk("* doPhyMMDRead =>phyaddr=%d,  dev_addr=%d, data_addr=0x%04X , value=0x%04X\r\n", phyaddr, dev_addr,  i, value);
			#else
			tcMiiStationWrite(phyaddr, page_reg, 0x00); 					//switch to main page
			tcMiiStationWrite(phyaddr, MMD_Control_register, (0<<14)+dev_addr);
			tcMiiStationWrite(phyaddr, MMD_addr_data_register, i);
			tcMiiStationWrite(phyaddr, MMD_Control_register, (1<<14)+dev_addr);
			value = tcMiiStationRead(phyaddr, MMD_addr_data_register);
			printk("* doPhyMMDRead_CL22 =>phyaddr=%d,  dev_addr=%d, data_addr=0x%04X , value=0x%04X\r\n", phyaddr, dev_addr,  i, value);
			#endif
		}
		printk("\r\n");
	}
    else
	{
		// tce emiir <DevAddr> <PhyAddr> <regAddr>
		phyaddr = checked_atoi(argv[1]);//0~31
		//dev_addr = checked_atoi(argv[2]);//0~31
		sscanf(argv[2], "%x", &dev_addr);//hex
		sscanf(argv[3], "%x", &reg_addr);//hex
		#ifdef CL45_CMD_SUPPORT
		value = mtEMiiRegRead(phyaddr, dev_addr, reg_addr);
		printk("* doPhyMMDRead =>phyaddr=%d,  dev_addr=%d, data_addr=0x%04X , value=0x%04X\r\n", phyaddr, dev_addr, reg_addr, value);
		#else
		tcMiiStationWrite(phyaddr, page_reg, 0x00); 						//switch to main page
		tcMiiStationWrite(phyaddr, MMD_Control_register, (0<<14)+dev_addr);
		tcMiiStationWrite(phyaddr, MMD_addr_data_register, reg_addr);
		tcMiiStationWrite(phyaddr, MMD_Control_register, (1<<14)+dev_addr);
		value = tcMiiStationRead(phyaddr, MMD_addr_data_register);
		printk("* doPhyMMDRead_CL22=>phyaddr=%d,  dev_addr=%d, data_addr=0x%04X , value=0x%04X\r\n", phyaddr, dev_addr, reg_addr, value);
		#endif
		return 0;
    }
    #endif
    
    return 0;           
}

int doPhyMMDWrite(int argc, char *argv[], void *p) 	// cl45
{
#if debug_flag
    #ifndef CL45_CMD_SUPPORT
    const u16 MMD_Control_register=0xD;
    const u16 MMD_addr_data_register=0xE;
    const u16 page_reg=31;
    #endif
	u32 dev_addr=0;
	u32 phyaddr=0;
	u32 reg_addr=0;
	u32 value=0;
	u16 STBit=0;
	u16 BFlen=0;
	u16 BF,BFMsk=0;
	u32 RValue =0;
	u8 i=0;

	if(argc==5)
	{	// tce emiiw <DevAddr> <PhyAddr> <regAddr> <data>
		phyaddr = checked_atoi(argv[1]);//0~31
		//dev_addr = checked_atoi(argv[2]);//0~31
		#if debug_flag
		sscanf(argv[2], "%x", &dev_addr);//hex
		sscanf(argv[3], "%x", &reg_addr);//hex
		sscanf(argv[4], "%x", &value);//hex
		#endif
    }
	else if(argc==7)
	{
		if(stricmp(argv[0], "emiiwb") == 0)
		{
			phyaddr = checked_atoi(argv[1]);
			//reg = checked_atoi(argv[2]);
			#if debug_flag
			sscanf(argv[2], "%x", &dev_addr);
			sscanf(argv[3], "%x", &reg_addr);
			STBit = checked_atoi(argv[4]);
			BFlen = checked_atoi(argv[5]);
			sscanf(argv[6], "%x", &value);
			#endif
			//printk("* Phyaddr=%d, DevAddr=0x%02lX,RegAddr=0x%02lX, STBit=%0d, BFlen=%0d, value=0x%04lX\r\n", phyaddr,dev_addr, reg_addr, STBit, BFlen, value);		
		}
	}
#if debug_flag

 	if(((argc == 5) || (argc == 7)) && (phyaddr<=0x1f))
 	{
 		if(argc == 5)
 		{     	
        	#ifdef CL45_CMD_SUPPORT
			mtEMiiRegWrite(phyaddr, dev_addr,reg_addr,value);
			printk("* doPhyMMDWrite => phyaddr=%d, dev_addr=%d, data_addr=0x%04X , value=0x%04X\r\n", phyaddr, dev_addr, reg_addr, value);
        	#else
			tcMiiStationWrite(phyaddr, page_reg, 0x00); 						//switch to main page
			tcMiiStationWrite(phyaddr, MMD_Control_register, (0<<14)+dev_addr);
			tcMiiStationWrite(phyaddr, MMD_addr_data_register, reg_addr);
			tcMiiStationWrite(phyaddr, MMD_Control_register, (1<<14)+dev_addr);
			tcMiiStationWrite(phyaddr, MMD_addr_data_register, value);
			printk("* doPhyMMDWrite_CL22=> phyaddr=%d, dev_addr=%d, data_addr=0x%04X , value=0x%04X\r\n", phyaddr, dev_addr, reg_addr, value);
        	#endif
		}
 		else if(((argc == 7) && (stricmp(argv[0], "emiiwb")) == 0))
 		{
			RValue = mtEMiiRegRead(phyaddr,dev_addr, reg_addr);
			for(i=0; i<BFlen; i++)
			{
				BF=1;
				if ((STBit+i)>31) break; //coverify check
				BF=BF<<(STBit+i);
				BFMsk = BFMsk | BF;
				//printk("1: BF=%d, BFMsk=%04x\r\n", BF, BFMsk);
			}
			BFMsk = ~BFMsk;
			//printk("2: BF=%d, BFMsk=%04x\r\n", BF, BFMsk); 
			if (STBit > 32) return 1; //coverify check
			value = (RValue & BFMsk) | (value<<STBit);
			//printk("* Phyaddr=%d, RegAddr=0x%02lX, OrgValue=0x%04lX, Modified value=0x%04lX\r\n", phyaddr, reg,  RValue, value);
		  	printk("* Phyaddr=%d, DevAddr=0x%02X,RegAddr=0x%02X, STBit=%0d, BFlen=%0d, value=0x%04X\r\n", phyaddr,dev_addr, reg_addr, STBit, BFlen, value);		
			printk("* OrgValue=0x%04X,",RValue);
			mtEMiiRegWrite(phyaddr, dev_addr,reg_addr, value);
			RValue = mtEMiiRegRead(phyaddr,dev_addr, reg_addr);
			printk(" ModValue=0x%04X\r\n", RValue);
 		}
		return 0;
	}
	else	
	{
        printk("       CMD Error : emiiw <PhyAddr> <DevAddr> <regAddr> <data>\r\n");
		printk("                         emiiwb <PhyAddr> <DevAddr> <RegAddr> <STBit> <BFLen> <BFVal> \r\n");	
        return 0;      
	}
#endif	
#endif
   return 0;
}

#endif

