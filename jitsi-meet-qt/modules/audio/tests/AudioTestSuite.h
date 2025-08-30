#ifndef AUDIOTESTSUITE_H
#define AUDIOTESTSUITE_H

#include <QObject>
#include <QStringList>
#include <QElapsedTimer>
#include <QTextStream>

/**
 * @brief 音频模块测试套件
 * 
 * AudioTestSuite提供音频模块的完整测试套件管理，包括：
 * - 测试执行和结果收集
 * - 性能基准测试
 * - 兼容性验证
 * - 测试报告生成
 */
class AudioTestSuite : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 测试结果枚举
     */
    enum TestResult {
        Passed,     ///< 测试通过
        Failed,     ///< 测试失败
        Skipped,    ///< 测试跳过
        Error       ///< 测试错误
    };

    /**
     * @brief 测试类别枚举
     */
    enum TestCategory {
        BasicTests,         ///< 基础测试
        DeviceTests,        ///< 设备测试
        QualityTests,       ///< 质量测试
        LatencyTests,       ///< 延迟测试
        PerformanceTests,   ///< 性能测试
        StressTests,        ///< 压力测试
        CompatibilityTests, ///< 兼容性测试
        IntegrationTests,   ///< 集成测试
        PlatformTests       ///< 平台测试
    };

    /**
     * @brief 测试信息结构
     */
    struct TestInfo {
        QString name;           ///< 测试名称
        TestCategory category;  ///< 测试类别
        TestResult result;      ///< 测试结果
        qint64 executionTime;   ///< 执行时间(ms)
        QString errorMessage;   ///< 错误信息
        QVariantMap metrics;    ///< 性能指标
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit AudioTestSuite(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~AudioTestSuite();

    /**
     * @brief 运行所有测试
     * @return 所有测试是否通过
     */
    bool runAllTests();

    /**
     * @brief 运行指定类别的测试
     * @param category 测试类别
     * @return 测试是否通过
     */
    bool runTestCategory(TestCategory category);

    /**
     * @brief 运行单个测试
     * @param testName 测试名称
     * @return 测试是否通过
     */
    bool runSingleTest(const QString &testName);

    /**
     * @brief 获取测试结果
     * @return 测试结果列表
     */
    QList<TestInfo> testResults() const;

    /**
     * @brief 获取测试统计信息
     * @return 统计信息映射
     */
    QVariantMap testStatistics() const;

    /**
     * @brief 生成测试报告
     * @param filePath 报告文件路径
     * @return 生成成功返回true
     */
    bool generateReport(const QString &filePath) const;

    /**
     * @brief 生成HTML测试报告
     * @param filePath HTML文件路径
     * @return 生成成功返回true
     */
    bool generateHtmlReport(const QString &filePath) const;

    /**
     * @brief 设置测试超时时间
     * @param timeout 超时时间(ms)
     */
    void setTestTimeout(int timeout);

    /**
     * @brief 设置详细输出模式
     * @param verbose 是否详细输出
     */
    void setVerboseMode(bool verbose);

    /**
     * @brief 设置性能基准测试模式
     * @param enabled 是否启用
     */
    void setBenchmarkMode(bool enabled);

signals:
    /**
     * @brief 测试开始信号
     * @param testName 测试名称
     */
    void testStarted(const QString &testName);

    /**
     * @brief 测试完成信号
     * @param testName 测试名称
     * @param result 测试结果
     */
    void testCompleted(const QString &testName, TestResult result);

    /**
     * @brief 测试套件完成信号
     * @param passed 通过的测试数量
     * @param failed 失败的测试数量
     */
    void testSuiteCompleted(int passed, int failed);

    /**
     * @brief 测试进度信号
     * @param current 当前测试索引
     * @param total 总测试数量
     */
    void testProgress(int current, int total);

private:
    /**
     * @brief 初始化测试套件
     */
    void initializeTestSuite();

    /**
     * @brief 执行测试类
     * @param testClassName 测试类名
     * @return 测试结果
     */
    TestResult executeTestClass(const QString &testClassName);

    /**
     * @brief 收集性能指标
     * @param testName 测试名称
     * @return 性能指标
     */
    QVariantMap collectPerformanceMetrics(const QString &testName);

    /**
     * @brief 验证测试环境
     * @return 环境是否有效
     */
    bool validateTestEnvironment();

    /**
     * @brief 生成测试摘要
     * @return 摘要字符串
     */
    QString generateTestSummary() const;

    /**
     * @brief 格式化测试结果
     * @param info 测试信息
     * @return 格式化字符串
     */
    QString formatTestResult(const TestInfo &info) const;

    QList<TestInfo> m_testResults;      ///< 测试结果
    int m_testTimeout;                  ///< 测试超时时间
    bool m_verboseMode;                 ///< 详细输出模式
    bool m_benchmarkMode;               ///< 性能基准模式
    QElapsedTimer m_suiteTimer;         ///< 套件计时器
};

#endif // AUDIOTESTSUITE_H