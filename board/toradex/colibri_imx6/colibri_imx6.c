/*
 * Copyright (C) 2010-2013 Freescale Semiconductor, Inc.
 * Copyright (C) 2013, Boundary Devices <info@boundarydevices.com>
 * Copyright (C) 2014-2015, Toradex AG
 * copied from nitrogen6x
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <malloc.h>
#include <asm/arch/mx6-pins.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/imx-common/sata.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/video.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <micrel.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mxc_hdmi.h>
#include <i2c.h>

#include "../common/configblock.h"
#ifdef CONFIG_TRDX_CMD_IMX_MFGR
#include "pf0100.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm     | PAD_CTL_SRE_FAST)

#define BUTTON_PAD_CTRL (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define WEAK_PULLUP	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_SRE_SLOW)

#define NO_PULLUP	(					\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_SRE_SLOW)

#define WEAK_PULLDOWN	(PAD_CTL_PUS_100K_DOWN |		\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_HYS | PAD_CTL_SRE_SLOW)

#define OUTPUT_40OHM (PAD_CTL_SPEED_MED|PAD_CTL_DSE_40ohm)

#define OUTPUT_RGB (PAD_CTL_SPEED_MED|PAD_CTL_DSE_60ohm|PAD_CTL_SRE_FAST)

u32 get_board_rev(void);

int dram_init(void)
{
	/* use the DDR controllers configured size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    (ulong)imx_ddr_size());

	return 0;
}

/* Colibri UARTA */
iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_CSI0_DAT10__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT11__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
/* Colibri I2C */
struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6_PAD_GPIO_3__I2C3_SCL | PC,
		.gpio_mode = MX6_PAD_GPIO_3__GPIO1_IO03 | PC,
		.gp = IMX_GPIO_NR(1, 3)
	},
	.sda = {
		.i2c_mode = MX6_PAD_GPIO_6__I2C3_SDA | PC,
		.gpio_mode = MX6_PAD_GPIO_6__GPIO1_IO06 | PC,
		.gp = IMX_GPIO_NR(1, 6)
	}
};

/* Colibri local, PMIC, SGTL5000, STMPE811 */
struct i2c_pads_info i2c_pad_info_loc = {
	.scl = {
		.i2c_mode = MX6_PAD_EIM_EB2__I2C2_SCL | PC,
		.gpio_mode = MX6_PAD_EIM_EB2__GPIO2_IO30 | PC,
		.gp = IMX_GPIO_NR(2, 30)
	},
	.sda = {
		.i2c_mode = MX6_PAD_EIM_D16__I2C2_SDA | PC,
		.gpio_mode = MX6_PAD_EIM_D16__GPIO3_IO16 | PC,
		.gp = IMX_GPIO_NR(3, 16)
	}
};

/* Apalis MMC */
iomux_v3_cfg_t const usdhc1_pads[] = {
	MX6_PAD_SD1_CLK__SD1_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_CMD__SD1_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT0__SD1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT1__SD1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT2__SD1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT3__SD1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NANDF_D5__GPIO2_IO05 | MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
#	define GPIO_MMC_CD IMX_GPIO_NR(2, 5)
};

/* eMMC */
iomux_v3_cfg_t const usdhc3_pads[] = {
	MX6_PAD_SD3_CLK__SD3_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_CMD__SD3_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT4__SD3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT5__SD3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT6__SD3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT7__SD3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_RST__SD3_RESET  | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

iomux_v3_cfg_t const enet_pads[] = {
	MX6_PAD_ENET_MDC__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDIO__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_RXD0__ENET_RX_DATA0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_RXD1__ENET_RX_DATA1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_RX_ER__ENET_RX_ER		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_TX_EN__ENET_TX_EN		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_TXD0__ENET_TX_DATA0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_TXD1__ENET_TX_DATA1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_CRS_DV__ENET_RX_EN		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_GPIO_16__ENET_REF_CLK		| MUX_PAD_CTRL(ENET_PAD_CTRL),
};

static void setup_iomux_enet(void)
{
	imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));
}

/* mux auxiliary pins to GPIO, so they can be used from the U-Boot commandline */
iomux_v3_cfg_t const gpio_pads[] = {
	/* ADDRESS[17:18] [25] used as GPIO */
	MX6_PAD_KEY_ROW2__GPIO4_IO11	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_KEY_COL2__GPIO4_IO10	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_NANDF_D1__GPIO2_IO01	| MUX_PAD_CTRL(WEAK_PULLUP),
	/* ADDRESS[19:24] used as GPIO */
	MX6_PAD_DISP0_DAT23__GPIO5_IO17 | MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_DISP0_DAT22__GPIO5_IO16 | MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_DISP0_DAT21__GPIO5_IO15 | MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_DISP0_DAT20__GPIO5_IO14 | MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_DISP0_DAT19__GPIO5_IO13 | MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_DISP0_DAT18__GPIO5_IO12 | MUX_PAD_CTRL(WEAK_PULLUP),
	/* DATA[16:29] [31]	 used as GPIO */
	MX6_PAD_EIM_LBA__GPIO2_IO27	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_EIM_BCLK__GPIO6_IO31	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_NANDF_CS3__GPIO6_IO16	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_NANDF_CS1__GPIO6_IO14	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_NANDF_RB0__GPIO6_IO10	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_NANDF_ALE__GPIO6_IO08	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_NANDF_WP_B__GPIO6_IO09	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_NANDF_CS0__GPIO6_IO11	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_NANDF_CLE__GPIO6_IO07	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_GPIO_19__GPIO4_IO05	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_CSI0_MCLK__GPIO5_IO19	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_CSI0_PIXCLK__GPIO5_IO18 | MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_GPIO_4__GPIO1_IO04	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_GPIO_5__GPIO1_IO05	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_GPIO_2__GPIO1_IO02	| MUX_PAD_CTRL(WEAK_PULLUP),
	/* DQM[0:3]	 used as GPIO */
	MX6_PAD_EIM_EB0__GPIO2_IO28	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_EIM_EB1__GPIO2_IO29	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_SD2_DAT2__GPIO1_IO13	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_NANDF_D0__GPIO2_IO00	| MUX_PAD_CTRL(WEAK_PULLUP),
	/* RDY	used as GPIO */
	MX6_PAD_EIM_WAIT__GPIO5_IO00	| MUX_PAD_CTRL(WEAK_PULLUP),
	/* ADDRESS[16] DATA[30]	 used as GPIO */
	MX6_PAD_KEY_ROW4__GPIO4_IO15	| MUX_PAD_CTRL(WEAK_PULLDOWN),
	MX6_PAD_KEY_COL4__GPIO4_IO14	| MUX_PAD_CTRL(WEAK_PULLUP),
	/* CSI pins used as GPIO */
	MX6_PAD_EIM_A24__GPIO5_IO04	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_SD2_CMD__GPIO1_IO11	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_NANDF_CS2__GPIO6_IO15	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_EIM_D18__GPIO3_IO18	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_EIM_A19__GPIO2_IO19	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_EIM_D29__GPIO3_IO29	| MUX_PAD_CTRL(WEAK_PULLDOWN),
	MX6_PAD_EIM_A23__GPIO6_IO06	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_EIM_A20__GPIO2_IO18	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_EIM_A17__GPIO2_IO21	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_EIM_A18__GPIO2_IO20	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_EIM_EB3__GPIO2_IO31	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_EIM_D17__GPIO3_IO17	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_SD2_DAT0__GPIO1_IO15	| MUX_PAD_CTRL(WEAK_PULLUP),
	/* GPIO */
	MX6_PAD_EIM_D26__GPIO3_IO26	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_EIM_D27__GPIO3_IO27	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_NANDF_D6__GPIO2_IO06	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_NANDF_D3__GPIO2_IO03	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_ENET_REF_CLK__GPIO1_IO23| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_DI0_PIN4__GPIO4_IO20	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_SD4_DAT3__GPIO2_IO11	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_NANDF_D4__GPIO2_IO04	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_SD4_DAT0__GPIO2_IO08	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_GPIO_7__GPIO1_IO07	| MUX_PAD_CTRL(WEAK_PULLUP),
	MX6_PAD_GPIO_8__GPIO1_IO08	| MUX_PAD_CTRL(WEAK_PULLUP),
	/* USBH_OC */
	MX6_PAD_EIM_D30__GPIO3_IO30	| MUX_PAD_CTRL(WEAK_PULLUP),
	/* USBC_ID */
	MX6_PAD_NANDF_D2__GPIO2_IO02	| MUX_PAD_CTRL(WEAK_PULLUP),
	/* USBC_DET */
	MX6_PAD_GPIO_17__GPIO7_IO12	| MUX_PAD_CTRL(WEAK_PULLUP),
};

static void setup_iomux_gpio(void)
{
	imx_iomux_v3_setup_multiple_pads(gpio_pads, ARRAY_SIZE(gpio_pads));
}

iomux_v3_cfg_t const usb_pads[] = {
	/* TODO This pin has a dedicated USB power functionality, can we use it? */
	/* USB_PE */
	MX6_PAD_EIM_D31__GPIO3_IO31 | MUX_PAD_CTRL(NO_PAD_CTRL),
#	define GPIO_USBH_EN IMX_GPIO_NR(3, 31)
#if 0
/* FIXME USB CLIENT */
	/* USB_C_DET */
	MX6_PAD_ENET_RX_ER__USB_OTG_ID	| MUX_PAD_CTRL(WEAK_PULLUP),
#endif
};

/*
 * UARTs are used in DTE mode, switch the mode on all UARTs before
 * any pinmuxing connects a (DCE) output to a transceiver output.
 */
#define UFCR		0x90	/* FIFO Control Register */
#define UFCR_DCEDTE	(1<<6)	/* DCE=0 */

static void setup_dtemode_uart(void)
{
	setbits_le32((u32 *)(UART1_BASE + UFCR), UFCR_DCEDTE);
	setbits_le32((u32 *)(UART2_BASE + UFCR), UFCR_DCEDTE);
	setbits_le32((u32 *)(UART3_BASE + UFCR), UFCR_DCEDTE);
}

static void setup_iomux_uart(void)
{
	setup_dtemode_uart();
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

#ifdef CONFIG_USB_EHCI_MX6
int board_ehci_hcd_init(int port)
{
	imx_iomux_v3_setup_multiple_pads(usb_pads, ARRAY_SIZE(usb_pads));
	return 0;
}

int board_ehci_power(int port, int on)
{
	switch (port) {
	case 0:
		/* control OTG power */
		/* No special PE for USBC, always on when ID pin signals host mode */
		break;
	case 1:
		/* Control MXM USBH */
		/* Set MXM USBH power enable, '0' means on */
		gpio_direction_output(GPIO_USBH_EN, !on);
		mdelay(100);
		break;
	default:
		break;
	}
	return 0;
}
#endif

#ifdef CONFIG_FSL_ESDHC
/* use the following sequence: eMMC, MMC */
struct fsl_esdhc_cfg usdhc_cfg[CONFIG_SYS_FSL_USDHC_NUM] = {
	{USDHC3_BASE_ADDR},
	{USDHC1_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = true; /* default: assume inserted */

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		gpio_direction_input(GPIO_MMC_CD);
		ret = !gpio_get_value(GPIO_MMC_CD);
		break;
	}

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	s32 status = 0;
	u32 index = 0;

	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);

	usdhc_cfg[0].max_bus_width = 8;
	usdhc_cfg[1].max_bus_width = 4;

	for (index = 0; index < CONFIG_SYS_FSL_USDHC_NUM; ++index) {
		switch (index) {
		case 0:
			imx_iomux_v3_setup_multiple_pads(
				usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
			break;
		case 1:
			imx_iomux_v3_setup_multiple_pads(
				usdhc1_pads, ARRAY_SIZE(usdhc1_pads));
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
				"(%d) then supported by the board (%d)\n",
				index + 1, CONFIG_SYS_FSL_USDHC_NUM);
			return status;
		}

		status |= fsl_esdhc_initialize(bis, &usdhc_cfg[index]);
	}

	return status;
}
#endif

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	uint32_t base = IMX_FEC_BASE;
	struct mii_dev *bus = NULL;
	struct phy_device *phydev = NULL;
	int ret;

	/* provide the PHY clock from the i.MX 6 */
	ret = enable_fec_anatop_clock(0, ENET_50MHZ);
	if (ret)
		return ret;
	/* set gpr1[ENET_CLK_SEL] */
	setbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK);

	setup_iomux_enet();

#ifdef CONFIG_FEC_MXC
	bus = fec_get_miibus(base, -1);
	if (!bus)
		return 0;
	/* scan PHY 1..7 */
	phydev = phy_find_by_mask(bus, 0xff, PHY_INTERFACE_MODE_RMII);
	if (!phydev) {
		free(bus);
		puts("no PHY found\n");
		return 0;
	}
	phy_reset(phydev);
	printf("using PHY at %d\n", phydev->addr);
	ret = fec_probe(bis, -1, base, bus, phydev);
	if (ret) {
		printf("FEC MXC: %s:failed\n", __func__);
		free(phydev);
		free(bus);
	}
#endif
	return 0;
}

static iomux_v3_cfg_t const pwr_intb_pads[] = {
	/*
	 * the bootrom sets the iomux to vselect, potentially connecting
	 * two outputs. Set this back to GPIO
	 */
	MX6_PAD_GPIO_18__GPIO7_IO13 | MUX_PAD_CTRL(NO_PAD_CTRL)
};

#if defined(CONFIG_VIDEO_IPUV3)

static iomux_v3_cfg_t const backlight_pads[] = {
	/* Backlight On */
	MX6_PAD_EIM_D26__GPIO3_IO26 | MUX_PAD_CTRL(NO_PAD_CTRL),
#define RGB_BACKLIGHT_GP IMX_GPIO_NR(3, 26)
/* TODO PWM not GPIO */
	MX6_PAD_EIM_A22__GPIO2_IO16  | MUX_PAD_CTRL(NO_PULLUP),
	MX6_PAD_SD4_DAT1__GPIO2_IO09 | MUX_PAD_CTRL(NO_PAD_CTRL),
#define RGB_BACKLIGHTPWM_GP IMX_GPIO_NR(2, 9)
};

static iomux_v3_cfg_t const rgb_pads[] = {
		MX6_PAD_DI0_DISP_CLK__IPU1_DI0_DISP_CLK | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DI0_PIN15__IPU1_DI0_PIN15 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DI0_PIN2__IPU1_DI0_PIN02 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DI0_PIN3__IPU1_DI0_PIN03 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT0__IPU1_DISP0_DATA00 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT1__IPU1_DISP0_DATA01 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT2__IPU1_DISP0_DATA02 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT3__IPU1_DISP0_DATA03 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT4__IPU1_DISP0_DATA04 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT5__IPU1_DISP0_DATA05 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT6__IPU1_DISP0_DATA06 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT7__IPU1_DISP0_DATA07 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT8__IPU1_DISP0_DATA08 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT9__IPU1_DISP0_DATA09 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT10__IPU1_DISP0_DATA10 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT11__IPU1_DISP0_DATA11 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT12__IPU1_DISP0_DATA12 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT13__IPU1_DISP0_DATA13 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT14__IPU1_DISP0_DATA14 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT15__IPU1_DISP0_DATA15 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT16__IPU1_DISP0_DATA16 | MUX_PAD_CTRL(OUTPUT_RGB),
		MX6_PAD_DISP0_DAT17__IPU1_DISP0_DATA17 | MUX_PAD_CTRL(OUTPUT_RGB),
};

static void do_enable_hdmi(struct display_info_t const *dev)
{
	imx_enable_hdmi_phy();
}

static void enable_rgb(struct display_info_t const *dev)
{
	imx_iomux_v3_setup_multiple_pads(
		rgb_pads,
		ARRAY_SIZE(rgb_pads));
	gpio_direction_output(RGB_BACKLIGHT_GP, 1);
	gpio_direction_output(RGB_BACKLIGHTPWM_GP, 0);
}

static int detect_default(struct display_info_t const *dev)
{
	(void) dev;
	return 1;
}

struct display_info_t const displays[] = {{
	.bus	= -1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= detect_hdmi,
	.enable	= do_enable_hdmi,
	.mode	= {
		.name           = "HDMI",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= -1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB666,
	.detect	= detect_default,
	.enable	= enable_rgb,
	.mode	= {
		.name           = "vga-rgb",
		.refresh        = 60,
		.xres           = 640,
		.yres           = 480,
		.pixclock       = 33000,
		.left_margin    = 48,
		.right_margin   = 16,
		.upper_margin   = 31,
		.lower_margin   = 11,
		.hsync_len      = 96,
		.vsync_len      = 2,
		.sync           = 0,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= -1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB666,
	.enable	= enable_rgb,
	.mode	= {
		.name           = "wvga-rgb",
		.refresh        = 60,
		.xres           = 800,
		.yres           = 480,
		.pixclock       = 25000,
		.left_margin    = 40,
		.right_margin   = 88,
		.upper_margin   = 33,
		.lower_margin   = 10,
		.hsync_len      = 128,
		.vsync_len      = 2,
		.sync           = 0,
		.vmode          = FB_VMODE_NONINTERLACED
} } };
size_t display_count = ARRAY_SIZE(displays);

static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int reg;

	enable_ipu_clock();
	imx_setup_hdmi();
/* FIXME disable whatever LVDS stuff is initialized here */
	/* Turn on LDB0,IPU,IPU DI0 clocks */
	reg = __raw_readl(&mxc_ccm->CCGR3);
	reg |= MXC_CCM_CCGR3_LDB_DI0_MASK;
	writel(reg, &mxc_ccm->CCGR3);

	/* set LDB0, LDB1 clk select to 011/011 */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
		 |MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (3<<MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET)
	      |(3<<MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->cs2cdr);

	reg = readl(&mxc_ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV;
	writel(reg, &mxc_ccm->cscmr2);

	reg = readl(&mxc_ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<<MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->chsccdr);

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
	     |IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_HIGH
	     |IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW
	     |IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG
	     |IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT
	     |IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
	     |IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT
	     |IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED
	     |IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~(IOMUXC_GPR3_LVDS0_MUX_CTL_MASK
			|IOMUXC_GPR3_HDMI_MUX_CTL_MASK))
	    | (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
	       <<IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);

	/* backlight unconditionally on for now */
	imx_iomux_v3_setup_multiple_pads(backlight_pads,
					 ARRAY_SIZE(backlight_pads));
	/* use 0 for EDT 7", use 1 for LG fullHD panel */
	gpio_direction_output(RGB_BACKLIGHTPWM_GP, 0);
	gpio_direction_output(RGB_BACKLIGHT_GP, 1);
}
#endif /* defined(CONFIG_VIDEO_IPUV3) */

int board_early_init_f(void)
{
	imx_iomux_v3_setup_multiple_pads(pwr_intb_pads,
					ARRAY_SIZE(pwr_intb_pads));
	setup_iomux_uart();

#if defined(CONFIG_VIDEO_IPUV3)
	setup_display();
#endif
	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info_loc);

#ifdef CONFIG_TRDX_CMD_IMX_MFGR
	(void) pmic_init();
#endif

#ifdef CONFIG_CMD_SATA
	setup_sata();
#endif

	setup_iomux_gpio();

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	char env_str[256];
	u32 rev;

#if defined(CONFIG_REVISION_TAG) && defined(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)
	rev = get_board_rev();
	snprintf(env_str, ARRAY_SIZE(env_str), "%.4x", rev);
	setenv("board_rev", env_str);
#endif

	return 0;
}
#endif /* CONFIG_BOARD_LATE_INIT */

int checkboard_fallback(void)
{
	printf("Model: Toradex Colibri iMX6 %sMB\n",
		(gd->ram_size == 0x20000000) ? "512" : "256");
	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	{"mmc",	MAKE_CFGVAL(0x40, 0x20, 0x00, 0x00)},
	{NULL,	0},
};
#endif

int misc_init_r(void)
{
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif
	return 0;
}

/*
 * On Colibri iMX6 the DDR bus width depends on the CPU type
 * With Solo it is 32-bit, with DualLite 64-bit.
 * U-Boot is configured to use 32-bit on both models which works.
 * This commands patches this so that on subsequent boots a DL
 * will use 64-bit and thus all stuffed memory.
 */
int do_patch_ddr_size(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	char *ivt;
	unsigned len, *next = 0, *start_dcd = 0;
	struct mmc *mmc;
	int found = 0, ret = 0;

	ivt = memalign(ARCH_DMA_MINALIGN, 1024);
	if (ivt == NULL)
		goto fail;

	/* read IVT */
	mmc = find_mmc_device(0);
	/* Switch to primary eMMC boot area partition */
	mmc_switch_part(0, 1);
	ret = mmc->block_dev.block_read(0, 2, 2, ivt);
	if (ret != 2)
		goto fail;
	if (is_cpu_type(MXC_CPU_MX6DL)) {
		start_dcd = (unsigned *)(ivt + *(unsigned*)(&ivt[0xc]) -
					       *(unsigned*)(&ivt[0x14]));
		if ((*start_dcd & 0xfe0000ff) == 0x400000d2) {
			len = (*start_dcd & 0xffff00) >> 8;
			/* search register value for addr 0x21b0000 */
			for (next = start_dcd; next < (start_dcd + len/4);
			     next++) {
				if (*next == 0x00001b02) {
					next++;
					found = 1;
					break;
				}
			}
		}
		if (found) {
			if ((*next & 0x0000ff00) == 0x00001900) {
				*next &= ~0x0000ff00;
				*next |= 0x00001a00;
				ret = mmc->block_dev.block_write(0, 2, 2, ivt);
				puts("patched, ");
			}
		} else
			puts("reg/val pair not found, ");
	}
	/* Switch back to regular eMMC user partition */
	mmc_switch_part(0, 0);

	puts("done.\n");
	return CMD_RET_SUCCESS;

fail:
	puts("failed.\n");
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	patch_ddr_size, 1, 0, do_patch_ddr_size,
	"Patch the DCD table to the right ddr size depending on CPU type\n",
	""
);

#ifdef CONFIG_LDO_BYPASS_CHECK
/* TODO, use external pmic, for now always ldo_enable */
void ldo_mode_set(int ldo_bypass)
{
	return;
}
#endif
