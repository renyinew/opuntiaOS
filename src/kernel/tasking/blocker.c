/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <tasking/proc.h>

int should_unblock_join_block(proc_t* proc)
{
    // TODO: Add more checks here.
    if (proc->joinee->status == PROC_DEAD) {
        return 1;
    }
    return 0;
}

int init_join_blocker(proc_t* proc)
{
    proc->status = PROC_BLOCKED;
    proc->blocker.reason = BLOCKER_JOIN;
    proc->blocker.should_unblock = should_unblock_join_block;
    return 0;
}