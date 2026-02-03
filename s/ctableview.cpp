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

#include <qheaderview.h>
#include <qtableview.h>
#include <qscrollbar.h>
#include <qmenu.h>
#include <qpoint.h>
#include <QInputDialog.h>
#include <qobject.h>
#include "ctableview.h"
#include "CustomItemDelegrate.h"
#include <qabstractslider.h>
CTableView::CTableView(QWidget* parent)
	:QTableView(parent)
{
	s1 = new QTableView(this);
	s1->verticalHeader()->hide();//表头表隐藏表头
	//verticalHeader()->hide();
	viewport()->stackUnder(s1);//表头表浮在最上面
	s1->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑 
	s1->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//隐藏滚动条
	s1->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//隐藏滚动条水平
	s1->setFocusPolicy(Qt::NoFocus);//无焦点
	s1->setSelectionBehavior(QAbstractItemView::SelectRows);//选中行
	//s1->setStyleSheet("QTableWidget::section {selection-font:黑体}");
	setWordWrap(true);//自动换行
	setSelectionMode(QAbstractItemView::SingleSelection);//单选
	setSelectionBehavior(QAbstractItemView::SelectItems);//选择单栏
	s1->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑 
	s1->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//隐藏滚动条
	s1->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//隐藏滚动条水平
	//horizontalScrollBar()->setDisabled(true); // 禁用滚动水平
	//setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//隐藏滚动条水平
	//s1->setSelectionMode(QAbstractItemView::NoSelection);//不可选中
	s1->setFocusPolicy(Qt::NoFocus);//无焦点
	s1->setSelectionBehavior(QAbstractItemView::SelectRows);//选中行
	//s1->setStyleSheet("QTableWidget::section {selection-font:黑体}");
	setWordWrap(true);//自动换行
	setSelectionMode(QAbstractItemView::SingleSelection);//单选
	setSelectionBehavior(QAbstractItemView::SelectItems);//选择单栏
	setContextMenuPolicy(Qt::CustomContextMenu);
	// QPalette p = frozenTableView->palette();
	//设置冻结表头选中行关联色
	QPalette p = palette();
	p.setColor(QPalette::Inactive, QPalette::Highlight, p.color(QPalette::Active, QPalette::Dark));
	s1->setPalette(p);
	//单元格高度改变
	connect(verticalHeader(), &QHeaderView::sectionResized, this, &CTableView::updateSectionHeight);//s2 to s1
	connect(s1->verticalHeader(), &QHeaderView::sectionResized, this, &CTableView::updateSectionHeightS1);//s2 to s2
	//
	connect(s1->horizontalHeader(), &QHeaderView::sectionResized, this, &CTableView::updateSectionWidthS1);
	connect(horizontalHeader(), &QHeaderView::sectionResized, this, &CTableView::updateSectionWidth);
	//同步表垂直
	connect(verticalScrollBar(), &QAbstractSlider::valueChanged, s1->verticalScrollBar(), &QAbstractSlider::setValue);//同步s1
	connect(s1->verticalScrollBar(), &QAbstractSlider::valueChanged, verticalScrollBar(), &QAbstractSlider::setValue);//用s1同步
	//同步表水平
	connect(horizontalScrollBar(), &QAbstractSlider::valueChanged, [&]() {s1->show(); });//同步s1
	//选择栏变动
	connect(this, &QTableView::clicked, this,[&](const QModelIndex& index)->void {
		s1->selectRow(index.row());  });
	//右键按下
	connect(this, &QTableView::customContextMenuRequested, this, &CTableView::ClickedRightSlot);
}

CTableView::~CTableView()
{
	if(s1 )
		delete s1;
}

void CTableView::init()
{
	if (m_Model)
		delete m_Model;
	m_Model = new CViewModel;
	m_Model->set_t_Data( &t_Data);
	//设置表头 col 表头栏数   p_ColArray 表头结构，IsLineTop 是否使用行号作为行头
	s1->setModel(nullptr);
	setModel(m_Model);
	s1->setModel(m_Model);
	memset(&TableSize, 0, sizeof(TableSize));
	//设置水平表头
	//表头最小高度
	s1->horizontalHeader()->setMinimumHeight(40);
	horizontalHeader()->setMinimumHeight(40);

	//网格
	s1->setShowGrid(true);
	setShowGrid(true);
	//表头样式
	horizontalHeader()->setStyleSheet("QHeaderView::section{background-color:0xefefef}");
	for (int i = 0; i < t_Data.c_Array.size(); i++)
	{
		if (t_Data.c_Array[i].Ctrl == 1)
		{
			TableSize.btw += t_Data.c_Array[i].ColWidth;
			TableSize.btColCount++;
		}
		else
			s1->setColumnHidden(i, TRUE);
		TableSize.bw += t_Data.c_Array[i].ColWidth;//表宽
		s1->setColumnWidth(i, t_Data.c_Array[i].ColWidth);
		//添加表头数据,表头宽度
		setColumnWidth(i, t_Data.c_Array[i].ColWidth);

		//较验规则
		QString s;
		switch (t_Data.c_Array[i].DataStat)
		{
		case tWORD://整数
		case tDWORD:
			s = "\\d*";
			break;
		case tINT://数字第一个字符可为负号
		case tSHORT:
			s = "[-]?[0-9]\\d*";
			break;
		case tHEX://16进制数
			s = "[0-9a-fA-F]{1,4}";
			break;
		case tBOOL://布尔数
			s = "[YyNn01]{1}";
			break;
		case tFLOAT://浮点
			s = "^[-+]?([0-9]*\\.[0-9]+|[0-9]+(\\.[0-9]*)?)";
			break;
		case tSTR://字符串
		default:
			break;
		}
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		QRegExp reg(s);
#else
		QRegularExpression reg(s);
#endif
		setItemDelegateForColumn(i, new CustomItemDelegrate(reg,this));
	}
	show();
	s1->show();
	viewport()->update();
	setSelectItem(0);
	setFocus();
	return;
}


//填充一行数据
//void SetRowData(int row);

void CTableView::resize(const int w, const int h)
{
	//s1->resize(w, h);
	QWidget::resize(w, h);
	if (!s1)return;
	updateFrozenTableGeometry();
	if (TableSize.btColCount == 0)
		s1->hide();
	else
		s1->show();
}

void CTableView::clear()
{
	setModel(nullptr); s1->setModel(nullptr);
	t_Data.UndoCount = 0;
	t_Data.u_Array.clear();
}

void CTableView::getEditCell(int row, int col) 
{
	return;
}

void CTableView::setSelectItem(const int row, const int col, const int flage)
{
	//输入：行，列，滚动方式 = 0 绝对地址 ，否则相对地址
	QItemSelectionModel* model_selection = selectionModel();
	QModelIndexList IndexList = model_selection->selectedIndexes();
	int prvCol{ -1 }, prvRow{ -1 };
	//取上次的选择
	if (IndexList.isEmpty())
		prvCol = TableSize.btColCount, prvRow = 0;
	else
	{
		prvRow = IndexList[0].row();
		prvCol = IndexList[0].column();
	}
	//定位
	int newCol{}, newRow{};
	if (!flage)
	{
		newCol = col == -1 ? prvCol : col;
		newRow = row == -1 ? prvRow : row;
	}
	else
	{
		newRow = prvRow + row;
		newCol = prvCol + col;
	}
	newRow = newRow < 0 ? 0 : (newRow >= model()->rowCount() ? model()->rowCount() - 1 : newRow);
	newCol = newCol < TableSize.btColCount ? TableSize.btColCount :
		(newCol >= model()->columnCount() ? model()->columnCount() - 1 : newCol);
	//检测是否在显示范围内部
	CCellRange r = getDisplayCellRange();
	if (!r.isInRows(newRow))
		verticalScrollBar()->setValue(newRow);
	if (!r.isInCols(newCol))
		horizontalScrollBar()->setValue(newCol);
	viewport()->update();
	r = getDisplayCellRange();
	//使item 始终在可显示范围内
	updateFrozenTableGeometry();
	for (int i = horizontalScrollBar()->value();
		(columnViewportPosition(newCol) < TableSize.btw && i >= 0); i--)
		horizontalScrollBar()->setValue(horizontalScrollBar()->value() - 1);
	for (int i = horizontalScrollBar()->value();
		columnViewportPosition(newCol) + columnWidth(newCol) > viewport()->width() && i <= r.GetMaxCol(); i++)
		horizontalScrollBar()->setValue(i + 1);
	//
	s1->clearSelection();
	s1->selectRow(newRow);
	clearSelection();
	setCurrentIndex(model()->index(newRow, newCol));
	setFocus();
}

//取表格的可视范围
CCellRange CTableView::getDisplayCellRange()
{
	int ax = columnAt(TableSize.btw + 10);
	int ay = rowAt(10);
	int dx = -1; for (int i = viewport()->width(); (dx = columnAt(i)) == -1 && i > 0; i--);
	int dy = -1; for (int i = viewport()->height(); i > 0; i--)
		if ((dy = rowAt(i)) > -1)
			break;
	if (verticalScrollBar()->isActiveWindow())
		dy -= 1;
	return CCellRange(ay, ax, dy, dx);
}

void CTableView::keyPressEvent(QKeyEvent* event)
{
	if (event->matches(QKeySequence::Undo))
		undo();//撤消
	if (event->matches(QKeySequence::Redo))
		redo();//重做
	QItemSelectionModel* model_selection = selectionModel();
	QModelIndexList IndexList = model_selection->selectedIndexes();
	auto at = indexAt(QPoint( TableSize.btw + 10 ,20));
	auto at1 = indexAt(QPoint(size().width() - 10, size().height() - 10));
	if (event->matches(QKeySequence::MoveToEndOfDocument))
	{
		//跳转到表尾
		setSelectItem(m_Model->rowCount() - 1);
		return;
	}
	if (event->matches(QKeySequence::MoveToStartOfDocument))
	{
		//转到首行
		setSelectItem(0);
		return;
	}
	if (event->matches(QKeySequence::MoveToPreviousPage))
	{
		setSelectItem(-10, 0, 1);
		return;
	}
	if (event->matches(QKeySequence::MoveToNextPage))
	{
		setSelectItem(10, 0, 1);
		return;
	}
	if (event->matches(QKeySequence::MoveToNextLine))
	{
		setSelectItem(1, 0, 1);
		return;
	}
	if (event->matches(QKeySequence::MoveToPreviousLine))
	{
		setSelectItem(-1, 0, 1);
		return;
	}
	if (event->matches(QKeySequence::MoveToNextChar))
	{
		setSelectItem(0, 1, 1);
		return;
	}
	if (event->matches(QKeySequence::MoveToPreviousChar))
	{
		setSelectItem(0, -1, 1);
		return;
	}
	if (event->key() == Qt::Key_End)
	{
		setSelectItem(0, 1000, 1);
		return;
	}
	if (event->key() == Qt::Key_Home)
	{
		setSelectItem(0, -1000, 1);
		return;
	}

	QTableView::keyPressEvent(event);
}

void CTableView::set_t_Data(T_DATA& t) 
{
	t_Data = std::move(t);
	int col = t_Data.c_Array.size(), row = t_Data.d_Array.size();
	init();
}

QModelIndex CTableView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
	return QModelIndex();
}

void CTableView::scrollTo(const QModelIndex& index, ScrollHint hint)
{
}


static LPCSTR  menuStr[] =
{
	"转到首行",
	"跳转到……",
	"跳转到(16进制行)",
	"跳转到末行",
	"",
	"行前插入",
	"尾部追加",
	"删除",
	"运行",//8
	"使用存档运行缺省",//9
	"在地图上部署",//10
	"查看脚本",//11
	"查看调用者",//12
	"查看脚本调用",//13
	nullptr,
};


void CTableView::ClickedRightSlot(QPoint pos)
{
	auto index = indexAt(pos);
	if (!index.isValid())
	{
		return;
	}
	int pRow = index.row(), pCol = index.column();
	//m_LastCell = CCellID(pRow, pCol);
	// 弹出左键菜单，
	QMenu* pMenu = new QMenu(this);

	pMenu->setContextMenuPolicy(Qt::CustomContextMenu);
	for (int i = 0; menuStr[i]; i++)
	{
		QAction* a{};
		if (QString(menuStr[i]) == QString(""))
		{
			pMenu->addSeparator();//空行间隔
			continue;
		}
		else
		{
			a = pMenu->addAction(menuStr[i]);
		}
		if (i == 11 && t_Data.c_Array[pCol].m_isScript == 1
            && std::get<int>(t_Data.d_Array[pRow].ColVarList[pCol])
			)
		{
			//符合条件显示
			continue;//
		}
		if (i == 12 && t_Data.c_Array[pCol].m_isScript == 2 &&
            std::get<int>(t_Data.d_Array[pRow].ColVarList[pCol]))
		{
			//符合条件显示
			continue;//
		}
		if (i == 13 && t_Data.c_Array[pCol].m_isScript == 3 &&
            std::get<int>(t_Data.d_Array[pRow].ColVarList[pCol]))
		{
			//符合条件显示
			continue;//
		}
		if (!(m_popMenuFlags & (1 << i)))
		{
			a->setEnabled(FALSE);
			//筛除不符合条件的，不显示
			if (i == 2 || i > 7)
				a->setVisible(FALSE);
		}
	}
	connect(pMenu, &QMenu::triggered, this, &CTableView::menuSelected);
	// 右键菜单显示的地方
	pMenu->exec(QCursor::pos());
	delete pMenu;
}

void CTableView::menuSelected(QAction* act)
{

	QString s = act->text();
	QString msg = "执行" + s;
	emit SendMessages(msg);

	QItemSelectionModel* model_selection = selectionModel();
	QModelIndexList IndexList = model_selection->selectedIndexes();

	int n{};
	for (; menuStr[n] && s != QString(menuStr[n]); n++);
	switch (n)
	{
	case 0:
	{
		//转到首行
		setSelectItem(0);
		return;
	}
	case 1:
	{
		//"跳转到
		int  row = QInputDialog::getInt(nullptr, "跳转到", "请输入行号", QLineEdit::Normal);
		setSelectItem(row);
		return;
	}
	case 2:
	{
		//"跳转到16进制数
		int row = QInputDialog::getText(nullptr, "跳转到", "请输入16进制数", QLineEdit::Normal, "0000").toInt(NULL, 16);
		setSelectItem(row);
		return;
	}
	case 3:
	{
		//跳转到表尾
		int row = m_Model->rowCount();;
		row = m_Model->rowCount() - 1;
		setSelectItem(row);
		return;
	}
	case 5:
	{
		//插入
		insertRow(IndexList[0].row());
		break;
	}
	case 6:
	{
		//尾部插入
		insertRow(IndexList[0].row(), 1);
		break;
	}
	case 7:
	{
		//删除
		removeRow(IndexList[0].row());
		break;
	}
	case 8:
	case 9:
		break;
	case 10:
		//在地图上部署
	{
		auto row = IndexList[0].row();
        int scene = std::get<int>(t_Data.d_Array[row].ColVarList[indexScene]);
		slots deployMap(scene);
		break;
	}
	case 11:
	{
		//查看脚本 View script  _ckjb
		setFocus();
		int row = IndexList[0].row();
		int col = IndexList[0].column();
        emit ListScriptEntry(std::get<int>(t_Data.d_Array[row].ColVarList[col]));
		break;
	}
	case 12:
	{
		//执行查看调用
		setFocus();
		int row = IndexList[0].row();
		emit ListScriptCall(row);
		break;
	}
	default:
		return;;
	}
	
}

//信号用
// 单元格变动触发
//void slotDataChanged(int topLeft, int bottomRight);
//右键按下触发
//void ClickedLeftSlot(QTableViewItem* item);
//左键按下触发

/////////////////////////////////////////////////////////////////


// override
//virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;


//栏高度


//

void CTableView::updateSectionWidth(int logicalIndex, int oldSize, int newSize)
{
	if (logicalIndex < TableSize.btw)
		s1->setColumnWidth(logicalIndex, newSize);
	updateFrozenTableGeometry();
}

void CTableView::updateSectionWidthS1(int logicalIndex, int oldSize, int newSize)
{
	//s1改变
	if (logicalIndex < TableSize.btw)
		setColumnWidth(logicalIndex, newSize);
}

void CTableView::updateSectionHeight(int logicalIndex, int oldSize, int newSize)
{
	s1->setRowHeight(logicalIndex, newSize);
}

void CTableView::updateSectionHeightS1(int logicalIndex, int oldSize, int newSize)
{
	//S1改变
	setRowHeight(logicalIndex, newSize);
}

void CTableView::updateFrozenTableGeometry()
{
	int btWidth{ 2 };
	for (int n = 0; n < TableSize.btColCount; n++)
	{
		btWidth += columnWidth(n);
	}
	TableSize.btw = btWidth;
	s1->setGeometry(verticalHeader()->width() + frameWidth(),
		frameWidth(),
		btWidth,
		viewport()->height() + horizontalHeader()->height());
	s1->show();
}

//恢复
void CTableView::undo()
{
	if (t_Data.UndoCount == 0)
		return;
	UnDoData& ud = t_Data.u_Array[--t_Data.UndoCount];
	if (ud.uCol != -1)
	{
		t_Data.d_Array[ud.uRow].ColVarList[ud.uCol] = ud.undoCellData.ColVarList[0];
	}
	else
	{
		//恢复插入或删除的数据
		if (!ud.undoCellData.ColVarList.empty())
		{
			//恢复删除数据
			m_Model->insertRow(ud.uRow);
			t_Data.d_Array[ud.uRow] = ud.undoCellData;
		}
		else
		{
			//恢复插入数据
			m_Model->removeRow(ud.uRow);
		}
	}
	viewport()->update();
	setSelectItem(ud.uRow, ud.uCol);
	QString s = QString::asprintf(" 第 %d 行撤消成功，还有 %d 行可撤消 ",t_Data.UndoCount  + 1, t_Data.UndoCount);
	emit SendMessages(s);
}

//将恢复的数据还原
void CTableView::redo()
{
	if (t_Data.u_Array.size() <= t_Data.UndoCount)
		return;
	UnDoData& rd = t_Data.u_Array[t_Data.UndoCount++];
	if (rd.uCol > -1)
	{
		t_Data.d_Array[rd.uRow].ColVarList[rd.uCol] = rd.redoCellData.ColVarList[0];
	}
	else
	{
		if (rd.redoCellData.ColVarList.empty())//
		{
			m_Model->removeRow(rd.uRow);
		}
		else
		{
			m_Model->insertRow(rd.uRow);
			t_Data.d_Array[rd.uRow] = rd.redoCellData;
		}
	}
	viewport()->update();
	setSelectItem(rd.uRow, rd.uCol);
	QString s;
	s = QString::asprintf(" 第 %d 行恢复成功，还有 %d 行可恢复 ", t_Data.UndoCount , t_Data.u_Array.size() - t_Data.UndoCount);
	emit SendMessages(s);
}

void CTableView::insertRow(int row, int flage)
{
	int newRow{ row };
	ColumnClass r = t_Data.d_Array[row];//复制拷贝
	//插入空行
	if (flage)
	{
		newRow = model()->rowCount();
		m_Model->insertRow(newRow);
	}
	else
	{
		m_Model->insertRow(newRow);
	}
	r.oldRow = 0;
	//以下生成UNDO数据
	UnDoData ud;
	while (t_Data.u_Array.size() > t_Data.UndoCount)
	{
		// 从尾部删除，直到条件满足
		t_Data.u_Array.pop_back();
	};
	ud.uRow = newRow;
	ud.redoCellData = r;
	ud.uCol = -1;
	t_Data.d_Array[newRow] = r;
	t_Data.u_Array.push_back(ud);
	t_Data.UndoCount++;
	//刷新表格
	viewport()->update();
}

void CTableView::removeRow(int row)
{
	ColumnClass r = t_Data.d_Array[row];//复制拷贝
	m_Model->removeRow(row);
	while (t_Data.u_Array.size() > t_Data.UndoCount)
	{
		// 从尾部删除，直到条件满足
		t_Data.u_Array.pop_back();
	};
	UnDoData ud;
	ud.uRow = row;
	ud.undoCellData = r;
	t_Data.u_Array.push_back(ud);
	t_Data.UndoCount++;
	viewport()->update();
}

