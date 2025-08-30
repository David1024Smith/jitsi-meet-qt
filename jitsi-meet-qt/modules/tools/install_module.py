#!/usr/bin/env python3
"""
Module Installation Tool
模块安装工具

This tool handles the installation of individual Jitsi Meet Qt modules.
"""

import os
import sys
import json
import shutil
import tarfile
import argparse
import subprocess
from pathlib import Path
from typing import Dict, List, Optional

class ModuleInstaller:
    """模块安装器"""
    
    def __init__(self, install_prefix: str = "/usr/local"):
        self.install_prefix = Path(install_prefix)
        self.module_registry = self.install_prefix / "share" / "jitsi-qt" / "modules" / "registry.json"
        self.plugin_dir = self.install_prefix / "lib" / "jitsi-qt" / "plugins"
        
        # 确保目录存在
        self.module_registry.parent.mkdir(parents=True, exist_ok=True)
        self.plugin_dir.mkdir(parents=True, exist_ok=True)
    
    def load_registry(self) -> Dict:
        """加载模块注册表"""
        if self.module_registry.exists():
            with open(self.module_registry, 'r') as f:
                return json.load(f)
        return {"modules": {}, "version": "1.0.0"}
    
    def save_registry(self, registry: Dict):
        """保存模块注册表"""
        with open(self.module_registry, 'w') as f:
            json.dump(registry, f, indent=2)
    
    def extract_package(self, package_path: str, temp_dir: str) -> str:
        """解压模块包"""
        package_path = Path(package_path)
        temp_dir = Path(temp_dir)
        
        if not package_path.exists():
            raise FileNotFoundError(f"Package not found: {package_path}")
        
        print(f"Extracting package: {package_path}")
        
        with tarfile.open(package_path, 'r:gz') as tar:
            tar.extractall(temp_dir)
        
        # 查找解压后的目录
        extracted_dirs = [d for d in temp_dir.iterdir() if d.is_dir()]
        if not extracted_dirs:
            raise RuntimeError("No directories found in extracted package")
        
        return str(extracted_dirs[0])
    
    def validate_module(self, module_dir: str) -> Dict:
        """验证模块包"""
        module_dir = Path(module_dir)
        
        # 查找模块配置文件
        pri_files = list(module_dir.glob("*.pri"))
        if not pri_files:
            raise RuntimeError("No .pri configuration file found")
        
        module_name = pri_files[0].stem
        
        # 检查必需文件
        required_files = {
            "config": pri_files[0],
            "headers": list(module_dir.glob("include/*.h")),
            "libraries": list(module_dir.glob("lib*.so*"))
        }
        
        if not required_files["headers"]:
            print("Warning: No header files found")
        
        if not required_files["libraries"]:
            print("Warning: No library files found")
        
        # 读取模块信息
        module_info = {
            "name": module_name,
            "version": "1.0.0",  # 默认版本
            "files": {
                "config": str(required_files["config"]),
                "headers": [str(h) for h in required_files["headers"]],
                "libraries": [str(l) for l in required_files["libraries"]]
            }
        }
        
        return module_info
    
    def check_dependencies(self, module_info: Dict) -> bool:
        """检查模块依赖"""
        registry = self.load_registry()
        installed_modules = set(registry["modules"].keys())
        
        # 这里可以添加更复杂的依赖检查逻辑
        # 目前简单返回True
        return True
    
    def install_files(self, module_dir: str, module_info: Dict):
        """安装模块文件"""
        module_dir = Path(module_dir)
        module_name = module_info["name"]
        
        print(f"Installing module: {module_name}")
        
        # 安装头文件
        if module_info["files"]["headers"]:
            header_dest = self.install_prefix / "include" / "jitsi-qt" / module_name
            header_dest.mkdir(parents=True, exist_ok=True)
            
            for header_path in module_info["files"]["headers"]:
                header_file = Path(header_path)
                if header_file.exists():
                    shutil.copy2(header_file, header_dest)
                    print(f"  Installed header: {header_file.name}")
        
        # 安装库文件
        if module_info["files"]["libraries"]:
            lib_dest = self.install_prefix / "lib"
            lib_dest.mkdir(parents=True, exist_ok=True)
            
            for lib_path in module_info["files"]["libraries"]:
                lib_file = Path(lib_path)
                if lib_file.exists():
                    shutil.copy2(lib_file, lib_dest)
                    print(f"  Installed library: {lib_file.name}")
        
        # 安装配置文件
        config_dest = self.install_prefix / "share" / "jitsi-qt" / "modules"
        config_dest.mkdir(parents=True, exist_ok=True)
        
        config_file = Path(module_info["files"]["config"])
        if config_file.exists():
            shutil.copy2(config_file, config_dest)
            print(f"  Installed config: {config_file.name}")
        
        # 安装资源文件（如果存在）
        resources_dir = module_dir / "resources"
        if resources_dir.exists():
            resource_dest = self.install_prefix / "share" / "jitsi-qt" / "resources" / module_name
            resource_dest.mkdir(parents=True, exist_ok=True)
            
            for resource_file in resources_dir.rglob("*"):
                if resource_file.is_file():
                    rel_path = resource_file.relative_to(resources_dir)
                    dest_file = resource_dest / rel_path
                    dest_file.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(resource_file, dest_file)
                    print(f"  Installed resource: {rel_path}")
    
    def update_registry(self, module_info: Dict):
        """更新模块注册表"""
        registry = self.load_registry()
        registry["modules"][module_info["name"]] = {
            "version": module_info["version"],
            "installed_date": str(Path().cwd()),  # 简化的时间戳
            "files": module_info["files"]
        }
        self.save_registry(registry)
        print(f"Updated module registry for: {module_info['name']}")
    
    def run_post_install_scripts(self, module_dir: str, module_name: str):
        """运行安装后脚本"""
        module_dir = Path(module_dir)
        post_install_script = module_dir / "scripts" / "post_install.py"
        
        if post_install_script.exists():
            print(f"Running post-install script for {module_name}")
            try:
                subprocess.run([sys.executable, str(post_install_script)], 
                             check=True, cwd=module_dir)
                print("Post-install script completed successfully")
            except subprocess.CalledProcessError as e:
                print(f"Warning: Post-install script failed: {e}")
    
    def install_module(self, package_path: str, force: bool = False) -> bool:
        """安装模块"""
        import tempfile
        
        try:
            with tempfile.TemporaryDirectory() as temp_dir:
                # 解压包
                module_dir = self.extract_package(package_path, temp_dir)
                
                # 验证模块
                module_info = self.validate_module(module_dir)
                module_name = module_info["name"]
                
                # 检查是否已安装
                registry = self.load_registry()
                if module_name in registry["modules"] and not force:
                    print(f"Module {module_name} is already installed. Use --force to reinstall.")
                    return False
                
                # 检查依赖
                if not self.check_dependencies(module_info):
                    print(f"Dependency check failed for module: {module_name}")
                    return False
                
                # 安装文件
                self.install_files(module_dir, module_info)
                
                # 更新注册表
                self.update_registry(module_info)
                
                # 运行安装后脚本
                self.run_post_install_scripts(module_dir, module_name)
                
                print(f"Module {module_name} installed successfully!")
                return True
                
        except Exception as e:
            print(f"Installation failed: {e}")
            return False
    
    def list_installed_modules(self):
        """列出已安装的模块"""
        registry = self.load_registry()
        
        if not registry["modules"]:
            print("No modules installed.")
            return
        
        print("Installed modules:")
        for name, info in registry["modules"].items():
            print(f"  - {name} (version: {info['version']})")
    
    def get_module_info(self, module_name: str) -> Optional[Dict]:
        """获取模块信息"""
        registry = self.load_registry()
        return registry["modules"].get(module_name)

def main():
    parser = argparse.ArgumentParser(description="Jitsi Meet Qt Module Installer")
    parser.add_argument("--prefix", default="/usr/local", 
                       help="Installation prefix (default: /usr/local)")
    
    subparsers = parser.add_subparsers(dest="command", help="Available commands")
    
    # 安装命令
    install_parser = subparsers.add_parser("install", help="Install a module")
    install_parser.add_argument("package", help="Path to module package")
    install_parser.add_argument("--force", action="store_true", 
                               help="Force reinstallation")
    
    # 列表命令
    list_parser = subparsers.add_parser("list", help="List installed modules")
    
    # 信息命令
    info_parser = subparsers.add_parser("info", help="Show module information")
    info_parser.add_argument("module", help="Module name")
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return 1
    
    installer = ModuleInstaller(args.prefix)
    
    if args.command == "install":
        success = installer.install_module(args.package, args.force)
        return 0 if success else 1
    
    elif args.command == "list":
        installer.list_installed_modules()
        return 0
    
    elif args.command == "info":
        info = installer.get_module_info(args.module)
        if info:
            print(f"Module: {args.module}")
            print(f"Version: {info['version']}")
            print(f"Installed: {info['installed_date']}")
        else:
            print(f"Module {args.module} not found")
            return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())