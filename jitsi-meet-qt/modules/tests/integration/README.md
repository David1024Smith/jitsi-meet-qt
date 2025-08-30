# Module Integration Tests

This directory contains comprehensive integration tests for the Jitsi Meet Qt modular architecture.

## Overview

The Module Integration Test suite validates the interaction between different modules, ensuring proper:
- Module loading order and dependency management
- Inter-module communication
- Error propagation and recovery
- Performance characteristics
- Resource sharing and cleanup

## Test Structure

### Core Test Class
- `ModuleIntegrationTest.h/cpp` - Main test class implementing all integration test scenarios

### Test Categories

#### 1. Module Management Tests
- **Module Load Order Test** - Validates modules load in correct dependency order
- **Module Dependencies Test** - Verifies dependency relationships are respected
- **Module Unloading Test** - Tests proper module cleanup and unloading
- **Module Version Compatibility Test** - Checks version compatibility between modules
- **Module Health Check Test** - Validates module integrity and status

#### 2. Inter-Module Communication Tests
- **Audio-Video Integration** - Tests audio and camera module coordination
- **Chat-Network Integration** - Validates chat message transmission via network
- **UI-Performance Integration** - Tests performance metrics display in UI
- **Settings Module Integration** - Verifies settings propagation to all modules
- **Screen Share Integration** - Tests screen sharing with network and UI
- **Meeting Module Integration** - Validates meeting coordination with all modules

#### 3. End-to-End Workflow Tests
- **Complete Workflow Test** - Simulates full meeting workflow
- **Error Propagation Test** - Tests error handling across modules
- **Resource Sharing Test** - Validates shared resource access
- **Concurrent Operations Test** - Tests concurrent module operations
- **Memory Leak Detection Test** - Monitors for memory leaks during operations

#### 4. Performance Tests
- **Module Startup Performance** - Measures module initialization times
- **Module Communication Latency** - Tests inter-module communication speed
- **High Load Scenarios** - Tests system behavior under high load
- **Resource Constraints** - Tests behavior with limited resources

#### 5. Error Handling and Recovery Tests
- **Module Failure Recovery** - Tests recovery from module failures
- **Cascading Failure Handling** - Tests handling of cascading failures
- **Graceful Degradation** - Tests system behavior when modules become unavailable

## Configuration

### Test Configurations
The `data/test_configurations.json` file contains various test configurations:

- **basic_integration** - Basic module integration tests
- **full_integration** - Complete integration test suite
- **performance_focused** - Performance-oriented tests
- **stress_testing** - Stress and load testing

### Module Dependencies
Dependencies are defined in the configuration file and validated during testing:

```json
{
  "module_dependencies": {
    "utils": [],
    "settings": [],
    "performance": ["utils"],
    "audio": ["utils", "settings"],
    "network": ["utils", "settings"],
    "chat": ["network", "utils"],
    "meeting": ["network", "audio", "camera", "chat"],
    "ui": ["settings", "performance"]
  }
}
```

## Building and Running Tests

### Prerequisites
- Qt 6.0 or later
- CMake 3.16+ or qmake
- C++17 compatible compiler

### Build with CMake
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Build with qmake
```bash
qmake integration_tests.pro
make  # or nmake on Windows
```

### Run Tests

#### Windows
```cmd
run_integration_tests.bat
```

#### Linux/macOS
```bash
./run_integration_tests.sh
```

#### Manual Execution
```bash
# With CMake
ctest --output-on-failure

# Direct execution
./bin/ModuleIntegrationTest
```

## Test Output

### Console Output
Tests provide detailed console output showing:
- Test progress and status
- Module loading/unloading events
- Communication test results
- Performance measurements
- Error conditions and recovery

### Test Report
A comprehensive JSON test report is generated at:
- Windows: `%TEMP%\module_integration_test_report.json`
- Linux/macOS: `/tmp/module_integration_test_report.json`

The report includes:
- Test execution summary
- Individual test results
- Performance metrics
- Error details
- Timing information

## Test Scenarios

### Basic Integration Flow
1. Load modules in dependency order
2. Validate all dependencies are satisfied
3. Test basic inter-module communication
4. Verify module health and integrity
5. Clean shutdown and resource cleanup

### Full Integration Flow
1. Complete module ecosystem loading
2. End-to-end workflow simulation
3. Error injection and recovery testing
4. Performance measurement and validation
5. Stress testing under various conditions

### Performance Testing
1. Measure module startup times
2. Test communication latency between modules
3. Monitor memory usage and detect leaks
4. Validate performance under high load
5. Test behavior with resource constraints

## Customization

### Adding New Tests
To add new integration tests:

1. Add test method to `ModuleIntegrationTest` class
2. Follow naming convention: `test[TestName]()`
3. Use `TestResult` structure for consistent reporting
4. Add test configuration to `test_configurations.json`

### Module-Specific Tests
For module-specific integration tests:

1. Use conditional compilation based on module availability
2. Check module loading status before testing
3. Implement graceful degradation for missing modules
4. Document module dependencies in test descriptions

### Configuration Options
Test behavior can be customized via:

- **Environment Variables**
  - `INTEGRATION_TEST_CONFIG` - Path to custom configuration file
  - `INTEGRATION_TEST_TIMEOUT` - Override default test timeout
  - `INTEGRATION_TEST_VERBOSE` - Enable verbose output

- **Command Line Arguments**
  - `--config <file>` - Specify configuration file
  - `--performance` - Enable performance tests
  - `--stress` - Enable stress tests
  - `--module <name>` - Test specific module only

## Troubleshooting

### Common Issues

#### Module Loading Failures
- Verify module dependencies are available
- Check module library paths
- Ensure proper compilation flags are set

#### Communication Test Failures
- Verify signal/slot connections are established
- Check for threading issues
- Validate message format compatibility

#### Performance Test Failures
- Adjust performance thresholds in configuration
- Consider system load during testing
- Check for resource constraints

#### Memory Leak Detection
- Run tests with memory debugging tools
- Monitor system memory usage
- Check for proper resource cleanup

### Debug Mode
Enable debug output by setting:
```cpp
QLoggingCategory::setFilterRules("*.debug=true");
```

## Integration with CI/CD

### Automated Testing
The integration tests can be integrated into CI/CD pipelines:

```yaml
# Example GitHub Actions workflow
- name: Run Integration Tests
  run: |
    cd jitsi-meet-qt/modules/tests/integration
    ./run_integration_tests.sh
```

### Test Reporting
Test results can be parsed from the JSON report for CI/CD integration:

```bash
# Extract test results
jq '.passed, .failed, .total_tests' /tmp/module_integration_test_report.json
```

## Requirements Traceability

This integration test suite addresses the following requirements:

- **Requirement 11.4** - Module integration and communication testing
- **Requirement 11.5** - Inter-module dependency validation
- **Requirement 12.6** - End-to-end functionality testing and error propagation

## Contributing

When contributing to the integration tests:

1. Follow the existing test structure and naming conventions
2. Add appropriate documentation for new test scenarios
3. Update configuration files for new test parameters
4. Ensure tests are deterministic and repeatable
5. Add proper error handling and cleanup code

## License

This test suite is part of the Jitsi Meet Qt project and follows the same licensing terms.