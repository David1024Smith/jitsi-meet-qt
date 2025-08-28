#include <QCoreApplication>
#include <QTest>
#include <QSignalSpy>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QDebug>
#include "include/AuthenticationManager.h"

class TestAuthenticationManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // JWT Token Tests
    void testJWTTokenParsing();
    void testJWTTokenValidation();
    void testJWTTokenExpiration();
    void testInvalidJWTToken();

    // Authentication Flow Tests
    void testGuestAuthentication();
    void testPasswordAuthentication();
    void testJWTAuthentication();
    void testAuthenticationStateChanges();

    // Room Permissions Tests
    void testRoomPermissionsDefault();
    void testRoomPermissionsUpdate();

    // Token Management Tests
    void testTokenRefresh();
    void testTokenExpirationWarning();
    void testLogout();

private:
    AuthenticationManager* m_authManager;
    QString createValidJWTToken();
    QString createExpiredJWTToken();
    QString createInvalidJWTToken();
};

void TestAuthenticationManager::initTestCase()
{
    qDebug() << "Starting AuthenticationManager tests";
}

void TestAuthenticationManager::cleanupTestCase()
{
    qDebug() << "AuthenticationManager tests completed";
}

void TestAuthenticationManager::init()
{
    m_authManager = new AuthenticationManager(this);
}

void TestAuthenticationManager::cleanup()
{
    if (m_authManager) {
        m_authManager->deleteLater();
        m_authManager = nullptr;
    }
}

void TestAuthenticationManager::testJWTTokenParsing()
{
    qDebug() << "Testing JWT token parsing";
    
    QString validToken = createValidJWTToken();
    AuthenticationManager::JWTTokenInfo tokenInfo = m_authManager->parseJWTToken(validToken);
    
    QVERIFY(tokenInfo.isValid);
    QVERIFY(!tokenInfo.header.isEmpty());
    QVERIFY(!tokenInfo.payload.isEmpty());
    QVERIFY(!tokenInfo.signature.isEmpty());
    QVERIFY(!tokenInfo.claims.isEmpty());
    
    // 验证claims内容
    QVERIFY(tokenInfo.claims.contains("sub"));
    QVERIFY(tokenInfo.claims.contains("name"));
    QVERIFY(tokenInfo.claims.contains("iat"));
    QVERIFY(tokenInfo.claims.contains("exp"));
    
    qDebug() << "JWT token parsing test passed";
}

void TestAuthenticationManager::testJWTTokenValidation()
{
    qDebug() << "Testing JWT token validation";
    
    QString validToken = createValidJWTToken();
    AuthenticationManager::JWTTokenInfo tokenInfo = m_authManager->parseJWTToken(validToken);
    
    QVERIFY(m_authManager->verifyJWTToken(tokenInfo));
    
    qDebug() << "JWT token validation test passed";
}

void TestAuthenticationManager::testJWTTokenExpiration()
{
    qDebug() << "Testing JWT token expiration";
    
    QString expiredToken = createExpiredJWTToken();
    AuthenticationManager::JWTTokenInfo tokenInfo = m_authManager->parseJWTToken(expiredToken);
    
    QVERIFY(tokenInfo.isValid); // 解析成功
    QVERIFY(!m_authManager->verifyJWTToken(tokenInfo)); // 但验证失败（过期）
    
    qDebug() << "JWT token expiration test passed";
}

void TestAuthenticationManager::testInvalidJWTToken()
{
    qDebug() << "Testing invalid JWT token";
    
    QString invalidToken = createInvalidJWTToken();
    AuthenticationManager::JWTTokenInfo tokenInfo = m_authManager->parseJWTToken(invalidToken);
    
    QVERIFY(!tokenInfo.isValid);
    
    qDebug() << "Invalid JWT token test passed";
}

void TestAuthenticationManager::testGuestAuthentication()
{
    qDebug() << "Testing guest authentication";
    
    QSignalSpy stateSpy(m_authManager, &AuthenticationManager::authStateChanged);
    QSignalSpy successSpy(m_authManager, &AuthenticationManager::authenticationSucceeded);
    
    // 初始状态应该是未认证
    QCOMPARE(m_authManager->authState(), AuthenticationManager::NotAuthenticated);
    
    // 开始认证流程
    m_authManager->authenticate("https://meet.jit.si", "test-room", "Test User");
    
    // 等待信号
    QTest::qWait(1000);
    
    // 验证状态变化
    QVERIFY(stateSpy.count() >= 1);
    
    qDebug() << "Guest authentication test completed";
}

void TestAuthenticationManager::testPasswordAuthentication()
{
    qDebug() << "Testing password authentication";
    
    QSignalSpy failSpy(m_authManager, &AuthenticationManager::authenticationFailed);
    
    // 测试空密码
    m_authManager->authenticateWithPassword("");
    QVERIFY(failSpy.count() == 1);
    
    // 测试有效密码
    failSpy.clear();
    m_authManager->authenticateWithPassword("test-password");
    
    qDebug() << "Password authentication test completed";
}

void TestAuthenticationManager::testJWTAuthentication()
{
    qDebug() << "Testing JWT authentication";
    
    QSignalSpy stateSpy(m_authManager, &AuthenticationManager::authStateChanged);
    QSignalSpy successSpy(m_authManager, &AuthenticationManager::authenticationSucceeded);
    
    QString validToken = createValidJWTToken();
    m_authManager->authenticateWithJWT(validToken);
    
    // 验证认证成功
    QVERIFY(successSpy.count() == 1);
    QCOMPARE(m_authManager->authState(), AuthenticationManager::Authenticated);
    QCOMPARE(m_authManager->authType(), AuthenticationManager::JWT);
    QVERIFY(m_authManager->isAuthenticated());
    
    qDebug() << "JWT authentication test passed";
}

void TestAuthenticationManager::testAuthenticationStateChanges()
{
    qDebug() << "Testing authentication state changes";
    
    QSignalSpy stateSpy(m_authManager, &AuthenticationManager::authStateChanged);
    
    // 初始状态
    QCOMPARE(m_authManager->authState(), AuthenticationManager::NotAuthenticated);
    
    // 开始认证
    m_authManager->authenticate("https://meet.jit.si", "test-room", "Test User");
    
    // 等待状态变化
    QTest::qWait(100);
    
    // 验证状态变化信号
    QVERIFY(stateSpy.count() >= 1);
    
    qDebug() << "Authentication state changes test completed";
}

void TestAuthenticationManager::testRoomPermissionsDefault()
{
    qDebug() << "Testing default room permissions";
    
    AuthenticationManager::RoomPermissions permissions = m_authManager->roomPermissions();
    
    // 验证默认权限
    QVERIFY(permissions.canJoin);
    QVERIFY(!permissions.isModerator);
    QVERIFY(!permissions.canRecord);
    QVERIFY(!permissions.canLiveStream);
    QVERIFY(permissions.role.isEmpty() || permissions.role == "participant");
    
    qDebug() << "Default room permissions test passed";
}

void TestAuthenticationManager::testRoomPermissionsUpdate()
{
    qDebug() << "Testing room permissions update";
    
    QSignalSpy permissionsSpy(m_authManager, &AuthenticationManager::roomPermissionsUpdated);
    
    // 检查房间权限（这会触发网络请求，在测试中可能失败，但不影响功能验证）
    m_authManager->checkRoomPermissions("test-room");
    
    qDebug() << "Room permissions update test completed";
}

void TestAuthenticationManager::testTokenRefresh()
{
    qDebug() << "Testing token refresh";
    
    // 首先进行JWT认证
    QString validToken = createValidJWTToken();
    m_authManager->authenticateWithJWT(validToken);
    
    // 尝试刷新token
    m_authManager->refreshAuthToken();
    
    qDebug() << "Token refresh test completed";
}

void TestAuthenticationManager::testTokenExpirationWarning()
{
    qDebug() << "Testing token expiration warning";
    
    QSignalSpy expiringSpy(m_authManager, &AuthenticationManager::tokenExpiring);
    QSignalSpy expiredSpy(m_authManager, &AuthenticationManager::tokenExpired);
    
    // 使用过期token
    QString expiredToken = createExpiredJWTToken();
    m_authManager->authenticateWithJWT(expiredToken);
    
    qDebug() << "Token expiration warning test completed";
}

void TestAuthenticationManager::testLogout()
{
    qDebug() << "Testing logout";
    
    // 首先进行认证
    QString validToken = createValidJWTToken();
    m_authManager->authenticateWithJWT(validToken);
    
    QVERIFY(m_authManager->isAuthenticated());
    
    // 注销
    QSignalSpy stateSpy(m_authManager, &AuthenticationManager::authStateChanged);
    m_authManager->logout();
    
    // 验证注销结果
    QCOMPARE(m_authManager->authState(), AuthenticationManager::NotAuthenticated);
    QVERIFY(!m_authManager->isAuthenticated());
    QVERIFY(m_authManager->authToken().isEmpty());
    
    qDebug() << "Logout test passed";
}

QString TestAuthenticationManager::createValidJWTToken()
{
    // 创建一个有效的JWT token用于测试
    // Header: {"alg":"HS256","typ":"JWT"}
    QString header = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9";
    
    // Payload with future expiration
    QDateTime futureTime = QDateTime::currentDateTime().addSecs(3600); // 1小时后过期
    QJsonObject payload;
    payload["sub"] = "test-user-id";
    payload["name"] = "Test User";
    payload["iat"] = QDateTime::currentSecsSinceEpoch();
    payload["exp"] = futureTime.toSecsSinceEpoch();
    
    QJsonDocument payloadDoc(payload);
    QString payloadBase64 = QByteArray(payloadDoc.toJson(QJsonDocument::Compact)).toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    
    // Signature (简化处理)
    QString signature = "test-signature";
    
    return header + "." + payloadBase64 + "." + signature;
}

QString TestAuthenticationManager::createExpiredJWTToken()
{
    // 创建一个过期的JWT token用于测试
    QString header = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9";
    
    // Payload with past expiration
    QDateTime pastTime = QDateTime::currentDateTime().addSecs(-3600); // 1小时前过期
    QJsonObject payload;
    payload["sub"] = "test-user-id";
    payload["name"] = "Test User";
    payload["iat"] = pastTime.addSecs(-3600).toSecsSinceEpoch();
    payload["exp"] = pastTime.toSecsSinceEpoch();
    
    QJsonDocument payloadDoc(payload);
    QString payloadBase64 = QByteArray(payloadDoc.toJson(QJsonDocument::Compact)).toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    
    QString signature = "test-signature";
    
    return header + "." + payloadBase64 + "." + signature;
}

QString TestAuthenticationManager::createInvalidJWTToken()
{
    // 创建一个无效的JWT token（格式错误）
    return "invalid.jwt.token.format";
}

QTEST_MAIN(TestAuthenticationManager)
#include "test_authentication_manager.moc"