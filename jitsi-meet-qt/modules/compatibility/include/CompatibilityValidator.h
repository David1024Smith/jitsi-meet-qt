#ifndef COMPATIBILITYVALIDATOR_H
#define COMPATIBILITYVALIDATOR_H

#include "ICompatibilityValidator.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QHash>
#include <QMutex>
#include <QTimer>

class FunctionValidator;
class PerformanceValidator;

/**
 * @brief 兼容性验证器实现
 * 
 * 实现了兼容性验证器接口，提供功能验证和兼容性测试功能。
 */
class CompatibilityValidator : public ICompatibilityValidator
{
    Q_OBJECT

public:
    explicit CompatibilityValidator(QObject *parent = nullptr);
    ~CompatibilityValidator();

    // ICompatibilityValidator 接口实现
    bool initialize() override;
    ValidationResult validateFunctionality(const QString& moduleName) override;
    QStringList runCompatibilityTests() override;
    QList<ValidationReport> getDetailedReport() const override;
    bool validatePerformance(const QString& moduleName) override;
    void setValidationConfig(const QVariantMap& config) override;

    // 扩展功能
    void addCustomTest(const QString& testName, std::function<ValidationResult()> testFunction);
    void removeCustomTest(const QString& testName);
    QStringList getAvailableTests() const;
    
    void setTestTimeout(int timeoutMs);
    int getTestTimeout() const;
    
    void enableParallelTesting(bool enabled);
    bool isParallelTestingEnabled() const;

private slots:
    void onTestCompleted(const QString& testName, ValidationResult result);
    void onTestTimeout();

private:
    struct TestInfo {
        QString name;
        QString module;
        std::function<ValidationResult()> function;
        int timeout;
        bool enabled;
    };

    bool runSingleTest(const TestInfo& testInfo);
    ValidationResult validateAudioModule();
    ValidationResult validateNetworkModule();
    ValidationResult validateUIModule();
    ValidationResult validateChatModule();
    ValidationResult validateScreenShareModule();
    ValidationResult validateMeetingModule();
    ValidationResult validatePerformanceModule();
    ValidationResult validateSettingsModule();
    ValidationResult validateUtilsModule();
    
    void setupDefaultTests();
    void clearReports();
    ValidationReport createReport(const QString& testName, ValidationResult result, 
                                const QString& message, double executionTime);

    bool m_initialized;
    QVariantMap m_config;
    int m_testTimeout;
    bool m_parallelTestingEnabled;
    
    QHash<QString, TestInfo> m_tests;
    QList<ValidationReport> m_reports;
    
    FunctionValidator* m_functionValidator;
    PerformanceValidator* m_performanceValidator;
    
    QTimer* m_timeoutTimer;
    QString m_currentTest;
    
    mutable QMutex m_mutex;
};

#endif // COMPATIBILITYVALIDATOR_H