#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>

// Include the handlers and models
#include "handlers/URLHandler.h"
#include "handlers/ProtocolHandler.h"
#include "handlers/AuthHandler.h"
#include "models/Meeting.h"
#include "models/Room.h"
#include "models/Invitation.h"

void testURLHandler()
{
    qDebug() << "=== Testing URLHandler ===";
    
    URLHandler handler;
    
    // Test URL parsing
    QString testUrl = "https://meet.jit.si/TestRoom123?config.startWithAudioMuted=true";
    QVariantMap result = handler.parseURL(testUrl);
    
    qDebug() << "Parsed URL:" << testUrl;
    qDebug() << "Result:" << result;
    qDebug() << "Valid:" << result.value("valid").toBool();
    qDebug() << "Server:" << result.value("server").toString();
    qDebug() << "Room:" << result.value("roomName").toString();
    
    // Test URL validation
    bool isValid = handler.validateURL(testUrl);
    qDebug() << "URL validation:" << isValid;
    
    // Test URL building
    QString builtUrl = handler.buildMeetingURL("meet.jit.si", "TestRoom", 
                                              {{"config.startWithAudioMuted", "true"}});
    qDebug() << "Built URL:" << builtUrl;
    
    qDebug() << "";
}

void testProtocolHandler()
{
    qDebug() << "=== Testing ProtocolHandler ===";
    
    ProtocolHandler handler;
    
    // Test protocol URL parsing
    QString protocolUrl = "jitsi://meet.jit.si/TestRoom?jwt=token123";
    QVariantMap result = handler.parseProtocolUrl(protocolUrl);
    
    qDebug() << "Parsed protocol URL:" << protocolUrl;
    qDebug() << "Result:" << result;
    qDebug() << "Valid:" << result.value("valid").toBool();
    qDebug() << "Protocol:" << result.value("protocol").toString();
    qDebug() << "Server:" << result.value("server").toString();
    qDebug() << "Room:" << result.value("room").toString();
    
    // Test protocol URL building
    QString builtProtocolUrl = handler.buildProtocolUrl("jitsi", "meet.jit.si", "TestRoom", 
                                                       {{"jwt", "token123"}});
    qDebug() << "Built protocol URL:" << builtProtocolUrl;
    
    // Test conversion
    QString standardUrl = handler.convertToStandardUrl(protocolUrl);
    qDebug() << "Converted to standard URL:" << standardUrl;
    
    qDebug() << "";
}

void testAuthHandler()
{
    qDebug() << "=== Testing AuthHandler ===";
    
    AuthHandler handler;
    
    // Test guest authentication
    QVariantMap credentials;
    credentials["displayName"] = "Test User";
    credentials["email"] = "test@example.com";
    
    bool authResult = handler.authenticate(AuthHandler::GuestAuth, credentials);
    qDebug() << "Guest authentication result:" << authResult;
    qDebug() << "Auth status:" << static_cast<int>(handler.currentStatus());
    qDebug() << "Current user:" << handler.getCurrentUser();
    qDebug() << "User role:" << static_cast<int>(handler.getCurrentUserRole());
    
    // Test permission checking
    bool canJoin = handler.checkPermission("meeting", "join");
    bool canModerate = handler.checkPermission("meeting", "moderate");
    qDebug() << "Can join meeting:" << canJoin;
    qDebug() << "Can moderate meeting:" << canModerate;
    
    qDebug() << "";
}

void testMeetingModel()
{
    qDebug() << "=== Testing Meeting Model ===";
    
    Meeting meeting("Test Meeting", "https://meet.jit.si/TestRoom");
    
    qDebug() << "Meeting ID:" << meeting.id();
    qDebug() << "Meeting name:" << meeting.name();
    qDebug() << "Meeting URL:" << meeting.url();
    qDebug() << "Server:" << meeting.server();
    qDebug() << "Room name:" << meeting.roomName();
    
    // Test participant management
    meeting.addParticipant("user1");
    meeting.addParticipant("user2");
    qDebug() << "Participant count:" << meeting.participantCount();
    qDebug() << "Participants:" << meeting.participants();
    
    // Test settings
    meeting.setSetting("enableChat", true);
    meeting.setSetting("maxParticipants", 50);
    qDebug() << "Settings:" << meeting.settings();
    
    // Test serialization
    QString json = meeting.toJson();
    qDebug() << "Meeting JSON length:" << json.length();
    
    // Test validation
    qDebug() << "Is valid:" << meeting.isValid();
    qDebug() << "Validation errors:" << meeting.validationErrors();
    
    qDebug() << "";
}

void testRoomModel()
{
    qDebug() << "=== Testing Room Model ===";
    
    Room room("TestRoom", "meet.jit.si");
    
    qDebug() << "Room ID:" << room.id();
    qDebug() << "Room name:" << room.name();
    qDebug() << "Server:" << room.server();
    qDebug() << "Type:" << static_cast<int>(room.type());
    qDebug() << "Status:" << static_cast<int>(room.status());
    
    // Test participant management with roles
    room.addParticipant("user1", "moderator");
    room.addParticipant("user2", "participant");
    qDebug() << "Participant count:" << room.participantCount();
    qDebug() << "Moderators:" << room.moderators();
    
    // Test permissions
    bool canMute = room.hasPermission("user1", "mute_others");
    bool canKick = room.hasPermission("user2", "kick_participants");
    qDebug() << "User1 can mute others:" << canMute;
    qDebug() << "User2 can kick participants:" << canKick;
    
    // Test room settings
    room.setPassword("secret123");
    room.setLocked(true);
    qDebug() << "Requires password:" << room.requiresPassword();
    qDebug() << "Is locked:" << room.isLocked();
    
    // Test validation
    qDebug() << "Is valid:" << room.isValid();
    qDebug() << "Validation errors:" << room.validationErrors();
    
    qDebug() << "";
}

void testInvitationModel()
{
    qDebug() << "=== Testing Invitation Model ===";
    
    Invitation invitation("meeting123", "recipient@example.com");
    
    qDebug() << "Invitation ID:" << invitation.id();
    qDebug() << "Meeting ID:" << invitation.meetingId();
    qDebug() << "Recipient email:" << invitation.recipientEmail();
    qDebug() << "Type:" << static_cast<int>(invitation.type());
    qDebug() << "Status:" << static_cast<int>(invitation.status());
    
    // Set invitation details
    invitation.setSenderName("Test Sender");
    invitation.setSenderEmail("sender@example.com");
    invitation.setMeetingUrl("https://meet.jit.si/TestRoom");
    
    // Test sending
    bool sendResult = invitation.send();
    qDebug() << "Send result:" << sendResult;
    qDebug() << "Status after send:" << static_cast<int>(invitation.status());
    
    // Test expiration
    qDebug() << "Is expired:" << invitation.isExpired();
    qDebug() << "Time remaining:" << invitation.timeRemaining() << "seconds";
    
    // Test event tracking
    invitation.recordEvent("test_event", {{"data", "test_value"}});
    qDebug() << "Event history count:" << invitation.eventHistory().size();
    
    // Test validation
    qDebug() << "Is valid:" << invitation.isValid();
    qDebug() << "Validation errors:" << invitation.validationErrors();
    
    qDebug() << "";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "Testing Meeting Module Handlers and Models";
    qDebug() << "==========================================";
    
    testURLHandler();
    testProtocolHandler();
    testAuthHandler();
    testMeetingModel();
    testRoomModel();
    testInvitationModel();
    
    qDebug() << "All tests completed!";
    
    return 0;
}