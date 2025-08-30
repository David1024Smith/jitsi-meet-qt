#ifndef ICOMPATIBILITYVALIDATOR_H
#define ICOMPATIBILITYVALIDATOR_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

/**
 * @brief 兼容性验证器接口
 */
class ICompatibilityValidator : public QObject
{
    Q_OBJECT

public:
    enum ValidationResult {
        Passed,
        Failed,
        Warning,
        Skipped
    };
    Q_ENUM(ValidationResult)

    struct ValidationReport {
        QString testName;
        ValidationResult result;
        QString message;
        double executionTime;
        QVariantMap details;
    };

    explicit ICompatibilityValidator(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~ICompatibilityValidator() = default;

    virtual bool initialize() = 0;
    virtual ValidationResult validateFunctionality(const QString& moduleName) = 0;
    virtual QStringList runCompatibilityTests() = 0;
    virtual QList<ValidationReport> getDetailedReport() const = 0;
    virtual bool validatePerformance(const QString& moduleName) = 0;
    virtual void setValidationConfig(const QVariantMap& config) = 0;

signals:
    void validationStarted(const QString& testName);
    void validationCompleted(const QString& testName, ValidationResult result);
    void progressUpdated(int percentage);
    void errorOccurred(const QString& error);
};

Q_DECLARE_METATYPE(ICompatibilityValidator::ValidationReport)

#endif // ICOMPATIBILITYVALIDATOR_H