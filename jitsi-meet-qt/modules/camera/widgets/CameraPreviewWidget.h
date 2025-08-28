#ifndef CAMERAPREVIEWWIDGET_H
#define CAMERAPREVIEWWIDGET_H

#include <QWidget>
#include <QVideoWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QTimer>
#include <QProgressBar>
#include "../interfaces/ICameraDevice.h"
#include "../interfaces/ICameraManager.h"

/**
 * @brief 摄像头预览组件
 * 
 * 提供完整的摄像头预览界面，包括视频显示、控制按钮、设置面板等
 */
class CameraPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 显示模式
     */
    enum DisplayMode {
        VideoOnly,          ///< 仅视频
        VideoWithControls,  ///< 视频+基本控制
        FullInterface      ///< 完整界面
    };
    Q_ENUM(DisplayMode)

    explicit CameraPreviewWidget(QWidget* parent = nullptr);
    ~CameraPreviewWidget();

    // 摄像头管理器设置
    void setCameraManager(ICameraManager* manager);
    ICameraManager* cameraManager() const;

    // 显示模式
    void setDisplayMode(DisplayMode mode);
    DisplayMode displayMode() const;

    // 视频组件访问
    QVideoWidget* videoWidget() const;

    // 控制接口
    bool isControlsVisible() const;
    void setControlsVisible(bool visible);

    bool isStatusVisible() const;
    void setStatusVisible(bool visible);

    // 尺寸控制
    void setPreviewSize(const QSize& size);
    QSize previewSize() const;

    // 状态查询
    bool isCameraActive() const;
    QString currentDeviceName() const;
    QSize currentResolution() const;
    int currentFrameRate() const;

public slots:
    /**
     * @brief 启动摄像头预览
     */
    void startPreview();

    /**
     * @brief 停止摄像头预览
     */
    void stopPreview();

    /**
     * @brief 切换摄像头状态
     */
    void toggleCamera();

    /**
     * @brief 刷新设备列表
     */
    void refreshDevices();

    /**
     * @brief 应用质量预设
     */
    void applyQualityPreset(ICameraDevice::QualityPreset preset);

    /**
     * @brief 截图
     */
    void takeSnapshot();

signals:
    /**
     * @brief 摄像头状态改变
     */
    void cameraStatusChanged(bool active);

    /**
     * @brief 设备改变
     */
    void deviceChanged(const QString& deviceId);

    /**
     * @brief 质量设置改变
     */
    void qualityChanged(ICameraDevice::QualityPreset preset);

    /**
     * @brief 截图完成
     */
    void snapshotTaken(const QPixmap& snapshot);

    /**
     * @brief 错误发生
     */
    void errorOccurred(const QString& error);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onCameraManagerStatusChanged(ICameraManager::ManagerStatus status);
    void onCameraStarted();
    void onCameraStopped();
    void onDeviceSelectionChanged();
    void onQualityPresetChanged();
    void onCameraManagerError(const QString& error);
    void updateStatusInfo();
    void updateFrameRate();

private:
    void setupUI();
    void setupVideoWidget();
    void setupControls();
    void setupStatusBar();
    void connectSignals();
    void updateControlsVisibility();
    void updateDeviceList();
    void updateQualitySettings();
    void showError(const QString& error);
    void showStatus(const QString& status, int timeout = 3000);

    // UI组件
    QVBoxLayout* m_mainLayout;
    QVideoWidget* m_videoWidget;
    
    // 控制面板
    QWidget* m_controlsWidget;
    QHBoxLayout* m_controlsLayout;
    QPushButton* m_startStopButton;
    QPushButton* m_snapshotButton;
    QPushButton* m_refreshButton;
    QComboBox* m_deviceComboBox;
    QComboBox* m_qualityComboBox;
    
    // 状态栏
    QWidget* m_statusWidget;
    QHBoxLayout* m_statusLayout;
    QLabel* m_statusLabel;
    QLabel* m_resolutionLabel;
    QLabel* m_frameRateLabel;
    QProgressBar* m_performanceBar;
    
    // 设置
    DisplayMode m_displayMode;
    QSize m_previewSize;
    
    // 摄像头管理
    ICameraManager* m_cameraManager;
    
    // 状态更新
    QTimer* m_statusUpdateTimer;
    QTimer* m_frameRateTimer;
    int m_frameCount;
    
    // 样式
    QString m_errorStyleSheet;
    QString m_normalStyleSheet;
};

#endif // CAMERAPREVIEWWIDGET_H