#include <QtTest/QtTest>

// Test class declarations
class TestMainApplication;

// Main test runner
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    int result = 0;
    
    // Run MainApplication tests
    {
        TestMainApplication testMainApp;
        result |= QTest::qExec(&testMainApp, argc, argv);
    }
    
    return result;
}