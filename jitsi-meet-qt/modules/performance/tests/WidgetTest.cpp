#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QTimer>
#include <QDebug>
#include "PerformanceWidget.h"
#include "MetricsChart.h"
#include "PerformanceConfig.h"
#include "PerformanceManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Create main window
    QWidget mainWindow;
    mainWindow.setWindowTitle("Performance Widget Test");
    mainWindow.resize(800, 600);
    
    QVBoxLayout* layout = new QVBoxLayout(&mainWindow);
    
    // Create configuration
    PerformanceConfig* config = new PerformanceConfig(&mainWindow);
    
    // Create performance widget
    PerformanceWidget* perfWidget = new PerformanceWidget(&mainWindow);
    perfWidget->setConfig(config);
    
    layout->addWidget(perfWidget);
    
    // Create a simple metrics chart
    MetricsChart* chart = new MetricsChart(&mainWindow);
    chart->setMetricType(MetricsChart::CPUUsage);
    chart->setChartTitle("CPU Usage Test");
    
    layout->addWidget(chart);
    
    // Add some test data to the chart
    QTimer* dataTimer = new QTimer(&mainWindow);
    QObject::connect(dataTimer, &QTimer::timeout, [chart]() {
        static int counter = 0;
        double value = 50 + 30 * qSin(counter * 0.1); // Simulate CPU usage
        chart->addDataPoint(QDateTime::currentDateTime(), value);
        counter++;
    });
    
    dataTimer->start(1000); // Add data every second
    
    // Start real-time updates
    perfWidget->startRealTimeUpdate();
    chart->startRealTimeUpdate();
    
    mainWindow.show();
    
    qDebug() << "Performance Widget Test started";
    qDebug() << "Configuration loaded:" << config->isMonitoringEnabled();
    qDebug() << "Update interval:" << config->chartUpdateInterval();
    
    return app.exec();
}