#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>

namespace Ui {
class About;
}

class About : public QDialog
{
    Q_OBJECT

public:
    explicit About(QWidget *parent = 0);
    About(QWidget *parent, const QString ver, const QString releaseTime);
    ~About();

private slots:
    void on_pBtn_ok_about_clicked();

    void on_pBtn_logo_about_clicked();

    void resetUI();//recieve the changeLanguage signal

private:
    Ui::About *ui;

    QString verNum;
    QString releaseDate;
};

extern class About *about_dialog;//define global class object

#endif // ABOUT_H
