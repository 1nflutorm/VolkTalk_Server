#ifndef PRIVATEDIALOG_H
#define PRIVATEDIALOG_H

#include <QVector>
#include <QString>

class PrivateDialog
{
    QString firstUser;
    QString secondUser;

    QVector<QString> MessageHistory;
public:
    PrivateDialog(const QString& user_1, const QString& user_2);
    bool isIn(const QString& name);
    QVector<QString> GetMessageHistory();
    void pushNewMessage(const QString& msg);
    QString GetFirstUser();
    QString GetSecondUser();
};

#endif // PRIVATEDIALOG_H
