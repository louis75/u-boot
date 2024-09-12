// SPDX-License-Identifier: GPL-2.0+
/*
 * Renesas RCar Gen3 memory map tables
 *
 * Copyright (C) 2017 Marek Vasut <marek.vasut@gmail.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/armv8/mmu.h>
#include <asm/global_data.h>

#define GEN3_NR_REGIONS 16

static struct mm_region gen3_mem_map[GEN3_NR_REGIONS] = {
	{
		.virt = ADDR_ASSIGN_RGID(0x0UL, CONFIG_RCAR_RGID),
		.phys = ADDR_ASSIGN_RGID(0x0UL, CONFIG_RCAR_RGID),
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = ADDR_ASSIGN_RGID(0x40000000UL, CONFIG_RCAR_RGID),
		.phys = ADDR_ASSIGN_RGID(0x40000000UL, CONFIG_RCAR_RGID),
		.size = 0x03F00000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = ADDR_ASSIGN_RGID(0x48000000UL, CONFIG_RCAR_RGID),
		.phys = ADDR_ASSIGN_RGID(0x48000000UL, CONFIG_RCAR_RGID),
		.size = 0x15c00000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = ADDR_ASSIGN_RGID(0xc0000000UL, CONFIG_RCAR_RGID),
		.phys = ADDR_ASSIGN_RGID(0xc0000000UL, CONFIG_RCAR_RGID),
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt =ADDR_ASSIGN_RGID(0x100000000UL, CONFIG_RCAR_RGID),
		.phys = ADDR_ASSIGN_RGID(0x100000000UL, CONFIG_RCAR_RGID),
		.size = 0xf00000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = gen3_mem_map;

DECLARE_GLOBAL_DATA_PTR;

void enable_caches(void)
{
	u64 start, size;
	int bank, i = 0;

	/* Create map for RPC access */
	gen3_mem_map[i].virt = ADDR_ASSIGN_RGID(0x0ULL, CONFIG_RCAR_RGID);
	gen3_mem_map[i].phys = ADDR_ASSIGN_RGID(0x0ULL, CONFIG_RCAR_RGID);
	gen3_mem_map[i].size = 0x40000000ULL;
	gen3_mem_map[i].attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
				PTE_BLOCK_NON_SHARE |
				PTE_BLOCK_PXN | PTE_BLOCK_UXN;
	i++;

	/* Generate entires for DRAM in 32bit address space */
	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		start = gd->bd->bi_dram[bank].start;
		size = gd->bd->bi_dram[bank].size;

		/* Skip empty DRAM banks */
		if (!size)
			continue;

		/* Skip DRAM above 4 GiB */
		if (start >> 32ULL)
			continue;

		/* Mark memory reserved by ATF as cacheable too. */
		if (start == 0x48000000) {
			/* Unmark protection area (0x43F00000 to 0x47DFFFFF) */
			gen3_mem_map[i].virt = ADDR_ASSIGN_RGID(0x40000000ULL, CONFIG_RCAR_RGID);
			gen3_mem_map[i].phys = ADDR_ASSIGN_RGID(0x40000000ULL, CONFIG_RCAR_RGID);
			gen3_mem_map[i].size = 0x03F00000ULL;
			gen3_mem_map[i].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
						PTE_BLOCK_INNER_SHARE;
			i++;

			start = ADDR_ASSIGN_RGID(0x47E00000ULL, CONFIG_RCAR_RGID);
			size += 0x00200000ULL;
		}

		gen3_mem_map[i].virt = start;
		gen3_mem_map[i].phys = ADDR_ASSIGN_RGID(start, CONFIG_RCAR_RGID);
		gen3_mem_map[i].size = size;
		gen3_mem_map[i].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
					PTE_BLOCK_INNER_SHARE;
		i++;
	}

	/* Create map for register access */
	gen3_mem_map[i].virt = ADDR_ASSIGN_RGID(0xc0000000ULL, CONFIG_RCAR_RGID);
	gen3_mem_map[i].phys = ADDR_ASSIGN_RGID(0xc0000000ULL, CONFIG_RCAR_RGID);
	gen3_mem_map[i].size = 0x40000000ULL;
	gen3_mem_map[i].attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
				PTE_BLOCK_NON_SHARE |
				PTE_BLOCK_PXN | PTE_BLOCK_UXN;
	i++;

	/* Generate entires for DRAM in 64bit address space */
	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		start = gd->bd->bi_dram[bank].start;
		size = gd->bd->bi_dram[bank].size;

		/* Skip empty DRAM banks */
		if (!size)
			continue;

		/* Skip DRAM below 4 GiB */
		if (!(start >> 32ULL))
			continue;

		gen3_mem_map[i].virt = ADDR_ASSIGN_RGID(start, CONFIG_RCAR_RGID);
		gen3_mem_map[i].phys = ADDR_ASSIGN_RGID(start, CONFIG_RCAR_RGID);
		gen3_mem_map[i].size = size;
		gen3_mem_map[i].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
					PTE_BLOCK_INNER_SHARE;
		i++;
	}

	/* Zero out the remaining regions. */
	for (; i < GEN3_NR_REGIONS; i++) {
		gen3_mem_map[i].virt = 0;
		gen3_mem_map[i].phys = 0;
		gen3_mem_map[i].size = 0;
		gen3_mem_map[i].attrs = 0;
	}

	if (!icache_status())
		icache_enable();

	gd->arch.tlb_addr = ADDR_ASSIGN_RGID(gd->arch.tlb_addr, CONFIG_RCAR_RGID);
	dcache_enable();
}
