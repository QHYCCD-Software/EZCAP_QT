#include "cfwControl.h"
#include "ui_cfwControl.h"

#include "cfwSetup.h"
#include "myStruct.h"

#include <QDebug>
#include <QListView>
#include <QTime>

CFWControl *cfwControl_dialog;

CFWControl::CFWControl(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CFWControl)
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());//设置窗口不可改变大小
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);// hide the "?" help button ont the title bar

    filterLoaded = false;
}

CFWControl::~CFWControl()
{
    delete ui;

    if(cfwSetup_dialog)
    {
        delete cfwSetup_dialog;
        cfwSetup_dialog = NULL;
    }
}

void CFWControl::loadFilterSetup()
{
    disconnect(ui->comboBox_cfwPos, SIGNAL(currentIndexChanged(int)),0,0);
    ui->comboBox_cfwPos->clear();
    for(int i=0; i<ix.CFWSlotsNum; i++)
    {
        ui->comboBox_cfwPos->addItem(ix.filterNames_2.at(i));
    }

    if(ui->comboBox_cfwPos->count() > 0)
        ui->comboBox_cfwPos->setView(new QListView());  //结合style.qss中设置item高度

    // this will not work
    filterLoaded = true;  //标记filter窗口初始化完成，用于避免初始化时 执行cfw控制操作
    // filterLoaded will not work at the second time ,and anyother time except first time, so change to disconnect slot
    connect(ui->comboBox_cfwPos, SIGNAL(currentIndexChanged(int)),this,SLOT(on_comboBox_cfwPos_currentIndexChanged(int)));
}

void CFWControl::startCFWProgressBar()
{
    ui->label_curFilterPos->setText(tr("Moving ..."));
    ui->label_curFilterName->setText("-");

    int waitStep = CFW_WAITING_TIME / 100;
    //int waitStep = 28000 / 100;
    for(int i = 0; i < 100; i++)
    {
        //colorWheelRun_dialog->ui->proBar_colorWheelRun->setValue(i);
        ui->progressBar_cfwRunning->setValue(i);

        //delay waitStep...
        QTime dieTime = QTime::currentTime().addMSecs(waitStep);
        while( QTime::currentTime() < dieTime )
            QCoreApplication::processEvents();
        if(ix.CFWStatus == CFW_Idle)
        {   //CFW move done, then break
            i = 100;
        }

        QCoreApplication::processEvents();
    }
    ui->progressBar_cfwRunning->setValue(100);

    emit endCFWProgress();  //emit signal to stop cfwTimer
    if(ix.CFWStatus == CFW_Idle)
    {
        int curIndex = ix.curCfwPos[0] - 0x30;
        if(curIndex > ix.filterNames_2.count() || curIndex < 0){
            qCritical() << QString("Filter wheel Index illeagal   curIndex=[%1]  count=[%2]").arg(curIndex).arg(ix.filterNames_2.count()) ;
            return;
        }
        ui->label_curFilterPos->setText(QString::number(curIndex+1));
        ui->label_curFilterName->setText(ix.filterNames_2.at(curIndex));
    }
    else
    {
        qDebug() << "Filter wheel response timeout!";
        ui->label_curFilterPos->setText(tr("-"));
        ui->label_curFilterName->setText("-");
    }
}

void CFWControl::on_pushButton_setupCFW_clicked()
{
    qDebug() << "show CFW Setup Dialog";
    cfwSetup_dialog->show();

    cfwSetup_dialog->loadFilterSetup();
}

void CFWControl::on_comboBox_cfwPos_currentIndexChanged(int index)
{
    if(index >= 0 && filterLoaded)
    {
        char dst = index + 0x30;

        qDebug() << "change to cfw:" << index << ui->comboBox_cfwPos->itemText(index);
        ix.dstCfwPos = dst;
        emit changeCFWPosition();
    }
}

void CFWControl::filterNames_updated()
{
    int curCount = ui->comboBox_cfwPos->count();

    for(int i=0; i<ix.CFWSlotsNum; i++)
    {
        if(i >= curCount)
            ui->comboBox_cfwPos->addItem(ix.filterNames_2.at(i));
        else
            ui->comboBox_cfwPos->setItemText(i, ix.filterNames_2.at(i));

    }

    if(curCount > ix.CFWSlotsNum)
    {
        for(int i=curCount; i>=ix.CFWSlotsNum; i--)
        {
            ui->comboBox_cfwPos->removeItem(i);
        }
    }
}

void CFWControl::curCFWPos_changed(int dstIndex)
{    
    if(dstIndex >=0 && dstIndex < ui->comboBox_cfwPos->count())
    {
        ui->comboBox_cfwPos->setCurrentIndex(dstIndex);
    }
    else
    {
        qCritical() << QString("index %1 out of range").arg(dstIndex);
    }    
}

void CFWControl::camera_connected()
{
    if(ix.canFilterWheel)
    {
        this->loadFilterSetup();
    }
}
