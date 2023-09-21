#include "server.h"

Server::Server()
{
    status = false;
    nextBlockSize = 0;

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Server::backup);
    timer->start(30000);

    thread = new QThread(this);
    connect(this, SIGNAL(destroyed()), thread, SLOT(quit()));

    worker = new BackupWorker;
    connect(worker, &BackupWorker::signalAddLog, this, &Server::slotAddLog);

    worker->moveToThread(thread);

    connectToDataBase();

    loadDataFromDB();
}

void Server::start()
{
    if(this->listen(QHostAddress::Any, 2323))
    {
        status = true;
        emit signalAddOfflineUsers(OfflineVector);
        worker->LoadData(PrivateDialogVector, LogVector);
    }
    else
    {
        status = false;
    }
}

bool Server::GetStatus()
{
    return status;
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    socket = new QTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);

    connect(socket, &QTcpSocket::readyRead, this, &Server::slotReadyread);
    connect(socket, &QTcpSocket::stateChanged, this, &Server::clientDisconnected);
    connect(socket, &QTcpSocket::disconnected, socket, &Server::deleteLater);

    SocketVector.push_back(socket);
    qDebug() << "Client connected!";
}

void Server::slotReadyread()
{
    socket = static_cast<QTcpSocket*>(sender());
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_4);
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_4);
    if(in.status() == QDataStream::Ok)
    {
        while(true)
        {
            if(nextBlockSize == 0)
            {
                if(socket->bytesAvailable() < 2)
                {
                    //qDebug() << "Data < 2";
                    break;
                }
                in >> nextBlockSize;
                qDebug() << "nextBlockSize = " << nextBlockSize;

            }
            if(socket->bytesAvailable() < nextBlockSize)
            {
                qDebug() << "Data not full";
                break;
            }
            QString log;
            quint16 operation;
            in >> operation;
            switch (operation)
            {
            case REGISTRATION:
            {
                QTime time;
                QString Login, Password;
                QString name, surname, lastname;
                in >> Login >> Password >> surname >> name >> lastname >> time;

                addLog(Login + " отправил запрос на регистрацию в " + QTime::currentTime().toString());

                QSqlQuery query;
                query.prepare("SELECT Login FROM users WHERE LOGIN = ?");
                query.addBindValue(Login);
                query.exec();

                Data.clear();
                out.device()->seek(0);
                out << quint16(0);
                if(query.next())
                {
                    out << quint16(EXEPTION) << QString("Аккаунт с таким логином уже существует!");
                    out.device()->seek(0);
                    out << quint16(Data.size() - sizeof(quint16));
                    socket->write(Data);

                    addLog(Login + " отказ в регистрации (логин существует) в " + QTime::currentTime().toString());

                    break;
                }

                query.clear();
                query.prepare("INSERT INTO users (surname, first_name, last_name, login, password) VALUES (?, ?, ?, ?, ?)");
                query.addBindValue(surname);
                query.addBindValue(name);
                query.addBindValue(lastname);
                query.addBindValue(Login);
                query.addBindValue(Password);
                query.exec();

                out << quint16(LOG_IN);
                out.device()->seek(0);
                out << quint16(Data.size() - sizeof(quint16));
                socket->write(Data);

                emit signalMoveToOnline(Login);

                OnlineVector.push_back(Login);

                UserVector.push_back(new User(Login, socket));

                clientConnected(Login, out);

                addLog(Login + " успешно зарегестрировался в " + QTime::currentTime().toString());

                break;
            }
            case LOG_IN:
            {
                QTime time;
                QString Login, Password;
                in >> Login >> Password >> time;

                addLog(Login + " отправил запрос на вход в " + QTime::currentTime().toString());

                QSqlQuery query;
                query.prepare("SELECT Login, Password FROM users WHERE Login = ?");
                query.addBindValue(Login);
                query.exec();

                Data.clear();
                out.device()->seek(0);
                out << quint16(0);
                if(query.next())
                {
                    if(query.value(1).toString() != Password)
                    {
                        out << quint16(EXEPTION) << QString("Неверный пароль!");
                        out.device()->seek(0);
                        out << quint16(Data.size() - sizeof(quint16));
                        socket->write(Data);

                        addLog(Login + " отказ на вход (неверный пароль) в " + QTime::currentTime().toString());
                        break;
                    }
                }
                else
                {
                    out << quint16(EXEPTION) << QString("Пользователя с таким логином не существует!");
                    out.device()->seek(0);
                    out << quint16(Data.size() - sizeof(quint16));
                    socket->write(Data);

                    addLog(Login + " отказ на вход (неверный логин) в " + QTime::currentTime().toString());

                    break;
                }

                out << quint16(LOG_IN);
                out.device()->seek(0);
                out << quint16(Data.size() - sizeof(quint16));
                socket->write(Data);

                emit signalMoveToOnline(Login);

                moveToOnlineVector(Login);

                UserVector.push_back(new User(Login, socket));

                clientConnected(Login, out);

                addLog(Login + " Успешно вошел в аккаунт в " + QTime::currentTime().toString());

                break;
            }
            case CURRENT_STATE:
            {
                QString Login;
                in >> Login;

                addLog(Login + " запрос текущего состояния в " + QTime::currentTime().toString());

                Data.clear();
                out.device()->seek(0);
                out << quint16(0) << quint16(CURRENT_STATE) << quint16(OnlineVector.size() - 1);
                for(int i = 0; i < OnlineVector.size(); i++)
                {
                    if(OnlineVector[i] != Login)
                        out << OnlineVector[i];
                }
                out << quint16(OfflineVector.size());
                for(int i = 0; i < OfflineVector.size(); i++)
                {
                    out << OfflineVector[i];
                }
                out << quint16(GroupChatVector.size());
                for(int i = 0; i < GroupChatVector.size(); i++)
                {
                    out << GroupChatVector[i]->GetGroupName()
                        << quint16(GroupChatVector[i]->GetNumberOfUsers())
                        << quint16(GroupChatVector[i]->GetMaxNumberOfUsers());
                }
                out.device()->seek(0);
                out << quint16(Data.size() - sizeof(quint16));
                socket->write(Data);
                break;
            }
            case APPEND_MESSAGE_HISTORY:
            {
                quint16 msgType;
                QString nameFrom, nameTo;

                in >> msgType;
                if(msgType == PRIVATE_CHAT)
                {
                    in >> nameFrom >> nameTo;

                    addLog(nameFrom + " запрос истории личных сообщений с " + nameTo + " в " + QTime::currentTime().toString());

                    PrivateDialog* dialog = findDialog(nameFrom, nameTo);
                    if(!dialog)
                    {
                        createNewDialog(nameFrom, nameTo);
                        break;
                    }

                    Data.clear();
                    out.device()->seek(0);
                    QVector<QString> MessageHistory = dialog->GetMessageHistory();
                    out << quint16(0) << quint16(APPEND_MESSAGE_HISTORY) << quint16(MessageHistory.size());
                    for(auto message : MessageHistory)
                    {
                        out << message;
                    }
                    out.device()->seek(0);
                    out << quint16(Data.size() - sizeof(quint16));
                    socket->write(Data);
                }
                else if(msgType == GROUP_CHAT)
                {
                    QString GroupName;
                    in >> GroupName;

                    QString user = FindUserBySocket(socket);
                    addLog(user + " запрос истории групповых сообщений из " + GroupName + " комнаты в " + QTime::currentTime().toString());

                    GroupChat* chat = findChat(GroupName);
                    Data.clear();
                    out.device()->seek(0);
                    QVector<QString> MessageHistory = chat->GetMessageHistory();
                    out << quint16(0) << quint16(APPEND_MESSAGE_HISTORY) << quint16(MessageHistory.size());
                    for(auto message : MessageHistory)
                    {
                        out << message;
                    }
                    out.device()->seek(0);
                    out << quint16(Data.size() - sizeof(quint16));
                    socket->write(Data);
                }
                break;
            }
            case GET_INFO:
            {
                quint16 type;
                in >> type;

                Data.clear();
                out.device()->seek(0);
                out << quint16(0) << quint16(GET_INFO);
                if(type == GROUP_CHAT)
                {
                    QString GroupName;
                    in >> GroupName;

                    QString user = FindUserBySocket(socket);
                    addLog(user + " запрос информации о комнате " + GroupName + " в " + QTime::currentTime().toString());

                    GroupChat* chat = findChat(GroupName);

                    if(!chat)
                    {
                        break;
                        sendExeptionToUser("Чат не найден!", socket);
                    }

                    QVector<QString> Users = chat->GetUsersVector();


                    QVector<QString> MessageHistory = chat->GetMessageHistory();
                    out << quint16(GROUP_CHAT) << GroupName
                        << quint16(chat->GetMaxNumberOfUsers())
                        << quint16(chat->GetNumberOfUsers()) << quint16(Users.size());

                   for(int i = 0; i < Users.size(); i++)
                   {
                       out << Users[i];
                   }

                   out.device()->seek(0);
                   out << quint16(Data.size() - sizeof(quint16));
                   socket->write(Data);
                   break;
                }
                if(type == PRIVATE_CHAT)
                {
                    QString Login;
                    in >> Login;

                    QString user = FindUserBySocket(socket);
                    addLog(user + " запрос информации о пользователе " + Login + " в " + QTime::currentTime().toString());

                    QSqlQuery query;
                    query.prepare("SELECT * FROM users WHERE LOGIN = ?");
                    query.addBindValue(Login);
                    query.exec();

                    if(query.next())
                    {
                        QString id = query.value(0).toString();
                        QString surname = query.value(1).toString();
                        QString name = query.value(2).toString();
                        QString lastname = query.value(3).toString();
                        QString login = query.value(4).toString();
                        if(login != Login)
                            break;
                        out << quint16(PRIVATE_CHAT)<< login << id << surname << name << lastname;
                        out.device()->seek(0);
                        out << quint16(Data.size() - sizeof(quint16));
                        socket->write(Data);
                        break;

                    }

                    break;
                }
                break;
            }
            case PRIVATE_CHAT:
            {
                QString nameFrom, nameTo, msg, action;
                User* user_1, *user_2;
                PrivateDialog* dialog;

                in >> nameFrom >> nameTo >> msg;

                addLog(nameFrom + " отправил сообщение пользователю " + nameTo + " в " + QTime::currentTime().toString());

                user_1 = FindUser(nameFrom);
                user_2 = FindUser(nameTo);

                dialog = findDialog(nameFrom, nameTo);
                if(!dialog)
                {
                    createNewDialog(nameFrom, nameTo);
                }

                msg = nameFrom + ": " + msg + " " + QTime::currentTime().toString();
                dialog->pushNewMessage(msg);

                if(user_1)
                {
                    Data.clear();
                    out.device()->seek(0);
                    out << quint16(0) << quint16(PRIVATE_CHAT) << nameTo << msg;
                    out.device()->seek(0);
                    out << quint16(Data.size() - sizeof(quint16));
                    user_1->GetSocket()->write(Data);
                }

                if(user_2)
                {
                    Data.clear();
                    out.device()->seek(0);
                    out << quint16(0) << quint16(PRIVATE_CHAT) << nameFrom << msg;
                    out.device()->seek(0);
                    out << quint16(Data.size() - sizeof(quint16));
                    user_2->GetSocket()->write(Data);
                }
                break;
            }
            case GROUP_CHAT:
            {
                QString nameFrom, GroupName, msg;
                QTime time;

                in >> nameFrom >> GroupName >> msg;

                addLog(nameFrom + " отправил сообщение в комнату " + GroupName + " в " + QTime::currentTime().toString());

                GroupChat* chat = findChat(GroupName);

                if(!chat)
                {
                    break;
                }

                msg = nameFrom + ": " + msg + " " + time.toString();
                chat->pushNewMessage(msg);

                Data.clear();
                out.device()->seek(0);
                out << quint16(0) << quint16(GROUP_CHAT) << GroupName << msg;
                out.device()->seek(0);
                out << quint16(Data.size() - sizeof(quint16));

                QVector<QString> UsersVector = chat->GetUsersVector();

                for(int i = 0; i < UsersVector.size(); i++)
                {
                    User* user = FindUser(UsersVector[i]);
                    user->GetSocket()->write(Data);
                }
                break;
            }
            case CREATE_GROP_CHAT:
            {
                QString GroupName, Login;
                quint16 maxNumberOfUsers;
                in >> Login >> GroupName >> maxNumberOfUsers;

                addLog(Login + " запрос на создание группы " + GroupName + " в " + QTime::currentTime().toString());

                bool chatCreated = createNewChat(GroupName, maxNumberOfUsers, Login);

                if(!chatCreated)
                    break;

                emit signalChatCreated(GroupName, quint16(0), maxNumberOfUsers);

                Data.clear();
                out.device()->seek(0);
                out << quint16(0) << quint16(CREATE_GROP_CHAT) << GroupName << quint16(0) << quint16(maxNumberOfUsers);
                out.device()->seek(0);
                out << quint16(Data.size() - sizeof(quint16));

                for(int i = 0; i < UserVector.size(); i++)
                {
                    UserVector[i]->GetSocket()->write(Data);
                }

                addLog(Login + " создал группу " + GroupName + " в " + QTime::currentTime().toString());
                break;
            }
            case DELETE_GROUP_CHAT:
            {
                QString GroupName, Login;
                in >> Login >> GroupName;

                addLog(Login + " запрос на удаление комнаты " + GroupName + " в " + QTime::currentTime().toString());

                QString deleteMsg = Login + " удалил комнату " + GroupName + " в " + QTime::currentTime().toString();

                deleteChat(GroupName);

                emit signalChatDeleted(GroupName);

                Data.clear();
                out.device()->seek(0);
                out << quint16(0) << quint16(DELETE_GROUP_CHAT) << GroupName << deleteMsg;
                out.device()->seek(0);
                out << quint16(Data.size() - sizeof(quint16));

                for(int i = 0; i < UserVector.size(); i++)
                {
                    UserVector[i]->GetSocket()->write(Data);
                }

                addLog(Login + " удалил комнату " + GroupName + " в " + QTime::currentTime().toString());
                break;
            }
            case CLIENT_CONNECTED_TO_CHAT:
            {
                QString GroupName, Login;
                in >> Login >> GroupName;

                addLog(Login + " запрос на подключение к комнате " + GroupName + " в " + QTime::currentTime().toString());

                GroupChat* chat = findChat(GroupName);

                if(!chat)
                    break;

                int code = chat->addUser(Login);

                if(code == 1)
                {
                    addLog(Login + " отказ на подключение к комнате " + GroupName + " в " + QTime::currentTime().toString());
                    sendExeptionToUser("Не удалось подключиться к комнате, недостаточно места!", socket);
                    break;
                }

                if(code == 2)
                {
                    addLog(Login + " отказ на подключение к комнате " + GroupName + " в " + QTime::currentTime().toString());
                    sendExeptionToUser("Вы уже состоите в этой комнате!", socket);
                    break;
                }

                quint16 maxNumberOfusers = chat->GetMaxNumberOfUsers();
                quint16 NumberOfusers = chat->GetNumberOfUsers();

                Data.clear();
                out.device()->seek(0);
                out << quint16(0) << quint16(CLIENT_CONNECTED_TO_CHAT) << Login << GroupName << quint16(NumberOfusers) << quint16(maxNumberOfusers);
                out.device()->seek(0);
                out << quint16(Data.size() - sizeof(quint16));
                for(int i = 0; i < UserVector.size(); i++)
                {
                    UserVector[i]->GetSocket()->write(Data);
                }

                emit signalUpdateNumOfUsers(GroupName, NumberOfusers, maxNumberOfusers);
                addLog(Login + " подключился к комнате " + GroupName + " в " + QTime::currentTime().toString());
                break;
            }
            case CLIENT_DISCONNECTED_FROM_CHAT:
            {
                QString Login;
                in >> Login;

                GroupChat* chat = deleteUserFromAnyChat(Login);

                if(!chat)
                    break;

                addLog(Login + " запрос на отключение от комнаты " + chat->GetGroupName() + " в " + QTime::currentTime().toString());

                quint16 maxNumberOfusers = chat->GetMaxNumberOfUsers();
                quint16 NumberOfusers = chat->GetNumberOfUsers();

                Data.clear();
                out.device()->seek(0);
                out << quint16(0) << quint16(CLIENT_DISCONNECTED_FROM_CHAT) << Login << chat->GetGroupName() << quint16(NumberOfusers) << quint16(maxNumberOfusers);
                out.device()->seek(0);
                out << quint16(Data.size() - sizeof(quint16));
                for(int i = 0; i < UserVector.size(); i++)
                {
                    UserVector[i]->GetSocket()->write(Data);
                }

                emit signalUpdateNumOfUsers(chat->GetGroupName(), NumberOfusers, maxNumberOfusers);
                addLog(Login + " отключился от комнаты " + chat->GetGroupName() + " в " + QTime::currentTime().toString());
                break;
            }
            default:
                break;
            }
            nextBlockSize = 0;
        }
    }
    else
    {
        qDebug() << "QDataStream error";
    }
}

bool Server::connectToDataBase()
{
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("127.0.0.1");
    db.setDatabaseName("Volk_Talk_Users");
    db.setUserName("postgres");
    db.setPassword("");
    if (!db.open())
    {
        qDebug() << db.lastError().text();

        return false;
    }
    else
    {
        qDebug() << "success";
        DataBaseView* DBView = new DataBaseView;
        DBView->setWindowFlags(Qt::FramelessWindowHint);
        connect(this, &Server::signalShowDataBase, DBView, &DataBaseView::slotShowDataBase);

        return true;
    }


}

void Server::moveToOnlineVector(const QString Login)
{
    for(int i = 0; i < OfflineVector.size(); i++)
    {
        if(OfflineVector[i] == Login)
        {
            OnlineVector.push_back(OfflineVector[i]);
            OfflineVector.removeAt(i);
        }
    }

}

void Server::moveToOfflineVector(const QString Login)
{
    for(int i = 0; i < OnlineVector.size(); i++)
    {
        QString a = OnlineVector[i];
        if(a == Login)
        {
            OfflineVector.push_back(OnlineVector[i]);
            OnlineVector.removeAt(i);
        }
    }
}

void Server::clientConnected(const QString Login, QDataStream &out)
{
    Data.clear();
    out.device()->seek(0);
    out << quint16(0) << quint16(CLIENT_CONNECTED) << Login;
    out.device()->seek(0);
    out << quint16(Data.size() - sizeof(quint16));
    for(int i = 0; i < UserVector.size() - 1; i++)
        UserVector[i]->GetSocket()->write(Data);
}

User *Server::FindUser(const QString &Login)
{
    for(auto user : UserVector)
    {
        if(user->GetName() == Login)
            return user;
    }

    return nullptr;
}

void Server::DeleteUser(const QString &Login)
{
    for(int i = 0; i < UserVector.size(); i++)
    {
        if(UserVector[i]->GetName() == Login)
        {
            UserVector.removeAt(i);
        }
    }
}

PrivateDialog* Server::findDialog(const QString &user_1, const QString &user_2)
{
    for(int i = 0; i < PrivateDialogVector.size(); i++)
    {
        PrivateDialog* currentDialog = PrivateDialogVector[i];
        if(!currentDialog->isIn(user_1))
            continue;
        if(!currentDialog->isIn(user_2))
            continue;

        return currentDialog;
    }

    return nullptr;
}

GroupChat *Server::findChat(const QString &GroupName)
{
    for(int i = 0; i < GroupChatVector.size(); i++)
    {
        GroupChat* currentChat = GroupChatVector[i];
        if(currentChat->GetGroupName() == GroupName)
            return currentChat;
    }

    return nullptr;
}

void Server::createNewDialog(const QString &user_1, const QString &user_2)
{
    PrivateDialogVector.push_back(new PrivateDialog(user_1, user_2));
}

bool Server::createNewChat(const QString &GroupName, quint16 numberOfUsers, const QString &user)
{
    GroupChat* chatExist = findChat(GroupName);
    if(chatExist)
    {
        addLog(user + " отказ в создании группы " + GroupName + " в " + QTime::currentTime().toString());

        sendExeptionToUser("Комната с таким именем уже существует!", socket);
        return false;
    }
    GroupChatVector.push_back(new GroupChat(GroupName, numberOfUsers, user));
    return true;
}

void Server::sendExeptionToUser(const QString &exeption, QTcpSocket* UserSocket)
{
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_4);
    out.device()->seek(0);
    out << quint16(0);
    out << quint16(EXEPTION) << exeption;
    out.device()->seek(0);
    out << quint16(Data.size() - sizeof(quint16));
    UserSocket->write(Data);
}

GroupChat *Server::deleteUserFromAnyChat(const QString &Login)
{
    for(int i = 0; i < GroupChatVector.size(); i++)
    {
        if(GroupChatVector[i]->deleteUser(Login))
        {
            return GroupChatVector[i];
        }
    }

    return nullptr;
}

void Server::deleteChat(const QString &RoomName)
{
    for(int i = 0; i < GroupChatVector.size(); i++)
    {
        if(GroupChatVector[i]->GetGroupName() == RoomName)
        {
            GroupChatVector.removeAt(i);
        }
    }
}

QString Server::FindUserBySocket(QTcpSocket *UserSocket)
{
    for(int i = 0; i < UserVector.size(); i++)
    {
        if(UserVector[i]->GetSocket() == UserSocket)
        {
            return UserVector[i]->GetName();
        }
    }
    return "";
}

void Server::addLog(const QString &log)
{
    emit signalNewAction(log);
    LogVector.push_back(log);
}

void Server::backup()
{
    //qDebug() << "backup";
    QVector<PrivateDialog*> dialogs(PrivateDialogVector);
    QVector<QString> logs(LogVector);
    worker->startBackup(dialogs, logs);
}

void Server::loadDataFromDB()
{
    QSqlQuery query;
    query.exec("SELECT Login FROM users");
    QString Login;
    while(query.next())
    {
        Login =  query.value(0).toString();
        OfflineVector.push_back(Login);
    }
}


void Server::clientDisconnected(QAbstractSocket::SocketState socketState)
{
    if(socketState == QAbstractSocket::ClosingState)
    {
        qDebug() << "socket is in ClosingState";
        QTcpSocket* socketToDelete = static_cast<QTcpSocket*>(sender());
        if(!socketToDelete)
        {
            return;
        }

        QString Login = FindUserBySocket(socketToDelete);
        moveToOfflineVector(Login);

        addLog(Login + " отключился от сервера в " + QTime::currentTime().toString());

        emit signalMoveToOffline(Login);

        DeleteUser(Login);

        QDataStream out(&Data, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_4);
        Data.clear();
        out.device()->seek(0);
        out << quint16(0) << quint16(CLIENT_DISCONNECTED) << Login;
        out.device()->seek(0);
        out << quint16(Data.size() - sizeof(quint16));
        for(int i = 0; i < UserVector.size(); i++)
            UserVector[i]->GetSocket()->write(Data);
    }
}

void Server::slotAddLog(const QString &msg)
{
    emit signalNewAction(msg);
}

void Server::slotBackup()
{
    worker->startBackup(PrivateDialogVector, LogVector);
}

void Server::ShowDataBaseButton_clicked()
{
    model = new QSqlTableModel(this, db);
    model->setTable("users");
    model->select();

    emit signalShowDataBase(model);
}
