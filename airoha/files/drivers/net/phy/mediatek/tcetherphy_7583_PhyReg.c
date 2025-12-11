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


#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_AUTOBENCH) 
#include <ecnt_hook/ecnt_hook.h>
#endif

#endif //LINUX_OS


/************************************************************************
*                        Data for EPHY  (3)
**************************************************************************/
#ifdef TCPHY_FIELD_DBGTEST
#define MAX_RECORD_TC_ETHER_PHY_STR_NUM 256
#define MAX_RECORD_TC_ETHER_PHY_STR_LEN 16

char tc_ether_phy[MAX_RECORD_TC_ETHER_PHY_STR_NUM][MAX_RECORD_TC_ETHER_PHY_STR_LEN];
char RASVersion[3]="v4";
#endif

/************************************************************************
*            Variables for EPHY  (4)
**************************************************************************/

tcphy_mr0_reg_t 	mr0;
tcphy_mr4_reg_t 	mr4; 
tcphy_mr5_reg_t 	mr5;
tcphy_mr6_reg_t 	mr6;
tcphy_l0r28_reg_t 	mr28; 		// L0R28
tcphy_l3r18_reg_t 	mrl3_18; 	// L3R17
tcphy_1ErA2_reg_t 	mr1E_A2;
tcphy_7r3D_reg_t 	mr7_3D;
// for multiple phy support
tcphy_mr1_reg_t 		Nmr1[TCPHY_PORTNUM];
tcphy_l0r25_reg_t 		Nmr25[TCPHY_PORTNUM];

u8   tcSWVer_REG =1; 

#ifdef TCPHY_SUPPORT



#include <linux/phy.h>
#include <linux/device.h>
#include <linux/module.h>

int tcMiiStationRead(u32 phy_addr, u32 phy_reg) {

	//printk("Executing legacy module of mdio\n");
	struct mii_bus *mdio_bus ;
	struct device *dev ;
	struct phy_device *phydev;

	char bus_id[MII_BUS_ID_SIZE +3];

	snprintf(bus_id, sizeof(bus_id), PHY_ID_FMT,
                   "mt7530-0",phy_addr);  
	dev = bus_find_device_by_name(&mdio_bus_type, NULL, bus_id);
	if ( !dev ) {
		pr_err("Failed to find device\n") ;
		return -ENODEV;
	}
	
	phydev = to_phy_device(dev);
	if ( !phydev ) {
		pr_err("Failed to find phy device\n") ;
		return -ENODEV;
	}
	
	mdio_bus = phydev->mdio.bus ;
	
	if ( !mdio_bus ) {
		pr_err("Failed to find mdio_bus\n") ;
		return -ENODEV;
	}
	
	put_device(dev);

	u32 read_data = mdiobus_read(mdio_bus, phy_addr, phy_reg) ;

	return read_data ;
}
//EXPORT_SYMBOL(tcMiiStationRead);


int tcMiiStationWrite(u32 phy_addr, u32 phy_reg, u32 phy_data) {
	struct mii_bus *mdio_bus ;
	struct device *dev ;
	struct phy_device *phydev;

	char bus_id[MII_BUS_ID_SIZE +3];


	snprintf(bus_id, sizeof(bus_id), PHY_ID_FMT,
                   "mt7530-0",phy_addr);  
	dev = bus_find_device_by_name(&mdio_bus_type, NULL, bus_id);
	if ( !dev ) {
		pr_err("Failed to find device\n") ;
		return -ENODEV;
	}
	
	phydev = to_phy_device(dev);
	if ( !phydev ) {
		pr_err("Failed to find phy device\n") ;
		return -ENODEV;
	}
	
	mdio_bus = phydev->mdio.bus ;
	
	if ( !mdio_bus ) {
		pr_err("Failed to find mdio_bus\n") ;
		return -ENODEV;
	}
	
	put_device(dev);
	mdiobus_write(mdio_bus, phy_addr, phy_reg, phy_data) ;
	return 0 ;
}
//EXPORT_SYMBOL(tcMiiStationWrite);

MODULE_LICENSE("GPL v2");


void mtMiiRegWrite(int port_num, int reg_num, int reg_data)
{
	tcMiiStationWrite(port_num, reg_num, reg_data);
}
u32 mtMiiRegRead(u8 port_num,u8 reg_num)
{
	return(tcMiiStationRead(port_num, reg_num));
}
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_LOOPDETECT) || !(!defined(TCSUPPORT_AUTOBENCH) || !defined(TCSUPPORT_CPU_MT7520))
//EXPORT_SYMBOL(mtMiiRegWrite);
//EXPORT_SYMBOL(mtMiiRegRead);
#endif/*TCSUPPORT_COMPILE*/
//CL22 Write
void mtRegWrite_CL22(u32 port_num,u32 dev_num,u32 reg_num,u32 reg_data)
{
   tcMiiStationWrite(port_num, 0x1f, dev_num);
   tcMiiStationWrite(port_num, reg_num, reg_data);
   tcMiiStationWrite(port_num, 0x1f, 0);
}
//CL22 read
u32 mtRegRead_CL22(u32 port_num,u32 dev_num,u32 reg_num)
{
   u32 value=0;
   tcMiiStationWrite(port_num, 0x1f, dev_num);
   value = tcMiiStationRead(port_num, reg_num);
   tcMiiStationWrite(port_num, 0x1f, 0);
   return value;
}
//CL45 read
u32 mtEMiiRegRead(u32 port_num, u32 dev_num, u32 reg_num)
{
    const u16 MMD_Control_register=0xD;  	// 0xd=13
    const u16 MMD_addr_data_register=0xE;	// 0xe=14	
  	const u16 page_reg=31;
	u32 value=0;
	tcMiiStationWrite(port_num, page_reg, 0x00); //switch to main page
	tcMiiStationWrite(port_num, MMD_Control_register, (0<<14)+dev_num);
	tcMiiStationWrite(port_num, MMD_addr_data_register, reg_num);
	tcMiiStationWrite(port_num, MMD_Control_register, (1<<14)+dev_num);
	value = tcMiiStationRead(port_num, MMD_addr_data_register);
	//printk("* doPhyMMDRead_CL22=>phyaddr=%d,  dev_addr=%d, data_addr=0x%04lX , value=0x%04lX\r\n", port_num, dev_num, reg_num, value);
	return(value); 	
}
//CL45 write
void mtEMiiRegWrite(u32 port_num,u32 dev_num,u32 reg_num,u32 reg_data)
{
    const u16 MMD_Control_register=0xD;
    const u16 MMD_addr_data_register=0xE;
  	const u16 page_reg=31;
	tcMiiStationWrite(port_num, page_reg, 0x00); //switch to main page
	tcMiiStationWrite(port_num, MMD_Control_register, (0<<14)+dev_num);
	tcMiiStationWrite(port_num, MMD_addr_data_register, reg_num);
	tcMiiStationWrite(port_num, MMD_Control_register, (1<<14)+dev_num);
	tcMiiStationWrite(port_num, MMD_addr_data_register, reg_data);
	TCPHYDISP3("* doPhyMMDWrite_CL22=> phyaddr=%d, dev_addr=%d, data_addr=0x%04X , value=0x%04X\r\n", port_num, dev_num, reg_num, reg_data);
}

unsigned int toKenRingRead(unsigned int phy, unsigned int ch, unsigned int node, unsigned int data_addr)
{
	unsigned int data,last_page,high_byte,low_byte;
    if ((tcMiiStationRead(phy, 0) & SMIREG_PWDN_BIT) == 0) {
	  last_page=tcMiiStationRead(phy, 0x1f);
	  tcMiiStationWrite(phy, 0x1f, 0x52b5);
	  data=0xa000|(ch<<11)|(node<<7)|(data_addr<<1);
	  tcMiiStationWrite(phy, 0x10, data);
	  low_byte=tcMiiStationRead(phy, 0x11);
	  high_byte=tcMiiStationRead(phy, 0x12);
	  tcMiiStationWrite(phy, 0x1f, last_page);
	  data=(high_byte*65536)+low_byte;
	  return data;
    }else{
      TCPHYDISP3("toKenRingRead  invalid! \r\n");
	  return 0xffff;
	}
}

void toKenRingWrite(unsigned int phy, unsigned int ch, unsigned int node, unsigned int data_addr,unsigned int value)
{
	unsigned int data,last_page;
    if ((tcMiiStationRead(phy, 0) & SMIREG_PWDN_BIT) == 0) {
	  last_page=tcMiiStationRead(phy, 0x1f);
	  tcMiiStationWrite(phy, 0x1f, 0x52b5);
	  data=value&0xffff;
	  tcMiiStationWrite(phy, 0x11, data);
	  data=(value&0xffff0000)/65536;
	  tcMiiStationWrite(phy, 0x12, data);
	  data=0x8000|(ch<<11)|(node<<7)|(data_addr<<1);
	  tcMiiStationWrite(phy, 0x10, data);
	  tcMiiStationWrite(phy, 0x1f, last_page);
    }else{
 	  TCPHYDISP3("toKenRingWrite invalid! \r\n");
    }	  
}


unsigned int buck_pbus_read( int phy, unsigned int register_address)
{
	unsigned int last_page,data,high_byte,low_byte;


		last_page=tcMiiStationRead(phy, 0x1f);
		tcMiiStationWrite(phy, 0x1f, 0x0004);
		tcMiiStationWrite(phy, 0x10, 0x0000);
		data=(register_address&0xffff0000)>>16;
        tcMiiStationWrite(phy, 0x15, data);
		data=register_address&0xffff;
        tcMiiStationWrite(phy, 0x16, data);
		high_byte=tcMiiStationRead(phy, 0x17);
		low_byte=tcMiiStationRead(phy, 0x18);
		tcMiiStationWrite(phy, 0x1f,last_page);
		data=(high_byte<<16)+low_byte;
		return data;
}

void buck_pbus_write( int phy, unsigned int register_address,unsigned int value)
{
	unsigned int last_page,data;


		last_page=tcMiiStationRead(phy, 0x1f);
		tcMiiStationWrite(phy, 0x1f, 0x0004);
		tcMiiStationWrite(phy, 0x10, 0x0000);
		data=(register_address&0xffff0000)>>16;
        tcMiiStationWrite(phy, 0x11, data);
		data=register_address&0xffff;
        tcMiiStationWrite(phy, 0x12, data);
		data=(value&0xffff0000)>>16;
        tcMiiStationWrite(phy, 0x13, data);
		data=value&0xffff;
        tcMiiStationWrite(phy, 0x14, data);
		tcMiiStationWrite(phy, 0x1f, last_page);
		
}

#if defined(TCSUPPORT_AUTOBENCH) && defined(TCSUPPORT_CPU_MT7520)
EXPORT_SYMBOL(mtEMiiRegWrite);
EXPORT_SYMBOL(mtEMiiRegRead);
#endif
#ifdef LINUX_OS // allen_20130926 : merge 7502 & 7510/20

u32 mtPhyReadReg(u8 port_num, u8 reg_num)  // for GEPHY
{
    u32 val, val_r31;
    u32 phyAddr;
	
	if(ephy_addr_base == 0)
	{
		if((tcPhyVer == tcPhyVer_7583))	
			ephy_addr_base += EPHY_ADDR_P0;
	}
		
	phyAddr = ephy_addr_base + port_num;
    tcMiiStationWrite(phyAddr, 31, 0);
     
    if (tcPhyVer!=tcPhyVer_2031 && (reg_num<16 || reg_num==31))
	{     
        val = tcMiiStationRead(phyAddr, reg_num); 
        //printk("mtPhyReadReg:phyAddr =%d, reg_num =0x%02lX, val =0x%04lx\r\n",phyAddr,reg_num,val);
    }
    else
	{
        val_r31 = tcMiiStationRead(phyAddr, 31); // remember last page
        // set page to L0 if necessary
        if (val_r31 != 0x8000) 
		{
            tcMiiStationWrite(phyAddr, 31, 0x8000);
        }
        // read reg
        val = tcMiiStationRead(phyAddr, reg_num); 
        // restore page if necessary
        if (val_r31 != 0x8000) 
		{
            tcMiiStationWrite(phyAddr, 31, val_r31);
        }
    }

    // update variables
    switch(reg_num){
        
    case 0:
        mr0.main_reset = (val>>15)&0x00000001;
        mr0.force_speed    = (val>>13)&0x00000001;
        mr0.autoneg_enable = (val>>12)&0x00000001;
        mr0.powerdown      = (val>>11)&0x00000001;
        mr0.force_duplex   = (val>>8)&0x00000001;
        break;
        
    case 1:
        //mr1.autoneg_complete = (val>>5)&0x00000001;       
        //mr1_link_status_reg = val;
        //mr1.value = val;
        //mr1.link_status_prev = mr1.link_status; 
        //mr1.link_status = (val>>2)&0x00000001;
        Nmr1[port_num].value = val;
        Nmr1[port_num].link_status_prev = Nmr1[port_num].link_status;
        Nmr1[port_num].link_status = (val>>2)&0x00000001;
        //if(Nmr1[port_num].link_status !=  Nmr1[port_num].link_status_prev)
		//{
	    //    printk("mtPhyReadReg case 1 :Nmr1[%d].value =0x%04lx\r\n", port_num,Nmr1[port_num].value);
    	//    printk("mtPhyReadReg case 1 :Nmr1[%d].link_status_prev =0x%04lx\r\n",port_num,Nmr1[port_num].link_status_prev );
        //	printk("mtPhyReadReg case 1 :Nmr1[%d].link_status =0x%04lx\r\n", port_num,Nmr1[port_num].link_status);
        //}
        break;

    case 4:
        mr4.able100F = (val>>8)&0x0001;     
        mr4.able100H = (val>>7)&0x0001;     
        mr4.able10F = (val>>6)&0x0001;  
        mr4.able10H = (val>>5)&0x0001;  
        mr4.selector_field = (val)&0x001f;
        break;

    case 5:
        mr5.able100F = (val>>8)&0x0001;     
        mr5.able100H = (val>>7)&0x0001;     
        mr5.able10F = (val>>6)&0x0001;  
        mr5.able10H = (val>>5)&0x0001;  
        mr5.selector_field = (val)&0x001f;
		mr5.LPNextAble = (val>>15)&0x0001;
        break;
        
    case 6:     
        mr6.lp_np_able = (val>>3)&0x0001;
        mr6.lp_autoneg_able = (val)&0x0001;
        break;
        
    default:
        break;      
    }   
    return (val);
}
#endif

u32 tcPhyReadReg(u8 port_num, u8 reg_num)  // for FEPHY
{
    u32 val, val_r31;
    u32 phyAddr;

	if(ephy_addr_base == 0) 
	{
		if((tcPhyVer == tcPhyVer_7583))	
			ephy_addr_base += EPHY_ADDR_P0;
	}
	phyAddr = ephy_addr_base + port_num;

    if (tcPhyVer!=tcPhyVer_2031 && (reg_num<16 || reg_num==31))
    {
        val = tcMiiStationRead(phyAddr, reg_num); 
    }
    else
    {
        val_r31 = tcMiiStationRead(phyAddr, 31); // remember last page
        // set page to L0 if necessary
        if (val_r31 != 0x8000) 
        {
            tcMiiStationWrite(phyAddr, 31, 0x8000);
        }
        // read reg
        val = tcMiiStationRead(phyAddr, reg_num); 
        // restore page if necessary
        if (val_r31 != 0x8000) 
        {
            tcMiiStationWrite(phyAddr, 31, val_r31);
        }
    }

    // update variables
    switch(reg_num)
    {    
    	case 0:
        	mr0.main_reset 		= (val>>15)&0x00000001;
        	mr0.force_speed    	= (val>>13)&0x00000001;
        	mr0.autoneg_enable 	= (val>>12)&0x00000001;
        	mr0.powerdown      	= (val>>11)&0x00000001;
        	mr0.force_duplex   	= (val>>8)&0x00000001;
        	break;
        
    	case 1:
        	//mr1.autoneg_complete = (val>>5)&0x00000001;       
        	//mr1_link_status_reg = val;
        	//mr1.value = val;
        	//mr1.link_status_prev = mr1.link_status; 
        	//mr1.link_status = (val>>2)&0x00000001;
        	Nmr1[port_num].value 			= val;
        	Nmr1[port_num].link_status_prev = Nmr1[port_num].link_status;
        	Nmr1[port_num].link_status 		= (val>>2)&0x00000001;
        	break;

    	case 4:
        	mr4.able100F 		= (val>>8)&0x0001;     
        	mr4.able100H 		= (val>>7)&0x0001;     
        	mr4.able10F 		= (val>>6)&0x0001;  
        	mr4.able10H 		= (val>>5)&0x0001;  
        	mr4.selector_field 	= (val)&0x001f;
        	break;

    	case 5:
        	mr5.able100F 		= (val>>8)&0x0001;     
        	mr5.able100H 		= (val>>7)&0x0001;     
        	mr5.able10F 		= (val>>6)&0x0001;  
        	mr5.able10H 		= (val>>5)&0x0001;  
        	mr5.selector_field 	= (val)&0x001f;
			mr5.LPNextAble 		= (val>>15)&0x0001;
        	break;
        
    	case 6:     
        	mr6.lp_np_able 		= (val>>3)&0x0001;
        	mr6.lp_autoneg_able = (val)&0x0001;
        	break;
        
	    case 25:           
        	Nmr25[port_num].err_over_cnt_prev 	= Nmr25[port_num].err_over_cnt;
        	Nmr25[port_num].err_over_cnt		= (val & 0x0000007ff);
        	break;
        
    	case 28:
        	mr28.lch_sig_detect  	= (val>>15)&0x0001;
        	mr28.lch_rx_linkpulse	= (val>>14)&0x0001;
        	mr28.lch_linkup_100  	= (val>>13)&0x0001;
        	mr28.lch_linkup_10   	= (val>>12)&0x0001;
        	mr28.lch_linkup_mdi  	= (val>>11)&0x0001; 	// after LEM
        	mr28.lch_linkup_mdix 	= (val>>10)&0x0001; 	// after LEM
        	mr28.lch_descr_lock  	= (val>>9)&0x0001; 		// after LEM
        	mr28.mdix_status  		= (val>>5)&0x0001; 		/* {0:mdi,1:mdix} */   
        	mr28.tx_amp_save  		= (val>>3)&0x0003; 		/* 0:100%, 1:90%, 2:80%, 3:70% */
        	mr28.final_duplex 		= (val>>2)&0x0001; 		/* {0:half-duplex, 1:full-duplex} */
        	mr28.final_speed  		= (val>>1)&0x0001; 		/* {0:10, 1:100} */
        	mr28.final_link			= (val)&0x0001; 		/* {0:linkdown, 1:linkup} */      
        	break;
        
    	default:
        	break;      
    }   
    return (val);
}

void tcPhyWriteReg(u8 port_num,u8 reg_num,u32 reg_data)
{
    u32 val_r31;
    u32 phyAddr;

	if(ephy_addr_base == 0) // allen_20140822
	{
		if((tcPhyVer == tcPhyVer_7583))	
			ephy_addr_base += EPHY_ADDR_P0;
	}
	phyAddr = ephy_addr_base + port_num;

    val_r31 = tcMiiStationRead(phyAddr, 31); // remember last page
    // set page if necessary
    if (val_r31 != 0x8000) 
    {
        tcMiiStationWrite(phyAddr, 31, 0x8000); // page to L0
    }
    tcMiiStationWrite(phyAddr, reg_num, reg_data); 
    // restore page if necessary
    if (val_r31 != 0x8000) 
    {
        tcMiiStationWrite(phyAddr, 31, val_r31);
    }
}

// read Local Reg
u32 tcPhyReadLReg(u8 port_num,u8 page_num,u8 reg_num)
{
    u32 val, val_r31;
    u32 phyAddr;
    u32 pageAddr = (page_num<<12)+0x8000;

	if(ephy_addr_base == 0) // allen_20140822
	{
		if((tcPhyVer == tcPhyVer_7583))	
			ephy_addr_base += EPHY_ADDR_P0;
	}
	phyAddr = ephy_addr_base + port_num;

    val_r31 = tcMiiStationRead(phyAddr, 31);  // remember last page
    // set page if necessary
    if (val_r31 != pageAddr) 
	{
        tcMiiStationWrite(phyAddr, 31, pageAddr); // switch to page Lx
    }
    val = tcMiiStationRead(phyAddr, reg_num); 
    // restore page if necessary
    if (val_r31 != pageAddr) 
	{
        tcMiiStationWrite(phyAddr, 31, val_r31);
    }
	
    if (page_num==3) 
	{
		switch(reg_num)
		{
			case 18:
               mrl3_18.lp_eee_10g = (val>>3)&0x0001;
               mrl3_18.lp_eee_1000 = (val>>2)&0x0001;
               mrl3_18.lp_eee_100 = (val>>1)&0x0001;
               break;
			default:
               break; 
		}
    }
    return val;
}

// write Local Reg
void tcPhyWriteLReg(u8 port_num,u8 page_num,u8 reg_num,u32 reg_data)
{
    u32 val_r31;
    u32 phyAddr;
    u32 pageAddr = (page_num<<12)+0x8000;

	if(ephy_addr_base == 0) // allen_20140822
	{
		if((tcPhyVer == tcPhyVer_7583))	
			ephy_addr_base += EPHY_ADDR_P0;
	}
	phyAddr = ephy_addr_base + port_num;

    val_r31 = tcMiiStationRead(phyAddr, 31);  // remember last page
    // set page if necessary
    if (val_r31 != pageAddr) 
	{
        tcMiiStationWrite(phyAddr, 31, pageAddr); // switch to page Lx  
    }
    tcMiiStationWrite(phyAddr, reg_num, reg_data); 
    // restore page if necessary
    if (val_r31 != pageAddr) 
	{
        tcMiiStationWrite(phyAddr, 31, val_r31);
    }
}

// read Global Reg
u32 tcPhyReadGReg(u8 port_num,u8 page_num,u8 reg_num)
{
    u32 val, val_r31;
    u32 phyAddr;
    u32 pageAddr = (page_num<<12);

	if(ephy_addr_base == 0) // allen_20140822
	{
		if((tcPhyVer == tcPhyVer_7583))	
			ephy_addr_base += EPHY_ADDR_P0;
	}
	phyAddr = ephy_addr_base + port_num;

    val_r31 = tcMiiStationRead(phyAddr, 31);  // remember last page
    // set page if necessary
    if (val_r31 != pageAddr) 
	{
        tcMiiStationWrite(phyAddr, 31, pageAddr); // switch to page Gx  
    }
    val = tcMiiStationRead(phyAddr, reg_num); 
    // restore page if necessary
    if (val_r31 != pageAddr) 
	{
        tcMiiStationWrite(phyAddr, 31, val_r31);
    }
    
    return val;
}

// write Global Reg
void tcPhyWriteGReg(u8 port_num,u8 page_num,u8 reg_num,u32 reg_data)
{
    u32 val_r31;
    u32 phyAddr;
    u32 pageAddr = (page_num<<12);

	if(ephy_addr_base == 0) // allen_20140822
	{
		if((tcPhyVer == tcPhyVer_7583))	
			ephy_addr_base += EPHY_ADDR_P0;
	}
	phyAddr = ephy_addr_base + port_num;

    val_r31 = tcMiiStationRead(phyAddr, 31);  // remember last page
    // set page if necessary
    if (val_r31 != pageAddr) 
	{
        tcMiiStationWrite(phyAddr, 31, pageAddr); // switch to page Gx
    }
    tcMiiStationWrite(phyAddr, reg_num, reg_data); 
    // restore page if necessary
    if (val_r31 != pageAddr) 
	{
        tcMiiStationWrite(phyAddr, 31, val_r31);
    }
}

#ifdef LINUX_OS // allen_20130926 : merge 7502 & 7510/20
u32 mtPhyReadGReg(u32 port_num, u32 dev_num, u32 reg_num)
{
    u32 val;
    
    val = mtEMiiRegRead(port_num, dev_num, reg_num);

    // update variables
    switch(reg_num)
    {
	case 0xA2:
    	mr1E_A2.lch_SignalDetect  = (val>>15)&0x0001;
        mr1E_A2.lch_LinkPulse= (val>>14)&0x0001;
        mr1E_A2.lch_DescramblerLock1000  = (val>>13)&0x0001;
        mr1E_A2.lch_DescramblerLock100   = (val>>12)&0x0001;
        mr1E_A2.lch_LinkStatus1000_OK  = (val>>11)&0x0001;
        mr1E_A2.lch_LinkStatus100_OK = (val>>10)&0x0001; 
        mr1E_A2.lch_LinkStatus10_OK  = (val>>9)&0x0001; 
        mr1E_A2.lch_MrPageRx  = (val>>8)&0x0001; 
        mr1E_A2.lch_MrAutonegComplete  = (val>>7)&0x0001; 
        mr1E_A2.da_mdix  = (val>>6)&0x0001; 
        mr1E_A2.FullDuplexEnable  = (val>>5)&0x0001;    
        mr1E_A2.MSConfig1000  = (val>>4)&0x0001; 
        mr1E_A2.final_speed_1000 = (val>>3)&0x0001; 
        mr1E_A2.final_speed_100  = (val>>2)&0x0001; 
        mr1E_A2.final_speed_10   = (val>>1)&0x0001;       
        break;
        
	case 0x3d:
		mr7_3D.lp_eee_100=(val>>1)&0x0001;
		mr7_3D.lp_eee_1000=(val>>2)&0x0001;
		mr7_3D.lp_eee_10g=(val>>3)&0x0001;
		break;

		//printk("kant_7dev_3d = 0x%x \n\r", val);
		
    default:
        break;      
    }   
    return (val);
}
#endif
#endif		// TCPHY_SUPPORT


#ifdef LINUX_OS // allen_20130926 : merge 7502 & 7510/20
int32_t mtPhyMiiRead_TrDbg(u8 phyaddr, char *type, u32 data_addr , u8 ch_num)
{
    const u16 page_reg=31;
    const u32 Token_Ring_debug_reg=0x52B5;
    const u32 Token_Ring_Control_reg=0x10;
    const u32 Token_Ring_Low_data_reg=0x11;
    const u32 Token_Ring_High_data_reg=0x12;

    u16 ch_addr=0;
    u32 node_addr=0;

    u32 value=0;
    u32 value_high=0;
    u32 value_low=0;
    
  	if(stricmp(type, "DSPF") == 0)
  	{	// DSP Filter Debug Node
        ch_addr = 0x02;
		node_addr = 0x0D;
	}
	else if(stricmp(type, "PMA") == 0)
	{ 	// PMA Debug Node 
		ch_addr = 0x01;
		node_addr = 0x0F;
	}
	else if(stricmp(type, "TR") == 0)
	{ 	// Timing Recovery  Debug Node 
		ch_addr=0x01;
		node_addr=0x0D;
	}
	else if(stricmp(type, "PCS") == 0)
	{ 	// R1000PCS Debug Node 
		ch_addr=0x02;
		node_addr=0x0F;
	}
	else if(stricmp(type, "FFE") == 0)
	{ 	// FFE Debug Node 
		ch_addr=ch_num;
		node_addr=0x04;
	}
	else if(stricmp(type, "EC") == 0)
	{ 	// ECC Debug Node 
		ch_addr=ch_num;
		node_addr=0x00;
	}
	else if(stricmp(type, "ECT") == 0)
	{ 	// EC/Tail Debug Node 
		ch_addr=ch_num;
		node_addr=0x01;
	}
	else if(stricmp(type, "NC") == 0)
	{ 	// EC/NC Debug Node 
		ch_addr=ch_num;
		node_addr=0x01;
	}
	else if(stricmp(type, "DFEDC") == 0)
	{ 	// DFETail/DC Debug Node 
		ch_addr=ch_num;
		node_addr=0x05;
	}
	else if(stricmp(type, "DEC") == 0)
	{ 	// R1000DEC Debug Node 
		ch_addr=0x00; 
		node_addr=0x07;
	}
	else if(stricmp(type, "CRC") == 0)
	{ 	// R1000CRC Debug Node 
		ch_addr=ch_num;
		node_addr=0x06;
	}
	else if(stricmp(type, "AN") == 0)
	{ 	// Autoneg Debug Node 
		ch_addr=0x00; 
		node_addr=0x0F;
	}
	else if(stricmp(type, "CMI") == 0)
	{ 	// CMI Debug Node 
		ch_addr=0x03; 
		node_addr=0x0F;
	}
	else if(stricmp(type, "SUPV") == 0)
	{ 	// SUPV PHY  Debug Node 
		ch_addr=0x00; 
		node_addr=0x0D;
	}
	else
	{
		printk("Wrong TR register Type !");
    	return(0xFFFF);
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
		value=value_low+((value_high&0x00FF)<<16);
		TCPHYDISP4("*%s => Phyaddr=%d, ch_addr=%d, node_addr=0x%02X, data_addr=0x%02X , value=0x%08X\r\n", type,phyaddr, ch_addr, node_addr, data_addr, value);
	}
	tcMiiStationWrite(phyaddr, page_reg, 0x00);//V1.11
		
	return(value);
}

int mtPhyMiiWrite_TrDbg(u8 phyaddr, char *type, u32 data_addr ,u32 value, u8 ch_num)
{
    const u16 page_reg=31;
    const u32 Token_Ring_debug_reg=0x52B5;
    const u32 Token_Ring_Control_reg=0x10;
    const u32 Token_Ring_Low_data_reg=0x11;
    const u32 Token_Ring_High_data_reg=0x12;
	
    u16 ch_addr=0;
    u32 node_addr=0;
    u32 value_high=0;
    u32 value_low=0;
	
	if(stricmp(type, "DSPF") == 0)
	{	// DSP Filter Debug Node
		ch_addr=0x02;
		node_addr=0x0D;
	}
	else if(stricmp(type, "PMA") == 0)
	{ 	// PMA Debug Node 
		ch_addr=0x01;
		node_addr=0x0F;
	}
	else if(stricmp(type, "TR") == 0)
	{ 	// Timing Recovery	Debug Node 
		ch_addr=0x01;
		node_addr=0x0D;
	}
	else if(stricmp(type, "PCS") == 0)
	{ 	// R1000PCS Debug Node 
		ch_addr=0x02;
		node_addr=0x0F;
	}
	else if(stricmp(type, "FFE") == 0)
	{ 	// FFE Debug Node 
		ch_addr=ch_num;
		node_addr=0x04;
	}
	else if(stricmp(type, "EC") == 0)
	{ 	// ECC Debug Node 
		ch_addr=ch_num;
		node_addr=0x00;
	}
	else if(stricmp(type, "ECT") == 0)
	{ 	// EC/Tail Debug Node 
		ch_addr=ch_num;
		node_addr=0x01;
	}
	else if(stricmp(type, "NC") == 0)
	{ 	// EC/NC Debug Node 
		ch_addr=ch_num;
		node_addr=0x01;
	}
	else if(stricmp(type, "DFEDC") == 0)
	{ 	// DFETail/DC Debug Node 
		ch_addr=ch_num;
		node_addr=0x05;
	}
	else if(stricmp(type, "DEC") == 0)
	{ 	// R1000DEC Debug Node 
		ch_addr=0x00; 
		node_addr=0x07;
	}
	else if(stricmp(type, "CRC") == 0)
	{ 	// R1000CRC Debug Node 
		ch_addr=ch_num;
		node_addr=0x06;
	}
	else if(stricmp(type, "AN") == 0)
	{ 	// Autoneg Debug Node 
		ch_addr=0x00; 
		node_addr=0x0F;
	}
	else if(stricmp(type, "CMI") == 0)
	{ 	// CMI Debug Node 
		ch_addr=0x03; 
		node_addr=0x0F;
	}
	else if(stricmp(type, "SUPV") == 0)
	{ 	// SUPV PHY  Debug Node 
		ch_addr=0x00; 
		node_addr=0x0D;
	}

    if ((tcMiiStationRead(phyaddr, 0) & SMIREG_PWDN_BIT) != 0) //token ring can't access during power down
	{
		printk("Tokenring invalide !");
    	return(0xFFFF);
	}
	   
	data_addr=data_addr&0x3F;
	value_high=(0x00FF0000&value)>>16;
	value_low=(0x0000FFFF&value);
   
	tcMiiStationWrite(phyaddr, page_reg, Token_Ring_debug_reg);
		   
	tcMiiStationWrite(phyaddr, Token_Ring_Low_data_reg, value_low);
	tcMiiStationWrite(phyaddr, Token_Ring_High_data_reg, value_high);
	tcMiiStationWrite(phyaddr, Token_Ring_Control_reg, (1<<15)|(0<<13)|(ch_addr<<11)|(node_addr<<7)|(data_addr<<1));
	//while(!(tcMiiStationRead(phyaddr, Token_Ring_Control_reg)&0x8000)); 			// data ready
	{
		TCPHYDISP4("*%s => Phyaddr=%d, ch_addr=%d, node_addr=0x%02X, data_addr=0x%02X , value=0x%08X\r\n", type,phyaddr, ch_addr, node_addr, data_addr, value);
	}
	tcMiiStationWrite(phyaddr, page_reg, 0x00);			//V1.11
	return 0;
}


/************************************************************************
    Funtcion:       mdio_cl22_write
    Description:    
    Calls:
    Called by:      trgmii setting
    Input:
    Output:
    Return:
    Others:     
************************************************************************/
void mdio_cl22_write(u32 port_num,u32 dev_num,u32 reg_num,u32 reg_data)
{
    const u16 MMD_Control_register=0xD;
    const u16 MMD_addr_data_register=0xE;
  	const u16 page_reg=31;


	tcMiiStationWrite(port_num, page_reg, 0x00); //switch to main page
	tcMiiStationWrite(port_num, MMD_Control_register, (0<<14)+dev_num);
	tcMiiStationWrite(port_num, MMD_addr_data_register, reg_num);
	tcMiiStationWrite(port_num, MMD_Control_register, (1<<14)+dev_num);
	tcMiiStationWrite(port_num, MMD_addr_data_register, reg_data);
//	printk("* doPhyMMDWrite_CL22=> phyaddr=%d, dev_addr=%d, data_addr=0x%04lX , value=0x%04lX\r\n", port_num, dev_num, reg_num, reg_data);
}

#endif

