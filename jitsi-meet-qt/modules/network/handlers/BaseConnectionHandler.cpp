#include "../src/BaseConnectionHandler.h"

class BaseConnectionHandler::Private
{
public:
    IConnectionHandler::ConnectionType type;
    bool isConnected;
    QString lastError;
};

BaseConnectionHandler::BaseConnectionHandler(IConnectionHandler::ConnectionType type, QObject* parent)
    : IConnectionHandler(parent)
    , d(new Private)
{
    d->type = type;
    d->isConnected = false;
}

BaseConnectionHandler::~BaseConnectionHandler() = default;

IConnectionHandler::ConnectionType BaseConnectionHandler::connectionType() const
{
    return d->type;
}

bool BaseConnectionHandler::isConnected() const
{
    return d->isConnected;
}

QString BaseConnectionHandler::lastError() const
{
    return d->lastError;
}

bool BaseConnectionHandler::connect()
{
    // Base implementation - should be overridden
    d->isConnected = true;
    emit connectionStatusChanged(true);
    return true;
}

void BaseConnectionHandler::disconnect()
{
    if (d->isConnected) {
        d->isConnected = false;
        emit connectionStatusChanged(false);
    }
}

void BaseConnectionHandler::setError(const QString& error)
{
    d->lastError = error;
    emit connectionError(error);
}