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


u8   tcSWVer_FUNC = 1; 


u8 	allGeAnaCalWait(u32 delay, u8 phyaddr);  		 
void	GECal_Rext(u8 phyaddr, u32 delay);			 
void	GECal_tx_offset(u8 phyaddr, u32 delay);				 
//void	GECal_tx_amp(u8 phyaddr, u32 delay);			 
u8 	GECal_flag = 0;
static u8 	GECal_status = 0;
u8 	GECal_Rext_flag = 0;
u8 	allGeAnaCalWait_R45(u32 delay, u8 phyaddr); 
u8 	allGeAnaCalWait_TxOffset(u32 delay, u8 phyaddr); 
u8 	allGeAnaCalWait_TxAmp(u32 delay, u8 phyaddr); 

void doGePhyALLAnalogCal_R45(u8 phyaddr);
//void Calibration_all(u8 phyaddr,u8 resisiter); 			 
void	GECal_R45(u8 phyaddr, u32 delay);			

int Ghy_SetANSpeed(unsigned int phyaddr, int speed, int duplex );		

extern void mtMiiRegWrite(int port_num, int reg_num, int reg_data);
extern u32 mtMiiRegRead(u8 port_num,u8 reg_num);
extern void mtEMiiRegWrite(u32 port_num, u32 dev_num, u32 reg_num, u32 reg_data);;
extern u32 mtEMiiRegRead(u32 port_num, u32 dev_num, u32 reg_num);;
extern int tcMiiStationWrite(u32 phy_addr, u32 phy_reg, u32 phy_data);
extern int tcMiiStationRead(u32 phy_addr, u32 phy_reg);
extern void toKenRingWrite(unsigned int phy, unsigned int ch, unsigned int node, unsigned int data_addr,unsigned int value);
extern unsigned int toKenRingRead(unsigned int phy, unsigned int ch, unsigned int node, unsigned int data_addr);


/************************************************************************
*                       Calibration
*************************************************************************/
#define ANACAL_INIT			0x01
#define ANACAL_ERROR		0xFD
#define ANACAL_SATURATION	0xFE
#define	ANACAL_FINISH		0xFF

#define ANACAL_PAIR_A		0
#define ANACAL_PAIR_B		1
#define ANACAL_PAIR_C		2
#define ANACAL_PAIR_D		3

#define DAC_IN_0V					0x000
#define DAC_IN_2V					0x0f0	// +/-1V
#define TX_AMP_OFFSET_0mV			0
#define TX_AMP_OFFSET_VALID_BITS	6

#define FE_CAL_P0			0

//no define
const u8	ZCAL_0R_discrete_TXMR_TBL[64] =
{
   127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
   127,127,127,122,118,114,110,106,102, 98, 96, 92, 88, 85, 82, 80,
    76, 72, 70, 68, 64, 62, 60, 57, 55, 52, 50, 48, 46, 44, 41, 40,
    38, 36, 33, 32, 30, 28, 26, 24, 24, 22, 20, 18, 16, 16, 14, 12
};
//
const u8	ZCAL_5R_discrete_TXMR_TBL[64] =
{
   127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
   127,127,127,127,127,127,127,127,127,124,120, 116, 112, 110, 106, 102,
   99, 96, 93, 90, 88, 84, 81, 79, 76, 73, 71, 68, 66, 64, 61, 59,
   56, 54, 52, 50, 48, 46, 44, 42, 40, 39, 37, 36, 33, 32, 31, 29
};

//no define
const u8	ZCAL_0R_TXMR_TBL[64] =
{
	127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
	127,127,127,122,118,114,110,106,102, 98, 96, 92, 88, 85, 82, 80,
	 76, 72, 70, 68, 64, 62, 60, 57, 55, 52, 50, 48, 46, 44, 41, 40,
	 38, 36, 33, 32, 30, 28, 26, 24, 24, 22, 20, 18, 16, 16, 14, 12

};
//
const u8	ZCAL_5R_TXMR_TBL[64] =
{
	127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
	127,127,127,127,127,127,127,127,127,124,120, 116, 112, 110, 106, 102,
	99, 96, 93, 90, 88, 84, 81, 79, 76, 73, 71, 68, 66, 64, 61, 59,
	56, 54, 52, 50, 48, 46, 44, 42, 40, 39, 37, 36, 33, 32, 31, 29
};


// ************************************************************************
// 		Tx calibration use the Software flow : Rext, R50, Tx offset, Tx amp
//		Rx calibration use the Hardware flow
// ************************************************************************
#define   regWriteWord(addr,val)      ( *(volatile unsigned int *)(addr)=val )
#define   regReadWord(addr,val) 	  ( val=*(volatile unsigned int *)(addr) )


const u8 EN75xx_TX_OFS_TBL[64] =
{
  0x1f, 0x1e, 0x1d, 0x1c, 0x1b, 0x1a, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10,
  0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

extern int eco_rev;


void GECal_Rext(u8 phyaddr, u32 delay)
{
	u8	rg_zcal_ctrl,all_ana_cal_status;
	u16	ad_cal_comp_out_init;
	u16	dev1e_e0_ana_cal_r5,dev1f_ff_temp;
	int 	calibration_polarity;
	u8	cnt=0;

	// *** Iext/Rext Cal start ********************************************************************************
	all_ana_cal_status = ANACAL_INIT;
	// analog calibration enable, Rext calibration enable
	mtEMiiRegWrite(phyaddr, 0x1f, 0x0100, 0xc000);			// BG voltage output
	mtEMiiRegWrite(phyaddr, 0x1f, 0x00ff, 0x0002);			// rg_tst_mode2 [bit 3:2] 0
		// tienchung rg_tst_mode2 [bit 3:2] 0
	dev1f_ff_temp = mtEMiiRegRead(phyaddr, 0x1f, 0x00ff);
	dev1f_ff_temp = dev1f_ff_temp&0xfff3;
	mtEMiiRegWrite(phyaddr, 0x1f, 0x00ff, dev1f_ff_temp);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x1110);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0000);	// 1e_dc[0]:rg_txvos_calen
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00e1, 0x0000);	// 1e_e1[4]:rg_cal_refsel(0:1.2V)

	rg_zcal_ctrl = 0x20;  								// start with 0 dB
	dev1e_e0_ana_cal_r5 = mtEMiiRegRead(phyaddr, 0x1e, 0x00e0);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00e0, (rg_zcal_ctrl));	// 1e_e0[5:0]:rg_zcal_ctrl

	all_ana_cal_status = allGeAnaCalWait(delay, phyaddr); 	// delay 20 usec
	if(all_ana_cal_status == 0)
	{
		all_ana_cal_status = ANACAL_ERROR;	
		printk(" GE Rext AnaCal ERROR!   \r\n");
	}
	
	ad_cal_comp_out_init = (mtEMiiRegRead(phyaddr, 0x1e, 0x017a)>>8) & 0x1;		// 1e_17a[8]:ad_cal_comp_out	
	if(ad_cal_comp_out_init == 1)
	{
		calibration_polarity = -1;
	}
	else // ad_cal_comp_out_init == 0
	{
		calibration_polarity = 1;
	}
	
	cnt = 0;
	while(all_ana_cal_status < ANACAL_ERROR)
	{
		cnt ++;
		rg_zcal_ctrl += calibration_polarity;
		mtEMiiRegWrite(phyaddr, 0x1e, 0x00e0, (rg_zcal_ctrl));
		all_ana_cal_status = allGeAnaCalWait(delay, phyaddr); // delay 20 usec

		if(all_ana_cal_status == 0)
		{
			all_ana_cal_status = ANACAL_ERROR;	
			printk("  GE Rext AnaCal ERROR!   \r\n");
		}
		else if(((mtEMiiRegRead(9, 0x1e, 0x017a)>>8)&0x1) != ad_cal_comp_out_init) 
		{
			all_ana_cal_status = ANACAL_FINISH;	
			printk("  GE Rext AnaCal Done! (%d)(0x%x)  \r\n", cnt, rg_zcal_ctrl);
		}
		else
		{
			if((rg_zcal_ctrl == 0x3F)||(rg_zcal_ctrl == 0x00))	
			{
				all_ana_cal_status = ANACAL_SATURATION;  // need to FT(IC fail?)
				printk(" GE Rext AnaCal Saturation!  \r\n");
				rg_zcal_ctrl = 0x20;  // 0 dB
			}
			//else
			//	printk(" GE Rxet cal (%d)(%d)(%d)(0x%x)  \r\n", cnt, ad_cal_comp_out_init, ((mtEMiiRegRead(phyaddr_p0, 0x1e, 0x017a)>>8)&0x1), mtEMiiRegRead(phyaddr, 0x1e, 0xe0));
		}
	}

	if(all_ana_cal_status == ANACAL_ERROR)
	{	
		rg_zcal_ctrl = 0x20;  // 0 dB
		mtEMiiRegWrite(phyaddr, 0x1e, 0x00e0, (dev1e_e0_ana_cal_r5 | rg_zcal_ctrl));
		printk("  GE Rext AnaCal Fail! (0x%x)  \r\n", rg_zcal_ctrl);
		GECal_Rext_flag = 1;
	}
	else
	{
		mtEMiiRegWrite(phyaddr, 0x1e, 0x00e0, (dev1e_e0_ana_cal_r5 | rg_zcal_ctrl));
		mtEMiiRegWrite(phyaddr, 0x1e, 0x00e0, ((rg_zcal_ctrl<<8)|rg_zcal_ctrl));
		//****  1f_115[2:0] = rg_zcal_ctrl[5:3]  // Mog review
		mtEMiiRegWrite(phyaddr, 0x1f, 0x0115, ((rg_zcal_ctrl & 0x3f)>>3));
		printk("  GE Rext AnaCal Done! (%d)(0x%x)  \r\n", cnt, rg_zcal_ctrl);
		GECal_Rext_flag = 1;
/*       //steven say for 7580 test BER
		regReadWord(0xbfa2016c, reg_temp);
		//printk("RG_BG_RASEL = 0x%x (x%x)\r\n", reg_temp, rg_zcal_ctrl);
		reg_temp = (reg_temp & 0x1fff); 
		reg_temp = ((((rg_zcal_ctrl>>3)&0x7)<<13) | reg_temp);
		//printk("RG_BG_RASEL = 0x%x (x%x)\r\n", reg_temp, rg_zcal_ctrl);
		regWriteWord(0xbfa2016c, reg_temp);   // for ACD/steven simldo
		*/

	}
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1f, 0x0100, 0x0000);			// return default
	// *** Iext/Rext Cal end ***
/*	
	//IRSET debug
			printk("  phy=9 devid=0x1e register=0xe0 value=0x%x ",mtEMiiRegRead(9, 0x1e, 0xe0));
			printk("  phy=10 devid=0x1e register=0xe0 value=0x%x ",mtEMiiRegRead(10, 0x1e, 0xe0));
			printk("  phy=11 devid=0x1e register=0xe0 value=0x%x ",mtEMiiRegRead(11, 0x1e, 0xe0));
			printk("  phy=12 devid=0x1e register=0xe0 value=0x%x \n\r",mtEMiiRegRead(12, 0x1e, 0xe0));
*/
}

void R50Calibration(u8 phyaddr,u8 phyaddr_p0,u8 resisiter)
{
	u8	rg_zcal_ctrl, all_ana_cal_status;
	u16	ad_cal_comp_out_init;
	u16	dev1e_e0_ana_cal_r5, transformer_temp;
	int 	calibration_polarity;
	u16	calibration_pair;
	u16  cnt=0;
	// *** R45 Cal start *******************************************************************************
	mtEMiiRegWrite(phyaddr_p0, 0x1f, 0x0100, 0xc000);			// BG voltage output
	mtEMiiRegWrite(phyaddr_p0, 0x1e, 0x00db, 0x1100);	// 1e_dc[0]:rg_txvos_calen
	mtEMiiRegWrite(phyaddr_p0, 0x1e, 0x00dc, 0x0000);	// 1e_e1[4]:select 1V
	mtEMiiRegWrite(phyaddr_p0, 0x1e, 0x00e1, 0x0010);	// 1e_e1[4]:select 1V

	tcMiiStationWrite(phyaddr, 0x1f, 0x0000);		// page0
	tcMiiStationWrite(phyaddr, 0x0,  0x0140);		// set [12]AN disable, [8]full duplex, [13/6]1000Mbps
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0145, 0x1010);			// fix mdi
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0185, 0x0000);			// fix mdi
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00fb, 0x0100);			// fix mdi
	mtEMiiRegWrite( phyaddr, 0x1e, 0x00db, 0x1101);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
	mtEMiiRegWrite( phyaddr, 0x1e, 0x00dc, 0x0000);	// 1e_dc[0]:rg_txvos_calen
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00e1, 0x0010);	// 1e_e1[4]:select 1V

	for(calibration_pair = ANACAL_PAIR_A; calibration_pair <= ANACAL_PAIR_D; calibration_pair ++)
	{
		rg_zcal_ctrl = 0x20;  						// start with 0 dB
		dev1e_e0_ana_cal_r5 = (mtEMiiRegRead( phyaddr_p0, 0x1e, 0x00e0) & (~0x003f));
		mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x00e0, (dev1e_e0_ana_cal_r5 | rg_zcal_ctrl));	// 1e_e0[5:0]:rg_zcal_ctrl
		
		if(calibration_pair == ANACAL_PAIR_A)
		{
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x1101);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0000);
		}
		else if(calibration_pair == ANACAL_PAIR_B)
		{
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x1100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x1000);	// 1e_dc[12]:rg_zcalen_b
		}
		else if(calibration_pair == ANACAL_PAIR_C)
		{
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x1100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0100);	// 1e_dc[8]:rg_zcalen_c
		}
		else // if(calibration_pair == ANACAL_PAIR_D)
		{
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x1100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0010);	// 1e_dc[4]:rg_zcalen_d
		}

		all_ana_cal_status = allGeAnaCalWait_R45(20, phyaddr); // delay 20 usec
		if(all_ana_cal_status == 0)
		{
			all_ana_cal_status = ANACAL_ERROR;
			printk("  GE R45 AnaCal ERROR!   \r\n");
		}

		ad_cal_comp_out_init = (mtEMiiRegRead(phyaddr_p0, 0x1e, 0x017a)>>8) & 0x1;  // 1e_17a[8]:ad_cal_comp_out
		if(ad_cal_comp_out_init == 1)
		{
			calibration_polarity = -1;
		}
		else
		{
			calibration_polarity = 1;
		}

		cnt = 0;
		while(all_ana_cal_status < ANACAL_ERROR)
		{
			cnt ++;
			rg_zcal_ctrl += calibration_polarity;
			mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x00e0, (dev1e_e0_ana_cal_r5 | rg_zcal_ctrl));
			all_ana_cal_status = allGeAnaCalWait_R45(20, phyaddr_p0); // delay 20 usec

			if(all_ana_cal_status == 0)
			{
				all_ana_cal_status = ANACAL_ERROR;
				printk("  GE R45 AnaCal ERROR!   \r\n");
			}
			else if(((mtEMiiRegRead( phyaddr_p0, 0x1e, 0x017a)>>8)&0x1) != ad_cal_comp_out_init)
			{
				all_ana_cal_status = ANACAL_FINISH;
			}
			else
			{
				if((rg_zcal_ctrl == 0x3F)||(rg_zcal_ctrl == 0x00))
				{
					all_ana_cal_status = ANACAL_SATURATION;  // need to FT
					printk(" GE R45 AnaCal Saturation!  \r\n");
				}
			}
		}

		if(all_ana_cal_status == ANACAL_ERROR)
		{
			rg_zcal_ctrl = 0x20;  // 0 dB
		}
		else
		{
			printk("  GE R45 AnaCal zcal_idx(before table) (dec: %d) (hex: 0x%x) \r\n",  rg_zcal_ctrl, (rg_zcal_ctrl));
			//rg_zcal_ctrl = ZCAL_TO_R44ohm_TBL[rg_zcal_ctrl];
				//TXMR_or_discrete=0-->discrete TXMR_or_discrete=1-->TXMR
         if(phyaddr == 9) {transformer_temp=transformer_status&0x01;}
         if(phyaddr == 10) {transformer_temp=transformer_status&0x02;}
         if(phyaddr == 11) {transformer_temp=transformer_status&0x04;}
         if(phyaddr == 12) {transformer_temp=transformer_status&0x08;}
	         if(transformer_temp != 0) 
		     {
			   if(mdi_resister==5) 
			  	 rg_zcal_ctrl = ZCAL_5R_TXMR_TBL[rg_zcal_ctrl];
			   else
			     rg_zcal_ctrl = ZCAL_0R_TXMR_TBL[rg_zcal_ctrl];
			 } 
			 else 
			 { 
		       if(mdi_resister==5) 
			   	 rg_zcal_ctrl = ZCAL_5R_discrete_TXMR_TBL[rg_zcal_ctrl];
			   else
			   	 rg_zcal_ctrl = ZCAL_0R_discrete_TXMR_TBL[rg_zcal_ctrl];
			 }
			printk("  transformer_temp =%d  mdi_resister=%d  \r\n",transformer_temp, mdi_resister );			
			printk("  GE R45 AnaCal Done! (cnt:%d)(after table) ([7][6:0]:0x%x) ([6:0]:0x%x) \r\n", cnt, rg_zcal_ctrl, (rg_zcal_ctrl|0x80));
		}

		if(calibration_pair == ANACAL_PAIR_A)
		{
			ad_cal_comp_out_init = mtEMiiRegRead(phyaddr, 0x1e, 0x0174) & (~0x7f00);
			mtEMiiRegWrite(phyaddr, 0x1e, 0x0174, (ad_cal_comp_out_init | (((rg_zcal_ctrl<<8)&0xff00) | 0x8000)));	// 1e_174[15:8]
		}
		else if(calibration_pair == ANACAL_PAIR_B)
		{
			ad_cal_comp_out_init = mtEMiiRegRead(phyaddr, 0x1e, 0x0174) & (~0x007f);
			mtEMiiRegWrite(phyaddr, 0x1e, 0x0174, (ad_cal_comp_out_init | (((rg_zcal_ctrl<<0)&0x00ff) | 0x0080)));	// 1e_174[7:0]
		}
		else if(calibration_pair == ANACAL_PAIR_C)
		{
			ad_cal_comp_out_init = mtEMiiRegRead(phyaddr, 0x1e, 0x0175) & (~0x7f00);
			mtEMiiRegWrite(phyaddr, 0x1e, 0x0175, (ad_cal_comp_out_init | (((rg_zcal_ctrl<<8)&0xff00) | 0x8000)));	// 1e_175[15:8]
		}
		else // if(calibration_pair == ANACAL_PAIR_D)
		{
			ad_cal_comp_out_init = mtEMiiRegRead(phyaddr, 0x1e, 0x0175) & (~0x007f);
			mtEMiiRegWrite(phyaddr, 0x1e, 0x0175, (ad_cal_comp_out_init | (((rg_zcal_ctrl<<0)&0x00ff) | 0x0080)));	// 1e_175[7:0]
		}
	}

// return default
	tcMiiStationWrite(phyaddr, 0x1f, 0x0000);		// page0
	tcMiiStationWrite(phyaddr, 0x0,  0x1000);		// set [12]AN disable, [8]full duplex, [13/6]1000Mbps
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0145, 0x1000);			// fix mdi
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0185, 0x0001);			//
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00fb, 0x0000);			// 	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x00db, 0x0000);
	mtEMiiRegWrite( phyaddr, 0x1e, 0x00db, 0x0000);
	mtEMiiRegWrite( phyaddr, 0x1e, 0x00dc, 0x0000);
    mtEMiiRegWrite( phyaddr, 0x1f, 0x0100, 0x0000);

	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x00db, 0x0000);	
	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x00dc, 0x0000);	
	mtEMiiRegWrite( phyaddr_p0, 0x1f, 0x0100, 0x0000);			

/*
	//IRSET
			printk("  phy=9 devid=0x1e register=0xe0 value=0x%x ",mtEMiiRegRead(9, 0x1e, 0xe0));
			printk("  phy=10 devid=0x1e register=0xe0 value=0x%x ",mtEMiiRegRead(10, 0x1e, 0xe0));
			printk("  phy=11 devid=0x1e register=0xe0 value=0x%x ",mtEMiiRegRead(11, 0x1e, 0xe0));
			printk("  phy=12 devid=0x1e register=0xe0 value=0x%x \n\r",mtEMiiRegRead(12, 0x1e, 0xe0));
	//r50
	        printk("  phy=9 devid=0x1e register=0x174 value=0x%x ",mtEMiiRegRead(9, 0x1e, 0x174));
	        printk("  phy=9 devid=0x1e register=0x175 value=0x%x \n\r",mtEMiiRegRead(9, 0x1e, 0x175));
	        printk("  phy=10 devid=0x1e register=0x174 value=0x%x ",mtEMiiRegRead(10, 0x1e, 0x174));
	        printk("  phy=10 devid=0x1e register=0x175 value=0x%x \n\r",mtEMiiRegRead(10, 0x1e, 0x175));
	        printk("  phy=11 devid=0x1e register=0x174 value=0x%x ",mtEMiiRegRead(11, 0x1e, 0x174));
	        printk("  phy=11 devid=0x1e register=0x175 value=0x%x \n\r",mtEMiiRegRead(11, 0x1e, 0x175));
	        printk("  phy=12 devid=0x1e register=0x174 value=0x%x ",mtEMiiRegRead(12, 0x1e, 0x174));
	        printk("  phy=12 devid=0x1e register=0x175 value=0x%x \n\r",mtEMiiRegRead(12, 0x1e, 0x175));
*/
	// *** R50 Cal end ***
}


void TX_OFFSET_Calibration(u8 phyaddr,u8 phyaddr_p0)
{
	u8	all_ana_cal_status;
	u16	ad_cal_comp_out_init;
	int 	calibration_polarity, tx_offset_temp;
	u16	calibration_pair, cal_temp;
	u16	tx_offset_reg_shift; 
	u16	tx_offset_reg, reg_temp;
	u16	cnt=0;

	// *** Tx offset Cal start *************************************************************************
	//mtEMiiRegWrite( phyaddr_p0, 0x1f, 0x0100, 0xc000);
	mtEMiiRegWrite( phyaddr, 0x1f, 0x0100, 0xc000);
	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x00db, 0x0100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x00dc, 0x0001);	// 1e_dc[0]:rg_txvos_calen	
	//mtEMiiRegWrite(phyaddr_p0, 0x1e, 0x00dd, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x0100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0001);
	
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0145, 0x1010);			// fix mdi
	tcMiiStationWrite(phyaddr, 0x1f, 0x0000);		// page0 
	tcMiiStationWrite(phyaddr, 0x0,  0x0140);		// set [12]AN disable, [8]full duplex, [13/6]1000Mbps
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0096, 0x8000);	// 1e_96[15]:bypass_tx_offset_cal, Hw bypass, Fw cal
	mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xf808);	// 1e_3e
	
	mtEMiiRegWrite(phyaddr, 0x1e, 0x017d, (0x8000|DAC_IN_0V));	// 1e_17d:dac_in0_a
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0181, (0x8000|DAC_IN_0V));	// 1e_181:dac_in1_a
	mtEMiiRegWrite(phyaddr, 0x1e, 0x017e, (0x8000|DAC_IN_0V));	// 1e_17e:dac_in0_b
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0182, (0x8000|DAC_IN_0V));	// 1e_182:dac_in1_b	
	mtEMiiRegWrite(phyaddr, 0x1e, 0x017f, (0x8000|DAC_IN_0V));	// 1e_17f:dac_in0_c
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0183, (0x8000|DAC_IN_0V));	// 1e_183:dac_in1_c
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0180, (0x8000|DAC_IN_0V));	// 1e_180:dac_in0_d
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0184, (0x8000|DAC_IN_0V));	// 1e_184:dac_in1_d
			
	//mtEMiiRegWrite(phyaddr, 0x1e, 0x0185, 0x0000);			// fix mdi
	//mtEMiiRegWrite(phyaddr, 0x1e, 0x00fb, 0x0100);			// fix mdi



	//mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x0100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
	//mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0001);	// 1e_dc[0]:rg_txvos_calen
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x1000);



	//mtEMiiRegWrite(phyaddr, 0x1e, 0x0015, 0x0004);	// gating, cutoff thers pair 
	
	for(calibration_pair = ANACAL_PAIR_A; calibration_pair <= ANACAL_PAIR_D; calibration_pair ++)
	{
		tx_offset_temp = 0x20; //TX_AMP_OFFSET_0mV;
		
		if(calibration_pair == ANACAL_PAIR_A)
		{
			// for 7580, from ACD/BH's suggest
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x1000);				// 1e_dd[12]:rg_txg_calen_a
			//mtEMiiRegWrite(phyaddr_p0, 0x1e, 0x00dd, 0x1000);
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x0101);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0001);
			

			reg_temp = (mtEMiiRegRead(phyaddr, 0x1e, 0x0172) & (~0x3f00));
			tx_offset_reg_shift = 8;									// 1e_172[13:8]
			tx_offset_reg = 0x0172;
			//mtEMiiRegWrite(phyaddr, 0x1e, tx_offset_reg, (reg_temp|(tx_offset_temp<<tx_offset_reg_shift)));
		}
		else if(calibration_pair == ANACAL_PAIR_B)
		{
			// for 7580, from ACD/BH's suggest
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x0100);				// 1e_dd[8]:rg_txg_calen_b
			//mtEMiiRegWrite(phyaddr_p0, 0x1e, 0x00dd, 0x0100);
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x0100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x1001);	// 1e_dc[12]:rg_zcalen_b


			reg_temp = (mtEMiiRegRead(phyaddr, 0x1e, 0x0172) & (~0x003f));
			tx_offset_reg_shift = 0;									// 1e_172[5:0]
			tx_offset_reg = 0x0172;
			//mtEMiiRegWrite(phyaddr, 0x1e, tx_offset_reg, (reg_temp|(tx_offset_temp<<tx_offset_reg_shift)));
		}
		else if(calibration_pair == ANACAL_PAIR_C)
		{
			// for 7580, from ACD/BH's suggest
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x0010);				// 1e_dd[4]:rg_txg_calen_c
			//mtEMiiRegWrite(phyaddr_p0, 0x1e, 0x00dd, 0x0010);
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x0100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0101);	// 1e_dc[8]:rg_zcalen_c


			reg_temp = (mtEMiiRegRead(phyaddr, 0x1e, 0x0173) & (~0x3f00));
			tx_offset_reg_shift = 8;									// 1e_173[13:8]
			tx_offset_reg = 0x0173;
			//mtEMiiRegWrite(phyaddr, 0x1e, tx_offset_reg, (reg_temp|(tx_offset_temp<<tx_offset_reg_shift)));
		}
		else // if(calibration_pair == ANACAL_PAIR_D)
		{
			// for 7580, from ACD/BH's suggest
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x0001);				// 1e_dd[0]:rg_txg_calen_d
			//mtEMiiRegWrite(phyaddr_p0, 0x1e, 0x00dd, 0x0001);
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x0100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0011);	// 1e_dc[4]:rg_zcalen_d


			reg_temp = (mtEMiiRegRead(phyaddr, 0x1e, 0x0173) & (~0x003f));
			tx_offset_reg_shift = 0;									// 1e_173[5:0]
			tx_offset_reg = 0x0173;
			//mtEMiiRegWrite(phyaddr, 0x1e, tx_offset_reg, (reg_temp|(tx_offset_temp<<tx_offset_reg_shift)));
		}
		mtEMiiRegWrite(phyaddr, 0x1e, tx_offset_reg, (reg_temp|(tx_offset_temp<<tx_offset_reg_shift)));	// 1e_172, 1e_173
		
		all_ana_cal_status = allGeAnaCalWait_TxOffset(20, phyaddr_p0); // delay 20 usec
		if(all_ana_cal_status == 0)
		{
			all_ana_cal_status = ANACAL_ERROR;	
			printk(" GE Tx offset AnaCal ERROR!   \r\n");
		}

		//printk(" GE Tx offset check \r\n");
		//printk(" reg0=0x%x, 1f_100=0x%x, 1e_145=0x%x, 1e_185=0x%x  \r\n", mtMiiRegRead(phyaddr, 0), (mtEMiiRegRead(phyaddr, 0x1f, 0x100)), (mtEMiiRegRead(phyaddr, 0x1e, 0x145)), (mtEMiiRegRead(phyaddr, 0x1e, 0x185)));
		//printk(" 1e_db=0x%x, 1e_dc=0x%x, 1e_96=0x%x, 1e_3e=0x%x  \r\n", (mtEMiiRegRead(phyaddr, 0x1e, 0xdb)), (mtEMiiRegRead(phyaddr, 0x1e, 0xdc)), (mtEMiiRegRead(phyaddr, 0x1e, 0x96)), (mtEMiiRegRead(phyaddr, 0x1e, 0x3e)));
		//printk(" 1e_17d=0x%x, 1e_181=0x%x, 1e_17e=0x%x, 1e_182=0x%x  \r\n", (mtEMiiRegRead(phyaddr, 0x1e, 0x17d)), (mtEMiiRegRead(phyaddr, 0x1e, 0x181)), (mtEMiiRegRead(phyaddr, 0x1e, 0x17e)), (mtEMiiRegRead(phyaddr, 0x1e, 0x182)));
		//printk(" 1e_e0=0x%x, 1f_115=0x%x, 1f_426=0x%x, 1e_15=0x%x  \r\n", (mtEMiiRegRead(phyaddr, 0x1e, 0xe0)), (mtEMiiRegRead(phyaddr, 0x1f, 0x115)), (mtEMiiRegRead(phyaddr, 0x1f, 0x426)), (mtEMiiRegRead(phyaddr, 0x1e, 0x15)));
					
		ad_cal_comp_out_init = (mtEMiiRegRead(phyaddr_p0, 0x1e, 0x017a)>>8) & 0x1;		// 1e_17a[8]:ad_cal_comp_out
		
		if(ad_cal_comp_out_init == 0)
		{
			calibration_polarity = -1;
		}
		else
		{
			calibration_polarity = 1;
		}

		cnt = 0;
		while(all_ana_cal_status < ANACAL_ERROR)
		{
			cnt ++;
			tx_offset_temp += calibration_polarity;

			//printk(" GE Tx offset AnaCal cnt=%d,  comp_out = 0x%x, comp_init = 0x%x, tx_offset_temp=0x%x, cal_temp=0x%x, 1e_172=0x%x  \r\n", cnt, (mtEMiiRegRead(phyaddr_p0, 0x1e, 0x017a)), ad_cal_comp_out_init, tx_offset_temp, cal_temp, (mtEMiiRegRead(phyaddr, 0x1e, 0x0172)));
			
			if(tx_offset_temp >= 0)
			{
				cal_temp = tx_offset_temp;
			}
			else
			{
				tx_offset_temp = 0x3f;
				cal_temp = 0x3f; //(1<<(TX_AMP_OFFSET_VALID_BITS-1)) | abs(tx_offset_temp);
			}
			mtEMiiRegWrite(phyaddr, 0x1e, tx_offset_reg, (reg_temp|(cal_temp<<tx_offset_reg_shift)));

			all_ana_cal_status = allGeAnaCalWait_TxOffset(20, phyaddr_p0); // delay 20 usec
			if(all_ana_cal_status == 0)
			{
				all_ana_cal_status = ANACAL_ERROR;	
				printk(" GE Tx offset AnaCal ERROR!   \r\n");
			}
			else if(((mtEMiiRegRead(phyaddr_p0, 0x1e, 0x017a)>>8)&0x1) != ad_cal_comp_out_init) 
			{
				all_ana_cal_status = ANACAL_FINISH;	
			}
			else
			{
				//printk(" GE Tx offset AnaCal 1e_17a=0x%x, cal_temp=0x%x  \r\n", mtEMiiRegRead(phyaddr_p0, 0x1e, 0x017a), cal_temp);
				
				if((tx_offset_temp == -31)||(tx_offset_temp == 0x3f))	
				{
					all_ana_cal_status = ANACAL_SATURATION;  // need to FT
					printk(" GE Tx offset AnaCal Saturation!  \r\n");
				}
			}
		}
		
		if(all_ana_cal_status == ANACAL_ERROR)
		{	
			tx_offset_temp = TX_AMP_OFFSET_0mV;
			mtEMiiRegWrite(phyaddr, 0x1e, tx_offset_reg, (reg_temp|(tx_offset_temp<<tx_offset_reg_shift)));
		}
		else
		{
			printk(" GE Tx offset AnaCal Done! (%d)(0x%x)  \r\n", cnt, cal_temp);
		}
	}
	mtEMiiRegWrite( phyaddr_p0, 0x1f, 0x0100, 0x0000);
	//mtEMiiRegWrite( phyaddr_p0, 0x1f, 0x0101, 0x808c);	
	mtEMiiRegWrite(phyaddr_p0, 0x1e, 0x00db, 0x0000);	// disable analog calibration circuit
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0096, 0x0000);	// 1e_96[15]:bypass_tx_offset_cal, Hw bypass, Fw cal
	mtEMiiRegWrite(phyaddr, 0x1e, 0x017d, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x017e, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x017f, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0180, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0181, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0182, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0183, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0184, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x0000);	// disable analog calibration circuit
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0000);	// disable Tx offset calibration circuit
	mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xc000);	// disable Tx VLD force mode
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x0000);	// disable Tx offset/amplitude calibration circuit
	
	mtEMiiRegWrite(phyaddr_p0, 0x1e, 0x00dc, 0x0000);	// disable Tx offset calibration circuit
	mtEMiiRegWrite(phyaddr_p0, 0x1e, 0x00dd, 0x0000);	// disable analog calibration circuit
	/*
	//IRSET
			printk("  phy=9 devid=0x1e register=0xe0 value=0x%x ",mtEMiiRegRead(9, 0x1e, 0xe0));
			printk("  phy=10 devid=0x1e register=0xe0 value=0x%x ",mtEMiiRegRead(10, 0x1e, 0xe0));
			printk("  phy=11 devid=0x1e register=0xe0 value=0x%x ",mtEMiiRegRead(11, 0x1e, 0xe0));
			printk("  phy=12 devid=0x1e register=0xe0 value=0x%x \n\r",mtEMiiRegRead(12, 0x1e, 0xe0));
	//r50
	        printk("  phy=9 devid=0x1e register=0x174 value=0x%x ",mtEMiiRegRead(9, 0x1e, 0x174));
	        printk("  phy=9 devid=0x1e register=0x175 value=0x%x \n\r",mtEMiiRegRead(9, 0x1e, 0x175));
	        printk("  phy=10 devid=0x1e register=0x174 value=0x%x ",mtEMiiRegRead(10, 0x1e, 0x174));
	        printk("  phy=10 devid=0x1e register=0x175 value=0x%x \n\r",mtEMiiRegRead(10, 0x1e, 0x175));
	        printk("  phy=11 devid=0x1e register=0x174 value=0x%x ",mtEMiiRegRead(11, 0x1e, 0x174));
	        printk("  phy=11 devid=0x1e register=0x175 value=0x%x \n\r",mtEMiiRegRead(11, 0x1e, 0x175));
	        printk("  phy=12 devid=0x1e register=0x174 value=0x%x ",mtEMiiRegRead(12, 0x1e, 0x174));
	        printk("  phy=12 devid=0x1e register=0x175 value=0x%x \n\r",mtEMiiRegRead(12, 0x1e, 0x175));
	//tx offset
	        printk("  phy=9 devid=0x1e register=0x172 value=0x%x ",mtEMiiRegRead(9, 0x1e, 0x172));
	        printk("  phy=9 devid=0x1e register=0x173 value=0x%x \n\r",mtEMiiRegRead(9, 0x1e, 0x173));
	        printk("  phy=10 devid=0x1e register=0x172 value=0x%x ",mtEMiiRegRead(10, 0x1e, 0x172));
	        printk("  phy=10 devid=0x1e register=0x173 value=0x%x \n\r",mtEMiiRegRead(10, 0x1e, 0x173));
	        printk("  phy=11 devid=0x1e register=0x172 value=0x%x ",mtEMiiRegRead(11, 0x1e, 0x172));
	        printk("  phy=11 devid=0x1e register=0x173 value=0x%x \n\r",mtEMiiRegRead(11, 0x1e, 0x173));
	        printk("  phy=12 devid=0x1e register=0x172 value=0x%x ",mtEMiiRegRead(12, 0x1e, 0x172));
	        printk("  phy=12 devid=0x1e register=0x173 value=0x%x \n\r",mtEMiiRegRead(12, 0x1e, 0x173));
			*/
	// *** Tx offset Cal end ***
}

void TX_AMP_Calibration(u8 phyaddr,u8 phyaddr_p0)
{
	u8	all_ana_cal_status;
	u16	ad_cal_comp_out_init;
	int 	calibration_polarity;
	u16	calibration_pair;
	u16	tx_amp_reg_shift; 
	u16	reg_temp;
	u16	tx_amp_temp, tx_amp_reg, cnt=0, tx_amp_reg_100;


		// *** Tx Amp Cal start *******************************************************************************
	mtEMiiRegWrite(phyaddr_p0, 0x1f, 0x0100, 0xc000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00e1, 0x0010);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0145, 0x1010);
	tcMiiStationWrite(phyaddr, 0x1f, 0x0000);
	tcMiiStationWrite(phyaddr, 0x0,  0x0140);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xf808);	// 1e_3e:enable Tx VLD
	mtEMiiRegWrite(phyaddr, 0x1e, 0x017d, (0x8000|DAC_IN_2V));	// 1e_17d:dac_in0_a	
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0181, (0x8000|DAC_IN_2V));	// 1e_181:dac_in1_a	
	mtEMiiRegWrite(phyaddr, 0x1e, 0x017e, (0x8000|DAC_IN_2V));	// 1e_17e:dac_in0_b
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0182, (0x8000|DAC_IN_2V));	// 1e_182:dac_in1_b	
	mtEMiiRegWrite(phyaddr, 0x1e, 0x017f, (0x8000|DAC_IN_2V));	// 1e_17f:dac_in0_c
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0183, (0x8000|DAC_IN_2V));	// 1e_183:dac_in1_c	
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0180, (0x8000|DAC_IN_2V));	// 1e_180:dac_in0_d
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0184, (0x8000|DAC_IN_2V));	// 1e_184:dac_in1_d	
			// 
	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x00db, 0x1100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
    udelay(1);
	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x00dc, 0x0001);	// 1e_dc[0]:rg_txvos_calen
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x1000);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
	
	for(calibration_pair = ANACAL_PAIR_A; calibration_pair <= ANACAL_PAIR_D; calibration_pair ++)
	{
		tx_amp_temp = 0x20;	// start with 0 dB

		if(calibration_pair == ANACAL_PAIR_A)
		{
			// for 7580, from ACD/BH's suggest
			tx_amp_reg_shift = 10;					// 1e_12[15:10]
			tx_amp_reg = 0x12;
			tx_amp_reg_100 = 0x16;
		    mtEMiiRegWrite(phyaddr, 0x1e, tx_amp_reg, (tx_amp_temp<<tx_amp_reg_shift));	
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x1000);				// 1e_dd[4]:rg_txg_calen_a
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x1101);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0001);
		}
		else if(calibration_pair == ANACAL_PAIR_B)
		{
			// for 7580, from ACD/BH's suggest
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x0100);				// 1e_dd[8]:tx_b amp calibration enable
			tx_amp_reg_shift = 8;										// 1e_17[13:8]
			tx_amp_reg = 0x17;
			tx_amp_reg_100 = 0x18;
		    mtEMiiRegWrite(phyaddr, 0x1e, tx_amp_reg, (tx_amp_temp<<tx_amp_reg_shift));	
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x0100);				// 1e_dd[4]:rg_txg_calen_b
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x1100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x1001);	// 1e_dc[12]:rg_zcalen_b
		}
		else if(calibration_pair == ANACAL_PAIR_C)
		{
			// for 7580, from ACD/BH's suggest
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x0010);				// 1e_dd[4]:tx_c amp calibration enable
			tx_amp_reg_shift = 8;										// 1e_19[13:8]
			tx_amp_reg = 0x19;
			tx_amp_reg_100 = 0x20;
		    mtEMiiRegWrite(phyaddr, 0x1e, tx_amp_reg, (tx_amp_temp<<tx_amp_reg_shift));	
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x0010);				// 1e_dd[4]:rg_txg_calen_c
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x1100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0101);	// 1e_dc[8]:rg_zcalen_c
		}
		else //if(calibration_pair == ANACAL_PAIR_D)
		{
			// for 7580, from ACD/BH's suggest
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x0001);				// 1e_dd[0]:tx_d amp calibration enable
			tx_amp_reg_shift = 8;										// 1e_21[13:8]
			tx_amp_reg = 0x21;
			tx_amp_reg_100 = 0x22;
			mtEMiiRegWrite(phyaddr, 0x1e, tx_amp_reg, (tx_amp_temp<<tx_amp_reg_shift));	
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x0001);				// 1e_dd[4]:rg_txg_calen_c
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x1100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0011);	// 1e_dc[4]:rg_zcalen_d	
		}
		
		all_ana_cal_status = allGeAnaCalWait_TxAmp(100, phyaddr_p0); 	// delay 20 usec
		if(all_ana_cal_status == 0)
		{
			all_ana_cal_status = ANACAL_ERROR;	
			printk(" GE Tx amp AnaCal ERROR!   \r\n");
			GECal_status = 0;
		}
	
		ad_cal_comp_out_init = (mtEMiiRegRead(phyaddr_p0, 0x1e, 0x017a)>>8) & 0x1;		// 1e_17a[8]:ad_cal_comp_out
		if(ad_cal_comp_out_init == 1)
		{
			calibration_polarity = -1;
		}
		else
		{
			calibration_polarity = 1;
		}

		//printk(" GE Tx amp check \r\n");
		//printk(" reg0=0x%x, 1f_100=0x%x, 1e_145=0x%x, 1e_185=0x%x  \r\n", mtMiiRegRead(phyaddr, 0), (mtEMiiRegRead(phyaddr, 0x1f, 0x100)), (mtEMiiRegRead(phyaddr, 0x1e, 0x145)), (mtEMiiRegRead(phyaddr, 0x1e, 0x185)));
		//printk(" 1e_db=0x%x, 1e_dc=0x%x, 1e_96=0x%x, 1e_3e=0x%x  \r\n", (mtEMiiRegRead(phyaddr, 0x1e, 0xdb)), (mtEMiiRegRead(phyaddr, 0x1e, 0xdc)), (mtEMiiRegRead(phyaddr, 0x1e, 0x96)), (mtEMiiRegRead(phyaddr, 0x1e, 0x3e)));
		//printk(" 1e_17d=0x%x, 1e_181=0x%x, 1e_17e=0x%x, 1e_182=0x%x  \r\n", (mtEMiiRegRead(phyaddr, 0x1e, 0x17d)), (mtEMiiRegRead(phyaddr, 0x1e, 0x181)), (mtEMiiRegRead(phyaddr, 0x1e, 0x17e)), (mtEMiiRegRead(phyaddr, 0x1e, 0x182)));
		//printk(" 1e_e0=0x%x, 1f_115=0x%x, 1f_426=0x%x, 1e_15=0x%x  \r\n", (mtEMiiRegRead(phyaddr, 0x1e, 0xe0)), (mtEMiiRegRead(phyaddr, 0x1f, 0x115)), (mtEMiiRegRead(phyaddr, 0x1f, 0x426)), (mtEMiiRegRead(phyaddr, 0x1e, 0x15)));
		
		cnt =0;
		while(all_ana_cal_status < ANACAL_ERROR)
		{
			cnt ++;
			tx_amp_temp += calibration_polarity;

			//mtEMiiRegWrite(phyaddr, 0x1e, tx_amp_reg, (reg_temp|(tx_amp_temp<<tx_amp_reg_shift)));		// 1e_12, 1e_17, 1e_19, 1e_21
			mtEMiiRegWrite(phyaddr, 0x1e, tx_amp_reg, (tx_amp_temp<<tx_amp_reg_shift));	
			all_ana_cal_status = allGeAnaCalWait_TxAmp(100, phyaddr_p0); // delay 20 usec

			//printk(" GE Tx amp AnaCal cnt=%d,  comp_out = 0x%x, comp_init = 0x%x, tx_amp_temp=0x%x, 1e_12=0x%x  \r\n", cnt, (mtEMiiRegRead(phyaddr_p0, 0x1e, 0x017a)), ad_cal_comp_out_init, tx_amp_temp, (mtEMiiRegRead(phyaddr, 0x1e, 0x012)));
			
			if(all_ana_cal_status == 0)
			{
				all_ana_cal_status = ANACAL_ERROR;	
				printk(" GE Tx amp AnaCal ERROR!   \r\n");
				GECal_status = 0;
			}
			else if(((mtEMiiRegRead(phyaddr_p0, 0x1e, 0x017a)>>8)&0x1) != ad_cal_comp_out_init) 
			{
				all_ana_cal_status = ANACAL_FINISH;	
				reg_temp = mtEMiiRegRead(phyaddr, 0x1e, tx_amp_reg)&(~0xff00);

				mtEMiiRegWrite(phyaddr, 0x1e, tx_amp_reg, (tx_amp_temp<<tx_amp_reg_shift));		// 1e_12, 1e_17, 1e_19, 1e_21
			}
			else
			{
				//mtEMiiRegWrite(phyaddr, 0x1e, tx_amp_reg, (reg_temp|(tx_amp_temp<<tx_amp_reg_shift)));		// 1e_12, 1e_17, 1e_19, 1e_21
				
				if((tx_amp_temp == 0x3f)||(tx_amp_temp == 0x00))	
				{
					all_ana_cal_status = ANACAL_SATURATION;  // need to FT
					printk(" GE Tx amp AnaCal Saturation!  \r\n");
					GECal_status = 0;
				}
			}
		}
		
		if(all_ana_cal_status == ANACAL_ERROR)
		{	
			tx_amp_temp = 0x20;
			mtEMiiRegWrite(phyaddr, 0x1e, tx_amp_reg, (tx_amp_temp<<tx_amp_reg_shift));
			GECal_status = 0;
			
		}
		else
		{
			printk(" GE Tx amp AnaCal Done! (%d) (0x%x)\r\n", cnt, tx_amp_temp);
			mtEMiiRegWrite(phyaddr, 0x1e, tx_amp_reg, ((tx_amp_temp<<tx_amp_reg_shift)+tx_amp_temp));
			mtEMiiRegWrite(phyaddr, 0x1e, tx_amp_reg_100, ((tx_amp_temp<<tx_amp_reg_shift)+tx_amp_temp));
			GECal_status = 1;
			if(all_ana_cal_status == ANACAL_SATURATION){printk(" GE Tx amp SATURATION! (%d) (0x%x)\r\n", cnt, tx_amp_temp);}

		}
	}
	//
	mtEMiiRegWrite(phyaddr, 0x1e, 0x017d, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x017e, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x017f, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0180, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0181, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0182, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0183, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0184, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0145, 0x1000);
	
	mtEMiiRegWrite(phyaddr_p0, 0x1f, 0x0100, 0x0000);
	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x00db, 0x0000);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x00dc, 0x0000);	// 1e_dc[0]:rg_txvos_calen
	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x00dd, 0x0000);	// 1e_dc[0]:rg_txvos_calen
	
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x0000);	// disable analog calibration circuit
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0000);	// disable Tx offset calibration circuit
	mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xc000);	// disable Tx VLD force mode
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x0000);	// disable Tx offset/amplitude calibration circuit
	// *** Tx Amp Cal end ***

}

void Calibration_all(u8 phyaddr,u8 resisiter) 
{
	u8 phyaddr_p0=9;
	u16 reg_temp;


	//mtEMiiRegWrite(phyaddr, 0x1e, 0x0185, 0x0000);			// disable tx slew control
	//eco_rev = VPint(0xbfb0005c);
	//printk(" 7580 GbE eco_rev (0x%x) &0xff(0x%x)\r\n", eco_rev, (eco_rev&0xff));

	// calibration start ======================================================================================
if(GECal_Rext_flag == 0){GECal_Rext(phyaddr_p0, 100);}
R50Calibration(phyaddr,phyaddr_p0,resisiter);
TX_OFFSET_Calibration(phyaddr,phyaddr_p0);
TX_AMP_Calibration(phyaddr,phyaddr_p0);


	// *** Rx offset Cal start ******************************************************************************
	//mtEMiiRegWrite(phyaddr, 0x1e, 0x0096, 0x8000);					// 1e_96[15]:bypass_tx_offset_cal, Hw bypass, Fw cal

	mtEMiiRegWrite(phyaddr, 0x1e, 0x0037, 0x0033);					// tx/rx_cal_criteria_value
	reg_temp = (mtEMiiRegRead(phyaddr, 0x1e, 0x0039) & (~0x4800));	// [14]: bypass all calibration, [11]: bypass adc offset cal analog
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0039, reg_temp);				// rx offset cal by Hw setup
	reg_temp = (mtEMiiRegRead(phyaddr, 0x1f, 0x0107) & (~0x1000));	// [12]: enable rtune calibration
	mtEMiiRegWrite(phyaddr, 0x1f, 0x0107, reg_temp);				// disable rtune calibration
	reg_temp = (mtEMiiRegRead(phyaddr, 0x1e, 0x0171) & (~0x0180));	// 1e_171[8:7]: bypass tx/rx dc offset cancellation process
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0171, (reg_temp | 0x0180));
	reg_temp = mtEMiiRegRead(phyaddr, 0x1e, 0x0039);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0039, (reg_temp | 0x2000));		// rx offset calibration start
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0039, (reg_temp & (~0x2000)));	// rx offset calibration end
	mdelay(10);														// mdelay for Hw calibration finish
	reg_temp = (mtEMiiRegRead(phyaddr, 0x1e, 0x0171) & (~0x0180));
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0171, reg_temp);


	mtEMiiRegWrite(phyaddr, 0x1f, 0x0100, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0145, 0x1000);
	// *** Rx offset Cal end ***

    mtEMiiRegWrite(phyaddr, 0x1e, 0x00e1, 0x0001);	// 1e_e1[4]:select 1V
	tcMiiStationWrite(phyaddr, 0x0,  0x1200);
    mtEMiiRegWrite(phyaddr, 0x1e, 0x0096, 0x0000);					// 1e_96[15]:bypass_tx_offset_cal, Hw bypass, Fw cal
}

#if debug_flag
void doGePhyALLAnalogCal_R45(u8 phyaddr) 
{
    u8 phyaddr_p0=9;
	u16	reg_temp; 

	phyaddr_p0 = 9;

	
	// calibration start ======================================================================================
	if(GECal_Rext_flag == 0){GECal_Rext(phyaddr, 100);}
R50Calibration(phyaddr,phyaddr_p0,45);
TX_OFFSET_Calibration(phyaddr,phyaddr_p0);
TX_AMP_Calibration(phyaddr,phyaddr_p0);


	// *** Rx offset Cal start ******************************************************************************
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0096, 0x0000);					// 1e_96[15]:bypass_tx_offset_cal, Hw bypass, Fw cal
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0037, 0x0033);					// tx/rx_cal_criteria_value
	reg_temp = (mtEMiiRegRead(phyaddr, 0x1e, 0x0039) & (~0x4800));	// [14]: bypass all calibration, [11]: bypass adc offset cal analog
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0039, reg_temp);				// rx offset cal by Hw setup
	reg_temp = (mtEMiiRegRead(phyaddr, 0x1f, 0x0107) & (~0x1000));	// [12]: enable rtune calibration
	mtEMiiRegWrite(phyaddr, 0x1f, 0x0107, reg_temp);				// disable rtune calibration
	reg_temp = (mtEMiiRegRead(phyaddr, 0x1e, 0x0171) & (~0x0180));	// 1e_171[8:7]: bypass tx/rx dc offset cancellation process
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0171, (reg_temp | 0x0180));
	reg_temp = mtEMiiRegRead(phyaddr, 0x1e, 0x0039);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0039, (reg_temp | 0x2000));		// rx offset calibration start
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0039, (reg_temp & (~0x2000)));	// rx offset calibration end
	mdelay(10);														// mdelay for Hw calibration finish
	reg_temp = (mtEMiiRegRead(phyaddr, 0x1e, 0x0171) & (~0x0180));
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0171, reg_temp);

	//tcMiiStationWrite(phyaddr, 0x0,  reg0_temp);
	mtEMiiRegWrite(phyaddr, 0x1f, 0x0100, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0145, 0x1000);
	// *** Rx offset Cal end ***



	tcMiiStationWrite(phyaddr, 0x0,  0x1200);

}
#endif


void	GECal_tx_offset(u8 phyaddr, u32 delay)
{
	u8	all_ana_cal_status=0, phyaddr_p0=0;
	u16	ad_cal_comp_out_init=0;
	u16	reg0_temp=0, dev1e_145_temp=0; 
	int 	calibration_polarity=0, tx_offset_temp=0;
	u16	cnt=0,calibration_pair=0, cal_temp=0;
	u16	tx_offset_reg_shift=0; 
	u16	tx_offset_reg=0, reg_temp=0;
	//u16	tx_amp_temp=0, tx_amp_temp_eco_lsb=0, tx_amp_temp_eco_msb=0, tx_amp_reg=0, cnt=0, tx_amp_reg_100=0;
	//u16	tx_amp_offset_hbt=0, tx_amp_offset_hbt_p2=0, tx_amp_offset_hbt_p3=0, tx_amp_offset_hbt_p4=0, tx_amp_offset_tbt=0, tx_amp_offset_tbt_p4=0, tx_amp_temp_hbt=0, tx_amp_temp_tst=0, tx_amp_temp_tbt=0;
	//u16	tx_amp_offset_hbt_p0_a=0, tx_amp_offset_hbt_p0_b=0, tx_amp_offset_hbt_p1_a=0, tx_amp_offset_hbt_p1_b=0, tx_amp_offset_hbt_p2_a=0, tx_amp_offset_hbt_p2_b=0, tx_amp_offset_hbt_p3_a=0, tx_amp_offset_hbt_p3_b=0, tx_amp_offset_hbt_p4_a=0, tx_amp_offset_hbt_p4_b=0; 
	//u16	tx_amp_offset_tst=0, tx_amp_offset_tst_p8_p9=0, tx_amp_offset_tst_p10=0, tx_amp_offset_tst_p11=0;
	
	phyaddr_p0 = 9;
	
	tcMiiStationWrite(phyaddr, 0x1f, 0x0000);		// g0 
	reg0_temp = tcMiiStationRead(phyaddr, 0x0); 	// keep the default value
	tcMiiStationWrite(phyaddr, 0x0,  0x0140);		// set [12]AN disable, [8]full duplex, [13/6]1000Mbps
	
	mtEMiiRegWrite(phyaddr, 0x1f, 0x0100, 0xc000);			// BG voltage output
	dev1e_145_temp = mtEMiiRegRead(phyaddr, 0x1e, 0x0145);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0145, 0x1010);			// fix mdi
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0185, 0x0000);			// disable tx slew control
		
	//eco_rev = VPint(0xbfb0005c);
	
	//printk(" 7523 GbE eco_rev (0x%x) &0xff(0x%x)\r\n", eco_rev, (eco_rev&0xff));
	mtEMiiRegWrite(9, 0x1e, 0x0015, 0x0004);	// gating, cutoff thers pair 
	mtEMiiRegWrite(10, 0x1e, 0x0015, 0x0004);	// gating, cutoff thers pair
	mtEMiiRegWrite(11, 0x1e, 0x0015, 0x0004);	// gating, cutoff thers pair
	mtEMiiRegWrite(12, 0x1e, 0x0015, 0x0004);	// gating, cutoff thers pair
		
	// *** Tx offset Cal start *************************************************************************
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x0100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0001);	// 1e_dc[0]:rg_txvos_calen
	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x00db, 0x0100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x00dc, 0x0001);	// 1e_dc[0]:rg_txvos_calen
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0096, 0x8000);	// 1e_96[15]:bypass_tx_offset_cal, Hw bypass, Fw cal
	mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0xf808);	// 1e_3e

	mtEMiiRegWrite(phyaddr, 0x1e, 0x0015, 0x0004);	// gating, cutoff thers pair 
	
	for(calibration_pair = ANACAL_PAIR_A; calibration_pair <= ANACAL_PAIR_D; calibration_pair ++)
	{
		tx_offset_temp = TX_AMP_OFFSET_0mV;
		
		if(calibration_pair == ANACAL_PAIR_A)
		{
			// for 75.., from ACD/BH's suggest
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x1000);				// 1e_dd[12]:rg_txg_calen_a
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x0101);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0001);
			
			mtEMiiRegWrite(phyaddr, 0x1e, 0x017d, (0x8000|DAC_IN_0V));	// 1e_17d:dac_in0_a
			mtEMiiRegWrite(phyaddr, 0x1e, 0x0181, (0x8000|DAC_IN_0V));	// 1e_181:dac_in1_a
			reg_temp = (mtEMiiRegRead(phyaddr, 0x1e, 0x0172) & (~0x3f00));
			tx_offset_reg_shift = 8;									// 1e_172[13:8]
			tx_offset_reg = 0x0172;
			//mtEMiiRegWrite(phyaddr, 0x1e, tx_offset_reg, (reg_temp|(tx_offset_temp<<tx_offset_reg_shift)));
		}
		else if(calibration_pair == ANACAL_PAIR_B)
		{
			// for 75.., from ACD/BH's suggest
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x0100);				// 1e_dd[8]:rg_txg_calen_b
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x0100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x1001);	// 1e_dc[12]:rg_zcalen_b

			mtEMiiRegWrite(phyaddr, 0x1e, 0x017e, (0x8000|DAC_IN_0V));	// 1e_17e:dac_in0_b
			mtEMiiRegWrite(phyaddr, 0x1e, 0x0182, (0x8000|DAC_IN_0V));	// 1e_182:dac_in1_b
			reg_temp = (mtEMiiRegRead(phyaddr, 0x1e, 0x0172) & (~0x003f));
			tx_offset_reg_shift = 0;									// 1e_172[5:0]
			tx_offset_reg = 0x0172;
			//mtEMiiRegWrite(phyaddr, 0x1e, tx_offset_reg, (reg_temp|(tx_offset_temp<<tx_offset_reg_shift)));
		}
		else if(calibration_pair == ANACAL_PAIR_C)
		{
			// for 75.., from ACD/BH's suggest
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x0010);				// 1e_dd[4]:rg_txg_calen_c
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x0100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0101);	// 1e_dc[8]:rg_zcalen_c

			mtEMiiRegWrite(phyaddr, 0x1e, 0x017f, (0x8000|DAC_IN_0V));	// 1e_17f:dac_in0_c
			mtEMiiRegWrite(phyaddr, 0x1e, 0x0183, (0x8000|DAC_IN_0V));	// 1e_183:dac_in1_c
			reg_temp = (mtEMiiRegRead(phyaddr, 0x1e, 0x0173) & (~0x3f00));
			tx_offset_reg_shift = 8;									// 1e_173[13:8]
			tx_offset_reg = 0x0173;
			//mtEMiiRegWrite(phyaddr, 0x1e, tx_offset_reg, (reg_temp|(tx_offset_temp<<tx_offset_reg_shift)));
		}
		else // if(calibration_pair == ANACAL_PAIR_D)
		{
			// for 75.., from ACD/BH's suggest
			//mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x0001);				// 1e_dd[0]:rg_txg_calen_d
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x0100);	// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
			mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0011);	// 1e_dc[4]:rg_zcalen_d

			mtEMiiRegWrite(phyaddr, 0x1e, 0x0180, (0x8000|DAC_IN_0V));	// 1e_180:dac_in0_d
			mtEMiiRegWrite(phyaddr, 0x1e, 0x0184, (0x8000|DAC_IN_0V));	// 1e_184:dac_in1_d
			reg_temp = (mtEMiiRegRead(phyaddr, 0x1e, 0x0173) & (~0x003f));
			tx_offset_reg_shift = 0;									// 1e_173[5:0]
			tx_offset_reg = 0x0173;
			//mtEMiiRegWrite(phyaddr, 0x1e, tx_offset_reg, (reg_temp|(tx_offset_temp<<tx_offset_reg_shift)));
		}
		mtEMiiRegWrite(phyaddr, 0x1e, tx_offset_reg, (reg_temp|(tx_offset_temp<<tx_offset_reg_shift)));	// 1e_172, 1e_173
		
		all_ana_cal_status = allGeAnaCalWait_TxOffset(delay, phyaddr); // delay 20 usec
		if(all_ana_cal_status == 0)
		{
			all_ana_cal_status = ANACAL_ERROR;	
			printk(" GE Tx offset AnaCal ERROR!   \r\n");
		}
		
		printk(" GE Tx offset check \r\n");
		printk(" reg0=0x%x, 1f_100=0x%x, 1e_145=0x%x, 1e_185=0x%x  \r\n", mtMiiRegRead(phyaddr, 0), (mtEMiiRegRead(phyaddr, 0x1f, 0x100)), (mtEMiiRegRead(phyaddr, 0x1e, 0x145)), (mtEMiiRegRead(phyaddr, 0x1e, 0x185)));
		printk(" 1e_db=0x%x, 1e_dc=0x%x, 1e_96=0x%x, 1e_3e=0x%x  \r\n", (mtEMiiRegRead(phyaddr, 0x1e, 0xdb)), (mtEMiiRegRead(phyaddr, 0x1e, 0xdc)), (mtEMiiRegRead(phyaddr, 0x1e, 0x96)), (mtEMiiRegRead(phyaddr, 0x1e, 0x3e)));
		printk(" 1e_17d=0x%x, 1e_181=0x%x, 1e_17e=0x%x, 1e_182=0x%x  \r\n", (mtEMiiRegRead(phyaddr, 0x1e, 0x17d)), (mtEMiiRegRead(phyaddr, 0x1e, 0x181)), (mtEMiiRegRead(phyaddr, 0x1e, 0x17e)), (mtEMiiRegRead(phyaddr, 0x1e, 0x182)));
		printk(" 1e_e0=0x%x, 1f_115=0x%x, 1f_426=0x%x, 1e_15=0x%x  \r\n", (mtEMiiRegRead(phyaddr, 0x1e, 0xe0)), (mtEMiiRegRead(phyaddr, 0x1f, 0x115)), (mtEMiiRegRead(phyaddr, 0x1f, 0x426)), (mtEMiiRegRead(phyaddr, 0x1e, 0x15)));
				
		ad_cal_comp_out_init = (mtEMiiRegRead(phyaddr_p0, 0x1e, 0x017a)>>8) & 0x1;		// 1e_17a[8]:ad_cal_comp_out
		
		if(ad_cal_comp_out_init == 1)
		{
			calibration_polarity = -1;
		}
		else
		{
			calibration_polarity = 1;
		}

		cnt = 0;
		while(all_ana_cal_status < ANACAL_ERROR)
		{
			cnt ++;
			tx_offset_temp += calibration_polarity;

			//printk(" GE Tx offset AnaCal cnt=%d,  comp_out = 0x%x, comp_init = 0x%x, tx_offset_temp=0x%x, cal_temp=0x%x  \r\n", cnt, (mtEMiiRegRead(phyaddr_p0, 0x1e, 0x017a)), ad_cal_comp_out_init, tx_offset_temp, cal_temp);
			//printk(" GE Tx offset AnaCal cnt=%d,  comp_out = 0x%x, comp_init = 0x%x, tx_offset_temp=0x%x, cal_temp=0x%x, 1e_172=0x%x  \r\n", cnt, (mtEMiiRegRead(phyaddr_p0, 0x1e, 0x017a)), ad_cal_comp_out_init, tx_offset_temp, cal_temp, (mtEMiiRegRead(phyaddr, 0x1e, 0x0172)));
		
			if(tx_offset_temp >= 0)
			{
				cal_temp = tx_offset_temp;
			}
			else
			{
				//cal_temp = (1<<(TX_AMP_OFFSET_VALID_BITS-1)) | abs(tx_offset_temp);
				tx_offset_temp = 0x3f;
				cal_temp = 0x3f; 
			}
			mtEMiiRegWrite(phyaddr, 0x1e, tx_offset_reg, (reg_temp|(cal_temp<<tx_offset_reg_shift)));

			all_ana_cal_status = allGeAnaCalWait_TxOffset(delay, phyaddr); // delay 20 usec
			if(all_ana_cal_status == 0)
			{
				all_ana_cal_status = ANACAL_ERROR;	
				printk(" GE Tx offset AnaCal ERROR!   \r\n");
			}
			else if(((mtEMiiRegRead(phyaddr_p0, 0x1e, 0x017a)>>8)&0x1) != ad_cal_comp_out_init) 
			{
				all_ana_cal_status = ANACAL_FINISH;	
			}
			else
			{
				//printk(" GE Tx offset AnaCal 1e_17a=0x%x, cal_temp=0x%x  \r\n", mtEMiiRegRead(phyaddr_p0, 0x1e, 0x017a), cal_temp);
				
				if((tx_offset_temp == -31)||(tx_offset_temp == 0x3f))	
				{
					all_ana_cal_status = ANACAL_SATURATION;  // need to FT
					printk(" GE Tx offset AnaCal Saturation!  \r\n");
				}
			}
		}
		
		if(all_ana_cal_status == ANACAL_ERROR)
		{	
			tx_offset_temp = TX_AMP_OFFSET_0mV;
			mtEMiiRegWrite(phyaddr, 0x1e, tx_offset_reg, (reg_temp|(tx_offset_temp<<tx_offset_reg_shift)));
		}
		else
		{
			printk(" GE Tx offset AnaCal Done! (%d)(0x%x)  \r\n", cnt, cal_temp);
		}
	}
	mtEMiiRegWrite(phyaddr, 0x1e, 0x017d, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x017e, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x017f, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0180, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0181, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0182, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0183, 0x0000);
	mtEMiiRegWrite(phyaddr, 0x1e, 0x0184, 0x0000);
	
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00db, 0x0000);	// disable analog calibration circuit
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00dc, 0x0000);	// disable Tx offset calibration circuit
	mtEMiiRegWrite(phyaddr, 0x1e, 0x003e, 0x0000);	// disable Tx VLD force mode
	mtEMiiRegWrite(phyaddr, 0x1e, 0x00dd, 0x0000);	// disable Tx offset/amplitude calibration circuit
	// *** Tx offset Cal end ***
	GECal_flag = 1;
}



u8 allGeAnaCalWait(u32 delay, u8 phyaddr) 
{
	u8 all_ana_cal_status, phyaddr_p0=9;	

	mtEMiiRegWrite(phyaddr_p0, 0x1e, 0x017c, 0x0001);	// da_calin_flag pull high
	
	udelay(delay);

	all_ana_cal_status = mtEMiiRegRead(phyaddr_p0, 0x1e, 0x017b) & 0x1;
	mtEMiiRegWrite(phyaddr_p0, 0x1e, 0x017c, 0x0000);	// da_calin_flag pull low

	return all_ana_cal_status;
}


u8 allGeAnaCalWait_R45(u32 delay, u8 phyaddr)  
{
	u8 all_ana_cal_status, phyaddr_p0=9;	

	//phyaddr = 8;
	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x017c, 0x0001);	// da_calin_flag pull high
	
	udelay(delay);

	//printk(" 1e_e0(0x%x), 1e_17a(0x%x) \r\n", mtEMiiRegRead( 8, 0x1e, 0xe0), mtEMiiRegRead( 8, 0x1e, 0x17a));
	
	all_ana_cal_status = mtEMiiRegRead( phyaddr_p0, 0x1e, 0x017b) & 0x1;
	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x017c, 0x0000);	// da_calin_flag pull low

	return all_ana_cal_status;
}

u8 allGeAnaCalWait_TxOffset(u32 delay, u8 phyaddr)
{
	u8 all_ana_cal_status, phyaddr_p0=9;	

	//phyaddr = 8;
	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x017c, 0x0001);	// da_calin_flag pull high
	
	udelay(delay);

	//printk(" 1e_172(0x%x), 1e_173(0x%x), 1e_17a(0x%x) \r\n", mtEMiiRegRead(phyaddr, 0x1e, 0x172), mtEMiiRegRead(phyaddr, 0x1e, 0x173), mtEMiiRegRead( 8, 0x1e, 0x17a));
	
	all_ana_cal_status = mtEMiiRegRead( phyaddr_p0, 0x1e, 0x017b) & 0x1;
	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x017c, 0x0000);	// da_calin_flag pull low

	return all_ana_cal_status;
}

u8 allGeAnaCalWait_TxAmp(u32 delay, u8 phyaddr)
{
	u8 all_ana_cal_status, phyaddr_p0=9;	

	//phyaddr = 8;
	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x017c, 0x0001);	// da_calin_flag pull high
	
	udelay(delay);

	//printk(" 1e_12(0x%x), 1e_17(0x%x), 1e_19(0x%x), 1e_21(0x%x), 1e_17a(0x%x) \r\n", mtEMiiRegRead(phyaddr, 0x1e, 0x12), mtEMiiRegRead(phyaddr, 0x1e, 0x17), mtEMiiRegRead(phyaddr, 0x1e, 0x19), mtEMiiRegRead(phyaddr, 0x1e, 0x21), mtEMiiRegRead( 8, 0x1e, 0x17a));
	
	all_ana_cal_status = mtEMiiRegRead( phyaddr_p0, 0x1e, 0x017b) & 0x1;

	//printk(" 1e_12(0x%x), 1e_17(0x%x), 1e_19(0x%x), 1e_21(0x%x), 1e_17b(0x%x), 1e_17a(0x%x) \r\n", mtEMiiRegRead(phyaddr, 0x1e, 0x12), mtEMiiRegRead(phyaddr, 0x1e, 0x17), mtEMiiRegRead(phyaddr, 0x1e, 0x19), mtEMiiRegRead(phyaddr, 0x1e, 0x21), mtEMiiRegRead( 8, 0x1e, 0x17b), mtEMiiRegRead( 8, 0x1e, 0x17a));
	
	mtEMiiRegWrite( phyaddr_p0, 0x1e, 0x017c, 0x0000);	// da_calin_flag pull low

	return all_ana_cal_status;
}



int auto_select_transformer(unsigned int phy)
{
	unsigned int auto_select_transformer_temp,auto_select_transformer_loop,auto_select_transformer_all=0;
	//force dac code 1v
	tcMiiStationWrite(phy, 0x00, 0x0140);
	auto_select_transformer_temp=mtEMiiRegRead(phy, 0x1f, 0x271);
	auto_select_transformer_temp=auto_select_transformer_temp&(0xffe0);
	auto_select_transformer_temp=auto_select_transformer_temp|(0x0003);
	mtEMiiRegWrite(phy, 0x1f, 0x271, auto_select_transformer_temp);
	
	auto_select_transformer_temp=mtEMiiRegRead(phy, 0x1f, 0x269);
	auto_select_transformer_temp=auto_select_transformer_temp&(0x0fff);
	auto_select_transformer_temp=auto_select_transformer_temp|(0x2000);
	mtEMiiRegWrite(phy, 0x1f, 0x269, auto_select_transformer_temp);
	
	auto_select_transformer_temp=mtEMiiRegRead(phy, 0x1f, 0x26f);
	auto_select_transformer_temp=auto_select_transformer_temp&(0x8fff);
	mtEMiiRegWrite(phy, 0x1f, 0x26f, auto_select_transformer_temp);
	
	mtEMiiRegWrite(phy, 0x1e, 0xdd, 0x1000); //cha
	//mtEMiiRegWrite(phy, 0x1f, 0x300, 0x818); //cha
	//mtEMiiRegWrite(phy, 0x1e, 0xdd, 0x0100); //chb
	//mtEMiiRegWrite(phy, 0x1f, 0x300, 0x419); //chb
	//mtEMiiRegWrite(phy, 0x1e, 0xdd, 0x0010); //chc
	//mtEMiiRegWrite(phy, 0x1f, 0x300, 0x21a); //chc
	//mtEMiiRegWrite(phy, 0x1e, 0xdd, 0x0001); //chd
	//mtEMiiRegWrite(phy, 0x1f, 0x300, 0x11b); //chd
	
	printk("dd=1000\r\n");
	
	mtEMiiRegWrite(phy, 0x1e, 0x17d, 0x80f0);
	mtEMiiRegWrite(phy, 0x1e, 0x17e, 0x80f0);
	mtEMiiRegWrite(phy, 0x1e, 0x17f, 0x80f0);
	mtEMiiRegWrite(phy, 0x1e, 0x180, 0x80f0);
	mtEMiiRegWrite(phy, 0x1e, 0x181, 0x80f0);
	mtEMiiRegWrite(phy, 0x1e, 0x182, 0x80f0);
	mtEMiiRegWrite(phy, 0x1e, 0x183, 0x80f0);
	mtEMiiRegWrite(phy, 0x1e, 0x184, 0x80f0);
	//force pGA gain 
	auto_select_transformer_temp=toKenRingRead(phy, 0x1, 0xf, 0x10);
	auto_select_transformer_temp=auto_select_transformer_temp&(0xe07fff);
	auto_select_transformer_temp=auto_select_transformer_temp|(0xf8000);
	toKenRingWrite(phy, 0x1, 0xf, 0x10,auto_select_transformer_temp);
	//printk(" PHY=%d  1 f 10 value=0x%x ", phy , auto_select_transformer_temp);
	
	auto_select_transformer_temp=toKenRingRead(phy, 0x1, 0xf, 0x11);
	auto_select_transformer_temp=auto_select_transformer_temp&(0x030303);
	auto_select_transformer_temp=auto_select_transformer_temp|(0x7c7c7c);
	toKenRingWrite(phy, 0x1, 0xf, 0x11,auto_select_transformer_temp);
	//printk(" PHY=%d  1 f 11 value=0x%x \n\r", phy , auto_select_transformer_temp);

    mtEMiiRegWrite(phy, 0x1e, 0xc9, 0xffff);
	mtEMiiRegWrite(phy, 0x1e, 0x151, 0x0012);

	
	
	//read data back
if(phy == 9) { mtEMiiRegWrite(phy, 0x1f, 0x15, 0x1161);}
if(phy == 10) { mtEMiiRegWrite(phy, 0x1f, 0x15, 0x3161);}
if(phy == 11) { mtEMiiRegWrite(phy, 0x1f, 0x15, 0x5161);}
if(phy == 12) { mtEMiiRegWrite(phy, 0x1f, 0x15, 0x7161);}
	//mtEMiiRegWrite(phy, 0x1f, 0x15, 0x1161); //cha
	//mtEMiiRegWrite(phy, 0x1f, 0x15, 0x1162); //chb
	//mtEMiiRegWrite(phy, 0x1f, 0x15, 0x1163); //chc
	//mtEMiiRegWrite(phy, 0x1f, 0x15, 0x1164); //chd
	printk("cha\r\n");
	//delay1ms(500);
	msleep(500);
	for(auto_select_transformer_loop = 0; auto_select_transformer_loop<= 3; auto_select_transformer_loop++)
	{
		auto_select_transformer_temp=mtEMiiRegRead(phy, 0x1f, 0x1a);
		printk(" PHY=%d  reg=0x1a value=0x%x ", phy , auto_select_transformer_temp);
		auto_select_transformer_all=auto_select_transformer_all+(auto_select_transformer_temp&0xff);
	}
	printk("\r\n");
	
	//switch to default
	mtEMiiRegWrite(phy, 0x1f, 0x15, 0x1000);
	
	mtEMiiRegWrite(phy, 0x1e, 0xdd, 0x0000);
	mtEMiiRegWrite(phy, 0x1e, 0x17d, 0x0000);
	mtEMiiRegWrite(phy, 0x1e, 0x17e, 0x0000);
	mtEMiiRegWrite(phy, 0x1e, 0x17f, 0x0000);
	mtEMiiRegWrite(phy, 0x1e, 0x180, 0x0000);
	mtEMiiRegWrite(phy, 0x1e, 0x181, 0x0000);
	mtEMiiRegWrite(phy, 0x1e, 0x182, 0x0000);
	mtEMiiRegWrite(phy, 0x1e, 0x183, 0x0000);
	mtEMiiRegWrite(phy, 0x1e, 0x184, 0x0000);
	//mtEMiiRegWrite(phy, 0x1f, 0x300, 0x0000);
	
    mtEMiiRegWrite(phy, 0x1e, 0xc9, 0x0fff);
	mtEMiiRegWrite(phy, 0x1e, 0x151, 0x0000);	
	toKenRingWrite(phy, 0x1, 0xf, 0x10,0x0020000);
	toKenRingWrite(phy, 0x1, 0xf, 0x11,0x0000000);
	//check value and return value 0-->disceret  1-->TXMR
    if(auto_select_transformer_temp >= 0xe0) 
	{ 
	   if(phy == 9 ) {transformer_status=transformer_status&0xfe;}
       if(phy == 10 ) {transformer_status=transformer_status&0xfd;}
	   if(phy == 11 ) {transformer_status=transformer_status&0xfb;}
	   if(phy == 12 ) {transformer_status=transformer_status&0xf7;}
       return 0;
	} 
	else 
	{
		if(phy == 9 ) {transformer_status=transformer_status|0x01;}
		if(phy == 10 ) {transformer_status=transformer_status|0x02;}
		if(phy == 11 ) {transformer_status=transformer_status|0x04;}
		if(phy == 12 ) {transformer_status=transformer_status|0x08;}		
		return 1;
	}
	
}

extern  int settingANSpeed (int argc, char *argv[], void *p);
extern int doPhyForceEEE (int argc, char *argv[], void *p);
int Ghy_SetANSpeed(unsigned int phyaddr, int speed, int duplex )
{ 
  char* argv[4]; int argc;
  char addr[25];
  char spd[25];
  char DorH[25];
  snprintf(addr,25,"%d",phyaddr);
  snprintf(spd,25,"%d",speed);
  snprintf(DorH,25,"%d",speed);
  argv[0] =  "setANSpeed";
  argv[1] = addr;
  argv[2] = spd;
  argv[3] = DorH;
  argc = sizeof(argv) / sizeof (char*) ;  
  return settingANSpeed ( argc, argv, NULL);
}

int Ghy_SetEEE(unsigned int phyaddr ,char *option)	
{	
  char* argv[3]; int argc;
  char addr[25];
  snprintf(addr,25,"%d",phyaddr);
  argv[0] =  "forceEEE";
  argv[1] = addr;
  argv[2] = option;  
  argc = sizeof(argv) / sizeof (char*) ;  
  return doPhyForceEEE( argc, argv, NULL);
}
EXPORT_SYMBOL(Ghy_SetANSpeed);
EXPORT_SYMBOL(Ghy_SetEEE);
