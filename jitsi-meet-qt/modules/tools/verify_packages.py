#!/usr/bin/env python3
"""
Package Verification Tool
包验证工具

This tool verifies the integrity and completeness of module packages.
"""

import os
import sys
import json
import hashlib
import tarfile
import tempfile
from pathlib import Path
from typing import Dict, List, Tuple

class PackageVerifier:
    """包验证器"""
    
    def __init__(self):
        self.verification_results = []
    
    def calculate_file_hash(self, file_path: Path) -> str:
        """计算文件哈希值"""
        sha256_hash = hashlib.sha256()
        with open(file_path, "rb") as f:
            for byte_block in iter(lambda: f.read(4096), b""):
                sha256_hash.update(byte_block)
        return sha256_hash.hexdigest()
    
    def verify_package_structure(self, package_path: Path) -> Tuple[bool, List[str]]:
        """验证包结构"""
        errors = []
        
        if not package_path.exists():
            errors.append(f"Package file not found: {package_path}")
            return False, errors
        
        if not package_path.suffix == '.gz':
            errors.append(f"Invalid package format: {package_path}")
            return False, errors
        
        try:
            with tarfile.open(package_path, 'r:gz') as tar:
                members = tar.getnames()
                
                # 检查基本文件结构
                has_pri_file = any(name.endswith('.pri') for name in members)
                has_headers = any('include/' in name for name in members)
                has_sources = any('src/' in name for name in members)
                
                if not has_pri_file:
                    errors.append("No .pri configuration file found")
                
                if not has_headers:
                    errors.append("No header files found in include/ directory")
                
                # 检查目录结构
                expected_dirs = ['include/', 'src/', 'config/']
                for expected_dir in expected_dirs:
                    if not any(name.startswith(expected_dir) for name in members):
                        errors.append(f"Missing expected directory: {expected_dir}")
        
        except tarfile.TarError as e:
            errors.append(f"Failed to read package: {e}")
            return False, errors
        
        return len(errors) == 0, errors
    
    def verify_package_dependencies(self, package_path: Path) -> Tuple[bool, List[str]]:
        """验证包依赖关系"""
        errors = []
        
        try:
            with tempfile.TemporaryDirectory() as temp_dir:
                with tarfile.open(package_path, 'r:gz') as tar:
                    tar.extractall(temp_dir)
                
                # 查找.pri文件
                temp_path = Path(temp_dir)
                pri_files = list(temp_path.rglob("*.pri"))
                
                if not pri_files:
                    errors.append("No .pri file found for dependency analysis")
                    return False, errors
                
                # 分析依赖关系
                pri_content = pri_files[0].read_text()
                
                # 简单的依赖检查（可以扩展为更复杂的解析）
                if "UTILS_MODULE_AVAILABLE" in pri_content:
                    # 检查是否正确声明了对utils模块的依赖
                    pass
                
        except Exception as e:
            errors.append(f"Dependency verification failed: {e}")
            return False, errors
        
        return len(errors) == 0, errors
    
    def verify_package_metadata(self, package_path: Path) -> Tuple[bool, List[str]]:
        """验证包元数据"""
        errors = []
        
        try:
            with tempfile.TemporaryDirectory() as temp_dir:
                with tarfile.open(package_path, 'r:gz') as tar:
                    tar.extractall(temp_dir)
                
                temp_path = Path(temp_dir)
                
                # 查找元数据文件
                metadata_files = list(temp_path.rglob("metadata.json"))
                
                if metadata_files:
                    metadata_file = metadata_files[0]
                    with open(metadata_file, 'r') as f:
                        metadata = json.load(f)
                    
                    # 验证元数据字段
                    required_fields = ["name", "version", "description"]
                    for field in required_fields:
                        if field not in metadata:
                            errors.append(f"Missing metadata field: {field}")
                
        except json.JSONDecodeError as e:
            errors.append(f"Invalid metadata JSON: {e}")
        except Exception as e:
            errors.append(f"Metadata verification failed: {e}")
        
        return len(errors) == 0, errors
    
    def verify_package_integrity(self, package_path: Path) -> Tuple[bool, List[str]]:
        """验证包完整性"""
        errors = []
        
        # 计算包文件哈希
        package_hash = self.calculate_file_hash(package_path)
        
        # 查找对应的哈希文件
        hash_file = package_path.with_suffix(package_path.suffix + '.sha256')
        
        if hash_file.exists():
            expected_hash = hash_file.read_text().strip().split()[0]
            if package_hash != expected_hash:
                errors.append(f"Package integrity check failed: hash mismatch")
        else:
            # 如果没有哈希文件，创建一个
            hash_file.write_text(f"{package_hash}  {package_path.name}\n")
        
        return len(errors) == 0, errors
    
    def verify_single_package(self, package_path: Path) -> Dict:
        """验证单个包"""
        result = {
            "package": str(package_path),
            "success": True,
            "errors": [],
            "warnings": []
        }
        
        print(f"Verifying package: {package_path.name}")
        
        # 验证包结构
        structure_ok, structure_errors = self.verify_package_structure(package_path)
        if not structure_ok:
            result["success"] = False
            result["errors"].extend(structure_errors)
        
        # 验证依赖关系
        deps_ok, deps_errors = self.verify_package_dependencies(package_path)
        if not deps_ok:
            result["success"] = False
            result["errors"].extend(deps_errors)
        
        # 验证元数据
        metadata_ok, metadata_errors = self.verify_package_metadata(package_path)
        if not metadata_ok:
            result["warnings"].extend(metadata_errors)  # 元数据错误作为警告
        
        # 验证完整性
        integrity_ok, integrity_errors = self.verify_package_integrity(package_path)
        if not integrity_ok:
            result["success"] = False
            result["errors"].extend(integrity_errors)
        
        if result["success"]:
            print(f"  ✓ Package verification successful")
        else:
            print(f"  ✗ Package verification failed")
            for error in result["errors"]:
                print(f"    Error: {error}")
        
        if result["warnings"]:
            for warning in result["warnings"]:
                print(f"    Warning: {warning}")
        
        return result
    
    def verify_packages_directory(self, packages_dir: Path) -> List[Dict]:
        """验证包目录中的所有包"""
        if not packages_dir.exists():
            print(f"Packages directory not found: {packages_dir}")
            return []
        
        # 查找所有包文件
        package_files = list(packages_dir.glob("*.tar.gz"))
        
        if not package_files:
            print(f"No package files found in: {packages_dir}")
            return []
        
        print(f"Found {len(package_files)} packages to verify")
        
        results = []
        for package_file in package_files:
            result = self.verify_single_package(package_file)
            results.append(result)
        
        return results
    
    def generate_verification_report(self, results: List[Dict], output_file: Path = None):
        """生成验证报告"""
        total_packages = len(results)
        successful_packages = sum(1 for r in results if r["success"])
        failed_packages = total_packages - successful_packages
        
        report = {
            "verification_summary": {
                "total_packages": total_packages,
                "successful": successful_packages,
                "failed": failed_packages,
                "success_rate": f"{(successful_packages/total_packages*100):.1f}%" if total_packages > 0 else "0%"
            },
            "results": results
        }
        
        print("\n" + "="*50)
        print("PACKAGE VERIFICATION REPORT")
        print("="*50)
        print(f"Total packages: {total_packages}")
        print(f"Successful: {successful_packages}")
        print(f"Failed: {failed_packages}")
        print(f"Success rate: {report['verification_summary']['success_rate']}")
        
        if failed_packages > 0:
            print("\nFailed packages:")
            for result in results:
                if not result["success"]:
                    print(f"  - {Path(result['package']).name}")
                    for error in result["errors"]:
                        print(f"    * {error}")
        
        if output_file:
            with open(output_file, 'w') as f:
                json.dump(report, f, indent=2)
            print(f"\nDetailed report saved to: {output_file}")
        
        return report

def main():
    if len(sys.argv) < 2:
        print("Usage: verify_packages.py <packages_directory> [output_report.json]")
        return 1
    
    packages_dir = Path(sys.argv[1])
    output_file = Path(sys.argv[2]) if len(sys.argv) > 2 else None
    
    verifier = PackageVerifier()
    results = verifier.verify_packages_directory(packages_dir)
    
    if results:
        report = verifier.generate_verification_report(results, output_file)
        
        # 返回适当的退出码
        failed_count = sum(1 for r in results if not r["success"])
        return 1 if failed_count > 0 else 0
    else:
        return 1

if __name__ == "__main__":
    sys.exit(main())