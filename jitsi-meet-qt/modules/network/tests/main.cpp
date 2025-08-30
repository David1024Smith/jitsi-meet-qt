#include "NetworkModuleTest.h"
#include "NetworkTestSuite.cpp"
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTest>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("NetworkModuleTests");
    app.setApplicationVersion("1.0");

    // 设置命令行解析器
    QCommandLineParser parser;
    parser.setApplicationDescription("网络模块测试程序");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption suiteOption(QStringList() << "s" << "suite",
        "运行指定的测试套件 (standard|quick|full|performance)", "suite", "standard");
    parser.addOption(suiteOption);

    QCommandLineOption groupOption(QStringList() << "g" << "group",
        "运行指定的测试分组", "group");
    parser.addOption(groupOption);

    QCommandLineOption listOption(QStringList() << "l" << "list",
        "列出所有可用的测试套件和分组");
    parser.addOption(listOption);

    QCommandLineOption verboseOption(QStringList() << "v" << "verbose",
        "启用详细输出");
    parser.addOption(verboseOption);

    // 解析命令行参数
    parser.process(app);

    // 设置环境
    if (parser.isSet(verboseOption)) {
        qputenv("NETWORK_TEST_VERBOSE", "1");
    }

    // 处理命令行选项
    if (parser.isSet(listOption)) {
        NetworkTestSuiteManager::listSuites();
        return 0;
    }

    int result = 0;

    if (parser.isSet(groupOption)) {
        // 运行指定分组的测试
        QString group = parser.value(groupOption);
        result = NetworkTestSuiteManager::runGroup(group);
    } else {
        // 运行测试套件
        QString suite = parser.value(suiteOption);
        result = NetworkTestSuiteManager::runSuite(suite);
    }

    return result;
}