#!/usr/bin/env python3
"""
Jitsi Meet Qt Modular Build System
Jitsi Meet Qt 模块化构建系统

This is the main build system script that orchestrates the entire build process.
"""

import os
import sys
import json
import time
import argparse
import subprocess
from pathlib import Path
from typing import Dict, List, Optional

class JitsiModularBuildSystem:
    """Jitsi模块化构建系统"""
    
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.modules_dir = self.project_root / "modules"
        self.tools_dir = self.modules_dir / "tools"
        self.build_dir = self.project_root / "build"
        self.dist_dir = self.project_root / "dist"
        
        # 确保目录存在
        self.build_dir.mkdir(exist_ok=True)
        self.dist_dir.mkdir(exist_ok=True)
        
        # 构建配置
        self.build_config = self.load_build_config()
    
    def load_build_config(self) -> Dict:
        """加载构建配置"""
        config_file = self.modules_dir / "build_config.json"
        
        default_config = {
            "version": "2.1.0",
            "build_type": "release",
            "enable_optimizations": True,
            "enable_testing": True,
            "enable_packaging": True,
            "parallel_jobs": os.cpu_count() or 4,
            "modules": {
                "core": {"enabled": True, "required": True},
                "utils": {"enabled": True, "required": True},
                "settings": {"enabled": True, "required": False},
                "performance": {"enabled": True, "required": False},
                "camera": {"enabled": True, "required": False},
                "audio": {"enabled": True, "required": False},
                "network": {"enabled": True, "required": False},
                "ui": {"enabled": True, "required": False},
                "chat": {"enabled": True, "required": False},
                "screenshare": {"enabled": True, "required": False},
                "meeting": {"enabled": True, "required": False},
                "compatibility": {"enabled": True, "required": False}
            }
        }
        
        if config_file.exists():
            try:
                with open(config_file, 'r') as f:
                    user_config = json.load(f)
                # 深度合并配置
                self.deep_merge_config(default_config, user_config)
            except Exception as e:
                print(f"Warning: Failed to load build config: {e}")
        
        return default_config
    
    def deep_merge_config(self, base: Dict, update: Dict):
        """深度合并配置"""
        for key, value in update.items():
            if key in base and isinstance(base[key], dict) and isinstance(value, dict):
                self.deep_merge_config(base[key], value)
            else:
                base[key] = value
    
    def run_tool(self, tool_name: str, args: List[str]) -> bool:
        """运行工具脚本"""
        tool_path = self.tools_dir / f"{tool_name}.py"
        
        if not tool_path.exists():
            print(f"Tool not found: {tool_path}")
            return False
        
        try:
            cmd = [sys.executable, str(tool_path)] + args
            result = subprocess.run(cmd, check=True, cwd=self.project_root)
            return result.returncode == 0
        except subprocess.CalledProcessError as e:
            print(f"Tool {tool_name} failed with exit code {e.returncode}")
            return False
    
    def setup_build_environment(self) -> bool:
        """设置构建环境"""
        print("Setting up build environment...")
        
        # 运行构建优化设置
        if not self.run_tool("optimize_build", [str(self.project_root), "setup"]):
            print("Failed to setup build optimizations")
            return False
        
        # 创建必要的目录
        directories = [
            self.build_dir / "modules",
            self.build_dir / "plugins",
            self.build_dir / "packages",
            self.dist_dir / "packages",
            self.dist_dir / "installers"
        ]
        
        for directory in directories:
            directory.mkdir(parents=True, exist_ok=True)
        
        print("✓ Build environment setup complete")
        return True
    
    def configure_modules(self) -> bool:
        """配置模块"""
        print("Configuring modules...")
        
        # 生成优化的modules.pri文件
        optimized_pri = self.modules_dir / "modules_optimized.pri"
        target_pri = self.modules_dir / "modules.pri"
        
        if optimized_pri.exists():
            # 备份原始文件
            if target_pri.exists():
                backup_file = target_pri.with_suffix(".pri.backup")
                target_pri.rename(backup_file)
            
            # 使用优化版本
            import shutil
            shutil.copy2(optimized_pri, target_pri)
            print("✓ Using optimized modules configuration")
        
        # 设置环境变量
        disabled_modules = []
        for module_name, module_config in self.build_config["modules"].items():
            if not module_config["enabled"]:
                disabled_modules.append(module_name)
        
        if disabled_modules:
            os.environ["JITSI_DISABLE_MODULES"] = ",".join(disabled_modules)
            print(f"Disabled modules: {', '.join(disabled_modules)}")
        
        # 设置构建类型
        if self.build_config["build_type"] == "debug":
            os.environ["CONFIG"] = "debug"
        else:
            os.environ["CONFIG"] = "release"
        
        print("✓ Module configuration complete")
        return True
    
    def build_modules(self) -> bool:
        """构建模块"""
        print("Building modules...")
        
        if self.build_config["enable_optimizations"]:
            # 使用优化构建
            return self.run_tool("optimize_build", [str(self.project_root), "build"])
        else:
            # 标准构建
            return self.build_standard()
    
    def build_standard(self) -> bool:
        """标准构建过程"""
        try:
            # 运行qmake
            qmake_cmd = ["qmake", str(self.project_root / "jitsi-meet-qt.pro")]
            subprocess.run(qmake_cmd, cwd=self.build_dir, check=True)
            
            # 运行make
            make_cmd = ["make", f"-j{self.build_config['parallel_jobs']}"]
            subprocess.run(make_cmd, cwd=self.build_dir, check=True)
            
            print("✓ Standard build complete")
            return True
            
        except subprocess.CalledProcessError as e:
            print(f"Build failed: {e}")
            return False
    
    def run_tests(self) -> bool:
        """运行测试"""
        if not self.build_config["enable_testing"]:
            print("Testing disabled, skipping...")
            return True
        
        print("Running tests...")
        
        # 查找测试可执行文件
        test_files = list(self.build_dir.rglob("*test*"))
        test_executables = [f for f in test_files if f.is_file() and os.access(f, os.X_OK)]
        
        if not test_executables:
            print("No test executables found")
            return True
        
        failed_tests = 0
        for test_exe in test_executables:
            try:
                print(f"Running: {test_exe.name}")
                subprocess.run([str(test_exe)], check=True, cwd=test_exe.parent)
                print(f"✓ {test_exe.name} passed")
            except subprocess.CalledProcessError:
                print(f"✗ {test_exe.name} failed")
                failed_tests += 1
        
        if failed_tests > 0:
            print(f"✗ {failed_tests} tests failed")
            return False
        
        print("✓ All tests passed")
        return True
    
    def create_packages(self) -> bool:
        """创建包"""
        if not self.build_config["enable_packaging"]:
            print("Packaging disabled, skipping...")
            return True
        
        print("Creating packages...")
        
        # 运行打包工具
        package_dir = self.build_dir / "packages"
        
        # 这里会调用packaging.pri中定义的目标
        try:
            make_cmd = ["make", "package_all"]
            subprocess.run(make_cmd, cwd=self.build_dir, check=True)
            
            print("✓ Packages created successfully")
            return True
            
        except subprocess.CalledProcessError as e:
            print(f"Packaging failed: {e}")
            return False
    
    def verify_packages(self) -> bool:
        """验证包"""
        print("Verifying packages...")
        
        package_dir = self.build_dir / "packages"
        return self.run_tool("verify_packages", [str(package_dir)])
    
    def create_distribution(self) -> bool:
        """创建分发包"""
        print("Creating distribution...")
        
        try:
            make_cmd = ["make", "create_distribution"]
            subprocess.run(make_cmd, cwd=self.build_dir, check=True)
            
            print("✓ Distribution created successfully")
            return True
            
        except subprocess.CalledProcessError as e:
            print(f"Distribution creation failed: {e}")
            return False
    
    def generate_build_report(self) -> Dict:
        """生成构建报告"""
        report = {
            "build_info": {
                "version": self.build_config["version"],
                "build_type": self.build_config["build_type"],
                "timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
                "platform": sys.platform,
                "python_version": sys.version
            },
            "modules": self.build_config["modules"],
            "build_artifacts": {
                "executables": [],
                "libraries": [],
                "packages": []
            }
        }
        
        # 查找构建产物
        if self.build_dir.exists():
            # 可执行文件
            for exe_file in self.build_dir.rglob("*.exe"):
                report["build_artifacts"]["executables"].append(str(exe_file.relative_to(self.build_dir)))
            
            # 库文件
            for lib_pattern in ["*.so", "*.dll", "*.dylib"]:
                for lib_file in self.build_dir.rglob(lib_pattern):
                    report["build_artifacts"]["libraries"].append(str(lib_file.relative_to(self.build_dir)))
            
            # 包文件
            for pkg_file in self.build_dir.rglob("*.tar.gz"):
                report["build_artifacts"]["packages"].append(str(pkg_file.relative_to(self.build_dir)))
        
        return report
    
    def full_build(self) -> bool:
        """完整构建流程"""
        print("Starting full build process...")
        start_time = time.time()
        
        steps = [
            ("Setup Environment", self.setup_build_environment),
            ("Configure Modules", self.configure_modules),
            ("Build Modules", self.build_modules),
            ("Run Tests", self.run_tests),
            ("Create Packages", self.create_packages),
            ("Verify Packages", self.verify_packages),
            ("Create Distribution", self.create_distribution)
        ]
        
        for step_name, step_func in steps:
            print(f"\n{'='*50}")
            print(f"Step: {step_name}")
            print('='*50)
            
            if not step_func():
                print(f"✗ Build failed at step: {step_name}")
                return False
        
        end_time = time.time()
        build_time = end_time - start_time
        
        print(f"\n{'='*50}")
        print("BUILD COMPLETED SUCCESSFULLY")
        print('='*50)
        print(f"Total build time: {build_time:.2f} seconds")
        
        # 生成构建报告
        report = self.generate_build_report()
        report_file = self.build_dir / "build_report.json"
        with open(report_file, 'w') as f:
            json.dump(report, f, indent=2)
        
        print(f"Build report saved to: {report_file}")
        return True
    
    def clean_build(self):
        """清理构建"""
        print("Cleaning build...")
        
        if self.build_dir.exists():
            import shutil
            shutil.rmtree(self.build_dir)
            print("✓ Build directory cleaned")
        
        # 清理构建缓存
        self.run_tool("optimize_build", [str(self.project_root), "clean"])

def main():
    parser = argparse.ArgumentParser(description="Jitsi Meet Qt Modular Build System")
    parser.add_argument("project_root", help="Path to project root directory")
    
    subparsers = parser.add_subparsers(dest="command", help="Available commands")
    
    # 构建命令
    build_parser = subparsers.add_parser("build", help="Full build process")
    build_parser.add_argument("--config", help="Build configuration file")
    
    # 清理命令
    clean_parser = subparsers.add_parser("clean", help="Clean build artifacts")
    
    # 测试命令
    test_parser = subparsers.add_parser("test", help="Run tests only")
    
    # 打包命令
    package_parser = subparsers.add_parser("package", help="Create packages only")
    
    # 报告命令
    report_parser = subparsers.add_parser("report", help="Generate build report")
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return 1
    
    build_system = JitsiModularBuildSystem(args.project_root)
    
    if args.command == "build":
        success = build_system.full_build()
        return 0 if success else 1
    
    elif args.command == "clean":
        build_system.clean_build()
        return 0
    
    elif args.command == "test":
        success = build_system.run_tests()
        return 0 if success else 1
    
    elif args.command == "package":
        success = build_system.create_packages()
        return 0 if success else 1
    
    elif args.command == "report":
        report = build_system.generate_build_report()
        print(json.dumps(report, indent=2))
        return 0
    
    return 0

if __name__ == "__main__":
    sys.exit(main())