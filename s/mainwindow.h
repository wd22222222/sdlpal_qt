#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <qtextedit.h>
#include <qmainwindow.h>
#include <qtreewidget.h>
//#include <qpushbutton.h>
//#include <QSplitter>
#include "ctableview.h"
#include "cgetpaldata.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    //进程控制
    int JobCtrl{};//文件控制，从那个文件读取 0基础 ,> 0 存档文件
    int WorkCtrl{};//进程控制，目前进行的是那个进程
    //int okExit{};//退出时按下正常退出键=1
    int MenuNumber{};//菜单号 0 根菜单， 1 基础菜单 ，2 存档菜单 
    int haveModified{};//数据修改过
    //下拉表数据结构数组,存放不同的数据 -0- ITEM数据 
    SELECT_ITEN_ARRAY Select_Item_Array;
    QString q_SrcToStr(int i);
    inline static CGetPalData* m_Pal{};
public:
    //修改设置
    void Edit_Game_Seting(int work);
	VOID Edit_EventObject(int work);
    void Edit_FontLibraryPath(int work);
    //修改玩家属性
    void Edit_PlayerAttributes(int work);
    void Edit_Dialog(int swtch);
    void Edit_Enemy(int work);
    void Edit_EnemyTeam(int swtch);
    void Edit_Magic(int swtch);
    void Edit_Map(int swtch);
    void Edit_ObjectItem(int swtch);
    void Edit_ObjectPict(int swtch);
    void Edit_Parameters(int swtch);
    void Edit_Poison(int swtch);
    void Edit_PlayerLevelUPMagic(int swtch);
    void Edit_BattleField(int swtch);
    void Edit_Scene(int swtch);
    void Edit_Store(int swtch);
    void Edit_Invenyory(int swtch);
    void Edit_ObjectName(int swtch);
    void Edit_Explain(int swtch);
    void Edit_ScriptEntry(int swtch);
    void DataUpDate(int Work);
    INT  DoChangeDir(bool change = false);
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void Set_Tree( int MenuNumber = 0);
    static CGetPalData* pal() { return m_Pal; }
protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void closeEvent(QCloseEvent* event)override;
private:

public slots:
    void SelectTreeItem(QTreeWidgetItem*);//当treeItem 被选择时接收
    void ShowMessage(const QString& msg);
    void ListScriptEntry(WORD src);
    void ListScriptCall(WORD src);
signals:
    void SendMessages(const QString& msg);//发送信号到 m_Edit

protected:
    QTextEdit * m_Edit{};
    CTableView *m_Grid{};
    QTreeWidget*m_Tree{};
    //用于显示查询列表
    QTableView* m_ListView{};
};
#endif // MAINWINDOW_H
