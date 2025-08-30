#!/usr/bin/env python3
"""
Plugin Metadata Generator
插件元数据生成器

This tool generates metadata for dynamically loaded module plugins.
"""

import os
import sys
import json
import hashlib
from pathlib import Path
from typing import Dict, List

def calculate_file_hash(file_path: Path) -> str:
    """计算文件哈希值"""
    sha256_hash = hashlib.sha256()
    with open(file_path, "rb") as f:
        for byte_block in iter(lambda: f.read(4096), b""):
            sha256_hash.update(byte_block)
    return sha256_hash.hexdigest()

def get_plugin_info(plugin_path: Path) -> Dict:
    """获取插件信息"""
    plugin_info = {
        "name": plugin_path.stem,
        "path": str(plugin_path),
        "size": plugin_path.stat().st_size,
        "hash": calculate_file_hash(plugin_path),
        "version": "1.0.0",  # 默认版本
        "dependencies": [],
        "interfaces": []
    }
    
    # 尝试从文件名推断模块类型
    if "audio" in plugin_path.name.lower():
        plugin_info["type"] = "audio"
        plugin_info["interfaces"] = ["IAudioDevice", "IAudioManager"]
    elif "network" in plugin_path.name.lower():
        plugin_info["type"] = "network"
        plugin_info["interfaces"] = ["INetworkManager", "IConnectionHandler"]
    elif "ui" in plugin_path.name.lower():
        plugin_info["type"] = "ui"
        plugin_info["interfaces"] = ["IUIManager", "IThemeManager"]
    elif "chat" in plugin_path.name.lower():
        plugin_info["type"] = "chat"
        plugin_info["interfaces"] = ["IChatManager", "IMessageHandler"]
    elif "camera" in plugin_path.name.lower():
        plugin_info["type"] = "camera"
        plugin_info["interfaces"] = ["ICameraDevice", "ICameraManager"]
    elif "performance" in plugin_path.name.lower():
        plugin_info["type"] = "performance"
        plugin_info["interfaces"] = ["IPerformanceMonitor", "IResourceTracker"]
    elif "screenshare" in plugin_path.name.lower():
        plugin_info["type"] = "screenshare"
        plugin_info["interfaces"] = ["IScreenCapture", "IScreenShareManager"]
    elif "meeting" in plugin_path.name.lower():
        plugin_info["type"] = "meeting"
        plugin_info["interfaces"] = ["IMeetingManager", "ILinkHandler"]
    elif "settings" in plugin_path.name.lower():
        plugin_info["type"] = "settings"
        plugin_info["interfaces"] = ["ISettingsManager", "IPreferencesHandler"]
    elif "utils" in plugin_path.name.lower():
        plugin_info["type"] = "utils"
        plugin_info["interfaces"] = ["ILogger", "IFileHandler"]
    else:
        plugin_info["type"] = "unknown"
    
    return plugin_info

def generate_metadata(plugin_dir: str, output_file: str):
    """生成插件元数据"""
    plugin_dir = Path(plugin_dir)
    output_file = Path(output_file)
    
    if not plugin_dir.exists():
        print(f"Plugin directory not found: {plugin_dir}")
        return False
    
    # 查找所有插件文件
    plugin_files = []
    for ext in ["*.so", "*.dll", "*.dylib"]:
        plugin_files.extend(plugin_dir.glob(ext))
    
    if not plugin_files:
        print(f"No plugin files found in: {plugin_dir}")
        return False
    
    # 生成元数据
    metadata = {
        "version": "1.0.0",
        "generated_by": "generate_plugin_metadata.py",
        "plugin_directory": str(plugin_dir),
        "total_plugins": len(plugin_files),
        "plugins": {}
    }
    
    print(f"Found {len(plugin_files)} plugin files")
    
    for plugin_file in plugin_files:
        print(f"Processing: {plugin_file.name}")
        plugin_info = get_plugin_info(plugin_file)
        metadata["plugins"][plugin_info["name"]] = plugin_info
    
    # 添加依赖关系信息
    add_dependency_info(metadata)
    
    # 保存元数据
    output_file.parent.mkdir(parents=True, exist_ok=True)
    with open(output_file, 'w') as f:
        json.dump(metadata, f, indent=2)
    
    print(f"Plugin metadata generated: {output_file}")
    return True

def add_dependency_info(metadata: Dict):
    """添加模块依赖关系信息"""
    # 定义模块依赖关系
    dependencies = {
        "audio": ["utils"],
        "network": ["utils", "settings"],
        "ui": ["settings"],
        "chat": ["network", "utils"],
        "meeting": ["network", "utils"],
        "screenshare": ["utils"],
        "performance": ["utils"],
        "camera": ["utils"]
    }
    
    # 更新插件依赖信息
    for plugin_name, plugin_info in metadata["plugins"].items():
        plugin_type = plugin_info.get("type", "unknown")
        if plugin_type in dependencies:
            plugin_info["dependencies"] = dependencies[plugin_type]

def validate_metadata(metadata_file: str) -> bool:
    """验证元数据文件"""
    metadata_file = Path(metadata_file)
    
    if not metadata_file.exists():
        print(f"Metadata file not found: {metadata_file}")
        return False
    
    try:
        with open(metadata_file, 'r') as f:
            metadata = json.load(f)
        
        # 验证必需字段
        required_fields = ["version", "plugin_directory", "total_plugins", "plugins"]
        for field in required_fields:
            if field not in metadata:
                print(f"Missing required field: {field}")
                return False
        
        # 验证插件信息
        for plugin_name, plugin_info in metadata["plugins"].items():
            plugin_required_fields = ["name", "path", "size", "hash", "type"]
            for field in plugin_required_fields:
                if field not in plugin_info:
                    print(f"Plugin {plugin_name} missing field: {field}")
                    return False
            
            # 验证文件是否存在
            plugin_path = Path(plugin_info["path"])
            if not plugin_path.exists():
                print(f"Plugin file not found: {plugin_path}")
                return False
            
            # 验证哈希值
            current_hash = calculate_file_hash(plugin_path)
            if current_hash != plugin_info["hash"]:
                print(f"Hash mismatch for plugin: {plugin_name}")
                return False
        
        print("Metadata validation successful")
        return True
        
    except json.JSONDecodeError as e:
        print(f"Invalid JSON in metadata file: {e}")
        return False
    except Exception as e:
        print(f"Metadata validation failed: {e}")
        return False

def main():
    if len(sys.argv) < 3:
        print("Usage: generate_plugin_metadata.py <plugin_directory> <output_file> [--validate]")
        return 1
    
    plugin_dir = sys.argv[1]
    output_file = sys.argv[2]
    
    if len(sys.argv) > 3 and sys.argv[3] == "--validate":
        # 验证现有元数据
        success = validate_metadata(output_file)
        return 0 if success else 1
    else:
        # 生成新元数据
        success = generate_metadata(plugin_dir, output_file)
        return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())