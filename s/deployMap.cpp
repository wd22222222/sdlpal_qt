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

#include <qevent.h>
#include <qscrollbar.h>
#include "deploymap.h"
#include "mainwindow.h"
#include "t_data.h"
#include "cgetpaldata.h"
#include "../main/cpaldata.h"
#include "../main/cpalbaseio.h"
#include <qinputdialog.h>
#include <qmenu.h>
static const char * pMenu_Str[] =
{
	"1.显示障碍",
	"2.显示全体对象",
	"3.固定显示对象 %3.3d",
	"4.移动对象 %3.3d",
	"5.修改对象 %3.3d触发范围",
	"6.修改对象 %3.3d形象",
	//"7.在光标处插入新的对象",
	nullptr
};

enum tagMapFlags
{
	a_ShowBarrier = 1 << 0,//障碍标识
	a_AllObject = 1 << 1,//显示全体对象
	a_TheObject = 1 << 2,//固定显示对象%3.3d
	a_MoveObject = 1 << 3,//移动对象 % 3.3d
	a_MakeTriggerMode = 1 << 4, //修改对象%3.3d触发范围
	a_ModifiTargetImage = 1 << 5,//修改对象 %3.3d形象
	a_InsertObject = 1 << 6, //在光标处插入新的对象
};


INT deployMap::MakeMapImage(QImage& mm, const DWORD flag)
{
	QImage m = QImage(MapPixWidth, MapPixHight, QImage::Format_Indexed8);
	m.setColorTable(m_Pal->sdl_PaltteToQColorList( m_Pal->pal->gpPalette->colors));
	m.fill(m_Pal->sdl_colorToUint( m_Pal->pal->gpPalette->colors[0]));
	PAL_Rect rect = { 0,0,MapPixWidth,MapPixHight };
	//画底层
	m_Pal->PAL_MapBlitToSurface(m_Pal->m_pPalMap, m, &rect, 0);
	//画顶层
	m_Pal->PAL_MapBlitToSurface(m_Pal->m_pPalMap, m, &rect, 1);
	//画全部对象
	//
	// Event Objects (Monsters/NPCs/others)
	//
	int i, x, y, vy;
	auto gpGlobals = m_Pal->pal->gpGlobals;
	for (i = gpGlobals->g.rgScene[iScene - 1].wEventObjectIndex;
		i < gpGlobals->g.rgScene[iScene].wEventObjectIndex; i++)
	{
		LPCBITMAPRLE     lpFrame{ nullptr };
		LPEVENTOBJECT    lpEvtObj = &(gpGlobals->g.lprgEventObject[i]);
		int              iFrame{ 0 };

		int t;
		if (!(m_Flags & a_AllObject) && (m_ListSelectRow != i - (t = gpGlobals->g.rgScene[iScene - 1].wEventObjectIndex)))
		{
			continue;//非画全体
		}
		//
		// Get the sprite
		//
		auto lpSprite = m_Pal->pal->PAL_GetEventObjectSprite((WORD)i + 1);
		if ( lpSprite == nullptr)
		{
			continue;
		}
		iFrame = lpEvtObj->wCurrentFrameNum;
		lpFrame = m_Pal->pal->PAL_SpriteGetFrame((LPCSPRITE)lpSprite,
			lpEvtObj->wDirection * lpEvtObj->nSpriteFrames + iFrame);

		if (lpFrame == NULL)
		{
			continue;
		}
		//
		// Calculate the coordinate and check if outside the screen
		//
		x = (SHORT)lpEvtObj->x;
		x -= m_Pal->pal->PAL_RLEGetWidth(lpFrame) / 2;

		y = (SHORT)lpEvtObj->y;
		y += lpEvtObj->sLayer * 8 + 9;

		vy = y - m_Pal->pal->PAL_RLEGetHeight(lpFrame) - lpEvtObj->sLayer * 8 + 2;

		m_Pal->PAL_RLEBlitToSurface(lpFrame, m, PAL_XY(x, vy));
	}
	mm = m.convertToFormat(QImage::Format_ARGB32);
	//画对象所在位置，用黄笔
	//auto& gpGlobals = pal->gpGlobals;
	for (int i = gpGlobals->g.rgScene[iScene - 1].wEventObjectIndex;
		i < gpGlobals->g.rgScene[iScene].wEventObjectIndex; i++)
	{
		if (!(m_Flags & a_AllObject) && (m_ListSelectRow != i - (gpGlobals->g.rgScene[iScene - 1].wEventObjectIndex)))
		{
			continue;//非画全体
		}
		LPEVENTOBJECT    lpEvtObj = &(gpGlobals->g.lprgEventObject[i]);
		DrawingRhombShape(mm, 32, 16, lpEvtObj->x - 16, lpEvtObj->y - 8, qRgb(240, 240, 0));
	}
	//画障碍
	if (flag & a_ShowBarrier)
		DrawObstacles(mm, &rect);
	if (((short)m_ListSelectRow) < 0)
		m_ListSelectRow = 0;
	//固定显示对象
	if (flag & (a_TheObject | a_MoveObject))
	{
		WORD k = m_Pal->pal->gpGlobals->g.rgScene[iScene - 1].wEventObjectIndex +
			m_ListSelectRow;
		LPEVENTOBJECT p = &m_Pal->pal->gpGlobals->g.lprgEventObject[k];
		m_OriginPoint.setX(p->x);
		m_OriginPoint.setY(p->y);
		m_TriggerMode = p->wTriggerMode;
	}
	else
	{
		m_TriggerMode = 0;
		m_OriginPoint.setX(0);
		m_OriginPoint.setY(0);
	}
	return 0;
}

//将界面重新画一遍
void deployMap::drawAllMap()
{
	//m_Times++;
	//if (m_ImageLabel.)
		//return;
	m_NoDraw = 0;
	//调整放大倍率
	m_MapZoom = m_MapZoom < 1.5 ? m_MapZoom : 1.5;
	m_MapZoom = m_MapZoom > 0.25 ? m_MapZoom : 0.25;
	//建立中间表
	auto dRect = PalQRect(m_ImageLabel.geometry());
	dRect.x = dRect.y = 0;
	QPixmap dstMap(dRect.w, dRect.h);// QImage::Format_ARGB32);
	PalQRect sRect = labelRectToImage(dRect, m_RightTopPos, m_MapZoom);
	//原表向中间表写
	imageCopy(dstMap, dRect.toQRect(), m_Map, sRect.toQRect());
	QSize s_Size(sRect.w, sRect.h);
	s_Size *= m_MapZoom;
	QRect spRect = sRect.toQRect();//从地图图片上剪裁的区域
	if (m_Times & 4)
	{
		//在上次点击处画菱形y=ax+b x=(y-b)/a
		PalQRect r = imageToLabelRect(m_MapPixSelectRect, m_RightTopPos, m_MapZoom);
		auto s = m_MapPixSelectRect.toQRect().contains(m_lastScreenPoint);
		DrawingRhombShape(&dstMap, r.w, r.h, r.x, r.y, qRgb(255, 255, 255), 1);
	}
	if (((short)m_ListSelectRow) < 0)
		m_ListSelectRow = 0;
	{
		WORD k = m_Pal->pal->gpGlobals->g.rgScene[iScene - 1].wEventObjectIndex +
			m_ListSelectRow;
		LPEVENTOBJECT p = &m_Pal->pal->gpGlobals->g.lprgEventObject[k];
		m_FixedPoint = PAL_XY(p->x, p->y);
		m_TriggerMode = p->wTriggerMode;
	}
	if ((m_FixedPoint) && ((m_Times + 1) & 4))
	{
		//标注固定点
		//移动屏幕，将点位置移动到中部
		if (m_Flags & (a_TheObject | a_MoveObject | a_MakeTriggerMode))
			moveDotToMiddle(m_FixedPoint);//设置成不可移动
		PAL_Rect fRect(PAL_X(m_FixedPoint) - 16, PAL_Y(m_FixedPoint) - 8, 32, 16);
		PAL_Rect pRect = imageToLabelRect(fRect, m_RightTopPos, m_MapZoom);
		DrawingRhombShape(&dstMap, pRect.w, pRect.h, pRect.x, pRect.y, qRgb(0, 255, 255), 6);
	}
	if (m_TriggerMode > 0 && (m_Times + 2) & 4)
	{
		int w = ((m_TriggerMode & 3) << 6) + 32;
		int h = ((m_TriggerMode & 3) << 5) + 16;
		PAL_Rect fRect((PAL_X(m_FixedPoint)) - w / 2, (PAL_Y(m_FixedPoint)) - h / 2, w, h);
		PAL_Rect pRect = imageToLabelRect(fRect, m_RightTopPos, m_MapZoom);
		DrawingRhombShape(&dstMap, pRect.w, pRect.h, pRect.x, pRect.y,
			m_TriggerMode & 4 ? qRgb(0, 0, 255) : qRgb(0, 255, 0), 4);
	}
	m_ImageLabel.setPixmap((dstMap));
	//小图
	PAL_Rect& r = sRect;
	double miniZoom = 1.0f * m_MiniMapLable.width() / m_Map.width();
	QPixmap mini = QPixmap::fromImage(m_Map).scaled(m_Map.size() * miniZoom, Qt::KeepAspectRatio);
	DrawSquare(&mini, (r.w * miniZoom), (r.h * miniZoom), r.x * miniZoom, r.y * miniZoom, qRgb(255, 255, 255), 2);
	m_MiniMapLable.setPixmap(mini);

	QString s;
	QString s1;
	WORD k = m_Pal->pal->gpGlobals->g.rgScene[iScene - 1].wEventObjectIndex
		+ m_ListSelectRow;
	for (size_t n = 0; n < 2; n++)
	{
		s1 = QString::asprintf("%s%s, ",pMenu_Str[n], m_Flags & 1 << n ? "√" : "×");
		s += s1;
	}
	s1 = QString::asprintf(pMenu_Str[2], k);
	s += s1;
	if (m_UndoCount)
	{
		//历史不为空，显示历史数据
		s1 = QString::asprintf("已经修改 %d 处，Ctr+Z 撤销， ", m_UndoCount);
		s += s1;
	}
	for (size_t n = 3; pMenu_Str[n]; n++)
	{
		if (m_Flags & 1 << n)
		{
			s += "  编辑(双击)：";
			s1 = QString::asprintf(pMenu_Str[n], k);
			s += s1;
		}
	}
	m_tEdit.setText(s);
}

void deployMap::listSelected(int row)
{
	m_ListSelectRow = row;
	WORD k = m_Pal->pal->gpGlobals->g.rgScene[iScene - 1].wEventObjectIndex +
		m_ListSelectRow;
	LPEVENTOBJECT p = &m_Pal->pal->gpGlobals->g.lprgEventObject[k];
	m_TriggerMode = p->wTriggerMode;
	MakeMapImage(m_Map, m_Flags);
}

int deployMap::doUpdateMap()
{
	int rt = CPixEdit::doUpdateMap();
	if (rt != -1)
		return rt;//不撤消修改返回
	//撤消修改
	while (m_UndoCount > 0)
	{
		doUndo();
	}
	m_UndoArray.clear();
	return 1;
}

deployMap::deployMap(int scene,T_DATA * data, QWidget* para)
	:CPixEdit(para), iScene(scene), m_pData(data)
{
	auto w = qobject_cast<MainWindow*>(para);
	m_Pal = w->pal();
	makeALL();
	listSelected(0);

	connect(m_List, &QTableView::clicked, this, [&](const QModelIndex& index)->void {
		listSelected(index.row());
		});
	connect(this, &CPixEdit::RightButtonClicked, this, &deployMap::ClickedRightSlot);
}

deployMap::~deployMap()
{
	if (m_UndoCount)
	{
		//保存修改，g.lprgEventObject有改动
		m_Pal->saveGameDataToCache();
		auto& s_RowData = s_Data.d_Array;
		auto p = m_Pal->pal->gpGlobals;
		auto n_Row = p->g.rgScene[iScene - 1].wEventObjectIndex ;
		auto& rowData = m_pData->d_Array;
		//以下更新主表数据
		for (int n = 0; n_Row < p->g.rgScene[iScene].wEventObjectIndex; n_Row++,n++)
		{
			rowData[n_Row].ColVarList.at(3) = s_RowData.at(n).ColVarList.at(1);//X
			rowData[n_Row].ColVarList.at(4) = s_RowData.at(n).ColVarList.at(2);//Y
			rowData[n_Row].ColVarList.at(5) = s_RowData.at(n).ColVarList.at(3);//形象号
			rowData[n_Row].ColVarList.at(9) = p->g.lprgEventObject[n_Row].wTriggerMode;//触发模式
		}
	}
}

void deployMap::makeALL()
{
	//准备工作设置数据
	const int totalCol = 4;
	auto g = m_Pal->pal->gpGlobals;
	g->wNumScene = iScene;
	if (!m_Pal->pal->gpResources)
		m_Pal->pal->gpResources = new CPalBaseIO::RESOURCES;
	m_Pal->pal->PAL_SetPalette(0, 0);
	m_Pal->pal->PAL_SetLoadFlags(kLoadScene | kLoadPlayerSprite);
	m_Pal->pal->PAL_LoadResources();
	m_Pal->m_pPalMap = m_Pal->pal->gpResources->lpMap;
	auto iMapNum = g->g.rgScene[iScene - 1].wMapNum;
	{
		QString text = QString::asprintf("在地图上部署 场景号 %d 地图号 %d", iScene, iMapNum);
		setWindowTitle(text);
	}
	auto& w_ColData = s_Data.c_Array;
	w_ColData.resize(totalCol);
	w_ColData[0].GetData("对象", 74, 0, ctrl_Null, tWORD);
	w_ColData[1].GetData("位置X", 74, 1, ctrl_Null, tWORD);
	w_ColData[2].GetData("位置Y", 74, 2, ctrl_Null, tWORD);
	w_ColData[3].GetData("形象号", 74, 3, ctrl_Null, tWORD);
	auto& s_RowData = s_Data.d_Array;
	auto& p = m_Pal->pal->gpGlobals->g;
	int maxRow = p.rgScene[iScene].wEventObjectIndex - p.rgScene[iScene - 1].wEventObjectIndex;
	s_RowData.resize(maxRow);
	int  n_Row{}, n_Object{};
	for (n_Object = p.rgScene[iScene - 1].wEventObjectIndex;
		(n_Object < (p.rgScene[iScene].wEventObjectIndex))
		&& n_Row < p.nEventObject; n_Object++, n_Row++)
	{
		s_RowData[n_Row].ColVarList.resize(totalCol);
		s_RowData[n_Row].ColVarList[0] = n_Object;
		s_RowData[n_Row].ColVarList[1] = p.lprgEventObject[n_Object].x;
		s_RowData[n_Row].ColVarList[2] = p.lprgEventObject[n_Object].y;
		s_RowData[n_Row].ColVarList[3] = p.lprgEventObject[n_Object].wSpriteNum;
	}
	if (m_ListModel)
		delete m_ListModel;
	m_ListModel = new CViewModel;
	m_ListModel->set_t_Data(&s_Data);
	m_List->setModel(m_ListModel);
	for (int i = 0; i < 4; i++)
		m_List->setColumnWidth(i, i < 3 ? 56 : 62);;
	m_List->setCurrentIndex(m_ListModel->index(0, 0));
	//以下生成图
	MakeMapImage(m_Map, m_Flags);
}

void deployMap::doUndo()
{
	if (m_UndoCount == 0)
		return;
	auto& u = m_UndoArray[--m_UndoCount];
	int k = u.tPos;
	m_Pal->pal->gpGlobals->g.lprgEventObject[k] = u.tOld;
	m_TriggerMode = u.tOld.wTriggerMode;
	makeALL();
	//MakeMapImage(m_Map, m_Flags);
}

void deployMap::doRedo()
{
	if (m_UndoCount >= (int)m_UndoArray.size())
		return;
	auto &u = m_UndoArray.at(m_UndoCount++);
	auto k = u.tPos;
	m_Pal->pal->gpGlobals->g.lprgEventObject[k] = u.tNew;
	m_TriggerMode = u.tNew.wTriggerMode;
	//MakeMapImage(m_Map, m_Flags);
	makeALL();
}

void deployMap::ClickedRightSlot(const QPoint pos)
{
	QMenu* pMenu = new QMenu(&m_ImageLabel);
	pMenu->setContextMenuPolicy(Qt::CustomContextMenu);
	//显示菜单，选中项打钩
	for (int i = 0; pMenu_Str[i]; i++)
	{
		QAction* a;
		QString text;
		WORD k = m_Pal->pal->gpGlobals->g.rgScene[iScene - 1].wEventObjectIndex + m_ListSelectRow;
		text = QString::asprintf(pMenu_Str[i], k);
		if (i != 5 && (m_Flags & (1 << i)))
			text += "  √";
		a = pMenu->addAction(text);
	}
	//点击菜单动作
	connect(pMenu, &QMenu::triggered, this, [&](QAction* action) {
		QString s = action->text();
		int i = s.left(1).toInt() - 1;
		if (i < 3)
		{
			m_Flags ^= (1 << i);
			if (i < 2)
			{
				MakeMapImage(m_Map, m_Flags);
			}
		}
		if (i == 5)
		{
			auto& s_RowData = s_Data.d_Array;
			//uint s_row = m_ListSelectRow;
			WORD k = m_Pal->pal->gpGlobals->g.rgScene[iScene - 1].wEventObjectIndex + m_ListSelectRow;
            int n = std::get<int>(s_RowData[m_ListSelectRow].ColVarList[3]);
			QString s = QString::asprintf("%d", n);
			//修改形象号
			bool ok{};
			int  row = QInputDialog::getText(nullptr, "修改形象", "请输入形象号",
				QLineEdit::Normal,s,&ok).toInt(NULL);
			int max = m_Pal->pal->PAL_MKFGetChunkCount( m_Pal->pal->gpGlobals->f.fpMGO);
			if (ok && row < max)
			{
				auto& p = m_Pal->pal->gpGlobals->g.lprgEventObject[k];
				while (m_UndoCount < m_UndoArray.size())
					m_UndoArray.pop_back();
				SceneUndo u;
				u.tOld = p;
				u.tPos = k;
				p.wSpriteNum = row;
				u.tNew = p;
				m_UndoArray.push_back(u);
				m_UndoCount++;
				makeALL();
				listSelected(m_ListSelectRow);
			}
		}
		if (i > 2 && i < 6)
		{
			m_Flags &= 7 + (1 << i);
			m_Flags ^= (1 << i);
		}
		});

	// 右键菜单显示的地方
	pMenu->exec(pos);
	delete pMenu;

}

void deployMap::resizeEvent(QResizeEvent* event)
{
	constexpr int listSize = 260;
	QDialog::resizeEvent(event);
	int cx = event->size().width();
	int cy = event->size().height();
	if (!m_List)
		return;
	m_List->resize(listSize, cy - listSize - 4);
	m_List->move(4, 4);
	m_vScrollBar->resize(20, cy - 60);
	m_vScrollBar->move(cx - 30, 4);
	m_hScrollBar->resize(cx - listSize - 40, 20);
	m_hScrollBar->move(listSize + 10, cy - 60);
	m_MiniMapLable.resize(listSize, listSize);
	m_MiniMapLable.move(4, cy - listSize - 4);
	m_tEdit.resize(cx - listSize - 14, 40);
	m_tEdit.move(listSize + 14, cy - 42);
	m_ImageLabel.resize(cx - listSize - 40, cy - 43 - 25);
	m_ImageLabel.move(listSize + 10, 4);
}

void deployMap::mouseDoubleClickEvent(QMouseEvent* event)
{
	auto pos = event->pos() - QPoint(m_ImageLabel.geometry().x(), m_ImageLabel.geometry().y());
	if (a_MoveObject & m_Flags) //移动对象 % 3.3d
		if (m_MapPixSelectRect.toQRect() == QRect(PAL_X(m_FixedPoint) - 16, PAL_Y(m_FixedPoint) - 8, 32, 16))
			return;//点击在目标上
		else
		{
			WORD k = m_Pal->pal->gpGlobals->g.rgScene[iScene - 1].wEventObjectIndex
				+ m_ListSelectRow;
			auto &p = m_Pal->pal->gpGlobals->g.lprgEventObject[k];
			while (m_UndoCount > m_UndoArray.size())
				m_UndoArray.pop_back();
			SceneUndo u;
			u.tNew = u.tOld = p;
			u.tPos = k;
			u.tNew.x = m_MapPixSelectRect.x + 16; u.tNew.y = m_MapPixSelectRect.y + 8;
			p.x = u.tNew.x; p.y = u.tNew.y;
			m_UndoArray.push_back(u);
			m_UndoCount++;
			MakeMapImage(m_Map, m_Flags);//重画大图
		}
	else if (a_MakeTriggerMode & m_Flags)//修改对象%3.3d触发范围
	{
		//修改触发方式
		if(m_MapPixSelectRect.toQRect() != QRect(PAL_X(m_FixedPoint) - 16, PAL_Y(m_FixedPoint) - 8, 32, 16))
			return;//末点击在目标上
		WORD k = m_Pal->pal->gpGlobals->g.rgScene[iScene - 1].wEventObjectIndex
			+ m_ListSelectRow;
		LPEVENTOBJECT p = &m_Pal->pal->gpGlobals->g.lprgEventObject[k];
		//建立菜单
		QMenu *m = new QMenu(&m_ImageLabel);
		m->setContextMenuPolicy(Qt::CustomContextMenu);
		for (int n = 0; n < 8; n++)
		{
			QString s = QString::asprintf(n < 4 ? "%1d.被动触发" : "%1d.主动触发", n + 1);
			if (n == p->wTriggerMode)
				s += "√";
			m->addAction(s);
		}
		connect(m, &QMenu::triggered, this, [&](QAction* action) {
			QString s = action->text();
			int i = s.left(1).toInt() - 1;
			WORD k = m_Pal->pal->gpGlobals->g.rgScene[iScene - 1].wEventObjectIndex
				+ m_ListSelectRow;
			EVENTOBJECT &p = m_Pal->pal->gpGlobals->g.lprgEventObject[k];
			if (i == p.wTriggerMode)
				return;
			SceneUndo undo;
			while (m_UndoCount > m_UndoArray.size())
				m_UndoArray.pop_back();
			m_UndoCount++;
			undo.tNew = undo.tOld = p;
			undo.tPos = k;
			p.wTriggerMode = m_TriggerMode = undo.tNew.wTriggerMode = i;
			m_UndoArray.push_back(undo);
			});
		m->exec(QCursor::pos());
		delete m;
	};
	CPixEdit::mouseDoubleClickEvent(event);
}

