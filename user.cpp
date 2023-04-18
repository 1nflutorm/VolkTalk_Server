#include "user.h"

User::User(QString name, QTcpSocket *socket)
{
    this->name = name;
    this->socket = socket;
}

QString User::GetName()
{
    return name;
}

QTcpSocket *User::GetSocket()
{
    return socket;
}
