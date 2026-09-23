#include "sdr_device_manager.h"

#include <cassert>

#include "core/devices/rtl_sdr_device.h"

std::unique_ptr<SdrDevice> SdrDeviceManager::create_device() {
    switch (selectedSource()) {
        case RTL_SDR_USB:
            return std::make_unique<RtlSdrDevice>();
        default:
            return nullptr;
    }

    return nullptr;
}

int SdrDeviceManager::scan_rtl_devices() {
    _availableDevices.clear();

    uint32_t count = RtlSdrDevice::get_devices_count();

    if (count == 0) {
        return 0;
    }

    for (uint32_t i = 0; i < count; ++i) {
        char manufact[256] = { 0 };
        char product[256] = { 0 };
        char serial[256] = { 0 };

        if (RtlSdrDevice::get_device_info_from_index(i, manufact, product, serial) < 0) return -1;

        device_entry_t entry;

        entry.deviceIndex = i;
        entry.serial = serial;
        entry.deviceName = RtlSdrDevice::get_device_name_from_index(i);

        _availableDevices.push_back(entry);
    }

    return 0;
}

int SdrDeviceManager::scan_devices(device_source_t source) {
    switch (source) {
        case RTL_SDR_USB:
            _selectedSource = source;

            return scan_rtl_devices();
        default:
            return -1;
    }

    return -1;
}

int SdrDeviceManager::set_current_device(int index, sdr_config_t init_config) {
    assert(availableDevices().size() > 0 && "List of available devices is empty. You may forgot to call scan_devices()");

    assert(index >= 0 && index < availableDevices().size() && "Device Entry index is not valid");

    if (_currentDevice) {
        _currentDevice->sdr_destroy();
    }

    _currentDevice = create_device();

    int rc = _currentDevice->sdr_init(index, init_config);

    if (rc < 0) {
        _currentDevice = nullptr;

        return -1;
    }

    return 0;
}