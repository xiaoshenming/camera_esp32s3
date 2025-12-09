/ESP32/camera_test1
Executing "/home/ming/.espressif/python_env/idf5.5_py3.13_env/bin/python /opt/esp-idf/tools/idf_monitor.py -p /dev/ttyUSB0 -b 115200 --toolchain-prefix xtensa-esp32s3-elf- --target esp32s3 --revision 0 /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/camera_test1.elf /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/bootloader/bootloader.elf -m '/home/ming/.espressif/python_env/idf5.5_py3.13_env/bin/python' '/opt/esp-idf/tools/idf.py'"...
--- esp-idf-monitor 1.7.0 on /dev/ttyUSB0 115200
--- Quit: Ctrl+] | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H
�xxVH(
B���J�ESP-ROM:esp32s3-20210327
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
I (79) esp_image: segment 0: paddr=00010020 vaddr=3c0a0020 size=20518h (132376) map
I (110) esp_image: segment 1: paddr=00030540 vaddr=3fc9d500 size=0766ch ( 30316) load
I (116) esp_image: segment 2: paddr=00037bb4 vaddr=40374000 size=08464h ( 33892) load
I (124) esp_image: segment 3: paddr=00040020 vaddr=42000020 size=9a19ch (631196) map
I (235) esp_image: segment 4: paddr=000da1c4 vaddr=4037c464 size=11038h ( 69688) load
I (256) esp_image: segment 5: paddr=000eb204 vaddr=50000000 size=00020h (    32) load
I (266) boot: Loaded app from partition at offset 0x10000
I (267) boot: Disabling RNG early entropy source...
I (277) octal_psram: vendor id    : 0x0d (AP)
I (277) octal_psram: dev id       : 0x02 (generation 3)
I (277) octal_psram: density      : 0x03 (64 Mbit)
I (279) octal_psram: good-die     : 0x01 (Pass)
I (283) octal_psram: Latency      : 0x01 (Fixed)
I (288) octal_psram: VCC          : 0x01 (3V)
I (292) octal_psram: SRF          : 0x01 (Fast Refresh)
I (297) octal_psram: BurstType    : 0x01 (Hybrid Wrap)
I (302) octal_psram: BurstLen     : 0x01 (32 Byte)
I (306) octal_psram: Readlatency  : 0x02 (10 cycles@Fixed)
I (311) octal_psram: DriveStrength: 0x00 (1/1)
I (316) MSPI Timing: PSRAM timing tuning index: 4
I (320) esp_psram: Found 8MB PSRAM device
I (324) esp_psram: Speed: 80MHz
I (327) cpu_start: Multicore app
I (756) esp_psram: SPI SRAM memory test OK
I (765) cpu_start: Pro cpu start user code
I (765) cpu_start: cpu freq: 160000000 Hz
I (765) app_init: Application information:
I (765) app_init: Project name:     camera_test1
I (769) app_init: App version:      10c0fbf-dirty
I (774) app_init: Compile time:     Dec  9 2025 13:01:20
I (779) app_init: ELF file SHA256:  7a26c331c...
I (783) app_init: ESP-IDF:          v5.5.1
I (787) efuse_init: Min chip rev:     v0.0
I (791) efuse_init: Max chip rev:     v0.99 
I (795) efuse_init: Chip rev:         v0.2
I (798) heap_init: Initializing. RAM available for dynamic allocation:
I (805) heap_init: At 3FCA5828 len 00043EE8 (271 KiB): RAM
I (810) heap_init: At 3FCE9710 len 00005724 (21 KiB): RAM
I (815) heap_init: At 3FCF0000 len 00008000 (32 KiB): DRAM
I (820) heap_init: At 600FE000 len 00001FE8 (7 KiB): RTCRAM
I (826) esp_psram: Adding pool of 8179K of PSRAM memory to heap allocator
I (833) spi_flash: detected chip: generic
I (836) spi_flash: flash io: dio
W (839) spi_flash: Detected size(16384k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (851) sleep_gpio: Configure to isolate all GPIO pins in sleep state
I (857) sleep_gpio: Enable automatic switching of GPIO sleep configuration
I (864) main_task: Started on CPU0
I (884) esp_psram: Reserving pool of 32K of internal memory for DMA/iJ�I (894) uart: UART initialized successfully
Hello World from ESP32!
I (894) uart: Sent data: Hello World from ESP32!

I (894) lcd: Initializing I2C...
I (894) lcd: I2C initialized successfully
I (894) lcd: Initializing PCA9557...
I (904) lcd: PCA9557 initialized successfully
I (954) camera: Initializing camera...
I (1054) s3 ll_cam: DMA Channel=0
I (1054) cam_hal: cam init ok
I (1064) ov7725: Mismatch PID=0xe8
I (1064) ov7760: Mismatch PID=0xe8
I (1064) gc032a: Mismatch PID=0xf302
I (1064) camera: Camera PID=0x9b VER=0x00 MIDL=0x00 MIDH=0x00
I (1064) camera: Detected GC0308 camera
I (1064) camera: Detected camera at address=0x21
I (1284) cam_hal: PSRAM DMA mode disabled
I (1284) s3 ll_cam: node_size: 3840, nodes_per_line: 1, lines_per_node: 6
I (1284) s3 ll_cam: dma_half_buffer_min:  3840, dma_half_buffer: 15360, lines_per_half_buffer: 24, dma_buffer_size: 30720
I (1294) cam_hal: buffer_size: 30720, half_buffer_size: 15360, node_buffer_size: 3840, node_cnt: 8, total_cnt: 10
I (1304) cam_hal: Allocating 153600 Byte frame buffer in PSRAM
I (1304) cam_hal: Allocating 153600 Byte frame buffer in PSRAM
I (1314) cam_hal: cam config ok
I (1314) gc0308: subsample win:640x480, ratio:0.500000
I (1324) camera: Camera initialized successfully
I (1324) wifi: Initializing WiFi in STA mode...
I (1374) pp: pp rom version: e7ae62f
I (1374) net80211: net80211 rom version: e7ae62f
I (1384) wifi:wifi driver task: 3fcbb75c, prio:23, stack:6656, core=0
I (1394) wifi:wifi firmware version: 14da9b7
I (1394) wifi:wifi certification version: v7.0
I (1394) wifi:config NVS flash: enabled
I (1394) wifi:config nano formatting: disabled
I (1404) wifi:Init data frame dynamic rx buffer num: 32
I (1404) wifi:Init static rx mgmt buffer num: 5
I (1414) wifi:Init management short buffer num: 32
I (1414) wifi:Init dynamic tx buffer num: 32
I (1414) wifi:Init static tx FG buffer num: 2
I (1424) wifi:Init static rx buffer size: 1600
I (1424) wifi:Init static rx buffer num: 10
I (1434) wifi:Init dynamic rx buffer num: 32
I (1434) wifi_init: rx ba win: 6
I (1434) wifi_init: accept mbox: 6
I (1444) wifi_init: tcpip mbox: 32
I (1444) wifi_init: udp mbox: 6
I (1444) wifi_init: tcp mbox: 6
I (1454) wifi_init: tcp tx win: 5760
I (1454) wifi_init: tcp rx win: 5760
I (1454) wifi_init: tcp mss: 1440
I (1454) wifi_init: WiFi IRAM OP enabled
I (1464) wifi_init: WiFi RX IRAM OP enabled
I (1464) phy_init: phy_version 701,f4f1da3a,Mar  3 2025,15:50:10
I (1504) wifi:mode : sta (30:ed:a0:a8:46:c0)
I (1504) wifi:enable tsf
I (1514) wifi: WiFi initialization completed
I (1514) wifi: WiFi started, connecting to AP...
I (1514) main: Waiting for WiFi connection...
I (1594) wifi:new:<6,2>, old:<1,0>, ap:<255,255>, sta:<6,2>, prof:1, snd_ch_cfg:0x0
I (1594) wifi:state: init -> auth (0xb0)
I (1854) wifi:state: auth -> assoc (0x0)
I (2014) main: Waiting for WiFi connection... 1/20
I (2364) wifi:state: assoc -> run (0x10)
I (2514) main: Waiting for WiFi connection... 2/20
I (3014) main: Waiting for WiFi connection... 3/20
I (3514) main: Waiting for WiFi connection... 4/20
I (3514) wifi:<ba-add>idx:0 (ifx:0, 80:ea:07:a0:c3:88), tid:0, ssn:1, winSize:64
I (3614) wifi:connected with 309Study, aid = 1, channel 6, 40D, bssid = 80:ea:07:a0:c3:88
I (3614) wifi:security: WPA2-PSK, phy: bgn, rssi: -55
I (3624) wifi:pm start, type: 1

I (3624) wifi:dp: 1, bi: 102400, li: 3, scale listen interval from 307200 us to 307200 us
I (3624) wifi:set rx beacon pti, rx_bcn_pti: 0, bcn_timeout: 25000, mt_pti: 0, mt_time: 10000
I (3634) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (4014) main: Waiting for WiFi connection... 5/20
I (4514) main: Waiting for WiFi connection... 6/20
I (4764) esp_netif_handlers: sta ip: 192.168.1.100, mask: 255.255.255.0, gw: 192.168.1.1
I (4764) wifi: Got IP address: 192.168.1.100
I (5014) main: Waiting for WiFi connection... 7/20
I (5014) main: WiFi connected successfully!
I (5014) camera: Camera config updated
I (5014) camera: Starting camera with config: LCD=0, FPS=1, Capturer=0
I (5014) camera: Camera started successfully
I (5014) camera: FPS monitor task started
I (5024) main: Starting FPV mode...
I (5024) camera: Starting FPV mode...
W (5034) wifi: Failed to set send buffer size: Protocol not available
I (5034) wifi: UDP broadcast initialized on port 8888
I (5044) camera: FPV server started on IP: 192.168.1.100, Port: 8888
I (5044) camera: FPV mode started successfully
I (5044) camera: FPV transmission task started
I (5054) main: FPV Camera system started successfully!
I (5064) main: Current config: LCD=0, FPS=1, Capture=0, Clock=50000000
I (5064) main: WiFi Info - SSID: 309Study, IP: 192.168.1.100, Channel: 6
W (5074) wifi: UDP send failed: Not enough space
W (5074) wifi: Failed to send packet 33/152 for frame 0
W (5084) camera: Failed to send frame 0
W (5084) wifi: UDP send failed: Not enough space
W (5094) wifi: Failed to send packet 1/152 for frame 0
W (5094) camera: Failed to send frame 0
W (5124) wifi: UDP send failed: Not enough space
W (5124) wifi: Failed to send packet 3/152 for frame 0
W (5124) camera: Failed to send frame 0
W (5154) wifi: UDP send failed: Not enough space
W (5154) wifi: Failed to send packet 2/152 for frame 0
W (5154) camera: Failed to send frame 0
W (5174) wifi: UDP send failed: Not enough space
W (5174) wifi: Failed to send packet 5/152 for frame 0
W (5174) camera: Failed to send frame 0
W (5204) wifi: UDP send failed: Not enough space
W (5204) wifi: Failed to send packet 7/152 for frame 0
W (5204) camera: Failed to send frame 0
W (5224) wifi: UDP send failed: Not enough space
W (5224) wifi: Failed to send packet 4/152 for frame 0
W (5224) camera: Failed to send frame 0
W (5254) wifi: UDP send failed: Not enough space
W (5254) wifi: Failed to send packet 7/152 for frame 0
W (5254) camera: Failed to send frame 0
W (5274) wifi: UDP send failed: Not enough space
W (5274) wifi: Failed to send packet 4/152 for frame 0
W (5274) camera: Failed to send frame 0
W (5304) wifi: UDP send failed: Not enough space
W (5304) wifi: Failed to send packet 6/152 for frame 0
W (5304) camera: Failed to send frame 0
W (5324) wifi: UDP send failed: Not enough space
W (5324) wifi: Failed to send packet 2/152 for frame 0
W (5324) camera: Failed to send frame 0
W (5354) wifi: UDP send failed: Not enough space
W (5354) wifi: Failed to send packet 4/152 for frame 0
W (5354) camera: Failed to send frame 0
W (5374) wifi: UDP send failed: Not enough space
W (5374) wifi: Failed to send packet 4/152 for frame 0
W (5374) camera: Failed to send frame 0
W (5404) wifi: UDP send failed: Not enough space
W (5404) wifi: Failed to send packet 8/152 for frame 0
W (5404) camera: Failed to send frame 0
W (5424) wifi: UDP send failed: Not enough space
W (5424) wifi: Failed to send packet 4/152 for frame 0
W (5424) camera: Failed to send frame 0
W (5454) wifi: UDP send failed: Not enough space
W (5454) wifi: Failed to send packet 4/152 for frame 0
W (5454) camera: Failed to send frame 0
W (5474) wifi: UDP send failed: Not enough space
W (5474) wifi: Failed to send packet 4/152 for frame 0
W (5474) camera: Failed to send frame 0
W (5504) wifi: UDP send failed: Not enough space
W (5504) wifi: Failed to send packet 6/152 for frame 0
W (5504) camera: Failed to send frame 0
W (5524) wifi: UDP send failed: Not enough space
W (5524) wifi: Failed to send packet 3/152 for frame 0
W (5524) camera: Failed to send frame 0
W (5554) wifi: UDP send failed: Not enough space
W (5554) wifi: Failed to send packet 6/152 for frame 0
W (5554) camera: Failed to send frame 0
W (5574) wifi: UDP send failed: Not enough space
W (5574) wifi: Failed to send packet 4/152 for frame 0
W (5574) camera: Failed to send frame 0
W (5604) wifi: UDP send failed: Not enough space
W (5604) wifi: Failed to send packet 4/152 for frame 0
W (5604) camera: Failed to send frame 0
W (5624) wifi: UDP send failed: Not enough space
W (5624) wifi: Failed to send packet 2/152 for frame 0
W (5624) camera: Failed to send frame 0
W (5654) wifi: UDP send failed: Not enough space
W (5654) wifi: Failed to send packet 4/152 for frame 0
W (5654) camera: Failed to send frame 0
W (5674) wifi: UDP send failed: Not enough space
W (5674) wifi: Failed to send packet 5/152 for frame 0
W (5674) camera: Failed to send frame 0
W (5704) wifi: UDP send failed: Not enough space
W (5704) wifi: Failed to send packet 6/152 for frame 0
W (5704) camera: Failed to send frame 0
W (5724) wifi: UDP send failed: Not enough space
W (5724) wifi: Failed to send packet 5/152 for frame 0
W (5724) camera: Failed to send frame 0
W (5754) wifi: UDP send failed: Not enough space
W (5754) wifi: Failed to send packet 1/152 for frame 0
W (5754) camera: Failed to send frame 0
W (5774) wifi: UDP send failed: Not enough space
W (5774) wifi: Failed to send packet 6/152 for frame 0
W (5774) camera: Failed to send frame 0
W (5804) wifi: UDP send failed: Not enough space
W (5804) wifi: Failed to send packet 5/152 for frame 0
W (5804) camera: Failed to send frame 0
W (5824) wifi: UDP send failed: Not enough space
W (5824) wifi: Failed to send packet 4/152 for frame 0
W (5824) camera: Failed to send frame 0
W (5854) wifi: UDP send failed: Not enough space
W (5854) wifi: Failed to send packet 1/152 for frame 0
W (5854) camera: Failed to send frame 0
W (5874) wifi: UDP send failed: Not enough space
W (5874) wifi: Failed to send packet 2/152 for frame 0
W (5874) camera: Failed to send frame 0
W (5904) wifi: UDP send failed: Not enough space
W (5904) wifi: Failed to send packet 6/152 for frame 0
W (5904) camera: Failed to send frame 0
W (5924) wifi: UDP send failed: Not enough space
W (5924) wifi: Failed to send packet 4/152 for frame 0
W (5924) camera: Failed to send frame 0
W (5954) wifi: UDP send failed: Not enough space
W (5954) wifi: Failed to send packet 2/152 for frame 0
W (5954) camera: Failed to send frame 0
W (5974) wifi: UDP send failed: Not enough space
W (5974) wifi: Failed to send packet 3/152 for frame 0
W (5974) camera: Failed to send frame 0
W (6004) wifi: UDP send failed: Not enough space
W (6004) wifi: Failed to send packet 3/152 for frame 0
W (6004) camera: Failed to send frame 0
I (6024) camera: Camera FPS: 0.0, LCD FPS: 0.0
W (6024) wifi: UDP send failed: Not enough space
W (6024) wifi: Failed to send packet 7/152 for frame 0
W (6024) camera: Failed to send frame 0
W (6054) wifi: UDP send failed: Not enough space
W (6054) wifi: Failed to send packet 2/152 for frame 0
W (6054) camera: Failed to send frame 0
W (6074) wifi: UDP send failed: Not enough space
W (6074) wifi: Failed to send packet 3/152 for frame 0
W (6074) camera: Failed to send frame 0
W (6104) wifi: UDP send failed: Not enough space
W (6104) wifi: Failed to send packet 2/152 for frame 0
W (6104) camera: Failed to send frame 0
W (6124) wifi: UDP send failed: Not enough space
W (6124) wifi: Failed to send packet 3/152 for frame 0
W (6124) camera: Failed to send frame 0
W (6154) wifi: UDP send failed: Not enough space
W (6154) wifi: Failed to send packet 3/152 for frame 0
W (6154) camera: Failed to send frame 0
W (6174) wifi: UDP send failed: Not enough space
W (6174) wifi: Failed to send packet 4/152 for frame 0
W (6174) camera: Failed to send frame 0
W (6204) wifi: UDP send failed: Not enough space
W (6204) wifi: Failed to send packet 7/152 for frame 0
W (6204) camera: Failed to send frame 0
W (6224) wifi: UDP send failed: Not enough space
W (6224) wifi: Failed to send packet 3/152 for frame 0
W (6224) camera: Failed to send frame 0
W (6254) wifi: UDP send failed: Not enough space
W (6254) wifi: Failed to send packet 1/152 for frame 0
W (6254) camera: Failed to send frame 0
W (6274) wifi: UDP send failed: Not enough space
W (6274) wifi: Failed to send packet 4/152 for frame 0
W (6274) camera: Failed to send frame 0
W (6304) wifi: UDP send failed: Not enough space
W (6304) wifi: Failed to send packet 6/152 for frame 0
W (6304) camera: Failed to send frame 0
W (6324) wifi: UDP send failed: Not enough space
W (6324) wifi: Failed to send packet 5/152 for frame 0
W (6324) camera: Failed to send frame 0
W (6354) wifi: UDP send failed: Not enough space
W (6354) wifi: Failed to send packet 4/152 for frame 0
W (6354) camera: Failed to send frame 0
W (6374) wifi: UDP send failed: Not enough space
W (6374) wifi: Failed to send packet 3/152 for frame 0
W (6374) camera: Failed to send frame 0
W (6404) wifi: UDP send failed: Not enough space
W (6404) wifi: Failed to send packet 5/152 for frame 0
W (6404) camera: Failed to send frame 0
W (6424) wifi: UDP send failed: Not enough space
W (6424) wifi: Failed to send packet 3/152 for frame 0
W (6424) camera: Failed to send frame 0
W (6454) wifi: UDP send failed: Not enough space
W (6454) wifi: Failed to send packet 2/152 for frame 0
W (6454) camera: Failed to send frame 0
W (6474) wifi: UDP send failed: Not enough space
W (6474) wifi: Failed to send packet 4/152 for frame 0
W (6474) camera: Failed to send frame 0
W (6504) wifi: UDP send failed: Not enough space
W (6504) wifi: Failed to send packet 5/152 for frame 0
W (6504) camera: Failed to send frame 0
W (6524) wifi: UDP send failed: Not enough space
W (6524) wifi: Failed to send packet 7/152 for frame 0
W (6534) camera: Failed to send frame 0
W (6554) wifi: UDP send failed: Not enough space
W (6554) wifi: Failed to send packet 3/152 for frame 0
W (6554) camera: Failed to send frame 0
W (6574) wifi: UDP send failed: Not enough space
W (6574) wifi: Failed to send packet 4/152 for frame 0
W (6574) camera: Failed to send frame 0
W (6604) wifi: UDP send failed: Not enough space
W (6604) wifi: Failed to send packet 2/152 for frame 0
W (6604) camera: Failed to send frame 0
W (6624) wifi: UDP send failed: Not enough space
W (6624) wifi: Failed to send packet 7/152 for frame 0
W (6624) camera: Failed to send frame 0
W (6654) wifi: UDP send failed: Not enough space
W (6654) wifi: Failed to send packet 6/152 for frame 0
W (6654) camera: Failed to send frame 0
W (6674) wifi: UDP send failed: Not enough space
W (6674) wifi: Failed to send packet 3/152 for frame 0
W (6674) camera: Failed to send frame 0
W (6704) wifi: UDP send failed: Not enough space
W (6704) wifi: Failed to send packet 7/152 for frame 0
W (6704) camera: Failed to send frame 0
W (6724) wifi: UDP send failed: Not enough space
W (6724) wifi: Failed to send packet 3/152 for frame 0
W (6724) camera: Failed to send frame 0
W (6754) wifi: UDP send failed: Not enough space
W (6754) wifi: Failed to send packet 9/152 for frame 0
W (6754) camera: Failed to send frame 0
W (6774) wifi: UDP send failed: Not enough space
W (6774) wifi: Failed to send packet 3/152 for frame 0
W (6774) camera: Failed to send frame 0
W (6804) wifi: UDP send failed: Not enough space
W (6804) wifi: Failed to send packet 2/152 for frame 0
W (6804) camera: Failed to send frame 0
W (6824) wifi: UDP send failed: Not enough space
W (6824) wifi: Failed to send packet 6/152 for frame 0
W (6824) camera: Failed to send frame 0
W (6854) wifi: UDP send failed: Not enough space
W (6854) wifi: Failed to send packet 5/152 for frame 0
W (6854) camera: Failed to send frame 0
W (6874) wifi: UDP send failed: Not enough space
W (6874) wifi: Failed to send packet 1/152 for frame 0
W (6874) camera: Failed to send frame 0
W (6904) wifi: UDP send failed: Not enough space
W (6904) wifi: Failed to send packet 4/152 for frame 0
W (6904) camera: Failed to send frame 0
W (6924) wifi: UDP send failed: Not enough space
W (6924) wifi: Failed to send packet 6/152 for frame 0
W (6924) camera: Failed to send frame 0
W (6954) wifi: UDP send failed: Not enough space
W (6954) wifi: Failed to send packet 5/152 for frame 0
W (6954) camera: Failed to send frame 0
W (6974) wifi: UDP send failed: Not enough space
W (6974) wifi: Failed to send packet 1/152 for frame 0
W (6974) camera: Failed to send frame 0
W (7004) wifi: UDP send failed: Not enough space
W (7004) wifi: Failed to send packet 3/152 for frame 0
W (7004) camera: Failed to send frame 0
I (7024) camera: Camera FPS: 0.0, LCD FPS: 0.0
W (7024) wifi: UDP send failed: Not enough space
W (7024) wifi: Failed to send packet 1/152 for frame 0
W (7024) camera: Failed to send frame 0
W (7054) wifi: UDP send failed: Not enough space
W (7054) wifi: Failed to send packet 7/152 for frame 0
W (7054) camera: Failed to send frame 0
W (7074) wifi: UDP send failed: Not enough space
W (7074) wifi: Failed to send packet 3/152 for frame 0
W (7074) camera: Failed to send frame 0
W (7104) wifi: UDP send failed: Not enough space
W (7104) wifi: Failed to send packet 4/152 for frame 0
W (7104) camera: Failed to send frame 0
W (7124) wifi: UDP send failed: Not enough space
W (7124) wifi: Failed to send packet 4/152 for frame 0
W (7124) camera: Failed to send frame 0
W (7154) wifi: UDP send failed: Not enough space
W (7154) wifi: Failed to send packet 8/152 for frame 0
W (7154) camera: Failed to send frame 0
W (7174) wifi: UDP send failed: Not enough space
W (7174) wifi: Failed to send packet 1/152 for frame 0
W (7174) camera: Failed to send frame 0
W (7204) wifi: UDP send failed: Not enough space
W (7204) wifi: Failed to send packet 4/152 for frame 0
W (7204) camera: Failed to send frame 0
W (7224) wifi: UDP send failed: Not enough space
W (7224) wifi: Failed to send packet 6/152 for frame 0
W (7224) camera: Failed to send frame 0
W (7254) wifi: UDP send failed: Not enough space
W (7254) wifi: Failed to send packet 6/152 for frame 0
W (7254) camera: Failed to send frame 0
W (7274) wifi: UDP send failed: Not enough space
W (7274) wifi: Failed to send packet 2/152 for frame 0
W (7274) camera: Failed to send frame 0
W (7304) wifi: UDP send failed: Not enough space
W (7304) wifi: Failed to send packet 4/152 for frame 0
W (7304) camera: Failed to send frame 0
W (7324) wifi: UDP send failed: Not enough space
W (7324) wifi: Failed to send packet 2/152 for frame 0
W (7324) camera: Failed to send frame 0
W (7354) wifi: UDP send failed: Not enough space
W (7354) wifi: Failed to send packet 2/152 for frame 0
W (7354) camera: Failed to send frame 0
W (7374) wifi: UDP send failed: Not enough space
W (7374) wifi: Failed to send packet 2/152 for frame 0
W (7374) camera: Failed to send frame 0
W (7404) wifi: UDP send failed: Not enough space
W (7404) wifi: Failed to send packet 3/152 for frame 0
W (7404) camera: Failed to send frame 0
W (7424) wifi: UDP send failed: Not enough space
W (7424) wifi: Failed to send packet 4/152 for frame 0
W (7424) camera: Failed to send frame 0
W (7454) wifi: UDP send failed: Not enough space
W (7454) wifi: Failed to send packet 4/152 for frame 0
W (7454) camera: Failed to send frame 0
W (7474) wifi: UDP send failed: Not enough space
W (7474) wifi: Failed to send packet 4/152 for frame 0
W (7474) camera: Failed to send frame 0
W (7504) wifi: UDP send failed: Not enough space
W (7504) wifi: Failed to send packet 3/152 for frame 0
W (7504) camera: Failed to send frame 0
W (7524) wifi: UDP send failed: Not enough space
W (7524) wifi: Failed to send packet 4/152 for frame 0
W (7524) camera: Failed to send frame 0
W (7554) wifi: UDP send failed: Not enough space
W (7554) wifi: Failed to send packet 1/152 for frame 0
W (7554) camera: Failed to send frame 0
W (7574) wifi: UDP send failed: Not enough space
W (7574) wifi: Failed to send packet 1/152 for frame 0
W (7574) camera: Failed to send frame 0
W (7604) wifi: UDP send failed: Not enough space
W (7604) wifi: Failed to send packet 2/152 for frame 0
W (7604) camera: Failed to send frame 0
W (7624) wifi: UDP send failed: Not enough space
W (7624) wifi: Failed to send packet 3/152 for frame 0
W (7624) camera: Failed to send frame 0
W (7654) wifi: UDP send failed: Not enough space
W (7654) wifi: Failed to send packet 6/152 for frame 0
W (7654) camera: Failed to send frame 0
W (7674) wifi: UDP send failed: Not enough space
W (7674) wifi: Failed to send packet 4/152 for frame 0
W (7674) camera: Failed to send frame 0
W (7704) wifi: UDP send failed: Not enough space
W (7704) wifi: Failed to send packet 2/152 for frame 0
W (7704) camera: Failed to send frame 0
W (7724) wifi: UDP send failed: Not enough space
W (7724) wifi: Failed to send packet 2/152 for frame 0
W (7724) camera: Failed to send frame 0
W (7754) wifi: UDP send failed: Not enough space
W (7754) wifi: Failed to send packet 6/152 for frame 0
W (7754) camera: Failed to send frame 0
W (7774) wifi: UDP send failed: Not enough space
W (7774) wifi: Failed to send packet 3/152 for frame 0
W (7774) camera: Failed to send frame 0
W (7804) wifi: UDP send failed: Not enough space
W (7804) wifi: Failed to send packet 4/152 for frame 0
W (7804) camera: Failed to send frame 0
W (7824) wifi: UDP send failed: Not enough space
W (7824) wifi: Failed to send packet 5/152 for frame 0
W (7824) camera: Failed to send frame 0
W (7854) wifi: UDP send failed: Not enough space
W (7854) wifi: Failed to send packet 5/152 for frame 0
W (7854) camera: Failed to send frame 0
W (7874) wifi: UDP send failed: Not enough space
W (7874) wifi: Failed to send packet 3/152 for frame 0
W (7874) camera: Failed to send frame 0
W (7904) wifi: UDP send failed: Not enough space
W (7904) wifi: Failed to send packet 2/152 for frame 0
W (7904) camera: Failed to send frame 0
W (7924) wifi: UDP send failed: Not enough space
W (7924) wifi: Failed to send packet 6/152 for frame 0
W (7924) camera: Failed to send frame 0
W (7954) wifi: UDP send failed: Not enough space
W (7954) wifi: Failed to send packet 4/152 for frame 0
W (7954) camera: Failed to send frame 0
W (7974) wifi: UDP send failed: Not enough space
W (7974) wifi: Failed to send packet 4/152 for frame 0
W (7974) camera: Failed to send frame 0
W (8004) wifi: UDP send failed: Not enough space
W (8004) wifi: Failed to send packet 2/152 for frame 0
W (8004) camera: Failed to send frame 0
I (8024) camera: Camera FPS: 0.0, LCD FPS: 0.0
W (8024) wifi: UDP send failed: Not enough space
W (8024) wifi: Failed to send packet 3/152 for frame 0
W (8024) camera: Failed to send frame 0
W (8054) wifi: UDP send failed: Not enough space
W (8054) wifi: Failed to send packet 4/152 for frame 0
W (8054) camera: Failed to send frame 0
W (8074) wifi: UDP send failed: Not enough space
W (8074) wifi: Failed to send packet 5/152 for frame 0
W (8074) camera: Failed to send frame 0
W (8104) wifi: UDP send failed: Not enough space
W (8104) wifi: Failed to send packet 2/152 for frame 0
W (8104) camera: Failed to send frame 0
W (8124) wifi: UDP send failed: Not enough space
W (8124) wifi: Failed to send packet 6/152 for frame 0
W (8124) camera: Failed to send frame 0
W (8154) wifi: UDP send failed: Not enough space
W (8154) wifi: Failed to send packet 6/152 for frame 0
W (8154) camera: Failed to send frame 0
W (8174) wifi: UDP send failed: Not enough space
W (8174) wifi: Failed to send packet 3/152 for frame 0
W (8174) camera: Failed to send frame 0
W (8204) wifi: UDP send failed: Not enough space
W (8204) wifi: Failed to send packet 2/152 for frame 0
W (8204) camera: Failed to send frame 0
W (8224) wifi: UDP send failed: Not enough space
W (8224) wifi: Failed to send packet 4/152 for frame 0
W (8224) camera: Failed to send frame 0
W (8254) wifi: UDP send failed: Not enough space
W (8254) wifi: Failed to send packet 3/152 for frame 0
W (8254) camera: Failed to send frame 0
W (8274) wifi: UDP send failed: Not enough space
W (8274) wifi: Failed to send packet 5/152 for frame 0
W (8274) camera: Failed to send frame 0
W (8304) wifi: UDP send failed: Not enough space
W (8304) wifi: Failed to send packet 2/152 for frame 0
W (8304) camera: Failed to send frame 0
W (8324) wifi: UDP send failed: Not enough space
W (8324) wifi: Failed to send packet 5/152 for frame 0
W (8324) camera: Failed to send frame 0
W (8354) wifi: UDP send failed: Not enough space
W (8354) wifi: Failed to send packet 3/152 for frame 0
W (8354) camera: Failed to send frame 0
W (8374) wifi: UDP send failed: Not enough space
W (8374) wifi: Failed to send packet 3/152 for frame 0
W (8374) camera: Failed to send frame 0
W (8404) wifi: UDP send failed: Not enough space
W (8404) wifi: Failed to send packet 1/152 for frame 0
W (8404) camera: Failed to send frame 0
W (8424) wifi: UDP send failed: Not enough space
W (8424) wifi: Failed to send packet 5/152 for frame 0
W (8424) camera: Failed to send frame 0
W (8454) wifi: UDP send failed: Not enough space
W (8454) wifi: Failed to send packet 4/152 for frame 0
W (8454) camera: Failed to send frame 0
W (8474) wifi: UDP send failed: Not enough space
W (8474) wifi: Failed to send packet 5/152 for frame 0
W (8474) camera: Failed to send frame 0
W (8504) wifi: UDP send failed: Not enough space
W (8504) wifi: Failed to send packet 2/152 for frame 0
W (8504) camera: Failed to send frame 0
W (8524) wifi: UDP send failed: Not enough space
W (8524) wifi: Failed to send packet 2/152 for frame 0
W (8524) camera: Failed to send frame 0
W (8554) wifi: UDP send failed: Not enough space
W (8554) wifi: Failed to send packet 3/152 for frame 0
W (8554) camera: Failed to send frame 0
W (8574) wifi: UDP send failed: Not enough space
W (8574) wifi: Failed to send packet 5/152 for frame 0
W (8574) camera: Failed to send frame 0
W (8604) wifi: UDP send failed: Not enough space
W (8604) wifi: Failed to send packet 3/152 for frame 0
W (8604) camera: Failed to send frame 0
W (8624) wifi: UDP send failed: Not enough space
W (8624) wifi: Failed to send packet 4/152 for frame 0
W (8624) camera: Failed to send frame 0
W (8654) wifi: UDP send failed: Not enough space
W (8654) wifi: Failed to send packet 5/152 for frame 0
W (8654) camera: Failed to send frame 0
W (8674) wifi: UDP send failed: Not enough space
W (8674) wifi: Failed to send packet 3/152 for frame 0
W (8674) camera: Failed to send frame 0
W (8704) wifi: UDP send failed: Not enough space
W (8704) wifi: Failed to send packet 5/152 for frame 0
W (8704) camera: Failed to send frame 0
W (8724) wifi: UDP send failed: Not enough space
W (8724) wifi: Failed to send packet 4/152 for frame 0
W (8724) camera: Failed to send frame 0
W (8754) wifi: UDP send failed: Not enough space
W (8754) wifi: Failed to send packet 4/152 for frame 0
W (8754) camera: Failed to send frame 0
W (8774) wifi: UDP send failed: Not enough space
W (8774) wifi: Failed to send packet 4/152 for frame 0
W (8774) camera: Failed to send frame 0
W (8804) wifi: UDP send failed: Not enough space
W (8804) wifi: Failed to send packet 5/152 for frame 0
W (8804) camera: Failed to send frame 0
W (8824) wifi: UDP send failed: Not enough space
W (8824) wifi: Failed to send packet 2/152 for frame 0
W (8824) camera: Failed to send frame 0
W (8854) wifi: UDP send failed: Not enough space
W (8854) wifi: Failed to send packet 5/152 for frame 0
W (8854) camera: Failed to send frame 0
W (8874) wifi: UDP send failed: Not enough space
W (8874) wifi: Failed to send packet 3/152 for frame 0
W (8874) camera: Failed to send frame 0
W (8904) wifi: UDP send failed: Not enough space
W (8904) wifi: Failed to send packet 5/152 for frame 0
W (8904) camera: Failed to send frame 0
W (8924) wifi: UDP send failed: Not enough space
W (8924) wifi: Failed to send packet 4/152 for frame 0
W (8924) camera: Failed to send frame 0
W (8954) wifi: UDP send failed: Not enough space
W (8954) wifi: Failed to send packet 2/152 for frame 0
W (8954) camera: Failed to send frame 0
W (8974) wifi: UDP send failed: Not enough space
W (8974) wifi: Failed to send packet 2/152 for frame 0
W (8974) camera: Failed to send frame 0
W (9004) wifi: UDP send failed: Not enough space
W (9004) wifi: Failed to send packet 3/152 for frame 0
W (9004) camera: Failed to send frame 0
I (9024) camera: Camera FPS: 0.0, LCD FPS: 0.0
W (9024) wifi: UDP send failed: Not enough space
W (9024) wifi: Failed to send packet 3/152 for frame 0
W (9024) camera: Failed to send frame 0
W (9054) wifi: UDP send failed: Not enough space
W (9054) wifi: Failed to send packet 4/152 for frame 0
W (9054) camera: Failed to send frame 0
W (9074) wifi: UDP send failed: Not enough space
W (9074) wifi: Failed to send packet 3/152 for frame 0
W (9074) camera: Failed to send frame 0
W (9104) wifi: UDP send failed: Not enough space
W (9104) wifi: Failed to send packet 3/152 for frame 0
W (9104) camera: Failed to send frame 0
W (9124) wifi: UDP send failed: Not enough space
W (9124) wifi: Failed to send packet 3/152 for frame 0
W (9124) camera: Failed to send frame 0
W (9154) wifi: UDP send failed: Not enough space
W (9154) wifi: Failed to send packet 3/152 for frame 0
W (9154) camera: Failed to send frame 0
W (9174) wifi: UDP send failed: Not enough space
W (9174) wifi: Failed to send packet 5/152 for frame 0
W (9174) camera: Failed to send frame 0
W (9204) wifi: UDP send failed: Not enough space
W (9204) wifi: Failed to send packet 4/152 for frame 0
W (9204) camera: Failed to send frame 0
W (9224) wifi: UDP send failed: Not enough space
W (9224) wifi: Failed to send packet 3/152 for frame 0
W (9224) camera: Failed to send frame 0
W (9254) wifi: UDP send failed: Not enough space
W (9254) wifi: Failed to send packet 1/152 for frame 0
W (9254) camera: Failed to send frame 0
W (9274) wifi: UDP send failed: Not enough space
W (9274) wifi: Failed to send packet 3/152 for frame 0
W (9274) camera: Failed to send frame 0
W (9304) wifi: UDP send failed: Not enough space
W (9304) wifi: Failed to send packet 3/152 for frame 0
W (9304) camera: Failed to send frame 0
W (9324) wifi: UDP send failed: Not enough space
W (9324) wifi: Failed to send packet 2/152 for frame 0
W (9324) camera: Failed to send frame 0
W (9354) wifi: UDP send failed: Not enough space
W (9354) wifi: Failed to send packet 3/152 for frame 0
W (9354) camera: Failed to send frame 0
W (9374) wifi: UDP send failed: Not enough space
W (9374) wifi: Failed to send packet 4/152 for frame 0
W (9374) camera: Failed to send frame 0
W (9404) wifi: UDP send failed: Not enough space
W (9404) wifi: Failed to send packet 2/152 for frame 0
W (9404) camera: Failed to send frame 0
W (9424) wifi: UDP send failed: Not enough space
W (9424) wifi: Failed to send packet 2/152 for frame 0
W (9424) camera: Failed to send frame 0
W (9454) wifi: UDP send failed: Not enough space
W (9454) wifi: Failed to send packet 4/152 for frame 0
W (9454) camera: Failed to send frame 0
W (9474) wifi: UDP send failed: Not enough space
W (9474) wifi: Failed to send packet 3/152 for frame 0
W (9474) camera: Failed to send frame 0
W (9504) wifi: UDP send failed: Not enough space
W (9504) wifi: Failed to send packet 3/152 for frame 0
W (9504) camera: Failed to send frame 0
W (9524) wifi: UDP send failed: Not enough space
W (9524) wifi: Failed to send packet 5/152 for frame 0
W (9524) camera: Failed to send frame 0
W (9554) wifi: UDP send failed: Not enough space
W (9554) wifi: Failed to send packet 4/152 for frame 0
W (9554) camera: Failed to send frame 0
W (9574) wifi: UDP send failed: Not enough space
W (9574) wifi: Failed to send packet 6/152 for frame 0
W (9574) camera: Failed to send frame 0
W (9604) wifi: UDP send failed: Not enough space
W (9604) wifi: Failed to send packet 5/152 for frame 0
W (9604) camera: Failed to send frame 0
W (9624) wifi: UDP send failed: Not enough space
W (9624) wifi: Failed to send packet 7/152 for frame 0
W (9624) camera: Failed to send frame 0
W (9654) wifi: UDP send failed: Not enough space
W (9654) wifi: Failed to send packet 3/152 for frame 0
W (9654) camera: Failed to send frame 0
W (9674) wifi: UDP send failed: Not enough space
W (9674) wifi: Failed to send packet 5/152 for frame 0
W (9674) camera: Failed to send frame 0
W (9704) wifi: UDP send failed: Not enough space
W (9704) wifi: Failed to send packet 5/152 for frame 0
W (9704) camera: Failed to send frame 0
W (9724) wifi: UDP send failed: Not enough space
W (9724) wifi: Failed to send packet 3/152 for frame 0
W (9724) camera: Failed to send frame 0
W (9754) wifi: UDP send failed: Not enough space
W (9754) wifi: Failed to send packet 3/152 for frame 0
W (9754) camera: Failed to send frame 0
W (9774) wifi: UDP send failed: Not enough space
W (9774) wifi: Failed to send packet 7/152 for frame 0
W (9774) camera: Failed to send frame 0
W (9804) wifi: UDP send failed: Not enough space
W (9804) wifi: Failed to send packet 4/152 for frame 0
W (9804) camera: Failed to send frame 0
W (9824) wifi: UDP send failed: Not enough space
W (9824) wifi: Failed to send packet 5/152 for frame 0
W (9824) camera: Failed to send frame 0
W (9854) wifi: UDP send failed: Not enough space
W (9854) wifi: Failed to send packet 2/152 for frame 0
W (9854) camera: Failed to send frame 0
W (9874) wifi: UDP send failed: Not enough space
W (9874) wifi: Failed to send packet 4/152 for frame 0
W (9874) camera: Failed to send frame 0
W (9904) wifi: UDP send failed: Not enough space
W (9904) wifi: Failed to send packet 4/152 for frame 0
W (9904) camera: Failed to send frame 0
W (9924) wifi: UDP send failed: Not enough space
W (9924) wifi: Failed to send packet 4/152 for frame 0
W (9924) camera: Failed to send frame 0
W (9954) wifi: UDP send failed: Not enough space
W (9954) wifi: Failed to send packet 1/152 for frame 0
W (9954) camera: Failed to send frame 0
W (9974) wifi: UDP send failed: Not enough space
W (9974) wifi: Failed to send packet 2/152 for frame 0
W (9974) camera: Failed to send frame 0
W (10004) wifi: UDP send failed: Not enough space
W (10004) wifi: Failed to send packet 4/152 for frame 0
W (10004) camera: Failed to send frame 0
I (10024) camera: Camera FPS: 0.0, LCD FPS: 0.0
W (10024) wifi: UDP send failed: Not enough space
W (10024) wifi: Failed to send packet 6/152 for frame 0
W (10024) camera: Failed to send frame 0
W (10054) wifi: UDP send failed: Not enough space
W (10054) wifi: Failed to send packet 3/152 for frame 0
W (10054) camera: Failed to send frame 0
I (10074) main: FPV Status - FPS: 0.0, Frames: 0, Packets: 0, Throughput: 0.00 Mbps
I (10074) main: Camera Status - Camera FPS: 0.0
W (10074) wifi: UDP send failed: Not enough space
W (10074) wifi: Failed to send packet 5/152 for frame 0
W (10084) camera: Failed to send frame 0
W (10104) wifi: UDP send failed: Not enough space
W (10104) wifi: Failed to send packet 2/152 for frame 0
W (10104) camera: Failed to send frame 0
W (10124) wifi: UDP send failed: Not enough space
W (10124) wifi: Failed to send packet 5/152 for frame 0
W (10124) camera: Failed to send frame 0
W (10154) wifi: UDP send failed: Not enough space
W (10154) wifi: Failed to send packet 3/152 for frame 0
W (10154) camera: Failed to send frame 0
W (10174) wifi: UDP send failed: Not enough space
W (10174) wifi: Failed to send packet 4/152 for frame 0
W (10174) camera: Failed to send frame 0
W (10204) wifi: UDP send failed: Not enough space
W (10204) wifi: Failed to send packet 5/152 for frame 0
W (10204) camera: Failed to send frame 0
W (10224) wifi: UDP send failed: Not enough space
W (10224) wifi: Failed to send packet 5/152 for frame 0
W (10224) camera: Failed to send frame 0
W (10254) wifi: UDP send failed: Not enough space
W (10254) wifi: Failed to send packet 2/152 for frame 0
W (10254) camera: Failed to send frame 0
W (10274) wifi: UDP send failed: Not enough space
W (10274) wifi: Failed to send packet 5/152 for frame 0
W (10274) camera: Failed to send frame 0
W (10304) wifi: UDP send failed: Not enough space
W (10304) wifi: Failed to send packet 5/152 for frame 0
W (10304) camera: Failed to send frame 0
W (10324) wifi: UDP send failed: Not enough space
W (10324) wifi: Failed to send packet 5/152 for frame 0
W (10324) camera: Failed to send frame 0
W (10354) wifi: UDP send failed: Not enough space
W (10354) wifi: Failed to send packet 1/152 for frame 0
W (10354) camera: Failed to send frame 0
W (10374) wifi: UDP send failed: Not enough space
W (10374) wifi: Failed to send packet 6/152 for frame 0
W (10374) camera: Failed to send frame 0
W (10404) wifi: UDP send failed: Not enough space
W (10404) wifi: Failed to send packet 6/152 for frame 0
W (10404) camera: Failed to send frame 0
W (10424) wifi: UDP send failed: Not enough space
W (10424) wifi: Failed to send packet 4/152 for frame 0
W (10424) camera: Failed to send frame 0
W (10454) wifi: UDP send failed: Not enough space
W (10454) wifi: Failed to send packet 2/152 for frame 0
W (10454) camera: Failed to send frame 0
W (10474) wifi: UDP send failed: Not enough space
W (10474) wifi: Failed to send packet 4/152 for frame 0
W (10474) camera: Failed to send frame 0
W (10504) wifi: UDP send failed: Not enough space
W (10504) wifi: Failed to send packet 5/152 for frame 0
W (10504) camera: Failed to send frame 0
W (10524) wifi: UDP send failed: Not enough space
W (10524) wifi: Failed to send packet 3/152 for frame 0
W (10524) camera: Failed to send frame 0
W (10554) wifi: UDP send failed: Not enough space
W (10554) wifi: Failed to send packet 3/152 for frame 0
W (10554) camera: Failed to send frame 0
W (10574) wifi: UDP send failed: Not enough space
W (10574) wifi: Failed to send packet 5/152 for frame 0
W (10574) camera: Failed to send frame 0
W (10604) wifi: UDP send failed: Not enough space
W (10604) wifi: Failed to send packet 2/152 for frame 0
W (10604) camera: Failed to send frame 0
W (10624) wifi: UDP send failed: Not enough space
W (10624) wifi: Failed to send packet 4/152 for frame 0
W (10624) camera: Failed to send frame 0
W (10654) wifi: UDP send failed: Not enough space
W (10654) wifi: Failed to send packet 3/152 for frame 0
W (10654) camera: Failed to send frame 0
W (10674) wifi: UDP send failed: Not enough space
W (10674) wifi: Failed to send packet 4/152 for frame 0
W (10674) camera: Failed to send frame 0
W (10704) wifi: UDP send failed: Not enough space
W (10704) wifi: Failed to send packet 5/152 for frame 0
W (10704) camera: Failed to send frame 0
W (10724) wifi: UDP send failed: Not enough space
W (10724) wifi: Failed to send packet 4/152 for frame 0
W (10724) camera: Failed to send frame 0
W (10754) wifi: UDP send failed: Not enough space
W (10754) wifi: Failed to send packet 7/152 for frame 0
W (10754) camera: Failed to send frame 0
W (10774) wifi: UDP send failed: Not enough space
W (10774) wifi: Failed to send packet 1/152 for frame 0
W (10774) camera: Failed to send frame 0
W (10804) wifi: UDP send failed: Not enough space
W (10804) wifi: Failed to send packet 2/152 for frame 0
W (10804) camera: Failed to send frame 0
W (10824) wifi: UDP send failed: Not enough space
W (10824) wifi: Failed to send packet 2/152 for frame 0
W (10824) camera: Failed to send frame 0
W (10854) wifi: UDP send failed: Not enough space
W (10854) wifi: Failed to send packet 7/152 for frame 0
W (10854) camera: Failed to send frame 0
W (10874) wifi: UDP send failed: Not enough space
W (10874) wifi: Failed to send packet 4/152 for frame 0
W (10874) camera: Failed to send frame 0
W (10904) wifi: UDP send failed: Not enough space
W (10904) wifi: Failed to send packet 3/152 for frame 0
W (10904) camera: Failed to send frame 0
W (10924) wifi: UDP send failed: Not enough space
W (10924) wifi: Failed to send packet 5/152 for frame 0
W (10924) camera: Failed to send frame 0
W (10954) wifi: UDP send failed: Not enough space
W (10954) wifi: Failed to send packet 6/152 for frame 0
W (10954) camera: Failed to send frame 0
W (10974) wifi: UDP send failed: Not enough space
W (10974) wifi: Failed to send packet 4/152 for frame 0
W (10974) camera: Failed to send frame 0
W (11004) wifi: UDP send failed: Not enough space
W (11004) wifi: Failed to send packet 5/152 for frame 0
W (11004) camera: Failed to send frame 0
I (11024) camera: Camera FPS: 0.0, LCD FPS: 0.0
W (11024) wifi: UDP send failed: Not enough space
W (11024) wifi: Failed to send packet 4/152 for frame 0
W (11024) camera: Failed to send frame 0
W (11054) wifi: UDP send failed: Not enough space
W (11054) wifi: Failed to send packet 7/152 for frame 0
W (11054) camera: Failed to send frame 0
W (11074) wifi: UDP send failed: Not enough space
W (11074) wifi: Failed to send packet 4/152 for frame 0
W (11074) camera: Failed to send frame 0
W (11104) wifi: UDP send failed: Not enough space
W (11104) wifi: Failed to send packet 6/152 for frame 0
W (11104) camera: Failed to send frame 0
W (11124) wifi: UDP send failed: Not enough space
W (11124) wifi: Failed to send packet 3/152 for frame 0
W (11124) camera: Failed to send frame 0
W (11154) wifi: UDP send failed: Not enough space
W (11154) wifi: Failed to send packet 2/152 for frame 0
W (11154) camera: Failed to send frame 0
W (11174) wifi: UDP send failed: Not enough space
W (11174) wifi: Failed to send packet 2/152 for frame 0
W (11174) camera: Failed to send frame 0
W (11204) wifi: UDP send failed: Not enough space
W (11204) wifi: Failed to send packet 1/152 for frame 0
W (11204) camera: Failed to send frame 0
W (11224) wifi: UDP send failed: Not enough space
W (11224) wifi: Failed to send packet 3/152 for frame 0
W (11224) camera: Failed to send frame 0
W (11254) wifi: UDP send failed: Not enough space
W (11254) wifi: Failed to send packet 5/152 for frame 0
W (11254) camera: Failed to send frame 0
W (11274) wifi: UDP send failed: Not enough space
W (11274) wifi: Failed to send packet 2/152 for frame 0
W (11274) camera: Failed to send frame 0
W (11304) wifi: UDP send failed: Not enough space
W (11304) wifi: Failed to send packet 3/152 for frame 0
W (11304) camera: Failed to send frame 0
W (11324) wifi: UDP send failed: Not enough space
W (11324) wifi: Failed to send packet 2/152 for frame 0
W (11324) camera: Failed to send frame 0
W (11354) wifi: UDP send failed: Not enough space
W (11354) wifi: Failed to send packet 6/152 for frame 0
W (11354) camera: Failed to send frame 0
W (11374) wifi: UDP send failed: Not enough space
W (11374) wifi: Failed to send packet 2/152 for frame 0
W (11374) camera: Failed to send frame 0
W (11404) wifi: UDP send failed: Not enough space
W (11404) wifi: Failed to send packet 1/152 for frame 0
W (11404) camera: Failed to send frame 0
W (11424) wifi: UDP send failed: Not enough space
W (11424) wifi: Failed to send packet 4/152 for frame 0
W (11424) camera: Failed to send frame 0
W (11454) wifi: UDP send failed: Not enough space
W (11454) wifi: Failed to send packet 4/152 for frame 0
W (11454) camera: Failed to send frame 0
W (11474) wifi: UDP send failed: Not enough space
W (11474) wifi: Failed to send packet 2/152 for frame 0
W (11474) camera: Failed to send frame 0
W (11504) wifi: UDP send failed: Not enough space
W (11504) wifi: Failed to send packet 2/152 for frame 0
W (11504) camera: Failed to send frame 0
W (11524) wifi: UDP send failed: Not enough space
W (11524) wifi: Failed to send packet 3/152 for frame 0
W (11524) camera: Failed to send frame 0
W (11554) wifi: UDP send failed: Not enough space
W (11554) wifi: Failed to send packet 3/152 for frame 0
W (11554) camera: Failed to send frame 0
W (11574) wifi: UDP send failed: Not enough space
W (11574) wifi: Failed to send packet 3/152 for frame 0
W (11574) camera: Failed to send frame 0
W (11604) wifi: UDP send failed: Not enough space
W (11604) wifi: Failed to send packet 5/152 for frame 0
W (11604) camera: Failed to send frame 0
W (11624) wifi: UDP send failed: Not enough space
W (11624) wifi: Failed to send packet 4/152 for frame 0
W (11624) camera: Failed to send frame 0
W (11654) wifi: UDP send failed: Not enough space
W (11654) wifi: Failed to send packet 2/152 for frame 0
W (11654) camera: Failed to send frame 0
W (11674) wifi: UDP send failed: Not enough space
W (11674) wifi: Failed to send packet 3/152 for frame 0
W (11674) camera: Failed to send frame 0
W (11704) wifi: UDP send failed: Not enough space
W (11704) wifi: Failed to send packet 5/152 for frame 0
W (11704) camera: Failed to send frame 0

Done

╭─░▒▓     ~/d/P/C/ESP3/camera_test1  on    main ⇡6