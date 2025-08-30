#ifndef SCREENSHAREWIDGET_H
#define SCREENSHAREWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QProgressBar>
#include <QGroupBox>
#include "../interfaces/IScreenShareManager.h"

class ScreenShareManager;
class ScreenShareConfig;
class ScreenSelector;
class CapturePreview;

/**
 * @brief 屏幕共享主控制组件
 * 
 * 提供完整的屏幕共享控制界面
 */
class ScreenShareWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool sharing READ isSharing NOTIFY sharingChanged)
    Q_PROPERTY(QString currentSource READ currentSource NOTIFY currentSourceChanged)

public:
    explicit ScreenShareWidget(QWidget *parent = nullptr);
    virtual ~ScreenShareWidget();

    // 管理器接口
    void setScreenShareManager(ScreenShareManager* manager);
    ScreenShareManager* screenShareManager() const;

    // 状态查询接口
    bool isSharing() const;
    QString currentSource() const;
    IScreenShareManager::ManagerStatus status() const;

    // 配置接口
    void setConfiguration(ScreenShareConfig* config);
    ScreenShareConfig* configuration() const;

    // UI控制接口
    void setControlsEnabled(bool enabled);
    bool areControlsEnabled() const;
    void setPreviewEnabled(bool enabled);
    bool isPreviewEnabled() const;

public slots:
    /**
     * @brief 开始屏幕共享
     */
    void startSharing();

    /**
     * @brief 停止屏幕共享
     */
    void stopSharing();

    /**
     * @brief 暂停屏幕共享
     */
    void pauseSharing();

    /**
     * @brief 恢复屏幕共享
     */
    void resumeSharing();

    /**
     * @brief 刷新可用源
     */
    void refreshSources();

    /**
     * @brief 显示设置对话框
     */
    void showSettings();

signals:
    /**
     * @brief 共享状态改变信号
     * @param sharing 是否正在共享
     */
    void sharingChanged(bool sharing);

    /**
     * @brief 当前源改变信号
     * @param source 当前源标识
     */
    void currentSourceChanged(const QString& source);

    /**
     * @brief 开始共享请求信号
     */
    void startSharingRequested();

    /**
     * @brief 停止共享请求信号
     */
    void stopSharingRequested();

    /**
     * @brief 设置改变信号
     */
    void settingsChanged();

private slots:
    void onStartButtonClicked();
    void onStopButtonClicked();
    void onPauseButtonClicked();
    void onSourceSelectionChanged();
    void onQualitySliderChanged(int value);
    void onFrameRateChanged(int fps);
    void onManagerStatusChanged(IScreenShareManager::ManagerStatus status);
    void onManagerError(const QString& error);
    void onStatisticsUpdated(const QVariantMap& statistics);
    void onPreviewClicked();
    void onSettingsButtonClicked();

private:
    void setupUI();
    void setupControlsGroup();
    void setupSourceGroup();
    void setupQualityGroup();
    void setupStatisticsGroup();
    void setupPreviewGroup();
    void connectSignals();
    void updateUI();
    void updateControlButtons();
    void updateSourceList();
    void updateQualityControls();
    void updateStatistics();
    void updateStatusDisplay();

    class Private;
    Private* d;
};

#endif // SCREENSHAREWIDGET_H