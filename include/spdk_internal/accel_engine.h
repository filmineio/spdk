/*-
 *   BSD LICENSE
 *
 *   Copyright (c) Intel Corporation.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SPDK_INTERNAL_ACCEL_ENGINE_H
#define SPDK_INTERNAL_ACCEL_ENGINE_H

#include "spdk/stdinc.h"

#include "spdk/accel_engine.h"
#include "spdk/queue.h"

struct spdk_accel_task {
	spdk_accel_completion_cb	cb;
	void				*cb_arg;
	uint8_t				offload_ctx[0];
};

struct spdk_accel_engine {
	uint64_t (*get_capabilities)(void);
	int (*copy)(struct spdk_io_channel *ch, void *dst, void *src,
		    uint64_t nbytes, spdk_accel_completion_cb cb_fn, void *cb_arg);
	int (*dualcast)(struct spdk_io_channel *ch, void *dst1, void *dst2, void *src,
			uint64_t nbytes, spdk_accel_completion_cb cb_fn, void *cb_arg);
	uint32_t (*batch_get_max)(void);
	struct spdk_accel_batch *(*batch_create)(struct spdk_io_channel *ch);
	int (*batch_prep_copy)(struct spdk_io_channel *ch, struct spdk_accel_batch *batch,
			       void *dst, void *src, uint64_t nbytes, spdk_accel_completion_cb cb_fn, void *cb_arg);
	int (*batch_prep_dualcast)(struct spdk_io_channel *ch, struct spdk_accel_batch *batch,
				   void *dst1, void *dst2, void *src, uint64_t nbytes,
				   spdk_accel_completion_cb cb_fn, void *cb_arg);
	int (*batch_prep_compare)(struct spdk_io_channel *ch, struct spdk_accel_batch *batch,
				  void *src1, void *src2, uint64_t nbytes, spdk_accel_completion_cb cb_fn, void *cb_arg);
	int (*batch_prep_fill)(struct spdk_io_channel *ch, struct spdk_accel_batch *batch,
			       void *dst, uint8_t fill, uint64_t nbytes, spdk_accel_completion_cb cb_fn, void *cb_arg);
	int (*batch_prep_crc32c)(struct spdk_io_channel *ch, struct spdk_accel_batch *batch,
				 uint32_t *dst, void *src, uint32_t seed, uint64_t nbytes,
				 spdk_accel_completion_cb cb_fn, void *cb_arg);
	int (*batch_submit)(struct spdk_io_channel *ch, struct spdk_accel_batch *batch,
			    spdk_accel_completion_cb cb_fn, void *cb_arg);
	int (*compare)(struct spdk_io_channel *ch, void *src1, void *src2,
		       uint64_t nbytes, spdk_accel_completion_cb cb_fn, void *cb_arg);
	int (*fill)(struct spdk_io_channel *ch, void *dst, uint8_t fill,
		    uint64_t nbytes, spdk_accel_completion_cb cb_fn, void *cb_arg);
	int (*crc32c)(struct spdk_io_channel *ch, uint32_t *dst, void *src,
		      uint32_t seed, uint64_t nbytes, spdk_accel_completion_cb cb_fn, void *cb_arg);
	struct spdk_io_channel *(*get_io_channel)(void);
};

struct spdk_accel_module_if {
	/** Initialization function for the module.  Called by the spdk
	 *   application during startup.
	 *
	 *  Modules are required to define this function.
	 */
	int	(*module_init)(void);

	/** Finish function for the module.  Called by the spdk application
	 *   before the spdk application exits to perform any necessary cleanup.
	 *
	 *  Modules are not required to define this function.
	 */
	void	(*module_fini)(void *ctx);

	/** Function called to return a text string representing the
	 *   module's configuration options for inclusion in an
	 *   spdk configuration file.
	 */
	void	(*config_text)(FILE *fp);

	/**
	 * Write Acceleration module configuration into provided JSON context.
	 */
	void	(*write_config_json)(struct spdk_json_write_ctx *w);

	/**
	 * Returns the allocation size required for the modules to use for context.
	 */
	size_t	(*get_ctx_size)(void);

	TAILQ_ENTRY(spdk_accel_module_if)	tailq;
};

void spdk_accel_hw_engine_register(struct spdk_accel_engine *accel_engine);
void spdk_accel_module_list_add(struct spdk_accel_module_if *accel_module);

#define SPDK_ACCEL_MODULE_REGISTER(init_fn, fini_fn, config_fn, config_json, ctx_size_fn)				\
	static struct spdk_accel_module_if init_fn ## _if = {						\
	.module_init		= init_fn,								\
	.module_fini		= fini_fn,								\
	.config_text		= config_fn,								\
	.write_config_json	= config_json,								\
	.get_ctx_size		= ctx_size_fn,								\
	};												\
	__attribute__((constructor)) static void init_fn ## _init(void)					\
	{												\
		spdk_accel_module_list_add(&init_fn ## _if);						\
	}

#endif
