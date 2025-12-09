#!/usr/bin/env python3
"""
网络连接测试脚本
"""

import socket
import struct
import time

def test_udp_receive():
    """测试UDP接收"""
    print("🔍 测试UDP接收...")
    
    # 创建socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('0.0.0.0', 8888))
    sock.settimeout(5.0)  # 5秒超时
    
    print("✅ Socket已绑定到 0.0.0.0:8888")
    print("⏳ 等待UDP数据包（5秒超时）...")
    
    try:
        while True:
            try:
                data, addr = sock.recvfrom(65536)
                print(f"📦 收到数据包: 来源 {addr}, 大小 {len(data)} 字节")
                
                if len(data) >= 6:
                    magic, width, height = struct.unpack('<HHH', data[:6])
                    print(f"🔍 包头: 魔数=0x{magic:04X}, 宽度={width}, 高度={height}")
                    
                    if magic == 0x5056:
                        print("✅ 魔数匹配！这是ESP32的数据包")
                        print(f"📊 图像数据大小: {len(data)-6} 字节")
                        return True
                    else:
                        print(f"⚠️ 魔数不匹配: 期望0x5056, 实际0x{magic:04X}")
                
            except socket.timeout:
                print("⏰ 5秒内未收到数据包")
                return False
            except KeyboardInterrupt:
                print("\n🛑 用户中断")
                return False
                
    finally:
        sock.close()

def test_network_info():
    """测试网络信息"""
    print("\n🌐 网络信息:")
    
    # 获取本机IP地址
    try:
        # 连接到外部地址获取本机IP
        test_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        test_sock.connect(("8.8.8.8", 80))
        local_ip = test_sock.getsockname()[0]
        test_sock.close()
        print(f"📍 本机IP地址: {local_ip}")
    except:
        print("❌ 无法获取本机IP地址")
    
    # 测试端口是否被占用
    try:
        test_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        test_sock.bind(('0.0.0.0', 8888))
        test_sock.close()
        print("✅ 端口8888可用")
    except:
        print("❌ 端口8888被占用")

if __name__ == '__main__':
    print("=" * 50)
    print("🧪 ESP32 FPV 网络连接测试")
    print("=" * 50)
    
    test_network_info()
    print()
    
    success = test_udp_receive()
    
    if success:
        print("\n🎉 网络连接测试成功！")
        print("💡 如果看到这个消息，说明ESP32正在发送数据")
    else:
        print("\n❌ 网络连接测试失败")
        print("💡 可能的原因:")
        print("   1. ESP32未运行或未连接WiFi")
        print("   2. 网络配置问题（不在同一网段）")
        print("   3. 防火墙阻止UDP端口8888")
        print("   4. ESP32发送到错误的IP地址")
