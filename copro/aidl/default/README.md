# hardware-copro #

This module contains the STMicroelectronics Copro HAL source code.
It is part of the STMicroelectronics delivery for Android.

## Description ##

This module implements android.hardware.copro AIDL version 1.
Please see the Android delivery release notes for more details.

## Documentation ##

* The [release notes][] provide information on the release.
[release notes]: https://wiki.st.com/stm32mpu-ecosystem-v5/wiki/STM32_MPU_OpenSTDroid_release_note_-_v5.1.0

## Dependencies ##

This module can't be used alone. It is part of the STMicroelectronics Android delivery.

To be able to use it the device.mk must have the following packages:
```
PRODUCT_PACKAGES += \
    android.hardware.copro-service.stm32mpu \
    CoproService
```

## Containing ##

This directory contains the sources and associated Android makefile to generate the copro hardware service binary.

## License ##

This module is distributed under the Apache License, Version 2.0 found in the [LICENSE](./LICENSE) file.
