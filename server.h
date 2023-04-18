#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QTime>
#include <QMessageBox>
#include <QListWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMutex>
#include <QThread>
#include <QTimer>
#include "databaseview.h"
#include "user.h"
#include "privatedialog.h"
#include "groupchat.h"
#include "backupworker.h"

enum functions
{
    REGISTRATION = 21,
    LOG_IN,
    CLIENT_CONNECTED,
    CLIENT_DISCONNECTED,

    CURRENT_STATE,
    APPEND_MESSAGE_HISTORY,
    GET_INFO,

    PRIVATE_CHAT,
    GROUP_CHAT,
    NONE_TYPE,

    CREATE_GROP_CHAT,
    DELETE_GROUP_CHAT,
    CLIENT_CONNECTED_TO_CHAT,
    CLIENT_DISCONNECTED_FROM_CHAT,
    EXEPTION,
    DISCONNECTED
};

class Server : public QTcpServer
{
    Q_OBJECT

public:
    Server();
    bool GetStatus();
    void start();

private:
    bool status;
    QVector<QTcpSocket*> SocketVector;
    QVector<User*> UserVector;
    QVector<QString> OnlineVector;
    QVector<QString> OfflineVector;
    QVector<PrivateDialog*> PrivateDialogVector;
    QVector<GroupChat*> GroupChatVector;
    QVector<QString> LogVector;
    QByteArray Data;
    quint16 nextBlockSize;
    QTcpSocket* socket;

    QSqlTableModel* model;
    QSqlDatabase db;
    DataBaseView* DBView;

    QTimer *timer;

    QThread* thread;
    BackupWorker* worker;

    bool connectToDataBase();
    bool createNewChat(const QString& GroupName, quint16 numberOfUsers, const QString &user);
    void moveToOnlineVector(const QString Login);
    void moveToOfflineVector(const QString Login);
    void clientConnected(const QString Login, QDataStream& out);
    void DeleteUser(const QString& Login);
    void createNewDialog(const QString& user_1, const QString& user_2);
    void sendExeptionToUser(const QString& exeption, QTcpSocket* UserSocket);
    void deleteChat(const QString& RoomName);
    void addLog(const QString& log);
    void backup();
    void loadDataFromDB();
    User* FindUser(const QString& Login);
    PrivateDialog* findDialog(const QString& user_1, const QString& user_2);
    GroupChat* findChat(const QString& GroupName);
    GroupChat* deleteUserFromAnyChat(const QString& Login);
    QString FindUserBySocket(QTcpSocket* UserSocket);

public slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyread();
    void ShowDataBaseButton_clicked();
    void clientDisconnected(QAbstractSocket::SocketState socketState);
    void slotAddLog(const QString& msg);
    void slotBackup();

signals:
    void signalNewAction(const QString& action);
    void signalShowDataBase(QSqlTableModel *model);
    void signalAddOfflineUsers(QVector<QString> OfflineVector);
    void signalMoveToOnline(const QString Login);
    void signalMoveToOffline(const QString Login);
    void signalChatCreated(const QString& ChatName, const quint16& curNumOfUsers, const quint16& maxNumOfUsers);
    void signalUpdateNumOfUsers(const QString& ChatName, const quint16& NumOfUsers, const quint16& maxNumOfUsers);
    void signalChatDeleted(const QString& ChatName);
};

#endif // SERVER_H
