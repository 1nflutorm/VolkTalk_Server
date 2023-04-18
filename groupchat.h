#ifndef GROUPCHAT_H
#define GROUPCHAT_H

#include <QVector>
#include <QString>
#include <QTime>

class GroupChat
{
    QString GroupName;
    quint16 MaxNumberOfUsers;
    quint16 numberOfUsers;

    QVector<QString> UsersVector;
    QVector<QString> MessageHistory;

public:
    GroupChat(const QString& GroupName, quint16 MaxNumberOfUsers, const QString &user);
    bool isIn(const QString &name);
    QVector<QString> GetMessageHistory();
    void pushNewMessage(const QString& msg);
    QString GetGroupName();
    quint16 GetNumberOfUsers();
    quint16 GetMaxNumberOfUsers();
    int addUser(const QString& user);
    QVector<QString> GetUsersVector();
    bool deleteUser(const QString &name);
};

#endif // GROUPCHAT_H
