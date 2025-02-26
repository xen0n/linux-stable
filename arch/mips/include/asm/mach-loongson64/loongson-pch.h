/*
 *  Copyright (C) 2013, Loongson Technology Corporation Limited, Inc.
 *  Copyright (C) 2014-2017, Lemote, Inc.
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 */
#ifndef _LOONGSON_PCH_H
#define _LOONGSON_PCH_H

#include <linux/types.h>
#include <linux/pci.h>
#include <asm/addrspace.h>

/* ============== LS2H registers =============== */

#define LS2H_PCH_REG_BASE		0x1b000000

/* CHIP CONFIG regs */
#define LS2H_CHIPCFG_REG_BASE		(LS2H_PCH_REG_BASE + 0x00d00000)
/* INT CONFIG regs */
#define LS2H_INT_REG_BASE		(LS2H_PCH_REG_BASE + 0x00d00040)
/* DMA ORDER regs */
#define LS2H_DMA_ORDER_REG_BASE		(LS2H_PCH_REG_BASE + 0x00d00100)
/* WIN CONFIG regs */
#define LS2H_WIN_CFG_BASE		(LS2H_PCH_REG_BASE + 0x00d80000)
/* GPU regs */
#define LS2H_GPU_REG_BASE		(LS2H_PCH_REG_BASE + 0x00e40000)
/* DC regs */
#define LS2H_DC_REG_BASE		(LS2H_PCH_REG_BASE + 0x00e50000)
/* HPET regs */
#define LS2H_HPET_REG_BASE		(LS2H_PCH_REG_BASE + 0x00ec0000)
/* NAND regs */
#define LS2H_NAND_REG_BASE		(LS2H_PCH_REG_BASE + 0x00ee0000)
/* ACPI regs */
#define LS2H_ACPI_REG_BASE		(LS2H_PCH_REG_BASE + 0x00ef0000)
/* LPC regs */
#define LS2H_LPC_IO_BASE		(LS2H_PCH_REG_BASE + 0x00f00000)
#define LS2H_LPC_REG_BASE		(LS2H_PCH_REG_BASE + 0x00f10000)

#define LS2H_INT_ISR0_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0040)
#define LS2H_INT_IEN0_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0044)
#define LS2H_INT_SET0_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0048)
#define LS2H_INT_CLR0_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x004c)
#define LS2H_INT_POL0_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0050)
#define LS2H_INT_EDGE0_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0054)
#define LS2H_GPIO_CFG_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x00c0)
#define LS2H_GPIO_OE_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x00c4)
#define LS2H_GPIO_IN_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x00c8)
#define LS2H_GPIO_OUT_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x00cc)
#define LS2H_CHIP_CFG0_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0200)
#define LS2H_CHIP_CFG1_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0204)
#define LS2H_CHIP_CFG2_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0208)
#define LS2H_CHIP_CFG3_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x020c)
#define LS2H_CHIP_SAMP0_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0210)
#define LS2H_CHIP_SAMP1_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0214)
#define LS2H_CHIP_SAMP2_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0218)
#define LS2H_CHIP_SAMP3_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x021c)
#define LS2H_CLK_CTRL0_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0220)
#define LS2H_CLK_CTRL1_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0224)
#define LS2H_CLK_CTRL2_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0228)
#define LS2H_CLK_CTRL3_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x022c)
#define LS2H_PIXCLK0_CTRL0_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0230)
#define LS2H_PIXCLK0_CTRL1_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0234)
#define LS2H_PIXCLK1_CTRL0_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x0238)
#define LS2H_PIXCLK1_CTRL1_REG		(void *)TO_UNCAC(LS2H_CHIPCFG_REG_BASE + 0x023c)

#define LS2H_M1_WIN4_BASE_REG		(void *)TO_UNCAC(LS2H_WIN_CFG_BASE + 0x0120)
#define LS2H_M1_WIN4_MASK_REG		(void *)TO_UNCAC(LS2H_WIN_CFG_BASE + 0x0160)
#define LS2H_M1_WIN4_MMAP_REG		(void *)TO_UNCAC(LS2H_WIN_CFG_BASE + 0x01a0)
#define LS2H_M1_WIN6_BASE_REG		(void *)TO_UNCAC(LS2H_WIN_CFG_BASE + 0x0130)
#define LS2H_M1_WIN6_MASK_REG		(void *)TO_UNCAC(LS2H_WIN_CFG_BASE + 0x0170)
#define LS2H_M1_WIN6_MMAP_REG		(void *)TO_UNCAC(LS2H_WIN_CFG_BASE + 0x01b0)
#define LS2H_M4_WIN0_BASE_REG		(void *)TO_UNCAC(LS2H_WIN_CFG_BASE + 0x0400)
#define LS2H_M4_WIN0_MASK_REG		(void *)TO_UNCAC(LS2H_WIN_CFG_BASE + 0x0440)
#define LS2H_M4_WIN0_MMAP_REG		(void *)TO_UNCAC(LS2H_WIN_CFG_BASE + 0x0480)

#define LS2H_FB_CFG_DVO_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1240)
#define LS2H_FB_CFG_VGA_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1250)
#define LS2H_FB_ADDR0_DVO_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1260)
#define LS2H_FB_ADDR0_VGA_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1270)
#define LS2H_FB_STRI_DVO_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1280)
#define LS2H_FB_STRI_VGA_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1290)
#define LS2H_FB_DITCFG_DVO_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1360)
#define LS2H_FB_DITCFG_VGA_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1370)
#define LS2H_FB_DITTAB_LO_DVO_REG	(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1380)
#define LS2H_FB_DITTAB_LO_VGA_REG	(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1390)
#define LS2H_FB_DITTAB_HI_DVO_REG	(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x13a0)
#define LS2H_FB_DITTAB_HI_VGA_REG	(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x13b0)
#define LS2H_FB_PANCFG_DVO_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x13c0)
#define LS2H_FB_PANCFG_VGA_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x13d0)
#define LS2H_FB_PANTIM_DVO_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x13e0)
#define LS2H_FB_PANTIM_VGA_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x13f0)
#define LS2H_FB_HDISPLAY_DVO_REG	(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1400)
#define LS2H_FB_HDISPLAY_VGA_REG	(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1410)
#define LS2H_FB_HSYNC_DVO_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1420)
#define LS2H_FB_HSYNC_VGA_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1430)
#define LS2H_FB_VDISPLAY_DVO_REG	(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1480)
#define LS2H_FB_VDISPLAY_VGA_REG	(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1490)
#define LS2H_FB_VSYNC_DVO_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x14a0)
#define LS2H_FB_VSYNC_VGA_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x14b0)
#define LS2H_FB_GAMINDEX_DVO_REG	(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x14e0)
#define LS2H_FB_GAMINDEX_VGA_REG	(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x14f0)
#define LS2H_FB_GAMDATA_DVO_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1500)
#define LS2H_FB_GAMDATA_VGA_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1510)
#define LS2H_FB_CUR_CFG_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1520)
#define LS2H_FB_CUR_ADDR_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1530)
#define LS2H_FB_CUR_LOC_ADDR_REG	(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1540)
#define LS2H_FB_CUR_BACK_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1550)
#define LS2H_FB_CUR_FORE_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1560)
#define LS2H_FB_INT_REG			(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1570)
#define LS2H_FB_ADDR1_DVO_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1580)
#define LS2H_FB_ADDR1_VGA_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1590)
#define LS2H_FB_DAC_CTRL_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1600)
#define LS2H_FB_DVO_OUTPUT_REG		(void *)TO_UNCAC(LS2H_DC_REG_BASE + 0x1630)

#define LS2H_PM_SOC_REG			(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x0000)
#define LS2H_PM_RESUME_REG		(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x0004)
#define LS2H_PM_RTC_REG			(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x0008)
#define LS2H_PM_EVT_REG			(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x000c)
#define LS2H_PM_ENA_REG			(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x0010)
#define LS2H_PM_CNT_REG			(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x0014)
#define LS2H_PM_TMR_REG			(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x0018)
#define LS2H_P_CNT_REG			(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x001c)
#define LS2H_P_LVL2_REG			(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x0020)
#define LS2H_P_LVL3_REG			(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x0024)
#define LS2H_GPE0_STS_REG		(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x0028)
#define LS2H_GPE0_ENA_REG		(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x002c)
#define LS2H_RST_CNT_REG		(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x0030)
#define LS2H_WD_SET_REG			(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x0034)
#define LS2H_WD_TIMER_REG		(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x0038)
#define LS2H_DVFS_CNT_REG		(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x003c)
#define LS2H_DVFS_STS_REG		(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x0040)
#define LS2H_MS_CNT_REG			(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x0044)
#define LS2H_MS_THT_REG			(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x0048)
#define	LS2H_THSENS_CNT_REG		(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x004c)
#define LS2H_GEN_RTC1_REG		(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x0050)
#define LS2H_GEN_RTC2_REG		(void *)TO_UNCAC(LS2H_ACPI_REG_BASE + 0x0054)

#define LS2H_LPC_INT_CTL		(void *)TO_UNCAC(LS2H_LPC_REG_BASE + 0x0)
#define LS2H_LPC_INT_ENA		(void *)TO_UNCAC(LS2H_LPC_REG_BASE + 0x4)
#define LS2H_LPC_INT_STS		(void *)TO_UNCAC(LS2H_LPC_REG_BASE + 0x8)
#define LS2H_LPC_INT_CLR		(void *)TO_UNCAC(LS2H_LPC_REG_BASE + 0xc)

#define LS2H_PCIE_MAX_PORTNUM			3
#define LS2H_PCIE_MEM0_BASE(portnum)		(0x10000000 + (portnum << 25))
#define LS2H_PCIE_MEM1_BASE(portnum)		(0x40000000 + (portnum << 28))
#define LS2H_PCIE_IO_BASE(portnum)		CKSEG1ADDR(0x18100000 + (portnum << 22))
#define LS2H_PCIE_PORT_HEAD_BASE(portnum)	CKSEG1ADDR(0x18114000 + (portnum << 22))
#define LS2H_PCIE_DEV_HEAD_BASE(portnum)	CKSEG1ADDR(0x18116000 + (portnum << 22))
#define LS2H_PCIE_PORT_REG_BASE(portnum)	CKSEG1ADDR(0x18118000 + (portnum << 22))
#define LS2H_PCIE_PORT_REG_CTR0			0x0
#define  LS2H_PCIE_REG_CTR0_BIT_LTSSM_EN	(1 << 3)
#define  LS2H_PCIE_REG_CTR0_BIT_REQ_L1		(1 << 12)
#define  LS2H_PCIE_REG_CTR0_BIT_RDY_L23		(1 << 13)
#define LS2H_PCIE_PORT_REG_CTR1			0x4
#define LS2H_PCIE_PORT_REG_STAT0		0x8
#define LS2H_PCIE_PORT_REG_STAT1		0xc
#define  LS2H_PCIE_REG_STAT1_MASK_LTSSM		0x0000003f
#define  LS2H_PCIE_REG_STAT1_BIT_LINKUP		(1 << 6)
#define LS2H_PCIE_PORT_REG_INTSTS		0x18
#define LS2H_PCIE_PORT_REG_INTCLR		0x1c
#define LS2H_PCIE_PORT_REG_INTMSK		0x20
#define LS2H_PCIE_PORT_REG_CFGADDR		0x24
#define LS2H_PCIE_PORT_REG_CTR_STAT		0x28
#define  LS2H_PCIE_REG_CTR_STAT_BIT_ISX4	(1 << 26)
#define  LS2H_PCIE_REG_CTR_STAT_BIT_ISRC	(1 << 27)
#define LS2H_PCI_EXP_LNKCAP			0x7c
#define LS2H_CLK_CTRL3_BIT_PEREF_EN(portnum)	(1 << (24 + portnum))

/* ============== LS7A registers =============== */

#define LS7A_PCH_REG_BASE		0x10000000
/* MISC reg base */
#define LS7A_MISC_REG_BASE		(LS7A_PCH_REG_BASE + 0x00080000)
/* CHIPCFG regs */
#define LS7A_CHIPCFG_REG_BASE		(LS7A_PCH_REG_BASE + 0x0a000000)
/* ACPI regs */
#define LS7A_ACPI_REG_BASE		(LS7A_MISC_REG_BASE + 0x00050000)
/* RTC regs */
#define LS7A_RTC_REG_BASE		(LS7A_MISC_REG_BASE + 0x00050100)

#define LS7A_INT_MASK_REG		(void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x020)
#define LS7A_INT_EDGE_REG		(void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x060)
#define LS7A_INT_CLEAR_REG		(void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x080)
#define LS7A_INT_HTMSI_EN_REG		(void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x040)
#define LS7A_INT_ROUTE_ENTRY_REG	(void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x100)
#define LS7A_INT_HTMSI_VEC_REG		(void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x200)
#define LS7A_INT_STATUS_REG		(void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x3a0)
#define LS7A_LPC_INT_CTL		(void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x2000)
#define LS7A_LPC_INT_ENA		(void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x2004)
#define LS7A_LPC_INT_STS		(void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x2008)
#define LS7A_LPC_INT_CLR		(void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x200c)

#define LS7A_PMCON_SOC_REG		(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x000)
#define LS7A_PMCON_RESUME_REG		(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x004)
#define LS7A_PMCON_RTC_REG		(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x008)
#define LS7A_PM1_EVT_REG		(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x00c)
#define LS7A_PM1_ENA_REG		(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x010)
#define LS7A_PM1_CNT_REG		(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x014)
#define LS7A_PM1_TMR_REG		(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x018)
#define LS7A_P_CNT_REG			(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x01c)
#define LS7A_GPE0_STS_REG		(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x028)
#define LS7A_GPE0_ENA_REG		(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x02c)
#define LS7A_RST_CNT_REG		(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x030)
#define LS7A_WD_SET_REG			(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x034)
#define LS7A_WD_TIMER_REG		(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x038)
#define LS7A_THSENS_CNT_REG		(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x04c)
#define LS7A_GEN_RTC_1_REG		(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x050)
#define LS7A_GEN_RTC_2_REG		(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x054)
#define LS7A_DPM_CFG_REG		(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x400)
#define LS7A_DPM_STS_REG		(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x404)
#define LS7A_DPM_CNT_REG		(void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x408)

#define LS7A_FB_CFG_DVO0_REG		(0x1240)
#define LS7A_FB_CFG_DVO1_REG		(0x1250)
#define LS7A_FB_ADDR0_DVO0_REG		(0x1260)
#define LS7A_FB_ADDR0_DVO1_REG		(0x1270)
#define LS7A_FB_STRI_DVO0_REG		(0x1280)
#define LS7A_FB_STRI_DVO1_REG		(0x1290)

#define LS7A_FB_DITCFG_DVO0_REG		(0x1360)
#define LS7A_FB_DITCFG_DVO1_REG		(0x1370)
#define LS7A_FB_DITTAB_LO_DVO0_REG	(0x1380)
#define LS7A_FB_DITTAB_LO_DVO1_REG	(0x1390)
#define LS7A_FB_DITTAB_HI_DVO0_REG	(0x13a0)
#define LS7A_FB_DITTAB_HI_DVO1_REG	(0x13b0)
#define LS7A_FB_PANCFG_DVO0_REG		(0x13c0)
#define LS7A_FB_PANCFG_DVO1_REG		(0x13d0)
#define LS7A_FB_PANTIM_DVO0_REG		(0x13e0)
#define LS7A_FB_PANTIM_DVO1_REG		(0x13f0)

#define LS7A_FB_HDISPLAY_DVO0_REG	(0x1400)
#define LS7A_FB_HDISPLAY_DVO1_REG	(0x1410)
#define LS7A_FB_HSYNC_DVO0_REG		(0x1420)
#define LS7A_FB_HSYNC_DVO1_REG		(0x1430)

#define LS7A_FB_VDISPLAY_DVO0_REG	(0x1480)
#define LS7A_FB_VDISPLAY_DVO1_REG	(0x1490)
#define LS7A_FB_VSYNC_DVO0_REG		(0x14a0)
#define LS7A_FB_VSYNC_DVO1_REG		(0x14b0)

#define LS7A_FB_GAMINDEX_DVO0_REG	(0x14e0)
#define LS7A_FB_GAMINDEX_DVO1_REG	(0x14f0)
#define LS7A_FB_GAMDATA_DVO0_REG	(0x1500)
#define LS7A_FB_GAMDATA_DVO1_REG	(0x1510)

#define LS7A_FB_CUR_CFG_REG		(0x1520)
#define LS7A_FB_CUR_ADDR_REG		(0x1530)
#define LS7A_FB_CUR_LOC_ADDR_REG	(0x1540)
#define LS7A_FB_CUR_BACK_REG		(0x1550)
#define LS7A_FB_CUR_FORE_REG		(0x1560)

#define LS7A_FB_INT_REG			(0x1570)

#define LS7A_FB_ADDR1_DVO0_REG		(0x1580)
#define LS7A_FB_ADDR1_DVO1_REG		(0x1590)

#define LS7A_FB_DAC_CTRL_REG		(0x1600)
#define LS7A_FB_DVO_OUTPUT_REG		(0x1630)

#define LS7A_PCIE_BAR_BASE(bus, dev, func) \
	readl((void *)TO_UNCAC(LS7A_CHIPCFG_REG_BASE | (bus << 16) | (dev << 11) | (func << 8) | 0x10))

/* ============== RS780/SBX00 registers =============== */

#define SBX00_ACPI_IO_BASE 0x800
#define SBX00_ACPI_IO_SIZE 0x100

#define SBX00_PM_EVT_BLK       (SBX00_ACPI_IO_BASE + 0x00) /* 4 bytes */
#define SBX00_PM_CNT_BLK       (SBX00_ACPI_IO_BASE + 0x04) /* 2 bytes */
#define SBX00_PMA_CNT_BLK      (SBX00_ACPI_IO_BASE + 0x0F) /* 1 byte */
#define SBX00_PM_TMR_BLK       (SBX00_ACPI_IO_BASE + 0x18) /* 4 bytes */
#define SBX00_GPE0_BLK         (SBX00_ACPI_IO_BASE + 0x10) /* 8 bytes */
#define SBX00_PM_END           (SBX00_ACPI_IO_BASE + 0x80)

#define PM_INDEX        0xCD6
#define PM_DATA         0xCD7
#define PM2_INDEX       0xCD0
#define PM2_DATA        0xCD1

/* ============== Data structrues =============== */

enum b_type { /* BoardType(BridgeType) */
	LS2H   = 1,
	LS7A   = 2,
	RS780E = 3
};

struct platform_controller_hub {
	int	type; /* BoardType(BridgeType) */
	int 	pcidev_max_funcs;
	void	(*early_config)(void);
	void	(*init_irq)(void);
	void	(*irq_dispatch)(void);
	int	(*pcibios_map_irq)(const struct pci_dev *dev, u8 slot, u8 pin);
	int	(*pcibios_dev_init)(struct pci_dev *dev);
	void	(*pch_arch_initcall)(void);
	void	(*pch_device_initcall)(void);
};

extern struct platform_controller_hub ls2h_pch;
extern struct platform_controller_hub ls7a_pch;
extern struct platform_controller_hub rs780_pch;
extern struct platform_controller_hub *loongson_pch;

extern struct pci_ops ls2h_pci_ops[4];
extern void ls2h_init_irq(void);
extern void ls2h_irq_dispatch(void);
extern int ls2h_pcibios_map_irq(const struct pci_dev *dev, u8 slot, u8 pin);

extern struct pci_ops ls7a_pci_ops;
extern void ls7a_init_irq(void);
extern void ls7a_irq_dispatch(void);
extern int ls7a_pcibios_map_irq(const struct pci_dev *dev, u8 slot, u8 pin);

extern struct pci_ops rs780_pci_ops;
extern void rs780_init_irq(void);
extern void rs780_irq_dispatch(void);
extern int rs780_pcibios_map_irq(const struct pci_dev *dev, u8 slot, u8 pin);

#endif
