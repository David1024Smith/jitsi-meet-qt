#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QDebug>
#include <QStringList>
#include <QDir>

/**
 * Simple resource verification utility
 * Tests that all resources are properly accessible
 */

bool verifyIcon(const QString& iconPath) {
    QIcon icon(iconPath);
    bool isValid = !icon.isNull();
    qDebug() << (isValid ? "✓" : "✗") << iconPath << (isValid ? "OK" : "FAILED");
    return isValid;
}

bool verifyStylesheet(const QString& stylePath) {
    QFile file(stylePath);
    bool canOpen = file.open(QIODevice::ReadOnly);
    if (canOpen) {
        qint64 size = file.size();
        file.close();
        qDebug() << "✓" << stylePath << "OK (" << size << "bytes)";
        return true;
    } else {
        qDebug() << "✗" << stylePath << "FAILED";
        return false;
    }
}

bool verifyImage(const QString& imagePath) {
    QFile file(imagePath);
    bool exists = file.exists();
    qDebug() << (exists ? "✓" : "✗") << imagePath << (exists ? "OK" : "FAILED");
    return exists;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== Jitsi Meet Qt Resource Verification ===\n";
    
    int totalTests = 0;
    int passedTests = 0;
    
    // Test icons
    qDebug() << "Testing Icons:";
    QStringList icons = {
        ":/icons/app.svg",
        ":/icons/settings.svg",
        ":/icons/about.svg",
        ":/icons/back.svg",
        ":/icons/recent.svg",
        ":/icons/microphone.svg",
        ":/icons/microphone-off.svg",
        ":/icons/camera.svg",
        ":/icons/camera-off.svg",
        ":/icons/screen-share.svg",
        ":/icons/chat.svg",
        ":/icons/participants.svg",
        ":/icons/phone-hangup.svg",
        ":/icons/send.svg",
        ":/icons/volume-up.svg",
        ":/icons/volume-off.svg",
        ":/icons/fullscreen.svg",
        ":/icons/fullscreen-exit.svg",
        ":/icons/dropdown.svg",
        ":/icons/dropdown-dark.svg",
        ":/icons/refresh.svg",
        ":/icons/close.svg",
        ":/icons/warning.svg",
        ":/icons/error.svg",
        ":/icons/success.svg",
        ":/icons/loading.svg",
        ":/icons/minimize.svg",
        ":/icons/maximize.svg",
        ":/icons/join.svg"
    };
    
    for (const QString& icon : icons) {
        totalTests++;
        if (verifyIcon(icon)) {
            passedTests++;
        }
    }
    
    qDebug() << "";
    
    // Test stylesheets
    qDebug() << "Testing Stylesheets:";
    QStringList stylesheets = {
        ":/styles/default.qss",
        ":/styles/dark.qss",
        ":/styles/modern.qss"
    };
    
    for (const QString& stylesheet : stylesheets) {
        totalTests++;
        if (verifyStylesheet(stylesheet)) {
            passedTests++;
        }
    }
    
    qDebug() << "";
    
    // Test images
    qDebug() << "Testing Images:";
    QStringList images = {
        ":/images/logo.svg",
        ":/images/placeholder.svg",
        ":/images/welcome-bg.svg",
        ":/images/conference-bg.svg",
        ":/images/pattern.svg"
    };
    
    for (const QString& image : images) {
        totalTests++;
        if (verifyImage(image)) {
            passedTests++;
        }
    }
    
    qDebug() << "";
    
    // Test specific functionality
    qDebug() << "Testing Functionality:";
    
    // Test QRC compilation
    totalTests++;
    QFile qrcTest(":/styles/default.qss");
    if (qrcTest.open(QIODevice::ReadOnly)) {
        QString content = qrcTest.readAll();
        qrcTest.close();
        if (content.contains("QApplication")) {
            qDebug() << "✓ QRC compilation OK";
            passedTests++;
        } else {
            qDebug() << "✗ QRC content invalid";
        }
    } else {
        qDebug() << "✗ QRC compilation FAILED";
    }
    
    // Test icon loading with QIcon
    totalTests++;
    QIcon testIcon(":/icons/app.svg");
    if (!testIcon.isNull() && !testIcon.availableSizes().isEmpty()) {
        qDebug() << "✓ QIcon loading OK";
        passedTests++;
    } else {
        qDebug() << "✗ QIcon loading FAILED";
    }
    
    // Test stylesheet application
    totalTests++;
    QFile styleFile(":/styles/default.qss");
    if (styleFile.open(QIODevice::ReadOnly)) {
        QString styleContent = styleFile.readAll();
        styleFile.close();
        
        // Try to apply the stylesheet
        app.setStyleSheet(styleContent);
        qDebug() << "✓ Stylesheet application OK";
        passedTests++;
    } else {
        qDebug() << "✗ Stylesheet application FAILED";
    }
    
    qDebug() << "";
    qDebug() << "=== Results ===";
    qDebug() << "Total tests:" << totalTests;
    qDebug() << "Passed:" << passedTests;
    qDebug() << "Failed:" << (totalTests - passedTests);
    qDebug() << "Success rate:" << QString::number((double)passedTests / totalTests * 100, 'f', 1) << "%";
    
    if (passedTests == totalTests) {
        qDebug() << "\n🎉 All resources verified successfully!";
        return 0;
    } else {
        qDebug() << "\n❌ Some resources failed verification.";
        return 1;
    }
}