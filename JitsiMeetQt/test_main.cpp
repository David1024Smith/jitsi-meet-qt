#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "测试应用程序启动";
    
    QWidget window;
    window.setWindowTitle("Jitsi Meet Qt 测试");
    window.resize(400, 300);
    
    QVBoxLayout *layout = new QVBoxLayout(&window);
    QLabel *label = new QLabel("Jitsi Meet Qt 应用程序正在运行！", &window);
    layout->addWidget(label);
    
    window.show();
    
    qDebug() << "测试窗口已显示";
    
    return app.exec();
}