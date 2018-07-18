#include <linux/mm.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>
#include <linux/swiotlb.h>
#include <linux/bootmem.h>

#include <asm/bootinfo.h>
#include <boot_param.h>
#include <dma-coherence.h>

static inline void *dma_to_virt(struct device *dev, dma_addr_t dma_addr)
{
	return phys_to_virt(dma_to_phys(dev, dma_addr));
}

static void *loongson_dma_alloc_coherent(struct device *dev, size_t size,
		dma_addr_t *dma_handle, gfp_t gfp, struct dma_attrs *attrs)
{
	void *ret;

	if (dma_alloc_from_coherent(dev, size, dma_handle, &ret))
		return ret;

	/* ignore region specifiers */
	gfp &= ~(__GFP_DMA | __GFP_DMA32 | __GFP_HIGHMEM);

#ifdef CONFIG_ISA
	if (dev == NULL)
		gfp |= __GFP_DMA;
	else
#endif
#ifdef CONFIG_ZONE_DMA
	if (dev->coherent_dma_mask < DMA_BIT_MASK(32))
		gfp |= __GFP_DMA;
	else
#endif
#ifdef CONFIG_ZONE_DMA32
	if (dev->coherent_dma_mask < DMA_BIT_MASK(40))
		gfp |= __GFP_DMA32;
	else
#endif
	;
	gfp |= __GFP_NORETRY;

	ret = swiotlb_alloc_coherent(dev, size, dma_handle, gfp);
	if (!plat_device_is_coherent(dev)) {
		dma_cache_wback_inv((unsigned long)dma_to_virt(dev, *dma_handle), size);
		ret = UNCAC_ADDR(ret);
	}
	mb();

	return ret;
}

static void loongson_dma_free_coherent(struct device *dev, size_t size,
		void *vaddr, dma_addr_t dma_handle, struct dma_attrs *attrs)
{
	int order = get_order(size);

	if (dma_release_from_coherent(dev, order, vaddr))
		return;

	if (!plat_device_is_coherent(dev)) {
		vaddr = CAC_ADDR(vaddr);
		dma_cache_wback_inv((unsigned long)dma_to_virt(dev, dma_handle), size);
	}
	swiotlb_free_coherent(dev, size, vaddr, dma_handle);
}

static int loongson_dma_mmap(struct device *dev, struct vm_area_struct *vma,
	void *cpu_addr, dma_addr_t dma_addr, size_t size, struct dma_attrs *attrs)
{
	int ret = -ENXIO;
	unsigned long user_count = vma_pages(vma);
	unsigned long count = PAGE_ALIGN(size) >> PAGE_SHIFT;
	unsigned long pfn = page_to_pfn(virt_to_page(cpu_addr));
	unsigned long off = vma->vm_pgoff;

	if (!plat_device_is_coherent(dev)) {
		if (dma_get_attr(DMA_ATTR_WRITE_COMBINE, attrs))
			vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
		else
			vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	}

	if (dma_mmap_from_coherent(dev, vma, cpu_addr, size, &ret))
		return ret;

	if (off < count && user_count <= (count - off)) {
		ret = remap_pfn_range(vma, vma->vm_start, pfn + off,
				      user_count << PAGE_SHIFT, vma->vm_page_prot);
	}

	return ret;
}

static dma_addr_t loongson_dma_map_page(struct device *dev, struct page *page,
				unsigned long offset, size_t size,
				enum dma_data_direction dir,
				struct dma_attrs *attrs)
{
	dma_addr_t daddr = swiotlb_map_page(dev, page, offset, size,
					dir, attrs);
	if (!plat_device_is_coherent(dev))
		dma_cache_sync(dev, dma_to_virt(dev, daddr), size, dir);
	mb();

	return daddr;
}

static void loongson_dma_unmap_page(struct device *dev, dma_addr_t dev_addr,
			size_t size, enum dma_data_direction dir,
			struct dma_attrs *attrs)
{
	if (!plat_device_is_coherent(dev))
		dma_cache_sync(dev, dma_to_virt(dev, dev_addr), size, dir);
	swiotlb_unmap_page(dev, dev_addr, size, dir, attrs);
}

static int loongson_dma_map_sg(struct device *dev, struct scatterlist *sgl,
				int nents, enum dma_data_direction dir,
				struct dma_attrs *attrs)
{
	int i, r;
	struct scatterlist *sg;

	r = swiotlb_map_sg_attrs(dev, sgl, nents, dir, NULL);
	if (!plat_device_is_coherent(dev)) {
		for_each_sg(sgl, sg, nents, i)
			dma_cache_sync(dev, dma_to_virt(dev, sg->dma_address), sg->length, dir);
	}
	mb();

	return r;
}

static void loongson_dma_unmap_sg(struct device *dev, struct scatterlist *sgl,
			int nelems, enum dma_data_direction dir,
			struct dma_attrs *attrs)
{
	int i;
	struct scatterlist *sg;

	if (!plat_device_is_coherent(dev) && dir != DMA_TO_DEVICE) {
		for_each_sg(sgl, sg, nelems, i)
			dma_cache_sync(dev, dma_to_virt(dev, sg->dma_address), sg->length, dir);
	}

	swiotlb_unmap_sg_attrs(dev, sgl, nelems, dir, attrs);
}

static void loongson_dma_sync_single_for_cpu(struct device *dev, dma_addr_t dev_addr,
			size_t size, enum dma_data_direction dir)
{
	if (!plat_device_is_coherent(dev))
		dma_cache_sync(dev, dma_to_virt(dev, dev_addr), size, dir);
	swiotlb_sync_single_for_cpu(dev, dev_addr, size, dir);
}

static void loongson_dma_sync_single_for_device(struct device *dev,
				dma_addr_t dma_handle, size_t size,
				enum dma_data_direction dir)
{
	swiotlb_sync_single_for_device(dev, dma_handle, size, dir);
	if (!plat_device_is_coherent(dev))
		dma_cache_sync(dev, dma_to_virt(dev, dma_handle), size, dir);
	mb();
}

static void loongson_dma_sync_sg_for_cpu(struct device *dev,
				struct scatterlist *sgl, int nents,
				enum dma_data_direction dir)
{
	int i;
	struct scatterlist *sg;

	if (!plat_device_is_coherent(dev)) {
		for_each_sg(sgl, sg, nents, i) {
			dma_cache_sync(dev, dma_to_virt(dev,
				sg->dma_address), sg->length, dir);
		}
	}
	swiotlb_sync_sg_for_cpu(dev, sgl, nents, dir);
}

static void loongson_dma_sync_sg_for_device(struct device *dev,
				struct scatterlist *sgl, int nents,
				enum dma_data_direction dir)
{
	int i;
	struct scatterlist *sg;

	swiotlb_sync_sg_for_device(dev, sgl, nents, dir);
	if (!plat_device_is_coherent(dev)) {
		for_each_sg(sgl, sg, nents, i) {
			dma_cache_sync(dev, dma_to_virt(dev,
				sg->dma_address), sg->length, dir);
		}
	}
	mb();
}

static int loongson_dma_set_mask(struct device *dev, u64 mask)
{
	if (mask > DMA_BIT_MASK(loongson_sysconf.dma_mask_bits)) {
		*dev->dma_mask = DMA_BIT_MASK(loongson_sysconf.dma_mask_bits);
		return -EIO;
	}

	*dev->dma_mask = mask;

	return 0;
}

dma_addr_t phys_to_dma(struct device *dev, phys_addr_t paddr)
{
	long nid;
#ifdef CONFIG_PHYS48_TO_HT40
	/* We extract 2bit node id (bit 44~47, only bit 44~45 used now) from
	 * Loongson-3's 48bit address space and embed it into 40bit */
	nid = (paddr >> 44) & 0x3;
	paddr = ((nid << 44) ^ paddr) | (nid << 37);
#endif
	return paddr;
}

phys_addr_t dma_to_phys(struct device *dev, dma_addr_t daddr)
{
	long nid;
#ifdef CONFIG_PHYS48_TO_HT40
	/* We extract 2bit node id (bit 44~47, only bit 44~45 used now) from
	 * Loongson-3's 48bit address space and embed it into 40bit */
	nid = (daddr >> 37) & 0x3;
	daddr = ((nid << 37) ^ daddr) | (nid << 44);
#endif
	return daddr;
}

static struct dma_map_ops loongson_dma_map_ops = {
	.alloc = loongson_dma_alloc_coherent,
	.free = loongson_dma_free_coherent,
	.mmap = loongson_dma_mmap,
	.map_page = loongson_dma_map_page,
	.unmap_page = loongson_dma_unmap_page,
	.map_sg = loongson_dma_map_sg,
	.unmap_sg = loongson_dma_unmap_sg,
	.sync_single_for_cpu = loongson_dma_sync_single_for_cpu,
	.sync_single_for_device = loongson_dma_sync_single_for_device,
	.sync_sg_for_cpu = loongson_dma_sync_sg_for_cpu,
	.sync_sg_for_device = loongson_dma_sync_sg_for_device,
	.mapping_error = swiotlb_dma_mapping_error,
	.dma_supported = swiotlb_dma_supported,
	.set_dma_mask = loongson_dma_set_mask
};

void __init plat_swiotlb_setup(void)
{
	swiotlb_init(1);
	mips_dma_map_ops = &loongson_dma_map_ops;
}
