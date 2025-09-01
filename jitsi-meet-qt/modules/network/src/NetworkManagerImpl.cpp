#include "../include/NetworkManagerImpl.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

class NetworkManagerImpl::Private
{
public:
    QNetworkAccessManager* networkManager;
    QTimer* connectionTimer;
    bool isConnected;
    QString lastError;
};

NetworkManagerImpl::NetworkManagerImpl(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    d->networkManager = new QNetworkAccessManager(this);
    d->connectionTimer = new QTimer(this);
    d->isConnected = false;
    
    connect(d->connectionTimer, &QTimer::timeout,
            this, &NetworkManagerImpl::checkConnection);
    
    d->connectionTimer->start(5000); // Check every 5 seconds
}

NetworkManagerImpl::~NetworkManagerImpl() = default;

bool NetworkManagerImpl::isConnected() const
{
    return d->isConnected;
}

QString NetworkManagerImpl::lastError() const
{
    return d->lastError;
}

void NetworkManagerImpl::checkConnection()
{
    // Simple connection check implementation
    QNetworkRequest request(QUrl("https://www.google.com"));
    request.setRawHeader("User-Agent", "JitsiMeetQt");
    
    QNetworkReply* reply = d->networkManager->head(request);
    
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        bool wasConnected = d->isConnected;
        d->isConnected = (reply->error() == QNetworkReply::NoError);
        
        if (wasConnected != d->isConnected) {
            emit connectionStatusChanged(d->isConnected);
        }
        
        if (!d->isConnected) {
            d->lastError = reply->errorString();
            emit connectionError(d->lastError);
        }
        
        reply->deleteLater();
    });
}