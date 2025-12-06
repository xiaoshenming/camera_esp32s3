❯ idf.py build &&  idf.py flash monitor
Executing action: all (aliases: build)
Running ninja in directory /home/ming/data/Project/ClionProject/ESP32/camera_test1/build
Executing "ninja all"...
[7/1109] Generating ../../partition_table/partition-table.bin
Partition table binary generated. Contents:
*******************************************************************************
# ESP-IDF Partition Table
# Name, Type, SubType, Offset, Size, Flags
nvs,data,nvs,0x9000,24K,
phy_init,data,phy,0xf000,4K,
factory,app,factory,0x10000,1M,
*******************************************************************************
[1101/1109] Performing configure step for 'bootloader'
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
-- Configuring done (4.7s)
-- Generating done (0.0s)
-- Build files have been written to: /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/bootloader
[1/1] cd /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/bootloader/esp-idf/esptool_py && /home/mi...et 0x8000 bootloader 0x0 /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/bootloader/bootloader.bin
Bootloader binary size 0x5240 bytes. 0x2dc0 bytes (36%) free.
[1108/1109] Generating binary image from built executable
esptool.py v4.10.dev2
Creating esp32s3 image...
Merged 2 ELF sections
Successfully created esp32s3 image.
Generated /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/camera_test1.bin
[1109/1109] cd /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/esp-idf/esptool_py && /home/ming/.e...rtition_table/partition-table.bin /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/camera_test1.bin
camera_test1.bin binary size 0x5ab00 bytes. Smallest app partition is 0x100000 bytes. 0xa5500 bytes (65%) free.

Project build complete. To flash, run:
 idf.py flash
or
 idf.py -p PORT flash
or
 python -m esptool --chip esp32s3 -b 460800 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_size 2MB --flash_freq 80m 0x0 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x10000 build/camera_test1.bin
or from the "/home/ming/data/Project/ClionProject/ESP32/camera_test1/build" directory
 python -m esptool --chip esp32s3 -b 460800 --before default_reset --after hard_reset write_flash "@flash_args"
Executing action: flash
Serial port /dev/ttyUSB0
Connecting....
Detecting chip type... ESP32-S3
Running ninja in directory /home/ming/data/Project/ClionProject/ESP32/camera_test1/build
Executing "ninja flash"...
[1/5] cd /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/esp-idf/esptool_py && /home/ming/.espress...rtition_table/partition-table.bin /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/camera_test1.bin
camera_test1.bin binary size 0x5ab00 bytes. Smallest app partition is 0x100000 bytes. 0xa5500 bytes (65%) free.
[1/1] cd /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/bootloader/esp-idf/esptool_py && /home/mi...et 0x8000 bootloader 0x0 /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/bootloader/bootloader.bin
Bootloader binary size 0x5240 bytes. 0x2dc0 bytes (36%) free.
[4/5] cd /opt/esp-idf/components/esptool_py && /home/ming/data/Jetbrains/clion-2025.2.1/bin/cmake/linux/x64/bin/...g/data/Project/ClionProject/ESP32/camera_test1/build -P /opt/esp-idf/components/esptool_py/run_serial_tool.cmake
esptool.py --chip esp32s3 -p /dev/ttyUSB0 -b 460800 --before=default_reset --after=hard_reset write_flash --flash_mode dio --flash_freq 80m --flash_size 2MB 0x0 bootloader/bootloader.bin 0x10000 camera_test1.bin 0x8000 partition_table/partition-table.bin
esptool.py v4.10.dev2
Serial port /dev/ttyUSB0
Connecting....
Chip is ESP32-S3 (QFN56) (revision v0.2)
Features: WiFi, BLE, Embedded PSRAM 8MB (AP_3v3)
Crystal is 40MHz
MAC: 30:ed:a0:a8:46:c0
Uploading stub...
Running stub...
Stub running...
Changing baud rate to 460800
Changed.
Configuring flash size...
Flash will be erased from 0x00000000 to 0x00005fff...
Flash will be erased from 0x00010000 to 0x0006afff...
Flash will be erased from 0x00008000 to 0x00008fff...
SHA digest in image updated
Compressed 21056 bytes to 13389...
Writing at 0x00000000... (100 %)
Wrote 21056 bytes (13389 compressed) at 0x00000000 in 0.5 seconds (effective 372.7 kbit/s)...
Hash of data verified.
Compressed 371456 bytes to 200339...
Writing at 0x00068f16... (100 %)
Wrote 371456 bytes (200339 compressed) at 0x00010000 in 4.5 seconds (effective 658.9 kbit/s)...
Hash of data verified.
Compressed 3072 bytes to 103...
Writing at 0x00008000... (100 %)
Wrote 3072 bytes (103 compressed) at 0x00008000 in 0.0 seconds (effective 839.6 kbit/s)...
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
Executing action: monitor
Running idf_monitor in directory /home/ming/data/Project/ClionProject/ESP32/camera_test1
Executing "/home/ming/.espressif/python_env/idf5.5_py3.13_env/bin/python /opt/esp-idf/tools/idf_monitor.py -p /dev/ttyUSB0 -b 115200 --toolchain-prefix xtensa-esp32s3-elf- --target esp32s3 --revision 0 /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/camera_test1.elf /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/bootloader/bootloader.elf -m '/home/ming/.espressif/python_env/idf5.5_py3.13_env/bin/python' '/opt/esp-idf/tools/idf.py'"...
--- esp-idf-monitor 1.7.0 on /dev/ttyUSB0 115200
--- Quit: Ctrl+] | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0x1 (POWERON),boot:0xb (SPI_FAST_FLASH_BOOT)
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fce2820,len:0x158c
load:0x403c8700,len:0xd24
--- 0x403c8700: _stext at ??:?
load:0x403cb700,len:0x2f34
entry 0x403c8924
--- 0x403c8924: call_start_cpu0 at /opt/esp-idf/components/bootloader/subproject/main/bootloader_start.c:25
I (24) boot: ESP-IDF v5.5.1 2nd stage bootloader
I (25) boot: compile time Dec  6 2025 17:35:18
I (25) boot: Multicore bootloader
I (25) boot: chip revision: v0.2
I (28) boot: efuse block revision: v1.3
I (31) boot.esp32s3: Boot SPI Speed : 80MHz
I (35) boot.esp32s3: SPI Mode       : DIO
I (39) boot.esp32s3: SPI Flash Size : 2MB
I (43) boot: Enabling RNG early entropy source...
I (47) boot: Partition Table:
I (50) boot: ## Label            Usage          Type ST Offset   Length
I (56) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (63) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (69) boot:  2 factory          factory app      00 00 00010000 00100000
I (76) boot: End of partition table
I (81) esp_image: segment 0: paddr=00010020 vaddr=3c030020 size=11ec8h ( 73416) map
I (99) esp_image: segment 1: paddr=00021ef0 vaddr=3fc98000 size=0587ch ( 22652) load
I (104) esp_image: segment 2: paddr=00027774 vaddr=40374000 size=088a4h ( 34980) load
I (112) esp_image: segment 3: paddr=00030020 vaddr=42000020 size=2f3e8h (193512) map
I (147) esp_image: segment 4: paddr=0005f410 vaddr=4037c8a4 size=0b69ch ( 46748) load
I (157) esp_image: segment 5: paddr=0006aab4 vaddr=50000000 size=00020h (    32) load
I (166) boot: Loaded app from partition at offset 0x10000
I (166) boot: Disabling RNG early entropy source...
I (176) octal_psram: vendor id    : 0x0d (AP)
I (176) octal_psram: dev id       : 0x02 (generation 3)
I (176) octal_psram: density      : 0x03 (64 Mbit)
I (178) octal_psram: good-die     : 0x01 (Pass)
I (182) octal_psram: Latency      : 0x01 (Fixed)
I (187) octal_psram: VCC          : 0x01 (3V)
I (191) octal_psram: SRF          : 0x01 (Fast Refresh)
I (196) octal_psram: BurstType    : 0x01 (Hybrid Wrap)
I (201) octal_psram: BurstLen     : 0x01 (32 Byte)
I (205) octal_psram: Readlatency  : 0x02 (10 cycles@Fixed)
I (210) octal_psram: DriveStrength: 0x00 (1/1)
I (215) MSPI Timing: PSRAM timing tuning index: 4
I (219) esp_psram: Found 8MB PSRAM device
I (223) esp_psram: Speed: 80MHz
I (226) cpu_start: Multicore app
I (655) esp_psram: SPI SRAM memory test OK
I (664) cpu_start: Pro cpu start user code
I (669) cpu_start: cpu freq: 160000000 Hz
I (669) app_init: Application information:
I (669) app_init: Project name:     camera_test1
I (673) app_init: App version:      05dc451-dirty
I (677) app_init: Compile time:     Dec  6 2025 19:59:25
I (682) app_init: ELF file SHA256:  00a2f0f13...
I (687) app_init: ESP-IDF:          v5.5.1
I (691) efuse_init: Min chip rev:     v0.0
I (694) efuse_init: Max chip rev:     v0.99 
I (698) efuse_init: Chip rev:         v0.2
I (702) heap_init: Initializing. RAM available for dynamic allocation:
I (709) heap_init: At 3FC9E2E0 len 0004B430 (301 KiB): RAM
I (714) heap_init: At 3FCE9710 len 00005724 (21 KiB): RAM
I (719) heap_init: At 3FCF0000 len 00008000 (32 KiB): DRAM
I (724) heap_init: At 600FE000 len 00001FE8 (7 KiB): RTCRAM
I (730) esp_psram: Adding pool of 8192K of PSRAM memory to heap allocator
I (737) spi_flash: detected chip: generic
I (740) spi_flash: flash io: dio
W (743) spi_flash: Detected size(16384k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (755) sleep_gpio: Configure to isolate all GPIO pins in sleep state
I (761) sleep_gpio: Enable automatic switching of GPIO sleep configuration
I (768) main_task: Started on CPU0
I (788) esp_psram: Reserving pool of 32K of internal memory for DMA/internal allocations
I (7��I (798) uart: UART initialized successfully
Hello World from ESP32!
I (798) uart: Sent data: Hello World from ESP32!

I (798) lcd: Initializing I2C...
I (798) lcd: I2C initialized successfully
I (798) lcd: Initializing PCA9557...
I (808) lcd: PCA9557 initialized successfully
I (858) camera: Initializing camera...
I (958) s3 ll_cam: DMA Channel=0
I (958) cam_hal: cam init ok
I (968) ov7725: Mismatch PID=0xe8
I (968) ov7760: Mismatch PID=0xe8
I (968) gc032a: Mismatch PID=0xf302
I (968) camera: Camera PID=0x9b VER=0x00 MIDL=0x00 MIDH=0x00
I (968) camera: Detected GC0308 camera
I (968) camera: Detected camera at address=0x21
I (1238) cam_hal: PSRAM DMA mode disabled
I (1238) s3 ll_cam: node_size: 3840, nodes_per_line: 1, lines_per_node: 12
I (1238) s3 ll_cam: dma_half_buffer_min:  3840, dma_half_buffer:  7680, lines_per_half_buffer: 24, dma_buffer_size: 30720
I (1248) cam_hal: buffer_size: 30720, half_buffer_size: 7680, node_buffer_size: 3840, node_cnt: 8, total_cnt: 5
I (1258) cam_hal: Allocating 38400 Byte frame buffer in PSRAM
I (1258) cam_hal: cam config ok
I (1258) gc0308: subsample win:640x480, ratio:0.250000
I (1278) camera: Camera initialized successfully
I (1278) lcd: Initializing LCD...
I (1448) lcd: Setting LCD backlight: 100%
E (1448) ledc: ledc_set_duty(1063): LEDC is not initialized
E (1448) lcd: LEDC set duty failed: ESP_ERR_INVALID_STATE
E (1458) lcd: LCD backlight on failed
E (1458) main: LCD initialization failed
I (1458) main_task: Returned from app_main()
