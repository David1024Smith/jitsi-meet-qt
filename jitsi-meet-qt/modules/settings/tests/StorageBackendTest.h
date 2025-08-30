#ifndef STORAGEBACKENDTEST_H
#define STORAGEBACKENDTEST_H

#include <QObject>
#include <QTest>

class QTemporaryDir;

/**
 * @brief 存储后端测试类
 * 
 * 专门测试各种存储后端的功能，包括本地存储、云端存储和注册表存储。
 * 测试数据持久化、同步、性能和错误处理等方面。
 */
class StorageBackendTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // Local storage tests
    void testLocalStorageBasicOperations();
    void testLocalStoragePersistence();
    void testLocalStorageGroups();
    
    // Cloud storage tests
    void testCloudStorageOfflineMode();
    void testCloudStorageNetworkSimulation();
    void testCloudStorageConflictResolution();
    
    // Registry storage tests (Windows only)
    void testRegistryStorageWindows();
    void testRegistryStoragePermissions();
    
    // Cross-backend tests
    void testStorageBackendSwitching();
    void testStorageBackendPerformance();

private:
    QTemporaryDir* m_tempDir = nullptr;
};

#endif // STORAGEBACKENDTEST_H