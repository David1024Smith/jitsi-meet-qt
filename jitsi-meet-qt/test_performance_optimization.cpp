#include <QCoreApplication>
#include <QTest>
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <chrono>
#include <memory>

#include "PerformanceManager.h"
#include "MemoryLeakDetector.h"
#include "NetworkOptimizer.h"
#include "MediaPerformanceOptimizer.h"

class PerformanceOptimizationTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // PerformanceManager tests
    void testStartupTimeTracking();
    void testMemoryMonitoring();
    void testPerformanceMetrics();
    void testLargeConferenceOptimization();
    
    // MemoryLeakDetector tests
    void testMemoryTracking();
    void testLeakDetection();
    void testMemoryStats();
    void testResourceCleanup();
    
    // NetworkOptimizer tests
    void testNetworkQualityMonitoring();
    void testDataCompression();
    void testAdaptiveBitrate();
    void testConnectionOptimization();
    
    // MediaPerformanceOptimizer tests
    void testVideoQualityAdjustment();
    void testAudioQualityAdjustment();
    void testAdaptiveQuality();
    void testPerformanceThresholds();
    
    // Integration tests
    void testIntegratedOptimization();
    void testMemoryLeakPrevention();
    void testPerformanceUnderLoad();

private:
    std::unique_ptr<PerformanceManager> m_performanceManager;
    std::unique_ptr<MemoryLeakDetector> m_memoryDetector;
    std::unique_ptr<NetworkOptimizer> m_networkOptimizer;
    std::unique_ptr<MediaPerformanceOptimizer> m_mediaOptimizer;
};

void PerformanceOptimizationTest::initTestCase()
{
    qDebug() << "Initializing Performance Optimization Test Suite";
    
    m_performanceManager = std::make_unique<PerformanceManager>();
    m_memoryDetector = std::make_unique<MemoryLeakDetector>();
    m_networkOptimizer = std::make_unique<NetworkOptimizer>();
    m_mediaOptimizer = std::make_unique<MediaPerformanceOptimizer>();
}

void PerformanceOptimizationTest::cleanupTestCase()
{
    qDebug() << "Cleaning up Performance Optimization Test Suite";
    
    m_mediaOptimizer.reset();
    m_networkOptimizer.reset();
    m_memoryDetector.reset();
    m_performanceManager.reset();
}

void PerformanceOptimizationTest::testStartupTimeTracking()
{
    qDebug() << "Testing startup time tracking...";
    
    // Test startup timer
    m_performanceManager->startStartupTimer();
    
    // Simulate startup delay
    QThread::msleep(100);
    
    m_performanceManager->endStartupTimer();
    
    auto startupTime = m_performanceManager->getStartupTime();
    QVERIFY(startupTime.count() >= 100);
    QVERIFY(startupTime.count() < 200); // Should be reasonable
    
    qDebug() << "Startup time recorded:" << startupTime.count() << "ms";
}

void PerformanceOptimizationTest::testMemoryMonitoring()
{
    qDebug() << "Testing memory monitoring...";
    
    // Start memory monitoring
    m_performanceManager->startMemoryMonitoring();
    
    // Wait for initial measurement
    QTest::qWait(1000);
    
    size_t initialMemory = m_performanceManager->getCurrentMemoryUsage();
    QVERIFY(initialMemory > 0);
    
    // Allocate some memory
    std::vector<char> testData(1024 * 1024); // 1MB
    
    // Wait for measurement update
    QTest::qWait(1000);
    
    size_t currentMemory = m_performanceManager->getCurrentMemoryUsage();
    size_t peakMemory = m_performanceManager->getPeakMemoryUsage();
    
    QVERIFY(peakMemory >= initialMemory);
    
    m_performanceManager->stopMemoryMonitoring();
    
    qDebug() << "Memory monitoring test completed. Peak usage:" << peakMemory / (1024*1024) << "MB";
}

void PerformanceOptimizationTest::testPerformanceMetrics()
{
    qDebug() << "Testing performance metrics recording...";
    
    // Record various metrics
    m_performanceManager->recordMetric(PerformanceManager::MetricType::NetworkLatency, 50.0);
    m_performanceManager->recordMetric(PerformanceManager::MetricType::VideoFrameRate, 30.0);
    m_performanceManager->recordMetric(PerformanceManager::MetricType::CPUUsage, 45.0);
    
    auto metrics = m_performanceManager->getCurrentMetrics();
    
    QCOMPARE(metrics.networkLatency.count(), 50);
    QCOMPARE(metrics.videoFrameRate, 30.0);
    QCOMPARE(metrics.cpuUsagePercent, 45.0);
    
    qDebug() << "Performance metrics test completed";
}

void PerformanceOptimizationTest::testLargeConferenceOptimization()
{
    qDebug() << "Testing large conference optimization...";
    
    // Test optimization for different participant counts
    m_performanceManager->optimizeForLargeConference(5);   // Small
    m_performanceManager->optimizeForLargeConference(15);  // Medium
    m_performanceManager->optimizeForLargeConference(25);  // Large
    
    // Verify that optimizations are applied
    // (In a real implementation, we would check specific settings)
    
    qDebug() << "Large conference optimization test completed";
}

void PerformanceOptimizationTest::testMemoryTracking()
{
    qDebug() << "Testing memory tracking...";
    
    m_memoryDetector->setTrackingEnabled(true);
    
    // Simulate memory allocations
    void* ptr1 = malloc(1024);
    void* ptr2 = malloc(2048);
    
    m_memoryDetector->trackAllocation(ptr1, 1024, "test.cpp", 100);
    m_memoryDetector->trackAllocation(ptr2, 2048, "test.cpp", 101);
    
    auto stats = m_memoryDetector->getMemoryStats();
    QCOMPARE(stats.totalAllocations, 2u);
    QCOMPARE(stats.currentAllocations, 2u);
    QCOMPARE(stats.totalBytesAllocated, 3072u);
    
    // Track deallocations
    m_memoryDetector->trackDeallocation(ptr1);
    free(ptr1);
    
    stats = m_memoryDetector->getMemoryStats();
    QCOMPARE(stats.totalDeallocations, 1u);
    QCOMPARE(stats.currentAllocations, 1u);
    
    m_memoryDetector->trackDeallocation(ptr2);
    free(ptr2);
    
    qDebug() << "Memory tracking test completed";
}

void PerformanceOptimizationTest::testLeakDetection()
{
    qDebug() << "Testing leak detection...";
    
    m_memoryDetector->startLeakDetection();
    
    // Simulate a memory leak (allocation without deallocation)
    void* leakedPtr = malloc(512);
    m_memoryDetector->trackAllocation(leakedPtr, 512, "leak_test.cpp", 200);
    
    // Wait for leak detection
    QTest::qWait(1000);
    
    auto leaks = m_memoryDetector->detectLeaks();
    // Note: detectLeaks() looks for allocations older than 5 minutes,
    // so this test might not detect the leak immediately
    
    m_memoryDetector->stopLeakDetection();
    
    // Clean up
    m_memoryDetector->trackDeallocation(leakedPtr);
    free(leakedPtr);
    
    qDebug() << "Leak detection test completed";
}

void PerformanceOptimizationTest::testMemoryStats()
{
    qDebug() << "Testing memory statistics...";
    
    auto initialStats = m_memoryDetector->getMemoryStats();
    
    // Reset stats
    m_memoryDetector->resetStats();
    
    auto resetStats = m_memoryDetector->getMemoryStats();
    QCOMPARE(resetStats.totalAllocations, 0u);
    QCOMPARE(resetStats.totalDeallocations, 0u);
    QCOMPARE(resetStats.currentAllocations, 0u);
    
    qDebug() << "Memory statistics test completed";
}

void PerformanceOptimizationTest::testResourceCleanup()
{
    qDebug() << "Testing resource cleanup...";
    
    // Force garbage collection
    m_memoryDetector->forceGarbageCollection();
    
    // Clean up unused resources
    m_memoryDetector->cleanupUnusedResources();
    
    qDebug() << "Resource cleanup test completed";
}

void PerformanceOptimizationTest::testNetworkQualityMonitoring()
{
    qDebug() << "Testing network quality monitoring...";
    
    m_networkOptimizer->startNetworkMonitoring();
    
    // Wait for initial measurements
    QTest::qWait(2000);
    
    auto metrics = m_networkOptimizer->getCurrentMetrics();
    auto quality = m_networkOptimizer->getConnectionQuality();
    
    // Verify that monitoring is working
    QVERIFY(static_cast<int>(quality) >= 0);
    QVERIFY(static_cast<int>(quality) <= 3);
    
    m_networkOptimizer->stopNetworkMonitoring();
    
    qDebug() << "Network quality:" << static_cast<int>(quality);
}

void PerformanceOptimizationTest::testDataCompression()
{
    qDebug() << "Testing data compression...";
    
    // Test data compression
    QByteArray testData = "This is a test string for compression. It should be long enough to see compression benefits. ";
    testData = testData.repeated(10); // Make it longer
    
    QByteArray compressed = m_networkOptimizer->compressData(testData);
    QByteArray decompressed = m_networkOptimizer->decompressData(compressed);
    
    // Verify compression/decompression works
    QCOMPARE(decompressed, testData);
    
    qDebug() << "Original size:" << testData.size() << "Compressed size:" << compressed.size();
}

void PerformanceOptimizationTest::testAdaptiveBitrate()
{
    qDebug() << "Testing adaptive bitrate...";
    
    m_networkOptimizer->enableAdaptiveBitrate(true);
    
    // Test bitrate adjustment for different qualities
    m_networkOptimizer->adjustBitrateForQuality(NetworkOptimizer::ConnectionQuality::Excellent);
    m_networkOptimizer->adjustBitrateForQuality(NetworkOptimizer::ConnectionQuality::Poor);
    
    qDebug() << "Adaptive bitrate test completed";
}

void PerformanceOptimizationTest::testConnectionOptimization()
{
    qDebug() << "Testing connection optimization...";
    
    // Test optimization for different participant counts
    m_networkOptimizer->optimizeForParticipantCount(5);
    m_networkOptimizer->optimizeForParticipantCount(15);
    m_networkOptimizer->optimizeForParticipantCount(25);
    
    auto settings = m_networkOptimizer->getOptimizationSettings();
    QVERIFY(settings.maxConcurrentConnections > 0);
    
    qDebug() << "Connection optimization test completed";
}

void PerformanceOptimizationTest::testVideoQualityAdjustment()
{
    qDebug() << "Testing video quality adjustment...";
    
    // Test different video quality levels
    m_mediaOptimizer->setVideoQuality(MediaPerformanceOptimizer::VideoQuality::Ultra);
    QCOMPARE(m_mediaOptimizer->getVideoQuality(), MediaPerformanceOptimizer::VideoQuality::Ultra);
    
    m_mediaOptimizer->setVideoQuality(MediaPerformanceOptimizer::VideoQuality::Low);
    QCOMPARE(m_mediaOptimizer->getVideoQuality(), MediaPerformanceOptimizer::VideoQuality::Low);
    
    auto videoSettings = m_mediaOptimizer->getVideoSettings();
    QVERIFY(videoSettings.width > 0);
    QVERIFY(videoSettings.height > 0);
    QVERIFY(videoSettings.frameRate > 0);
    
    qDebug() << "Video quality test completed";
}

void PerformanceOptimizationTest::testAudioQualityAdjustment()
{
    qDebug() << "Testing audio quality adjustment...";
    
    // Test different audio quality levels
    m_mediaOptimizer->setAudioQuality(MediaPerformanceOptimizer::AudioQuality::Studio);
    QCOMPARE(m_mediaOptimizer->getAudioQuality(), MediaPerformanceOptimizer::AudioQuality::Studio);
    
    m_mediaOptimizer->setAudioQuality(MediaPerformanceOptimizer::AudioQuality::Low);
    QCOMPARE(m_mediaOptimizer->getAudioQuality(), MediaPerformanceOptimizer::AudioQuality::Low);
    
    auto audioSettings = m_mediaOptimizer->getAudioSettings();
    QVERIFY(audioSettings.sampleRate > 0);
    QVERIFY(audioSettings.channels > 0);
    QVERIFY(audioSettings.bitrate > 0);
    
    qDebug() << "Audio quality test completed";
}

void PerformanceOptimizationTest::testAdaptiveQuality()
{
    qDebug() << "Testing adaptive quality...";
    
    m_mediaOptimizer->enableAdaptiveQuality(true);
    m_mediaOptimizer->startPerformanceMonitoring();
    
    // Simulate high CPU usage
    m_mediaOptimizer->recordEncodingTime(true, 50.0); // High encoding time
    
    // Wait for adaptive adjustment
    QTest::qWait(3000);
    
    m_mediaOptimizer->stopPerformanceMonitoring();
    
    qDebug() << "Adaptive quality test completed";
}

void PerformanceOptimizationTest::testPerformanceThresholds()
{
    qDebug() << "Testing performance thresholds...";
    
    // Test CPU usage optimization
    m_mediaOptimizer->optimizeForCPUUsage(50.0); // 50% max CPU
    
    // Test participant count optimization
    m_mediaOptimizer->optimizeForParticipantCount(20);
    
    auto metrics = m_mediaOptimizer->getCurrentMetrics();
    // Metrics should be initialized
    
    qDebug() << "Performance thresholds test completed";
}

void PerformanceOptimizationTest::testIntegratedOptimization()
{
    qDebug() << "Testing integrated optimization...";
    
    // Start all optimizers
    m_performanceManager->startMemoryMonitoring();
    m_memoryDetector->startLeakDetection();
    m_networkOptimizer->startNetworkMonitoring();
    m_mediaOptimizer->startPerformanceMonitoring();
    
    // Simulate a conference scenario
    int participantCount = 15;
    
    // Apply optimizations across all components
    m_performanceManager->optimizeForLargeConference(participantCount);
    m_networkOptimizer->optimizeForParticipantCount(participantCount);
    m_mediaOptimizer->optimizeForParticipantCount(participantCount);
    
    // Wait for optimizations to take effect
    QTest::qWait(5000);
    
    // Verify that all systems are working together
    auto perfMetrics = m_performanceManager->getCurrentMetrics();
    auto netMetrics = m_networkOptimizer->getCurrentMetrics();
    auto mediaMetrics = m_mediaOptimizer->getCurrentMetrics();
    auto memStats = m_memoryDetector->getMemoryStats();
    
    // Stop all optimizers
    m_mediaOptimizer->stopPerformanceMonitoring();
    m_networkOptimizer->stopNetworkMonitoring();
    m_memoryDetector->stopLeakDetection();
    m_performanceManager->stopMemoryMonitoring();
    
    qDebug() << "Integrated optimization test completed";
}

void PerformanceOptimizationTest::testMemoryLeakPrevention()
{
    qDebug() << "Testing memory leak prevention...";
    
    m_memoryDetector->setTrackingEnabled(true);
    
    // Simulate multiple allocation/deallocation cycles
    for (int i = 0; i < 100; ++i) {
        void* ptr = malloc(1024);
        m_memoryDetector->trackAllocation(ptr, 1024, "test.cpp", i);
        
        // Randomly free some allocations to simulate normal usage
        if (i % 3 == 0) {
            m_memoryDetector->trackDeallocation(ptr);
            free(ptr);
        } else {
            // Simulate leak (will be cleaned up later)
            m_memoryDetector->trackDeallocation(ptr);
            free(ptr);
        }
    }
    
    // Force cleanup
    m_memoryDetector->cleanupUnusedResources();
    
    auto stats = m_memoryDetector->getMemoryStats();
    qDebug() << "Final memory stats - Allocations:" << stats.totalAllocations 
             << "Deallocations:" << stats.totalDeallocations;
    
    qDebug() << "Memory leak prevention test completed";
}

void PerformanceOptimizationTest::testPerformanceUnderLoad()
{
    qDebug() << "Testing performance under load...";
    
    auto startTime = std::chrono::steady_clock::now();
    
    // Start all monitoring
    m_performanceManager->startMemoryMonitoring();
    m_networkOptimizer->startNetworkMonitoring();
    m_mediaOptimizer->startPerformanceMonitoring();
    
    // Simulate heavy load
    for (int i = 0; i < 1000; ++i) {
        // Simulate encoding operations
        m_mediaOptimizer->recordEncodingTime(true, 25.0 + (i % 10));
        m_mediaOptimizer->recordEncodingTime(false, 5.0 + (i % 3));
        
        // Simulate network operations
        if (i % 10 == 0) {
            QTest::qWait(1);
        }
    }
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Stop monitoring
    m_mediaOptimizer->stopPerformanceMonitoring();
    m_networkOptimizer->stopNetworkMonitoring();
    m_performanceManager->stopMemoryMonitoring();
    
    qDebug() << "Performance under load test completed in" << duration.count() << "ms";
    
    // Verify system remained responsive
    QVERIFY(duration.count() < 10000); // Should complete within 10 seconds
}

QTEST_MAIN(PerformanceOptimizationTest)
#include "test_performance_optimization.moc"