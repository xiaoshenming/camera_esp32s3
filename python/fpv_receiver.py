#!/usr/bin/env python3
"""
ESP32 FPV Camera Receiver - 简化版
实时UDP接收器，支持完整帧传输
"""

import socket
import struct
import threading
import time
import numpy as np
import cv2
import queue
import argparse
import logging

# 尝试导入CUDA支持
try:
    import cupy as cp
    CUDA_AVAILABLE = True
    print("CUDA加速已启用")
except ImportError:
    CUDA_AVAILABLE = False
    print("CUDA不可用，使用CPU处理")

# 检查CUDA运行时库
try:
    import cupy as cp
    cp.cuda.Device(0).compute_capability
    print("CUDA运行时库检查通过")
except Exception as e:
    CUDA_AVAILABLE = False
    print(f"CUDA运行时库检查失败: {e}")
    print("将使用CPU处理")

# 配置日志
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# 常量定义
UDP_MAGIC = 0x5056  # 魔数
FRAME_WIDTH = 160    # QQVGA（实际工作分辨率）
FRAME_HEIGHT = 120   # QQVGA（实际工作分辨率）
PIXEL_FORMAT = 'RGB565'
MAX_FRAME_SIZE = FRAME_WIDTH * FRAME_HEIGHT * 2  # RGB565 = 2 bytes per pixel

class FPVReceiver:
    """简化的FPV接收器"""
    
    def __init__(self, bind_ip: str = '0.0.0.0', port: int = 8888, 
                 enable_gpu: bool = True, display_window: bool = True, esp32_ip: str = '192.168.1.100'):
        self.bind_ip = bind_ip
        self.port = port
        self.esp32_ip = esp32_ip  # 新增ESP32 IP配置
        # 强制禁用GPU以确保稳定性
        self.enable_gpu = False
        self.display_window = display_window
        
        # 网络相关
        self.socket = None
        self.running = False
        
        # 帧队列
        self.frame_queue = queue.Queue(maxsize=1)  # 只保留最新帧
        
        # 统计信息
        self.stats = {
            'frames_received': 0,
            'frames_dropped': 0,
            'fps': 0.0,
            'last_fps_time': time.time(),
            'fps_frames': 0
        }
        
        logger.info(f"FPV接收器初始化完成 - 分辨率: {FRAME_WIDTH}x{FRAME_HEIGHT}")
        logger.info(f"GPU加速: {'启用' if self.enable_gpu else '禁用'}")
    
    def start(self):
        """启动接收器"""
        try:
            # 创建UDP socket
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 64*1024*1024)  # 64MB接收缓冲区
            self.socket.bind((self.bind_ip, self.port))
            self.socket.setblocking(False)
            
            self.running = True
            
            # 启动接收线程
            self.receive_thread = threading.Thread(target=self._receive_loop, daemon=True)
            self.receive_thread.start()
            
            # 启动显示线程
            if self.display_window:
                self.display_thread = threading.Thread(target=self._display_loop, daemon=True)
                self.display_thread.start()
            
            logger.info(f"FPV接收器已启动，监听 {self.bind_ip}:{self.port}")
            
        except Exception as e:
            logger.error(f"启动接收器失败: {e}")
            self.stop()
            raise
    
    def stop(self):
        """停止接收器"""
        self.running = False
        if self.socket:
            self.socket.close()
        logger.info("FPV接收器已停止")
    
    def _receive_loop(self):
        """接收数据包的主循环"""
        print(f"🔍 开始监听UDP数据包，期望来自ESP32 ({self.esp32_ip})...")
        while self.running:
            try:
                data, addr = self.socket.recvfrom(65536)  # 最大UDP包大小
                
                # 打印接收到的数据包信息（调试用）
                print(f"📦 收到UDP包: 来源 {addr}, 大小 {len(data)} 字节")
                
                # 检查是否来自期望的ESP32 IP（允许广播）
                if addr[0] != self.esp32_ip and addr[0] != "255.255.255.255":
                    print(f"⚠️ 数据包来源不匹配: 期望 {self.esp32_ip} 或广播, 实际 {addr[0]}")
                    continue
                
                # 解析包头
                if len(data) < 6:  # 最小包头大小
                    print(f"⚠️ 数据包太小: {len(data)} 字节")
                    continue
                
                try:
                    magic, width, height = struct.unpack('<HHH', data[:6])
                    print(f"🔍 包头解析: 魔数=0x{magic:04X}, 宽度={width}, 高度={height}")
                    
                    if magic != UDP_MAGIC:
                        print(f"⚠️ 魔数不匹配: 期望0x{UDP_MAGIC:04X}, 实际0x{magic:04X}")
                        continue
                    if width != FRAME_WIDTH or height != FRAME_HEIGHT:
                        print(f"⚠️ 分辨率不匹配: 期望{FRAME_WIDTH}x{FRAME_HEIGHT}, 实际{width}x{height}")
                        continue
                    
                    # 获取图像数据
                    frame_data = data[6:]
                    if len(frame_data) != MAX_FRAME_SIZE:
                        print(f"⚠️ 帧大小不匹配: 期望{MAX_FRAME_SIZE}, 实际{len(frame_data)}")
                        continue
                    
                    # 处理帧
                    self._process_frame(frame_data)
                    print(f"✅ 成功接收帧: {len(frame_data)} 字节")
                    
                except struct.error as e:
                    print(f"⚠️ 包头解析错误: {e}")
                    continue
                
            except socket.error as e:
                continue  # 非阻塞socket的正常行为
            except Exception as e:
                logger.error(f"接收数据包错误: {e}")
    
    def _process_frame(self, frame_data: bytes):
        """处理接收到的完整帧"""
        try:
            # 如果是Web模式且有Web解码函数，直接调用
            if hasattr(self, '_web_decode_and_display') and not self.display_window:
                self._web_decode_and_display(0, frame_data)
                self.stats['frames_received'] += 1
                self.stats['fps_frames'] += 1
                return
            
            # 将帧数据放入队列（非阻塞）
            if not self.frame_queue.empty():
                try:
                    self.frame_queue.get_nowait()  # 丢弃旧帧
                    self.stats['frames_dropped'] += 1
                except queue.Empty:
                    pass
            
            self.frame_queue.put(frame_data)
            self.stats['frames_received'] += 1
            self.stats['fps_frames'] += 1
            
            # 实时更新统计
            self._update_fps()
            
        except queue.Full:
            self.stats['frames_dropped'] += 1
        except Exception as e:
            logger.error(f"处理帧错误: {e}")
    
    def _display_loop(self):
        """显示循环"""
        while self.running:
            try:
                frame_data = self.frame_queue.get(timeout=0.1)
                frame = self._decode_rgb565(frame_data)
                
                if frame is not None:
                    # 更新FPS统计
                    self._update_fps()
                    
                    # 显示帧
                    cv2.imshow('FPV Camera', frame)
                    if cv2.waitKey(1) & 0xFF == ord('q'):
                        self.running = False
                
            except queue.Empty:
                continue
            except Exception as e:
                logger.error(f"显示帧错误: {e}")
    
    def _decode_rgb565(self, frame_data: bytes) -> np.ndarray:
        """解码RGB565数据"""
        try:
            if len(frame_data) != MAX_FRAME_SIZE:
                return None
            
            if self.enable_gpu:
                return self._decode_rgb565_gpu(frame_data)
            else:
                return self._decode_rgb565_cpu(frame_data)
                
        except Exception as e:
            logger.error(f"解码帧错误: {e}")
            return None
    
    def _decode_rgb565_cpu(self, frame_data: bytes) -> np.ndarray:
        """CPU解码RGB565数据"""
        try:
            if len(frame_data) != MAX_FRAME_SIZE:
                logger.error(f"帧数据大小错误: {len(frame_data)}, 期望: {MAX_FRAME_SIZE}")
                return None
            
            # 将字节数据转换为uint16数组，注意字节序
            rgb565 = np.frombuffer(frame_data, dtype=np.uint16)
            
            # 调试：打印前几个像素的原始值
            if self.stats['frames_received'] % 100 == 1:  # 每100帧打印一次
                logger.info(f"RGB565原始数据前4个值: {rgb565[:4]}")
            
            # 转换为RGB888 - 修复字节序问题
            r = ((rgb565 >> 11) & 0x1F) << 3
            g = ((rgb565 >> 5) & 0x3F) << 2
            b = (rgb565 & 0x1F) << 3
            
            # 确保值在有效范围内
            r = np.clip(r, 0, 255)
            g = np.clip(g, 0, 255)
            b = np.clip(b, 0, 255)
            
            # 合并为RGB图像
            rgb = np.stack([r, g, b], axis=-1)
            rgb = rgb.reshape(FRAME_HEIGHT, FRAME_WIDTH, 3)
            
            # 调试：检查图像数据
            if self.stats['frames_received'] % 100 == 1:  # 每100帧打印一次
                logger.info(f"解码后图像统计 - R:[{rgb[:,:,0].min()}-{rgb[:,:,0].max()}] "
                           f"G:[{rgb[:,:,1].min()}-{rgb[:,:,1].max()}] "
                           f"B:[{rgb[:,:,2].min()}-{rgb[:,:,2].max()}]")
            
            return rgb.astype(np.uint8)
            
        except Exception as e:
            logger.error(f"CPU解码错误: {e}")
            return None
    
    def _decode_rgb565_gpu(self, frame_data: bytes) -> np.ndarray:
        """GPU加速解码RGB565数据"""
        try:
            # 检查CuPy是否真正可用
            if not CUDA_AVAILABLE:
                logger.warning("CuPy不可用，回退到CPU解码")
                return self._decode_rgb565_cpu(frame_data)
            
            # 将数据传输到GPU
            rgb565_gpu = cp.frombuffer(frame_data, dtype=cp.uint16)
            
            # GPU并行转换
            r = ((rgb565_gpu >> 11) & 0x1F) << 3
            g = ((rgb565_gpu >> 5) & 0x3F) << 2
            b = (rgb565_gpu & 0x1F) << 3
            
            # 合并为RGB图像
            rgb_gpu = cp.stack([r, g, b], axis=-1)
            rgb_gpu = rgb_gpu.reshape(FRAME_HEIGHT, FRAME_WIDTH, 3)
            
            # 传回CPU
            rgb = cp.asnumpy(rgb_gpu).astype(np.uint8)
            
            return rgb
            
        except Exception as e:
            logger.error(f"GPU解码错误: {e}")
            logger.info("回退到CPU解码")
            return self._decode_rgb565_cpu(frame_data)
    
    def _update_fps(self):
        """更新FPS统计"""
        current_time = time.time()
        if current_time - self.stats['last_fps_time'] >= 1.0:
            self.stats['fps'] = self.stats['fps_frames'] / (current_time - self.stats['last_fps_time'])
            self.stats['last_fps_time'] = current_time
            self.stats['fps_frames'] = 0
            
            # 打印统计信息
            logger.info(f"FPS: {self.stats['fps']:.1f}, "
                       f"接收帧: {self.stats['frames_received']}, "
                       f"丢弃帧: {self.stats['frames_dropped']}")
    
    def get_stats(self) -> dict:
        """获取统计信息"""
        return self.stats.copy()

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description='ESP32 FPV Camera Receiver')
    parser.add_argument('--ip', default='0.0.0.0', help='绑定IP地址')
    parser.add_argument('--port', type=int, default=8888, help='监听端口')
    parser.add_argument('--no-gpu', action='store_true', help='禁用GPU加速')
    parser.add_argument('--no-display', action='store_true', help='禁用显示窗口')
    
    args = parser.parse_args()
    
    # 创建接收器
    receiver = FPVReceiver(
        bind_ip=args.ip,
        port=args.port,
        enable_gpu=not args.no_gpu,
        display_window=not args.no_display
    )
    
    try:
        # 启动接收器
        receiver.start()
        
        # 主循环
        while receiver.running:
            time.sleep(0.1)
            
    except KeyboardInterrupt:
        logger.info("接收到中断信号")
    finally:
        receiver.stop()
        if receiver.display_window:
            cv2.destroyAllWindows()

if __name__ == '__main__':
    main()
