#ifndef STATUSBAR_H
#define STATUSBAR_H

#include "BaseWidget.h"
#include <QStatusBar>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>

/**
 * @brief 自定义状态栏组件
 * 
 * StatusBar继承自QStatusBar，提供增强的状态栏功能，
 * 包括状态指示器、进度显示和主题集成。
 */
class StatusBar : public QStatusBar
{
    Q_OBJECT
    Q_PROPERTY(QString statusText READ statusText WRITE setStatusText NOTIFY statusTextChanged)
    Q_PROPERTY(StatusType statusType READ statusType WRITE setStatusType NOTIFY statusTypeChanged)
    Q_PROPERTY(bool progressVisible READ isProgressVisible WRITE setProgressVisible NOTIFY progressVisibleChanged)
    Q_PROPERTY(int progressValue READ progressValue WRITE setProgressValue NOTIFY progressValueChanged)

public:
    enum StatusType {
        InfoStatus,
        SuccessStatus,
        WarningStatus,
        ErrorStatus,
        BusyStatus
    };
    Q_ENUM(StatusType)

    explicit StatusBar(QWidget *parent = nullptr);
    ~StatusBar() override;

    // 状态文本
    QString statusText() const;
    void setStatusText(const QString& text);
    void showMessage(const QString& message, int timeout = 0);
    void showMessage(const QString& message, StatusType type, int timeout = 0);

    // 状态类型
    StatusType statusType() const;
    void setStatusType(StatusType type);

    // 进度显示
    bool isProgressVisible() const;
    void setProgressVisible(bool visible);
    int progressValue() const;
    void setProgressValue(int value);
    void setProgressRange(int minimum, int maximum);
    void showProgress(const QString& text = QString());
    void hideProgress();

    // 状态指示器
    void showConnectionStatus(bool connected);
    void showNetworkQuality(int quality); // 0-100
    void showRecordingStatus(bool recording);
    void showMuteStatus(bool muted);

    // 主题支持
    void applyTheme(std::shared_ptr<BaseTheme> theme);

    // 配置管理
    QVariantMap getConfiguration() const;
    void setConfiguration(const QVariantMap& config);

    // 组件信息
    QString componentName() const;

signals:
    void statusTextChanged(const QString& text);
    void statusTypeChanged(StatusType type);
    void progressVisibleChanged(bool visible);
    void progressValueChanged(int value);
    void connectionStatusChanged(bool connected);
    void networkQualityChanged(int quality);
    void recordingStatusChanged(bool recording);
    void muteStatusChanged(bool muted);

protected:
    // 主题相关
    void onThemeChanged(std::shared_ptr<BaseTheme> theme);
    QString getDefaultStyleSheet() const;
    void updateThemeColors();
    void updateThemeFonts();

    // 配置相关
    QVariantMap getDefaultConfiguration() const;
    bool validateConfiguration(const QVariantMap& config) const;

    // 事件处理
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onMessageTimeout();
    void onProgressUpdate();

private:
    void setupStatusBar();
    void setupWidgets();
    void updateStatusDisplay();
    void updateProgressDisplay();
    void updateIndicators();
    QString getStatusIcon(StatusType type) const;
    QString getStatusColor(StatusType type) const;
    void arrangeWidgets();

    // 状态信息
    QString m_statusText;
    StatusType m_statusType;
    
    // 进度信息
    bool m_progressVisible;
    int m_progressValue;
    int m_progressMinimum;
    int m_progressMaximum;
    
    // 状态指示器
    bool m_connectionStatus;
    int m_networkQuality;
    bool m_recordingStatus;
    bool m_muteStatus;

    // UI组件
    QLabel* m_statusLabel;
    QLabel* m_statusIcon;
    QProgressBar* m_progressBar;
    QLabel* m_connectionIndicator;
    QLabel* m_networkIndicator;
    QLabel* m_recordingIndicator;
    QLabel* m_muteIndicator;

    // 定时器
    QTimer* m_messageTimer;
    QTimer* m_progressTimer;

    std::shared_ptr<BaseTheme> m_currentTheme;
};

#endif // STATUSBAR_H