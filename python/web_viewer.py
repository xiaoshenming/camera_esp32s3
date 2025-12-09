#!/usr/bin/env python3
"""
ESP32 FPV Camera Web Viewer
基于Flask的Web界面，支持实时视频流查看
"""

from flask import Flask, render_template, Response, request, jsonify
import threading
import time
import json
import logging
from fpv_receiver import FPVReceiver
import cv2
import base64
import io
from PIL import Image
import numpy as np

# 配置日志
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class WebViewer:
    """Web查看器类"""
    
    def __init__(self, host='0.0.0.0', port=5000):
        self.host = host
        self.port = port
        self.app = Flask(__name__)
        self.receiver = None
        self.current_frame = None
        self.frame_lock = threading.Lock()
        self.stats = {
            'fps': 0.0,
            'frames_complete': 0,
            'frames_dropped': 0,
            'packets_received': 0,
            'last_update': time.time()
        }
        
        # 设置路由
        self._setup_routes()
    
    def _setup_routes(self):
        """设置Flask路由"""
        
        @self.app.route('/')
        def index():
            """主页"""
            return render_template('index.html')
        
        @self.app.route('/start_receiver', methods=['POST'])
        def start_receiver():
            """启动接收器"""
            try:
                data = request.get_json()
                bind_ip = data.get('bind_ip', '0.0.0.0')
                port = int(data.get('port', 8888))
                enable_gpu = data.get('enable_gpu', True)
                
                # 停止现有接收器
                if self.receiver:
                    self.receiver.stop()
                
                # 创建新接收器
                self.receiver = FPVReceiver(
                    bind_ip=bind_ip,
                    port=port,
                    enable_gpu=enable_gpu,
                    display_window=False  # Web模式下不显示窗口
                )
                
                # 修改接收器以支持Web显示
                self.receiver._decode_and_display = self._web_decode_and_display
                
                # 启动接收器
                self.receiver.start()
                
                logger.info(f"Web接收器已启动，监听 {bind_ip}:{port}")
                
                return jsonify({
                    'status': 'success',
                    'message': f'接收器已启动，监听 {bind_ip}:{port}'
                })
                
            except Exception as e:
                logger.error(f"启动接收器失败: {e}")
                return jsonify({
                    'status': 'error',
                    'message': f'启动失败: {str(e)}'
                }), 500
        
        @self.app.route('/stop_receiver', methods=['POST'])
        def stop_receiver():
            """停止接收器"""
            try:
                if self.receiver:
                    self.receiver.stop()
                    self.receiver = None
                
                with self.frame_lock:
                    self.current_frame = None
                
                return jsonify({
                    'status': 'success',
                    'message': '接收器已停止'
                })
                
            except Exception as e:
                logger.error(f"停止接收器失败: {e}")
                return jsonify({
                    'status': 'error',
                    'message': f'停止失败: {str(e)}'
                }), 500
        
        @self.app.route('/video_feed')
        def video_feed():
            """视频流"""
            return Response(self._generate_frames(),
                           mimetype='multipart/x-mixed-replace; boundary=frame')
        
        @self.app.route('/stats')
        def get_stats():
            """获取统计信息"""
            if self.receiver:
                receiver_stats = self.receiver.get_stats()
                self.stats.update(receiver_stats)
            
            return jsonify(self.stats)
        
        @self.app.route('/snapshot')
        def snapshot():
            """获取当前帧快照"""
            with self.frame_lock:
                if self.current_frame is not None:
                    # 将OpenCV图像转换为JPEG
                    _, buffer = cv2.imencode('.jpg', self.current_frame)
                    frame_bytes = buffer.tobytes()
                    
                    # 编码为base64
                    frame_b64 = base64.b64encode(frame_bytes).decode('utf-8')
                    
                    return jsonify({
                        'status': 'success',
                        'image': f'data:image/jpeg;base64,{frame_b64}',
                        'timestamp': time.time()
                    })
                else:
                    return jsonify({
                        'status': 'error',
                        'message': '没有可用的帧'
                    }), 404
    
    def _web_decode_and_display(self, frame_id: int, frame_data: bytes):
        """Web模式下的帧解码和显示"""
        try:
            # 解码帧
            if self.receiver.enable_gpu:
                frame = self.receiver._decode_rgb565_gpu(frame_data)
            else:
                frame = self.receiver._decode_rgb565_cpu(frame_data)
            
            if frame is not None:
                # 更新当前帧
                with self.frame_lock:
                    self.current_frame = frame.copy()
                
                # 更新统计信息
                self.receiver._update_fps()
                
        except Exception as e:
            logger.error(f"Web解码帧错误: {e}")
    
    def _generate_frames(self):
        """生成视频流帧"""
        while True:
            with self.frame_lock:
                if self.current_frame is not None:
                    # 将OpenCV图像转换为JPEG
                    _, buffer = cv2.imencode('.jpg', self.current_frame, [cv2.IMWRITE_JPEG_QUALITY, 80])
                    frame_bytes = buffer.tobytes()
                    
                    yield (b'--frame\r\n'
                           b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n')
                else:
                    # 发送空白帧
                    yield (b'--frame\r\n'
                           b'Content-Type: image/jpeg\r\n\r\n' + b'\r\n')
            
            time.sleep(0.033)  # 约30FPS
    
    def run(self):
        """运行Web应用"""
        logger.info(f"Web查看器启动在 http://{self.host}:{self.port}")
        self.app.run(host=self.host, port=self.port, threaded=True, debug=False)

def main():
    """主函数"""
    import argparse
    
    parser = argparse.ArgumentParser(description='ESP32 FPV Camera Web Viewer')
    parser.add_argument('--host', default='0.0.0.0', help='Web服务器主机地址')
    parser.add_argument('--port', type=int, default=5000, help='Web服务器端口')
    
    args = parser.parse_args()
    
    # 创建并运行Web查看器
    web_viewer = WebViewer(host=args.host, port=args.port)
    
    try:
        web_viewer.run()
    except KeyboardInterrupt:
        logger.info("接收到中断信号")
    finally:
        if web_viewer.receiver:
            web_viewer.receiver.stop()

if __name__ == '__main__':
    main()
