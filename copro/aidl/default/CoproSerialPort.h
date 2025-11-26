#ifndef AIDL_ANDROID_HARDWARE_COPRO_COPROSERIALPORT_H
#define AIDL_ANDROID_HARDWARE_COPRO_COPROSERIALPORT_H

#include <aidl/android/hardware/copro/ICoproSerialPort.h>
#include <aidl/android/hardware/copro/BnCoproSerialPort.h>
#include <android/binder_interface_utils.h>
#include <android/binder_status.h>
#include <string>
#include <vector>

namespace aidl {
namespace android {
namespace hardware {
namespace copro {

class CoproSerialPort : public BnCoproSerialPort {
public:
    CoproSerialPort();
    virtual ~CoproSerialPort();

    // Implémentation des méthodes générées par AIDL
    ::ndk::ScopedAStatus open(int32_t in_mode, bool* _aidl_return) override;
    ::ndk::ScopedAStatus close() override;
    ::ndk::ScopedAStatus read(std::string* _aidl_return) override;
    ::ndk::ScopedAStatus write(const std::string& command) override;
    ::ndk::ScopedAStatus readB(int32_t size, std::vector<uint8_t>* _aidl_return) override;
    ::ndk::ScopedAStatus writeB(const std::vector<uint8_t>& command, int32_t* _aidl_return) override;

private:
    int mFdRpmsg;  // Descripteur de fichier pour le port série
};

}  // namespace copro
}  // namespace hardware
}  // namespace android
}  // namespace aidl

#endif  // AIDL_ANDROID_HARDWARE_COPRO_COPROSERIALPORT_H
