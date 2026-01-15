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
#include <qheaderview.h>
#include <qpainter.h>
#include <QRgb>
#include <qtimer.h>
#include <qbitmap.h>
#include <qmenu.h>
#include <qmessagebox.h>
#include <qdebug.h>
#include "cmapedit.h"
#include "cgetpaldata.h"

const char* pMap_Menu_Str[10] =
{
    "0.显示底层",
    "1.显示上层",
    "2.显示障碍",
    "3.添加障碍",
    "4.清除障碍",
    "5.清除底层",
    "6.清除上层",
    "7.使用%3.3d替换底层",
    "8.使用%3.3d替换上层",
    nullptr
};


CMapEdit::CMapEdit(CGetPalData* pPal, QWidget* para):
	CPixEdit(para)
{
    m_Pal = pPal;
    m_ImageList = new QTableView(this);
    m_ImageList->setSelectionMode(QAbstractItemView::SingleSelection);//单选
    m_ImageList->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑 
    m_ImageList->setSelectionBehavior(QAbstractItemView::SelectRows);//选择行
    m_ImageList->verticalHeader()->setVisible(true);//显示表头

    connect(this, &CPixEdit::RightButtonClicked, this, &CMapEdit::ClickedRightSlot);
    //完成初始设置
    init(-1);
}

CMapEdit::~CMapEdit()
{
    delete m_ImageList;
}

//弹出菜单设置
void CMapEdit::ClickedRightSlot(QPoint pos)
{
    Q_UNUSED(pos)
    // 弹出左键菜单，
    QMenu* pMenu = new QMenu(&m_ImageLabel);
    pMenu->setContextMenuPolicy(Qt::CustomContextMenu);
    for (int i = 0; pMap_Menu_Str[i]; i++)
    {
        //QAction* a;
        QString text;
        if (i > 6)
            text = QString::asprintf(pMap_Menu_Str[i], m_ListSelectRow);
        else text = pMap_Menu_Str[i];
        if (m_Flags & (1 << i))
            text += "  √" ;
        pMenu->addAction(text);
    }
    connect(pMenu, &QMenu::triggered, this, [&](QAction* action) {
        QString s = action->text();
        int i = s.left(1).toInt();
        if (i < 3)
        {
            auto sFlags = m_Flags;
            sFlags ^= (1 << i);
            if (sFlags & 0b011)
            {
                m_Flags = sFlags;
                MakeMapImage(m_Map, m_Flags);
            }
        }
        if (i > 2 && i < 10)
        {
            m_Flags &= 7 + (1 << i);
            m_Flags ^= (1 << i);
        }
        }); 
    // 右键菜单显示的地方
    pMenu->exec(QCursor::pos());
    delete pMenu;

}

void CMapEdit::mapListClicked(const QModelIndex& index)
{
    int sRow = index.row();
    if (sRow == m_MapEdited)
        return;//选择未改变
    if (!doUpdateMap())
    {
        m_List->setCurrentIndex(m_ListModel->index(m_MapEdited, 0));//重新选择
        return;//已经修改放弃改变
    }
    init(sRow);//地图号从1 开始
    m_MapEdited = sRow;
};

void CMapEdit::doUndo()
{
    if (m_UndoCount <= 0)
        return;
    MAPUNDO& u = m_undoArray.at(--m_UndoCount);
    auto p = u.tPos;
    int x = (p >> 1) & 0x7f; int  y = (p >> 16) & 0xff; int h = p & 1;
    m_Pal->m_pPalMap->MapTiles[y][x][h] = u.tOld;
    m_MapPixSelectRect = ImagePointToDraw(u.tPos);
    MakeMapImage(m_Map, m_Flags);
}

void CMapEdit::doRedo()
{
    if (m_UndoCount >= (int)m_undoArray.size())
        return;
    MAPUNDO& u = m_undoArray.at(m_UndoCount++);
    auto i = u.tPos;
    int x = (i >> 1) & 0x7f, y = (i >> 16) & 0xff, h = i & 1;
    m_Pal->m_pPalMap->MapTiles[y][x][h] = u.tNew;
    m_MapPixSelectRect = ImagePointToDraw(u.tPos);
    MakeMapImage(m_Map, m_Flags);
}

void CMapEdit::doAddUndo(MAPUNDO & undo)
{
    while (m_UndoCount < (int)m_undoArray.size())
        m_undoArray.pop_back();
    m_undoArray.push_back(std::move(undo));
    m_UndoCount++;
}

int CMapEdit::doUpdateMap()
{
    int rt = CPixEdit::doUpdateMap();
    if (rt == 1 && m_UndoCount)
    {
        //保存地图数据到缓存
        m_Pal->doSaveMapTilesToArray(m_MapEdited);
    }
    if (rt == 0)//cancel
        return 0;
    //清除undo数据
    m_UndoCount = 0;
    m_undoArray.clear();
    return rt;
}


void CMapEdit::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    int cx = event->size().width();
    int cy = event->size().height();

    if (!m_List)
        return;
    m_List->resize(80, cy - 260);
    m_List->move(4, 4);
    m_ImageList->resize(188, cy - 260);
    m_ImageList->move(84, 4);
    m_ImageLabel.resize(cx - 266 - 30, cy - 50 - 25);
    m_ImageLabel.move(266, 4);
    m_vScrollBar->resize(20, cy - 60);
    m_vScrollBar->move(cx - 30, 4);
    m_hScrollBar->resize(cx - 266 - 30, 20);
    m_hScrollBar->move(266, cy - 60);
    m_MiniMapLable.resize(256, 256);
    m_MiniMapLable.move(4, cy - 260);
    m_tEdit.resize(cx - 270, 46);
    m_tEdit.move(270, cy - 46);
    m_tEdit.show();
}

void CMapEdit::init(int sMap)
{
    m_Pal->pal->PAL_SetPalette(0, 0);
    auto& fpMAP = m_Pal->pal->gpGlobals->f.fpMAP;
    auto& fpGOP = m_Pal->pal->gpGlobals->f.fpGOP;
    if (m_Pal->m_pPalMap)
    {
        m_Pal->pal->PAL_FreeMap(m_Pal->m_pPalMap);
    }
    m_Pal->m_pPalMap = m_Pal->pal->gpResources->lpMap = nullptr;
    if (sMap == -1)//初始阶段
    {
        int s_nMap{};
        s_nMap = m_Pal->pal->gpGlobals->PAL_MKFGetChunkCount(fpMAP);
        ColArray& w_ColData = s_Data.c_Array;
        w_ColData.resize(1);
        w_ColData[0].GetData("地图", 50, 0, ctrl_Null, tINT);
        m_List->setSelectionMode(QAbstractItemView::SingleSelection);//单选
        m_List->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑 
        m_List->setSelectionBehavior(QAbstractItemView::SelectRows);//选择行
        m_List->verticalHeader()->setVisible(false);//不显示表头
        /*设置默认行高*/
        m_List->verticalHeader()->setDefaultSectionSize(30);

        DataArray& s_RowData = s_Data.d_Array;

        s_RowData.resize(s_nMap);//行数

        for (int n = 0; n < s_nMap; n++)
        {
            s_RowData[n].ColVarList.resize(1);
            s_RowData[n].ColVarList[0] = n;
        }
        if (!m_ListModel)
            delete m_ListModel;
        m_ListModel = new CViewModel;
        m_ListModel->set_t_Data(&s_Data);
        m_List->setModel(m_ListModel);
        m_List->setColumnWidth(0, 50);
        m_List->setCurrentIndex(m_ListModel->index(0, 0));
        m_ListSelectRow = 0;
        //
        //connect(m_List, &QTableView::clicked, this, &CMapEdit::mapListClicked);
        connect(m_List->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [&](const QModelIndex& current, const QModelIndex& previous)->void {
                if (!current.isValid())
                    return;
                mapListClicked(current);
                //qDebug() << current.row() ;
            });

        //connect(m_List,&QTableView::sele)
        connect(m_ImageList, &QTableView::clicked, this, [&](const QModelIndex& index)->void {
            m_ListSelectRow = index.row(); });
        init(0);
        return;
    }
    auto& palMap = m_Pal->m_pPalMap;
    palMap = m_Pal->pal->PAL_LoadMap(sMap,m_Pal->pal->gpGlobals->f.fpMAP,m_Pal->pal->gpGlobals->f.fpGOP);
    if (!palMap)
    {
        m_ImageList->hide();
        m_Map.fill(Qt::black);
        m_ImageLabel.setPixmap(QPixmap::fromImage( m_Map));
        m_MiniMapLable.setPixmap(QPixmap::fromImage(m_Map));
        return;
    }
    WORD ntitle = *(WORD*)palMap->pTileSprite - 1;
    QString title = QString::asprintf("地图编辑 %3.3d 图片数 %d", sMap,ntitle);
    setWindowTitle(title);
    ColArray& w_ColData = s_ImageData.c_Array;
    w_ColData.resize(1);
    w_ColData[0].GetData("地图图片", 160, 0, ctrl_Pixmap, tIMAGE);
    DataArray& s_RowData = s_ImageData.d_Array;
    s_RowData.resize(ntitle);
    for (int i = 0; i < ntitle; i++)
    {
        s_RowData[i].ColVarList.resize(1);
        s_RowData[i].ColVarList[0] = i;
    }
    //设置取图片函数
    w_ColData[0].f_Pixmap = [&](int row)->QPixmap{
            if (!palMap)
                return QPixmap();
            QImage m(32, 15, QImage::Format_Indexed8);
            m.setColorTable(m_Pal->sdl_PaltteToQColorList( m_Pal->pal->gpPalette->colors));
            m.fill(0);
            auto lpSprite = m_Pal->pal->PAL_SpriteGetFrame((LPBYTE)palMap->pTileSprite, row);
            if (!lpSprite)
                return QPixmap();
            m_Pal->PAL_RLEBlitToSurface(lpSprite, m, 0);
            qreal scaleFactor = 3.0; // 放大3倍
            auto pix = QPixmap::fromImage(m);
            QBitmap mask(pix.size()); // 创建一个与原图片大小相同的位图掩码
            mask.fill(m_Pal->sdl_colorToUint( m_Pal->pal->gpPalette->colors[0])); //设置透明色 
            // 将掩码应用到图片上
            pix.setMask(mask.createMaskFromColor(
                m_Pal->sdl_colorToUint( m_Pal->pal->gpPalette->colors[0]), Qt::MaskOutColor));

            return pix.scaled(pix.size() * scaleFactor, Qt::KeepAspectRatio);
        };

    m_ImageModel = new CViewModel;
    m_ImageModel->set_t_Data(&s_ImageData);
    m_ImageList->setModel(m_ImageModel);
    m_ImageList->setGridStyle(Qt::NoPen); // 去除表格线
    m_ImageList->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);//自动设置行高
    m_ImageList->verticalHeader()->setVisible(true);
    m_ImageList->show();
    setSelectKeepHighlighted(m_ImageList);
    m_ImageList->setCurrentIndex(m_ImageModel->index(0, 0));
    //以下制作大图和小图
    MakeMapImage(m_Map, m_Flags);
}

INT CMapEdit::MakeMapImage(QImage& mm, const DWORD flag)
{
    QImage m = QImage(MapPixWidth, MapPixHight, QImage::Format_Indexed8);
    m.setColorTable(m_Pal->sdl_PaltteToQColorList( m_Pal->pal->gpPalette->colors));
    m.fill(m_Pal->sdl_colorToUint( m_Pal->pal->gpPalette->colors[0]));
    PAL_Rect rect = { 0,0,MapPixWidth,MapPixHight };
    //画底层
    if (flag & map_Bottom)
        m_Pal->PAL_MapBlitToSurface(m_Pal->m_pPalMap, m, &rect, 0);
    //画顶层
    if (flag & map_Top)
        m_Pal->PAL_MapBlitToSurface(m_Pal->m_pPalMap, m, &rect, 1);
    mm = m.convertToFormat(QImage::Format_ARGB32);
    //画障碍
    if (flag & map_Barrier)
        DrawObstacles(mm, &rect);
    return 0;
}

#define Tile_B(d)( (d & 0xff) |((d >> 4) & 0x100))
#define Tile_T(d) ( ( (d >> 16) & 0xff)|( (d>>20) & 0x100) ) 

//根据几个关键变量绘制大地图和小地图
// Plot large and small maps based on a few key variables
void CMapEdit::drawAllMap()
{
    if (m_ImageList->isHidden())
    {
        m_NoDraw = 1;
        return;
    }
    m_NoDraw = 0;
    //调整放大倍率
    m_MapZoom = m_MapZoom < 1.5 ? m_MapZoom : 1.5;
    m_MapZoom = m_MapZoom > 0.25 ? m_MapZoom : 0.25;
    //建立中间表
    auto dRect = PalQRect(m_ImageLabel.geometry());
    dRect.x = dRect.y = 0;
    QPixmap dstMap(dRect.w, dRect.h);

    PalQRect sRect = labelRectToImage(dRect, (QPoint)m_RightTopPos, m_MapZoom);
 
    //原表向中间表写
    imageCopy(dstMap, dRect.toQRect(), m_Map, sRect.toQRect());
    QSize s_Size(sRect.w, sRect.h);
    s_Size *= m_MapZoom;
    QRect spRect = sRect.toQRect();//从地图图片上剪裁的区域

    if (m_Times & 4)
    {
        //在上次点击处画菱形y=ax+b x=(y-b)/a
        PAL_Rect r = imageToLabelRect(m_MapPixSelectRect, m_RightTopPos, m_MapZoom);
        m_MapPixSelectRect.toQRect().contains(m_lastScreenPoint);
        DrawingRhombShape(&dstMap, r.w, r.h, r.x, r.y, qRgb(255, 255, 255), 5);
    }
    m_ImageLabel.setPixmap((dstMap));
    PAL_Rect& r = sRect;
    double miniZoom = 256.0 / m_Map.width();
    QPixmap mini = QPixmap::fromImage(m_Map).scaled(m_Map.size() * miniZoom, Qt::KeepAspectRatio);
    DrawSquare(&mini, (r.w * miniZoom), (r.h * miniZoom), r.x * miniZoom, r.y * miniZoom, qRgb(255, 255, 255), 2);
    m_MiniMapLable.setPixmap(mini);

    QString text;
    text =QString::asprintf("放大倍率 %2.3F 原点位置:(%3d,%3d)滚动条位置(%3d,%3d)图显示大小 (%4d,%4d)底层 %3.3d 顶层 %3.3d 障碍 %s,已修改 %d 处,",
        m_MapZoom, m_RightTopPos.x(), m_RightTopPos.y(),
        m_hScrollBar->value(), m_vScrollBar->value(), sRect.w, sRect.h,
        (int)Tile_B(m_MapSelectedTile), (int)Tile_T(m_MapSelectedTile),
        m_MapSelectedTile & 0x2000 ? "√" : "×",
        m_UndoCount);
    QString t;
    for (int i = 3; i < 10; i++)
        if (m_Flags & (1 << i))
        {
            t = QString::asprintf(pMap_Menu_Str[i], m_ListSelectRow);
            t = "双击修改: " + t.mid(2);
        }
    text += t;
    m_tEdit.setText(text);
    //在地图上画上次点击处
}


//
void CMapEdit::mouseDoubleClickEvent(QMouseEvent* event)
{
    QPoint pos = event->pos();;
    QRect r;
    int sx{ -10 }, sy{ -10 }, sh{ -10 };
    if ((r = m_ImageLabel.geometry()).contains(pos) && !m_ImageList->isHidden())
    {
        m_lastScreenPoint = pos - QPoint(m_ImageLabel.geometry().x(), m_ImageLabel.geometry().y());
        m_MapPixSelectRect = mapTitleRectFromPos(m_lastScreenPoint);
        auto  tpos = labelPosToImage(m_lastScreenPoint, m_RightTopPos, m_MapZoom);
        m_MapSelectedTile = FindTileCoor(tpos.x(), tpos.y(), sx, sy, sh);
    }
    else
    { 
        QDialog::mouseDoubleClickEvent(event);
        return; 
    }
    WORD tTop = PAL_Y(m_MapSelectedTile), tBottom = PAL_X(m_MapSelectedTile);

    switch (m_Flags ^ 7)
    {
    case		map_AddObstacle://添加障碍
        tBottom |= 0x2000;
        break;
    case 		map_ClearObstacle://清除障碍
        if (tBottom & 0x2000)
            tBottom ^= 0x2000;
        break;
    case		map_ClearBottom://清除底层
        tBottom &= 0x2000;
        break;
    case		map_ClearTop://清除上层
        tTop = 0;
        break;
    case		map_ReplaceBottom: //	使用%3.3d替换底层
        tBottom &= 0xffff2000;
        tBottom |= m_ListSelectRow & 0xff;
        tBottom |= (m_ListSelectRow & 0x100) << 4;
        break;
    case		map_ReplaceTop: //	使用%3.3d替换上层
        tTop |= m_ListSelectRow & 0xff;
        tTop |= (m_ListSelectRow & 0x100) << 4;
        break;

    default:
        break;
    }
    MAPUNDO sUndo;
    sUndo.tPos = (sx << 1) + sh + (sy << 16);
    sUndo.tOld = m_MapSelectedTile;
    sUndo.tNew = PAL_XY(tBottom, tTop);
    //检查修改
    if (sUndo.tOld == sUndo.tNew)
        return;
    m_Pal->m_pPalMap->MapTiles[sy][sx][sh] = sUndo.tNew;
    doAddUndo(sUndo);
    //更新屏幕底图
    MakeMapImage(m_Map,m_Flags);
}

