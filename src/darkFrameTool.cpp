#include "darkframetool.h"
#include "ui_darkFrameTool.h"
#include "planner.h"
//#include "qhyccdStatus.h"

#include <QDebug>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QMouseEvent>
#include <QSettings>
#include <QMutex>
#include <QItemDelegate>

DarkFrameTool *darkFrameTool_dialog;
extern bool darkframeflag;

DarkFrameTool::DarkFrameTool(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DarkFrameTool)
{
    ui->setupUi(this);

    this->setFixedSize(this->width(), this->height());//fixed the size of the window
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);// hide the "?" help button ont the title bar
}

DarkFrameTool::~DarkFrameTool()
{
    delete ui;
}



void DarkFrameTool::Save2PLN(QString name,int rowcount,int value[])
{
    QFile file(name);

    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QDataStream data(&file);
        QString str;

        data.setVersion(QDataStream::Qt_4_8);
        // Write a header with a "magic number"
        data << quint32(0xA0B0C0D0);

        for( int r = 0; r < rowcount; ++r )
        {
            if(str.size() != 0)
                str += "\n";

            //always used in capture dark frame script
            str += "1";

            char tmpvalue[64];

            //add the table item to csv file
            for( int c = 1; c < 9; ++c )
            {
                memset(tmpvalue,'\0',64);
                sprintf(tmpvalue,"%d",value[8 * r + c - 1]);
                str += "," + QString(tmpvalue);//ui->tableWidget_planner->item(r, c)->text();
            }
        }
        data << str;

        file.close();
    }
}


void DarkFrameTool::on_pBtn_startcapture_clicked()
{
    int value[2048];
    int cfwpos = 0,row = 0;
    int col = 8;

    if(ui->radioButton_filter0->QAbstractButton::isChecked() == true)
    {
        cfwpos = 0;
    }
    else if(ui->radioButton_filter1->QAbstractButton::isChecked() == true)
    {
        cfwpos = 1;
    }
    else if(ui->radioButton_filter2->QAbstractButton::isChecked() == true)
    {
        cfwpos = 2;
    }
    else if(ui->radioButton_filter3->QAbstractButton::isChecked() == true)
    {
        cfwpos = 3;
    }
    else if(ui->radioButton_filter4->QAbstractButton::isChecked() == true)
    {
        cfwpos = 4;
    }
    else if(ui->radioButton_filter5->QAbstractButton::isChecked() == true)
    {
        cfwpos = 5;
    }
    else if(ui->radioButton_filter6->QAbstractButton::isChecked() == true)
    {
        cfwpos = 6;
    }
    else if(ui->radioButton_filter7->QAbstractButton::isChecked() == true)
    {
        cfwpos = 7;
    }
    else if(ui->radioButton_filter8->QAbstractButton::isChecked() == true)
    {
        cfwpos = 8;
    }

    if(ui->checkBox_exp_10S->QAbstractButton::isChecked() == true)
    {
        if(ui->checkBox_gain_0->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 10;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 0;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_10->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 10;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 10;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_20->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 10;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 20;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_30->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 10;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 30;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_40->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 10;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 40;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_50->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 10;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 50;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_60->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 10;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 60;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_70->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 10;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 70;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

    }

    if(ui->checkBox_exp_30S->QAbstractButton::isChecked() == true)
    {
        if(ui->checkBox_gain_0->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 30;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 0;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_10->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 30;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 10;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_20->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 30;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 20;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_30->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 30;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 30;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_40->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 30;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 40;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_50->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 30;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 50;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_60->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 30;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 60;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_70->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 30;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 70;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

    }

    if(ui->checkBox_exp_60S->QAbstractButton::isChecked() == true)
    {
        if(ui->checkBox_gain_0->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 60;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 0;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_10->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 60;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 10;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_20->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 60;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 20;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_30->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 60;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 30;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_40->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 60;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 40;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_50->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 60;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 50;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_60->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 60;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 60;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_70->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 60;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 70;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

    }

    if(ui->checkBox_exp_120S->QAbstractButton::isChecked() == true)
    {
        if(ui->checkBox_gain_0->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 120;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 0;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_10->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 120;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 10;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_20->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 120;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 20;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_30->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 120;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 30;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_40->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 120;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 40;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_50->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 120;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 50;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_60->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 120;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 60;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_70->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 120;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 70;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

    }

    if(ui->checkBox_exp_180S->QAbstractButton::isChecked() == true)
    {
        if(ui->checkBox_gain_0->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 180;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 0;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_10->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 180;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 10;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_20->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 180;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 20;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_30->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 180;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 30;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_40->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 180;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 40;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_50->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 180;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 50;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_60->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 180;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 60;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_70->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 180;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 70;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

    }

    if(ui->checkBox_exp_300S->QAbstractButton::isChecked() == true)
    {
        if(ui->checkBox_gain_0->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 300;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 0;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_10->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 300;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 10;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_20->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 300;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 20;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_30->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 300;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 30;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_40->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 300;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 40;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_50->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 300;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 50;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_60->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 300;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 60;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_70->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 300;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 70;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

    }

    if(ui->checkBox_exp_600S->QAbstractButton::isChecked() == true)
    {
        if(ui->checkBox_gain_0->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 600;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 0;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_10->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 600;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 10;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_20->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 600;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 20;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_30->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 600;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 30;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_40->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 600;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 40;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_50->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 600;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 50;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_60->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 600;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 60;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_70->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 600;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 70;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

    }

    if(ui->checkBox_exp_1200S->QAbstractButton::isChecked() == true)
    {
        if(ui->checkBox_gain_0->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 1200;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 0;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_10->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 1200;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 10;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_20->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 1200;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 20;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_30->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 1200;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 30;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_40->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 1200;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 40;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_50->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 1200;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 50;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_60->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 1200;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 60;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_70->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 1200;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 70;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

    }

    if(ui->checkBox_exp_1800S->QAbstractButton::isChecked() == true)
    {
        if(ui->checkBox_gain_0->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 1800;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 0;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_10->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 1800;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 10;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_20->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 1800;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 20;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_30->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 1800;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 30;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_40->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 1800;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 40;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_50->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 1800;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 50;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_60->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 1800;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 60;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_70->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 1800;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 70;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

    }

    if(ui->checkBox_exp_3600S->QAbstractButton::isChecked() == true)
    {
        if(ui->checkBox_gain_0->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 3600;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 0;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_10->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 3600;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 10;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_20->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 3600;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 20;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_30->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 3600;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 30;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_40->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 3600;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 40;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_50->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 3600;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 50;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_60->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 3600;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 60;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_70->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 3600;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 70;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

    }

    if(ui->checkBox_exp_7200S->QAbstractButton::isChecked() == true)
    {
        if(ui->checkBox_gain_0->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 7200;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 0;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_10->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 7200;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 10;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_20->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 7200;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 20;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_30->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 7200;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 30;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_40->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 7200;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 40;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_50->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 7200;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 50;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_60->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 7200;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 60;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

        if(ui->checkBox_gain_70->QAbstractButton::isChecked() == true)
        {
            value[row * col + 0] = 1;
            value[row * col + 1] = 7200;
            value[row * col + 2] = 20;
            value[row * col + 3] = cfwpos;
            value[row * col + 4] = 0;
            value[row * col + 5] = 70;
            value[row * col + 6] = 1;
            value[row * col + 7] = 0;
            row++;
        }

    }
    Save2PLN(QString("darkframetool.PLN"),row,value);

    planner_dialog->show();

    darkFrameTool_dialog->close();

    planner_dialog->loadPLN("./darkframetool.PLN");

    darkframeflag = true;

    planner_dialog->on_pBtn_start_planner_clicked();

}
