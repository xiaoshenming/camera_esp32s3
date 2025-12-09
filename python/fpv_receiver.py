#!/usr/bin/env python3
"""
ESP32 FPV Camera Receiver
高性能UDP接收器，支持GPU加速图像处理
"""

import socket
import struct
import threading
import time
import numpy as np
import cv2
from collections import deque
from dataclasses import dataclass
from typing import Optional, Dict, Tuple
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

# 配置日志
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# 常量定义
UDP_MAGIC = 0xFPFV  # 魔数
FRAME_WIDTH = 320
FRAME_HEIGHT = 240
PIXEL_FORMAT = 'RGB565'
PACKET_SIZE = 1024  # UDP包大小
HEADER_SIZE = 8     # 包头大小
DATA_PER_PACKET = PACKET_SIZE - HEADER_SIZE

@dataclass
class PacketHeader:
    """UDP数据包头部"""
    magic: int
    frame_id: int
    packet_id: int
    total_packets: int
    
    @classmethod
    def from_bytes(cls, data: bytes) -> Optional['PacketHeader']:
        """从字节数据解析包头"""
        if len(data) < HEADER_SIZE:
            return None
        
        try:
            magic, frame_id, packet_id, total_packets = struct.unpack('<HHHH', data[:HEADER_SIZE])
            if magic != UDP_MAGIC:
                return None
            return cls(magic, frame_id, packet_id, total_packets)
        except struct.error:
            return None

class FrameBuffer:
    """帧缓冲区，用于重组图像帧"""
    
    def __init__(self, frame_id: int, total_packets: int):
        self.frame_id = frame_id
        self.total_packets = total_packets
        self.packets = {}
        self.received_count = 0
        self.timestamp = time.time()
        self.expected_size = total_packets * DATA_PER_PACKET
        
    def add_packet(self, packet_id: int, data: bytes) -> bool:
        """添加数据包"""
        if packet_id in self.packets:
            return False  # 重复包
        
        self.packets[packet_id] = data
        self.received_count += 1
        return True
    
    def is_complete(self) -> bool:
        """检查帧是否完整"""
        return self.received_count == self.total_packets
    
    def get_frame_data(self) -> Optional[bytes]:
        """获取完整的帧数据"""
        if not self.is_complete():
            return None
        
        # 按包ID顺序组装数据
        frame_data = bytearray()
        for i in range(self.total_packets):
            if i not in self.packets:
                return None
            frame_data.extend(self.packets[i])
        
        return bytes(frame_data)
    
    def is_expired(self, timeout: float = 0.1) -> bool:
        """检查帧是否过期"""
        return time.time() - self.timestamp > timeout

class FPVReceiver:
    """FPV接收器主类"""
    
    def __init__(self, bind_ip: str = '0.0.0.0', port: int = 8888, 
                 enable_gpu: bool = True, display_window: bool = True):
        self.bind_ip = bind_ip
        self.port = port
        self.enable_gpu = enable_gpu and CUDA_AVAILABLE
        self.display_window = display_window
        
        # 网络相关
        self.socket = None
        self.running = False
        
        # 帧缓冲区
        self.frame_buffers: Dict[int, FrameBuffer] = {}
        self.current_frame_id = 0
        self.frame_queue = queue.Queue(maxsize=2)  # 显示队列
        
        # 统计信息
        self.stats = {
            'packets_received': 0,
            'frames_complete': 0,
            'frames_dropped': 0,
            'fps': 0.0,
            'last_fps_time': time.time(),
            'fps_frames': 0
        }
        
        # GPU内存池（如果启用CUDA）
        if self.enable_gpu:
            self.gpu_memory_pool = cp.get_default_memory_pool()
            self.gpu_memory_pool.set_limit(size=2**30)  # 1GB限制
    
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
            
            # 启动处理线程
            self.process_thread = threading.Thread(target=self._process_loop, daemon=True)
            self.process_thread.start()
            
            # 启动显示线程
            if self.display_window:
                self.display_thread = threading.Thread(target=self._display_loop, daemon=True)
                self.display_thread.start()
            
            logger.info(f"FPV接收器已启动，监听 {self.bind_ip}:{self.port}")
            logger.info(f"GPU加速: {'启用' if self.enable_gpu else '禁用'}")
            
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
        while self.running:
            try:
                data, addr = self.socket.recvfrom(PACKET_SIZE)
                self.stats['packets_received'] += 1
                
                # 解析包头
                header = PacketHeader.from_bytes(data)
                if not header:
                    continue
                
                # 获取数据部分
                packet_data = data[HEADER_SIZE:]
                
                # 处理帧
                self._process_packet(header, packet_data)
                
            except socket.error:
                continue  # 非阻塞socket的正常行为
            except Exception as e:
                logger.error(f"接收数据包错误: {e}")
    
    def _process_packet(self, header: PacketHeader, data: bytes):
        """处理接收到的数据包"""
        frame_id = header.frame_id
        
        # 清理过期帧
        self._cleanup_expired_frames()
        
        # 获取或创建帧缓冲区
        if frame_id not in self.frame_buffers:
            self.frame_buffers[frame_id] = FrameBuffer(frame_id, header.total_packets)
        
        frame_buffer = self.frame_buffers[frame_id]
        
        # 添加数据包
        if frame_buffer.add_packet(header.packet_id, data):
            # 检查帧是否完成
            if frame_buffer.is_complete():
                self._complete_frame(frame_id)
    
    def _cleanup_expired_frames(self):
        """清理过期的帧"""
        current_time = time.time()
        expired_frames = []
        
        for frame_id, frame_buffer in self.frame_buffers.items():
            if frame_buffer.is_expired():
                expired_frames.append(frame_id)
                self.stats['frames_dropped'] += 1
        
        for frame_id in expired_frames:
            del self.frame_buffers[frame_id]
    
    def _complete_frame(self, frame_id: int):
        """处理完整的帧"""
        frame_buffer = self.frame_buffers[frame_id]
        frame_data = frame_buffer.get_frame_data()
        
        if frame_data:
            try:
                # 将帧数据放入处理队列
                if not self.frame_queue.full():
                    self.frame_queue.put((frame_id, frame_data))
                    self.stats['frames_complete'] += 1
                    self.stats['fps_frames'] += 1
                else:
                    # 队列满，丢弃最旧的帧
                    try:
                        self.frame_queue.get_nowait()
                        self.frame_queue.put((frame_id, frame_data))
                    except queue.Empty:
                        pass
                
            except Exception as e:
                logger.error(f"处理完整帧错误: {e}")
        
        # 清理已完成的帧
        del self.frame_buffers[frame_id]
    
    def _process_loop(self):
        """处理帧的主循环"""
        while self.running:
            try:
                frame_id, frame_data = self.frame_queue.get(timeout=0.1)
                self._decode_and_display(frame_id, frame_data)
            except queue.Empty:
                continue
            except Exception as e:
                logger.error(f"处理帧错误: {e}")
    
    def _decode_and_display(self, frame_id: int, frame_data: bytes):
        """解码并显示帧"""
        try:
            # RGB565转RGB888
            if self.enable_gpu:
                frame = self._decode_rgb565_gpu(frame_data)
            else:
                frame = self._decode_rgb565_cpu(frame_data)
            
            if frame is not None:
                # 更新FPS统计
                self._update_fps()
                
                # 如果启用显示窗口，显示帧
                if self.display_window:
                    cv2.imshow('FPV Camera', frame)
                    if cv2.waitKey(1) & 0xFF == ord('q'):
                        self.running = False
                
        except Exception as e:
            logger.error(f"解码帧错误: {e}")
    
    def _decode_rgb565_cpu(self, frame_data: bytes) -> Optional[np.ndarray]:
        """CPU解码RGB565数据"""
        try:
            # 将字节数据转换为uint16数组
            rgb565 = np.frombuffer(frame_data, dtype=np.uint16)
            
            # 转换为RGB888
            r = ((rgb565 >> 11) & 0x1F) << 3
            g = ((rgb565 >> 5) & 0x3F) << 2
            b = (rgb565 & 0x1F) << 3
            
            # 合并为RGB图像
            rgb = np.stack([r, g, b], axis=-1)
            rgb = rgb.reshape(FRAME_HEIGHT, FRAME_WIDTH, 3)
            
            return rgb
            
        except Exception as e:
            logger.error(f"CPU解码错误: {e}")
            return None
    
    def _decode_rgb565_gpu(self, frame_data: bytes) -> Optional[np.ndarray]:
        """GPU加速解码RGB565数据"""
        try:
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
            rgb = cp.asnumpy(rgb_gpu)
            
            return rgb
            
        except Exception as e:
            logger.error(f"GPU解码错误: {e}")
            return None
    
    def _display_loop(self):
        """显示循环（单独线程）"""
        # 这个方法现在在_decode_and_display中处理
        pass
    
    def _update_fps(self):
        """更新FPS统计"""
        current_time = time.time()
        if current_time - self.stats['last_fps_time'] >= 1.0:
            self.stats['fps'] = self.stats['fps_frames'] / (current_time - self.stats['last_fps_time'])
            self.stats['last_fps_time'] = current_time
            self.stats['fps_frames'] = 0
            
            # 打印统计信息
            logger.info(f"FPS: {self.stats['fps']:.1f}, "
                       f"完整帧: {self.stats['frames_complete']}, "
                       f"丢弃帧: {self.stats['frames_dropped']}, "
                       f"接收包: {self.stats['packets_received']}")
    
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
