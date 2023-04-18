#include "groupchat.h"

GroupChat::GroupChat(const QString &GroupName, quint16 MaxNumberOfUsers, const QString &user)
{
    this->GroupName = GroupName;
    this->MaxNumberOfUsers = MaxNumberOfUsers;
    numberOfUsers = 0;
    QString FirstMsg = user + " создал эту комнату в " + QTime::currentTime().toString();
    MessageHistory = {FirstMsg};
}

bool GroupChat::isIn(const QString &name)
{
    for(auto user : UsersVector)
    {
        if(user == name)
            return true;
    }

    return false;
}

QVector<QString> GroupChat::GetMessageHistory()
{
    return MessageHistory;
}

void GroupChat::pushNewMessage(const QString &msg)
{
    MessageHistory.push_back(msg);
}

QString GroupChat::GetGroupName()
{
    return GroupName;
}

quint16 GroupChat::GetNumberOfUsers()
{
    return numberOfUsers;
}

quint16 GroupChat::GetMaxNumberOfUsers()
{
    return MaxNumberOfUsers;
}

int GroupChat::addUser(const QString &user)
{
    if(MaxNumberOfUsers == numberOfUsers)
        return 1;

    for(auto usr : UsersVector)
    {
        if(usr == user)
            return 2;
    }

    UsersVector.push_back(user);
    numberOfUsers++;

    return 0;
}

QVector<QString> GroupChat::GetUsersVector()
{
    return UsersVector;
}

bool GroupChat::deleteUser(const QString &name)
{
    for(int i = 0; i < UsersVector.size(); i++)
    {
        if(UsersVector[i] == name)
        {
            UsersVector.removeAt(i);
            numberOfUsers--;
            return true;
        }
    }
    return false;
}
