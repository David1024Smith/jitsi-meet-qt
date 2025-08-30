#ifndef PROGRESSIVEREPLACEMENTTEST_H
#define PROGRESSIVEREPLACEMENTTEST_H

#include <QObject>
#include <QTest>

class ProgressiveReplacementManager;
class ReplacementConfig;

/**
 * @brief 渐进式代码替换测试套件
 * 
 * 测试渐进式代码替换管理器的各项功能
 */
class ProgressiveReplacementTest : public QObject
{
    Q_OBJECT

public:
    explicit ProgressiveReplacementTest(QObject *parent = nullptr);

private slots:
    // 测试框架方法
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 功能测试
    void testInitialization();
    void testReplacementPlanManagement();
    void testReplacementExecution();
    void testReplacementControl();
    void testParallelMode();
    void testValidationAndTesting();
    void testSafetyControls();
    void testScheduling();
    void testBatchReplacement();
    void testReporting();
    void testConfigurationIntegration();
    void testErrorHandling();

private:
    ProgressiveReplacementManager* m_manager;
    ReplacementConfig* m_config;
};

#endif // PROGRESSIVEREPLACEMENTTEST_H