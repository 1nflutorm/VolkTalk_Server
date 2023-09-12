#include "serverwindow.h"
#include "ui_serverwindow.h"

ServerWindow::ServerWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::ServerWindow)
{
    ui->setupUi(this);

    ui->MainTabWidget->tabBar()->hide();
    ui->MainTabWidget->setCurrentIndex(0);

    ui->UsersWidget->setFocusPolicy(Qt::NoFocus);
    ui->RoomsWidget->setFocusPolicy(Qt::NoFocus);

    QFontDatabase::addApplicationFont(":/fonts/Roboto-Medium.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto-Thin.ttf");

    this->setAttribute(Qt::WA_TranslucentBackground);

    connect(ui->closeButton, &QPushButton::clicked, this, &ServerWindow::closeButton_clicked);
    connect(ui->hideButton, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(ui->maximizeButton, &QPushButton::clicked, this, &QWidget::showMinimized);

    connect(ui->ShowDataBaseButton, &QPushButton::clicked, &server, &Server::ShowDataBaseButton_clicked);
    connect(ui->StartButton, &QPushButton::clicked, this, &ServerWindow::startServerButton_clicked);
    connect(&server, &Server::signalAddOfflineUsers, this, &ServerWindow::slotAddOfflineUsers);
    connect(&server, &Server::signalMoveToOnline, this, &ServerWindow::slotMoveToOnline);
    connect(&server, &Server::signalMoveToOffline, this, &ServerWindow::slotMoveToOffline);
    connect(&server, &Server::signalChatCreated, this, &ServerWindow::slotChatCreated);
    connect(&server, &Server::signalUpdateNumOfUsers, this, &ServerWindow::slotUpdateNumOfUsers);
    connect(&server, &Server::signalChatDeleted, this, &ServerWindow::slotChatDeleted);
    connect(this, &ServerWindow::signalBackup, &server, &Server::slotBackup);


    connect(&server, &Server::signalNewAction, this, &ServerWindow::slotNewAction);

}

void ServerWindow::startServerButton_clicked()
{
    server.start();
    if(server.GetStatus())
    {
        ui->MainTabWidget->setCurrentIndex(1);
    }
}

void ServerWindow::closeButton_clicked()
{
    emit signalBackup();
    this->close();
}

void ServerWindow::slotAddOfflineUsers(QVector<QString> OfflineVector)
{
    for(int i = 0; i < OfflineVector.size(); i++)
    {
        QListWidgetItem *item = new QListWidgetItem(ui->UsersWidget);
        item->setText(OfflineVector[i]);
        item->setForeground(Qt::lightGray);
    }
    ui->UsersWidget->sortItems(Qt::AscendingOrder);
}

void ServerWindow::slotMoveToOnline(const QString Login)
{
    for(int i = 0; i < ui->UsersWidget->count(); i++)
    {
        QListWidgetItem *item = ui->UsersWidget->item(i);
        if(item->text() == Login)
        {
            item->setForeground(QColor(0, 160, 0));
            return;
        }
    }
    QListWidgetItem *item = new QListWidgetItem(ui->UsersWidget);
    item->setText(Login);
    item->setForeground(QColor(0, 160, 0));
    ui->UsersWidget->sortItems(Qt::AscendingOrder);
}

void ServerWindow::slotMoveToOffline(const QString Login)
{
    for(int i = 0; i < ui->UsersWidget->count(); i++)
    {
        QListWidgetItem *item = ui->UsersWidget->item(i);
        if(item->text() == Login)
        {
            item->setForeground(Qt::lightGray);
            return;
        }
    }
}

void ServerWindow::slotNewAction(const QString &action)
{
    ui->MessageBrowser->append(action);
}

void ServerWindow::slotChatCreated(const QString &ChatName, const quint16& curNumOfUsers, const quint16& maxNumOfUsers)
{
    QListWidgetItem *item = new QListWidgetItem(ui->RoomsWidget);
    QString Name = ChatName + "   " + QString::number(curNumOfUsers) + "/" + QString::number(maxNumOfUsers);
    item->setText(Name);
    item->setForeground(QColor(0, 160, 0));
    ui->UsersWidget->sortItems(Qt::AscendingOrder);
}

void ServerWindow::slotUpdateNumOfUsers(const QString &ChatName, const quint16 &NumOfUsers, const quint16 &maxNumOfUsers)
{
    for(int i = 0; i < ui->RoomsWidget->count(); i++)
    {
        QListWidgetItem *item = ui->RoomsWidget->item(i);
        QString temp = item->text();
        if(temp.startsWith(ChatName))
        {
            QString Name = ChatName + "   " + QString::number(NumOfUsers) + "/" + QString::number(maxNumOfUsers);
            item->setText(Name);
            return;
        }
    }
}

void ServerWindow::slotChatDeleted(const QString &ChatName)
{
    for(int i = 0; i < ui->RoomsWidget->count(); i++)
    {
        QString currentName = ui->RoomsWidget->item(i)->text();
        if(currentName.startsWith(ChatName))
        {
            QListWidgetItem* item = ui->RoomsWidget->takeItem(i);
            delete item;
        }
    }
}

ServerWindow::~ServerWindow()
{
    delete ui;
}

void ServerWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_pCursor = event->globalPos() - this->geometry().topLeft();
        event->accept();
    }
}

void ServerWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        this->move(event->globalPos() - m_pCursor);
        event->accept();
    }
}

