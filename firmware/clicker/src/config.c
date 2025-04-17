#include "config.h"

#include <zephyr/drivers/flash.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>

LOG_MODULE_REGISTER(app_config);

// NVS partition
#define NVS_PARTITION        storage_partition
#define NVS_PARTITION_DEVICE FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET FIXED_PARTITION_OFFSET(NVS_PARTITION)
#define NVS_FS_ENTRY_ID      1

// Global state
static bool is_initialized = false;

static struct nvs_fs fs;

/*********************************************************************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************************************************************/

static int init_nvs() {
    if (is_initialized) return 0;

    if (!device_is_ready(NVS_PARTITION_DEVICE)) {
        LOG_ERR("NVS flash device is not ready");
        return -ENODEV;
    }

    struct flash_pages_info info;
    int res = flash_get_page_info_by_offs(NVS_PARTITION_DEVICE, NVS_PARTITION_OFFSET, &info);
    if (res) {
        LOG_ERR("Failed to get page info: %d", res);
        return res;
    }

    fs.flash_device = NVS_PARTITION_DEVICE;
    fs.offset       = NVS_PARTITION_OFFSET;
    fs.sector_size  = info.size;
    fs.sector_count = 3;

    res = nvs_mount(&fs);
    if (res) {
        LOG_ERR("Failed to mount NVS: %d", res);
        return res;
    }

    is_initialized = true;
    return 0;
}

/*********************************************************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************************************************/

int config_load(struct config_t *config) {
    // Initialize the NVS if necessary
    int res = init_nvs();
    if (res) return res;

    // Load the configuration from NVS
    res = nvs_read(&fs, NVS_FS_ENTRY_ID, config, sizeof(*config));
    if (res > 0) {
        LOG_INF("Configuration loaded from NVS (%d bytes)", res);
    } else if (res == -ENOENT) {
        LOG_WRN("Configuration not found in NVS, using default values");
        memset(config, 0, sizeof(*config));
    } else {
        LOG_ERR("Failed to read configuration from NVS: %d", res);
        return res;
    }

    return 0;
}

int config_save(const struct config_t *config) {
    // Initialize the NVS if necessary
    int res = init_nvs();
    if (res) return res;

    // Save the configuration to NVS
    res = nvs_write(&fs, NVS_FS_ENTRY_ID, config, sizeof(*config));
    if (res > 0) {
        LOG_INF("Configuration saved to NVS (%d bytes)", res);
    } else if (res == 0) {
        LOG_INF("Configuration already up-to-date, nothing written");
    } else {
        LOG_ERR("Failed to write configuration to NVS: %d", res);
        return res;
    }

    return 0;
}