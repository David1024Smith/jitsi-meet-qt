#ifndef MOCKWIDGET_H
#define MOCKWIDGET_H

#include <QWidget>
#include <QVariantMap>
#include <QString>
#include "../../widgets/BaseWidget.h"

/**
 * @brief Mock widget implementation for testing
 * 
 * This mock widget provides a controllable widget implementation
 * that can be used to test widget-related functionality without
 * depending on actual widget rendering or complex UI interactions.
 */
class MockWidget : public BaseWidget
{
    Q_OBJECT

public:
    explicit MockWidget(QWidget* parent = nullptr);
    ~MockWidget() override;

    // BaseWidget interface
    QString componentName() const override;
    QVariantMap getConfiguration() const override;
    bool setConfiguration(const QVariantMap& config) override;
    void applyTheme(BaseTheme* theme) override;

    // Mock-specific methods
    void setMockComponentName(const QString& name);
    void setMockConfiguration(const QVariantMap& config);
    void setMockConfigurationResult(bool result);
    void setMockThemeApplication(bool successful);
    
    // Test helpers
    int applyThemeCallCount() const { return m_applyThemeCallCount; }
    int setConfigurationCallCount() const { return m_setConfigurationCallCount; }
    BaseTheme* lastAppliedTheme() const { return m_lastAppliedTheme; }
    QVariantMap lastConfiguration() const { return m_lastConfiguration; }
    
    void resetCallCounts();

    // Size and geometry mocking
    void setMockSizeHint(const QSize& size);
    void setMockMinimumSize(const QSize& size);
    void setMockMaximumSize(const QSize& size);
    
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void mockThemeApplied(BaseTheme* theme);
    void mockConfigurationSet(const QVariantMap& config);

protected:
    // Override paint event for testing
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QString m_mockComponentName;
    QVariantMap m_mockConfiguration;
    bool m_mockConfigurationResult;
    bool m_mockThemeApplicationSuccessful;
    
    // Call tracking
    int m_applyThemeCallCount;
    int m_setConfigurationCallCount;
    BaseTheme* m_lastAppliedTheme;
    QVariantMap m_lastConfiguration;
    
    // Size mocking
    QSize m_mockSizeHint;
    QSize m_mockMinimumSize;
    QSize m_mockMaximumSize;
    
    // Event tracking
    int m_paintEventCount;
    int m_resizeEventCount;
};

#endif // MOCKWIDGET_H