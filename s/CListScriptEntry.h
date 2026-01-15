#pragma once
#include <qtableview.h>
#include "cviewmodel.h"
#include "mainwindow.h"
#include "cgetpaldata.h"
#include <map>
#include "..\main\command.h"
#include "cviewmodel.h"

//using namespace std;
using  MAPScript = std::map<WORD, WORD>;

std::string  p_Script(int i);

class CListScriptEntry :
    public QTableView
{
    MainWindow* m_parat{};
	CViewModel* m_Model{};
	T_DATA s_Data;
public:
    CListScriptEntry(MainWindow* para, WORD scriptEntry)
        :QTableView(para), m_parat((MainWindow*)para)
    {
        QString s;
        s = QString::asprintf("查看 %4.4X 脚本调用", scriptEntry);
        setWindowTitle(s);

		auto s_Size = para->size();
		s_Size -= {80, 60};
		resize(s_Size);
		setSelectionMode(QAbstractItemView::SingleSelection);//单选
		setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑 
		setSelectionBehavior(QAbstractItemView::SelectRows);//选择行
		verticalHeader()->setVisible(true);//显示表头
		setWindowFlags(Qt::Dialog);//对话框样式
		if (!scriptEntry)
			return;
		auto Pal = m_parat->pal();
		int totalCol = 6;
		ColArray& w_ColData = s_Data.c_Array;
		//auto p_SrcToStr = std::bind(&MainWindow::q_SrcToStr, this, std::placeholders::_1);
		w_ColData.resize(totalCol);
		w_ColData[0].GetData("脚本ID", 80, 0, ctrl_Fix, tHEX);
		w_ColData[1].GetData("入口", 80, 1, ctrl_Fix, tHEX);
		w_ColData[2].GetData("参数1", 80, 2, ctrl_Null, tHEX);
		w_ColData[3].GetData("参数2", 80, 3, ctrl_Null, tHEX);
		w_ColData[4].GetData("参数3", 80, 4, ctrl_Null, tHEX);
		w_ColData[5].GetData("说  明", 700, 5, ctrl_Null, tSTR);

		int line{ 0 };
		
		int sn = scriptEntry;
		do
		{
			MAPScript mMapScrict;
			MAPScript::iterator iter;
			Pal->MarkSprictJumpsAddress(scriptEntry, mMapScrict);
			DataArray &s_RowData = s_Data.d_Array;
			int line = 0;
			for (iter = mMapScrict.begin(); iter != mMapScrict.end(); iter++)
			{
				std::string cS;
				sn = iter->first;
				if (sn == 0)
					continue;			
				LPSCRIPTENTRY p = &Pal->pal->gpGlobals->g.lprgScriptEntry[sn];
				ColumnClass c;
				c.ColVarList.resize(totalCol);
				c.ColVarList[0] = sn;
				c.ColVarList[1] = p->wOperation;
				c.ColVarList[2] = p->rgwOperand[0];
				c.ColVarList[3] = p->rgwOperand[1];
				c.ColVarList[4] = p->rgwOperand[2];
				cS = p_Script((INT)p->wOperation);
				if (p->wOperation == 0xffff) 
				{ 
					cS += ":  《" + Pal->pal->PAL_GetMsg(p->rgwOperand[0]) +"》";
				};
				c.ColVarList[5] = cS;
				s_RowData.push_back(std::move( c));
				line++;
			}
		} while (false);
		if (m_Model)
			delete m_Model;
		m_Model = new CViewModel;
		m_Model->set_t_Data(&s_Data);
		setModel(m_Model);
		//自动设置行高
		verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
		for (int i = 0; i < totalCol; i++)
		{
			setColumnWidth(i, w_ColData[i].ColWidth);
		}
		show();
		setFocus();
		setCurrentIndex(model()->index(0,0));
	};
	
	virtual ~CListScriptEntry() 
	{
		if (m_Model)
			delete m_Model;
	};

	void focusOutEvent(QFocusEvent* event) override
	{
		// 调用父类的 focusOutEvent 实现
		QTableView::focusOutEvent(event);
		this->hide();
	};
	void keyPressEvent(QKeyEvent* event)override
	{
		if (event->matches(QKeySequence::Cancel) || event->key() == Qt::Key_Space)
			hide();
		else
			QTableView::keyPressEvent(event);
	};

};

