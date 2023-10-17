// SPDX-License-Identifier: GPL-2.0+
/*
 * board/renesas/common/gen5-common.c
 *
 * Copyright (C) 2024 Renesas Electronics Corp.
 */

#include <asm/arch/renesas.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/processor.h>
#include <asm/system.h>
#include <linux/errno.h>

DECLARE_GLOBAL_DATA_PTR;

static void init_generic_timer(void)
{
	const u32 freq = CONFIG_SYS_CLK_FREQ;

	/* Update memory mapped and register based freqency */
	asm volatile ("msr cntfrq_el0, %0" :: "r" (freq));
	writel(freq, CNTFID0);

	/* Enable counter */
	setbits_le32(CNTCR_BASE, CNTCR_EN);
}

static void init_gic_v3(void)
{
	/* GIC v3 power on */
	writel(BIT(1), GICR_LPI_PWRR);

	/* Wait till the WAKER_CA_BIT changes to 0 */
	clrbits_le32(GICR_LPI_WAKER, BIT(1));
	while (readl(GICR_LPI_WAKER) & BIT(2))
		;

	writel(0xffffffff, GICR_SGI_BASE + GICR_IGROUPR0);
}

void s_init(void)
{
	if (current_el() == 3)
		init_generic_timer();
}

int board_early_init_f(void)
{
	return 0;
}

int board_init(void)
{
	if (current_el() != 3)
		return 0;
	init_gic_v3();

	return 0;
}

#define RST_BASE	0xC1320000 /* Domain0 */
#define RST_SWSRES1A	(RST_BASE + 0x410)
#define RST_RESKCPROT0	(RST_BASE + 0x4F0)
#define RST_KCPROT	0xA5A5A500
#define RST_KCE		0x1

void __weak reset_cpu(void)
{
	writel(RST_KCPROT | RST_KCE, RST_RESKCPROT0);
	writel(1, RST_SWSRES1A);
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}
