QT       += core gui
QT       += network
QT       += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    backupworker.cpp \
    databaseview.cpp \
    groupchat.cpp \
    main.cpp \
    privatedialog.cpp \
    server.cpp \
    serverwindow.cpp \
    user.cpp

HEADERS += \
    backupworker.h \
    databaseview.h \
    groupchat.h \
    privatedialog.h \
    server.h \
    serverwindow.h \
    user.h

FORMS += \
    databaseview.ui \
    serverwindow.ui

win32:RC_FILE = icon.rc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resourses.qrc
