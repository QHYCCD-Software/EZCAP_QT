#ifndef DARKFRAMETOOL_H
#define DARKFRAMETOOL_H

#include <QDialog>
#include <QDebug>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QMouseEvent>
#include <QSettings>
#include <QMutex>
#include <QItemDelegate>

namespace Ui {
class DarkFrameTool;
}

class DarkFrameTool : public QDialog
{
    Q_OBJECT

public:
    explicit DarkFrameTool(QWidget *parent = 0);
    ~DarkFrameTool();

    void Save2PLN(QString name,int rowcount,int value[]);

private slots:

    void on_pBtn_startcapture_clicked();

private:
    Ui::DarkFrameTool *ui;
};

extern class DarkFrameTool *darkFrameTool_dialog;//定义全局类对象，供主界面类中使用

#endif // PHDLINK_H
