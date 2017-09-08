/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Fabio Estevam <fabio.estevam@freescale.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <linux/list.h>
#include <asm/gpio.h>
#include <asm/arch/iomux-mx51.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>

#define MX51EVK_LCD_3V3		IMX_GPIO_NR(4, 9)
#define MX51EVK_LCD_5V		IMX_GPIO_NR(4, 10)
#define MX51EVK_LCD_BACKLIGHT	IMX_GPIO_NR(3, 4)

static struct fb_videomode const claa_wvga = {
	.name		= "CLAA07LC0ACW",
	.refresh	= 57,
	.xres		= 800,
	.yres		= 480,
	.pixclock	= 37037,
	.left_margin	= 40,
	.right_margin	= 60,
	.upper_margin	= 10,
	.lower_margin	= 10,
	.hsync_len	= 20,
	.vsync_len	= 10,
	.sync		= 0,
	.vmode		= FB_VMODE_NONINTERLACED
};

static struct fb_videomode const dvi = {
	.name		= "DVI panel",
	.refresh	= 60,
	.xres		= 1024,
	.yres		= 768,
	.pixclock	= 15385,
	.left_margin	= 220,
	.right_margin	= 40,
	.upper_margin	= 21,
	.lower_margin	= 7,
	.hsync_len	= 60,
	.vsync_len	= 10,
	.sync		= 0,
	.vmode		= FB_VMODE_NONINTERLACED
};

static struct fb_videomode const my_vga = {
	.name		= "MY-VGA",
	.refresh	= 57,
	.xres		= 640,
	.yres		= 480,
	.pixclock	= 40000,
	.left_margin	= 144,
	.right_margin	= 16,
	.upper_margin	= 33,
	.lower_margin	= 12,
	.hsync_len	= 96,
	.vsync_len	= 2,
	.sync		= 0,
	.vmode		= FB_VMODE_NONINTERLACED
};

void setup_iomux_lcd(void)
{
	/* DI2_PIN15 */
	imx_iomux_v3_setup_pad(MX51_PAD_DI_GP4__DI2_PIN15);

	/* Pad settings for DI2_DISP_CLK */
	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_DI2_DISP_CLK__DI2_DISP_CLK,
			    PAD_CTL_PKE | PAD_CTL_DSE_MAX | PAD_CTL_SRE_SLOW));

	/* Turn on 3.3V voltage for LCD */
	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_CSI2_D12__GPIO4_9,
						NO_PAD_CTRL));
	gpio_direction_output(MX51EVK_LCD_3V3, 1);

	/* Turn on 5V voltage for LCD */
	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_CSI2_D13__GPIO4_10,
						NO_PAD_CTRL));
	gpio_direction_output(MX51EVK_LCD_5V, 1);

	/* Turn on GPIO backlight */
	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_DI1_D1_CS__GPIO3_4,
						NO_PAD_CTRL));
	gpio_direction_output(MX51EVK_LCD_BACKLIGHT, 1);
}

int board_video_skip(void)
{
	int ret;
	/*
	char const *e = getenv("panel");

	if (e) {
		if (strcmp(e, "claa") == 0) {
			ret = ipuv3_fb_init(&claa_wvga, 1, IPU_PIX_FMT_RGB565);
			if (ret)
				printf("claa cannot be configured: %d\n", ret);
			return ret;
		}
	}
	
	ret = ipuv3_fb_init(&dvi, 0, IPU_PIX_FMT_RGB24);*/
	ret = ipuv3_fb_init(&my_vga, 0, IPU_PIX_FMT_LVDS666);
	if (ret)
		printf("MY VGA cannot be configured: %d\n", ret);
	return ret;
}
