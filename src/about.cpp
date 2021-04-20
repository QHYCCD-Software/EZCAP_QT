#include "about.h"
#include "ui_about.h"
//#include "qhyccd.h"
#include "include/dllqhyccd.h"
#include <QException>

#include <QDesktopServices>
#include <QUrl>

About *about_dialog;

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());//设置窗口不可改变大小
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);// hide the "?" help button ont the title bar

}

About::About(QWidget *parent, const QString ver, const QString releaseTime) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);

    setFixedSize(this->width(), this->height());//设置窗口不可改变大小
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);// hide the "?" help button ont the title bar

    this->verNum = ver;
    this->releaseDate = releaseTime;

    ui->label_version->setText(QString("EZCAP_Qt %1").arg(verNum));
    ui->label_version->setFont(QFont("Tahoma", 11,  QFont::Bold));
    ui->label_release->setText(QString("Release on %1").arg(releaseDate));
    ui->label_release->setFont(QFont("Tahoma"));




    ui->label_copyright->setFont(QFont("Tahoma"));

    ui->label_website->setText(tr("<html><head/><body><p><a href='http://www.qhyccd.com'><span style='text-decoration: underline; color:#ECECEC;'>QHYCCD WebSite: http://www.qhyccd.com</span></a></p></body></html>"));
    ui->label_forum->setText(tr("<html><head/><body><p><a href='http://www.qhyccd.com/ccdbbs'><span style='text-decoration: underline; color:#ECECEC;'>QHYCCD Discuss Forum: http://www.qhyccd.com/ccdbbs</span></a></p></body></html>"));
}

About::~About()
{
    delete ui;
}

/**
 * @brief About::on_pBtn_ok_about_clicked       ok 按钮被点击
 */
void About::on_pBtn_ok_about_clicked()
{
    this->close();
}

/**
 * @brief About::on_pBtn_logo_about_clicked     logo图标被点击
 */
void About::on_pBtn_logo_about_clicked()
{

    QDesktopServices::openUrl(QUrl("http://www.qhyccd.com"));
}

void About::resetUI()
{
    ui->retranslateUi(this);

    ui->label_version->setText(QString("EZCAP_Qt %1").arg(verNum));
    ui->label_release->setText(QString("Release on %1").arg(releaseDate));


    try
    {
    uint32_t year,month,day,subday;
    libqhyccd->GetQHYCCDSDKVersion(&year,&month,&day,&subday);
    ui->label_SDKVERSION->setText(QString("with QHYCCD SDK Version:%1-%2-%3-%4").arg(year).arg(month).arg(day).arg(subday));
    }
    catch(QException e)
    {
        qCritical("GetQHYCCDSDKVersion: have no this function !");
    }
}
