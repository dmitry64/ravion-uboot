/*
 * Toradex Colibri PXA320 support file
 *
 * Copyright (C) 2009 Marek Vasut <marek.vasut@gmail.com>
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
#include <asm/arch/pxa-regs.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscelaneous platform dependent initialisations
 */

int board_init (void)
{
	/* arch number for linux kernel */
	gd->bd->bi_arch_number = MACH_TYPE_COLIBRI300;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0xa0000100;

	/* configuration for ethernet chip */
	GPIO1 = 0x1c01;
	MSC0 = 0x7ff07ff0;
	MSC1 = 0x7ff80779;
	CSADRCFG0 = 0x00020000;
	CSADRCFG1 = 0x00020000;
	CSADRCFG2 = 0x0032c809;
	CSADRCFG3 = 0x00020000;
	CSMSADRCFG = 2;

	/* MMC */
	GPIO3 = 0x804;
	GPIO4 = 0x804;
	GPIO5 = 0x804;
	GPIO6 = 0x804;
	GPIO7 = 0x804;
	GPIO14 = 0x805;

	/* USB host */
	GPIO0_2 = 0x801;
	GPIO1_2 = 0x801;

	return 0;
}

int board_late_init(void)
{
	setenv("stdout", "serial");
	setenv("stderr", "serial");
	return 0;
}

int dram_init (void)
{
	uint32_t cpuid;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	/* Check if CPU is PXA310. PXA310 colibri has twice as much RAM */
	asm volatile("mrc p15, 0, %0, c0, c0, 0" : "=r"(cpuid));
	if (cpuid & 0x10)	/* PXA310 */
		gd->bd->bi_dram[0].size <<= 1;

	return 0;
}

#ifdef	CONFIG_CMD_USB
int usb_board_init(void)
{
	UHCHR = (UHCHR | UHCHR_PCPL | UHCHR_PSPL) &
		~(UHCHR_SSEP0 | UHCHR_SSEP1 | UHCHR_SSEP2 | UHCHR_SSE);

	UHCHR |= UHCHR_FSBIR;

	while (UHCHR & UHCHR_FSBIR);

	UHCHR &= ~UHCHR_SSE;
	UHCHIE = (UHCHIE_UPRIE | UHCHIE_RWIE);

	UHCRHDA &= ~(0x200);
	UHCRHDA |= 0x100;

	/* Set port power control mask bits, only 3 ports. */
	UHCRHDB |= (0x7<<17);

	/* enable port 2 */
	UP2OCR |= UP2OCR_HXOE | UP2OCR_HXS | UP2OCR_DMPDE | UP2OCR_DPPDE;

//	CKENA &= ~CKENA_2_USBHOST;

	return 0;
}

void usb_board_init_fail(void)
{
	return;
}

void usb_board_stop(void)
{
	UHCHR |= UHCHR_FHR;
	udelay(11);
	UHCHR &= ~UHCHR_FHR;

	UHCCOMS |= 1;
	udelay(10);

	CKENA &= ~CKENA_2_USBHOST;

	return;
}
#endif
