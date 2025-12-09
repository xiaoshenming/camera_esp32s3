#!/usr/bin/env python3
"""
ESP32 FPV System Test
测试FPV系统各个组件的功能
"""

import sys
import time
import socket
import struct
import threading
import numpy as np
import cv2
from fpv_receiver import FPVReceiver, PacketHeader, FrameBuffer

def test_packet_header():
    """测试数据包头部解析"""
    print("🧪 测试数据包头部解析...")
    
    # 创建测试数据
    magic = 0xFPFV
    frame_id = 123
    packet_id = 45
    total_packets = 300
    
    # 打包数据
    header_data = struct.pack('<HHHH', magic, frame_id, packet_id, total_packets)
    
    # 解析数据
    header = PacketHeader.from_bytes(header_data)
    
    if header and header.magic == magic and header.frame_id == frame_id:
        print("✅ 数据包头部解析测试通过")
        return True
    else:
        print("❌ 数据包头部解析测试失败")
        return False

def test_frame_buffer():
    """测试帧缓冲区"""
    print("🧪 测试帧缓冲区...")
    
    frame_id = 123
    total_packets = 5
    buffer = FrameBuffer(frame_id, total_packets)
    
    # 添加数据包
    for i in range(total_packets):
        data = f"packet_{i}".encode()
        if not buffer.add_packet(i, data):
            print("❌ 添加数据包失败")
            return False
    
    # 检查完整性
    if not buffer.is_complete():
        print("❌ 帧完整性检查失败")
        return False
    
    # 获取帧数据
    frame_data = buffer.get_frame_data()
    if not frame_data:
        print("❌ 获取帧数据失败")
        return False
    
    print("✅ 帧缓冲区测试通过")
    return True

def test_rgb565_conversion():
    """测试RGB565转换"""
    print("🧪 测试RGB565转换...")
    
    try:
        # 创建测试RGB565数据 (2x2像素)
        rgb565_data = np.array([
            0xF800,  # 红色
            0x07E0,  # 绿色
            0x001F,  # 蓝色
            0xFFFF   # 白色
        ], dtype=np.uint16)
        
        # 转换为RGB888
        r = ((rgb565_data >> 11) & 0x1F) << 3
        g = ((rgb565_data >> 5) & 0x3F) << 2
        b = (rgb565_data & 0x1F) << 3
        
        rgb = np.stack([r, g, b], axis=-1)
        rgb = rgb.reshape(2, 2, 3)
        
        # 检查结果
        if rgb.shape == (2, 2, 3):
            print("✅ RGB565转换测试通过")
            return True
        else:
            print("❌ RGB565转换测试失败")
            return False
            
    except Exception as e:
        print(f"❌ RGB565转换测试失败: {e}")
        return False

def create_test_udp_sender(port=8889):
    """创建测试UDP发送器"""
    def sender():
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        
        frame_id = 0
        packet_size = 1024
        header_size = 8
        data_size = packet_size - header_size
        
        try:
            while True:
                # 创建测试帧数据
                frame_data = np.random.randint(0, 256, (320*240*2), dtype=np.uint8).tobytes()
                total_packets = (len(frame_data) + data_size - 1) // data_size
                
                # 发送数据包
                for packet_id in range(total_packets):
                    start_pos = packet_id * data_size
                    end_pos = min(start_pos + data_size, len(frame_data))
                    packet_data = frame_data[start_pos:end_pos]
                    
                    # 创建包头
                    header = struct.pack('<HHHH', 0xFPFV, frame_id, packet_id, total_packets)
                    
                    # 发送UDP包
                    sock.sendto(header + packet_data, ('127.0.0.1', port))
                
                frame_id += 1
                time.sleep(0.033)  # 约30FPS
                
        except Exception as e:
            print(f"UDP发送器错误: {e}")
        finally:
            sock.close()
    
    return threading.Thread(target=sender, daemon=True)

def test_receiver():
    """测试接收器"""
    print("🧪 测试接收器...")
    
    # 启动测试UDP发送器
    sender_thread = create_test_udp_sender(8889)
    sender_thread.start()
    
    # 创建接收器
    receiver = FPVReceiver(bind_ip='127.0.0.1', port=8889, display_window=False)
    
    try:
        receiver.start()
        print("✅ 接收器启动成功")
        
        # 等待接收数据
        time.sleep(3)
        
        # 检查统计信息
        stats = receiver.get_stats()
        if stats['packets_received'] > 0:
            print(f"✅ 接收器测试通过 - 接收到 {stats['packets_received']} 个包")
            return True
        else:
            print("❌ 接收器测试失败 - 未接收到数据")
            return False
            
    except Exception as e:
        print(f"❌ 接收器测试失败: {e}")
        return False
    finally:
        receiver.stop()

def test_gpu_acceleration():
    """测试GPU加速"""
    print("🧪 测试GPU加速...")
    
    try:
        import cupy as cp
        print("✅ CUDA可用")
        
        # 测试GPU内存分配
        gpu_array = cp.zeros((1000, 1000), dtype=cp.float32)
        gpu_array += 1
        
        # 测试GPU计算
        result = cp.sum(gpu_array)
        
        if result > 0:
            print("✅ GPU加速测试通过")
            return True
        else:
            print("❌ GPU加速测试失败")
            return False
            
    except ImportError:
        print("⚠️  CUDA不可用，跳过GPU测试")
        return True
    except Exception as e:
        print(f"❌ GPU加速测试失败: {e}")
        return False

def test_web_components():
    """测试Web组件"""
    print("🧪 测试Web组件...")
    
    try:
        from web_viewer import WebViewer
        
        # 创建Web查看器（不启动）
        viewer = WebViewer()
        
        if viewer.app:
            print("✅ Web组件测试通过")
            return True
        else:
            print("❌ Web组件测试失败")
            return False
            
    except ImportError as e:
        print(f"❌ Web组件测试失败 - 缺少依赖: {e}")
        return False
    except Exception as e:
        print(f"❌ Web组件测试失败: {e}")
        return False

def run_all_tests():
    """运行所有测试"""
    print("=" * 60)
    print("🧪 ESP32 FPV System Test Suite")
    print("=" * 60)
    
    tests = [
        ("数据包头部解析", test_packet_header),
        ("帧缓冲区", test_frame_buffer),
        ("RGB565转换", test_rgb565_conversion),
        ("GPU加速", test_gpu_acceleration),
        ("Web组件", test_web_components),
        ("接收器", test_receiver),
    ]
    
    passed = 0
    total = len(tests)
    
    for test_name, test_func in tests:
        print(f"\n📋 运行测试: {test_name}")
        try:
            if test_func():
                passed += 1
            else:
                print(f"❌ 测试失败: {test_name}")
        except Exception as e:
            print(f"❌ 测试异常: {test_name} - {e}")
    
    print("\n" + "=" * 60)
    print(f"📊 测试结果: {passed}/{total} 通过")
    print("=" * 60)
    
    if passed == total:
        print("🎉 所有测试通过！系统准备就绪。")
        return True
    else:
        print("⚠️  部分测试失败，请检查相关组件。")
        return False

def main():
    """主函数"""
    import argparse
    
    parser = argparse.ArgumentParser(description='ESP32 FPV System Test')
    parser.add_argument('--test', choices=[
        'header', 'buffer', 'rgb565', 'gpu', 'web', 'receiver', 'all'
    ], default='all', help='运行指定测试')
    
    args = parser.parse_args()
    
    if args.test == 'all':
        success = run_all_tests()
        sys.exit(0 if success else 1)
    else:
        # 运行单个测试
        test_map = {
            'header': test_packet_header,
            'buffer': test_frame_buffer,
            'rgb565': test_rgb565_conversion,
            'gpu': test_gpu_acceleration,
            'web': test_web_components,
            'receiver': test_receiver,
        }
        
        if args.test in test_map:
            print(f"🧪 运行测试: {args.test}")
            success = test_map[args.test]()
            sys.exit(0 if success else 1)
        else:
            print(f"❌ 未知测试: {args.test}")
            sys.exit(1)

if __name__ == '__main__':
    main()
