#!/usr/bin/env python3
"""
Jitsi Meet Qt 版本控制和回滚机制
管理应用程序版本、模块版本和配置版本
"""

import json
import os
import sys
import shutil
import argparse
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Optional

class VersionManager:
    def __init__(self, base_dir: str = "."):
        self.base_dir = Path(base_dir)
        self.version_file = self.base_dir / "version_info.json"
        self.backup_dir = self.base_dir / "backups"
        self.backup_dir.mkdir(exist_ok=True)
        
    def get_current_version(self) -> Dict:
        """获取当前版本信息"""
        if self.version_file.exists():
            with open(self.version_file, 'r', encoding='utf-8') as f:
                return json.load(f)
        else:
            return self._create_default_version_info()
            
    def _create_default_version_info(self) -> Dict:
        """创建默认版本信息"""
        default_version = {
            "application": {
                "name": "Jitsi Meet Qt",
                "version": "2.0.0",
                "build_number": 1,
                "build_date": datetime.now().isoformat(),
                "git_commit": self._get_git_commit(),
                "branch": self._get_git_branch()
            },
            "modules": {
                "core": "2.0.0",
                "audio": "2.0.0",
                "network": "2.0.0",
                "ui": "2.0.0",
                "chat": "2.0.0",
                "screenshare": "2.0.0",
                "meeting": "2.0.0",
                "performance": "2.0.0",
                "settings": "2.0.0",
                "utils": "2.0.0",
                "compatibility": "2.0.0"
            },
            "dependencies": {
                "qt_version": self._get_qt_version(),
                "cmake_version": self._get_cmake_version(),
                "compiler": self._get_compiler_info()
            },
            "compatibility": {
                "min_supported_version": "1.0.0",
                "config_version": "2.0.0",
                "api_version": "2.0.0"
            }
        }
        
        self._save_version_info(default_version)
        return default_version
        
    def _save_version_info(self, version_info: Dict):
        """保存版本信息"""
        with open(self.version_file, 'w', encoding='utf-8') as f:
            json.dump(version_info, f, indent=2, ensure_ascii=False)
            
    def _get_git_commit(self) -> str:
        """获取Git提交哈希"""
        try:
            import subprocess
            result = subprocess.run(['git', 'rev-parse', 'HEAD'], 
                                  capture_output=True, text=True, cwd=self.base_dir)
            return result.stdout.strip() if result.returncode == 0 else "unknown"
        except:
            return "unknown"
            
    def _get_git_branch(self) -> str:
        """获取Git分支名"""
        try:
            import subprocess
            result = subprocess.run(['git', 'branch', '--show-current'], 
                                  capture_output=True, text=True, cwd=self.base_dir)
            return result.stdout.strip() if result.returncode == 0 else "unknown"
        except:
            return "unknown"
            
    def _get_qt_version(self) -> str:
        """获取Qt版本"""
        try:
            import subprocess
            result = subprocess.run(['qmake', '-query', 'QT_VERSION'], 
                                  capture_output=True, text=True)
            return result.stdout.strip() if result.returncode == 0 else "unknown"
        except:
            return "unknown"
            
    def _get_cmake_version(self) -> str:
        """获取CMake版本"""
        try:
            import subprocess
            result = subprocess.run(['cmake', '--version'], 
                                  capture_output=True, text=True)
            if result.returncode == 0:
                lines = result.stdout.split('\n')
                if lines:
                    return lines[0].split()[-1] if 'cmake version' in lines[0].lower() else "unknown"
            return "unknown"
        except:
            return "unknown"
            
    def _get_compiler_info(self) -> Dict:
        """获取编译器信息"""
        try:
            import subprocess
            
            # 尝试获取GCC版本
            try:
                result = subprocess.run(['gcc', '--version'], 
                                      capture_output=True, text=True)
                if result.returncode == 0:
                    gcc_version = result.stdout.split('\n')[0]
                    return {"type": "gcc", "version": gcc_version}
            except:
                pass
                
            # 尝试获取Clang版本
            try:
                result = subprocess.run(['clang', '--version'], 
                                      capture_output=True, text=True)
                if result.returncode == 0:
                    clang_version = result.stdout.split('\n')[0]
                    return {"type": "clang", "version": clang_version}
            except:
                pass
                
            # 尝试获取MSVC版本
            try:
                result = subprocess.run(['cl'], 
                                      capture_output=True, text=True)
                if "Microsoft" in result.stderr:
                    return {"type": "msvc", "version": "detected"}
            except:
                pass
                
            return {"type": "unknown", "version": "unknown"}
        except:
            return {"type": "unknown", "version": "unknown"}
            
    def update_version(self, component: str, new_version: str):
        """更新组件版本"""
        version_info = self.get_current_version()
        
        if component == "application":
            version_info["application"]["version"] = new_version
            version_info["application"]["build_number"] += 1
            version_info["application"]["build_date"] = datetime.now().isoformat()
        elif component in version_info["modules"]:
            version_info["modules"][component] = new_version
        else:
            raise ValueError(f"未知组件: {component}")
            
        self._save_version_info(version_info)
        print(f"✅ 已更新 {component} 版本到 {new_version}")
        
    def create_backup(self, backup_name: Optional[str] = None) -> str:
        """创建系统备份"""
        if not backup_name:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            backup_name = f"backup_{timestamp}"
            
        backup_path = self.backup_dir / backup_name
        backup_path.mkdir(exist_ok=True)
        
        # 备份版本信息
        if self.version_file.exists():
            shutil.copy2(self.version_file, backup_path / "version_info.json")
            
        # 备份配置文件
        config_dirs = [
            "modules",
            "config"
        ]
        
        for config_dir in config_dirs:
            source_dir = self.base_dir / config_dir
            if source_dir.exists():
                target_dir = backup_path / config_dir
                shutil.copytree(source_dir, target_dir, dirs_exist_ok=True)
                
        # 备份可执行文件
        executables = ["jitsi-meet-qt", "jitsi-meet-qt.exe"]
        for exe in executables:
            exe_path = self.base_dir / exe
            if exe_path.exists():
                shutil.copy2(exe_path, backup_path / exe)
                
        # 创建备份信息文件
        backup_info = {
            "backup_name": backup_name,
            "creation_date": datetime.now().isoformat(),
            "version_info": self.get_current_version(),
            "backup_size": self._get_directory_size(backup_path)
        }
        
        with open(backup_path / "backup_info.json", 'w', encoding='utf-8') as f:
            json.dump(backup_info, f, indent=2, ensure_ascii=False)
            
        print(f"✅ 已创建备份: {backup_name}")
        return backup_name
        
    def list_backups(self) -> List[Dict]:
        """列出所有备份"""
        backups = []
        
        for backup_dir in self.backup_dir.iterdir():
            if backup_dir.is_dir():
                backup_info_file = backup_dir / "backup_info.json"
                if backup_info_file.exists():
                    with open(backup_info_file, 'r', encoding='utf-8') as f:
                        backup_info = json.load(f)
                        backups.append(backup_info)
                else:
                    # 为旧备份创建信息
                    backup_info = {
                        "backup_name": backup_dir.name,
                        "creation_date": datetime.fromtimestamp(backup_dir.stat().st_mtime).isoformat(),
                        "version_info": "unknown",
                        "backup_size": self._get_directory_size(backup_dir)
                    }
                    backups.append(backup_info)
                    
        return sorted(backups, key=lambda x: x["creation_date"], reverse=True)
        
    def restore_backup(self, backup_name: str) -> bool:
        """恢复备份"""
        backup_path = self.backup_dir / backup_name
        
        if not backup_path.exists():
            print(f"❌ 备份不存在: {backup_name}")
            return False
            
        print(f"🔄 正在恢复备份: {backup_name}")
        
        try:
            # 创建当前状态的备份
            current_backup = self.create_backup("pre_restore_backup")
            print(f"💾 已创建恢复前备份: {current_backup}")
            
            # 恢复版本信息
            version_backup = backup_path / "version_info.json"
            if version_backup.exists():
                shutil.copy2(version_backup, self.version_file)
                
            # 恢复配置文件
            config_dirs = ["modules", "config"]
            for config_dir in config_dirs:
                source_dir = backup_path / config_dir
                target_dir = self.base_dir / config_dir
                
                if source_dir.exists():
                    if target_dir.exists():
                        shutil.rmtree(target_dir)
                    shutil.copytree(source_dir, target_dir)
                    
            # 恢复可执行文件
            executables = ["jitsi-meet-qt", "jitsi-meet-qt.exe"]
            for exe in executables:
                exe_backup = backup_path / exe
                exe_target = self.base_dir / exe
                
                if exe_backup.exists():
                    shutil.copy2(exe_backup, exe_target)
                    
            print(f"✅ 备份恢复完成: {backup_name}")
            return True
            
        except Exception as e:
            print(f"❌ 备份恢复失败: {e}")
            return False
            
    def delete_backup(self, backup_name: str) -> bool:
        """删除备份"""
        backup_path = self.backup_dir / backup_name
        
        if not backup_path.exists():
            print(f"❌ 备份不存在: {backup_name}")
            return False
            
        try:
            shutil.rmtree(backup_path)
            print(f"✅ 已删除备份: {backup_name}")
            return True
        except Exception as e:
            print(f"❌ 删除备份失败: {e}")
            return False
            
    def _get_directory_size(self, directory: Path) -> int:
        """获取目录大小"""
        total_size = 0
        for dirpath, dirnames, filenames in os.walk(directory):
            for filename in filenames:
                filepath = os.path.join(dirpath, filename)
                try:
                    total_size += os.path.getsize(filepath)
                except (OSError, FileNotFoundError):
                    pass
        return total_size
        
    def format_size(self, size_bytes: int) -> str:
        """格式化文件大小"""
        for unit in ['B', 'KB', 'MB', 'GB']:
            if size_bytes < 1024.0:
                return f"{size_bytes:.1f} {unit}"
            size_bytes /= 1024.0
        return f"{size_bytes:.1f} TB"
        
    def check_compatibility(self, target_version: str) -> Dict:
        """检查版本兼容性"""
        current_version = self.get_current_version()
        current_app_version = current_version["application"]["version"]
        
        compatibility_info = {
            "compatible": True,
            "warnings": [],
            "errors": [],
            "migration_required": False
        }
        
        # 检查主版本兼容性
        current_major = int(current_app_version.split('.')[0])
        target_major = int(target_version.split('.')[0])
        
        if target_major > current_major:
            compatibility_info["migration_required"] = True
            compatibility_info["warnings"].append(f"主版本升级 ({current_major} → {target_major})，可能需要数据迁移")
            
        elif target_major < current_major:
            compatibility_info["compatible"] = False
            compatibility_info["errors"].append(f"不支持降级到较低主版本 ({current_major} → {target_major})")
            
        # 检查模块兼容性
        min_supported = current_version.get("compatibility", {}).get("min_supported_version", "1.0.0")
        if self._version_compare(target_version, min_supported) < 0:
            compatibility_info["compatible"] = False
            compatibility_info["errors"].append(f"目标版本 {target_version} 低于最低支持版本 {min_supported}")
            
        return compatibility_info
        
    def _version_compare(self, version1: str, version2: str) -> int:
        """比较版本号"""
        v1_parts = [int(x) for x in version1.split('.')]
        v2_parts = [int(x) for x in version2.split('.')]
        
        # 补齐版本号长度
        max_len = max(len(v1_parts), len(v2_parts))
        v1_parts.extend([0] * (max_len - len(v1_parts)))
        v2_parts.extend([0] * (max_len - len(v2_parts)))
        
        for v1, v2 in zip(v1_parts, v2_parts):
            if v1 < v2:
                return -1
            elif v1 > v2:
                return 1
        return 0

def main():
    parser = argparse.ArgumentParser(
        description='Jitsi Meet Qt 版本控制和回滚工具',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例用法:
  # 查看当前版本
  python version_control.py --show-version
  
  # 创建备份
  python version_control.py --create-backup
  
  # 列出备份
  python version_control.py --list-backups
  
  # 恢复备份
  python version_control.py --restore-backup backup_20250130_143022
  
  # 更新版本
  python version_control.py --update-version audio 2.1.0
  
  # 检查兼容性
  python version_control.py --check-compatibility 2.1.0
        """
    )
    
    parser.add_argument('--base-dir', default='.', help='基础目录路径')
    parser.add_argument('--show-version', action='store_true', help='显示当前版本信息')
    parser.add_argument('--create-backup', nargs='?', const=None, help='创建备份')
    parser.add_argument('--list-backups', action='store_true', help='列出所有备份')
    parser.add_argument('--restore-backup', help='恢复指定备份')
    parser.add_argument('--delete-backup', help='删除指定备份')
    parser.add_argument('--update-version', nargs=2, metavar=('COMPONENT', 'VERSION'), 
                       help='更新组件版本')
    parser.add_argument('--check-compatibility', help='检查版本兼容性')
    
    args = parser.parse_args()
    
    print("🔧 Jitsi Meet Qt 版本控制工具 v2.0.0")
    print("=" * 50)
    
    vm = VersionManager(args.base_dir)
    
    try:
        if args.show_version:
            version_info = vm.get_current_version()
            print("📋 当前版本信息:")
            print(json.dumps(version_info, indent=2, ensure_ascii=False))
            
        elif args.create_backup is not None:
            backup_name = vm.create_backup(args.create_backup)
            print(f"✅ 备份创建完成: {backup_name}")
            
        elif args.list_backups:
            backups = vm.list_backups()
            if backups:
                print("📦 可用备份:")
                for backup in backups:
                    size_str = vm.format_size(backup["backup_size"])
                    print(f"  • {backup['backup_name']} ({backup['creation_date']}) - {size_str}")
            else:
                print("📦 没有找到备份")
                
        elif args.restore_backup:
            success = vm.restore_backup(args.restore_backup)
            sys.exit(0 if success else 1)
            
        elif args.delete_backup:
            success = vm.delete_backup(args.delete_backup)
            sys.exit(0 if success else 1)
            
        elif args.update_version:
            component, version = args.update_version
            vm.update_version(component, version)
            
        elif args.check_compatibility:
            compatibility = vm.check_compatibility(args.check_compatibility)
            print(f"🔍 兼容性检查结果 (目标版本: {args.check_compatibility}):")
            print(f"  兼容: {'✅' if compatibility['compatible'] else '❌'}")
            print(f"  需要迁移: {'是' if compatibility['migration_required'] else '否'}")
            
            if compatibility['warnings']:
                print("  ⚠️  警告:")
                for warning in compatibility['warnings']:
                    print(f"    • {warning}")
                    
            if compatibility['errors']:
                print("  ❌ 错误:")
                for error in compatibility['errors']:
                    print(f"    • {error}")
                    
        else:
            parser.print_help()
            
    except KeyboardInterrupt:
        print("\n⚠️  操作被用户中断")
        sys.exit(1)
    except Exception as e:
        print(f"\n❌ 操作失败: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()