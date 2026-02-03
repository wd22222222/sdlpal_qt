#pragma once
#include "..\main\command.h"
//#include <variant>
#include <qtableview.h>
//#include <qmenu.h>
//#include <qheaderview.h>
//#include <qscrollbar.h>
#include <qevent.h>
#include "cviewmodel.h"
//#include "cpopview.h"
#include "ccellrange.h"
#include "t_data.h"

//using namespace std;

class QTableView;



class CTableView :
    public QTableView
{
	Q_OBJECT

private:
    T_DATA t_Data;
	//数据源 
	CViewModel * m_Model{};
	class MainWindow* m_parent{};
	BOOL bisVirtualTable{};
	QTableView* s1{};
	std::vector<UnDoData> mUndoArray{};
	void init();
	struct tagTableSize
	{
		int btColCount{};//表头栏数
		int btw{};//表头宽度
		int bw{};//表宽度
	} TableSize;
public:
	//保存修改数据，用于数组
	int UndoCtrl{ 0 };
	//弹出菜单控制数据 从尾部计算 4 位跳转相关 5 空 6-8 插入追加删除 11 部属，脚本，调用 
	DWORD  m_popMenuFlags{};
	//class CPopTable * m_popView{};
	int indexScene{ -1 };//场景号位置 用于在地图上布置场景
public:
    CTableView(QWidget* parent = (QWidget*)nullptr);
	~CTableView();
	void resize(const int w, const int h);
	void clear();;
	void getEditCell(int row, int col);
	//选择指定栏，并滚动到
	void setSelectItem(const int row = -1,const int col = -1,const int flage = 0);
	CCellRange getDisplayCellRange();
	void set_t_Data(T_DATA &t);;
    T_DATA * getDataArray() { return &t_Data; };//取表数据数组指针
signals:
	void SendMessages(const QString& msg);//发送信号到 m_Edit
	void ListScriptEntry(int script);
	void ListScriptCall(int script);
	void deployMap(int scene);

	//弹出菜单设置
protected:
	QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
	void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible) override;
	void keyPressEvent(QKeyEvent* event)override;
	void ClickedRightSlot(QPoint pos);

	void menuSelected(QAction* act);

public:
	//
	void updateSectionWidth(int logicalIndex, int oldSize, int newSize);
	void updateSectionHeight(int logicalIndex, int oldSize, int newSize);
	//栏高度
	void updateSectionHeightS1(int logicalIndex, int oldSize, int newSize);
	//
	void updateSectionWidthS1(int logicalIndex, int oldSize, int newSize);

	void updateFrozenTableGeometry();

	CViewModel* getModol() const{return m_Model; }
	void undo();//恢复，
	void redo();//重做，将恢复的数据还原
	int isUpdate() { return t_Data.UndoCount ; };
	void insertRow(int row, int flage = 0);
	void removeRow(int row);
};

