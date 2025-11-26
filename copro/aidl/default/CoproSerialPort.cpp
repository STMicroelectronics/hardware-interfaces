#include "CoproSerialPort.h"
#include <android-base/logging.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define SERIAL_DEVICE_PATH "/dev/ttyRPMSG0"
#define COPRO_TTYRMPSG_OPEN_TIMEOUT_S 4
#define MAX_BUFFER_SIZE 1024

namespace aidl::android::hardware::copro {

CoproSerialPort::CoproSerialPort() : mFdRpmsg(-1) {}

CoproSerialPort::~CoproSerialPort() {
    if (mFdRpmsg != -1) {
        ::close(mFdRpmsg);
    }
}

::ndk::ScopedAStatus CoproSerialPort::open(int32_t in_mode, bool* _aidl_return) {
    LOG(VERBOSE) << "Start CoproSerialPort::open with mode " << in_mode;
    struct termios tiorpmsg;
    int timeout = COPRO_TTYRMPSG_OPEN_TIMEOUT_S;

    if (mFdRpmsg == -1) {
        while ((mFdRpmsg = ::open(SERIAL_DEVICE_PATH, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0) {
            sleep(1);
            timeout--;
            if (timeout == 0) {
                LOG(ERROR) << "Failed to open " << SERIAL_DEVICE_PATH << " after timeout";
                return ::ndk::ScopedAStatus::fromServiceSpecificError(errno);
            }
            LOG(WARNING) << "Failed to open, retrying for " << timeout << " seconds";
        }

        LOG(VERBOSE) << "Configuring CoproSerialPort::open";
        tcgetattr(mFdRpmsg, &tiorpmsg);  // Get current port settings

        if (in_mode == 0) {  // MODE_RAW
            LOG(VERBOSE) << "CoproSerialPort::open in RAW mode";
            tiorpmsg.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
            tiorpmsg.c_oflag &= ~OPOST;
            tiorpmsg.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
            tiorpmsg.c_cflag &= ~(CSIZE | PARENB);
            tiorpmsg.c_cflag |= CS8;
        } else {
            LOG(VERBOSE) << "CoproSerialPort::open in non-RAW mode";
            tiorpmsg.c_lflag &= ~ECHO;  // Disable ECHO
            tiorpmsg.c_oflag &= ~ONLCR; // Do not convert LF to CR LF
        }

        tcsetattr(mFdRpmsg, TCSANOW, &tiorpmsg);
    }

    *_aidl_return = true;
    LOG(VERBOSE) << "End CoproSerialPort::open";
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus CoproSerialPort::close() {
    LOG(VERBOSE) << "Start CoproSerialPort::close";
    if (mFdRpmsg != -1) {
        if (::close(mFdRpmsg) < 0) {
            LOG(ERROR) << "Error closing " << SERIAL_DEVICE_PATH << ", err=" << errno;
            return ::ndk::ScopedAStatus::fromServiceSpecificError(errno);
        }
        mFdRpmsg = -1;
    }
    LOG(VERBOSE) << "End CoproSerialPort::close";
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus CoproSerialPort::read(std::string* _aidl_return) {
    LOG(VERBOSE) << "Start CoproSerialPort::read";
    if (mFdRpmsg == -1) {
        LOG(ERROR) << "Error reading " << SERIAL_DEVICE_PATH << ", file descriptor is not set";
        return ::ndk::ScopedAStatus::fromServiceSpecificError(errno);
    }

    int byte_avail = 0;
    ioctl(mFdRpmsg, FIONREAD, &byte_avail);

    if (byte_avail > 0) {
        _aidl_return->resize(byte_avail);
        int byte_rd = ::read(mFdRpmsg, &_aidl_return->at(0), byte_avail);
        if (byte_rd < 0) {
            LOG(ERROR) << "Error reading from " << SERIAL_DEVICE_PATH << ", err=" << errno;
            return ::ndk::ScopedAStatus::fromServiceSpecificError(errno);
        }
        LOG(INFO) << "Read " << byte_rd << " bytes from " << SERIAL_DEVICE_PATH;
    }

    LOG(VERBOSE) << "End CoproSerialPort::read";
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus CoproSerialPort::write(const std::string& command) {
    LOG(VERBOSE) << "Start CoproSerialPort::write";
    if (mFdRpmsg == -1) {
        LOG(ERROR) << "Error writing " << SERIAL_DEVICE_PATH << ", file descriptor is not set";
        return ::ndk::ScopedAStatus::fromServiceSpecificError(errno);
    }

    if (::write(mFdRpmsg, command.c_str(), command.size()) <= 0) {
        LOG(ERROR) << "Error writing to " << SERIAL_DEVICE_PATH << ", err=" << errno;
        return ::ndk::ScopedAStatus::fromServiceSpecificError(errno);
    }

    LOG(VERBOSE) << "End CoproSerialPort::write";
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus CoproSerialPort::readB(int32_t size, std::vector<uint8_t>* _aidl_return) {
    LOG(VERBOSE) << "Start CoproSerialPort::readB with size " << size;
    if (mFdRpmsg == -1) {
        LOG(ERROR) << "Error reading " << SERIAL_DEVICE_PATH << ", file descriptor is not set";
        return ::ndk::ScopedAStatus::fromServiceSpecificError(errno);
    }

    int byte_avail = 0;
    ioctl(mFdRpmsg, FIONREAD, &byte_avail);

    if (byte_avail > 0) {
        int bytes_to_read = std::min(size, byte_avail);
        _aidl_return->resize(bytes_to_read);
        int byte_rd = ::read(mFdRpmsg, _aidl_return->data(), bytes_to_read);
        if (byte_rd < 0) {
            LOG(ERROR) << "Error reading from " << SERIAL_DEVICE_PATH << ", err=" << errno;
            return ::ndk::ScopedAStatus::fromServiceSpecificError(errno);
        }
        LOG(INFO) << "Read " << byte_rd << " bytes from " << SERIAL_DEVICE_PATH;
    }

    LOG(VERBOSE) << "End CoproSerialPort::readB";
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus CoproSerialPort::writeB(const std::vector<uint8_t>& command, int32_t* _aidl_return) {
    LOG(VERBOSE) << "Start CoproSerialPort::writeB";
    if (mFdRpmsg == -1) {
        LOG(ERROR) << "Error writing " << SERIAL_DEVICE_PATH << ", file descriptor is not set";
        return ::ndk::ScopedAStatus::fromServiceSpecificError(errno);
    }

    int ret_val = ::write(mFdRpmsg, command.data(), command.size());
    if (ret_val <= 0) {
        LOG(ERROR) << "Error writing to " << SERIAL_DEVICE_PATH << ", err=" << errno;
        return ::ndk::ScopedAStatus::fromServiceSpecificError(errno);
    }

    *_aidl_return = ret_val;
    LOG(VERBOSE) << "End CoproSerialPort::writeB";
    return ::ndk::ScopedAStatus::ok();
}

}  // namespace aidl::android::hardware::copro
