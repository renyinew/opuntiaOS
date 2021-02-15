#include <errno.h>
#include <fs/vfs.h>
#include <io/tty/pty_master.h>
#include <io/tty/pty_slave.h>
#include <log.h>
#include <utils/kassert.h>

/**
 * Since pty masters are virtual files and don't present on real hd,
 * we use an array of dentries. This dentries won't be present in the
 * list of dentries (dentry.c), so other threads con't access it.
 */
pty_master_entry_t pty_masters[PTYS_COUNT];

int _pty_master_free_dentry_data(dentry_t* dentry);
bool pty_master_can_read(dentry_t* dentry, uint32_t start);
bool pty_master_can_write(dentry_t* dentry, uint32_t start);
int pty_master_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len);
int pty_master_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len);

static fs_ops_t pty_master_ops = {
    .recognize = 0,
    .prepare_fs = 0,
    .eject_device = 0,
    .dentry = {
        .read_inode = 0,
        .write_inode = 0,
        .free_inode = _pty_master_free_dentry_data,
        .get_fsdata = 0,
    },
    .file = {
        .can_read = pty_master_can_read,
        .can_write = pty_master_can_write,
        .read = pty_master_read,
        .write = pty_master_write,
        .open = 0,
        .truncate = 0,
        .create = 0,
        .unlink = 0,
        .getdents = 0,
        .lookup = 0,
        .mkdir = 0,
        .rmdir = 0,
        .ioctl = 0,
        .mmap = 0,
    }
};

static pty_master_entry_t* _ptm_get(dentry_t* dentry)
{
    for (int i = 0; i < PTYS_COUNT; i++) {
        if (dentry == &pty_masters[i].dentry) {
            return &pty_masters[i];
        }
    }
    return 0;
}

int _pty_master_free_dentry_data(dentry_t* dentry)
{
    return 0;
}

bool pty_master_can_read(dentry_t* dentry, uint32_t start)
{
    pty_master_entry_t* ptm = _ptm_get(dentry);
    ASSERT(ptm);
    return ringbuffer_space_to_read(&ptm->buffer) >= 1;
}

bool pty_master_can_write(dentry_t* dentry, uint32_t start)
{
    pty_master_entry_t* ptm = _ptm_get(dentry);
    ASSERT(ptm);
    return ringbuffer_space_to_write(&ptm->pts->buffer) >= 0;
}

int pty_master_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    pty_master_entry_t* ptm = _ptm_get(dentry);
    ASSERT(ptm);
    uint32_t leno = ringbuffer_space_to_read(&ptm->buffer);
    if (leno > len) {
        leno = len;
    }
    int res = ringbuffer_read(&ptm->buffer, buf, leno);
    return leno;
}

int pty_master_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    pty_master_entry_t* ptm = _ptm_get(dentry);
    ASSERT(ptm);
    ringbuffer_write(&ptm->pts->buffer, buf, len);
    return len;
}

int pty_master_alloc(file_descriptor_t* fd)
{
    pty_master_entry_t* ptm = 0;
    for (int i = 0; i < PTYS_COUNT; i++) {
        if (pty_masters[i].dentry.inode_indx == 0) {
            /* According to dentry.c when inode_indx==0, dentry is free */
            ptm = &pty_masters[i];
            ptm->dentry.inode_indx = i + 1;
            break;
        }
    }

    if (!ptm) {
        return -EBUSY;
    }

    /* 
       DENTRY_CUSTOM is set for the dentry, since it's not a cache of
       a file (or a dir). We also set a required function for this
       type of dentries free_inode, which is called when dentry is
       freed.
       
       DENTRY_PRIVATE shows that dentry and fd, shouldn't be copied
       when a new process is forked.
    */
    ptm->dentry.d_count = 1;
    ptm->dentry.flags = 0;
    dentry_set_flag(&ptm->dentry, DENTRY_PRIVATE);
    dentry_set_flag(&ptm->dentry, DENTRY_CUSTOM);
    ptm->dentry.ops = &pty_master_ops;

    fd->dentry = &ptm->dentry;
    fd->ops = &pty_master_ops.file;
    fd->mapped_times = 0;
    fd->flags = 0;
    fd->offset = 0;
    fd->type = FD_TYPE_FILE;

    pty_slave_create(ptm->dentry.inode_indx, ptm);
    ptm->buffer = ringbuffer_create_std();

    return 0;
}