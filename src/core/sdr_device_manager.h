#ifndef SDR_DEVICE_MANAGER_H
#define SDR_DEVICE_MANAGER_H

#include <string>
#include <vector>
#include <memory>

#include "core/singleton.h"
#include "core/devices/sdr_device.h"

typedef enum {
    UNSELECTED_SOURCE = -1,
    RTL_SDR_USB = 0
} device_source_t;

typedef struct device_entry {
    std::string deviceName;
    std::string serial;
    int deviceIndex;
} device_entry_t;

class SdrDeviceManager : public Singleton<SdrDeviceManager>
{
    // enable to base class Singleton to call private/protected methods
    friend class Singleton<SdrDeviceManager>;
private:
    std::unique_ptr<SdrDevice> _currentDevice;

    device_source_t _selectedSource = UNSELECTED_SOURCE;

    std::vector<device_entry_t> _availableDevices;

    SdrDeviceManager() = default; // only Singleton class can construct it

    ~SdrDeviceManager() = default;

    std::unique_ptr<SdrDevice> create_device();

    int scan_rtl_devices();
public:
    SdrDevice* currentDevice() { return this->_currentDevice.get(); }

    const device_source_t selectedSource() { return this->_selectedSource; }

    const std::vector<device_entry_t>& availableDevices() { return this->_availableDevices; }

    int scan_devices(device_source_t source);

    int set_current_device(int index, sdr_config_t init_config);
};

#endif // SDR_DEVICE_MANAGER_H
