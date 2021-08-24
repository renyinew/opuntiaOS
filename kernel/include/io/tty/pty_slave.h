/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_IO_TTY_PTY_SLAVE_H
#define _KERNEL_IO_TTY_PTY_SLAVE_H

#include <algo/sync_ringbuffer.h>

#ifndef PTYS_COUNT
#define PTYS_COUNT 4
#endif

struct pty_master_entry;
struct pty_slave_entry {
    int inode_indx;
    struct pty_master_entry* ptm;
    sync_ringbuffer_t buffer;
};
typedef struct pty_slave_entry pty_slave_entry_t;

extern pty_slave_entry_t pty_slaves[PTYS_COUNT];

int pty_slave_create(int id, struct pty_master_entry* ptm);

#endif