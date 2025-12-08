# ESP32-S3 Camera RTSP Stream Viewer

一个专业的ESP32-S3摄像头RTSP视频流查看器，支持实时视频流接收、Web界面显示、快照和录制功能。

## 🌟 功能特性

### 📹 视频流功能
- **RTSP协议支持**: 标准RTSP协议，兼容性强
- **实时显示**: 低延迟视频流传输
- **自适应分辨率**: 支持多种分辨率和帧率
- **连接状态监控**: 实时显示连接状态和FPS

### 🎨 Web界面
- **现代化设计**: Bootstrap 5 + 响应式布局
- **实时状态**: Socket.IO实时状态更新
- **直观操作**: 简洁易用的控制面板
- **移动端支持**: 完美适配手机和平板

### 📸 媒体功能
- **快照拍摄**: 一键保存当前画面
- **视频录制**: 支持AVI格式视频录制
- **自动命名**: 时间戳自动命名文件
- **下载功能**: 快照和录制文件下载

### 🔧 系统特性
- **多客户端支持**: 支持多个浏览器同时观看
- **错误恢复**: 自动重连和错误处理
- **性能优化**: 低CPU和内存占用
- **日志记录**: 详细的操作日志

## 🚀 快速开始

### 环境要求
- Python 3.8+
- Conda 或 pip
- Arch Linux (推荐) 或其他Linux发行版

### 安装步骤

#### 方法一：使用Conda (推荐)

1. **创建Conda环境**
```bash
cd /home/ming/data/Project/ClionProject/ESP32/camera_test1/python
conda env create -f conda_env.yml
conda activate esp32-camera-rtsp
```

2. **启动应用**
```bash
python app.py
```

#### 方法二：使用pip

1. **创建虚拟环境**
```bash
cd /home/ming/data/Project/ClionProject/ESP32/camera_test1/python
python -m venv venv
source venv/bin/activate  # Linux/Mac
# 或 venv\Scripts\activate  # Windows
```

2. **安装依赖**
```bash
pip install -r requirements.txt
```

3. **启动应用**
```bash
python app.py
```

### 启动选项

```bash
# 基本启动
python app.py

# 指定端口
python app.py --port 8080

# 指定主机
python app.py --host 127.0.0.1

# 调试模式
python app.py --debug

# 自动连接RTSP流
python app.py --rtsp rtsp://192.168.1.100:8554/stream
```

## 📱 使用说明

### 1. 访问Web界面
启动应用后，在浏览器中访问：
```
http://localhost:5000
```

### 2. 连接RTSP流
1. 在RTSP流地址输入框中输入ESP32-S3的RTSP地址
2. 点击"连接"按钮
3. 等待连接成功，视频将自动开始播放

### 3. 使用功能
- **拍摄快照**: 点击"拍摄快照"保存当前画面
- **录制视频**: 点击"开始录制"开始录制视频
- **查看状态**: 实时查看FPS、连接状态等信息

## 🔌 ESP32-S3 配置

### WiFi配置
编辑 `components/wifi/wifi.h` 文件，修改WiFi信息：

```c
#define WIFI_SSID "你的WiFi名称"
#define WIFI_PASSWORD "你的WiFi密码"
```

### 编译和烧录
```bash
cd /home/ming/data/Project/ClionProject/ESP32/camera_test1
idf.py build
idf.py flash monitor
```

### 获取RTSP地址
设备启动后，串口会显示RTSP流地址：
```
RTSP Stream URL: rtsp://192.168.1.100:8554/stream
```

## 🌐 网络配置

### 防火墙设置 (Arch Linux)
```bash
# 开放5000端口 (Web服务)
sudo ufw allow 5000/tcp

# 开放8554端口 (RTSP服务)
sudo ufw allow 8554/tcp

# 查看防火墙状态
sudo ufw status
```

### 网络测试
```bash
# 测试RTSP连接
ffplay rtsp://192.168.1.100:8554/stream

# 测试Web服务
curl http://localhost:5000/api/status
```

## 📊 API接口

### REST API

#### 连接RTSP流
```bash
POST /api/connect
Content-Type: application/json

{
    "rtsp_url": "rtsp://192.168.1.100:8554/stream"
}
```

#### 断开连接
```bash
POST /api/disconnect
```

#### 获取状态
```bash
GET /api/status
```

#### 拍摄快照
```bash
POST /api/snapshot
```

#### 录制控制
```bash
POST /api/record
Content-Type: application/json

{
    "action": "start"  # 或 "stop"
}
```

### WebSocket事件

#### 连接事件
```javascript
socket.on('status', (data) => {
    console.log('状态更新:', data);
    // data: { is_streaming, rtsp_url, fps, is_recording }
});
```

## 🛠️ 故障排除

### 常见问题

#### 1. 无法连接RTSP流
- 检查ESP32-S3是否正常启动
- 确认WiFi连接正常
- 检查防火墙设置
- 验证RTSP地址格式

#### 2. 视频延迟过高
- 检查网络带宽
- 调整摄像头分辨率
- 减少网络拥塞
- 使用有线网络

#### 3. Web界面无法访问
- 检查Python服务是否启动
- 确认端口是否被占用
- 检查防火墙设置
- 查看应用日志

#### 4. 录制功能异常
- 检查磁盘空间
- 确认写入权限
- 检查视频编码器
- 查看错误日志

### 日志查看

#### 应用日志
```bash
# 查看详细日志
python app.py --debug

# 查看特定级别日志
export PYTHONPATH=.
python -c "import logging; logging.basicConfig(level=logging.DEBUG); import app"
```

#### ESP32日志
```bash
idf.py monitor
```

## 🔧 高级配置

### 性能优化

#### 1. OpenCV优化
```python
# 在app.py中调整缓冲区大小
cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)  # 减少延迟
cap.set(cv2.CAP_PROP_FPS, 30)         # 设置帧率
```

#### 2. Web服务器优化
```python
# 使用生产环境服务器
gunicorn -w 4 -b 0.0.0.0:5000 app:app
```

### 自定义配置

#### 修改默认RTSP地址
编辑 `python/templates/index.html`:
```javascript
value="rtsp://你的ESP32_IP:8554/stream"
```

#### 调整视频质量
编辑 `python/app.py`:
```python
# 调整JPEG质量
_, buffer = cv2.imencode('.jpg', frame, [cv2.IMWRITE_JPEG_QUALITY, 90])
```

## 📈 性能监控

### 系统资源监控
```bash
# CPU使用率
top -p $(pgrep -f app.py)

# 内存使用
ps aux | grep app.py

# 网络连接
netstat -an | grep :5000
netstat -an | grep :8554
```

### 性能指标
- **CPU占用**: 通常 < 10%
- **内存占用**: 通常 < 200MB
- **网络延迟**: < 100ms (局域网)
- **视频延迟**: < 200ms

## 🔒 安全考虑

### 网络安全
- 使用强WiFi密码
- 定期更新系统
- 限制网络访问
- 监控异常连接

### 应用安全
- 不要暴露到公网
- 使用HTTPS (生产环境)
- 定期更新依赖包
- 备份重要数据

## 📚 开发指南

### 项目结构
```
python/
├── app.py                 # 主应用文件
├── requirements.txt       # Python依赖
├── conda_env.yml         # Conda环境配置
├── templates/
│   └── index.html        # Web界面模板
├── static/
│   ├── css/              # CSS样式文件
│   └── js/               # JavaScript文件
└── README.md             # 说明文档
```

### 扩展功能

#### 1. 添加AI处理
```python
def ai_process_frame(frame):
    # 添加AI处理逻辑
    return processed_frame
```

#### 2. 添加存储功能
```python
def save_to_cloud(image):
    # 添加云存储逻辑
    pass
```

## 🤝 贡献指南

### 提交问题
1. 详细描述问题
2. 提供错误日志
3. 说明系统环境
4. 包含复现步骤

### 功能建议
1. 描述功能需求
2. 说明使用场景
3. 提供设计思路
4. 考虑兼容性

## 📄 许可证

本项目采用 MIT 许可证，详见 LICENSE 文件。

## 🙏 致谢

- ESP-IDF 开发团队
- OpenCV 社区
- Flask 框架
- Bootstrap UI框架

## 📞 支持

如有问题或建议，请通过以下方式联系：
- 提交 GitHub Issue
- 发送邮件至开发者
- 加入技术交流群

---

**享受你的ESP32-S3视频流体验！** 🎉
