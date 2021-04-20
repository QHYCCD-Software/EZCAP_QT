#include "cfwSetup.h"
#include "ui_cfwSetup.h"

#include "myStruct.h"
#include <QDebug>

CFWSetup *cfwSetup_dialog;

CFWSetup::CFWSetup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CFWSetup)
{
    ui->setupUi(this);

    this->setFixedSize(this->width(), this->height());//设置窗口不可改变大小
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);// hide the "?" help button ont the title bar

    //this->setModal(true);
    this->setWindowModality(Qt::ApplicationModal);

    ui->tableWidget_cfwSetup->horizontalHeader()->setFixedHeight(22);
    ui->tableWidget_cfwSetup->verticalHeader()->setFixedWidth(25);
    ui->tableWidget_cfwSetup->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->tableWidget_cfwSetup->setColumnWidth(0, 50);
}

CFWSetup::~CFWSetup()
{
    delete ui;
}

void CFWSetup::loadFilterSetup()
{
    ui->spinBox_filterSlotsNum->setValue(ix.CFWSlotsNum);
    for(int i=0; i<ix.CFWSlotsNum; i++)
    {
        ui->tableWidget_cfwSetup->item(i, 1)->setText(ix.filterNames_2.at(i));
    }
}

void CFWSetup::on_spinBox_filterSlotsNum_valueChanged(int arg1)
{
    qDebug() << "value change " << arg1;

    int curItemsCount = ui->tableWidget_cfwSetup->rowCount();
    qDebug() << "items count" << curItemsCount;

    if(arg1 < curItemsCount)
    {
        for(int i=curItemsCount; i>arg1; i--)
        {
            ui->tableWidget_cfwSetup->removeRow(i-1);
        }
    }
    else if(arg1 > curItemsCount)
    {
        for(int i=curItemsCount; i<arg1; i++)
        {
            ui->tableWidget_cfwSetup->insertRow(i);

            QTableWidgetItem *tempItem1 = new QTableWidgetItem();
            tempItem1->setText(QString::number(i+1));
            tempItem1->setFlags(Qt::NoItemFlags);
            tempItem1->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget_cfwSetup->setItem(i, 0, tempItem1);

            QTableWidgetItem *tempItem2 = new QTableWidgetItem();
            tempItem2->setTextAlignment(Qt::AlignCenter);
            tempItem2->setText(QString("Pos %1").arg(i+1));
            ui->tableWidget_cfwSetup->setItem(i, 1, tempItem2);
        }
    }
}

void CFWSetup::on_tableWidget_cfwSetup_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    //qDebug() << current->column() << current->row() << current->text();
    previousText = current->text();
}

void CFWSetup::on_tableWidget_cfwSetup_itemChanged(QTableWidgetItem *item)
{
    if(item->column() == 1)
    {
        if(QString::compare(item->text(), QString("")) == 0)
        {
            item->setText(previousText);
        }
    }
}

void CFWSetup::on_pushButton_cancle_clicked()
{
    this->close();
}

void CFWSetup::on_pushButton_ok_clicked()
{
    ix.CFWSlotsNum = ui->spinBox_filterSlotsNum->value();
    iniFileParams.CFWSlotsNum = ix.CFWSlotsNum;
    //update ix.filterNames context
    ix.filterNames_2.clear();
    for(int i=0; i<ix.CFWSlotsNum; i++)
    {
        ix.filterNames_2.append(ui->tableWidget_cfwSetup->item(i, 1)->text());
    }
    //update iniFileParams.filterNames context
    iniFileParams.filterNames_2.clear();
    for(int i=0; i<ix.CFWSlotsNum; i++)
    {
        iniFileParams.filterNames_2.append(ix.filterNames_2.at(i));
    }

    //emit signal to CFWControl to exec the slot
    emit updateFilterNames();

    this->close();
}
