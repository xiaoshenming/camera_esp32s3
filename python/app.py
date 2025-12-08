#!/usr/bin/env python3
"""
ESP32-S3 Camera RTSP Stream Viewer
实时视频流查看器，支持RTSP流接收和Web显示
"""

import cv2
import numpy as np
import base64
import io
from flask import Flask, render_template, Response, request, jsonify
from flask_socketio import SocketIO, emit
import threading
import time
import logging
from datetime import datetime
import os
import sys
import socket
import struct
from socket import SO_KEEPALIVE, TCP_NODELAY, SO_RCVBUF, SO_SNDBUF

# 配置日志
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class ESP32CameraStreamer:
    """ESP32摄像头RTSP流处理器"""
    
    def __init__(self):
        self.app = Flask(__name__)
        self.app.config['SECRET_KEY'] = 'esp32-camera-secret-key'
        self.socketio = SocketIO(self.app, cors_allowed_origins="*", async_mode='threading')
        
        # 摄像头相关变量
        self.cap = None
        self.rtsp_url = ""
        self.is_streaming = False
        self.current_frame = None
        self.frame_lock = threading.Lock()
        self.stream_thread = None
        
        # 统计信息
        self.fps = 0
        self.frame_count = 0
        self.start_time = time.time()
        self.last_fps_update = time.time()
        
        # 录制相关
        self.is_recording = False
        self.video_writer = None
        self.recorded_frames = []
        
        # 设置路由
        self.setup_routes()
        self.setup_socketio_events()
        
    def setup_routes(self):
        """设置Flask路由"""
        
        @self.app.route('/')
        def index():
            """主页"""
            return render_template('index.html')
        
        @self.app.route('/video_feed')
        def video_feed():
            """视频流路由"""
            return Response(self.generate_frames(),
                          mimetype='multipart/x-mixed-replace; boundary=frame')
        
        @self.app.route('/api/connect', methods=['POST'])
        def connect_camera():
            """连接到RTSP流"""
            data = request.get_json()
            rtsp_url = data.get('rtsp_url', '')
            
            if not rtsp_url:
                return jsonify({'success': False, 'message': 'RTSP URL不能为空'})
            
            if self.connect_rtsp(rtsp_url):
                return jsonify({'success': True, 'message': '连接成功'})
            else:
                return jsonify({'success': False, 'message': '连接失败'})
        
        @self.app.route('/api/disconnect', methods=['POST'])
        def disconnect_camera():
            """断开RTSP连接"""
            self.disconnect_rtsp()
            return jsonify({'success': True, 'message': '已断开连接'})
        
        @self.app.route('/api/status')
        def get_status():
            """获取流状态"""
            return jsonify({
                'is_streaming': self.is_streaming,
                'rtsp_url': self.rtsp_url,
                'fps': self.fps,
                'is_recording': self.is_recording
            })
        
        @self.app.route('/api/snapshot', methods=['POST'])
        def take_snapshot():
            """拍摄快照"""
            with self.frame_lock:
                if self.current_frame is not None:
                    # 保存快照
                    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
                    filename = f"snapshot_{timestamp}.jpg"
                    cv2.imwrite(filename, self.current_frame)
                    
                    # 转换为base64返回
                    _, buffer = cv2.imencode('.jpg', self.current_frame)
                    img_base64 = base64.b64encode(buffer).decode('utf-8')
                    
                    return jsonify({
                        'success': True,
                        'filename': filename,
                        'image': f"data:image/jpeg;base64,{img_base64}"
                    })
            
            return jsonify({'success': False, 'message': '没有可用的帧'})
        
        @self.app.route('/api/record', methods=['POST'])
        def toggle_recording():
            """切换录制状态"""
            data = request.get_json()
            action = data.get('action', 'toggle')
            
            if action == 'start':
                return self.start_recording()
            elif action == 'stop':
                return self.stop_recording()
            else:
                return self.toggle_recording()
    
    def setup_socketio_events(self):
        """设置SocketIO事件"""
        
        @self.socketio.on('connect')
        def handle_connect():
            """客户端连接事件"""
            logger.info("客户端已连接")
            emit('status', {
                'is_streaming': self.is_streaming,
                'rtsp_url': self.rtsp_url,
                'fps': self.fps
            })
        
        @self.socketio.on('disconnect')
        def handle_disconnect():
            """客户端断开事件"""
            logger.info("客户端已断开")
    
    def connect_rtsp(self, rtsp_url):
        """连接到RTSP流"""
        try:
            if self.is_streaming:
                self.disconnect_rtsp()
            
            logger.info(f"正在连接到RTSP流: {rtsp_url}")
            
            # 解析RTSP URL
            parsed_url = self.parse_rtsp_url(rtsp_url)
            if not parsed_url:
                logger.error("无法解析RTSP URL")
                return False
            
            # 创建RTSP连接
            if not self.setup_rtsp_connection(parsed_url):
                logger.error("无法建立RTSP连接")
                return False
            
            self.rtsp_url = rtsp_url
            self.is_streaming = True
            
            # 启动流处理线程
            self.stream_thread = threading.Thread(target=self.rgb565_stream_worker, daemon=True)
            self.stream_thread.start()
            
            logger.info("RTSP流连接成功")
            self.socketio.emit('status', {
                'is_streaming': True,
                'rtsp_url': self.rtsp_url,
                'fps': self.fps
            })
            
            return True
            
        except Exception as e:
            logger.error(f"连接RTSP流时出错: {e}")
            return False
    
    def disconnect_rtsp(self):
        """断开RTSP连接"""
        self.is_streaming = False
        
        # 关闭RTSP socket
        if hasattr(self, 'rtsp_socket'):
            try:
                # 发送TEARDOWN请求
                if hasattr(self, 'cseq') and hasattr(self, 'rtsp_url') and self.rtsp_url:
                    parsed_url = self.parse_rtsp_url(self.rtsp_url)
                    if parsed_url:
                        self.cseq += 1
                        teardown_request = f"TEARDOWN {parsed_url['path']} RTSP/1.0\r\n"
                        teardown_request += f"CSeq: {self.cseq}\r\n"
                        teardown_request += "User-Agent: ESP32-Camera-Client\r\n\r\n"
                        
                        try:
                            self.rtsp_socket.send(teardown_request.encode())
                        except:
                            pass  # 忽略发送错误
                
                self.rtsp_socket.close()
            except:
                pass  # 忽略关闭错误
        
        if self.cap:
            self.cap.release()
            self.cap = None
        
        if self.stream_thread and self.stream_thread.is_alive():
            self.stream_thread.join(timeout=2)
        
        # 清理缓冲区
        if hasattr(self, 'data_buffer'):
            self.data_buffer = b''
        
        self.rtsp_url = ""
        self.current_frame = None
        
        logger.info("RTSP流已断开")
        self.socketio.emit('status', {
            'is_streaming': False,
            'rtsp_url': "",
            'fps': 0
        })
    
    def stream_worker(self):
        """流处理工作线程"""
        logger.info("流处理线程已启动")
        
        while self.is_streaming and self.cap:
            try:
                ret, frame = self.cap.read()
                if not ret:
                    logger.warning("无法读取帧，尝试重连...")
                    time.sleep(1)
                    continue
                
                with self.frame_lock:
                    self.current_frame = frame.copy()
                
                # 更新FPS
                self.frame_count += 1
                current_time = time.time()
                if current_time - self.last_fps_update >= 1.0:
                    self.fps = self.frame_count / (current_time - self.last_fps_update)
                    self.frame_count = 0
                    self.last_fps_update = current_time
                    
                    # 录制帧
                    if self.is_recording and self.video_writer:
                        self.video_writer.write(frame)
                
                time.sleep(0.033)  # 约30FPS
                
            except Exception as e:
                logger.error(f"流处理错误: {e}")
                break
        
        logger.info("流处理线程已结束")
    
    def generate_frames(self):
        """生成JPEG帧用于HTTP流"""
        while True:
            with self.frame_lock:
                if self.current_frame is not None:
                    # 编码为JPEG
                    _, buffer = cv2.imencode('.jpg', self.current_frame, [cv2.IMWRITE_JPEG_QUALITY, 80])
                    frame_bytes = buffer.tobytes()
                    
                    yield (b'--frame\r\n'
                           b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n')
                else:
                    # 发送占位符帧
                    placeholder = np.zeros((480, 640, 3), dtype=np.uint8)
                    cv2.putText(placeholder, "等待视频流...", (200, 240), 
                               cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
                    _, buffer = cv2.imencode('.jpg', placeholder)
                    frame_bytes = buffer.tobytes()
                    
                    yield (b'--frame\r\n'
                           b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n')
            
            time.sleep(0.1)
    
    def toggle_recording(self):
        """切换录制状态"""
        if self.is_recording:
            return self.stop_recording()
        else:
            return self.start_recording()
    
    def start_recording(self):
        """开始录制"""
        if self.is_recording:
            return jsonify({'success': False, 'message': '已经在录制中'})
        
        if not self.is_streaming:
            return jsonify({'success': False, 'message': '没有视频流'})
        
        try:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"recording_{timestamp}.avi"
            
            # 获取当前帧的尺寸
            with self.frame_lock:
                if self.current_frame is not None:
                    height, width = self.current_frame.shape[:2]
                else:
                    height, width = 480, 640
            
            # 创建VideoWriter
            fourcc = cv2.VideoWriter_fourcc(*'XVID')
            self.video_writer = cv2.VideoWriter(filename, fourcc, 20.0, (width, height))
            
            self.is_recording = True
            logger.info(f"开始录制: {filename}")
            
            return jsonify({'success': True, 'message': f'开始录制: {filename}'})
            
        except Exception as e:
            logger.error(f"开始录制时出错: {e}")
            return jsonify({'success': False, 'message': f'录制失败: {e}'})
    
    def stop_recording(self):
        """停止录制"""
        if not self.is_recording:
            return jsonify({'success': False, 'message': '没有在录制'})
        
        try:
            self.is_recording = False
            if self.video_writer:
                self.video_writer.release()
                self.video_writer = None
            
            logger.info("录制已停止")
            return jsonify({'success': True, 'message': '录制已停止'})
            
        except Exception as e:
            logger.error(f"停止录制时出错: {e}")
            return jsonify({'success': False, 'message': f'停止录制失败: {e}'})
    
    def parse_rtsp_url(self, rtsp_url):
        """解析RTSP URL"""
        try:
            # 解析 rtsp://ip:port/path
            if rtsp_url.startswith('rtsp://'):
                url = rtsp_url[7:]  # 移除 'rtsp://'
                parts = url.split('/')
                if len(parts) >= 1:
                    host_port = parts[0]
                    if ':' in host_port:
                        host, port = host_port.split(':')
                        port = int(port)
                    else:
                        host = host_port
                        port = 554  # 默认RTSP端口
                    
                    path = '/' + '/'.join(parts[1:]) if len(parts) > 1 else '/'
                    
                    return {
                        'host': host,
                        'port': port,
                        'path': path
                    }
        except Exception as e:
            logger.error(f"解析RTSP URL失败: {e}")
        return None
    
    def setup_rtsp_connection(self, parsed_url):
        """设置RTSP连接"""
        try:
            # 创建TCP连接
            self.rtsp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

            # 优化socket设置
            self.rtsp_socket.setsockopt(socket.SOL_SOCKET, SO_KEEPALIVE, 1)
            self.rtsp_socket.setsockopt(socket.IPPROTO_TCP, TCP_NODELAY, 1)
            self.rtsp_socket.setsockopt(socket.SOL_SOCKET, SO_RCVBUF, 65536)  # 64KB接收缓冲区
            self.rtsp_socket.setsockopt(socket.SOL_SOCKET, SO_SNDBUF, 65536)  # 64KB发送缓冲区

            self.rtsp_socket.settimeout(5)  # 减少超时时间
            logger.info(f"正在连接到 {parsed_url['host']}:{parsed_url['port']}")
            self.rtsp_socket.connect((parsed_url['host'], parsed_url['port']))
            logger.info("TCP连接建立成功")
            
            # 发送OPTIONS请求
            self.cseq = 1
            options_request = f"OPTIONS {parsed_url['path']} RTSP/1.0\r\n"
            options_request += f"CSeq: {self.cseq}\r\n"
            options_request += "User-Agent: ESP32-Camera-Client\r\n\r\n"
            
            self.rtsp_socket.send(options_request.encode())
            response = self.rtsp_socket.recv(1024).decode()
            
            if "200 OK" not in response:
                logger.error("OPTIONS请求失败")
                return False
            
            # 发送DESCRIBE请求
            self.cseq += 1
            describe_request = f"DESCRIBE {parsed_url['path']} RTSP/1.0\r\n"
            describe_request += f"CSeq: {self.cseq}\r\n"
            describe_request += "Accept: application/sdp\r\n"
            describe_request += "User-Agent: ESP32-Camera-Client\r\n\r\n"
            
            self.rtsp_socket.send(describe_request.encode())
            response = self.rtsp_socket.recv(4096).decode()
            
            if "200 OK" not in response:
                logger.error("DESCRIBE请求失败")
                return False
            
            # 发送SETUP请求
            self.cseq += 1
            setup_request = f"SETUP {parsed_url['path']}/streamid=0 RTSP/1.0\r\n"
            setup_request += f"CSeq: {self.cseq}\r\n"
            setup_request += "Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n"
            setup_request += "User-Agent: ESP32-Camera-Client\r\n\r\n"
            
            self.rtsp_socket.send(setup_request.encode())
            response = self.rtsp_socket.recv(1024).decode()
            
            if "200 OK" not in response:
                logger.error("SETUP请求失败")
                return False
            
            # 发送PLAY请求
            self.cseq += 1
            play_request = f"PLAY {parsed_url['path']} RTSP/1.0\r\n"
            play_request += f"CSeq: {self.cseq}\r\n"
            play_request += "User-Agent: ESP32-Camera-Client\r\n\r\n"
            
            self.rtsp_socket.send(play_request.encode())
            response = self.rtsp_socket.recv(1024).decode()
            
            if "200 OK" not in response:
                logger.error("PLAY请求失败")
                return False
            
            logger.info("RTSP连接建立成功")
            return True
            
        except Exception as e:
            logger.error(f"设置RTSP连接失败: {e}")
            return False
    
    def rgb565_stream_worker(self):
        """RGB565流处理工作线程"""
        logger.info("RGB565流处理线程已启动")

        retry_count = 0
        max_retries = 3
        retry_delay = 2  # 秒

        while self.is_streaming and retry_count < max_retries:
            try:
                # 设置socket超时
                self.rtsp_socket.settimeout(5.0)

                consecutive_errors = 0
                max_consecutive_errors = 10

                while self.is_streaming:
                    try:
                        # 使用较小的接收缓冲区，匹配ESP32的512字节块大小
                        data = self.rtsp_socket.recv(1024)  # 1KB缓冲区
                        if not data:
                            logger.info("连接已关闭")
                            break

                        # 检查是否是RTSP响应（以RTSP/开头）
                        if data.startswith(b'RTSP/'):
                            logger.debug(f"收到RTSP响应: {data[:100]}")
                            continue

                        # 解析TCP interleaved数据
                        self.process_interleaved_data(data)

                        # 重置错误计数器
                        consecutive_errors = 0
                        retry_count = 0  # 成功接收数据，重置重试计数

                    except socket.timeout:
                        # 超时是正常的，继续循环
                        consecutive_errors = 0
                        continue
                    except ConnectionResetError as e:
                        logger.error(f"连接被重置: {e}")
                        consecutive_errors += 1
                        break
                    except ConnectionAbortedError as e:
                        logger.error(f"连接被中止: {e}")
                        consecutive_errors += 1
                        break
                    except Exception as e:
                        logger.error(f"接收数据时出错: {e}")
                        consecutive_errors += 1
                        if consecutive_errors >= max_consecutive_errors:
                            logger.error(f"连续错误{consecutive_errors}次，尝试重新连接")
                            break
                        time.sleep(0.1)  # 短暂延迟后重试
                        continue

                # 如果退出内循环但仍然需要流，则尝试重新连接
                if self.is_streaming:
                    logger.info(f"尝试重新连接 ({retry_count + 1}/{max_retries})")
                    self.rtsp_socket.close()

                    # 等待一段时间后重新连接
                    time.sleep(retry_delay)

                    # 解析URL并重新建立连接
                    parsed_url = self.parse_rtsp_url(self.rtsp_url)
                    if parsed_url and self.setup_rtsp_connection(parsed_url):
                        logger.info("重新连接成功")
                        continue
                    else:
                        logger.error("重新连接失败")
                        retry_count += 1
                else:
                    break

            except Exception as e:
                logger.error(f"流处理错误: {e}")
                retry_count += 1
                time.sleep(retry_delay)

        # 最终清理
        try:
            if hasattr(self, 'rtsp_socket'):
                self.rtsp_socket.close()
        except:
            pass

        if retry_count >= max_retries:
            logger.error(f"重连{max_retries}次失败，停止流处理")
            self.is_streaming = False

        logger.info("RGB565流处理线程已结束")
    
    def process_interleaved_data(self, data):
        """处理简化的RGB565帧数据"""
        try:
            # 缓冲区用于处理跨包的数据
            if not hasattr(self, 'data_buffer'):
                self.data_buffer = b''
                self.debug_last_log = 0
                self.total_frames_received = 0

            # 将新数据添加到缓冲区
            self.data_buffer += data

            # 限制缓冲区大小，防止内存泄漏
            max_buffer_size = 2 * 1024 * 1024  # 2MB缓冲区限制
            if len(self.data_buffer) > max_buffer_size:
                logger.warning(f"缓冲区过大，重置缓冲区: {len(self.data_buffer)} bytes")
                self.data_buffer = b''
                return

            # 减少日志输出频率
            current_time = time.time()
            should_log = current_time - self.debug_last_log > 5  # 每5秒记录一次
            if should_log:
                logger.info(f"缓冲区大小: {len(self.data_buffer)} 字节")
                if len(self.data_buffer) > 0:
                    logger.info(f"数据前8字节: {self.data_buffer[:8].hex()}")
                self.debug_last_log = current_time

            frame_count = 0
            max_frames_per_process = 5  # 每次最多处理5帧

            # 寻找帧同步头: AA55FFFE
            while len(self.data_buffer) >= 614408 and frame_count < max_frames_per_process:  # 8字节头 + 614400字节数据
                # 寻找帧同步头
                sync_pos = self.data_buffer.find(b'\xAA\x55\xFF\xFE')
                if sync_pos == -1:
                    # 没有找到同步头，清空大部分缓冲区
                    self.data_buffer = self.data_buffer[-100:] if len(self.data_buffer) > 100 else b''
                    break

                # 移除同步头之前的数据
                if sync_pos > 0:
                    self.data_buffer = self.data_buffer[sync_pos:]

                # 检查是否有完整的帧
                if len(self.data_buffer) < 614408:  # 8字节头 + 614400字节数据
                    break

                # 解析帧头
                if len(self.data_buffer) >= 8:
                    # 验证同步头
                    if (self.data_buffer[0] == 0xAA and self.data_buffer[1] == 0x55 and
                        self.data_buffer[2] == 0xFF and self.data_buffer[3] == 0xFE):

                        # 解析数据长度
                        data_len = (self.data_buffer[4] << 24) | (self.data_buffer[5] << 16) | (self.data_buffer[6] << 8) | self.data_buffer[7]

                        # 验证数据长度
                        if data_len == 614400:  # RGB565数据长度
                            # 提取RGB565数据
                            rgb565_data = self.data_buffer[8:8+data_len]

                            # 转换并显示图像
                            self.convert_rgb565_to_bgr(rgb565_data)
                            frame_count += 1
                            self.total_frames_received += 1

                            # 每20帧记录一次
                            if self.total_frames_received % 20 == 0:
                                logger.info(f"成功处理了 {self.total_frames_received} 帧")

                            # 移除已处理的帧
                            self.data_buffer = self.data_buffer[8+data_len:]
                            continue
                        else:
                            logger.warning(f"数据长度不匹配: 期望614400, 实际{data_len}")
                            # 跳过这个同步头
                            self.data_buffer = self.data_buffer[1:]
                            continue
                    else:
                        # 同步头不匹配，跳过第一个字节
                        self.data_buffer = self.data_buffer[1:]

            if frame_count > 0 and should_log:
                logger.info(f"本次处理了 {frame_count} 帧")

        except Exception as e:
            logger.error(f"处理RGB565数据错误: {e}")
            import traceback
            logger.error(f"详细错误信息: {traceback.format_exc()}")
            # 出错时重置缓冲区
            self.data_buffer = b''
    
    def convert_rgb565_to_bgr(self, rgb565_data):
        """将RGB565数据转换为BGR格式"""
        try:
            # RGB565数据长度应该是640*480*2 = 614400字节
            expected_length = 640 * 480 * 2
            if len(rgb565_data) != expected_length:
                logger.debug(f"RGB565数据长度不匹配: 期望{expected_length}, 实际{len(rgb565_data)}")
                return
            
            # 直接逐字节解析RGB565数据 (大端序)
            height, width = 480, 640
            bgr_image = np.zeros((height, width, 3), dtype=np.uint8)

            for i in range(height):
                for j in range(width):
                    # 每个像素2字节
                    idx = (i * width + j) * 2
                    if idx + 1 < len(rgb565_data):
                        # 大端序: 高字节在前，低字节在后
                        high_byte = rgb565_data[idx]
                        low_byte = rgb565_data[idx + 1]
                        rgb565 = (high_byte << 8) | low_byte

                        # 提取RGB分量
                        r5 = (rgb565 >> 11) & 0x1F  # 5位红色
                        g6 = (rgb565 >> 5) & 0x3F   # 6位绿色
                        b5 = rgb565 & 0x1F         # 5位蓝色

                        # 扩展到8位
                        r8 = (r5 * 255) // 31
                        g8 = (g6 * 255) // 63
                        b8 = (b5 * 255) // 31

                        # 设置BGR像素值
                        bgr_image[i, j] = [b8, g8, r8]
            
            # 检查图像是否有效
            if np.any(bgr_image > 0):
                with self.frame_lock:
                    self.current_frame = bgr_image.copy()
                
                # 更新FPS
                self.frame_count += 1
                current_time = time.time()
                if current_time - self.last_fps_update >= 1.0:
                    self.fps = self.frame_count / (current_time - self.last_fps_update)
                    self.frame_count = 0
                    self.last_fps_update = current_time
                    
                    # 录制帧
                    if self.is_recording and self.video_writer:
                        self.video_writer.write(bgr_image)
            else:
                logger.debug("接收到全黑图像")
                    
        except Exception as e:
            logger.error(f"RGB565转换错误: {e}")
            import traceback
            logger.error(f"详细错误信息: {traceback.format_exc()}")
    
    def run(self, host='0.0.0.0', port=5000, debug=False):
        """运行Flask应用"""
        logger.info(f"启动Web服务器: http://{host}:{port}")
        try:
            self.socketio.run(self.app, host=host, port=port, debug=debug)
        except KeyboardInterrupt:
            logger.info("收到中断信号，正在关闭...")
        except Exception as e:
            logger.error(f"服务器运行错误: {e}")
        finally:
            # 确保清理资源
            self.disconnect_rtsp()

def main():
    """主函数"""
    import argparse
    import signal
    import sys

    parser = argparse.ArgumentParser(description='ESP32-S3 Camera RTSP Stream Viewer')
    parser.add_argument('--host', default='0.0.0.0', help='服务器主机地址')
    parser.add_argument('--port', type=int, default=5000, help='服务器端口')
    parser.add_argument('--debug', action='store_true', help='启用调试模式')
    parser.add_argument('--rtsp', help='自动连接的RTSP URL')

    args = parser.parse_args()

    # 创建流媒体服务器
    streamer = ESP32CameraStreamer()

    def signal_handler(sig, frame):
        logger.info("收到信号，正在关闭...")
        streamer.disconnect_rtsp()
        sys.exit(0)

    # 注册信号处理器
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    # 如果提供了RTSP URL，自动连接
    if args.rtsp:
        logger.info(f"自动连接到RTSP流: {args.rtsp}")
        streamer.connect_rtsp(args.rtsp)

    # 启动服务器
    try:
        streamer.run(host=args.host, port=args.port, debug=args.debug)
    except Exception as e:
        logger.error(f"服务器错误: {e}")
    finally:
        streamer.disconnect_rtsp()

if __name__ == '__main__':
    main()
