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
            
            # 创建VideoCapture对象
            self.cap = cv2.VideoCapture(rtsp_url)
            
            # 设置缓冲区大小以减少延迟
            self.cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
            
            # 设置超时
            self.cap.set(cv2.CAP_PROP_OPEN_TIMEOUT_MSEC, 5000)
            self.cap.set(cv2.CAP_PROP_READ_TIMEOUT_MSEC, 5000)
            
            # 检查连接是否成功
            if not self.cap.isOpened():
                logger.error("无法打开RTSP流")
                return False
            
            # 读取第一帧测试
            ret, frame = self.cap.read()
            if not ret:
                logger.error("无法从RTSP流读取帧")
                self.cap.release()
                return False
            
            self.rtsp_url = rtsp_url
            self.is_streaming = True
            
            # 启动流处理线程
            self.stream_thread = threading.Thread(target=self.stream_worker, daemon=True)
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
