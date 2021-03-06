/*
 * Copyright (C) 2013 Red Hat
 * Author: Rob Clark <robdclark@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/dma-buf.h>
#include "etnaviv_drv.h"
#include "etnaviv_gem.h"

#include <linux/dma-buf.h>

struct sg_table *etnaviv_gem_prime_get_sg_table(struct drm_gem_object *obj)
{
	struct etnaviv_gem_object *etnaviv_obj = to_etnaviv_bo(obj);

	BUG_ON(!etnaviv_obj->sgt);  /* should have already pinned! */

	return etnaviv_obj->sgt;
}

void *etnaviv_gem_prime_vmap(struct drm_gem_object *obj)
{
	return etnaviv_gem_vaddr(obj);
}

void etnaviv_gem_prime_vunmap(struct drm_gem_object *obj, void *vaddr)
{
	/* TODO msm_gem_vunmap() */
}

int etnaviv_gem_prime_pin(struct drm_gem_object *obj)
{
	if (!obj->import_attach) {
		struct drm_device *dev = obj->dev;

		mutex_lock(&dev->struct_mutex);
		etnaviv_gem_get_pages(obj);
		mutex_unlock(&dev->struct_mutex);
	}
	return 0;
}

void etnaviv_gem_prime_unpin(struct drm_gem_object *obj)
{
	if (!obj->import_attach) {
		struct drm_device *dev = obj->dev;

		mutex_lock(&dev->struct_mutex);
		msm_gem_put_pages(obj);
		mutex_unlock(&dev->struct_mutex);
	}
}

static void etnaviv_gem_prime_release(struct etnaviv_gem_object *etnaviv_obj)
{
	if (etnaviv_obj->vaddr)
		dma_buf_vunmap(etnaviv_obj->base.import_attach->dmabuf,
			       etnaviv_obj->vaddr);

	/* Don't drop the pages for imported dmabuf, as they are not
	 * ours, just free the array we allocated:
	 */
	if (etnaviv_obj->pages)
		drm_free_large(etnaviv_obj->pages);

	drm_prime_gem_destroy(&etnaviv_obj->base, etnaviv_obj->sgt);
}

static const struct etnaviv_gem_ops etnaviv_gem_prime_ops = {
	/* .get_pages should never be called */
	.release = etnaviv_gem_prime_release,
};

struct drm_gem_object *etnaviv_gem_prime_import_sg_table(struct drm_device *dev,
		struct dma_buf_attachment *attach, struct sg_table *sgt)
{
	struct etnaviv_gem_object *etnaviv_obj;
	uint32_t size = attach->dmabuf->size;
	int ret, npages;

	size = PAGE_ALIGN(size);

	ret = etnaviv_gem_new_private(dev, size, ETNA_BO_WC, &etnaviv_obj);
	if (ret < 0)
		return ERR_PTR(ret);

	npages = size / PAGE_SIZE;

	etnaviv_obj->ops = &etnaviv_gem_prime_ops;
	etnaviv_obj->sgt = sgt;
	etnaviv_obj->pages = drm_malloc_ab(npages, sizeof(struct page *));
	if (!etnaviv_obj->pages) {
		ret = -ENOMEM;
		goto fail;
	}

	ret = drm_prime_sg_to_page_addr_arrays(sgt, etnaviv_obj->pages,
					       NULL, npages);
	if (ret)
		goto fail;

	return &etnaviv_obj->base;

fail:
	drm_gem_object_unreference_unlocked(&etnaviv_obj->base);

	return ERR_PTR(ret);
}
