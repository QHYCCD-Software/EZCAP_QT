#include <QtGui>
#include <QSpinBox>
#include <QLineEdit>

#include "delegate.h"

//*******************************************************************************************
//BinDelegate委托类，用于QTableWidgetItem中BIN列，利用QSpinBox委托进行输入限制，只能输入1－4之间的数字
//*******************************************************************************************
BinDelegate::BinDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

QWidget *BinDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/,
                                    const QModelIndex &/*index*/) const
{
     QSpinBox *editor = new QSpinBox(parent);
     editor->setMinimum(1);
     editor->setMaximum(4);
     return editor;
}

void BinDelegate::setEditorData(QWidget *editor,
                                  const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();

    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(value);
}

void BinDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void BinDelegate::updateEditorGeometry(QWidget *editor,
  const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

//*******************************************************************************************
//RepeatDelegate委托类，用于QTableWidgetItem中BIN列，利用QSpinBox委托进行输入限制，只能输入1－999之间的数字
//*******************************************************************************************
RepeatDelegate::RepeatDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

QWidget *RepeatDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/,
                                    const QModelIndex &/*index*/) const
{
     QSpinBox *editor = new QSpinBox(parent);
     editor->setMinimum(1);
     editor->setMaximum(999);
     return editor;
}

void RepeatDelegate::setEditorData(QWidget *editor,
                                  const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();

    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(value);
}

void RepeatDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void RepeatDelegate::updateEditorGeometry(QWidget *editor,
  const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

//*******************************************************************************************
//CFWDelegate委托类，用于QTableWidgetItem中CFW列，利用QSpinBox委托进行输入限制，只能输入0－9之间的数字
//*******************************************************************************************
CFWDelegate::CFWDelegate(QObject *parent)
    : QItemDelegate(parent)
{
    maxValue = 9;
    minValue = 0;
}

void CFWDelegate::setRange(int min, int max)
{
    maxValue = max;
    minValue = min;
}

QWidget *CFWDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/,
                                    const QModelIndex &/*index*/) const
{
     QSpinBox *editor = new QSpinBox(parent);
     editor->setMinimum(minValue);
     editor->setMaximum(maxValue);
     return editor;
}

void CFWDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();

    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(value);
}

void CFWDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void CFWDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                       const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

//*******************************************************************************************
//ExpTimeDelegate委托类，用于QTableWidgetItem中EXPOSURE列和DeLay列，
//利用QLineEdit委托和正则表达式对输入进行限制，只能输入数字和小数点，且小数点只能有一个
//*******************************************************************************************
ExpTimeDelegate::ExpTimeDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

QWidget *ExpTimeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/,
                                    const QModelIndex &/*index*/) const
{
    QLineEdit *editor = new QLineEdit(parent);
    QRegExp regExp("(^[0-9]+\\.[0-9]*)|(^[0-9]*)$");
    editor->setValidator(new QRegExpValidator(regExp, parent));
    return editor;
}

void ExpTimeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString text = index.model()->data(index, Qt::EditRole).toString();
    double i = text.toDouble();
    if(i > 1)
    {
        int j = (int)i;
        text = QString::number(j);
    }
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    lineEdit->setText(text);
}

void ExpTimeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    QString text = lineEdit->text();
    double i = text.toDouble();
    if(i > 1)
    {
        int j = (int)i;
        text = QString::number(j);
    }
    model->setData(index, text, Qt::EditRole);
}

void ExpTimeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                       const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

//*******************************************************************************************
//GainDelegate委托类，用于QTableWidgetItem中Gain列，
//利用QSpinBox委托进行输入限制，只能输入0－100之间的数字
//*******************************************************************************************
GainDelegate::GainDelegate(QObject *parent)
    : QItemDelegate(parent)
{
    maxValue = 100;
    minValue = 0;
}

void GainDelegate::setRange(int min, int max)
{
    maxValue = max;
    minValue = min;
}

QWidget *GainDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/,
                                    const QModelIndex &/*index*/) const
{
     QSpinBox *editor = new QSpinBox(parent);
     editor->setMinimum(minValue);
     editor->setMaximum(maxValue);
     return editor;
}

void GainDelegate::setEditorData(QWidget *editor,
                                  const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();

    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(value);
}

void GainDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void GainDelegate::updateEditorGeometry(QWidget *editor,
  const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

//*******************************************************************************************
//SubDarkDelegate委托类，用于QTableWidgetItem中AVG列,SubDark列和SubBias列，
//利用QSpinBox委托进行输入限制，只能输入0－1之间的数字
//*******************************************************************************************
SubDarkDelegate::SubDarkDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

QWidget *SubDarkDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/,
                                    const QModelIndex &/*index*/) const
{
     QSpinBox *editor = new QSpinBox(parent);
     editor->setMinimum(0);
     editor->setMaximum(1);
     return editor;
}

void SubDarkDelegate::setEditorData(QWidget *editor,
                                  const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();

    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(value);
}

void SubDarkDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void SubDarkDelegate::updateEditorGeometry(QWidget *editor,
  const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

