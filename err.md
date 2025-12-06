erified.

Leaving...
Hard resetting via RTS pin...
Executing action: monitor
Running idf_monitor in directory /home/ming/data/Project/ClionProject/ESP32/camera_test1
Executing "/home/ming/.espressif/python_env/idf5.5_py3.13_env/bin/python /opt/esp-idf/tools/idf_monitor.py -p /dev/ttyUSB0 -b 115200 --toolchain-prefix xtensa-esp32s3-elf- --target esp32s3 --revision 0 /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/camera_test1.elf /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/bootloader/bootloader.elf -m '/home/ming/.espressif/python_env/idf5.5_py3.13_env/bin/python' '/opt/esp-idf/tools/idf.py'"...
--- esp-idf-monitor 1.7.0 on /dev/ttyUSB0 115200
--- Quit: Ctrl+] | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H
�������������ESP-ROM:esp32s3-20210327
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
I (79) esp_image: segment 0: paddr=00010020 vaddr=3c040020 size=12be0h ( 76768) map
I (100) esp_image: segment 1: paddr=00022c08 vaddr=3fc96e00 size=053b8h ( 21432) load
I (105) esp_image: segment 2: paddr=00027fc8 vaddr=40374000 size=08050h ( 32848) load
I (112) esp_image: segment 3: paddr=00030020 vaddr=42000020 size=31440h (201792) map
I (148) esp_image: segment 4: paddr=00061468 vaddr=4037c050 size=0aca4h ( 44196) load
I (163) esp_image: segment 5: paddr=0006c114 vaddr=50000000 size=00020h (    32) load
I (171) boot: Loaded app from partition at offset 0x10000
I (171) boot: Disabling RNG early entropy source...
I (181) cpu_start: Multicore app
I (190) cpu_start: Pro cpu start user code
I (190) cpu_start: cpu freq: 160000000 Hz
I (190) app_init: Application information:
I (190) app_init: Project name:     camera_test1
I (195) app_init: App version:      f3147d7-dirty
I (199) app_init: Compile time:     Dec  6 2025 18:26:15
I (204) app_init: ELF file SHA256:  5919b5585...
I (208) app_init: ESP-IDF:          v5.5.1
I (212) efuse_init: Min chip rev:     v0.0
I (217) efuse_init: Max chip rev:     v0.99 
I (220) efuse_init: Chip rev:         v0.2
I (224) heap_init: Initializing. RAM available for dynamic allocation:
I (230) heap_init: At 3FC9D3A8 len 0004C368 (304 KiB): RAM
I (235) heap_init: At 3FCE9710 len 00005724 (21 KiB): RAM
I (241) heap_init: At 3FCF0000 len 00008000 (32 KiB): DRAM
I (246) heap_init: At 600FE000 len 00001FE8 (7 KiB): RTCRAM
I (252) spi_flash: detected chip: generic
I (255) spi_flash: flash io: dio
W (258) spi_flash: Detected size(16384k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (270) sleep_gpio: Configure to isolate all GPIO pins in sleep state
I (276) sleep_gpio: Enable automatic switching of GPIO sleep configuration
I (283) main_task: Started on CPU0
I (303) m��I (303) uart: UART initialized successfully
Hello World from ESP32!
I (303) uart: Sent data: Hello World from ESP32!

I (303) lcd: Initializing I2C...
I (303) lcd: I2C initialized successfully
I (313) camera: Initializing camera...
I (313) s3 ll_cam: DMA Channel=0
I (313) cam_hal: cam init ok
E (333) i2c.master: this port has not been initialized, please initialize it first
E (333) sccb-ng: failed to get SCCB I2C Bus handle for port 0
E (333) i2c.master: this port has not been initialized, please initialize it first
E (343) sccb-ng: failed to get SCCB I2C Bus handle for port 0
E (343) i2c.master: this port has not been initialized, please initialize it first
E (353) sccb-ng: failed to get SCCB I2C Bus handle for port 0
E (363) i2c.master: this port has not been initialized, please initialize it first
E (363) sccb-ng: failed to get SCCB I2C Bus handle for port 0
E (373) i2c.master: this port has not been initialized, please initialize it first
E (383) sccb-ng: failed to get SCCB I2C Bus handle for port 0
E (383) i2c.master: this port has not been initialized, please initialize it first
E (393) sccb-ng: failed to get SCCB I2C Bus handle for port 0
E (393) i2c.master: this port has not been initialized, please initialize it first
E (403) sccb-ng: failed to get SCCB I2C Bus handle for port 0
E (413) i2c.master: this port has not been initialized, please initialize it first
E (413) sccb-ng: failed to get SCCB I2C Bus handle for port 0
E (423) i2c.master: this port has not been initialized, please initialize it first
E (433) sccb-ng: failed to get SCCB I2C Bus handle for port 0
E (433) i2c.master: this port has not been initialized, please initialize it first
E (443) sccb-ng: failed to get SCCB I2C Bus handle for port 0
E (443) i2c.master: this port has not been initialized, please initialize it first
E (453) sccb-ng: failed to get SCCB I2C Bus handle for port 0
E (463) i2c.master: this port has not been initialized, please initialize it first
E (463) sccb-ng: failed to get SCCB I2C Bus handle for port 0
E (473) i2c.master: this port has not been initialized, please initialize it first
E (483) sccb-ng: failed to get SCCB I2C Bus handle for port 0
E (483) i2c.master: this port has not been initialized, please initialize it first
E (493) sccb-ng: failed to get SCCB I2C Bus handle for port 0
E (503) i2c.master: this port has not been initialized, please initialize it first
E (503) sccb-ng: failed to get SCCB I2C Bus handle for port 0
E (513) i2c.master: this port has not been initialized, please initialize it first
E (523) sccb-ng: failed to get SCCB I2C Bus handle for port 0
E (523) i2c.master: this port has not been initialized, please initialize it first
E (533) sccb-ng: failed to get SCCB I2C Bus handle for port 0
E (533) camera: Detected camera not supported.
E (543) camera: Camera probe failed with error 0x106(ESP_ERR_NOT_SUPPORTED)
E (543) camera: Camera init failed with error 0x106
E (553) main: Camera initialization failed
I (553) main_task: Returned from app_main()