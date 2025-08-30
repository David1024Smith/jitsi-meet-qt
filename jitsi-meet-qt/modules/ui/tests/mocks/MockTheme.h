#ifndef MOCKTHEME_H
#define MOCKTHEME_H

#include <QObject>
#include <QColor>
#include <QString>
#include <QVariantMap>
#include "../../themes/BaseTheme.h"

/**
 * @brief Mock theme implementation for testing
 * 
 * This mock theme provides a controllable theme implementation
 * that can be used to test theme-related functionality without
 * depending on actual theme files or resources.
 */
class MockTheme : public BaseTheme
{
    Q_OBJECT

public:
    explicit MockTheme(const QString& name = "mock", QObject* parent = nullptr);
    ~MockTheme() override;

    // BaseTheme interface
    bool loadFromResource(const QString& resourcePath) override;
    bool loadFromFile(const QString& filePath) override;
    QString styleSheet() const override;
    QVariantMap properties() const override;
    bool isValid() const override;

    // Mock-specific methods
    void setMockStyleSheet(const QString& styleSheet);
    void setMockProperties(const QVariantMap& properties);
    void setMockValid(bool valid);
    void setMockLoadResult(bool result);
    
    // Test helpers
    int loadFromResourceCallCount() const { return m_loadFromResourceCallCount; }
    int loadFromFileCallCount() const { return m_loadFromFileCallCount; }
    QString lastResourcePath() const { return m_lastResourcePath; }
    QString lastFilePath() const { return m_lastFilePath; }
    
    void resetCallCounts();

signals:
    void mockLoadFromResourceCalled(const QString& resourcePath);
    void mockLoadFromFileCalled(const QString& filePath);

private:
    QString m_mockStyleSheet;
    QVariantMap m_mockProperties;
    bool m_mockValid;
    bool m_mockLoadResult;
    
    // Call tracking
    int m_loadFromResourceCallCount;
    int m_loadFromFileCallCount;
    QString m_lastResourcePath;
    QString m_lastFilePath;
};

#endif // MOCKTHEME_H