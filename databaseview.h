#ifndef DATABASEVIEW_H
#define DATABASEVIEW_H

#include <QDialog>
#include <QSqlTableModel>
#include <QFontDatabase>
#include <QMouseEvent>

namespace Ui {
class DataBaseView;
}

class DataBaseView : public QDialog
{
    Q_OBJECT

public:
    explicit DataBaseView(QWidget *parent = nullptr);
    ~DataBaseView();

public slots:
    void slotShowDataBase(QSqlTableModel* model);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    Ui::DataBaseView *ui;
    QPoint m_pCursor;
};

#endif // DATABASEVIEW_H
