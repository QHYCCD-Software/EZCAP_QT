#ifndef FITHEADER_H
#define FITHEADER_H

#include <QDialog>
#include <QItemSelection>

namespace Ui {
class FitHeader;
}

class FitHeader : public QDialog
{
    Q_OBJECT

public:
    explicit FitHeader(QWidget *parent = 0);
    ~FitHeader();

    void saveAsCSV(QString name);
    void loadCSV(QString fileName);
    void FITRead(QString nameStr, long length, unsigned char *Buf);
    void FITWrite(QString nameStr,unsigned char *Buf);
    void FITwrite_Common(QString filename, int ImageWidth, int ImageHeight, unsigned char *Buf);

    void initFitHeader();

private slots:
    void selectedRowChanged(const QItemSelection & selected, const QItemSelection & deselected);

    void on_pBtn_set_fitHeader_clicked();

    void on_pBtn_delete_fitHeader_clicked();

    void on_keyName_fitHeader_currentIndexChanged(int index);

    void resetUI();//recieve the changeLanguage signal

    void camera_connected();

    void fitHeaderInfo_changed();

protected:
    bool eventFilter(QObject *o, QEvent *e);

public:
    Ui::FitHeader *ui;

private:
    QStringList keyWords;
    QStringList values;
    QStringList dataTypes;
    QStringList descriptions;
};

extern class FitHeader *fitHeader_dialog;//define global class object

#endif // FITHEADER_H
