#ifndef AIDL_ANDROID_HARDWARE_COPRO_ICOPRO_H
#define AIDL_ANDROID_HARDWARE_COPRO_ICOPRO_H

#include <aidl/android/hardware/copro/FirmwareInfo.h>
#include <aidl/android/hardware/copro/ICoproSerialPort.h>
#include <aidl/android/hardware/copro/BnCopro.h>
#include <android/binder_interface_utils.h>
#include <android/binder_status.h>
#include <vector>
#include <string>
#include <memory>

namespace aidl {
namespace android {
namespace hardware {
namespace copro {

class Copro : public BnCopro {
public:
    Copro();
    virtual ~Copro();

    ::ndk::ScopedAStatus getFirmwareList(std::vector<FirmwareInfo>* _aidl_return) override;
    ::ndk::ScopedAStatus getFirmwareByName(const std::string& name, FirmwareInfo* _aidl_return) override;
    ::ndk::ScopedAStatus isFirmwareRunning(int id, bool* _aidl_return) override;
    ::ndk::ScopedAStatus startFirmware(int id) override;
    ::ndk::ScopedAStatus stopFirmware() override;
    ::ndk::ScopedAStatus getSerialPort(std::shared_ptr<ICoproSerialPort>* _aidl_return) override;

    ::ndk::ScopedAStatus getInterfaceHash(std::string* _aidl_return);
    ::ndk::ScopedAStatus getInterfaceVersion(int32_t* _aidl_return);
private:
    const std::string FIRMWARE_PREFIX_PATH = "copro/";
    const std::string FIRMWARE_PATH = "/vendor/firmware/copro/";

    void initFirmwareList();
    std::string getCurrentFirmware();
    bool getCurrentFirmwareState();
    bool setFirmwarePath(const std::string& fileName);

    std::vector<FirmwareInfo> mFwList;
    int mIdCurrentRunningFw;
};

}  // namespace copro
}  // namespace hardware
}  // namespace android
}  // namespace aidl

#endif  // AIDL_ANDROID_HARDWARE_COPRO_ICOPRO_H
