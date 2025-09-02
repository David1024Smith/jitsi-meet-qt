#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <iostream>

int main(int argc, char *argv[])
{
    std::cout << "Starting simple Qt test..." << std::endl;
    qDebug() << "Qt Debug: Starting application";
    
    QApplication app(argc, argv);
    
    std::cout << "QApplication created successfully" << std::endl;
    qDebug() << "Qt Debug: QApplication created";
    
    QWidget window;
    window.setWindowTitle("Simple Qt Test");
    window.resize(300, 200);
    
    QVBoxLayout *layout = new QVBoxLayout(&window);
    QLabel *label = new QLabel("Qt is working!", &window);
    layout->addWidget(label);
    
    std::cout << "Window created, showing..." << std::endl;
    qDebug() << "Qt Debug: Showing window";
    
    window.show();
    
    std::cout << "Entering event loop..." << std::endl;
    qDebug() << "Qt Debug: Entering event loop";
    
    return app.exec();
}