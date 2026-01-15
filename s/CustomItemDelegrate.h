#pragma once
#ifndef CUSTOMITEMDELEGRATE_H
#define CUSTOMITEMDELEGRATE_H
//管理代理

#include <qwidget.h>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <qdebug.h>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QRegExp>
#else
#include <QRegularExpression>
#define QRegExp QRegularExpression
#endif

#include "ctableview.h"

class CustomItemDelegrate :
    public QStyledItemDelegate
{
    Q_OBJECT
public:
    CustomItemDelegrate(const QRegExp& regExp, QObject* parent = nullptr);
    explicit CustomItemDelegrate(QObject* parent = nullptr) :
        QStyledItemDelegate(parent) {}

    QWidget * setPopListView(const QModelIndex& index, QObject* parent = nullptr)const;
    ;
    ~CustomItemDelegrate();;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const Q_DECL_OVERRIDE;;
    //自定义代理组件必须继承以下4个函数

    //将代理组件的数据，保存到数据模型中
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const Q_DECL_OVERRIDE;

    //更新代理编辑组件的大小
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const Q_DECL_OVERRIDE;
signals:
    void SendMessages(const QString& msg);//发送信号到 m_Edit

signals:
    void editorFocusLost(const QModelIndex& index, const QVariant& data);
    void editorAboutToClose(const QModelIndex& index);


protected:
    bool eventFilter(QObject* editor, QEvent* event) override;
    
private:
    QRegExp m_regExp{};
    CTableView* m_parent{};
};


#endif


