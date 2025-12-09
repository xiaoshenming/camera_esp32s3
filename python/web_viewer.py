#!/usr/bin/env python3
"""
ESP32 FPV Camera Web Viewer - 简化版
接收器在后台持续运行，前端只需配置ESP32地址
"""

from flask import Flask, render_template, Response, request, jsonify
import threading
import time
import logging
from fpv_receiver import FPVReceiver
import cv2

# 配置日志
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class WebViewer:
    """简化的Web查看器类"""
    
    def __init__(self, host='0.0.0.0', port=5000):
        self.host = host
        self.port = port
        self.app = Flask(__name__)
        self.receiver = None
        self.current_esp32_ip = '192.168.1.100'
        
        # 启动后台接收器
        self._start_background_receiver()
        
        # 设置路由
        self._setup_routes()
    
    def _start_background_receiver(self):
        """启动后台接收器"""
        try:
            # 创建接收器（后台模式）
            self.receiver = FPVReceiver(
                bind_ip='0.0.0.0',
                port=8888,
                enable_gpu=False,
                display_window=False,  # 后台模式
                esp32_ip=self.current_esp32_ip
            )
            
            # 启动接收器
            self.receiver.start()
            logger.info("后台接收器已启动")
            
        except Exception as e:
            logger.error(f"启动后台接收器失败: {e}")
    
    def _setup_routes(self):
        """设置Flask路由"""
        
        @self.app.route('/')
        def index():
            """主页"""
            return render_template('index.html')
        
        @self.app.route('/update_esp32_config', methods=['POST'])
        def update_esp32_config():
            """更新ESP32配置"""
            try:
                data = request.get_json()
                esp32_ip = data.get('esp32_ip', '192.168.1.100')
                
                # 更新ESP32 IP地址
                self.current_esp32_ip = esp32_ip
                self.receiver.esp32_ip = esp32_ip
                
                logger.info(f"ESP32 IP地址已更新为: {esp32_ip}")
                
                return jsonify({
                    'status': 'success',
                    'message': f'ESP32地址已更新为 {esp32_ip}'
                })
                
            except Exception as e:
                logger.error(f"更新ESP32配置失败: {e}")
                return jsonify({
                    'status': 'error',
                    'message': f'更新失败: {str(e)}'
                }), 500
        
        @self.app.route('/video_feed')
        def video_feed():
            """视频流"""
            return Response(self.receiver._generate_frames(),
                           mimetype='multipart/x-mixed-replace; boundary=frame')
        
        @self.app.route('/stats')
        def get_stats():
            """获取统计信息"""
            if self.receiver:
                stats = self.receiver.get_stats()
                stats['esp32_ip'] = self.current_esp32_ip
                return jsonify(stats)
            else:
                return jsonify({'error': '接收器未运行'})
        
        @self.app.route('/status')
        def get_status():
            """获取系统状态"""
            return jsonify({
                'status': 'running',
                'esp32_ip': self.current_esp32_ip,
                'receiver_active': self.receiver is not None and self.receiver.running
            })
    
    def run(self):
        """运行Web应用"""
        logger.info(f"Web查看器启动在 http://{self.host}:{self.port}")
        logger.info(f"后台接收器已启动，监听端口8888")
        logger.info(f"当前ESP32地址: {self.current_esp32_ip}")
        
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
