#ifndef FIRMWAREINFO_H
#define FIRMWAREINFO_H

#include <string>
#include <optional>

namespace aidl {
namespace android {
namespace hardware {
namespace copro {

class FirmwareInfo {
public:
    FirmwareInfo(int id, const std::optional<std::string>& name, bool state)
        : id(id), name(name), state(state) {}
    int getId() const { return id; }
    std::optional<std::string> getName() const { return name; }
    bool getState() const { return state; }

private:
    int id;                                // ID of the firmware
    std::optional<std::string> name;       // Name of the firmware (nullable)
    bool state;                            // State of the firmware
};

} // namespace copro
} // namespace hardware
} // namespace android
} // namespace aidl

#endif // FIRMWAREINFO_H
