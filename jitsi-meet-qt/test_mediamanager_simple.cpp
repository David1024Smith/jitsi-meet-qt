#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QVideoWidget>
#include <QDebug>
#include "include/MediaManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QMainWindow window;
    window.setWindowTitle("MediaManager Test");
    window.resize(800, 600);
    
    QWidget* centralWidget = new QWidget(&window);
    window.setCentralWidget(centralWidget);
    
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    
    // Create MediaManager
    MediaManager mediaManager;
    
    // Test device enumeration
    QLabel* statusLabel = new QLabel("Testing MediaManager...", centralWidget);
    layout->addWidget(statusLabel);
    
    // Video devices
    auto videoDevices = mediaManager.availableVideoDevices();
    QLabel* videoLabel = new QLabel(QString("Video devices found: %1").arg(videoDevices.size()), centralWidget);
    layout->addWidget(videoLabel);
    
    // Audio input devices
    auto audioInputDevices = mediaManager.availableAudioInputDevices();
    QLabel* audioInputLabel = new QLabel(QString("Audio input devices found: %1").arg(audioInputDevices.size()), centralWidget);
    layout->addWidget(audioInputLabel);
    
    // Audio output devices
    auto audioOutputDevices = mediaManager.availableAudioOutputDevices();
    QLabel* audioOutputLabel = new QLabel(QString("Audio output devices found: %1").arg(audioOutputDevices.size()), centralWidget);
    layout->addWidget(audioOutputLabel);
    
    // Test buttons
    QPushButton* startVideoBtn = new QPushButton("Start Video", centralWidget);
    QPushButton* stopVideoBtn = new QPushButton("Stop Video", centralWidget);
    QPushButton* startAudioBtn = new QPushButton("Start Audio", centralWidget);
    QPushButton* stopAudioBtn = new QPushButton("Stop Audio", centralWidget);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(startVideoBtn);
    buttonLayout->addWidget(stopVideoBtn);
    buttonLayout->addWidget(startAudioBtn);
    buttonLayout->addWidget(stopAudioBtn);
    
    layout->addLayout(buttonLayout);
    
    // Connect buttons
    QObject::connect(startVideoBtn, &QPushButton::clicked, [&]() {
        mediaManager.startLocalVideo();
        statusLabel->setText("Video started");
    });
    
    QObject::connect(stopVideoBtn, &QPushButton::clicked, [&]() {
        mediaManager.stopLocalVideo();
        statusLabel->setText("Video stopped");
    });
    
    QObject::connect(startAudioBtn, &QPushButton::clicked, [&]() {
        mediaManager.startLocalAudio();
        statusLabel->setText("Audio started");
    });
    
    QObject::connect(stopAudioBtn, &QPushButton::clicked, [&]() {
        mediaManager.stopLocalAudio();
        statusLabel->setText("Audio stopped");
    });
    
    window.show();
    
    qDebug() << "MediaManager test application started";
    
    return app.exec();
}