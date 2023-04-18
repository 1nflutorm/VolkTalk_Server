#include "privatedialog.h"

PrivateDialog::PrivateDialog(const QString& user_1, const QString& user_2)
{
    firstUser = user_1;
    secondUser = user_2;
    MessageHistory = {};
}

bool PrivateDialog::isIn(const QString &name)
{
    if(firstUser == name)
        return true;
    if(secondUser == name)
        return true;

    return false;
}

QVector<QString> PrivateDialog::GetMessageHistory()
{
    return MessageHistory;
}

void PrivateDialog::pushNewMessage(const QString &msg)
{
    MessageHistory.push_back(msg);
}

QString PrivateDialog::GetFirstUser()
{
    return firstUser;
}

QString PrivateDialog::GetSecondUser()
{
    return secondUser;
}
