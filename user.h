#ifndef USER_H
#define USER_H
#include <QString>
#include <QTcpSocket>

class User
{
    QString name;
    QTcpSocket* socket;

public:
    User(QString name, QTcpSocket* socket);
    QString GetName();
    QTcpSocket* GetSocket();
};

#endif // USER_H
