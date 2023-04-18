#include "backupworker.h"

BackupWorker::BackupWorker(QObject *parent) : QObject{parent}
{

}

void BackupWorker::startBackup(QVector<PrivateDialog*> dialogs, QVector<QString> logs)
{
    quint16 size = dialogs.size();
    quint16 MsgHsrSize;
    qDebug() << "backup started!";
    QFile dataFile("D:/ServerData/Data.txt");

    if(!dataFile.exists())
    {
        qDebug() << "file not exists";
        return;
    }
    else
    {
        qDebug() << "file exists";
    }

    if(!dataFile.open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "file error";
        return;
    }
    QTextStream fout(&dataFile);
    fout << QString::number(size) << "\n";
    fout << "\n";
    for(int i = 0; i < size; i++)
    {
        PrivateDialog* currDialog = dialogs[i];
        fout << currDialog->GetFirstUser() << "\n" << currDialog->GetSecondUser() << "\n";
        QVector<QString> MsgHistory = currDialog->GetMessageHistory();
        MsgHsrSize = MsgHistory.size();
        fout << MsgHsrSize << "\n";
        for(int j = 0; j < MsgHsrSize; j++)
        {
            fout << MsgHistory[j] << "\n";
        }
        fout << "\n";
    }

    dataFile.close();

    QFile logFile("D:/ServerData/Logs.txt");
    if(!logFile.exists())
    {
        qDebug() << "file not exists";
        return;
    }
    else
    {
        qDebug() << "file exists";
    }


    if(!logFile.open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "file error";
        return;
    }

    size = logs.size();
    QTextStream out(&logFile);
    out << QString::number(size) << "\n";
    out << "\n";
    for(int i = 0; i < size; i++)
    {
        out << logs[i] << "\n";
    }
    logFile.close();
    qDebug() << "ended!";
}

void BackupWorker::LoadData(QVector<PrivateDialog *> &dialogs, QVector<QString> &logs)
{
    quint16 size;
    quint16 MsgHsrSize;
    QByteArray Text;
    qDebug() << "load started!";
    QFile dataFile("D:/ServerData/Data.txt");
    if(!dataFile.exists())
    {
        qDebug() << "file not exists";
        return;
    }

    if(!dataFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "file error";
        return;
    }

    dataFile.seek(0);
    Text.clear();
    Text = dataFile.readLine();
    size = QString::fromUtf8(Text).toInt();
    Text = dataFile.readLine();
    for(int i = 0; i < size; i++)
    {
        Text.clear();
        Text = dataFile.readLine();
        QString firstUser = QString::fromUtf8(Text).remove("\n");
        Text.clear();
        Text = dataFile.readLine();
        QString secondUser = QString::fromUtf8(Text).remove("\n");
        PrivateDialog* dialog = new PrivateDialog(firstUser, secondUser);
        dialogs.push_back(dialog);
        Text.clear();
        Text = dataFile.readLine();
        MsgHsrSize = QString::fromUtf8(Text).toInt();
        for(int j = 0; j < MsgHsrSize; j++)
        {
            Text.clear();
            Text = dataFile.readLine();
            QString msg = QString::fromUtf8(Text).remove("\n");
            dialog->pushNewMessage(msg);
        }
        Text = dataFile.readLine();
    }

    dataFile.close();

    QFile logFile("D:/ServerData/Logs.txt");
    if(!logFile.exists())
    {
        qDebug() << "file not exists";
        return;
    }
    if(!logFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "file error";
        return;
    }
    logFile.seek(0);
    Text.clear();
    Text = logFile.readLine();
    size = QString::fromUtf8(Text).toInt();
    Text = logFile.readLine();
    for(int i = 0; i < size; i++)
    {
        Text.clear();
        Text = logFile.readLine();
        QString msg = QString::fromUtf8(Text).remove("\n");
        logs.push_back(msg);
        signalAddLog(msg);
    }
}
