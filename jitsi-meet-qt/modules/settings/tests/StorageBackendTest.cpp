#include "StorageBackendTest.h"
#include "LocalStorage.h"
#include "CloudStorage.h"
#include "RegistryStorage.h"

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

void StorageBackendTest::initTestCase()
{
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
}

void StorageBackendTest::cleanupTestCase()
{
    delete m_tempDir;
}

void StorageBackendTest::init()
{
    // Setup for each test
}

void StorageBackendTest::cleanup()
{
    // Cleanup after each test
}

void StorageBackendTest::testLocalStorageBasicOperations()
{
    LocalStorage storage;
    QString testFile = m_tempDir->path() + "/test_local.ini";
    storage.setFilePath(testFile);
    
    QVERIFY(storage.initialize());
    
    // Test basic operations
    storage.setValue("test/string", "test_value");
    storage.setValue("test/integer", 42);
    storage.setValue("test/boolean", true);
    storage.setValue("test/double", 3.14159);
    
    QCOMPARE(storage.value("test/string").toString(), QString("test_value"));
    QCOMPARE(storage.value("test/integer").toInt(), 42);
    QCOMPARE(storage.value("test/boolean").toBool(), true);
    QCOMPARE(storage.value("test/double").toDouble(), 3.14159);
    
    // Test contains
    QVERIFY(storage.contains("test/string"));
    QVERIFY(!storage.contains("nonexistent/key"));
    
    // Test removal
    storage.remove("test/string");
    QVERIFY(!storage.contains("test/string"));
    
    // Test sync
    QVERIFY(storage.sync());
}

void StorageBackendTest::testLocalStoragePersistence()
{
    QString testFile = m_tempDir->path() + "/test_persistence.ini";
    
    // First storage instance
    {
        LocalStorage storage1;
        storage1.setFilePath(testFile);
        QVERIFY(storage1.initialize());
        
        storage1.setValue("persistent/key1", "value1");
        storage1.setValue("persistent/key2", 123);
        QVERIFY(storage1.sync());
    }
    
    // Second storage instance - should load persisted data
    {
        LocalStorage storage2;
        storage2.setFilePath(testFile);
        QVERIFY(storage2.initialize());
        
        QCOMPARE(storage2.value("persistent/key1").toString(), QString("value1"));
        QCOMPARE(storage2.value("persistent/key2").toInt(), 123);
    }
}

void StorageBackendTest::testLocalStorageGroups()
{
    LocalStorage storage;
    QString testFile = m_tempDir->path() + "/test_groups.ini";
    storage.setFilePath(testFile);
    
    QVERIFY(storage.initialize());
    
    // Set values in different groups
    storage.setValue("audio/volume", 75);
    storage.setValue("audio/muted", false);
    storage.setValue("video/resolution", "1920x1080");
    storage.setValue("video/fps", 30);
    storage.setValue("network/server", "https://meet.jit.si");
    
    // Test child keys
    QStringList audioKeys = storage.childKeys("audio");
    QVERIFY(audioKeys.contains("volume"));
    QVERIFY(audioKeys.contains("muted"));
    QCOMPARE(audioKeys.size(), 2);
    
    QStringList videoKeys = storage.childKeys("video");
    QVERIFY(videoKeys.contains("resolution"));
    QVERIFY(videoKeys.contains("fps"));
    QCOMPARE(videoKeys.size(), 2);
    
    // Test child groups
    QStringList groups = storage.childGroups("");
    QVERIFY(groups.contains("audio"));
    QVERIFY(groups.contains("video"));
    QVERIFY(groups.contains("network"));
    
    // Test all keys
    QStringList allKeys = storage.allKeys();
    QVERIFY(allKeys.contains("audio/volume"));
    QVERIFY(allKeys.contains("video/resolution"));
    QVERIFY(allKeys.contains("network/server"));
}

void StorageBackendTest::testCloudStorageOfflineMode()
{
    CloudStorage storage;
    
    // Test initialization in offline mode
    QVERIFY(storage.initialize());
    QCOMPARE(storage.syncStatus(), CloudStorage::Offline);
    
    // Test offline operations
    storage.setValue("cloud/test1", "offline_value1");
    storage.setValue("cloud/test2", 42);
    
    QCOMPARE(storage.value("cloud/test1").toString(), QString("offline_value1"));
    QCOMPARE(storage.value("cloud/test2").toInt(), 42);
    
    // Test local caching
    QVERIFY(storage.contains("cloud/test1"));
    QVERIFY(storage.contains("cloud/test2"));
    
    // Test sync in offline mode (should cache for later)
    QSignalSpy syncSpy(&storage, &CloudStorage::syncStatusChanged);
    storage.sync();
    
    // Should remain offline but queue changes
    QCOMPARE(storage.syncStatus(), CloudStorage::Offline);
    QVERIFY(storage.hasPendingChanges());
}

void StorageBackendTest::testCloudStorageNetworkSimulation()
{
    CloudStorage storage;
    storage.setServerUrl("http://test.server.com/api");
    storage.setAuthToken("test_token");
    
    QVERIFY(storage.initialize());
    
    // Test network error handling
    QSignalSpy errorSpy(&storage, &CloudStorage::errorOccurred);
    QSignalSpy statusSpy(&storage, &CloudStorage::syncStatusChanged);
    
    storage.setValue("network/test", "network_value");
    storage.sync();
    
    // Wait for network timeout
    QTest::qWait(2000);
    
    // Should handle network errors gracefully
    QVERIFY(errorSpy.count() > 0 || storage.syncStatus() == CloudStorage::Error);
}

void StorageBackendTest::testCloudStorageConflictResolution()
{
    CloudStorage storage;
    
    QVERIFY(storage.initialize());
    
    // Simulate conflict scenario
    storage.setValue("conflict/key", "local_value");
    
    // Simulate server value
    QVariantMap serverData;
    serverData["conflict/key"] = "server_value";
    storage.handleServerData(serverData);
    
    // Test conflict resolution strategy
    CloudStorage::ConflictResolution strategy = storage.conflictResolution();
    
    switch (strategy) {
    case CloudStorage::PreferLocal:
        QCOMPARE(storage.value("conflict/key").toString(), QString("local_value"));
        break;
    case CloudStorage::PreferServer:
        QCOMPARE(storage.value("conflict/key").toString(), QString("server_value"));
        break;
    case CloudStorage::Manual:
        // Should emit conflict signal
        QVERIFY(storage.hasConflicts());
        break;
    }
}

void StorageBackendTest::testRegistryStorageWindows()
{
#ifdef Q_OS_WIN
    RegistryStorage storage;
    storage.setRegistryPath("HKEY_CURRENT_USER/Software/JitsiMeetQt/Test");
    
    QVERIFY(storage.initialize());
    
    // Test registry operations
    storage.setValue("registry_test/string", "registry_value");
    storage.setValue("registry_test/integer", 999);
    storage.setValue("registry_test/boolean", true);
    
    QCOMPARE(storage.value("registry_test/string").toString(), QString("registry_value"));
    QCOMPARE(storage.value("registry_test/integer").toInt(), 999);
    QCOMPARE(storage.value("registry_test/boolean").toBool(), true);
    
    // Test persistence
    QVERIFY(storage.sync());
    
    // Create new instance to test persistence
    RegistryStorage storage2;
    storage2.setRegistryPath("HKEY_CURRENT_USER/Software/JitsiMeetQt/Test");
    QVERIFY(storage2.initialize());
    
    QCOMPARE(storage2.value("registry_test/string").toString(), QString("registry_value"));
    QCOMPARE(storage2.value("registry_test/integer").toInt(), 999);
    
    // Cleanup
    storage.remove("registry_test/string");
    storage.remove("registry_test/integer");
    storage.remove("registry_test/boolean");
    storage.sync();
#else
    QSKIP("Registry storage is Windows-only");
#endif
}

void StorageBackendTest::testRegistryStoragePermissions()
{
#ifdef Q_OS_WIN
    RegistryStorage storage;
    
    // Test with restricted path (should handle gracefully)
    storage.setRegistryPath("HKEY_LOCAL_MACHINE/SOFTWARE/Test");
    
    QSignalSpy errorSpy(&storage, &RegistryStorage::errorOccurred);
    bool result = storage.initialize();
    
    // Should either succeed or emit error
    if (!result) {
        QVERIFY(errorSpy.count() > 0);
    }
#else
    QSKIP("Registry storage is Windows-only");
#endif
}

void StorageBackendTest::testStorageBackendSwitching()
{
    // Test switching between storage backends
    QString testFile = m_tempDir->path() + "/backend_switch.ini";
    
    // Start with local storage
    LocalStorage localStorage;
    localStorage.setFilePath(testFile);
    QVERIFY(localStorage.initialize());
    
    localStorage.setValue("switch/test1", "local_value1");
    localStorage.setValue("switch/test2", 123);
    QVERIFY(localStorage.sync());
    
    // Export data
    QVariantMap exportedData = localStorage.exportAll();
    
    // Switch to cloud storage (offline mode)
    CloudStorage cloudStorage;
    QVERIFY(cloudStorage.initialize());
    
    // Import data
    cloudStorage.importAll(exportedData);
    
    // Verify data integrity
    QCOMPARE(cloudStorage.value("switch/test1").toString(), QString("local_value1"));
    QCOMPARE(cloudStorage.value("switch/test2").toInt(), 123);
}

void StorageBackendTest::testStorageBackendPerformance()
{
    LocalStorage storage;
    QString testFile = m_tempDir->path() + "/performance_test.ini";
    storage.setFilePath(testFile);
    
    QVERIFY(storage.initialize());
    
    QElapsedTimer timer;
    
    // Test write performance
    timer.start();
    for (int i = 0; i < 1000; ++i) {
        storage.setValue(QString("perf/key_%1").arg(i), QString("value_%1").arg(i));
    }
    qint64 writeTime = timer.elapsed();
    
    // Test read performance
    timer.restart();
    for (int i = 0; i < 1000; ++i) {
        QString value = storage.value(QString("perf/key_%1").arg(i)).toString();
        Q_UNUSED(value);
    }
    qint64 readTime = timer.elapsed();
    
    // Test sync performance
    timer.restart();
    QVERIFY(storage.sync());
    qint64 syncTime = timer.elapsed();
    
    // Performance assertions (adjust thresholds as needed)
    QVERIFY(writeTime < 2000); // 2 seconds for 1000 writes
    QVERIFY(readTime < 1000);  // 1 second for 1000 reads
    QVERIFY(syncTime < 3000);  // 3 seconds for sync
    
    qDebug() << "Performance results:";
    qDebug() << "Write time:" << writeTime << "ms";
    qDebug() << "Read time:" << readTime << "ms";
    qDebug() << "Sync time:" << syncTime << "ms";
}

QTEST_MAIN(StorageBackendTest)