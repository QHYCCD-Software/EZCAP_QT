#include "planner.h"
#include "ui_planner.h"
#include "ezCap.h"
#include "delegate.h"
#include "fitsio.h"
#include "managementMenu.h"
#include "ui_managementMenu.h"
#include "cfwControl.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QMouseEvent>
#include <QSettings>
#include <QMutex>
#include <QItemDelegate>

Planner *planner_dialog;
extern struct IX ix;

extern char camid[32];

Planner::Planner(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Planner)
{
    ui->setupUi(this);

    //窗口大小固定
    setFixedSize(this->width(), this->height());
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);// hide the "?" help button ont the title bar

    //获取父窗口指针
    mainWidget = (EZCAP*)this->parentWidget();

    //为table中各列加载委托，用以限制其输入格式等
    BinDelegate *spinBoxDelegate = new BinDelegate(this);
    ui->tableWidget_planner->setItemDelegateForColumn(1,spinBoxDelegate);
    ExpTimeDelegate *expTimeDelegate = new ExpTimeDelegate(this);
    ui->tableWidget_planner->setItemDelegateForColumn(2,expTimeDelegate);
    RepeatDelegate *repeatDelegate = new RepeatDelegate(this);
    ui->tableWidget_planner->setItemDelegateForColumn(3,repeatDelegate);
    CFWDelegate *cfwDelegate = new CFWDelegate(this);
    ui->tableWidget_planner->setItemDelegateForColumn(4,cfwDelegate);
    ExpTimeDelegate *delayDelegate = new ExpTimeDelegate(this);
    ui->tableWidget_planner->setItemDelegateForColumn(5,delayDelegate);
    GainDelegate *gainDelegate = new GainDelegate(this);
    ui->tableWidget_planner->setItemDelegateForColumn(6,gainDelegate);
    SubDarkDelegate *avgDelegate = new SubDarkDelegate(this);
    ui->tableWidget_planner->setItemDelegateForColumn(7,avgDelegate);
    SubDarkDelegate *subDarkDelegate = new SubDarkDelegate(this);
    ui->tableWidget_planner->setItemDelegateForColumn(8,subDarkDelegate);
    SubDarkDelegate *subBiasDelegate = new SubDarkDelegate(this);
    ui->tableWidget_planner->setItemDelegateForColumn(9,subBiasDelegate);

    //设置planner table不可改变大小
    ui->tableWidget_planner->resizeColumnsToContents();
    ui->tableWidget_planner->resizeRowsToContents();

    //显示表头
    ui->tableWidget_planner->verticalHeader()->setVisible(true);
    ui->tableWidget_planner->horizontalHeader()->setVisible(true);

    ui->tableWidget_planner->horizontalHeader()->setFixedHeight(22);
    ui->tableWidget_planner->verticalHeader()->setFixedWidth(25);
    ui->tableWidget_planner->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

    //set default column width
    ui->tableWidget_planner->setColumnWidth(0, 22);
    ui->tableWidget_planner->setColumnWidth(1, 35);
    ui->tableWidget_planner->setColumnWidth(3, 44);
    ui->tableWidget_planner->setColumnWidth(4, 35);
    ui->tableWidget_planner->setColumnWidth(5, 54);
    ui->tableWidget_planner->setColumnWidth(6, 44);
    ui->tableWidget_planner->setColumnWidth(7, 35);

    ui->colorBar1_planner->setVisible(false);//默认颜色条不显示
    colorSign = 1;//记录初始颜色为yellow
    firstRowIsBais = false; //标记 是否第一行用于bias拍摄

    mainWidget->exePlanTable = NULL;

}

Planner::~Planner()
{

    delete ui;
}

void Planner::cfwSetup_updated()
{
    if(ix.canFilterWheel)
    {
        CFWDelegate *cfwDelegate = static_cast<CFWDelegate*>(ui->tableWidget_planner->itemDelegateForColumn(4));
        cfwDelegate->setRange(0, ix.CFWSlotsNum);
    }
}

void Planner::camera_connected()
{
    //high speed
    ui->cBox_highReadSpeed_planner->setVisible(ix.canHighSpeed);

    //filter wheel
    ui->cBox_colorWheel_planner->setVisible(ix.canFilterWheel);
    ui->spinBox_colorWheel_planner->setVisible(ix.canFilterWheel);
    ui->colorBar2_planner->setVisible(ix.canFilterWheel);

    //limit the gain setting range
    GainDelegate *gainDelegate = static_cast<GainDelegate*>(ui->tableWidget_planner->itemDelegateForColumn(6));
    gainDelegate->setRange(ix.gainMin, ix.gainMax);

    if(ix.canFilterWheel)
    {
        //limit cfw setting range
        CFWDelegate *cfwDelegate = static_cast<CFWDelegate*>(ui->tableWidget_planner->itemDelegateForColumn(4));
        cfwDelegate->setRange(0, ix.CFWSlotsNum);
    }

    //初始化表格内容 设置只能输入数字以及初始值
    QTableWidgetItem *item1,*item2,*item3,*item4,*item5,*item6,*item7,*item8,*item9;

    ui->tableWidget_planner->setRowHeight(0,22);  //设置第0行高度
    if(ix.CamID.indexOf("MINICAM5") != -1)
    {
        firstRowIsBais = true;

        //第一行固定给Bias场次拍摄
        QTableWidgetItem *checkColumn = new QTableWidgetItem();  //use
        checkColumn->setCheckState(Qt::Checked);//设置复选框状态，Checked为选中状态
        checkColumn->setFlags(Qt::NoItemFlags);
        ui->tableWidget_planner->setItem(0,0,checkColumn);
        item1 = new QTableWidgetItem();    //bin
        item1->setData(Qt::DisplayRole,1);
        item1->setFlags(Qt::NoItemFlags);
        ui->tableWidget_planner->setItem(0,1,item1);
        item2 = new QTableWidgetItem();    //exp
        item2->setData(Qt::DisplayRole,0);
        item2->setFlags(Qt::NoItemFlags);
        ui->tableWidget_planner->setItem(0,2,item2);
        item3 = new QTableWidgetItem();   //repeat
        item3->setData(Qt::DisplayRole,20);
        item3->setFlags(Qt::NoItemFlags);
        ui->tableWidget_planner->setItem(0,3,item3);
        item4 = new QTableWidgetItem();   //cfw
        item4->setData(Qt::DisplayRole,0);
        item4->setFlags(Qt::NoItemFlags);
        ui->tableWidget_planner->setItem(0,4,item4);
        item5 = new QTableWidgetItem();   //delay
        item5->setData(Qt::DisplayRole,0);
        item5->setFlags(Qt::NoItemFlags);
        ui->tableWidget_planner->setItem(0,5,item5);
        item6 = new QTableWidgetItem();   //gain
        item6->setData(Qt::DisplayRole,0);
        item6->setFlags(Qt::NoItemFlags);
        ui->tableWidget_planner->setItem(0,6,item6);
        item7 = new QTableWidgetItem();   //avg
        item7->setData(Qt::DisplayRole,1);
        item7->setFlags(Qt::NoItemFlags);
        ui->tableWidget_planner->setItem(0,7,item7);
        item8 = new QTableWidgetItem();   //subbalck
        item8->setData(Qt::DisplayRole,0);
        item8->setFlags(Qt::NoItemFlags);
        ui->tableWidget_planner->setItem(0,8,item8);
        item9 = new QTableWidgetItem();   //subbias
        item9->setData(Qt::DisplayRole, 0);
        item9->setFlags(Qt::NoItemFlags);
        ui->tableWidget_planner->setItem(0,9,item9);
    }
    else
    {
        firstRowIsBais = false;

        QTableWidgetItem *checkColumn = new QTableWidgetItem();
        checkColumn->setCheckState(Qt::Unchecked);//设置复选框默认为未选状态，Checked为选中状态
        checkColumn->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsDragEnabled);
        ui->tableWidget_planner->setItem(0,0,checkColumn);
        item1 = new QTableWidgetItem();
        item1->setData(Qt::DisplayRole,1);
        ui->tableWidget_planner->setItem(0, 1, item1);
        item2 = new QTableWidgetItem();
        item2->setData(Qt::DisplayRole,1);
        ui->tableWidget_planner->setItem(0,2,item2);
        item3 = new QTableWidgetItem();
        item3->setData(Qt::DisplayRole,1);
        ui->tableWidget_planner->setItem(0,3,item3);
        item4 = new QTableWidgetItem();
        item4->setData(Qt::DisplayRole,0);
        ui->tableWidget_planner->setItem(0,4,item4);
        item5 = new QTableWidgetItem();
        item5->setData(Qt::DisplayRole,0);
        ui->tableWidget_planner->setItem(0,5,item5);
        item6 = new QTableWidgetItem();
        item6->setData(Qt::DisplayRole,0);
        ui->tableWidget_planner->setItem(0,6,item6);
        item7 = new QTableWidgetItem();
        item7->setData(Qt::DisplayRole,0);
        ui->tableWidget_planner->setItem(0,7,item7);
        item8 = new QTableWidgetItem();
        item8->setData(Qt::DisplayRole,0);
        ui->tableWidget_planner->setItem(0,8,item8);
        item9 = new QTableWidgetItem();
        item9->setData(Qt::DisplayRole, 0);
        ui->tableWidget_planner->setItem(0,9,item9);
    }

    //初始化第1到第10行内容
    for(int i = 1; i < 10; i++)
    {
        ui->tableWidget_planner->setRowHeight(i,22);
        item1 = new QTableWidgetItem();
        item1->setData(Qt::DisplayRole,1);
        ui->tableWidget_planner->setItem(i,1,item1);
        item2 = new QTableWidgetItem();
        item2->setData(Qt::DisplayRole,1);
        ui->tableWidget_planner->setItem(i,2,item2);
        item3 = new QTableWidgetItem();
        item3->setData(Qt::DisplayRole,1);
        ui->tableWidget_planner->setItem(i,3,item3);
        item4 = new QTableWidgetItem();
        item4->setData(Qt::DisplayRole,0);
        ui->tableWidget_planner->setItem(i,4,item4);
        item5 = new QTableWidgetItem();
        item5->setData(Qt::DisplayRole,0);
        ui->tableWidget_planner->setItem(i,5,item5);
        item6 = new QTableWidgetItem();
        item6->setData(Qt::DisplayRole,0);
        ui->tableWidget_planner->setItem(i,6,item6);
        item7 = new QTableWidgetItem();
        item7->setData(Qt::DisplayRole,0);
        ui->tableWidget_planner->setItem(i,7,item7);
        item8 = new QTableWidgetItem();
        item8->setData(Qt::DisplayRole,0);
        ui->tableWidget_planner->setItem(i,8,item8);
        item9 = new QTableWidgetItem();
        item9->setData(Qt::DisplayRole, 0);
        ui->tableWidget_planner->setItem(i,9,item9);
    }

}

/*********************************************************************************************************
 *       任务表相关操作
 *********************************************************************************************************/
/**
 * @brief Planner::saveASPLN     保存计划任务表为pln文件
 * @param name
 */
void Planner::saveASPLN(QString name)
{
    QFile file(name);

    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QDataStream data(&file);
        QString str;

        data.setVersion(QDataStream::Qt_4_8);
        // Write a header with a "magic number"
        data << quint32(0xA0B0C0D0);

        for( int r = 0; r < ui->tableWidget_planner->rowCount(); ++r )
        {
            if(str.size() != 0)
                str += "\n";
            if(ui->tableWidget_planner->item(r,0)->checkState() == Qt::Checked)
                str += "1";
            else
                str += "0";

            //add the table item to csv file
            for( int c = 1; c < ui->tableWidget_planner->columnCount(); ++c )
            {
                //Load items
                QTableWidgetItem* item = ui->tableWidget_planner->item(r,c);
                if (!item || item->text().isEmpty())
                {
                    //IF there is nothing write ""
                    ui->tableWidget_planner->setItem(r, c, new QTableWidgetItem(""));
                }
                str += "," + ui->tableWidget_planner->item(r, c)->text();
            }
        }
        data << str;

        file.close();
    }
}

/**
 * @brief Planner::loadPLN    加载pln文件
 * @param name
 */
void Planner::loadPLN(QString name)
{
    quint32 magic;
    QString str;
    QStringList rowOfData;
    QStringList rowData;

    QFile file(name);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_4_8);

        in >> magic ;
        if (magic != 0xA0B0C0D0)
        {
            QMessageBox::warning(this,tr("Warning"),tr("bad file format!"),QMessageBox::Ok);
        }
        else
            in >> str;
        file.close();
    }

    rowOfData = str.split("\n");
    for (int x = 1; x < rowOfData.size(); x++)
    {
        //Number of collumn
        rowData = rowOfData.at(x).split(",");

        //如果row count超出当前表格rowCount，新增row
        if(x > ui->tableWidget_planner->rowCount()-1)
            ui->tableWidget_planner->insertRow(x);

        QTableWidgetItem *checkColumn = new QTableWidgetItem();
        checkColumn->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsDragEnabled);
        if(rowData[0] == "1")
            checkColumn->setCheckState(Qt::Checked);//设置复选框默认为未选状态，Checked为选中状态
        else
            checkColumn->setCheckState(Qt::Unchecked);
        ui->tableWidget_planner->setItem(x, 0, checkColumn);

        for(int y = 1; y < rowData.size(); y++)
        {
            if(rowData[y] != " ")
            {
                QTableWidgetItem *item1 = new QTableWidgetItem();
                item1->setData(Qt::DisplayRole,rowData[y].toInt());
                ui->tableWidget_planner->setItem(x, y, item1);
            }
        }
    }


}

/**
 * @brief Planner::on_pBtn_loadTable_clicked    响应load按钮
 */
void Planner::on_pBtn_loadTable_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open"),
        "",
        tr("*.PLN"));
    if(fileName.isEmpty())
    {
        return;
    }
    else
    {
        loadPLN(fileName);
    }
}

/**
 * @brief Planner::on_pBtn_saveTable_clicked    响应save按钮
 */
void Planner::on_pBtn_saveTable_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this,
        tr("Save"),
        "",
        tr("*.PLN")); //选择路径
    if(filename.isEmpty())
    {
        return;
    }
    else
    {
        saveASPLN(filename);
    }
}

/**
 * @brief Planner::on_pBtn_addRow_clicked     响应add row 按钮
 */
void Planner::on_pBtn_addRow_clicked()
{
    int rowCount = ui->tableWidget_planner->rowCount();
    //add row
    ui->tableWidget_planner->insertRow(rowCount);
    //设置新加行的行高
    ui->tableWidget_planner->setRowHeight(rowCount,22);

    //为新增行添加checkbox以及初始值
    QTableWidgetItem *checkColumn = new QTableWidgetItem();
    checkColumn->setCheckState(Qt::Unchecked);//设置复选框默认为未选状态，Checked为选中状态
    checkColumn->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsDragEnabled);
    ui->tableWidget_planner->setItem(rowCount,0,checkColumn);
    QTableWidgetItem *item1 = new QTableWidgetItem();   //bin
    item1->setData(Qt::DisplayRole,1);
    ui->tableWidget_planner->setItem(rowCount,1,item1);
    QTableWidgetItem *item2 = new QTableWidgetItem();   //exp
    item2->setData(Qt::DisplayRole,1);
    ui->tableWidget_planner->setItem(rowCount,2,item2);
    QTableWidgetItem *item3 = new QTableWidgetItem();   //repeat
    item3->setData(Qt::DisplayRole,1);
    ui->tableWidget_planner->setItem(rowCount,3,item3);
    QTableWidgetItem *item4 = new QTableWidgetItem();   //cfw
    item4->setData(Qt::DisplayRole,0);
    ui->tableWidget_planner->setItem(rowCount,4,item4);
    QTableWidgetItem *item5 = new QTableWidgetItem();   //delay
    item5->setData(Qt::DisplayRole,0);
    ui->tableWidget_planner->setItem(rowCount,5,item5);
    QTableWidgetItem *item6 = new QTableWidgetItem();   //gain
    item6->setData(Qt::DisplayRole,0);
    ui->tableWidget_planner->setItem(rowCount,6,item6);
    QTableWidgetItem *item7 = new QTableWidgetItem();   //avg
    item7->setData(Qt::DisplayRole,0);
    ui->tableWidget_planner->setItem(rowCount,7,item7);
    QTableWidgetItem *item8 = new QTableWidgetItem();   //subdark
    item8->setData(Qt::DisplayRole,0);
    ui->tableWidget_planner->setItem(rowCount,8,item8);
    QTableWidgetItem *item9 = new QTableWidgetItem();   //subbias
    item9->setData(Qt::DisplayRole,0);
    ui->tableWidget_planner->setItem(rowCount,9,item9);

}

/**
 * @brief TableWidget::setRowColor   设置某一行的颜色
 * @param row
 * @param color
 */
void Planner::setRowColor(int row, QColor color)
{
    for (int col = 0; col < ui->tableWidget_planner->columnCount(); col++)
    {
        QTableWidgetItem *item = ui->tableWidget_planner->item(row, col);
        item->setBackgroundColor(color);
    }
}

/**
 * @brief Planner::on_tableWidget_planner_cellClicked  点击table中的某一单元
 * @param row
 * @param column
 */
void Planner::on_tableWidget_planner_cellClicked(int row, int column)
{
    if(column == 0)
    {
        if(firstRowIsBais && row == 0)
        {
        }
        else
        {
            if(ui->tableWidget_planner->item(row,column)->checkState() == Qt::Unchecked)
            {
                ui->tableWidget_planner->item(row,column)->setCheckState(Qt::Checked);
                setRowColor(row,QColor(80,50,0));
            }
            else
            {
                ui->tableWidget_planner->item(row,column)->setCheckState(Qt::Unchecked);
                setRowColor(row,QColor(17,17,17));
            }
        }
    }
}

/***********************************************************************************************************
 *           文件路径选择
 **********************************************************************************************************/
/**
 * @brief Planner::on_pBtn_folder_planner_clicked    响应select folder按钮
 */
void Planner::on_pBtn_folder_planner_clicked()
{
    QString fileDir = QFileDialog::getExistingDirectory(this,
        "",
        "");
    if(fileDir.isEmpty())
    {
        return;
    }
    else
    {
        ui->pBtn_folder_planner->setText(fileDir);
    }
}

/***********************************************************************************************************
 *           文件名改变
 **********************************************************************************************************/
/**
 * @brief Planner::changeFileName     改变文件名操作
 * @param str
 */
void Planner::changeFileName(QString str)
{
    ui->textEdit_fileName->setText(str);
}

/**
 * @brief Planner::on_pBtn_ngc_clicked
 */
void Planner::on_pBtn_ngc_clicked()
{
    changeFileName("NGC");
}

/**
 * @brief Planner::on_pBtn_m_clicked
 */
void Planner::on_pBtn_m_clicked()
{
    changeFileName("M");
}

/**
 * @brief Planner::on_pBtn_ic_clicked
 */
void Planner::on_pBtn_ic_clicked()
{
    changeFileName("IC");
}

/**
 * @brief Planner::on_pBtn_clear_clicked
 */
void Planner::on_pBtn_clear_clicked()
{
    changeFileName("");
}

/**
 * @brief Planner::on_pBtn_1_planner_clicked
 */
void Planner::on_pBtn_1_planner_clicked()
{
    QString text = ui->textEdit_fileName->document()->toPlainText();
    changeFileName(text+"1");
}

/**
 * @brief Planner::on_pBtn_2_planner_clicked
 */
void Planner::on_pBtn_2_planner_clicked()
{
    QString text = ui->textEdit_fileName->document()->toPlainText();
    changeFileName(text+"2");
}

/**
 * @brief Planner::on_pBtn_3_planner_clicked
 */
void Planner::on_pBtn_3_planner_clicked()
{
    QString text = ui->textEdit_fileName->document()->toPlainText();
    changeFileName(text+"3");
}

/**
 * @brief Planner::on_pBtn_4_planner_clicked
 */
void Planner::on_pBtn_4_planner_clicked()
{
    QString text = ui->textEdit_fileName->document()->toPlainText();
    changeFileName(text+"4");
}

/**
 * @brief Planner::on_pBtn_5_planner_clicked
 */
void Planner::on_pBtn_5_planner_clicked()
{
    QString text = ui->textEdit_fileName->document()->toPlainText();
    changeFileName(text+"5");
}

/**
 * @brief Planner::on_pBtn_6_planner_clicked
 */
void Planner::on_pBtn_6_planner_clicked()
{
    QString text = ui->textEdit_fileName->document()->toPlainText();
    changeFileName(text+"6");
}

/**
 * @brief Planner::on_pBtn_7_planner_clicked
 */
void Planner::on_pBtn_7_planner_clicked()
{
    QString text = ui->textEdit_fileName->document()->toPlainText();
    changeFileName(text+"7");
}

/**
 * @brief Planner::on_pBtn_8_planner_clicked
 */
void Planner::on_pBtn_8_planner_clicked()
{
    QString text = ui->textEdit_fileName->document()->toPlainText();
    changeFileName(text+"8");
}

/**
 * @brief Planner::on_pBtn_9_planner_clicked
 */
void Planner::on_pBtn_9_planner_clicked()
{
    QString text = ui->textEdit_fileName->document()->toPlainText();
    changeFileName(text+"9");
}

/**
 * @brief Planner::on_pBtn_0_planner_clicked
 */
void Planner::on_pBtn_0_planner_clicked()
{
    QString text = ui->textEdit_fileName->document()->toPlainText();
    changeFileName(text+"0");
}

/**
 * @brief Planner::on_pBtn_lParentheses_planner_clicked
 */
void Planner::on_pBtn_lParentheses_planner_clicked()
{
    QString text = ui->textEdit_fileName->document()->toPlainText();
    changeFileName(text+"(");
}

/**
 * @brief Planner::on_pBtn_rParentheses_planner_clicked
 */
void Planner::on_pBtn_rParentheses_planner_clicked()
{
    QString text = ui->textEdit_fileName->document()->toPlainText();
    changeFileName(text+")");
}


/*****************************************************************************************************************************
 *      其他功能设置相关函数操作
 ****************************************************************************************************************************/
/**
 * @brief Planner::on_cBox_colorWheel_planner_clicked    响应colorWheel checkbox点击
 */
void Planner::on_cBox_colorWheel_planner_clicked()
{
    if(ui->cBox_colorWheel_planner->checkState() == Qt::Checked)
    {
        ui->spinBox_colorWheel_planner->setEnabled(true);
        ui->colorBar1_planner->setVisible(true);//显示颜色条
    }
    else
    {
        ui->spinBox_colorWheel_planner->setEnabled(false);
        ui->colorBar1_planner->setVisible(false);
    }
}

/**
 * @brief Planner::on_cBox_highSpeed_planner_clicked    选择高速读出模式
 */
void Planner::on_cBox_highReadSpeed_planner_clicked()
{
    if(ui->cBox_highReadSpeed_planner->isChecked())
    {
        QMessageBox::warning(this,tr("Warning"),tr("High Readout Speed will increase readout noise,Please confirm before capture"));
    }

}

/**
 * @brief Planner::on_cBox_darkFrame_planner_clicked  选择DarkFrameCapture模式
 */
void Planner::on_cBox_darkFrame_planner_clicked()
{
    if(ui->cBox_darkFrame_planner->isChecked())
    {
        QMessageBox::warning(this,tr("Warning"),tr("This is dark frame capture.The shutter will not open during exposuring,Please confirm before capture"));

        ix.MechanicalShutterMode = 1;
    }
    else
    {
        ix.MechanicalShutterMode = 0;
    }
}


/**
 * @brief Planner::on_colorBar1_planner_clicked    颜色条被点击
 */
void Planner::colorBar1_planner_clicked()
{
    //由于colorBar控件没有直接的点击事件，所以通过响应鼠标的mousePressEvent来实现

    if(colorSign == 1)
    {//当前颜色yellow
        ui->colorBar1_planner->setBackground(QBrush(QColor(0,0,255)));
        colorSign = 4;//记录颜色为blue
    }
    else if(colorSign == 2)
    {//当前green
        ui->colorBar1_planner->setBackground(QBrush(QColor(255,0,0)));
        colorSign = 3;//记录颜色为red
    }
    else if(colorSign == 3)
    {//当前颜色red
        ui->colorBar1_planner->setBackground(QBrush(QColor(255,255,0)));
        colorSign = 1;//记录颜色为yellow
    }
    else if(colorSign == 4)
    {//当前颜色blue
        ui->colorBar1_planner->setBackground((QBrush(QColor(0,255,0))));
        colorSign = 2;//记录颜色为green
    }
    //需要刷新控件显示才能显示修改后的颜色，故采用先隐藏在显示的方式使得控件刷新
    ui->colorBar1_planner->setVisible(false);
    ui->colorBar1_planner->setVisible(true);
}

/**
 * @brief Planner::mousePressEvent  响应鼠标点击事件
 * @param e
 */
void Planner::mousePressEvent(QMouseEvent *e)
{
    int x = e->x();
    int y = e->y();

    //假如在QRect( left:525, top:10, width:10, height:259)这个区域里(即colorBar1_planner控件所在区域)，就发出信号
    if (ui->cBox_colorWheel_planner->isChecked() && x > 525 && x < 535 && y > 10 && y < 269)
    {
        emit colorBar1_planner_clicked();
    }

}

/**
 * brief Planner::getStatus_planner 获取状态栏内容
 * @return
 */
QString Planner::getStatus_planner()
{
    return ui->status1_planner->text();
}

/**
 * @brief Planner::setStatus_planner  设置状态栏内容
 * @param qstr
 */
void Planner::setStatus_planner(QString qstr)
{
    ui->status1_planner->setText(qstr);
}

/**************************************************************************************
 *        开始或停止planer任务表相关操作函数
 **************************************************************************************/
/**
 * @brief FITwrite  写Fit文件
 * @param namez
 * @param ImageWidth
 * @param ImageHeight
 * @param Buf
 */
void Planner::FITwrite(QString namez,int ImageWidth,int ImageHeight,unsigned char *Buf)
{
    while(QFile::exists(namez))
    {
        int a = namez.indexOf(".");
        int b = namez.length();

        for(int i=0; i<b; i++)
        {	//获取"."字符最后出现的位置
            QString Npos =  namez.mid(i,1);//第i个字符
            QString sample = ".";
            if(Npos.compare(sample) == 0)
            {
                a = i;
            }
        }

        namez = namez.left(a) + "-2" + namez.right(b-a);
    }

    IplImage *FitImg;
    FitImg = cvCreateImage(cvSize(ImageWidth,ImageHeight), IPL_DEPTH_16U, 1 );
    FitImg->imageData = (char*)Buf;

    fitsfile *fptr;       // pointer to the FITS file; defined in fitsio.h
    int status;//, ii, jj;
    long  fpixel = 1, naxis = 2, nelements;//, exposure;
    long naxes[2] = {ImageWidth, ImageHeight };   // image is 300 pixels wide by 200 rows
    //char ss[81];

    status = 0;         // initialize status before calling fitsio routines

    std::string str = namez.toStdString();
    const char* ch = str.c_str();
    fits_create_file(&fptr, ch, &status);

    // Create the primary array image (16-bit short integer pixels
    fits_create_img(fptr, USHORT_IMG, naxis, naxes, &status);

    // Write a keyword; must pass the ADDRESS of the value

    nelements = naxes[0] * naxes[1];          // number of pixels to write

    // Write the array of integers to the image
    fits_write_img(fptr, TUSHORT, fpixel, nelements, &FitImg->imageData[0], &status);
    fits_close_file(fptr, &status);            // close the file
    fits_report_error(stderr, status);  // print out any error messages

    FitImg->imageData=0;
    cvReleaseImage(&FitImg);

}

/**
 * @brief Planner::CheckFilePath  检查文件路径是否安全
 * @param path
 * @return
 */
bool Planner::CheckFilePath(QString path)
{
    //尝试写一个文件，测试文件路径是否安全
    bool i = false;
    unsigned char ImgData[100];

    QString TestName;
    TestName = path + "OnlyForTestFile.fit";

    //qDebug() << TestName;

    QFile::remove(TestName);

    FITwrite(TestName,10,10,ImgData);//保存测试图片

    if(QFile::exists(TestName))
        i=1;
    else
        i=0;

    QFile::remove(TestName);//删除测试图片

    return i;
}

/**
 * @brief Planner::on_pBtn_forceStop_planner_clicked  停止planner任务
 */
void Planner::on_pBtn_forceStop_planner_clicked()
{
    //ix.planestate = 0;
    ix.plannerState = PlannerStatus_Stop;

    ui->status1_planner->setText(tr("ForceSTOP"));
}

void Planner::resetUI()
{
    ui->retranslateUi(this);//切换显示语言后，重设UI界面控件显示内容
}

void Planner::errorInfo_showed(QString title, QString errorInfo)
{
    QMessageBox::warning(this,title,errorInfo,QMessageBox::Ok);
}

/**
 * @brief Planner::on_pBtn_start_planner_clicked  开始planner任务
 */
void Planner::on_pBtn_start_planner_clicked()
{
    QString path = "";

    if(ui->pBtn_folder_planner->text() == "Folder")
    {
        //path = QApplication::applicationDirPath() + "/" + ui->textEdit_fileName->document()->toPlainText();
        path = ui->textEdit_fileName->document()->toPlainText();
    }
    else
    {
        path = ui->pBtn_folder_planner->text() + "/" + ui->textEdit_fileName->document()->toPlainText();
    }
    path = QDir::toNativeSeparators(path);  //On Windows "/" return "\"  Linux or Mac is "\"

    if(CheckFilePath(path))
    {
        //Qt中使用QSettings类读写ini文件
        //QSettings构造函数的第一个参数是ini文件的路径,第二个参数表示针对ini文件,第三个参数可以缺省
        QString path_script = QCoreApplication::applicationDirPath() + "/" + "Script.ini";
        path_script = QDir::toNativeSeparators(path_script);
        QSettings *configIniWrite = new QSettings(path_script, QSettings::IniFormat);

        //清空已有内容
        configIniWrite->remove("");

        //向ini文件中写入内容,setValue函数的两个参数是键值对
        configIniWrite->setValue("/ScriptName/FileName", path);

        configIniWrite->beginGroup("Total");
        configIniWrite->setValue("TaskNum", ui->tableWidget_planner->rowCount());
        configIniWrite->endGroup();

        configIniWrite->beginGroup("Loop");
        if(ui->cBox_colorWheel_planner->isChecked())
            configIniWrite->setValue("LoopNum", ui->spinBox_colorWheel_planner->value());
        else
            configIniWrite->setValue("LoopNum", 1);
        configIniWrite->endGroup();

        if(firstRowIsBais)
        {
            char nameStr[1024];
            sprintf(nameStr,"./Bias/%s.fit",camid);
            if(!QFile::exists(nameStr))
            {   //camera is minicam5, and bais image not found, then do this:
                QMessageBox::information(this, tr("Info"), tr("No found Bias image, will capture Bias image at first. Please cover the lens cap!"),
                                         QMessageBox::Ok);

                configIniWrite->beginGroup("CAP" + QString::number(1));
                configIniWrite->setValue("Enabled", ui->tableWidget_planner->item(0,0)->checkState());
                configIniWrite->setValue("BIN", ui->tableWidget_planner->item(0,1)->text());
                configIniWrite->setValue("EXP", QString::number((int)(ui->tableWidget_planner->item(0,2)->text().toFloat()*1000)));
                configIniWrite->setValue("Repeat", ui->tableWidget_planner->item(0,3)->text());
                configIniWrite->setValue("CFW", ui->tableWidget_planner->item(0,4)->text());
                configIniWrite->setValue("Delay", ui->tableWidget_planner->item(0,5)->text());
                configIniWrite->setValue("GAIN", ui->tableWidget_planner->item(0,6)->text());
                configIniWrite->setValue("AVGEnabled", ui->tableWidget_planner->item(0,7)->text());
                configIniWrite->setValue("SubBlackEnabled", ui->tableWidget_planner->item(0,8)->text());
                configIniWrite->setValue("SubBiasEnabled", ui->tableWidget_planner->item(0,9)->text());
                configIniWrite->endGroup();
            }
        }
        else
        {   //if not mincam5, do these
            configIniWrite->beginGroup("CAP"+QString::number(1));
            if(ui->tableWidget_planner->item(0,0)->checkState() == Qt::Checked)
                configIniWrite->setValue("Enabled", true);
            else
                configIniWrite->setValue("Enabled", false);
            configIniWrite->setValue("BIN", ui->tableWidget_planner->item(0,1)->text());
            configIniWrite->setValue("EXP", QString::number((int)(ui->tableWidget_planner->item(0,2)->text().toFloat()*1000)));
            configIniWrite->setValue("Repeat", ui->tableWidget_planner->item(0,3)->text());
            configIniWrite->setValue("CFW", ui->tableWidget_planner->item(0,4)->text());
            configIniWrite->setValue("Delay", ui->tableWidget_planner->item(0,5)->text());
            configIniWrite->setValue("GAIN", ui->tableWidget_planner->item(0,6)->text());
            configIniWrite->setValue("AVGEnabled", ui->tableWidget_planner->item(0,7)->text());
            configIniWrite->setValue("SubBlackEnabled", ui->tableWidget_planner->item(0,8)->text());
            configIniWrite->setValue("SubBiasEnabled", ui->tableWidget_planner->item(0,9)->text());
            configIniWrite->endGroup();
        }

        for(int i = 2; i <= ui->tableWidget_planner->rowCount(); i++)
        {
            configIniWrite->beginGroup("CAP"+QString::number(i));
            configIniWrite->setValue("Enabled", ui->tableWidget_planner->item(i-1,0)->checkState());
            configIniWrite->setValue("BIN", ui->tableWidget_planner->item(i-1,1)->text());
            configIniWrite->setValue("EXP", QString::number((int)(ui->tableWidget_planner->item(i-1,2)->text().toFloat()*1000)));
            configIniWrite->setValue("Repeat", ui->tableWidget_planner->item(i-1,3)->text());
            configIniWrite->setValue("CFW", ui->tableWidget_planner->item(i-1,4)->text());
            configIniWrite->setValue("Delay", ui->tableWidget_planner->item(i-1,5)->text());
            configIniWrite->setValue("GAIN", ui->tableWidget_planner->item(i-1,6)->text());
            configIniWrite->setValue("AVGEnabled", ui->tableWidget_planner->item(i-1,7)->text());
            configIniWrite->setValue("SubBlackEnabled", ui->tableWidget_planner->item(i-1,8)->text());
            configIniWrite->setValue("SubBiasEnabled", ui->tableWidget_planner->item(i-1,9)->text());
            configIniWrite->endGroup();
        }
        //写入完成后删除指针
        delete(configIniWrite);


        qDebug() << "----start planner thread----";

        ix.plannerState = PlannerStatus_Start;

        ui->status1_planner->setText(tr("Begin"));
        ui->pBtn_start_planner->setEnabled(false);

        if(ix.workMode != WorkMode_Capture)
        {
            managerMenu->ui->head_capture->click();
            QApplication::processEvents(QEventLoop::AllEvents,25);//wait the GUI change finish
        }
        managerMenu->ui->head_preview->setCheckable(false);
        managerMenu->ui->head_focus->setCheckable(false);


        //开启线程执行任务
        if(mainWidget->exePlanTable)
        {
            delete(mainWidget->exePlanTable);
            mainWidget->exePlanTable = NULL;
        }
        mainWidget->exePlanTable = new ExecutePlanTable;

        //采用BlockingQueuedConnection模式,实现等待信号响应完成在继续执行
        QObject::connect(mainWidget->exePlanTable, SIGNAL(changeRowColor(int,QColor)), this, SLOT(setRowColor(int,QColor)), Qt::BlockingQueuedConnection);
        QObject::connect(mainWidget->exePlanTable, SIGNAL(changeCurCFWPos(int)), cfwControl_dialog, SLOT(curCFWPos_changed(int)), Qt::BlockingQueuedConnection);
        QObject::connect(mainWidget->exePlanTable, SIGNAL(startCaptureImage()), managerMenu->ui->pBtn_capture, SLOT(click()), Qt::BlockingQueuedConnection);
        QObject::connect(mainWidget->exePlanTable, SIGNAL(showErrorInfo(QString,QString)), this, SLOT(errorInfo_showed(QString,QString)), Qt::BlockingQueuedConnection);
        QObject::connect(mainWidget->exePlanTable, SIGNAL(finish_baisImages()), this, SLOT(baisImages_finished()), Qt::BlockingQueuedConnection);

        mainWidget->exePlanTable->start();

        while(ix.plannerState != PlannerStatus_Done)
        {
            QApplication::processEvents();//等待界面事件响应完成
        }
        ui->pBtn_start_planner->setEnabled(true);//拍摄计划完成，设置Start按钮使能

        mainWidget->exePlanTable->exit();

        qDebug() << "-----planner thread end----";

        managerMenu->ui->head_capture->setCheckable(true);
        managerMenu->ui->head_preview->setCheckable(true);
        managerMenu->ui->head_focus->setCheckable(true);

    }
    else
    {
        QMessageBox::warning(this,tr("Error"),tr("This folder can not be writen,Please select another folder!"),QMessageBox::Ok);
    }

}

void Planner::baisImages_finished()
{
    QMessageBox::information(this, tr("Info"), tr("Capture Bias image done. Please uncover the lens cap!"), QMessageBox::Ok);
}
