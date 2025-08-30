# Comprehensive Testing Framework for Jitsi Meet Qt Modular Architecture

This directory contains a complete testing framework that ensures quality and reliability of the modular Jitsi Meet Qt application.

## Overview

The testing framework provides:

- **Complete Unit Test Coverage** for all modules
- **Integration Testing** between modules
- **End-to-End Testing** of complete workflows
- **Performance Benchmarking** and regression detection
- **Automated Test Execution** and CI/CD integration
- **Code Coverage Analysis** with detailed reporting
- **Quality Assurance** metrics and thresholds

## Requirements Addressed

This implementation addresses the following requirements:

- **11.5**: 确保所有模块都有完整的单元测试覆盖
- **11.6**: 添加集成测试和端到端测试
- **12.6**: 实现自动化测试和持续集成，创建性能基准测试和回归测试

## Framework Components

### 1. TestCoverageFramework (`TestCoverageFramework.h/.cpp`)

The core testing framework that orchestrates all test types:

- **Unit Tests**: Individual module testing
- **Integration Tests**: Module interaction testing
- **End-to-End Tests**: Complete workflow testing
- **Performance Tests**: Benchmarking and optimization
- **Regression Tests**: Performance and functionality regression detection
- **Security Tests**: Input validation and security checks

### 2. AutomatedTestRunner (`AutomatedTestRunner.h/.cpp`)

Automated test execution and scheduling:

- **Scheduled Testing**: Periodic, file-change, or manual triggers
- **CI Integration**: GitHub Actions, GitLab CI, Jenkins, Azure DevOps
- **Notifications**: Email, Slack, webhook notifications
- **File Watching**: Automatic test execution on code changes

### 3. PerformanceBenchmarkSuite (`PerformanceBenchmarkSuite.h`)

Performance testing and benchmarking:

- **Startup Performance**: Application and module load times
- **Runtime Performance**: CPU, memory, network usage
- **Regression Detection**: Performance degradation alerts
- **Trend Analysis**: Historical performance tracking

### 4. Python Test Runner (`run_comprehensive_tests.py`)

High-level test orchestration:

- **Multi-language Support**: Coordinates C++ and Python tests
- **Report Generation**: HTML, JSON, JUnit XML formats
- **Environment Setup**: Test data preparation and cleanup
- **CI Integration**: Status reporting and artifact management

## Test Types

### Unit Tests

Each module has comprehensive unit tests covering:

- **Core Functionality**: All public APIs and methods
- **Error Handling**: Exception and error condition testing
- **Edge Cases**: Boundary conditions and invalid inputs
- **Mock Integration**: Isolated testing with mocked dependencies

**Location**: `modules/{module}/tests/`
**Execution**: `ctest -R "{module}_UnitTests"`

### Integration Tests

Cross-module interaction testing:

- **Module Communication**: Inter-module messaging and data flow
- **Dependency Resolution**: Module loading and dependency chains
- **Resource Sharing**: Shared services and utilities
- **Error Propagation**: Cross-module error handling

**Location**: `modules/tests/integration/`
**Execution**: `ctest -R "IntegrationTests"`

### End-to-End Tests

Complete workflow testing:

- **Meeting Workflows**: Join, leave, audio/video control
- **Chat Functionality**: Message sending and receiving
- **Screen Sharing**: Capture and transmission
- **Settings Management**: Configuration persistence and sync

**Location**: `modules/tests/e2e/`
**Execution**: `python3 run_comprehensive_tests.py --types e2e`

### Performance Tests

Benchmarking and performance validation:

- **Startup Time**: Application and module initialization
- **Memory Usage**: Peak and sustained memory consumption
- **CPU Usage**: Processing efficiency and optimization
- **Network Performance**: Latency and throughput testing

**Location**: `modules/tests/performance/`
**Execution**: `ctest -R "PerformanceBenchmarks"`

## Usage

### Quick Start

```bash
# Run all tests
python3 run_comprehensive_tests.py

# Run specific test types
python3 run_comprehensive_tests.py --types unit,integration

# Run tests for specific modules
python3 run_comprehensive_tests.py --modules core,audio,video

# Generate coverage report
python3 run_comprehensive_tests.py --coverage

# Verbose output
python3 run_comprehensive_tests.py --verbose
```

### CMake Integration

```bash
# Configure with testing enabled
cmake -B build -DENABLE_TESTING=ON -DENABLE_COVERAGE=ON

# Build all tests
cmake --build build --target run_all_tests

# Run specific test types
cmake --build build --target run_unit_tests
cmake --build build --target run_integration_tests
cmake --build build --target run_performance_tests

# Generate coverage report
cmake --build build --target coverage
```

### C++ Test Runner

```bash
# Build the main test runner
cmake --build build --target comprehensive_test_runner

# Run with options
./build/bin/comprehensive_test_runner --all --coverage --output test_results

# Automated mode
./build/bin/comprehensive_test_runner --automated --schedule periodic --interval 60
```

## Configuration

### Test Configuration (`test_config.json`)

The main configuration file controls:

- **Module Selection**: Which modules to test
- **Test Types**: Enabled test categories
- **Coverage Settings**: Thresholds and reporting formats
- **Performance Benchmarks**: Baseline values and regression thresholds
- **CI Integration**: Provider settings and notifications
- **Quality Gates**: Pass/fail criteria

### Environment Variables

- `QT_QPA_PLATFORM=offscreen`: Headless testing
- `DISPLAY=:99`: Virtual display for GUI tests
- `QT_LOGGING_RULES=*.debug=false`: Reduce log noise

## Coverage Analysis

### Coverage Metrics

- **Line Coverage**: Percentage of code lines executed
- **Function Coverage**: Percentage of functions called
- **Branch Coverage**: Percentage of code branches taken
- **Overall Coverage**: Weighted average of all metrics

### Coverage Thresholds

- **Minimum Overall**: 75%
- **Per-Module Minimum**: 70%
- **Critical Modules**: 85% (core, network, audio, video)

### Coverage Reports

- **HTML Report**: Interactive browsable coverage
- **JSON Report**: Machine-readable coverage data
- **XML Report**: CI integration format

## Performance Benchmarking

### Benchmark Categories

1. **Startup Performance**
   - Application launch time
   - Module initialization time
   - First UI render time

2. **Runtime Performance**
   - Audio processing latency
   - Video rendering FPS
   - Network message latency
   - Memory allocation patterns

3. **Resource Usage**
   - Peak memory consumption
   - CPU utilization patterns
   - Network bandwidth usage
   - Disk I/O operations

### Regression Detection

- **Threshold-based**: Alert on >10% performance degradation
- **Trend Analysis**: Detect gradual performance decline
- **Baseline Comparison**: Compare against known good versions

## CI/CD Integration

### GitHub Actions

The framework includes a comprehensive GitHub Actions workflow:

- **Matrix Testing**: Parallel execution across modules
- **Coverage Reporting**: Automated coverage analysis
- **Performance Tracking**: Benchmark result storage
- **Artifact Management**: Test result preservation

### Supported CI Providers

- **GitHub Actions**: Native workflow integration
- **GitLab CI**: Pipeline configuration support
- **Jenkins**: Jenkinsfile and plugin integration
- **Azure DevOps**: Pipeline YAML support
- **TeamCity**: Build configuration templates

## Quality Gates

### Test Success Criteria

- **Unit Test Pass Rate**: ≥95%
- **Integration Test Pass Rate**: ≥90%
- **Coverage Threshold**: ≥75%
- **Performance Regression**: <10% degradation
- **Memory Leaks**: Zero detected leaks

### Failure Handling

- **Automatic Retries**: Flaky test mitigation
- **Failure Categorization**: Critical vs. non-critical
- **Notification Escalation**: Team alerts on failures
- **Rollback Triggers**: Automatic deployment blocks

## Reporting

### Report Formats

1. **HTML Reports**: Interactive web-based results
2. **JSON Reports**: Machine-readable data
3. **JUnit XML**: CI system integration
4. **CSV Reports**: Spreadsheet analysis

### Dashboard Integration

- **Test Metrics**: Success rates and trends
- **Coverage Trends**: Historical coverage data
- **Performance Graphs**: Benchmark result visualization
- **Quality Indicators**: Overall system health

## Troubleshooting

### Common Issues

1. **Qt Platform Issues**
   ```bash
   export QT_QPA_PLATFORM=offscreen
   ```

2. **Display Problems**
   ```bash
   Xvfb :99 -screen 0 1024x768x24 &
   export DISPLAY=:99
   ```

3. **Coverage Generation**
   ```bash
   # Ensure gcov/lcov are installed
   sudo apt-get install lcov gcovr
   ```

4. **Permission Issues**
   ```bash
   # Make scripts executable
   chmod +x run_comprehensive_tests.py
   ```

### Debug Mode

Enable verbose logging for troubleshooting:

```bash
# Python runner
python3 run_comprehensive_tests.py --verbose

# C++ runner
./comprehensive_test_runner --verbose

# CMake
cmake -DCMAKE_BUILD_TYPE=Debug
```

## Contributing

### Adding New Tests

1. **Unit Tests**: Add to `modules/{module}/tests/`
2. **Integration Tests**: Add to `modules/tests/integration/`
3. **Performance Tests**: Add to `PerformanceBenchmarkSuite`
4. **Update Configuration**: Modify `test_config.json`

### Test Guidelines

- **Naming Convention**: `{Module}{Component}Test.cpp`
- **Mock Usage**: Isolate dependencies with mocks
- **Data Cleanup**: Ensure proper test cleanup
- **Documentation**: Document test purpose and setup

## License

This testing framework is part of the Jitsi Meet Qt project and follows the same licensing terms.