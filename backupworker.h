#ifndef BACKUPWORKER_H
#define BACKUPWORKER_H

#include <QObject>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include "privatedialog.h"
#include "groupchat.h"

class BackupWorker : public QObject
{
    Q_OBJECT

public:
    explicit BackupWorker(QObject *parent = nullptr);

    void startBackup(QVector<PrivateDialog*> dialogs, QVector<QString> logs);
    void LoadData(QVector<PrivateDialog*>& dialogs, QVector<QString>& logs);

private:
    QVector<PrivateDialog*> dialogs;
    QVector<GroupChat*> chats;

signals:
    void signalAddLog(const QString& log);

};

#endif // BACKUPWORKER_H
