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

#include "cpopview.h"
#include <qheaderview.h>

CPopView::CPopView(QWidget* pare)
	:QTableView(pare)
{
	verticalHeader()->hide();//表头表隐藏表头
	setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑 
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//隐藏滚动条水平
	setSelectionBehavior(QAbstractItemView::SelectRows);//选中行
	setSelectionMode(QAbstractItemView::SingleSelection);//单选
	viewport()->stackUnder(this);//浮在最上面
	setWindowFlags(Qt::FramelessWindowHint);//隐藏标题等
	//
}

void CPopView::focusOutEvent(QFocusEvent* event)
{
	// 调用父类的 focusOutEvent 实现
	QTableView::focusOutEvent(event);
	this->hide();
}
