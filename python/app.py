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

# 配置日志
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
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
            self.rtsp_socket.settimeout(10)
            self.rtsp_socket.connect((parsed_url['host'], parsed_url['port']))
            
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
        
        try:
            while self.is_streaming:
                # 接收数据
                data = self.rtsp_socket.recv(65536)
                if not data:
                    break
                
                # 解析TCP interleaved数据
                self.process_interleaved_data(data)
                
        except Exception as e:
            logger.error(f"RGB565流处理错误: {e}")
        finally:
            if hasattr(self, 'rtsp_socket'):
                self.rtsp_socket.close()
        
        logger.info("RGB565流处理线程已结束")
    
    def process_interleaved_data(self, data):
        """处理TCP interleaved数据"""
        try:
            offset = 0
            while offset < len(data):
                if offset + 4 > len(data):
                    break
                
                # 检查interleaved frame header: '$' + channel + length
                if data[offset] == 0x24:  # '$'
                    channel = data[offset + 1]
                    length = (data[offset + 2] << 8) | data[offset + 3]
                    
                    if offset + 4 + length > len(data):
                        break
                    
                    # 跳过RTP头部(12字节)，获取RGB565数据
                    if length > 12:
                        rgb565_data = data[offset + 4 + 12:offset + 4 + length]
                        self.convert_rgb565_to_bgr(rgb565_data)
                    
                    offset += 4 + length
                else:
                    offset += 1
                    
        except Exception as e:
            logger.error(f"处理interleaved数据错误: {e}")
    
    def convert_rgb565_to_bgr(self, rgb565_data):
        """将RGB565数据转换为BGR格式"""
        try:
            # RGB565数据长度应该是640*480*2 = 614400字节
            if len(rgb565_data) != 614400:
                return
            
            # 转换为numpy数组
            rgb565_array = np.frombuffer(rgb565_data, dtype=np.uint16)
            
            # 重塑为640x480
            rgb565_image = rgb565_array.reshape((480, 640))
            
            # 提取RGB分量
            r5 = (rgb565_image >> 11) & 0x1F
            g6 = (rgb565_image >> 5) & 0x3F
            b5 = rgb565_image & 0x1F
            
            # 扩展到8位
            r8 = (r5 << 3) | (r5 >> 2)
            g8 = (g6 << 2) | (g6 >> 4)
            b8 = (b5 << 3) | (b5 >> 2)
            
            # 创建BGR图像
            bgr_image = np.stack([b8, g8, r8], axis=2).astype(np.uint8)
            
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
                    
        except Exception as e:
            logger.error(f"RGB565转换错误: {e}")
    
    def run(self, host='0.0.0.0', port=5000, debug=False):
        """运行Flask应用"""
        logger.info(f"启动Web服务器: http://{host}:{port}")
        self.socketio.run(self.app, host=host, port=port, debug=debug)

def main():
    """主函数"""
    import argparse
    
    parser = argparse.ArgumentParser(description='ESP32-S3 Camera RTSP Stream Viewer')
    parser.add_argument('--host', default='0.0.0.0', help='服务器主机地址')
    parser.add_argument('--port', type=int, default=5000, help='服务器端口')
    parser.add_argument('--debug', action='store_true', help='启用调试模式')
    parser.add_argument('--rtsp', help='自动连接的RTSP URL')
    
    args = parser.parse_args()
    
    # 创建流媒体服务器
    streamer = ESP32CameraStreamer()
    
    # 如果提供了RTSP URL，自动连接
    if args.rtsp:
        logger.info(f"自动连接到RTSP流: {args.rtsp}")
        streamer.connect_rtsp(args.rtsp)
    
    # 启动服务器
    try:
        streamer.run(host=args.host, port=args.port, debug=args.debug)
    except KeyboardInterrupt:
        logger.info("服务器已停止")
    finally:
        streamer.disconnect_rtsp()

if __name__ == '__main__':
    main()
