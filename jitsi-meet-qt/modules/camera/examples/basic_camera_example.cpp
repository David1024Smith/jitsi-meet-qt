#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QVideoWidget>
#include <QMessageBox>

#include "../include/CameraFactory.h"
#include "../include/CameraManager.h"
#include "../widgets/CameraPreviewWidget.h"
#include "../config/CameraConfig.h"

/**
 * @brief 基础摄像头示例应用
 * 
 * 演示如何使用摄像头模块的基本功能
 */
class BasicCameraExample : public QMainWindow
{
    Q_OBJECT

public:
    BasicCameraExample(QWidget* parent = nullptr)
        : QMainWindow(parent)
        , m_cameraManager(nullptr)
        , m_previewWidget(nullptr)
    {
        setupUI();
        setupCamera();
        connectSignals();
    }

    ~BasicCameraExample()
    {
        if (m_cameraManager) {
            m_cameraManager->stopCamera();
            m_cameraManager->cleanup();
            CameraFactory::instance()->destroyCamera(m_cameraManager);
        }
    }

private slots:
    void onStartStopClicked()
    {
        if (!m_cameraManager) {
            return;
        }

        if (m_cameraManager->isCameraActive()) {
            m_cameraManager->stopCamera();
            m_startStopButton->setText("Start Camera");
        } else {
            if (m_cameraManager->startWithPreset(ICameraDevice::StandardQuality)) {
                m_startStopButton->setText("Stop Camera");
            } else {
                QMessageBox::warning(this, "Error", "Failed to start camera");
            }
        }
    }

    void onDeviceChanged()
    {
        QString deviceId = m_deviceComboBox->currentData().toString();
        if (!deviceId.isEmpty() && m_cameraManager) {
            m_cameraManager->selectDevice(deviceId);
        }
    }

    void onQualityChanged()
    {
        ICameraDevice::QualityPreset preset = 
            static_cast<ICameraDevice::QualityPreset>(m_qualityComboBox->currentData().toInt());
        
        if (m_cameraManager && m_cameraManager->isCameraActive()) {
            // 重启摄像头以应用新质量设置
            m_cameraManager->stopCamera();
            m_cameraManager->startWithPreset(preset);
        }
    }

    void onCameraStarted()
    {
        m_statusLabel->setText("Camera: Active");
        m_startStopButton->setText("Stop Camera");
        
        // 更新分辨率显示
        QSize resolution = m_cameraManager->currentResolution();
        m_resolutionLabel->setText(QString("Resolution: %1x%2")
                                  .arg(resolution.width())
                                  .arg(resolution.height()));
    }

    void onCameraStopped()
    {
        m_statusLabel->setText("Camera: Stopped");
        m_startStopButton->setText("Start Camera");
        m_resolutionLabel->setText("Resolution: N/A");
    }

    void onCameraError(const QString& error)
    {
        QMessageBox::critical(this, "Camera Error", error);
        m_statusLabel->setText("Camera: Error");
    }

    void onDevicesUpdated(const QStringList& devices)
    {
        m_deviceComboBox->clear();
        
        for (const QString& deviceId : devices) {
            // 这里应该获取设备的友好名称
            QString friendlyName = QString("Camera %1").arg(deviceId.left(8));
            m_deviceComboBox->addItem(friendlyName, deviceId);
        }
        
        if (!devices.isEmpty()) {
            m_cameraManager->selectDevice(devices.first());
        }
    }

private:
    void setupUI()
    {
        setWindowTitle("Basic Camera Example");
        setMinimumSize(800, 600);

        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

        // 预览区域
        m_previewWidget = new CameraPreviewWidget(this);
        m_previewWidget->setDisplayMode(CameraPreviewWidget::VideoOnly);
        mainLayout->addWidget(m_previewWidget, 1);

        // 控制面板
        QWidget* controlPanel = new QWidget();
        QHBoxLayout* controlLayout = new QHBoxLayout(controlPanel);

        // 设备选择
        controlLayout->addWidget(new QLabel("Device:"));
        m_deviceComboBox = new QComboBox();
        controlLayout->addWidget(m_deviceComboBox);

        // 质量选择
        controlLayout->addWidget(new QLabel("Quality:"));
        m_qualityComboBox = new QComboBox();
        m_qualityComboBox->addItem("Low Quality", static_cast<int>(ICameraDevice::LowQuality));
        m_qualityComboBox->addItem("Standard Quality", static_cast<int>(ICameraDevice::StandardQuality));
        m_qualityComboBox->addItem("High Quality", static_cast<int>(ICameraDevice::HighQuality));
        m_qualityComboBox->addItem("Ultra Quality", static_cast<int>(ICameraDevice::UltraQuality));
        m_qualityComboBox->setCurrentIndex(1); // Standard Quality
        controlLayout->addWidget(m_qualityComboBox);

        // 启动/停止按钮
        m_startStopButton = new QPushButton("Start Camera");
        controlLayout->addWidget(m_startStopButton);

        controlLayout->addStretch();
        mainLayout->addWidget(controlPanel);

        // 状态栏
        QWidget* statusPanel = new QWidget();
        QHBoxLayout* statusLayout = new QHBoxLayout(statusPanel);

        m_statusLabel = new QLabel("Camera: Stopped");
        statusLayout->addWidget(m_statusLabel);

        m_resolutionLabel = new QLabel("Resolution: N/A");
        statusLayout->addWidget(m_resolutionLabel);

        statusLayout->addStretch();
        mainLayout->addWidget(statusPanel);
    }

    void setupCamera()
    {
        // 创建摄像头管理器
        m_cameraManager = CameraFactory::instance()->createLocalCamera();
        
        if (!m_cameraManager) {
            QMessageBox::critical(this, "Error", "Failed to create camera manager");
            return;
        }

        // 初始化摄像头管理器
        if (!m_cameraManager->initialize()) {
            QMessageBox::critical(this, "Error", "Failed to initialize camera manager");
            return;
        }

        // 设置预览组件
        m_previewWidget->setCameraManager(m_cameraManager);

        // 刷新设备列表
        m_cameraManager->refreshDevices();
    }

    void connectSignals()
    {
        // 按钮信号
        connect(m_startStopButton, &QPushButton::clicked,
                this, &BasicCameraExample::onStartStopClicked);

        // 组合框信号
        connect(m_deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &BasicCameraExample::onDeviceChanged);
        connect(m_qualityComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &BasicCameraExample::onQualityChanged);

        // 摄像头管理器信号
        if (m_cameraManager) {
            connect(m_cameraManager, &ICameraManager::cameraStarted,
                    this, &BasicCameraExample::onCameraStarted);
            connect(m_cameraManager, &ICameraManager::cameraStopped,
                    this, &BasicCameraExample::onCameraStopped);
            connect(m_cameraManager, &ICameraManager::errorOccurred,
                    this, &BasicCameraExample::onCameraError);
            connect(m_cameraManager, &ICameraManager::devicesUpdated,
                    this, &BasicCameraExample::onDevicesUpdated);
        }
    }

private:
    // UI组件
    CameraPreviewWidget* m_previewWidget;
    QComboBox* m_deviceComboBox;
    QComboBox* m_qualityComboBox;
    QPushButton* m_startStopButton;
    QLabel* m_statusLabel;
    QLabel* m_resolutionLabel;

    // 摄像头组件
    CameraManager* m_cameraManager;
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("Basic Camera Example");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Jitsi Meet Qt");

    // 创建并显示主窗口
    BasicCameraExample window;
    window.show();

    return app.exec();
}

#include "basic_camera_example.moc"