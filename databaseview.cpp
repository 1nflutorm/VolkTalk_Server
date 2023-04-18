#include "databaseview.h"
#include "ui_databaseview.h"

DataBaseView::DataBaseView(QWidget *parent) : QDialog(parent), ui(new Ui::DataBaseView)
{
    ui->setupUi(this);
    QFontDatabase::addApplicationFont(":/fonts/Roboto-Regular.ttf");
    this->setAttribute(Qt::WA_TranslucentBackground);
    connect(ui->closeButton, &QPushButton::clicked, this, &QWidget::close);
    connect(ui->hideButton, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(ui->maximizeButton, &QPushButton::clicked, this, &QWidget::showMinimized);
}

DataBaseView::~DataBaseView()
{
    delete ui;
}

void DataBaseView::slotShowDataBase(QSqlTableModel *model)
{
    model->setHeaderData(0, Qt::Horizontal, "id");
    model->setHeaderData(1, Qt::Horizontal, "Фамилия");
    model->setHeaderData(2, Qt::Horizontal, "Имя");
    model->setHeaderData(3, Qt::Horizontal, "Отчество");
    model->setHeaderData(4, Qt::Horizontal, "Логин");
    model->setHeaderData(5, Qt::Horizontal, "Пароль");
    ui->DataBaseView_2->setModel(model);
    this->setModal(true);
    this->show();
}

void DataBaseView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_pCursor = event->globalPos() - this->geometry().topLeft();
        event->accept();
    }
}

void DataBaseView::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        this->move(event->globalPos() - m_pCursor);
        event->accept();
    }
}
