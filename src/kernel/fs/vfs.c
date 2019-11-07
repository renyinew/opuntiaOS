#include <fs/vfs.h>

// Private
vfs_device_t _vfs_devices[VFS_MAX_DEV_COUNT];
fs_desc_t _vfs_fses[VFS_MAX_FS_COUNT];
_vfs_devices_count = 0;
_vfs_fses_count = 0;


// Public

void vfs_install() {
    device_t cur_dev;
    uint8_t start_s = 0;
    cur_dev.type = DEVICE_STORAGE;
    while (cur_dev.type != DEVICE_BAD_SIGN) {
        cur_dev = get_device(cur_dev.type, start_s);
        if (cur_dev.type != DEVICE_BAD_SIGN) {
            vfs_add_device(cur_dev);
        }
        start_s = cur_dev.id + 1;
    }
}

void vfs_add_device(device_t t_new_dev) {
    if (t_new_dev.type != DEVICE_STORAGE) {
        return;
    }
    _vfs_devices[_vfs_devices_count].dev = t_new_dev;
    for (uint8_t i = 0; i < _vfs_fses_count; i++) {
        bool (*is_capable)(vfs_device_t *nd) = _vfs_fses[i].recognize;
        if (is_capable(&_vfs_devices[_vfs_devices_count])) {
            _vfs_devices[_vfs_devices_count++].fs = i;
            return;
        }
    }
    printf("Can't find FS\n");
}

void vfs_add_fs(fs_desc_t t_new_fs) {
    _vfs_fses[_vfs_fses_count++] = t_new_fs;
}
