eclaration -MD -MT esp-idf/camera/CMakeFiles/__idf_camera.dir/camera.c.obj -MF esp-idf/camera/CMakeFiles/__idf_camera.dir/camera.c.obj.d -o esp-idf/camera/CMakeFiles/__idf_camera.dir/camera.c.obj -c /home/ming/data/Project/ClionProject/ESP32/camera_test1/components/camera/camera.c
/home/ming/data/Project/ClionProject/ESP32/camera_test1/components/camera/camera.c:8:10: fatal error: wifi.h: No such file or directory
    8 | #include "wifi.h"
      |          ^~~~~~~~
compilation terminated.
[1003/1109] Performing configure step for 'bootloader'
-- Minimal build - OFF
-- Building ESP-IDF components for target esp32s3
-- Project sdkconfig file /home/ming/data/Project/ClionProject/ESP32/camera_test1/sdkconfig
-- Compiler supported targets: xtensa-esp-elf
-- Adding linker script /opt/esp-idf/components/soc/esp32s3/ld/esp32s3.peripherals.ld
-- Bootloader project name: "bootloader" version: 1
-- Adding linker script /opt/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.ld
-- Adding linker script /opt/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.api.ld
-- Adding linker script /opt/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.bt_funcs.ld
-- Adding linker script /opt/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.libgcc.ld
-- Adding linker script /opt/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.wdt.ld
-- Adding linker script /opt/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.version.ld
-- Adding linker script /opt/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.libc.ld
-- Adding linker script /opt/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.newlib.ld
-- Adding linker script /opt/esp-idf/components/bootloader/subproject/main/ld/esp32s3/bootloader.ld
-- Adding linker script /opt/esp-idf/components/bootloader/subproject/main/ld/esp32s3/bootloader.rom.ld
-- Components: bootloader bootloader_support efuse esp_app_format esp_bootloader_format esp_common esp_hw_support esp_rom esp_security esp_system esptool_py freertos hal log main micro-ecc newlib partition_table soc spi_flash xtensa
-- Component paths: /opt/esp-idf/components/bootloader /opt/esp-idf/components/bootloader_support /opt/esp-idf/components/efuse /opt/esp-idf/components/esp_app_format /opt/esp-idf/components/esp_bootloader_format /opt/esp-idf/components/esp_common /opt/esp-idf/components/esp_hw_support /opt/esp-idf/components/esp_rom /opt/esp-idf/components/esp_security /opt/esp-idf/components/esp_system /opt/esp-idf/components/esptool_py /opt/esp-idf/components/freertos /opt/esp-idf/components/hal /opt/esp-idf/components/log /opt/esp-idf/components/bootloader/subproject/main /opt/esp-idf/components/bootloader/subproject/components/micro-ecc /opt/esp-idf/components/newlib /opt/esp-idf/components/partition_table /opt/esp-idf/components/soc /opt/esp-idf/components/spi_flash /opt/esp-idf/components/xtensa
-- Configuring done (4.8s)
-- Generating done (0.0s)
-- Build files have been written to: /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/bootloader
ninja: build stopped: subcommand failed.
Missing "wifi.h" file name found in the following component(s): esp_wifi(/opt/esp-idf/components/esp_wifi/include/esp_private/wifi.h). Maybe one of the components needs to add the missing header directory to INCLUDE_DIRS of idf_component_register call in CMakeLists.txt. Another possibility may be that the component or its feature is not enabled in the configuration. Use "idf.py menuconfig" to check if the required options are enabled.
ninja failed with exit code 1, output of the command is in the /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/log/idf_py_stderr_output_583432 and /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/log/idf_py_stdout_output_583432

╭─░▒▓     ~/d/P/C/ESP32/camera_test1  on    main ⇡5 !3 ?3 ▓▒░       ░▒▓ 2 ✘  took 17s    base    22.19.0    at 12:19:48   ▓▒░─╮
╰─                                                                                                                                         ─╯

python python/start_fpv.py

============================================================
🎮 ESP32 FPV Camera System Launcher
============================================================
🔍 检查运行环境...
✅ CUDA加速可用
✅ 环境检查通过
🚀 启动Web查看器在 http://0.0.0.0:5000
❌ 启动Web查看器失败: invalid syntax (fpv_receiver.py, line 34)
❌ Web查看器启动失败
