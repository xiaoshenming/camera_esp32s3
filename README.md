# ESP32 Camera Test Project

## 项目结构

本项目已重构为模块化结构，便于维护和扩展：

```
camera_test1/
├── main/                    # 主程序目录
│   ├── main.c              # 主程序入口文件
│   ├── main.h              # 主程序头文件
│   └── CMakeLists.txt      # main 组件构建配置
├── components/             # 组件目录
│   ├── camera/            # 摄像头组件
│   │   ├── camera.c       # 摄像头功能实现
│   │   ├── camera.h       # 摄像头功能头文件
│   │   └── CMakeLists.txt # camera 组件构建配置
│   ├── uart/              # 串口组件
│   │   ├── uart.c         # 串口功能实现
│   │   ├── uart.h         # 串口功能头文件
│   │   └── CMakeLists.txt # uart 组件构建配置
│   └── lcd/               # LCD显示组件
│       ├── lcd.c          # LCD功能实现
│       ├── lcd.h          # LCD功能头文件
│       └── CMakeLists.txt # lcd 组件构建配置
├── CMakeLists.txt         # 项目根构建配置
├── sdkconfig             # ESP-IDF 配置文件
└── .gitignore            # Git 忽略文件
```

## 模块说明

### main 模块
- `main.c`: 程序入口点，包含 `app_main()` 函数
- `main.h`: 主程序头文件，包含全局声明
- 负责初始化和调用各个组件

### components 模块
- 所有功能模块都放在 `components/` 目录下
- 每个组件都是独立的 ESP-IDF 组件
- 组件可以有自己的源文件、头文件和 CMakeLists.txt

### camera 组件示例
- `camera.c`: 摄像头相关功能实现
- `camera.h`: 摄像头功能接口声明
- `CMakeLists.txt`: 组件构建配置，声明依赖关系

### uart 组件示例
- `uart.c`: 串口相关功能实现，包括初始化和数据发送
- `uart.h`: 串口功能接口声明
- `CMakeLists.txt`: 组件构建配置，声明依赖关系
- 功能：初始化 UART0，波特率 115200，可发送字符串和 "Hello World" 消息

### lcd 组件示例
- `lcd.c`: LCD显示相关功能实现，包括ST7789驱动和背光控制
- `lcd.h`: LCD功能接口声明
- `CMakeLists.txt`: 组件构建配置，声明依赖关系
- 功能：初始化 320x240 LCD显示屏，支持图片显示和摄像头实时视频流显示
- 硬件：ST7789驱动芯片，SPI接口，PCA9557 IO扩展芯片控制

## 实时视频流功能

本项目实现了摄像头实时视频流在LCD屏幕上的显示功能：

### 功能特性
- **实时显示**: 摄像头捕获的图像实时显示在LCD屏幕上
- **双任务架构**: 使用独立的摄像头捕获任务和LCD显示任务
- **队列通信**: 通过FreeRTOS队列实现摄像头帧数据的传递
- **内存管理**: 自动管理摄像头帧缓冲区的分配和释放
- **错误处理**: 完善的错误检测和恢复机制

### 工作流程
1. **初始化阶段**:
   - 初始化I2C接口（用于IO扩展芯片通信）
   - 初始化PCA9557 IO扩展芯片
   - 初始化LCD显示屏（ST7789驱动）
   - 初始化摄像头（RGB565格式，QVGA分辨率）

2. **运行阶段**:
   - 摄像头捕获任务持续获取图像帧
   - 通过队列将帧数据传递给LCD显示任务
   - LCD显示任务接收帧数据并实时显示

3. **硬件配置**:
   - 摄像头：GC0308，RGB565格式，320x240分辨率
   - LCD：ST7789驱动，320x240分辨率，SPI接口
   - IO扩展：PCA9557，控制LCD CS、摄像头PWDN等引脚

## 添加新组件

1. 在 `components/` 目录下创建新文件夹
2. 创建组件的源文件、头文件和 CMakeLists.txt
3. 在 main.c 中包含新组件的头文件
4. 在 app_main() 中调用新组件的初始化函数

## 构建和运行

```bash
# 配置项目
idf.py menuconfig
# 下载组件
idf.py reconfigure 
# 构建项目
idf.py build

# 烧录到设备
idf.py flash monitor
idf.py build &&  idf.py flash monitor
