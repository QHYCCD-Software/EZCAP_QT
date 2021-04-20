#include "fitHeader.h"
#include "ui_fitHeader.h"
#include "ezCap.h"
#include "mainMenu.h"
#include "fitsio.h"

#include <QFile>
#include <QtCore/QTextStream>
#include <QCloseEvent>
#include <QLabel>
#include <QDebug>
#include <QAbstractButton>
#include <QStylePainter>
#include <QDir>
#include <QListView>
#include <QTextCodec>
#include <QDateTime>


FitHeader *fitHeader_dialog;
extern IX ix;

FitHeader::FitHeader(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FitHeader)
{
    ui->setupUi(this);

    setFixedSize(this->width(), this->height());//设置窗口不可改变大小
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);// hide the "?" help button ont the title bar

    ui->keyName_fitHeader->setView(new QListView());  //结合style.qss设置combobox中item高度

    //行和列的大小设为与内容相匹配
    ui->tableWidget_fitHeader->resizeColumnsToContents();
    ui->tableWidget_fitHeader->resizeRowsToContents();
    ui->tableWidget_fitHeader->horizontalHeader()->setFixedHeight(22);//表头高度固定
    //ui->tableWidget_fitHeader->horizontalHeader()->setClickable(false);//列表头不可点击
    ui->tableWidget_fitHeader->setColumnWidth(0,80);
    ui->tableWidget_fitHeader->setColumnWidth(1,80);
    ui->tableWidget_fitHeader->horizontalHeader()->setVisible(true);//表头可见
    ui->tableWidget_fitHeader->verticalHeader()->setVisible(true);
    ui->tableWidget_fitHeader->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed); //行表头宽高固定
    ui->tableWidget_fitHeader->verticalHeader()->setDefaultAlignment(Qt::AlignCenter | Qt::AlignVCenter); //行表头文字居中

    //设置tableWidget左上角CornerButton text
    //Qt中API没有提供直接的访问方式,可以通过findChild()来定位到该按钮,然后设置其文本及显示宽度.
    QAbstractButton *btn = ui->tableWidget_fitHeader->findChild<QAbstractButton*>();
    if (btn)
    {
        btn->setText(tr("Keyword"));
        btn->setStyleSheet("color:white;font:12px Tahoma;");//设置text颜色，字体，大小（位置设置似乎无效）

        btn->installEventFilter(this);//添加事件过滤注册，用于实现设置cornerButton的text,color,positon等

        // adjust the width of the vertical header to match the preferred corner button width
        // (unfortunately QAbstractButton doesn't implement any size hinting functionality)
        QStyleOptionHeader opt;
        opt.text = btn->text();
        QSize s = (btn->style()->sizeFromContents(QStyle::CT_HeaderSection, &opt, QSize(), btn).expandedTo(QApplication::globalStrut()));
        if (s.isValid())
           ui->tableWidget_fitHeader->verticalHeader()->setMinimumWidth(s.width());
    }

    ui->keyName_fitHeader->installEventFilter(this);//在keyname_fitHeader控件上安装过滤器

    //关联表格中 当前选择行改变的信号与响应槽函数
    connect(ui->tableWidget_fitHeader->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(selectedRowChanged(QItemSelection,QItemSelection)));


}

FitHeader::~FitHeader()
{
    delete ui;
}

/**
 * @brief FitHeader::selectedRowChanged   当前选中行改变
 * @param selected
 * @param deselected
 */
void FitHeader::selectedRowChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    //selected.indexes().size()获取选择的行总数
    //获取当前选中的行所在行号，由于tableWidget只允许单选，故selected中只有一个值
    int curRow = selected.at(0).topLeft().row();

    //set currentIndex of keyname comboBox
    ui->keyName_fitHeader->setCurrentIndex(curRow);

}

/**
 * @brief FitHeader::on_keyName_fitHeader_currentIndexChanged   改变keyname comboBoxd当前选择项
 * @param index
 */
void FitHeader::on_keyName_fitHeader_currentIndexChanged(int index)
{
    //set the row of this index selected in table
    //ui->tableWidget_fitHeader->selectRow(index);

    if(ui->tableWidget_fitHeader->item(index, 0) == 0)
    {
        // the item for the given row and column has not been set, item(index, 0) return 0
        ui->value_fitHeader->setText("");
    }
    else
    {
        //set the value of value_fitHeader editText control
        ui->value_fitHeader->setText(ui->tableWidget_fitHeader->item(index,0)->text());
    }

    //set status of radioButtons
    if(ui->tableWidget_fitHeader->item(index,1)->text() == "TINT")
    {
        ui->rBtn_integer_fitHeader->setChecked(true);
    }
    else if(ui->tableWidget_fitHeader->item(index,1)->text() == "TDOUBLE")
    {
        ui->rBtn_double_fitHeader->setChecked(true);
    }
    else if(ui->tableWidget_fitHeader->item(index,1)->text() == "TLOGICAL")
    {
        ui->rBtn_logical_fitHeader->setChecked(true);
    }
    else
    {
        ui->rBtn_string_fitHeader->setChecked(true);
    }

    ui->rBtn_integer_fitHeader->setEnabled(false);
    ui->rBtn_double_fitHeader->setEnabled(false);
    ui->rBtn_logical_fitHeader->setEnabled(false);
    ui->rBtn_string_fitHeader->setEnabled(false);

    QString curKey = ui->tableWidget_fitHeader->verticalHeaderItem(index)->text();
    if(curKey.compare("SIMPLE") == 0 || curKey.compare("BITPIX") == 0 ||
       curKey.compare("NAXIS") == 0 || curKey.compare("NAXIS1") == 0 ||
       curKey.compare("NAXIS2") == 0 || curKey.compare("SWCREATE") == 0 ||
       curKey.compare("INSTRUME") == 0 )
    {
        ui->value_fitHeader->setEnabled(false);
        ui->pBtn_set_fitHeader->setEnabled(false);
        ui->pBtn_delete_fitHeader->setEnabled(false);
    }
    else
    {
        ui->value_fitHeader->setEnabled(true);
        ui->pBtn_set_fitHeader->setEnabled(true);
        ui->pBtn_delete_fitHeader->setEnabled(true);
    }
}

/**
 * @brief FitHeader::on_pBtn_set_fitHeader_clicked  点击set按钮
 */
void FitHeader::on_pBtn_set_fitHeader_clicked()
{
    QString keyname = ui->keyName_fitHeader->currentText();
    QString value = ui->value_fitHeader->toPlainText();
    QString dataType;

    if(ui->rBtn_integer_fitHeader->isChecked())
        dataType = "TINT";
    else if(ui->rBtn_double_fitHeader->isChecked())
        dataType = "TDOUBLE";
    else if(ui->rBtn_logical_fitHeader->isChecked())
        dataType = "TLOGICAL";
    else
        dataType = "TSTRING";

    //if set Button text is 'set', update current row data; then if 'add', add a new row
    if(ix.fitHeadEditState == FitHeader_Set)//ui->pBtn_set_fitHeader->text() == "Set")
    {
        int rowNum = ui->tableWidget_fitHeader->currentRow();
        ui->tableWidget_fitHeader->setItem(rowNum,0,new QTableWidgetItem(value));
    }
    else if(ix.fitHeadEditState == FitHeader_Add)//ui->pBtn_set_fitHeader->text() == "Add")
    {
        int rowCount = ui->tableWidget_fitHeader->rowCount();
        //insert a new row
        ui->tableWidget_fitHeader->insertRow(rowCount);
        //设置新加行的行高
        ui->tableWidget_fitHeader->setRowHeight(rowCount,22);
        //set item
        ui->tableWidget_fitHeader->setItem(rowCount,0,new QTableWidgetItem(value));
        ui->tableWidget_fitHeader->setItem(rowCount,1,new QTableWidgetItem(dataType));
        ui->tableWidget_fitHeader->setVerticalHeaderItem(rowCount, new QTableWidgetItem(keyname));

        //新增行之后，keyname comboBox也要相应增加item
        ui->keyName_fitHeader->addItem(keyname);
        //设置新增行为当前选择行
        ui->tableWidget_fitHeader->selectRow(rowCount);
    }

    //Add or Set done, then reset edit status to FitHeader_Set
    ui->rBtn_integer_fitHeader->setEnabled(false);
    ui->rBtn_double_fitHeader->setEnabled(false);
    ui->rBtn_logical_fitHeader->setEnabled(false);
    ui->rBtn_string_fitHeader->setEnabled(false);
    ui->pBtn_delete_fitHeader->setEnabled(true);
    ui->pBtn_set_fitHeader->setEnabled(true);
    ui->pBtn_set_fitHeader->setText(tr("Set"));
    ix.fitHeadEditState = FitHeader_Set;//set state
}

/**
 * @brief FitHeader::on_pBtn_delete_fitHeader_clicked
 */
void FitHeader::on_pBtn_delete_fitHeader_clicked()
{
    int curRow = ui->tableWidget_fitHeader->currentRow();
    ui->tableWidget_fitHeader->removeRow(curRow);

    //删除行后，keyname comboBox也要对应删除
    ui->keyName_fitHeader->removeItem(curRow);
}

/**
 * @brief FitHeader::eventFilter  事件过滤机制
 * @param o
 * @param e
 * @return
 */
bool FitHeader::eventFilter(QObject* o, QEvent* e)
{
    if (e->type() == QEvent::Paint)
    {
        QAbstractButton* btn = qobject_cast<QAbstractButton*>(o);
        if (btn)
        {
            // paint by hand (borrowed from QTableCornerButton)
            QStyleOptionHeader opt;
            opt.init(btn);
            QStyle::State state = QStyle::State_None;
            if (btn->isEnabled())
                state |= QStyle::State_Enabled;
            if (btn->isActiveWindow())
                state |= QStyle::State_Active;
            if (btn->isDown())
                state |= QStyle::State_Sunken;
            opt.state = state;
            opt.textAlignment = Qt::AlignCenter;//设置cornerbutton text位置居中
            opt.rect = btn->rect();
            opt.text = btn->text(); // this line is the only difference to QTableCornerButton

            opt.position = QStyleOptionHeader::OnlyOneSection;
            QStylePainter painter(btn);
            painter.drawControl(QStyle::CE_Header, opt);
            return true; // eat event

        }
    }

    if(o == ui->keyName_fitHeader)
    {
        //处理keyname_fitHeader控件 失去焦点事件
        if(e->type() == QEvent::FocusOut)
        {
            bool isNewKey = false;
            for(int i=0; i<ui->tableWidget_fitHeader->rowCount(); i++)
            {
                if(ui->keyName_fitHeader->currentText() == ui->tableWidget_fitHeader->verticalHeaderItem(i)->data(Qt::DisplayRole).toString())
                {
                    isNewKey = false;
                    break;
                }
                else
                {
                    isNewKey = true;
                    continue;
                }
            }

            if(isNewKey)
            {   //new key
                ui->value_fitHeader->setText("");
                ui->rBtn_integer_fitHeader->setEnabled(true);
                ui->rBtn_double_fitHeader->setEnabled(true);
                ui->rBtn_logical_fitHeader->setEnabled(true);
                ui->rBtn_string_fitHeader->setEnabled(true);
                ui->rBtn_string_fitHeader->setChecked(true);
                ui->pBtn_delete_fitHeader->setEnabled(false);
                ui->pBtn_set_fitHeader->setEnabled(true);
                ui->pBtn_set_fitHeader->setText(tr("Add"));
                ix.fitHeadEditState = FitHeader_Add;//add state
            }
        }
    }

    return false;
}

/**
 * @brief FitHeader::saveAsCSV   保存fitHeader table为csv文件
 * @param name
 */
void FitHeader::saveAsCSV(QString name)
{
    QFile file(name);    

    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream data( &file );
        QStringList strList;
        QString iKey;
        QString iText;

        //point the 0row 0column is " " in csv file
        strList << " ";
        // add horizontal header to csv file
        for( int c = 0; c < ui->tableWidget_fitHeader->columnCount(); ++c)
        {
            strList << ui->tableWidget_fitHeader->horizontalHeaderItem(c)->data(Qt::DisplayRole).toString();
        }
        data << strList.join(",") << "\n";

        for( int r = 0; r < ui->tableWidget_fitHeader->rowCount(); ++r)
        {
            strList.clear();
            //add the vertical header to csv file
            iKey = ui->tableWidget_fitHeader->verticalHeaderItem(r)->data(Qt::DisplayRole).toString();
            strList << iKey;
            //add the table item to csv file
            for( int c = 0; c < ui->tableWidget_fitHeader->columnCount(); ++c )
            {
                if(c == 0)
                {
                    if(iKey.compare("SIMPLE") == 0 || iKey.compare("NAXIS") == 0 || iKey.compare("BSCALE") == 0 ||
                       iKey.compare("BZERO") == 0 || iKey.compare("OBJECT") == 0 || iKey.compare("TELESCOP") == 0 ||
                       iKey.compare("OBSERVER") == 0 || iKey.compare("SWCREATE") == 0 || iKey.compare("INSTRUME") == 0)
                    {
                        if(ui->tableWidget_fitHeader->item(r,c) == 0)
                            iText = "";
                        else
                            iText = ui->tableWidget_fitHeader->item(r,c)->text();
                    }
                    else
                    {
                        iText = "";
                    }
                }
                else
                {
                    if(ui->tableWidget_fitHeader->item(r,c) == 0)
                    {
                        if(c == 1)
                            iText = "TSTRING";
                        else
                            iText = "";
                    }
                    else
                    {
                        iText = ui->tableWidget_fitHeader->item(r,c)->text();
                    }
                }

                strList << iText;
            }
            data << strList.join( "," )+"\n";
        }
        file.close();
    }

    qDebug() << "CSV file saved";
}

/**
 * 加载csv文件
 * @brief FitHeader::loadCSV
 * @param fileName
 */
void FitHeader::loadCSV(QString fileName)
{
    QString data;
    QStringList rowOfData;
    QStringList rowData;

    QFile fileCSV(fileName);
    if(fileCSV.exists())
    {
        data.clear();
        rowOfData.clear();
        rowData.clear();

        if (fileCSV.open(QFile::ReadOnly))
        {
            data = fileCSV.readAll();
            //Value on each row
            rowOfData = data.split("\n");
            fileCSV.close();
        }

        //由于保存csv文件时+“\n“标记，故data.split("\n")会多一行空数据,number of row为rowOfData.size()-1.
        //x=0是表horizontalHeader数据，循环开始x=1.
        for (int x = 1; x < rowOfData.size()-1; x++)
        {
            //data of column
            rowData = rowOfData.at(x).split(",");

            //qDebug() << rowData.at(0) << rowData.at(1) << rowData.at(2) << rowData.at(3);
            keyWords.append(rowData.at(0));
            values.append(rowData.at(1));
            dataTypes.append(rowData.at(2));
            descriptions.append(rowData.at(3));

        }
        qDebug() << "CSV file loaded";
    }
    else
    {
        for(int i=0; i<ui->tableWidget_fitHeader->rowCount(); i++)
        {
            keyWords.append(ui->tableWidget_fitHeader->verticalHeaderItem(i)->text());
            if(ui->tableWidget_fitHeader->item(i, 0) == 0)
                values.append("");
            else
                values.append(ui->tableWidget_fitHeader->item(i, 0)->text());
            if(ui->tableWidget_fitHeader->item(i, 1) == 0)
                dataTypes.append("TSTRING");
            else
                dataTypes.append(ui->tableWidget_fitHeader->item(i, 1)->text());
            if(ui->tableWidget_fitHeader->item(i, 2) == 0)
                descriptions.append("");
            else
                descriptions.append(ui->tableWidget_fitHeader->item(i, 2)->text());

        }
        qDebug() << "No CSV file found!";
    }

    this->initFitHeader();
    //ui->gBox_key_fitHeader->setEnabled(true);
}

void FitHeader::initFitHeader()
{
    //clear all rows...
    int rowN = ui->tableWidget_fitHeader->rowCount();
    while(rowN >= 0)
    {
        ui->tableWidget_fitHeader->removeRow(rowN);
        rowN--;
    }

    //add new rows
    ui->tableWidget_fitHeader->setRowCount(keyWords.size());
    for(int i=0; i<ui->tableWidget_fitHeader->rowCount(); i++)
    {
        ui->tableWidget_fitHeader->setRowHeight(i,22);
        //set item
        ui->tableWidget_fitHeader->setVerticalHeaderItem(i, new QTableWidgetItem(keyWords.at(i)));
        ui->tableWidget_fitHeader->setItem(i, 0, new QTableWidgetItem(values.at(i)));
        ui->tableWidget_fitHeader->setItem(i, 1, new QTableWidgetItem(dataTypes.at(i)));
        ui->tableWidget_fitHeader->setItem(i, 2, new QTableWidgetItem(descriptions.at(i)));
    }

    //加载行表头数据到keyname comboBox中
    ui->keyName_fitHeader->addItems(keyWords);
}

void FitHeader::FITRead(QString nameStr,long length,unsigned char *Buf)
{
    fitsfile *fptr;
    int status;
    std::string str = nameStr.toStdString();
    const char* ch = str.c_str();
    long firstelem = 1;
    long nelem = length;

    fits_open_image(&fptr,ch,READONLY,&status);

    fits_read_img(fptr,TUSHORT,firstelem,nelem,NULL,Buf,NULL,&status);

    fits_close_file(fptr,&status);
}


/**
 * @brief FitHeader::FITWrite  根据设置的FitHeader写FIT文件
 * @param nameStr  文件名（包含后缀）
 * @param Buf
 */
void FitHeader::FITWrite(QString nameStr, unsigned char *Buf)
{
    //文件保存
    //流程为：首先将检测是否有重名文件，如果有则自动更名。然后根据用户是否设置了IgnoreOverScanArea选项
    //获取对应的SUBFRAME，然后保存
    //保存文件尺寸的所有参数由全局变量IX.imageWidth, ix.imageHeight, ix.only.... 确定

    //判断文件是否存在
    while(QFile::exists(nameStr))
    {
        int a = nameStr.indexOf(".");
        int b = nameStr.length();

        for(int i=0; i<b; i++)
        {	//获取"."字符最后出现的位置
            QString Npos =  nameStr.mid(i,1);//第i个字符
            QString sample = ".";
            if(Npos.compare(sample) == 0)
            {
                a = i;
            }
        }

        nameStr = nameStr.left(a) + "-2" + nameStr.right(b-a);
        mainWidget->statusLabel_msg->setText(tr("Warn:File exist.Automatic Rename ") + nameStr);
    }

    //-----remove overscan area----
    int onlyStartX,onlyStartY,onlySizeX,onlySizeY;
    IplImage *FitImg,*OnlyImg;

    FitImg = cvCreateImage(cvSize(ix.imageX ,ix.imageY ), IPL_DEPTH_16U, 1 );
    //FitImg->imageData = (char*)Buf;
    memcpy(FitImg->imageData, Buf, FitImg->imageSize);

    if(ix.ignoreOverScan)//mainMenuBar->actIgnoreOverScanArea->isChecked())
    {
        onlyStartX = ix.onlyStartX;
        onlyStartY = ix.onlyStartY;
        onlySizeX = ix.onlySizeX;
        onlySizeY = ix.onlySizeY;
    }
    else
    {
        onlyStartX = 0;
        onlyStartY = 0;
        onlySizeX = ix.imageX ;
        onlySizeY = ix.imageY ;
    }
    OnlyImg = cvCreateImage(cvSize(onlySizeX,onlySizeY), IPL_DEPTH_16U, 1 );

    cvSetImageROI(FitImg, cvRect(onlyStartX,onlyStartY,onlySizeX,onlySizeY));
    cvCopy(FitImg, OnlyImg, NULL);
    cvResetImageROI(FitImg);
    qDebug() << ix.ignoreOverScan <<onlyStartX << onlyStartY << onlySizeX << onlySizeY << ix.imageX << ix.imageY;
    //----end----

    fitsfile *fptr;
    int status = 0;//, ii, jj;
    long  fpixel = 1, naxis = 2, nelements;//, exposure;
    long naxes[2] = {onlySizeX, onlySizeY };
    char ss[81];
    char keywords[100];
    char description[100];

    //string holds the text converted to Unicode, 支持中文路径
    QTextCodec *codec = QTextCodec::codecForName("GB18030");
    QByteArray encodedString = codec->fromUnicode(nameStr);
    fits_create_file(&fptr, encodedString.constData(), &status);

    fits_create_img(fptr, USHORT_IMG, naxis, naxes, &status);

    //QString->QByteArray->char*  不可以str.toLatin1().data()这样一步完成，可能会出错
    //write fit header info
    QByteArray ba_key;
    QByteArray ba_value;
    QByteArray ba_desc;
    QString iKey = "";
    QString iValue = "";
    QString iDateType = "";
    QString iDesc = "";
    int keyCount = ui->tableWidget_fitHeader->verticalHeader()->count();
    for(int rownumber=0; rownumber<keyCount; rownumber++)
    {   //keywords is not ""
        iKey = ui->tableWidget_fitHeader->verticalHeaderItem(rownumber)->text();
        if(iKey != "")
        {   //value
            if(ui->tableWidget_fitHeader->item(rownumber,0) == 0)
                iValue = "";
            else
                iValue = ui->tableWidget_fitHeader->item(rownumber,0)->text();
            //datetype
            if(ui->tableWidget_fitHeader->item(rownumber,1) == 0)
                iDateType = "TSTRING";
            else
                iDateType = ui->tableWidget_fitHeader->item(rownumber,1)->text();
            //description
            if(ui->tableWidget_fitHeader->item(rownumber,2) == 0)
                iDesc = "";
            else
                iDesc = ui->tableWidget_fitHeader->item(rownumber,2)->text();

            ba_key = iKey.toLatin1();
            strcpy(keywords, ba_key.data());
            ba_value = iValue.toLatin1();
            strcpy(ss, ba_value.data());
            ba_desc = iDesc.toLatin1();
            strcpy(description, ba_desc.data());

            if(iDateType.compare("TSTRING") == 0)
            {
                fits_update_key(fptr, TSTRING, keywords, &ss, description, &status);
            }
            else if(iDateType.compare("TINT") == 0)
            {
                if(iKey.compare("NAXIS") == 0 || iKey.compare("NAXIS1") == 0 || iKey.compare("NAXIS2") == 0)
                {
                }
                else
                {
                    bool ok;
                    int v = iValue.toInt(&ok, 10);
                    if(ok)
                        fits_update_key(fptr, TINT, keywords, &v, description, &status);
                }
            }
            else if(iDateType.compare("TDOUBLE") == 0)
            {
                if(iKey.compare("BSCALE") == 0 || iKey.compare("BZERO") == 0)
                {
                    //BSCALE,BZERO这两个关键字暂不明白用法，似乎fit文件中不写入，在使用maximdl打开fit时也存在这两个键值
                    //而且采用下面方法写入BZERO键值，FITSIO会写失败，FITSIO status = 412  numerical overflow during implicit datatype conversion
                }
                else
                {
                    bool ok;
                    double v = iValue.toDouble(&ok);
                    if(ok)
                        fits_update_key(fptr, TDOUBLE, keywords, &v, description, &status);
                }
            }
            else if(iDateType.compare("TLOGICAL") == 0)
            {
                fits_update_key(fptr, TLOGICAL, keywords, &ss, description, &status);
            }
        }
    }

    nelements = naxes[0] * naxes[1];

    fits_write_img(fptr, TUSHORT, fpixel, nelements, &OnlyImg->imageData[0], &status);
    fits_close_file(fptr, &status);
    fits_report_error(stderr, status);

    FitImg->imageData = 0;
    OnlyImg->imageData = 0;
    cvReleaseImage(&OnlyImg);
    cvReleaseImage(&FitImg);
}

/**
 * @brief FitHeader::FITwrite_Common   标准FIT保存函数，不考虑ignoreOverScanArea的影响
 * @param filename
 * @param ImageWidth
 * @param ImageHeight
 * @param Buf
 */
void FitHeader::FITwrite_Common(QString filename, int ImageWidth, int ImageHeight, unsigned char *Buf)
{
    //判断文件是否存在
    while(QFile::exists(filename))
    {
        int a = filename.indexOf(".");
        int b = filename.length();

        for(int i=0; i<b; i++)
        {	//获取"."字符最后出现的位置
            QString Npos =  filename.mid(i,1);//第i个字符
            QString sample = ".";
            if(Npos.compare(sample) == 0)
            {
                a = i;
            }
        }

        filename = filename.left(a) + "-2" + filename.right(b-a);
        mainWidget->statusLabel_msg->setText(tr("Warn:File exist.Automatic Rename ") + filename);
    }

    IplImage *FitImg;

    FitImg = cvCreateImage(cvSize(ImageWidth,ImageHeight), IPL_DEPTH_16U, 1);
    FitImg->imageData = (char*)Buf;

    fitsfile *fptr;
    int status;//, ii, jj;
    long fpixel = 1, naxis = 2, nelements;//, exposure;
    long naxes[2] = {ImageWidth, ImageHeight};
    char ss[81];
    char keywords[40];
    char description[40];

    status = 0;

    //string holds the text converted to Unicode, 支持中文路径
    QTextCodec *codec = QTextCodec::codecForName("GB18030");
    QByteArray encodedString = codec->fromUnicode(filename);
    fits_create_file(&fptr, encodedString.constData(), &status);

    fits_create_img(fptr, USHORT_IMG, naxis, naxes, &status);

    //write fit header info
    QByteArray ba_key;
    QByteArray ba_value;
    QByteArray ba_desc;
    QString iKey = "";
    QString iValue = "";
    QString iDateType = "";
    QString iDesc = "";
    int keyCount = ui->tableWidget_fitHeader->verticalHeader()->count();
    for(int rownumber=0; rownumber<keyCount; rownumber++)
    {   //keywords is not ""
        iKey = ui->tableWidget_fitHeader->verticalHeaderItem(rownumber)->text();
        if(iKey != "")
        {   //value
            if(ui->tableWidget_fitHeader->item(rownumber,0) == 0)
                iValue = "";
            else
                iValue = ui->tableWidget_fitHeader->item(rownumber,0)->text();
            //datetype
            if(ui->tableWidget_fitHeader->item(rownumber,1) == 0)
                iDateType = "TSTRING";
            else
                iDateType = ui->tableWidget_fitHeader->item(rownumber,1)->text();
            //description
            if(ui->tableWidget_fitHeader->item(rownumber,2) == 0)
                iDesc = "";
            else
                iDesc = ui->tableWidget_fitHeader->item(rownumber,2)->text();

            ba_key = iKey.toLatin1();
            strcpy(keywords, ba_key.data());
            ba_value = iValue.toLatin1();
            strcpy(ss, ba_value.data());
            ba_desc = iDesc.toLatin1();
            strcpy(description, ba_desc.data());

            if(iDateType.compare("TSTRING") == 0)
            {
                fits_update_key(fptr, TSTRING, keywords, &ss, description, &status);
            }
            else if(iDateType.compare("TINT") == 0)
            {
                if(iKey.compare("NAXIS") == 0 || iKey.compare("NAXIS1") == 0 || iKey.compare("NAXIS2") == 0)
                {
                }
                else
                {
                    bool ok;
                    int v = iValue.toInt(&ok, 10);
                    if(ok)
                        fits_update_key(fptr, TINT, keywords, &v, description, &status);
                }
            }
            else if(iDateType.compare("TDOUBLE") == 0)
            {
                if(iKey.compare("BSCALE") == 0 || iKey.compare("BZERO") == 0)
                {
                    //BSCALE,BZERO这两个关键字暂不明白用法，似乎fit文件中不写入，在使用maximdl打开fit时也存在这两个键值
                    //而且采用下面方法写入BZERO键值，FITSIO会写失败，FITSIO status = 412  numerical overflow during implicit datatype conversion
                }
                else
                {
                    bool ok;
                    double v = iValue.toDouble(&ok);
                    if(ok)
                        fits_update_key(fptr, TDOUBLE, keywords, &v, description, &status);
                }
            }
            else if(iDateType.compare("TLOGICAL") == 0)
            {
                fits_update_key(fptr, TLOGICAL, keywords, &ss, description, &status);
            }
        }
    }

    nelements = naxes[0] * naxes[1];

    fits_write_img(fptr, TUSHORT, fpixel, nelements, &FitImg->imageData[0], &status);
    fits_close_file(fptr, &status);
    fits_report_error(stderr, status);

    FitImg->imageData=0;
    cvReleaseImage(&FitImg);

}

void FitHeader::resetUI()
{
    ui->retranslateUi(this);//

    //设置tableWidget左上角CornerButton text
    //Qt中API没有提供直接的访问方式,可以通过findChild()来定位到该按钮,然后设置其文本及显示宽度.
    QAbstractButton *btn = ui->tableWidget_fitHeader->findChild<QAbstractButton*>();
    if (btn)
    {
        btn->setText(tr("Keyword"));
        btn->setStyleSheet("color:white;font:12px Tahoma;");//设置text颜色，字体，大小（位置设置似乎无效）

        btn->installEventFilter(this);//添加事件过滤注册，用于实现设置cornerButton的text,color,positon等

        // adjust the width of the vertical header to match the preferred corner button width
        // (unfortunately QAbstractButton doesn't implement any size hinting functionality)
        QStyleOptionHeader opt;
        opt.text = btn->text();
        QSize s = (btn->style()->sizeFromContents(QStyle::CT_HeaderSection, &opt, QSize(), btn).expandedTo(QApplication::globalStrut()));
        if (s.isValid())
           ui->tableWidget_fitHeader->verticalHeader()->setMinimumWidth(s.width());
    }
}

void FitHeader::camera_connected()
{
    QString iKey = "";
    int rowN = ui->tableWidget_fitHeader->rowCount();
    for(int i=0; i<rowN; i++)
    {
        iKey = ui->tableWidget_fitHeader->verticalHeaderItem(i)->text();
        if(iKey == "XPIXSZ")
        {
            ui->tableWidget_fitHeader->setItem(i, 0, new QTableWidgetItem(QString::number(ix.ccdPixelW)));
        }
        else if(iKey == "YPIXSZ")
        {
            ui->tableWidget_fitHeader->setItem(i, 0, new QTableWidgetItem(QString::number(ix.ccdPixelH)));
        }
        else if(iKey == "CAMERA")
        {
            ui->tableWidget_fitHeader->setItem(i, 0, new QTableWidgetItem(ix.CamModel));
        }
    }
}

void FitHeader::fitHeaderInfo_changed()
{
    QString iKey = "";
    int rowN = ui->tableWidget_fitHeader->rowCount();
    for(int i=0; i<rowN; i++)
    {
        iKey = ui->tableWidget_fitHeader->verticalHeaderItem(i)->text();

        if(iKey.compare("BITPIX") == 0)
        {
            ui->tableWidget_fitHeader->setItem(i, 0, new QTableWidgetItem(QString::number(ix.bits)));
        }
        else if(iKey.compare("NAXIS1") == 0)
        {
            ui->tableWidget_fitHeader->setItem(i, 0, new QTableWidgetItem(QString::number(ix.imageX)));
        }
        else if(iKey.compare("NAXIS2") == 0)
        {
            ui->tableWidget_fitHeader->setItem(i, 0, new QTableWidgetItem(QString::number(ix.imageY)));
        }
        else if(iKey.compare("DATE-OBS") == 0)
        {
            ui->tableWidget_fitHeader->setItem(i, 0, new QTableWidgetItem(ix.dateOBS));
        }
        else if(iKey.compare("EXPTIME") == 0)
        {
            ui->tableWidget_fitHeader->setItem(i, 0, new QTableWidgetItem(QString::number(ix.Exptime/1000.0)));  //unit: s
        }
        else if(iKey.compare("EXPOSURE") == 0)
        {
            ui->tableWidget_fitHeader->setItem(i, 0, new QTableWidgetItem(QString::number(ix.Exptime/1000.0))); //unit: s
        }
        else if(iKey.compare("GAIN") == 0)
        {
            if(ix.canGain)
                ui->tableWidget_fitHeader->setItem(i, 0, new QTableWidgetItem(QString::number(ix.gain)));
        }
        else if(iKey.compare("OFFSET") == 0)
        {
            if(ix.canOffset)
                ui->tableWidget_fitHeader->setItem(i, 0, new QTableWidgetItem(QString::number(ix.offset)));
        }
        else if(iKey.compare("SET-TEMP") == 0)
        {
            if(ix.canCooler)
                ui->tableWidget_fitHeader->setItem(i, 0, new QTableWidgetItem(QString::number(ix.targetTemp, 'f', 1)));
        }
        else if(iKey.compare("CCD-TEMP") == 0)
        {
            if(ix.canCooler)
                ui->tableWidget_fitHeader->setItem(i, 0, new QTableWidgetItem(QString::number(ix.nowTemp, 'f', 1)));
        }
        else if(iKey.compare("XBINNING") == 0)
        {
            ui->tableWidget_fitHeader->setItem(i, 0, new QTableWidgetItem(QString::number(ix.binx)));
        }
        else if(iKey.compare("YBINNING") == 0)
        {
            ui->tableWidget_fitHeader->setItem(i, 0, new QTableWidgetItem(QString::number(ix.biny)));
        }
    }
}

