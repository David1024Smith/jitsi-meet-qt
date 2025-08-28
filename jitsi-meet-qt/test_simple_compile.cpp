#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QtMultimedia/QMediaDevices>
#include <QtMultimedia/QCameraDevice>
#include <QtMultimedia/QAudioDevice>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QMainWindow window;
    window.setWindowTitle("Qt Multimedia Test");
    window.resize(600, 400);
    
    QWidget* centralWidget = new QWidget(&window);
    window.setCentralWidget(centralWidget);
    
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    
    // Test basic Qt functionality
    QLabel* titleLabel = new QLabel("Qt Multimedia Device Test", centralWidget);
    layout->addWidget(titleLabel);
    
    // Test device enumeration
    auto videoDevices = QMediaDevices::videoInputs();
    QLabel* videoLabel = new QLabel(QString("Video devices found: %1").arg(videoDevices.size()), centralWidget);
    layout->addWidget(videoLabel);
    
    for (const auto& device : videoDevices) {
        QLabel* deviceLabel = new QLabel(QString("- %1").arg(device.description()), centralWidget);
        layout->addWidget(deviceLabel);
    }
    
    auto audioInputDevices = QMediaDevices::audioInputs();
    QLabel* audioInputLabel = new QLabel(QString("Audio input devices found: %1").arg(audioInputDevices.size()), centralWidget);
    layout->addWidget(audioInputLabel);
    
    for (const auto& device : audioInputDevices) {
        QLabel* deviceLabel = new QLabel(QString("- %1").arg(device.description()), centralWidget);
        layout->addWidget(deviceLabel);
    }
    
    auto audioOutputDevices = QMediaDevices::audioOutputs();
    QLabel* audioOutputLabel = new QLabel(QString("Audio output devices found: %1").arg(audioOutputDevices.size()), centralWidget);
    layout->addWidget(audioOutputLabel);
    
    for (const auto& device : audioOutputDevices) {
        QLabel* deviceLabel = new QLabel(QString("- %1").arg(device.description()), centralWidget);
        layout->addWidget(deviceLabel);
    }
    
    // Test video widget creation
    QVideoWidget* videoWidget = new QVideoWidget(centralWidget);
    videoWidget->setMinimumSize(320, 240);
    layout->addWidget(videoWidget);
    
    QPushButton* closeButton = new QPushButton("Close", centralWidget);
    layout->addWidget(closeButton);
    
    QObject::connect(closeButton, &QPushButton::clicked, &app, &QApplication::quit);
    
    window.show();
    
    qDebug() << "Qt Multimedia test application started successfully";
    qDebug() << "Qt version:" << QT_VERSION_STR;
    
    return app.exec();
}