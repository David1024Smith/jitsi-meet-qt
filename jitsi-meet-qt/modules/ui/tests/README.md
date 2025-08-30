# UI Module Tests

This directory contains all tests for the UI module, including unit tests, integration tests, and UI component tests.

## Test Structure

```
tests/
├── unit/                    # Unit tests
│   ├── UIModuleTest.cpp    # UI module core tests
│   ├── UIManagerTest.cpp   # UI manager tests
│   ├── ThemeFactoryTest.cpp # Theme factory tests
│   └── UIConfigTest.cpp    # Configuration tests
├── integration/            # Integration tests
│   ├── ThemeIntegrationTest.cpp
│   └── LayoutIntegrationTest.cpp
├── widgets/               # Widget tests
│   ├── CustomButtonTest.cpp
│   ├── StatusBarTest.cpp
│   └── ToolBarTest.cpp
├── layouts/               # Layout tests
│   ├── MainLayoutTest.cpp
│   ├── ConferenceLayoutTest.cpp
│   └── SettingsLayoutTest.cpp
├── mocks/                 # Mock objects
│   ├── MockTheme.h
│   └── MockWidget.h
├── CMakeLists.txt         # CMake configuration
├── ui_tests.pro          # qmake configuration
├── run_tests.sh          # Linux/macOS test runner
├── run_tests.bat         # Windows test runner
└── TestRunner.cpp        # Main test runner
```

## Running Tests

### Using CMake
```bash
cd tests
mkdir build && cd build
cmake ..
make
./ui_tests
```

### Using qmake
```bash
cd tests
qmake ui_tests.pro
make
./ui_tests
```

### Using Scripts
```bash
# Linux/macOS
./run_tests.sh

# Windows
run_tests.bat
```

## Test Categories

### Unit Tests
- Test individual classes and functions in isolation
- Mock external dependencies
- Focus on business logic and algorithms

### Integration Tests
- Test interaction between UI components
- Test theme application across widgets
- Test layout management with real widgets

### Widget Tests
- Test custom widget behavior
- Test widget styling and theming
- Test widget configuration and state management

### Layout Tests
- Test layout application and management
- Test responsive behavior
- Test layout configuration and persistence

## Writing Tests

### Test Naming Convention
- Test files: `[ClassName]Test.cpp`
- Test methods: `test[MethodName]_[Scenario]_[ExpectedResult]()`
- Example: `testSetTheme_ValidTheme_ThemeApplied()`

### Test Structure
```cpp
class UIModuleTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();    // Run once before all tests
    void init();           // Run before each test
    void cleanup();        // Run after each test
    void cleanupTestCase(); // Run once after all tests
    
    // Actual test methods
    void testInitialize_ValidConfig_Success();
    void testSetTheme_InvalidTheme_Failure();
};
```

### Assertions
Use Qt Test framework assertions:
- `QVERIFY(condition)` - Verify boolean condition
- `QCOMPARE(actual, expected)` - Compare values
- `QVERIFY_EXCEPTION_THROWN(code, exception)` - Verify exception
- `QTEST_MAIN(TestClass)` - Main test entry point

## Mock Objects

Mock objects are provided for testing in isolation:
- `MockTheme` - Mock theme implementation
- `MockWidget` - Mock widget for layout testing
- `MockManager` - Mock manager classes

## Test Data

Test data files are located in the `data/` subdirectory:
- Configuration files for testing
- Sample theme files
- Test images and resources

## Continuous Integration

Tests are automatically run in CI/CD pipeline:
- All tests must pass before merge
- Code coverage reports are generated
- Performance benchmarks are tracked

## Debugging Tests

### Running Single Test
```bash
./ui_tests testMethodName
```

### Verbose Output
```bash
./ui_tests -v2
```

### Debug Mode
```bash
gdb ./ui_tests
(gdb) run testMethodName
```