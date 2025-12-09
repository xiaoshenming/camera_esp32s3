 &&  idf.py flash monitor
Executing action: all (aliases: build)
Running ninja in directory /home/ming/data/Project/ClionProject/ESP32/camera_test1/build
Executing "ninja all"...
[1/1] cd /home/ming/data/Project/ClionProject/ESP32/camera_test1/buil...oject/ClionProject/ESP32/camera_test1/build/bootloader/bootloader.bin
Bootloader binary size 0x5240 bytes. 0x2dc0 bytes (36%) free.
[5/10] Building C object esp-idf/main/CMakeFiles/__idf_main.dir/main.c.obj
/home/ming/data/Project/ClionProject/ESP32/camera_test1/main/main.c: In function 'app_main':
/home/ming/data/Project/ClionProject/ESP32/camera_test1/main/main.c:65:26: warning: unused variable 'capture_only_config' [-Wunused-variable]
   65 |     camera_user_config_t capture_only_config = {
      |                          ^~~~~~~~~~~~~~~~~~~
/home/ming/data/Project/ClionProject/ESP32/camera_test1/main/main.c:56:26: warning: unused variable 'streaming_config' [-Wunused-variable]
   56 |     camera_user_config_t streaming_config = {
      |                          ^~~~~~~~~~~~~~~~
[9/10] Generating binary image from built executable
esptool.py v4.10.dev2
Creating esp32s3 image...
Merged 2 ELF sections
Successfully created esp32s3 image.
Generated /home/ming/data/Project/ClionProject/ESP32/camera_test1/build/camera_test1.bin
[10/10] cd /home/ming/data/Project/ClionProject/ESP32/camera_test1/bu...g/