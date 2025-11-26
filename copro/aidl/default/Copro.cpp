#include "Copro.h"
#include "CoproSerialPort.h"
#include <android-base/logging.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <vector>
#include <string>

#define MAX_BUFFER_SIZE 80

namespace aidl {
namespace android {
namespace hardware {
namespace copro {

Copro::Copro() {
    mIdCurrentRunningFw = -1;
}

Copro::~Copro() {}

::ndk::ScopedAStatus Copro::getFirmwareList(std::vector<FirmwareInfo>* _aidl_return) {
    LOG(VERBOSE) << "COPRO AIDL: entering getFirmwareList";
    initFirmwareList();
    *_aidl_return = mFwList;
    LOG(VERBOSE) << "COPRO AIDL: end getFirmwareList";
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Copro::getFirmwareByName(const std::string& name, FirmwareInfo* _aidl_return) {
    LOG(VERBOSE) << "COPRO AIDL: entering getFirmwareByName";
    initFirmwareList();

    for (const auto& fw : mFwList) {
        if (fw.name == name) {
            *_aidl_return = fw;
            return ::ndk::ScopedAStatus::ok();
        }
    }

    return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
}

::ndk::ScopedAStatus Copro::isFirmwareRunning(int id, bool* _aidl_return) {
    LOG(VERBOSE) << "COPRO AIDL: entering isFirmwareRunning";
    if (id >= 0 && id < mFwList.size()) {
        *_aidl_return = mFwList[id].state;
        return ::ndk::ScopedAStatus::ok();
    }
    return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
}

::ndk::ScopedAStatus Copro::startFirmware(int id) {
    LOG(VERBOSE) << "COPRO AIDL: entering startFirmware";
    if (id < 0 || id >= mFwList.size() || getCurrentFirmwareState()) {
        return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    if (setFirmwarePath(mFwList[id].name)) {
        int fd = open("/sys/class/remoteproc/remoteproc0/state", O_RDWR);
        if (fd < 0) {
            LOG(ERROR) << "Error opening remoteproc0/state, err=" << errno;
            return ::ndk::ScopedAStatus::fromServiceSpecificError(errno);
        }

        write(fd, "start", strlen("start"));
        close(fd);

        mFwList[id].state = true;
        mIdCurrentRunningFw = id;
        return ::ndk::ScopedAStatus::ok();
    }

    return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
}

::ndk::ScopedAStatus Copro::stopFirmware() {
    LOG(VERBOSE) << "COPRO AIDL: entering stopFirmware";
    if (mIdCurrentRunningFw < 0 || mIdCurrentRunningFw >= mFwList.size()) {
        return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    int fd = open("/sys/class/remoteproc/remoteproc0/state", O_RDWR);
    if (fd < 0) {
        LOG(ERROR) << "Error opening remoteproc0/state, err=" << errno;
        return ::ndk::ScopedAStatus::fromServiceSpecificError(errno);
    }

    write(fd, "stop", strlen("stop"));
    close(fd);

    mFwList[mIdCurrentRunningFw].state = false;
    mIdCurrentRunningFw = -1;
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Copro::getSerialPort(std::shared_ptr<ICoproSerialPort>* _aidl_return) {
    *_aidl_return = ndk::SharedRefBase::make<CoproSerialPort>();
    return ::ndk::ScopedAStatus::ok();
}

void Copro::initFirmwareList() {
    mFwList.clear();
    mIdCurrentRunningFw = -1;

    DIR* dir = opendir(FIRMWARE_PATH.c_str());
    if (!dir) {
        LOG(ERROR) << "Error opening firmware directory";
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG) {
            FirmwareInfo fw;
            fw.id = mFwList.size();
            fw.name = entry->d_name;
            fw.state = (getCurrentFirmware() == (FIRMWARE_PREFIX_PATH + fw.name) && getCurrentFirmwareState());
            mFwList.push_back(fw);

            if (fw.state) {
                mIdCurrentRunningFw = fw.id;
            }
        }
    }

    closedir(dir);
}

std::string Copro::getCurrentFirmware() {
    int fd = open("/sys/class/remoteproc/remoteproc0/firmware", O_RDONLY);
    if (fd < 0) {
        LOG(ERROR) << "Error opening remoteproc0/firmware, err=" << errno;
        return "";
    }

    char buf[MAX_BUFFER_SIZE];
    ssize_t bytesRead = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (bytesRead > 0) {
        buf[bytesRead - 1] = '\0';  // Remove trailing newline
        return std::string(buf);
    }

    return "";
}

bool Copro::getCurrentFirmwareState() {
    int fd = open("/sys/class/remoteproc/remoteproc0/state", O_RDONLY);
    if (fd < 0) {
        LOG(ERROR) << "Error opening remoteproc0/state, err=" << errno;
        return false;
    }

    char buf[MAX_BUFFER_SIZE];
    ssize_t bytesRead = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (bytesRead > 0) {
        buf[bytesRead - 1] = '\0';  // Remove trailing newline
        return std::string(buf) == "running";
    }

    return false;
}

bool Copro::setFirmwarePath(const std::string& fileName) {
    int fd = open("/sys/class/remoteproc/remoteproc0/firmware", O_WRONLY);
    if (fd < 0) {
        LOG(ERROR) << "Error opening remoteproc0/firmware, err=" << errno;
        return false;
    }

    std::string path = FIRMWARE_PREFIX_PATH + fileName;
    ssize_t result = write(fd, path.c_str(), path.size());
    close(fd);

    return result > 0;
}

::ndk::ScopedAStatus Copro::getInterfaceHash(std::string* _aidl_return) {
    *_aidl_return = "";
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Copro::getInterfaceVersion(int32_t* _aidl_return) {
    *_aidl_return = 1;
    return ::ndk::ScopedAStatus::ok();
}

}  // namespace copro
}  // namespace hardware
}  // namespace android
}  // namespace aidl
