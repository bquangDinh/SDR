#ifndef SDR_DEVICE_MANAGER_H
#define SDR_DEVICE_MANAGER_H

#include <string>
#include <vector>
#include <memory>

#include "core/singleton.h"
#include "core/devices/sdr_device.h"

// Device source types and device entry structures for SDR device management
enum class DEVICE_SOURCE {
    UNSELECTED_SOURCE = -1,
    RTL_SDR_USB = 0
};

// Structure representing an SDR device entry in the available devices list
struct device_entry {
    std::string deviceName;
    std::string serial;
    int deviceIndex;
};

// Structure representing the initialization parameters for an SDR device
struct device_init {
    int deviceIndex;
    sdr_config initConfig;
};

class SdrDeviceManager : public Singleton<SdrDeviceManager>
{
    // enable to base class Singleton to call private/protected methods
    friend class Singleton<SdrDeviceManager>;
private:
    // Currently active SDR device
    std::unique_ptr<SdrDevice> _currentDevice;

    // Currently selected device source
    DEVICE_SOURCE _selectedSource = DEVICE_SOURCE::UNSELECTED_SOURCE;
    
    // List of available SDR devices
    std::vector<device_entry> _availableDevices;

    device_init _initParams;

    SdrDeviceManager() = default; // only Singleton class can construct it

    ~SdrDeviceManager() = default;

    // Create a new SDR device instance based on the selected source
    std::unique_ptr<SdrDevice> create_device();

    // Scan for RTL-SDR devices and populate the available devices list
    int scan_rtl_devices();

    // Set the initialization parameters for the current SDR device
    // This one is used with set_current_device() to store the initialization parameters before initializing the device
    void set_init_params(int index, sdr_config initConfig);
public:
    // Get the currently active SDR device
    SdrDevice* currentDevice() { return this->_currentDevice.get(); }

    /**
     * Get the index of the currently active SDR device in the available devices list
     * @return The index of the current device, or -1 if no device is active
     */
    int currentDeviceIndex() const { return this->_currentDevice == nullptr ? -1 : _initParams.deviceIndex; }

    // Get the currently selected device source
    const DEVICE_SOURCE selectedSource() { return this->_selectedSource; }

    // Get the list of available SDR devices
    const std::vector<device_entry>& availableDevices() { return this->_availableDevices; }

    /**
     * Scan for devices of the specified source and populate the available devices list
     * @param source The device source to scan for
     * @return 0 on success, -1 on failure
     */
    int scan_devices(DEVICE_SOURCE source);

    /**
     * Set the current SDR device by index and store its initialization parameters
     * @param index The index of the device in the available devices list
     * @param initConfig The initialization configuration for the device
     * @return 0 on success, -1 on failure
     */
    int set_current_device(int index, sdr_config initConfig);

    /**
     * Initialize the currently active SDR device with the stored initialization parameters
     * @return 0 on success, -1 on failure
     */
    int init_current_device();

    /**
     * Destroy the currently active SDR device
     * @return 0 on success, -1 on failure
     */
    int destroy_current_device();

    /**
     * Initialize a device with the specified configuration, creating and setting it as the current device if necessary
     * @param deviceIndex The index of the device in the available devices list
     * @param initConfig The initialization configuration for the device
     * @return 0 on success, -1 on failure
     */
    int init_device_with_config(int deviceIndex, sdr_config initConfig);

    /**
     * Get the count of available SDR devices
     * @return The number of available SDR devices
     */
    int devices_count() const { return static_cast<int>(_availableDevices.size()); }

    /**
     * Check if there is no currently active SDR device
     * @return true if there is no active device, false otherwise
     */
    bool device_empty() const { return this->_currentDevice == nullptr; }
};

#endif // SDR_DEVICE_MANAGER_H
