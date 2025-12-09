#!/usr/bin/env python3
"""
ESP32 FPV System Launcher
快速启动FPV系统的脚本
"""

import os
import sys
import subprocess
import argparse
import time
import threading
import webbrowser
from pathlib import Path

def check_environment():
    """检查运行环境"""
    print("🔍 检查运行环境...")
    
    # 检查Python版本
    if sys.version_info < (3, 8):
        print("❌ 需要Python 3.8或更高版本")
        return False
    
    # 检查必要的包
    required_packages = ['numpy', 'cv2', 'flask']
    missing_packages = []
    
    for package in required_packages:
        try:
            __import__(package)
        except ImportError:
            missing_packages.append(package)
    
    if missing_packages:
        print(f"❌ 缺少必要的包: {', '.join(missing_packages)}")
        print("请运行: pip install " + " ".join(missing_packages))
        return False
    
    # 检查CUDA支持（可选）
    try:
        import cupy
        print("✅ CUDA加速可用")
    except ImportError:
        print("⚠️  CUDA加速不可用，将使用CPU处理")
    
    print("✅ 环境检查通过")
    return True

def start_web_viewer(host='0.0.0.0', port=5000, open_browser=True):
    """启动Web查看器"""
    print(f"🚀 启动Web查看器在 http://{host}:{port}")
    
    # 导入web_viewer
    try:
        from web_viewer import WebViewer
        
        # 创建Web查看器
        viewer = WebViewer(host=host, port=port)
        
        # 在新线程中启动
        def run_viewer():
            viewer.run()
        
        viewer_thread = threading.Thread(target=run_viewer, daemon=True)
        viewer_thread.start()
        
        # 等待服务器启动
        time.sleep(2)
        
        # 打开浏览器
        if open_browser and host == '0.0.0.0':
            webbrowser.open(f'http://localhost:{port}')
        elif open_browser:
            webbrowser.open(f'http://{host}:{port}')
        
        return viewer
        
    except ImportError as e:
        print(f"❌ 无法导入web_viewer: {e}")
        return None
    except Exception as e:
        print(f"❌ 启动Web查看器失败: {e}")
        return None

def start_command_receiver(ip='0.0.0.0', port=8888, enable_gpu=True, enable_display=True):
    """启动命令行接收器"""
    print(f"🚀 启动命令行接收器在 {ip}:{port}")
    
    try:
        from fpv_receiver import FPVReceiver
        
        # 创建接收器
        receiver = FPVReceiver(
            bind_ip=ip,
            port=port,
            enable_gpu=enable_gpu,
            display_window=enable_display
        )
        
        # 启动接收器
        receiver.start()
        
        return receiver
        
    except ImportError as e:
        print(f"❌ 无法导入fpv_receiver: {e}")
        return None
    except Exception as e:
        print(f"❌ 启动命令行接收器失败: {e}")
        return None

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description='ESP32 FPV System Launcher')
    parser.add_argument('--mode', choices=['web', 'cli', 'both'], default='web',
                       help='启动模式: web(Web界面), cli(命令行), both(两者)')
    parser.add_argument('--host', default='0.0.0.0', help='绑定主机地址')
    parser.add_argument('--port', type=int, default=5000, help='Web服务器端口')
    parser.add_argument('--udp-port', type=int, default=8888, help='UDP接收端口')
    parser.add_argument('--no-gpu', action='store_true', help='禁用GPU加速')
    parser.add_argument('--no-browser', action='store_true', help='不自动打开浏览器')
    parser.add_argument('--no-display', action='store_true', help='命令行模式下不显示窗口')
    parser.add_argument('--check-only', action='store_true', help='仅检查环境，不启动')
    
    args = parser.parse_args()
    
    # 打印欢迎信息
    print("=" * 60)
    print("🎮 ESP32 FPV Camera System Launcher")
    print("=" * 60)
    
    # 检查环境
    if not check_environment():
        sys.exit(1)
    
    if args.check_only:
        print("✅ 环境检查完成，可以启动系统")
        return
    
    # 根据模式启动
    web_viewer = None
    cli_receiver = None
    
    try:
        if args.mode in ['web', 'both']:
            web_viewer = start_web_viewer(
                host=args.host,
                port=args.port,
                open_browser=not args.no_browser
            )
            
            if not web_viewer and args.mode == 'web':
                print("❌ Web查看器启动失败")
                sys.exit(1)
        
        if args.mode in ['cli', 'both']:
            cli_receiver = start_command_receiver(
                ip=args.host,
                port=args.udp_port,
                enable_gpu=not args.no_gpu,
                enable_display=not args.no_display
            )
            
            if not cli_receiver and args.mode == 'cli':
                print("❌ 命令行接收器启动失败")
                sys.exit(1)
        
        # 打印使用说明
        print("\n" + "=" * 60)
        print("📋 使用说明:")
        print("=" * 60)
        
        if web_viewer:
            print(f"🌐 Web界面: http://localhost:{args.port}")
            print("   - 在浏览器中打开上述地址")
            print("   - 配置接收参数并点击'开始接收'")
        
        if cli_receiver:
            print(f"📡 命令行接收器: {args.host}:{args.udp_port}")
            print("   - 按 Ctrl+C 停止接收")
            print("   - 按 'q' 键退出显示窗口")
        
        print("\n🔧 ESP32配置:")
        print("   - WiFi SSID: 309Study")
        print("   - WiFi 密码: ai12321")
        print("   - UDP端口: 8888")
        print("   - 数据格式: RGB565")
        
        print("\n⚡ 性能提示:")
        print("   - 确保ESP32和接收端在同一网络")
        print("   - 使用5GHz WiFi频段以获得更好性能")
        print("   - 启用GPU加速可显著降低延迟")
        
        print("\n按 Ctrl+C 停止系统")
        print("=" * 60)
        
        # 保持运行
        while True:
            time.sleep(1)
            
    except KeyboardInterrupt:
        print("\n\n🛑 正在停止系统...")
        
        # 停止接收器
        if cli_receiver:
            cli_receiver.stop()
            print("✅ 命令行接收器已停止")
        
        if web_viewer:
            print("✅ Web查看器已停止")
        
        print("👋 系统已退出")
        
    except Exception as e:
        print(f"\n❌ 系统错误: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()
