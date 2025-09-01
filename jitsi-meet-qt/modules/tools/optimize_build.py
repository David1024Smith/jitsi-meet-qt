#!/usr/bin/env python3
"""
Build Optimization Tool
构建优化工具

This tool optimizes the build process for Jitsi Meet Qt modules.
"""

import os
import sys
import json
import time
import shutil
import subprocess
from pathlib import Path
from typing import Dict, List, Optional

class BuildOptimizer:
    """构建优化器"""
    
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.modules_dir = self.project_root / "modules"
        self.build_cache_dir = self.project_root / ".build_cache"
        self.optimization_config = self.load_optimization_config()
        
        # 创建缓存目录
        self.build_cache_dir.mkdir(exist_ok=True)
    
    def load_optimization_config(self) -> Dict:
        """加载优化配置"""
        config_file = self.modules_dir / "build_optimization.json"
        
        default_config = {
            "parallel_jobs": os.cpu_count() or 4,
            "use_ccache": True,
            "use_precompiled_headers": True,
            "incremental_build": True,
            "optimize_level": "O2",
            "link_time_optimization": False,
            "strip_debug_symbols": False,
            "compress_resources": True,
            "module_build_order": [
                "utils", "settings", "performance", "core",
                "camera", "audio", "network", "screenshare",
                "chat", "meeting", "ui"
            ]
        }
        
        if config_file.exists():
            try:
                with open(config_file, 'r') as f:
                    user_config = json.load(f)
                default_config.update(user_config)
            except Exception as e:
                print(f"Warning: Failed to load optimization config: {e}")
        
        return default_config
    
    def setup_ccache(self) -> bool:
        """设置ccache"""
        if not self.optimization_config["use_ccache"]:
            return False
        
        # 检查ccache是否可用
        try:
            subprocess.run(["ccache", "--version"], 
                         capture_output=True, check=True)
            
            # 配置ccache
            ccache_dir = self.build_cache_dir / "ccache"
            ccache_dir.mkdir(exist_ok=True)
            
            os.environ["CCACHE_DIR"] = str(ccache_dir)
            os.environ["CCACHE_MAXSIZE"] = "2G"
            os.environ["CCACHE_COMPRESS"] = "1"
            
            print("✓ ccache configured")
            return True
            
        except (subprocess.CalledProcessError, FileNotFoundError):
            print("Warning: ccache not available")
            return False
    
    def generate_precompiled_headers(self) -> bool:
        """生成预编译头文件"""
        if not self.optimization_config["use_precompiled_headers"]:
            return False
        
        pch_dir = self.modules_dir / "pch"
        pch_dir.mkdir(exist_ok=True)
        
        # 创建预编译头文件
        pch_content = """
// Precompiled Headers for Jitsi Meet Qt Modules
#pragma once

// Qt Headers
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtNetwork>
#include <QtMultimedia>

// Standard Library
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>

// Common Module Headers
#ifdef UTILS_MODULE_AVAILABLE
#include "utils/include/Logger.h"
#include "utils/include/FileManager.h"
#endif

#ifdef SETTINGS_MODULE_AVAILABLE
#include "settings/include/SettingsManager.h"
#endif

#ifdef PERFORMANCE_MODULE_AVAILABLE
#include "performance/include/PerformanceManager.h"
#endif
"""
        
        pch_file = pch_dir / "modules_pch.h"
        pch_file.write_text(pch_content)
        
        print("✓ Precompiled headers generated")
        return True
    
    def optimize_qmake_files(self):
        """优化qmake文件"""
        # 查找所有.pri文件
        pri_files = list(self.modules_dir.rglob("*.pri"))
        
        for pri_file in pri_files:
            if pri_file.name == "modules.pri":
                continue  # 跳过主模块文件
            
            self.optimize_single_pri_file(pri_file)
    
    def optimize_single_pri_file(self, pri_file: Path):
        """优化单个.pri文件"""
        content = pri_file.read_text()
        
        # 添加优化选项
        optimizations = []
        
        if self.optimization_config["use_ccache"]:
            optimizations.append("# ccache support")
            optimizations.append("exists(/usr/bin/ccache) {")
            optimizations.append("    QMAKE_CXX = ccache g++")
            optimizations.append("}")
        
        if self.optimization_config["use_precompiled_headers"]:
            optimizations.append("# Precompiled headers")
            optimizations.append("CONFIG += precompile_header")
            optimizations.append("PRECOMPILED_HEADER = $$PWD/../pch/modules_pch.h")
        
        if self.optimization_config["parallel_jobs"] > 1:
            optimizations.append("# Parallel compilation")
            optimizations.append(f"QMAKE_CXXFLAGS += -j{self.optimization_config['parallel_jobs']}")
        
        # 优化级别
        opt_level = self.optimization_config["optimize_level"]
        optimizations.append(f"# Optimization level")
        optimizations.append(f"QMAKE_CXXFLAGS_RELEASE += -{opt_level}")
        
        if self.optimization_config["link_time_optimization"]:
            optimizations.append("# Link time optimization")
            optimizations.append("QMAKE_CXXFLAGS += -flto")
            optimizations.append("QMAKE_LFLAGS += -flto")
        
        # 检查是否已经包含优化选项
        if "# Build optimizations" not in content:
            optimization_block = "\n# Build optimizations\n" + "\n".join(optimizations) + "\n"
            content += optimization_block
            
            pri_file.write_text(content)
            print(f"✓ Optimized: {pri_file.name}")
    
    def create_dependency_graph(self) -> Dict[str, List[str]]:
        """创建模块依赖图"""
        dependencies = {
            "utils": [],
            "settings": ["utils"],
            "performance": ["utils"],
            "core": ["utils", "settings"],
            "camera": ["utils"],
            "audio": ["utils"],
            "network": ["utils", "settings"],
            "screenshare": ["utils"],
            "chat": ["network", "utils"],
            "meeting": ["network", "utils"],
            "ui": ["settings"],

        }
        
        return dependencies
    
    def calculate_build_order(self) -> List[str]:
        """计算最优构建顺序"""
        dependencies = self.create_dependency_graph()
        build_order = []
        built_modules = set()
        
        def can_build(module: str) -> bool:
            return all(dep in built_modules for dep in dependencies.get(module, []))
        
        remaining_modules = set(dependencies.keys())
        
        while remaining_modules:
            # 找到可以构建的模块
            buildable = [m for m in remaining_modules if can_build(m)]
            
            if not buildable:
                # 如果没有可构建的模块，可能存在循环依赖
                print("Warning: Possible circular dependency detected")
                buildable = list(remaining_modules)[:1]  # 强制构建一个
            
            # 按优先级排序（如果配置中有指定）
            config_order = self.optimization_config.get("module_build_order", [])
            buildable.sort(key=lambda x: config_order.index(x) if x in config_order else 999)
            
            # 构建第一个可用模块
            module = buildable[0]
            build_order.append(module)
            built_modules.add(module)
            remaining_modules.remove(module)
        
        return build_order
    
    def build_module(self, module_name: str) -> bool:
        """构建单个模块"""
        module_dir = self.modules_dir / module_name
        
        if not module_dir.exists():
            print(f"Module directory not found: {module_dir}")
            return False
        
        pri_file = module_dir / f"{module_name}.pri"
        if not pri_file.exists():
            print(f"Module configuration not found: {pri_file}")
            return False
        
        print(f"Building module: {module_name}")
        
        # 创建构建目录
        build_dir = self.build_cache_dir / f"build_{module_name}"
        build_dir.mkdir(exist_ok=True)
        
        try:
            # 运行qmake
            qmake_cmd = ["qmake", str(pri_file), f"-o", str(build_dir / "Makefile")]
            subprocess.run(qmake_cmd, cwd=build_dir, check=True, 
                         capture_output=True, text=True)
            
            # 运行make
            make_cmd = ["make", f"-j{self.optimization_config['parallel_jobs']}"]
            result = subprocess.run(make_cmd, cwd=build_dir, check=True,
                                  capture_output=True, text=True)
            
            print(f"✓ Module {module_name} built successfully")
            return True
            
        except subprocess.CalledProcessError as e:
            print(f"✗ Failed to build module {module_name}: {e}")
            if e.stdout:
                print("STDOUT:", e.stdout)
            if e.stderr:
                print("STDERR:", e.stderr)
            return False
    
    def build_all_modules(self) -> bool:
        """构建所有模块"""
        build_order = self.calculate_build_order()
        
        print(f"Build order: {' -> '.join(build_order)}")
        
        start_time = time.time()
        successful_builds = 0
        
        for module_name in build_order:
            if self.build_module(module_name):
                successful_builds += 1
            else:
                print(f"Stopping build due to failure in module: {module_name}")
                break
        
        end_time = time.time()
        build_time = end_time - start_time
        
        print(f"\nBuild completed in {build_time:.2f} seconds")
        print(f"Successfully built {successful_builds}/{len(build_order)} modules")
        
        return successful_builds == len(build_order)
    
    def clean_build_cache(self):
        """清理构建缓存"""
        if self.build_cache_dir.exists():
            shutil.rmtree(self.build_cache_dir)
            print("✓ Build cache cleaned")
    
    def generate_build_report(self) -> Dict:
        """生成构建报告"""
        report = {
            "optimization_config": self.optimization_config,
            "build_order": self.calculate_build_order(),
            "available_modules": [d.name for d in self.modules_dir.iterdir() 
                                if d.is_dir() and not d.name.startswith('.')],
            "ccache_available": shutil.which("ccache") is not None,
            "parallel_jobs": self.optimization_config["parallel_jobs"]
        }
        
        return report

def main():
    if len(sys.argv) < 2:
        print("Usage: optimize_build.py <project_root> [command]")
        print("Commands:")
        print("  setup     - Setup build optimizations")
        print("  build     - Build all modules with optimizations")
        print("  clean     - Clean build cache")
        print("  report    - Generate build report")
        return 1
    
    project_root = sys.argv[1]
    command = sys.argv[2] if len(sys.argv) > 2 else "setup"
    
    optimizer = BuildOptimizer(project_root)
    
    if command == "setup":
        print("Setting up build optimizations...")
        optimizer.setup_ccache()
        optimizer.generate_precompiled_headers()
        optimizer.optimize_qmake_files()
        print("✓ Build optimization setup complete")
        return 0
    
    elif command == "build":
        print("Building all modules with optimizations...")
        optimizer.setup_ccache()
        success = optimizer.build_all_modules()
        return 0 if success else 1
    
    elif command == "clean":
        optimizer.clean_build_cache()
        return 0
    
    elif command == "report":
        report = optimizer.generate_build_report()
        print(json.dumps(report, indent=2))
        return 0
    
    else:
        print(f"Unknown command: {command}")
        return 1

if __name__ == "__main__":
    sys.exit(main())