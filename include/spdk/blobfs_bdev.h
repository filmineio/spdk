/*   SPDX-License-Identifier: BSD-3-Clause
 *   Copyright (C) 2019 Intel Corporation.
 *   All rights reserved.
 */

/** \file
 * Operations on blobfs whose backing device is spdk_bdev
 */

#ifndef SPDK_BLOBFS_BDEV_H
#define SPDK_BLOBFS_BDEV_H

#include "spdk/stdinc.h"
#include "spdk/bdev.h"
#include "spdk/config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * blobfs on bdev operation completion callback.
 *
 * \param cb_arg Callback argument.
 * \param fserrno 0 if it completed successfully, or negative errno if it failed.
 */
typedef void (*spdk_blobfs_bdev_op_complete)(void *cb_arg, int fserrno);

/**
 * Detect whether blobfs exists on the given device.
 *
 * \param bdev_name Name of block device.
 * \param cb_fn Called when the detecting is complete. fserrno is -EILSEQ if no blobfs exists.
 * \param cb_arg Argument passed to function cb_fn.
 */
void spdk_blobfs_bdev_detect(const char *bdev_name,
			     spdk_blobfs_bdev_op_complete cb_fn, void *cb_arg);

/**
 * Create a blobfs on the given device.
 *
 * \param bdev_name Name of block device.
 * \param cluster_sz Size of cluster in bytes. Must be multiple of 4KiB page size.
 * \param cb_fn Called when the creation is complete.
 * \param cb_arg Argument passed to function cb_fn.
 */
void spdk_blobfs_bdev_create(const char *bdev_name, uint32_t cluster_sz,
			     spdk_blobfs_bdev_op_complete cb_fn, void *cb_arg);

/**
 * Mount a blobfs on given device to a host path by FUSE
 *
 * A new thread is created dedicatedly for one mountpoint to handle FUSE request
 * by blobfs API.
 *
 * \param bdev_name Name of block device.
 * \param mountpoint Host path to mount blobfs.
 * \param cb_fn Called when mount operation is complete. fserrno is -EILSEQ if no blobfs exists.
 * \param cb_arg Argument passed to function cb_fn.
 */
void spdk_blobfs_bdev_mount(const char *bdev_name, const char *mountpoint,
			    spdk_blobfs_bdev_op_complete cb_fn, void *cb_arg);

/**
 * Unmount a blobfs on given device from a host path by FUSE
 *
 * A new thread is created dedicatedly for one mountpoint to handle FUSE request
 * by blobfs API.
 *
 * \param bdev_name Name of block device.
 * \param mountpoint Host path to unmount blobfs from.
 * \param cb_fn Called when mount operation is complete. fserrno is -EILSEQ if no blobfs exists.
 * \param cb_arg Argument passed to function cb_fn.
 */
void spdk_blobfs_bdev_unmount(const char *bdev_name, const char *mountpoint,
			    spdk_blobfs_bdev_op_complete cb_fn, void *cb_arg);

struct blobfs_bdev_operation_ctx {
	char *bdev_name;
	struct spdk_filesystem *fs;

	/* If cb_fn is already called in other function, not _blobfs_bdev_unload_cb.
	 * cb_fn should be set NULL after its being called, in order to avoid repeated
	 * calling in _blobfs_bdev_unload_cb.
	 */
	spdk_blobfs_bdev_op_complete cb_fn;
	void *cb_arg;

	/* Variables for mount operation */
	char *mountpoint;
	struct spdk_thread *fs_loading_thread;

	/* Used in bdev_event_cb to do some proper operations on blobfs_fuse for
	 * asynchronous event of the backend bdev.
	 */
	struct spdk_blobfs_fuse *bfuse;
};

void free_blobfs_bdev_operation_ctx(struct blobfs_bdev_operation_ctx *ctx);

#ifdef __cplusplus
}
#endif

#endif /* SPDK_BLOBFS_BDEV_H */
