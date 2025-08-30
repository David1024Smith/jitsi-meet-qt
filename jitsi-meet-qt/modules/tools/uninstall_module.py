#!/usr/bin/env python3
"""
Module Uninstallation Tool
模块卸载工具

This tool handles the uninstallation of individual Jitsi Meet Qt modules.
"""

import os
import sys
import json
import shutil
import argparse
import subprocess
from pathlib import Path
from typing import Dict, List, Optional

class ModuleUninstaller:
    """模块卸载器"""
    
    def __init__(self, install_prefix: str = "/usr/local"):
        self.install_prefix = Path(install_prefix)
        self.module_registry = self.install_prefix / "share" / "jitsi-qt" / "modules" / "registry.json"
        self.plugin_dir = self.install_prefix / "lib" / "jitsi-qt" / "plugins"
    
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
    
    def check_dependencies(self, module_name: str) -> List[str]:
        """检查哪些模块依赖于要卸载的模块"""
        registry = self.load_registry()
        dependent_modules = []
        
        # 这里可以添加更复杂的依赖检查逻辑
        # 目前简单返回空列表
        return dependent_modules
    
    def run_pre_uninstall_scripts(self, module_name: str):
        """运行卸载前脚本"""
        script_path = self.install_prefix / "share" / "jitsi-qt" / "scripts" / f"{module_name}_pre_uninstall.py"
        
        if script_path.exists():
            print(f"Running pre-uninstall script for {module_name}")
            try:
                subprocess.run([sys.executable, str(script_path)], check=True)
                print("Pre-uninstall script completed successfully")
            except subprocess.CalledProcessError as e:
                print(f"Warning: Pre-uninstall script failed: {e}")
    
    def remove_module_files(self, module_name: str, module_info: Dict):
        """删除模块文件"""
        print(f"Removing files for module: {module_name}")
        
        files_removed = 0
        
        # 删除头文件
        header_dir = self.install_prefix / "include" / "jitsi-qt" / module_name
        if header_dir.exists():
            shutil.rmtree(header_dir)
            print(f"  Removed header directory: {header_dir}")
            files_removed += 1
        
        # 删除库文件
        lib_dir = self.install_prefix / "lib"
        for lib_pattern in [f"lib{module_name}*.so*", f"lib*{module_name}*.so*"]:
            for lib_file in lib_dir.glob(lib_pattern):
                lib_file.unlink()
                print(f"  Removed library: {lib_file.name}")
                files_removed += 1
        
        # 删除配置文件
        config_file = self.install_prefix / "share" / "jitsi-qt" / "modules" / f"{module_name}.pri"
        if config_file.exists():
            config_file.unlink()
            print(f"  Removed config: {config_file.name}")
            files_removed += 1
        
        # 删除资源文件
        resource_dir = self.install_prefix / "share" / "jitsi-qt" / "resources" / module_name
        if resource_dir.exists():
            shutil.rmtree(resource_dir)
            print(f"  Removed resource directory: {resource_dir}")
            files_removed += 1
        
        # 删除插件文件
        for plugin_file in self.plugin_dir.glob(f"*{module_name}*"):
            plugin_file.unlink()
            print(f"  Removed plugin: {plugin_file.name}")
            files_removed += 1
        
        return files_removed
    
    def update_registry(self, module_name: str):
        """从注册表中移除模块"""
        registry = self.load_registry()
        if module_name in registry["modules"]:
            del registry["modules"][module_name]
            self.save_registry(registry)
            print(f"Removed {module_name} from module registry")
    
    def cleanup_empty_directories(self):
        """清理空目录"""
        dirs_to_check = [
            self.install_prefix / "include" / "jitsi-qt",
            self.install_prefix / "share" / "jitsi-qt" / "resources",
            self.install_prefix / "share" / "jitsi-qt" / "modules",
            self.plugin_dir
        ]
        
        for dir_path in dirs_to_check:
            if dir_path.exists() and not any(dir_path.iterdir()):
                try:
                    dir_path.rmdir()
                    print(f"  Removed empty directory: {dir_path}")
                except OSError:
                    pass  # 目录不为空或无法删除
    
    def uninstall_module(self, module_name: str, force: bool = False) -> bool:
        """卸载模块"""
        try:
            # 检查模块是否已安装
            registry = self.load_registry()
            if module_name not in registry["modules"]:
                print(f"Module {module_name} is not installed")
                return False
            
            module_info = registry["modules"][module_name]
            
            # 检查依赖
            dependent_modules = self.check_dependencies(module_name)
            if dependent_modules and not force:
                print(f"Cannot uninstall {module_name}. The following modules depend on it:")
                for dep in dependent_modules:
                    print(f"  - {dep}")
                print("Use --force to uninstall anyway (may break dependent modules)")
                return False
            
            # 运行卸载前脚本
            self.run_pre_uninstall_scripts(module_name)
            
            # 删除文件
            files_removed = self.remove_module_files(module_name, module_info)
            
            # 更新注册表
            self.update_registry(module_name)
            
            # 清理空目录
            self.cleanup_empty_directories()
            
            print(f"Module {module_name} uninstalled successfully!")
            print(f"Removed {files_removed} files/directories")
            return True
            
        except Exception as e:
            print(f"Uninstallation failed: {e}")
            return False
    
    def uninstall_all_modules(self, force: bool = False) -> bool:
        """卸载所有模块"""
        registry = self.load_registry()
        
        if not registry["modules"]:
            print("No modules installed")
            return True
        
        modules_to_uninstall = list(registry["modules"].keys())
        
        if not force:
            print("This will uninstall all modules:")
            for module in modules_to_uninstall:
                print(f"  - {module}")
            
            response = input("Are you sure? (y/N): ")
            if response.lower() != 'y':
                print("Uninstallation cancelled")
                return False
        
        success_count = 0
        for module_name in modules_to_uninstall:
            print(f"\nUninstalling {module_name}...")
            if self.uninstall_module(module_name, force=True):
                success_count += 1
        
        print(f"\nUninstalled {success_count}/{len(modules_to_uninstall)} modules")
        return success_count == len(modules_to_uninstall)
    
    def list_installed_modules(self):
        """列出已安装的模块"""
        registry = self.load_registry()
        
        if not registry["modules"]:
            print("No modules installed.")
            return
        
        print("Installed modules:")
        for name, info in registry["modules"].items():
            print(f"  - {name} (version: {info['version']})")
    
    def verify_uninstallation(self, module_name: str) -> bool:
        """验证模块是否完全卸载"""
        registry = self.load_registry()
        
        # 检查注册表
        if module_name in registry["modules"]:
            print(f"Warning: {module_name} still in registry")
            return False
        
        # 检查文件系统
        remaining_files = []
        
        # 检查头文件
        header_dir = self.install_prefix / "include" / "jitsi-qt" / module_name
        if header_dir.exists():
            remaining_files.append(str(header_dir))
        
        # 检查库文件
        lib_dir = self.install_prefix / "lib"
        for lib_file in lib_dir.glob(f"*{module_name}*"):
            remaining_files.append(str(lib_file))
        
        # 检查配置文件
        config_file = self.install_prefix / "share" / "jitsi-qt" / "modules" / f"{module_name}.pri"
        if config_file.exists():
            remaining_files.append(str(config_file))
        
        # 检查资源文件
        resource_dir = self.install_prefix / "share" / "jitsi-qt" / "resources" / module_name
        if resource_dir.exists():
            remaining_files.append(str(resource_dir))
        
        if remaining_files:
            print(f"Warning: Some files for {module_name} still exist:")
            for file_path in remaining_files:
                print(f"  - {file_path}")
            return False
        
        print(f"Module {module_name} completely uninstalled")
        return True

def main():
    parser = argparse.ArgumentParser(description="Jitsi Meet Qt Module Uninstaller")
    parser.add_argument("--prefix", default="/usr/local", 
                       help="Installation prefix (default: /usr/local)")
    
    subparsers = parser.add_subparsers(dest="command", help="Available commands")
    
    # 卸载命令
    uninstall_parser = subparsers.add_parser("uninstall", help="Uninstall a module")
    uninstall_parser.add_argument("module", help="Module name to uninstall")
    uninstall_parser.add_argument("--force", action="store_true", 
                                 help="Force uninstallation (ignore dependencies)")
    
    # 卸载所有命令
    uninstall_all_parser = subparsers.add_parser("uninstall-all", 
                                                 help="Uninstall all modules")
    uninstall_all_parser.add_argument("--force", action="store_true", 
                                     help="Force uninstallation without confirmation")
    
    # 列表命令
    list_parser = subparsers.add_parser("list", help="List installed modules")
    
    # 验证命令
    verify_parser = subparsers.add_parser("verify", help="Verify module uninstallation")
    verify_parser.add_argument("module", help="Module name to verify")
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return 1
    
    uninstaller = ModuleUninstaller(args.prefix)
    
    if args.command == "uninstall":
        success = uninstaller.uninstall_module(args.module, args.force)
        return 0 if success else 1
    
    elif args.command == "uninstall-all":
        success = uninstaller.uninstall_all_modules(args.force)
        return 0 if success else 1
    
    elif args.command == "list":
        uninstaller.list_installed_modules()
        return 0
    
    elif args.command == "verify":
        success = uninstaller.verify_uninstallation(args.module)
        return 0 if success else 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())