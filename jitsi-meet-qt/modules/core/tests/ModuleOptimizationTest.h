#ifndef MODULEOPTIMIZATIONTEST_H
#define MODULEOPTIMIZATIONTEST_H

#include <QObject>
#include <QTest>
#include <QElapsedTimer>

class ModuleCommunicationBus;
class ModuleResourceManager;
class ModuleStartupOptimizer;
class ModulePerformanceIntegrator;

/**
 * @brief 模块优化系统测试类
 * 
 * 测试所有优化组件的功能和性能：
 * - 通信总线优化
 * - 资源管理优化
 * - 启动优化
 * - 性能集成
 */
class ModuleOptimizationTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // 通信优化测试
    void testCommunicationBusPerformance();
    void testCommunicationOptimization();
    
    // 资源管理优化测试
    void testResourceManagerOptimization();
    void testMemoryOptimization();
    
    // 启动优化测试
    void testStartupOptimization();
    
    // 性能集成测试
    void testPerformanceIntegration();
    void testFullSystemOptimization();

private:
    ModuleCommunicationBus* m_communicationBus;
    ModuleResourceManager* m_resourceManager;
    ModuleStartupOptimizer* m_startupOptimizer;
    ModulePerformanceIntegrator* m_performanceIntegrator;
};

#endif // MODULEOPTIMIZATIONTEST_H