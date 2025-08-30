#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "../include/ProgressiveReplacementManager.h"
#include "../config/ReplacementConfig.h"

/**
 * @brief 渐进式代码替换使用示例
 * 
 * 演示如何使用ProgressiveReplacementManager进行安全的代码替换
 */
class ProgressiveReplacementExample : public QObject
{
    Q_OBJECT

public:
    ProgressiveReplacementExample(QObject *parent = nullptr)
        : QObject(parent)
        , m_manager(new ProgressiveReplacementManager(this))
        , m_config(new ReplacementConfig(this))
    {
        // 连接信号以监控替换进度
        connect(m_manager, &ProgressiveReplacementManager::replacementStarted,
                this, &ProgressiveReplacementExample::onReplacementStarted);
        connect(m_manager, &ProgressiveReplacementManager::replacementProgress,
                this, &ProgressiveReplacementExample::onReplacementProgress);
        connect(m_manager, &ProgressiveReplacementManager::replacementCompleted,
                this, &ProgressiveReplacementExample::onReplacementCompleted);
        connect(m_manager, &ProgressiveReplacementManager::replacementFailed,
                this, &ProgressiveReplacementExample::onReplacementFailed);
        connect(m_manager, &ProgressiveReplacementManager::validationFailed,
                this, &ProgressiveReplacementExample::onValidationFailed);
        connect(m_manager, &ProgressiveReplacementManager::performanceIssueDetected,
                this, &ProgressiveReplacementExample::onPerformanceIssueDetected);
    }

    void run()
    {
        qDebug() << "=== 渐进式代码替换示例 ===";
        
        // 1. 初始化管理器
        if (!m_manager->initialize()) {
            qCritical() << "Failed to initialize ProgressiveReplacementManager";
            return;
        }
        
        qDebug() << "ProgressiveReplacementManager initialized successfully";
        
        // 2. 设置全局策略
        m_manager->setGlobalStrategy(ProgressiveReplacementManager::Balanced);
        qDebug() << "Global strategy set to Balanced";
        
        // 3. 演示不同的替换策略
        demonstrateConservativeReplacement();
        demonstrateBalancedReplacement();
        demonstrateAggressiveReplacement();
        
        // 4. 演示并行运行模式
        demonstrateParallelMode();
        
        // 5. 演示批量替换
        demonstrateBatchReplacement();
        
        // 6. 演示调度替换
        demonstrateScheduledReplacement();
        
        // 7. 生成报告
        generateReports();
        
        qDebug() << "=== 示例完成 ===";
    }

private slots:
    void onReplacementStarted(const QString& moduleName)
    {
        qDebug() << "Replacement started for module:" << moduleName;
    }
    
    void onReplacementProgress(const QString& moduleName, int percentage)
    {
        qDebug() << "Replacement progress for" << moduleName << ":" << percentage << "%";
    }
    
    void onReplacementCompleted(const QString& moduleName, bool success)
    {
        qDebug() << "Replacement completed for" << moduleName << "- Success:" << success;
    }
    
    void onReplacementFailed(const QString& moduleName, const QString& error)
    {
        qWarning() << "Replacement failed for" << moduleName << "- Error:" << error;
    }
    
    void onValidationFailed(const QString& moduleName, const QString& reason)
    {
        qWarning() << "Validation failed for" << moduleName << "- Reason:" << reason;
    }
    
    void onPerformanceIssueDetected(const QString& moduleName, const QVariantMap& metrics)
    {
        qWarning() << "Performance issue detected for" << moduleName;
        qWarning() << "Metrics:" << metrics;
    }

private:
    void demonstrateConservativeReplacement()
    {
        qDebug() << "\n--- 保守策略替换示例 ---";
        
        QString moduleName = "chat_module";
        
        // 创建保守策略的替换计划
        ProgressiveReplacementManager::ReplacementPlan plan;
        plan.moduleName = moduleName;
        plan.strategy = ProgressiveReplacementManager::Conservative;
        plan.priority = 1;
        plan.requiresValidation = true;
        plan.requiresPerformanceTest = true;
        plan.dependencies = QStringList(); // 无依赖
        
        if (m_manager->createReplacementPlan(moduleName, plan)) {
            qDebug() << "Created conservative replacement plan for" << moduleName;
            
            // 开始替换
            if (m_manager->startReplacement(moduleName)) {
                qDebug() << "Started conservative replacement for" << moduleName;
                
                // 监控执行状态
                ProgressiveReplacementManager::ExecutionState state = m_manager->getExecutionState(moduleName);
                qDebug() << "Current phase:" << state.currentPhase;
                qDebug() << "Status:" << state.status;
                qDebug() << "Run mode:" << state.runMode;
            }
        }
    }
    
    void demonstrateBalancedReplacement()
    {
        qDebug() << "\n--- 平衡策略替换示例 ---";
        
        QString moduleName = "meeting_module";
        
        // 创建平衡策略的替换计划
        ProgressiveReplacementManager::ReplacementPlan plan;
        plan.moduleName = moduleName;
        plan.strategy = ProgressiveReplacementManager::Balanced;
        plan.priority = 2;
        plan.requiresValidation = true;
        plan.requiresPerformanceTest = true;
        plan.dependencies = QStringList{"chat_module"}; // 依赖聊天模块
        
        if (m_manager->createReplacementPlan(moduleName, plan)) {
            qDebug() << "Created balanced replacement plan for" << moduleName;
            
            // 启用并行模式
            if (m_manager->enableParallelMode(moduleName)) {
                qDebug() << "Enabled parallel mode for" << moduleName;
            }
            
            // 开始替换
            if (m_manager->startReplacement(moduleName)) {
                qDebug() << "Started balanced replacement for" << moduleName;
            }
        }
    }
    
    void demonstrateAggressiveReplacement()
    {
        qDebug() << "\n--- 激进策略替换示例 ---";
        
        QString moduleName = "ui_module";
        
        // 创建激进策略的替换计划
        ProgressiveReplacementManager::ReplacementPlan plan;
        plan.moduleName = moduleName;
        plan.strategy = ProgressiveReplacementManager::Aggressive;
        plan.priority = 3;
        plan.requiresValidation = false;
        plan.requiresPerformanceTest = false;
        plan.dependencies = QStringList(); // 无依赖
        
        if (m_manager->createReplacementPlan(moduleName, plan)) {
            qDebug() << "Created aggressive replacement plan for" << moduleName;
            
            // 开始替换
            if (m_manager->startReplacement(moduleName)) {
                qDebug() << "Started aggressive replacement for" << moduleName;
            }
        }
    }
    
    void demonstrateParallelMode()
    {
        qDebug() << "\n--- 并行运行模式示例 ---";
        
        QString moduleName = "network_module";
        
        // 创建替换计划
        ProgressiveReplacementManager::ReplacementPlan plan;
        plan.moduleName = moduleName;
        plan.strategy = ProgressiveReplacementManager::Balanced;
        
        if (m_manager->createReplacementPlan(moduleName, plan)) {
            qDebug() << "Created plan for parallel mode demonstration";
            
            // 演示不同的运行模式
            m_manager->setCodeRunMode(moduleName, ProgressiveReplacementManager::LegacyOnly);
            qDebug() << "Set run mode to LegacyOnly";
            
            m_manager->setCodeRunMode(moduleName, ProgressiveReplacementManager::Parallel);
            qDebug() << "Set run mode to Parallel";
            
            m_manager->setCodeRunMode(moduleName, ProgressiveReplacementManager::Comparison);
            qDebug() << "Set run mode to Comparison";
            
            m_manager->setCodeRunMode(moduleName, ProgressiveReplacementManager::NewOnly);
            qDebug() << "Set run mode to NewOnly";
            
            // 运行功能对比验证
            if (m_manager->runFunctionalComparison(moduleName)) {
                qDebug() << "Started functional comparison for" << moduleName;
            }
            
            // 运行性能基准测试
            if (m_manager->runPerformanceBenchmark(moduleName)) {
                qDebug() << "Started performance benchmark for" << moduleName;
            }
        }
    }
    
    void demonstrateBatchReplacement()
    {
        qDebug() << "\n--- 批量替换示例 ---";
        
        QStringList moduleNames = {"audio_module", "video_module", "settings_module"};
        
        // 为每个模块创建替换计划
        for (const QString& moduleName : moduleNames) {
            ProgressiveReplacementManager::ReplacementPlan plan;
            plan.moduleName = moduleName;
            plan.strategy = ProgressiveReplacementManager::Balanced;
            plan.priority = 1;
            
            if (m_manager->createReplacementPlan(moduleName, plan)) {
                qDebug() << "Created plan for" << moduleName;
            }
        }
        
        // 批量开始替换
        m_manager->batchReplacement(moduleNames);
        qDebug() << "Started batch replacement for" << moduleNames.size() << "modules";
        
        // 检查活跃替换
        QStringList activeReplacements = m_manager->getActiveReplacements();
        qDebug() << "Active replacements:" << activeReplacements;
    }
    
    void demonstrateScheduledReplacement()
    {
        qDebug() << "\n--- 调度替换示例 ---";
        
        QString moduleName = "performance_module";
        
        // 创建替换计划
        ProgressiveReplacementManager::ReplacementPlan plan;
        plan.moduleName = moduleName;
        plan.strategy = ProgressiveReplacementManager::Conservative;
        
        if (m_manager->createReplacementPlan(moduleName, plan)) {
            qDebug() << "Created plan for scheduled replacement";
            
            // 调度5秒后执行
            QDateTime scheduledTime = QDateTime::currentDateTime().addSecs(5);
            m_manager->scheduleReplacement(moduleName, scheduledTime);
            qDebug() << "Scheduled replacement for" << scheduledTime;
        }
    }
    
    void generateReports()
    {
        qDebug() << "\n--- 报告生成示例 ---";
        
        // 生成进度报告
        QVariantMap progressReport = m_manager->generateProgressReport();
        qDebug() << "Progress Report:";
        qDebug() << "  Total modules:" << progressReport["total_modules"];
        qDebug() << "  Active replacements:" << progressReport["active_replacements"];
        qDebug() << "  Completed replacements:" << progressReport["completed_replacements"];
        qDebug() << "  Failed replacements:" << progressReport["failed_replacements"];
        
        // 生成详细报告（以第一个模块为例）
        QStringList modules = m_manager->getPlannedModules();
        if (!modules.isEmpty()) {
            QString moduleName = modules.first();
            QVariantMap detailedReport = m_manager->generateDetailedReport(moduleName);
            qDebug() << "Detailed Report for" << moduleName << ":";
            qDebug() << "  Strategy:" << detailedReport["strategy"];
            qDebug() << "  Current phase:" << detailedReport["current_phase"];
            qDebug() << "  Status:" << detailedReport["status"];
            qDebug() << "  Progress:" << detailedReport["progress_percentage"] << "%";
        }
        
        // 显示替换历史
        QStringList history = m_manager->getReplacementHistory();
        qDebug() << "Replacement History (last 5 entries):";
        int count = qMin(5, history.size());
        for (int i = history.size() - count; i < history.size(); ++i) {
            qDebug() << "  " << history[i];
        }
    }

private:
    ProgressiveReplacementManager* m_manager;
    ReplacementConfig* m_config;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    ProgressiveReplacementExample example;
    
    // 使用定时器延迟执行，以便事件循环启动
    QTimer::singleShot(100, &example, &ProgressiveReplacementExample::run);
    
    // 5秒后退出应用程序
    QTimer::singleShot(10000, &app, &QCoreApplication::quit);
    
    return app.exec();
}

#include "progressive_replacement_example.moc"