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

int SdrDeviceManager::set_current_device(int index, sdr_config_t initConfig) {
    assert(availableDevices().size() > 0 && "List of available devices is empty. You may forgot to call scan_devices()");

    assert(index >= 0 && index < availableDevices().size() && "Device Entry index is not valid");

    // Just in case I forgot to destroy it
    if (!device_empty()) {
        if (destroy_current_device() < 0) return -1;
    }

    _currentDevice = create_device();

    // sdr_init will be called when needed
    set_init_params(index, initConfig);

    return 0;
}

int SdrDeviceManager::init_current_device() {
    assert(_currentDevice && "No device created");

    assert(!_currentDevice->initialized() && "The device has been initialized");

    return _currentDevice->sdr_init(_initParams.deviceIndex, _initParams.initConfig);
}

int SdrDeviceManager::destroy_current_device() {
    assert(_currentDevice && "No device created");

    int res = _currentDevice->sdr_destroy();

    if (res < 0) return res;

    _currentDevice = nullptr;

    return 0;
}

int SdrDeviceManager::init_device_with_config(int deviceIndex, sdr_config_t initConfig) {
    set_current_device(deviceIndex, initConfig);

    return init_current_device();
}

void SdrDeviceManager::set_init_params(int index, sdr_config_t initConfig) {
    assert(index >= 0 && index < availableDevices().size() && "Device Entry index is not valid");

    _initParams.deviceIndex = index;

    _initParams.initConfig = initConfig;
}