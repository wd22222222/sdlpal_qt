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

#include "cviewmodel.h"
#include "cpopview.h"
#include <qcolor.h>
#include <qdebug.h>
#include <qsize.h>
#include <regex>
#include "mainwindow.h"

QVariant CViewModel::data(const QModelIndex& index, int role) const
{
	auto& t_Data = *t_pData;
	if (role == Qt::UserRole)
	{
        //int row = index.row(), col = index.column();
		//返回 并记录编辑的坐标
		return QVariant::fromValue(&t_Data);
	}
	if (!t_pData->c_Array.size() || !t_pData->d_Array.size())
		return QVariant();
	//
	if (!index.isValid())
		return QVariant();
	int row = index.row(), col = index.column();

    if (index.row() >= (int)t_Data.d_Array.size() || row < 0 || index.column() >= (int)
            t_Data.c_Array.size() || col < 0)
		return QVariant();

	switch (role)
	{
	case Qt::DisplayRole://显示
	case Qt::EditRole://编辑当单元格（item）进入编辑态时（一般双击会进入编辑态），要显示的数据
		if (t_Data.Set_Col_Data)
			t_Data.Set_Col_Data(&t_Data.d_Array[row],row);
		return t_pData->framCellData(row, col, role);
		break;
	case Qt::DecorationRole://显示图标
		if (t_Data.c_Array[col].f_Pixmap)
			return t_Data.c_Array[col].f_Pixmap(row);
		return QVariant(); 
		break;
	case Qt::BackgroundRole://底色
		if (row & 1)
			return QColor(0xfbfbfb);
		return QColor(0xffffff);
		break;
	case Qt::ForegroundRole://字体颜色
		//return QColor(0x010101);
		break;
	case Qt::FontRole://用于使用默认委托呈现的项目的字体. (QFont)
		break;
	case Qt::TextAlignmentRole://使用默认委托呈现的项的文本对齐方式. (Qt::Alignment)
		if (t_Data.c_Array[col].DataStat == tSTR || t_Data.c_Array[col].f_InCol)
			break;
        return (int)Qt::AlignRight | Qt::AlignVCenter;
		break;
	case Qt::WhatsThisRole://“这是什么？”模式下要显示的问题. (QString)
		break;
	default:
		break;
	}
	return QVariant();
};

bool CViewModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	QAbstractTableModel::setData(index, value, role);
	if (role != Qt::EditRole)
	{
		return FALSE;
	}
	auto oldVar = data(index, role);
	if (oldVar == value || value.isNull())
		return FALSE;
	//使用新数据更新
	int col = index.column(), row = index.row();
	chuckRT t = t_pData->c_Array[col].f_chuck;
	if (t &&
		t(value.toString(), row) == 0)//较验
		return FALSE;
	UnDoData adUndo{};
    while ((int)t_pData->u_Array.size() > t_pData->UndoCount)
	{
		// 从尾部删除，直到条件满足
		t_pData->u_Array.pop_back();
	};
	COLVARLIST oldVarList;
	oldVarList.push_back(getCellData(oldVar.toString(),col));
	COLVARLIST newVarList;
	newVarList.push_back(getCellData(value.toString(),col));

	adUndo.uRow = row, adUndo.uCol = col;
	adUndo.redoCellData.ColVarList = std::move(newVarList);
	adUndo.undoCellData.ColVarList = std::move(oldVarList);
	t_pData->u_Array.push_back(adUndo);
	t_pData->UndoCount++;
	//数据更新
	t_pData->d_Array[row].ColVarList[col] = getCellData(value.toString(), col);
	if (t_pData->Set_Col_Data)
		t_pData->Set_Col_Data(&(t_pData->d_Array[row]),0);
	return TRUE;
}

Qt::ItemFlags CViewModel::flags(const QModelIndex& index) const
{
	return QAbstractTableModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;;
}

//返回表头数据
QVariant CViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		return t_pData->c_Array[section].ColTop;
	}
	if (role == Qt::BackgroundRole && orientation == Qt::Horizontal)
		return QColor(0xf0f0f0);

	return QAbstractTableModel::headerData(section, orientation, role);
}

int CViewModel::insertRow(int row, const QModelIndex& aparent)
{
	beginInsertRows(aparent,row,row);
	auto& t = t_pData->d_Array;
	ColumnClass r{};
	t.insert(t.begin() + row, r);
	endInsertRows();
	return 1;
	//return QAbstractTableModel::insertRow(row, aparent);
}

int CViewModel::removeRow(int row, const QModelIndex& aparent)
{
	beginRemoveRows(aparent,row,row);
	auto& t = t_pData->d_Array;
	t.erase(t.begin() + row, t.begin() + row + 1);
	endRemoveRows();
	return 1;
}

//引入数据
void CViewModel::set_t_Data(T_DATA* mt_Data)
{
	t_pData = mt_Data;
}

//返回数据 供代理使用
T_DATA& CViewModel::get_t_Data() { 
	return *t_pData;
}

COLVAR CViewModel::getCellData(QString val, int n_col)
{
	// TODO: 在此处添加实现代码.
	//从显示数据 qstring 转换成存储数据 COLVAR
	COLDATA& colData = t_pData->c_Array[n_col];
	return getCellData(val, colData);
}

COLVAR CViewModel::getCellData(QString val, COLDATA& colData)
{
	std::string s_Text = QString(val).toStdString();
	COLVAR rtVal{};
	if (colData.f_OutCol)
		rtVal = (colData.f_OutCol)(s_Text.c_str());
	else if (colData.Ctrl > 1)
	{
		switch (colData.DataStat)
		{
		case tDWORD:
		{
			int i;
			sscanf_s(s_Text.c_str(), "%lu", &i);
			rtVal = i;
			break;
		}
		case tWORD:
		{
			int i;
			sscanf_s(s_Text.c_str(), "%u", &i);
			rtVal =(WORD) i;
			break;
		}
		case tINT:
		case tNull:
		{
			int i;
			sscanf_s(s_Text.c_str(), "%d", &i);
			rtVal = i;
			break;
		}
		case tSHORT:
		{
			int i;
			sscanf_s(s_Text.c_str(), "%d", &i);
			rtVal = (SHORT)i;
			break;
		}
		case tHEX:
		{
			int i;
			sscanf_s(s_Text.c_str(), "%x", &i);
			rtVal = i;
			break;
		}
		case tFLOAT:
		{
			double i;
			sscanf_s(s_Text.c_str(), "%lf", &i);
			rtVal = i;
			break;
		}
		case tSTR:
		{
			std::string s = s_Text + "";
			rtVal = s;
			break;
		}
		case tBOOL:
		{
			std::regex re("[Yy0]{1}");
			rtVal = std::regex_match(s_Text, re);
			break;
		}
		default:
			break;;
		}
	}
	return rtVal;
}


CViewModel::CViewModel(QObject* parent)
	: QAbstractTableModel(parent) {
	//connect(this, &CViewModel::SendMessages,this , &MainWindow::ShowMessage);//向窗口发送文字信息

}

int CViewModel::rowCount(const QModelIndex& parent) const
{ 
	Q_UNUSED(parent);

	if (!t_pData)
		return 0;
    return t_pData->d_Array.size();
}

int CViewModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	if (!t_pData)
		return 0;
	return t_pData->c_Array.size();
};

/////////////////////////////////////////////+

int CEditlistModel::rowCount(const QModelIndex& parent) const
{
	return m_List->data.size();
}

int CEditlistModel::columnCount(const QModelIndex& parent) const
{
	return 3;
}

QVariant CEditlistModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if (!m_List->data.size())
		return QVariant();
	if (role != Qt::DisplayRole)
		return QVariant();
	int row = index.row(), col = index.column();
	assert(row < m_List->data.size() && col < 3);
	QString t;
	switch (col)
	{
	case 0:
		t = QString::asprintf("%4d", row + 1);
		break;
	case 1:
		t = QString::asprintf("%4.4x", m_List->data[row].item);
		break;
	case 2:
		t = m_List->data[row].s.c_str();
		break;
	default:
		break;
	}
	return t;
}

QVariant CEditlistModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	
	if (role != Qt::DisplayRole || orientation  != Qt::Horizontal)
		return  QAbstractTableModel::headerData(section, orientation, role);
	QStringList t;
	t << "行" << "ID" << "名称";
	return t[section];
}

bool CEditlistModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	int row = index.row(), col = index.column();

	return true;;
}

//将原数据转换为指定格式的字符串
QVariant T_DATA::framCellData(int row, int col, int role)
{
	if (d_Array[row].ColVarList.empty())
		return QVariant();
	COLVAR var = d_Array[row].ColVarList[col];
	if (c_Array[col].f_InCol && role == Qt::DisplayRole)
        return c_Array[col].f_InCol(std::get<int>(var));//使用转换函数
	QString s_Text;
	switch (c_Array[col].DataStat)
	{
	case tSHORT:
        s_Text = QString::asprintf("%d", (SHORT)std::get<int>(var));
		break;
	case tINT:
	case tNull:
        s_Text = QString::asprintf("%d", std::get<int>(var));
		break;
	case tWORD:
        s_Text = QString::asprintf("%u",(WORD)std:: get<int>(var));
		break;
	case tDWORD:
        s_Text = QString::asprintf(("%lu"), (DWORD)std::get<int>(var));
		break;
	case tHEX:
        s_Text = QString::asprintf(("%4.4X"), (WORD)std::get<int>(var));
		break;
	case tFLOAT:
        s_Text = QString::asprintf(("%f"), (WORD)std::get<int>(var));
		break;
	case tSTR:
		//处理字符串
		s_Text = std::get<std::string>(var).c_str();
		break;
	case tBOOL:
		//布尔数
        s_Text = std::get<int>(var) == 0 ? "N" : "Y";
		break;
	case tIMAGE:
	default:
		return QVariant();
		break;
	}
	return s_Text;
}
