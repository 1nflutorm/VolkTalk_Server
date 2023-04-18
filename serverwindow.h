#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include <QFontDatabase>
#include <QTabBar>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QThread>
#include "server.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ServerWindow; }
QT_END_NAMESPACE

class ServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    Ui::ServerWindow *ui;
    QPoint m_pCursor;

    Server server;

private slots:
    void startServerButton_clicked();
    void closeButton_clicked();

public slots:
    void slotAddOfflineUsers(QVector<QString> OfflineVector);
    void slotMoveToOnline(const QString Login);
    void slotMoveToOffline(const QString Login);
    void slotNewAction(const QString& action);
    void slotChatCreated(const QString& ChatName, const quint16& curNumOfUsers, const quint16& maxNumOfUsers);
    void slotUpdateNumOfUsers(const QString& ChatName, const quint16& NumOfUsers, const quint16& maxNumOfUsers);
    void slotChatDeleted(const QString& ChatName);

signals:
    void signalShowDataBase(QSqlTableModel *model);
    void signalBackup();

};
#endif // SERVERWINDOW_H
