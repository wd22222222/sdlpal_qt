///* -*- mode: c; tab-width: 4; c-basic-offset: 4; c-file-style: "linux" -*- */
//
// Copyright (c) 2021-2026, Wu Dong.
// 
// All rights reserved.
//
// This file is part of SDLPAL.
//
// SDLPAL is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "CustomItemDelegrate.h"
#include "cviewmodel.h"
#include "cpopview.h"
#include <qdebug.h>
#include <qheaderview.h>
#include <qscrollbar.h>
static CPopView* m_PopListView;

CustomItemDelegrate::CustomItemDelegrate(const QRegExp& regExp, QObject* parent)
    :QStyledItemDelegate(parent)
{
    m_regExp = regExp;
    m_parent = qobject_cast<CTableView*>(parent);
}

QWidget* CustomItemDelegrate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    //通知上级表正在编辑的索引，以便需要时使用
    //创建带有正则表达式的输入框
    QVariant data = index.model()->data(index, Qt::UserRole);
    auto  dm_Data = data.value<LPT_DATA>();
    int col = index.column();//, row = index.row();
    //((CTableView*)m_parent)->sendEditItemCell(row, col);
    switch (dm_Data->c_Array[col].Ctrl)
    {
    case 0:
    case 1://不可编辑
        return nullptr;
    case 2://编辑栏
    {
        QLineEdit* editor = new QLineEdit(parent);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        // Qt5 方式
        QRegExpValidator* validator = new QRegExpValidator(m_regExp, parent);
#else
        // Qt6 方式
        QRegularExpressionValidator* validator = new QRegularExpressionValidator(m_regExp, parent);
#endif
        editor->setValidator(validator);
        QString text = index.model()->data(index, Qt::EditRole).toString();
        editor->setText(text);
        return editor;
    };
    case 3://下拉框
        return nullptr;
    case 4://下拉表，
    {
        return setPopListView(index,parent);
    }
    default:
        return nullptr;
    }
}

void CustomItemDelegrate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index)const
{
    
    QStyledItemDelegate::setModelData(editor, model, index);
    if (qobject_cast<CPopView*>(editor))
        //自定义代理，使用列表
        model->setData(index, qobject_cast<CPopView*>(editor)->text(), Qt::EditRole);
    else  if (qobject_cast<QLineEdit*>(editor))
        model->setData(index, qobject_cast<QLineEdit*>(editor)->text(), Qt::EditRole);
    //向主界面发送信息
    QString s = QString("第 %1 行 第 %2 栏修改 表格已经进行了 %3 次修改 ")
       .arg(index.row()).arg(index.column()).arg(m_parent->getDataArray()->UndoCount);
    m_parent->SendMessages(s);
};

QWidget *  CustomItemDelegrate::setPopListView( const QModelIndex& index,QObject* parent)const
{
    Q_UNUSED(parent);
    QVariant data = index.model()->data(index, Qt::UserRole);
    auto  dm_Data = data.value<LPT_DATA>();
    int col = index.column();//, row = index.row();
    m_PopListView = new CPopView;
    m_PopListView->hide();

    pSelect_Item list = dm_Data->c_Array[col].p_CtrlList;

    CEditlistModel* popModel = new CEditlistModel(list, m_PopListView);

    m_PopListView->setModel(popModel);
    m_PopListView->setColumnWidth(0, 44);
    m_PopListView->setColumnWidth(1, 47);
    m_PopListView->setColumnWidth(2, 93);
    m_PopListView->verticalHeader()->setVisible(false);
    //获得值对应的行
    int setRow{};
    data = index.model()->data(index, Qt::DisplayRole);
    for (; setRow < (int)list->data.size(); setRow++)
    {
        if (data.toString() == list->data[setRow].s.c_str())
            break;
    };
    if (setRow >= (int)list->data.size())
        setRow = 0;
    m_PopListView->verticalScrollBar()->setValue(setRow);
    m_PopListView->resize(210, 300);
    m_PopListView->show();
    m_PopListView->setFocus();//设置焦点
    m_PopListView->selectRow(setRow);
    m_PopListView->show();
    //点击后启动
    connect(m_PopListView, &CPopView::clicked, this, [&](const QModelIndex& index)->void {
        QVariant val = m_PopListView->model()->data(m_PopListView->model()->index(index.row(), 1), Qt::DisplayRole);
        m_PopListView->Text = QString("%1").arg(val.toString().toInt(0, 16));//将返回的16进制数转换为10 进制
        m_PopListView->hide();
        });
    return m_PopListView;
}

CustomItemDelegrate::~CustomItemDelegrate() {
    //该表已经由系统自动delete
    m_PopListView = 0;
}

bool CustomItemDelegrate::eventFilter(QObject* editor, QEvent* event)
{
    if (event->type() == QEvent::FocusOut) {
        QFocusEvent* focusEvent = static_cast<QFocusEvent*>(event);

        // 获取当前编辑的索引
        QWidget* widget = qobject_cast<QWidget*>(editor);
        
        qDebug() << "编辑器失去焦点，原因:";

        // 提交数据
        emit commitData(widget);

        // 关闭编辑器
        emit closeEditor(widget);
        return true;
    }
    return QStyledItemDelegate::eventFilter(editor, event);
}

;

void CustomItemDelegrate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QVariant data = index.model()->data(index, Qt::UserRole);
    auto  dm_Data = data.value<LPT_DATA>();
    int col = index.column();//, row = index.row();
    if (dm_Data->c_Array[col].p_CtrlList)
    {
        QRect r =  option.rect;
        auto p = QPoint(r.x(), r.y());
        auto p1 = m_parent->mapToGlobal(p);
        //auto p4 = editor->mapToParent( editor->pos());
        editor->move(p1);
        return;
    };
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

