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

#ifndef _TCETHERPHY__DEF
#define _TCETHERPHY__DEF
#include "tcetherphy_define.h"

//////////////////////////////////////////////////////
//define_TXMR_or_discrete TXMR =0-->5r discrete_TXMR
//define_TXMR_or_discrete TXMR =1-->0r discrete_TXMR
//define_TXMR_or_discrete TXMR =2-->5r TXMR
//define_TXMR_or_discrete TXMR =3-->0r TXMR

/*
#if TCSUPPORT_ETHPHY_5R_DIS
  //Discrete 5R driver code
#define define_TXMR_or_discrete_TXMR  0
#endif

#if TCSUPPORT_ETHPHY_0R_DIS
  //Discrete 0R driver code
  #define define_TXMR_or_discrete_TXMR  1
#endif

#if TCSUPPORT_ETHPHY_5R_TRA
  //Tradition 5R driver code
  #define define_TXMR_or_discrete_TXMR  2
#endif

#if TCSUPPORT_ETHPHY_0R_TRA
  //Tradition 0R driver code
  #define define_TXMR_or_discrete_TXMR  3
#endif
*/

u8   tcSWVer_COMMON =1; 

u8	ephy_addr_base = EPHY_ADDR_P0;		// replace  "mac_p->enetPhyAddr"
u8   tcPhyVer = 99; 
#ifdef TCPHY_SUPPORT

#ifdef TCPHY_DEBUG_DISP_LEVEL
u8 tcPhy_disp_level = 2; 				// default level 2
#else
u8 	tcPhy_disp_level = 0; 				// turn all message OFF for formal release
#endif
uint 	mtSkewCal_disp_level=0;
#endif

unsigned int slt_not_excute_loop,mdi_resister=5,transformer_status=0;

u16 checked_atoi(char *val)
{
	int temp = 0;
	#ifdef TCPHY_DEBUG
    // only check 1st char
    if(val[0]<'0' || val[0]>'9') return (0xffff);
	#endif
	temp = atoi(val);
	//int ret;
	//ret = kstrtoint(val, 16, &temp);
	//printf("Atoi %x\r\n", temp);
    //return (ret & 0xffff);
	return (temp & 0xffff);
}
		
#endif /* _TCETHERPHY__DEF */

