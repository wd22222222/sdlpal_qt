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
#include "cListScriptCall.h"
#include "cviewmodel.h"
#include "mainwindow.h"

CListScriptCall::CListScriptCall(MainWindow* para, unsigned short scriptEntry)
	:QTableView(para), m_parat(para)
	//查看脚本调用
{
	QString  title = QString::asprintf("查看 %4.4X 的脚本调用者", scriptEntry);
	setWindowTitle(title);
	auto s_Size = para->size();
	s_Size -= {80, 60};
	resize(s_Size);
	setSelectionMode(QAbstractItemView::SingleSelection);//单选
	setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑 
	setSelectionBehavior(QAbstractItemView::SelectRows);//选择行
	verticalHeader()->setVisible(true);//显示表头
	setWindowFlags(Qt::Dialog);//对话框样式
	auto Pal = m_parat->pal();
	auto& sgMark = Pal->pMark[scriptEntry].s;
	if (sgMark.size() == 0)
		return;

	int totalCol = 4;
	ColArray& w_ColData = s_Data.c_Array;
	//auto p_SrcToStr = std::bind(&MainWindow::q_SrcToStr, this, std::placeholders::_1);
	w_ColData.resize(totalCol);
	w_ColData[0].GetData("来  源", 180, 0, ctrl_Null, tSTR);
	w_ColData[1].GetData("调用者", 180, 1, ctrl_Null, tSTR);
	w_ColData[2].GetData("调用者位置", 180, 2, ctrl_Null, tSTR);
	w_ColData[3].GetData("调用者16进制", 180, 3, ctrl_Null, tSTR);
	do
	{
		auto& sgMark = Pal->pMark[scriptEntry].s;
		if (sgMark.size() == 0)
			return;
		DataArray& s_d = s_Data.d_Array;
		s_d.resize(sgMark.size()); ;
		for (uint32_t n = 0; n < sgMark.size(); n++)
		{
			QString cS;
			INT32 from = sgMark[n].from;
			INT32 save = sgMark[n].save;
			INT32 col = sgMark[n].col;
			INT32 row = sgMark[n].row;
			switch (save)
			{
			case 0:
				cS = ("初始设定");
				break;
			default:
				cS = QString::asprintf("%d.rpg", save - 
					CPalEvent::ggConfig->m_Function_Set[53]);
				break;
			}
			s_d[n].ColVarList.resize(4);
			s_d[n].ColVarList[0] = cS.toStdString();
			switch (from)
			{
			case 1:
				cS = "对象";;
				break;
			case 2:
				cS = "场景";
				break;
			case 3:
				cS = "事件";
				break;
			case 4:
				cS = "脚本";
				break;
			default:
				return;//出错
				break;
			}
			s_d[n].ColVarList[1] = cS.toStdString();
			cS = QString::asprintf("%d", (col) & 0xff);
			s_d[n].ColVarList[2] = cS.toStdString();
			cS = QString::asprintf("%4.4X", (row) & 0xffff);
			s_d[n].ColVarList[3] = cS.toStdString();
		}
	} while (FALSE);
	if (m_Model)
		delete m_Model;
	m_Model = new CViewModel;
	m_Model->set_t_Data(&s_Data);
	setModel(m_Model);
	for (int i = 0; i < totalCol; i++)
	{
		setColumnWidth(i, w_ColData[i].ColWidth);
	}
	show();
	setFocus();
	setCurrentIndex(model()->index(0, 0));
}

CListScriptCall::~CListScriptCall()
{
	if (m_Model)
		delete m_Model;
}

void CListScriptCall::focusOutEvent(QFocusEvent* event)
{
	// 调用父类的 focusOutEvent 实现
	QTableView::focusOutEvent(event);
	//this->hide();
}

void CListScriptCall::keyPressEvent(QKeyEvent* event)
{
	if (event->matches(QKeySequence::Cancel) || event->key() == Qt::Key_Space)
		hide();
	else
		QTableView::keyPressEvent(event);
}
