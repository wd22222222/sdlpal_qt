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
#include <qscrollbar.h>
#include <qtimer.h>
#include <qevent.h>
#include <qmessagebox.h>
#include "CPixEdit.h"
#include "cviewmodel.h"

using namespace std;
//选中的行始终保持高亮
void CPixEdit::setSelectKeepHighlighted(QTableView* table)
{
    QPalette p = palette();
    p.setColor(QPalette::Inactive, QPalette::Highlight, p.color(QPalette::Active, QPalette::Highlight));
    p.setColor(QPalette::Inactive, QPalette::HighlightedText, p.color(QPalette::Active, QPalette::HighlightedText));
    table->setPalette(p);
}

void CPixEdit::keyPressEvent(QKeyEvent* event)
{
    if (event->matches(QKeySequence::Undo))
        doUndo();//撤消
    else if (event->matches(QKeySequence::Redo))
        doRedo();//重做
    else if (event->matches(QKeySequence::Cancel))
        close();//
    else
        QDialog::keyPressEvent(event);
}

void CPixEdit::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    int cx = event->size().width();
    int cy = event->size().height();
    if (!m_List)
        return;
    m_List->resize(256, cy - 260);
    m_List->move(4, 4);
    m_vScrollBar->resize(20, cy - 60);
    m_vScrollBar->move(cx - 30, 4);
    m_hScrollBar->resize(cx - 266 - 30, 20);
    m_hScrollBar->move(266, cy - 60);
    m_MiniMapLable.resize(256, 256);
    m_MiniMapLable.move(4, cy - 260);
    m_tEdit.resize(cx - 270, 40);
    m_tEdit.move(270, cy - 42);
    m_ImageLabel.resize(cx - 266 - 30, cy - 43 - 25);
    m_ImageLabel.move(266, 4);

}

void CPixEdit::wheelEvent(QWheelEvent* event)
{
    // 输出滚轮滚动的方向和距离
    if (m_ImageLabel.isActiveWindow())
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // Qt6 写法
        QPoint angleDelta = event->angleDelta();
        if (!angleDelta.isNull()) {
            m_MapZoom += 0.0003 * angleDelta.y();
        }
#else
        m_MapZoom += 0.0003 * event->delta();
        return;
#endif
    }
    // 调用父类的wheelEvent来处理默认行为
    QWidget::wheelEvent(event);
}

int CPixEdit::doUpdateMap()
{
    if (!m_UndoCount)
        return -1;
    LPCSTR s = "注意！数据已经修改 保存吗？";
    QStringList buttons = { "是", "否", "取消" };
    int ret = CGetPalData::showChineseMessageBox(nullptr, "请确认", s, QMessageBox::Warning,buttons);

    if (2 == ret)
        return 0;
    if (1 == ret)
        return -1;
    //m_isUpdate = TRUE;
    return 1;//保存数据
}

void CPixEdit::closeEvent(QCloseEvent* event)
{
    if (doUpdateMap())
    {
        event->accept();//默认情况下接收关闭信号，关闭窗体
    }
    else
    {
        event->ignore();//忽略关闭信号，阻止窗体关闭
    }
}


void CPixEdit::scrollValueChanged(int velue, int flags)
{
    //鼠标拖动滚动条，引发值velue flag = 0 横轴 1 竖轴
    if (!flags)
    {
        m_RightTopPos.setY(velue * (MapPixHight - m_ImageLabel.height() * m_MapZoom) / MaxScrollValue);
    }
    else
    {
        m_RightTopPos.setX(velue * (MapPixWidth - m_ImageLabel.width() * m_MapZoom) / MaxScrollValue);
    }
}


void CPixEdit::mousePressEvent(QMouseEvent* event) {
    // 如果是左键点击，发射自定义的 clicked 信号
    QPoint pos;
    QRect r;
    if ((r = m_ImageLabel.geometry()).contains(pos = event->pos()) && !m_NoDraw)
    {
        if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {
            m_lastScreenPoint = pos - QPoint(m_ImageLabel.geometry().x(), m_ImageLabel.geometry().y());
            m_MapPixSelectRect = mapTitleRectFromPos(m_lastScreenPoint);
        }
        if (event->button() == Qt::RightButton)
        {
            //右键按下：
            emit RightButtonClicked(pos);
        }
    }
    QWidget::mousePressEvent(event);
}



//将图片上的区域范围显示出来 p 原点，sWidth sHeight 周边范围,返回需要显示的范围
PalQRect CPixEdit::ImagePointToDraw(DWORD p,int sWidth ,int sHeight)
{
    int x = p >> 1 & 0xff, h = p & 1, y = (p >> 16);
    int dx{ -1 }, dy{ -1 };
    GetMapTilePoint(x, y, h, dx, dy);
    if (dx == -1)return PalQRect();
    /*
    while ((m_lastScreenPoint = imageToLabelPos(QPoint(dx, dy), m_fixPos, m_MapZoom)).x() < (sWidth + 32) / m_MapZoom && m_hScrollBar->value() > 0)
        m_hScrollBar->setValue(m_hScrollBar->value() - 1);
    while (imageToLabelPos(QPoint(dx, dy), m_fixPos, m_MapZoom).x() > m_ImageLabel.geometry().width() - (sWidth + 32) / m_MapZoom && m_hScrollBar->value() < MaxScrollValue)
        m_hScrollBar->setValue(m_hScrollBar->value() + 1);
    while (imageToLabelPos(QPoint(dx, dy), m_fixPos, m_MapZoom).y() < (sHeight + 16) / m_MapZoom && m_vScrollBar->value() > 0)
        m_vScrollBar->setValue(m_vScrollBar->value() - 1);
    while (imageToLabelPos(QPoint(dx, dy), m_fixPos, m_MapZoom).y() > m_ImageLabel.geometry().height() - (sHeight + 16) / m_MapZoom && m_vScrollBar->value() < MaxScrollValue)
        m_vScrollBar->setValue(m_vScrollBar->value() + 1);
    */
    moveDotToMiddle(PAL_XY(dx, dy));
    return PalQRect(dx - sWidth, dy - sHeight, 2 * sWidth + 32, 2 * sHeight + 16);
}

//将图片上的点移动至屏幕中部Move the dot on the image to the middle of the screen
void CPixEdit::moveDotToMiddle(DWORD p)
{
    int dx = PAL_X(p), dy = PAL_Y(p);
    //计算滚动条滚动距离
    int ax = m_ImageLabel.geometry().width() / 2, ay = m_ImageLabel.geometry().height() / 2;//计算屏幕中部位置
    QPoint bPoint = labelPosToImage(QPoint(ax, ay), m_RightTopPos, m_MapZoom) - QPoint(dx, dy);//计算屏幕位置在表上的投影
    int  x = m_hScrollBar->value() - 1.0 / (MapPixWidth - m_ImageLabel.geometry().width() * m_MapZoom ) * (bPoint.x()) * MaxScrollValue;//计算滚动条位置
    int  y = m_vScrollBar->value() - 1.0 / (MapPixHight - m_ImageLabel.geometry().height() * m_MapZoom ) * (bPoint.y()) * MaxScrollValue ;
    x = max(0, x); x = min(x, MaxScrollValue);
    y = max(0, y); y = min(y, MaxScrollValue);
    m_hScrollBar->setValue(x);//移动滚动条
    m_vScrollBar->setValue(y);
    return;
}

//鼠标
void CPixEdit::mouseMoveEvent(QMouseEvent* event)
{
    auto pos = event->pos() - QPoint(m_ImageLabel.geometry().x(), m_ImageLabel.geometry().y());
    if ((m_ImageLabel.geometry()).contains(event->pos()))
        if (pos != m_lastScreenPoint)
        {
            if (event->modifiers() & Qt::ControlModifier)
            {
                //缩放
                if (pos.x() < m_lastScreenPoint.x())
                    m_MapZoom -= 0.01;
                else
                    m_MapZoom += 0.01;
            }
            else
            {
                //移动,修改滚动条的值 通过滚动条控制
                pos -= m_lastScreenPoint;
                if (pos.x())
                    m_hScrollBar->setValue(m_hScrollBar->value() - pos.x() / abs(pos.x()));
                if (pos.y())
                    m_vScrollBar->setValue(m_vScrollBar->value() - pos.y() / abs(pos.y()));
            }
            m_lastScreenPoint = event->pos() - QPoint(m_ImageLabel.geometry().x(), m_ImageLabel.geometry().y());
            m_NoDraw = 1;
        }
    QWidget::mouseMoveEvent(event);
}

//画障碍
VOID CPixEdit::DrawObstacles(QImage& m, const PAL_Rect* lpSrcRect)
{
    int              sx, sy, dx, dy, x, y, h, xPos, yPos;
    //
    // Convert the coordinate
    //
    sy = lpSrcRect->y / 16 - 1;
    dy = (lpSrcRect->y + lpSrcRect->h) / 16 + 2;
    sx = lpSrcRect->x / 32 - 1;
    dx = (lpSrcRect->x + lpSrcRect->w) / 32 + 2;
    yPos = sy * 16 - 8 - lpSrcRect->y;
    for (y = sy; y < dy; y++)
    {
        for (h = 0; h < 2; h++, yPos += 8)
        {
            xPos = sx * 32 + h * 16 - 16 - lpSrcRect->x;
            for (x = sx; x < dx; x++, xPos += 32)
            {
                //
                if (x >= 64 || y >= 128 || h > 1 || x < 0 || y < 0)
                    continue;
                DWORD32 l = m_Pal->m_pPalMap->MapTiles[y][x][h];
                if (l & 0x2000)
                {
                    //用白笔画
                    DrawingRhombShape(m, 32, 16, xPos, yPos, qRgb((int)150, (int)150, (int)150), m_MapZoom < 1.3 ? 2 : 3);
                }
            }
        }
    }
}

CPixEdit::CPixEdit(QWidget* parent)
	:QDialog(parent)
{
    m_List = new QTableView(this);
    m_List->setSelectionMode(QAbstractItemView::SingleSelection);//单选
    m_List->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑 
    m_List->setSelectionBehavior(QAbstractItemView::SelectRows);//选择行
    m_List->verticalHeader()->setVisible(false);//显示表头

    //设置表头选中行关联色,失去焦点选择保持高亮
    setSelectKeepHighlighted(m_List);
    m_tEdit.setFocusPolicy(Qt::NoFocus);//没有焦点
    m_tEdit.setParent(this);
    m_MiniMapLable.setBackgroundRole(QPalette::Base);
    m_MiniMapLable.setParent(this);
    m_ImageLabel.setParent(this);
    m_vScrollBar = new QScrollBar(Qt::Vertical, this);
    m_hScrollBar = new QScrollBar(Qt::Horizontal, this);
    m_vScrollBar->setMaximum(MaxScrollValue);
    m_hScrollBar->setMaximum(MaxScrollValue);
    m_MapZoom = 0.3;
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [&]() {m_Times++; drawAllMap(); });
    timer->start(100); // 每100毫秒刷新一次屏幕

    connect(m_vScrollBar, &QScrollBar::valueChanged, this, [&](int value) {scrollValueChanged(value, 0); });
    connect(m_hScrollBar, &QScrollBar::valueChanged, this, [&](int value) {scrollValueChanged(value, 1); });
}

CPixEdit::~CPixEdit()
{
    delete m_List;
}
