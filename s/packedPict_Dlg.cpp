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

#define NOMINMAX

#include <algorithm>
#include <qfileinfo.h>
#include <qfiledialog.h>
#include <qmenu.h>
#include <qevent.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <filesystem>
#include <cctype>
#include "packedpict_dlg.h"
#include "mainwindow.h"
#include <cscapi.h>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QRegExp>
#else
#include <QRegularExpression>
#define QRegExp QRegularExpression
#endif


static LPCSTR pMenu_Str[] =
{
	"1.存储本组图片到目录",
	"2.载入并替换本组图片",
	"3.载入图片到尾部",
	nullptr,
};

static INT(*getFileChunkSize)(UINT uiChunkNum,const ByteArray& fp);
static ByteArray (*getFChunk)(UINT chunk,const ByteArray& fp);

#ifdef _WIN32
std::string toLower(const std::string& str) {
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return result;
}
#endif

//比较两个目录 ，相同返回真
static bool comparePathsPlatformAware(const std::string& path1, const std::string& path2) {
	namespace fs = std::filesystem;
	fs::path p1 = fs::absolute(path1).lexically_normal();
	fs::path p2 = fs::absolute(path2).lexically_normal();

#ifdef _WIN32
	// Windows下路径不区分大小写
	return toLower(p1.string()) == toLower(p2.string());
#else
	// Linux/macOS下路径区分大小写
	return p1 == p2;
#endif
}
PackedPict_Dlg::PackedPict_Dlg(CGetPalData* pPal, std::string fName, QWidget* para)
	:CPixEdit(para)
{
	m_Pal = pPal;
	m_Pal->pal->PAL_SetPalette(0, 0);
	m_file = fName.c_str();
	QFileInfo fInfo(m_file);
	m_fileDir = fInfo.path().toStdString();
	m_fileName = fInfo.fileName();
	m_fileBaseName = fInfo.baseName();
	QString title = QString::asprintf("编辑文件 %s", fName.c_str());
	setWindowTitle(title);
	if (!comparePathsPlatformAware(m_fileDir + "/",m_Pal->pal->PalDir))
		m_notEditable = 1;
	if (m_notEditable)
	{

		lpfBuf = &fBuf;
		lpGopBuf = &gopBuf;
		if (m_Pal->pal->gpGlobals->loadData(fName, *lpfBuf))//打开文件名指定的文件
		{
			QString msg = QString("打开文件错，无法打开文件 %1").arg(fName.c_str());
			SendMessages(msg);
			close();
		};

		//导入地图图片文件
		QString gopName = QString::asprintf("%s/gop.mkf", m_fileDir.c_str());
		if (m_fileBaseName == "map") {
			if (m_Pal->pal->gpGlobals->loadData(gopName.toStdString(), *lpGopBuf))
			{
				QString msg = QString("打开文件错，无法编辑文件 %1").arg(gopName);
				SendMessages(msg);
				close();
			}
		}
	}
	else
	{
		//tr("对象图像 (mgo.mkf);;敌方战斗图像 (abc.mkf);;我方战斗图像(f.mkf);;魔法特效(fire.mkf);;背景图片 (fbp.mkf)"
		//";;头像 (rgm.mkf);;对象图片 (ball.mkf);;地图文件 (map.mkf);;All Files(*.*)")
		auto fileName = m_fileBaseName.toStdString();
		if (fileName == "ball") lpfBuf = &m_Pal->pal->gpGlobals->f.fpBALL;
		else if (fileName == "rgm")lpfBuf = &m_Pal->pal->gpGlobals->f.fpRGM;
		else if (fileName == "f")lpfBuf = &m_Pal->pal->gpGlobals->f.fpF;
		else if (fileName == "fire")lpfBuf = &m_Pal->pal->gpGlobals->f.fpFIRE;
		else if (fileName == "mgo")lpfBuf = &m_Pal->pal->gpGlobals->f.fpMGO;
		else if (fileName == "abc")lpfBuf = &m_Pal->pal->gpGlobals->f.fpABC;
		else if (fileName == "fbp")lpfBuf = &m_Pal->pal->gpGlobals->f.fpFBP;
		else if (fileName == "map")
		{
			lpGopBuf = &m_Pal->pal->gpGlobals->f.fpGOP;
			lpfBuf = &m_Pal->pal->gpGlobals->f.fpMAP;
		}
		else assert(false);
	}
	//标识文件类型
	if (m_fileName == "ball.mkf" || m_fileName == "rgm.mkf")
	{
		//处理非压缩文件
		m_Type = 1;
		getFChunk = &CScript::PAL_MKFReadChunk;
		getFileChunkSize = CScript::PAL_MKFGetChunkSize;
	}
	else if (m_fileName == "fbp.mkf")
	{
		m_Type = 4;
		getFChunk = CScript::PAL_MKFDecompressChunk;
		getFileChunkSize = CScript::PAL_MKFGetDecompressedSize;
		m_Pal->set_CompressFlag(m_Pal->isUseYJ_1(*lpfBuf));
	}
	else if (m_fileName == "map.mkf")
	{
		//地图图像 map gop文件
		m_Type = 2;
		getFChunk = &CScript::PAL_MKFReadChunk;
		getFileChunkSize = &CScript::PAL_MKFGetChunkSize;
		m_Pal->set_CompressFlag(m_Pal->isUseYJ_1(*lpfBuf));
	}
	else 
	{
		//压缩图像
		m_Type = 3;
		getFChunk = &CScript::PAL_MKFDecompressChunk;
		getFileChunkSize = &CScript::PAL_MKFGetDecompressedSize;
		m_Pal->set_CompressFlag(m_Pal->isUseYJ_1(*lpfBuf));
	}
	init();
	listSelect(0,1);
}

PackedPict_Dlg::~PackedPict_Dlg()
{
	//将压缩标志复位
	m_Pal->set_CompressFlag( -1);
}

void PackedPict_Dlg::drawAllMap()
{
	m_Pal->pal->PAL_SetPalette(0, 0);
	if (m_NoDraw)
		return;
	//大窗口
	auto sz = m_ImageLabel.geometry().size();
	auto image = QPixmap::fromImage(m_Map).scaled(sz);
	m_ImageLabel.setPixmap(image);
	//小窗口
	if (m_Type != 3)
		m_MiniMapLable.setPixmap(QPixmap::fromImage(m_Map).scaled(256,256));
	else
	{
		if (!m_itemCount) m_itemCount = 1;
		int i = (m_Times >> 3) % m_itemCount;
		QPixmap m(256, 256);
		imageCopy(m, QRect(0, 0, 256, 256), m_Map, QRect((i % m_colCount) * m_ItemW, (i / m_colCount) * m_ItemH, m_ItemW, m_ItemH));
		m_MiniMapLable.setPixmap(m);
	}
	return;
}

void PackedPict_Dlg::mousePressEvent(QMouseEvent* event)
{
	QPoint pos;
	QRect r;
	if ((r = m_ImageLabel.geometry()).contains(pos = event->pos()) && !m_NoDraw)
		if (event->button() == Qt::RightButton)
		{
			//右键按下：
			emit RightButtonClicked(pos);
		}

	QWidget::mousePressEvent(event);
}

void PackedPict_Dlg::listSelect(int row,int refresh)//refresh 强制刷新
{
	if (m_ListSelectRow == row && !refresh)
		return;
	m_ListSelectRow = row;
	if ((getFileChunkSize)(row, *lpfBuf) == 0)
	{
		m_Map = QImage(256, 256, QImage::Format_RGB32);
		m_Map.fill({0,0,0,255});
		m_ImageLabel.setPixmap(QPixmap::fromImage(m_Map).
			scaled({2048,2048}));
		m_MiniMapLable.setPixmap(QPixmap::fromImage(m_Map));
		m_NoDraw = 1;
		return;
	}
	m_NoDraw = 0;
	makeImage();
}

void PackedPict_Dlg::makeImage()
{
	auto pal = MainWindow::pal()->pal;
	auto colorFormat = QImage::Format_RGB32;
	int width{}, height{};
	int count{ 1 };
	int magnification = 0;//放大系数
	if (m_ListSelectRow > pal->PAL_MKFGetChunkCount(*lpfBuf))
		m_ListSelectRow = 0;
    ByteArray buf = (getFChunk)(m_ListSelectRow, *lpfBuf);
	if (buf.empty())
	{
		m_Map.fill({0,0,0,255});
		m_ImageLabel.setPixmap(QPixmap::fromImage(m_Map).
			scaled(m_ImageLabel.geometry().size()));
		m_MiniMapLable.setPixmap(QPixmap::fromImage(m_Map).
			scaled(m_MiniMapLable.geometry().size()));
		m_NoDraw = 1;
		return;
	}
	m_NoDraw = 0;
	switch (m_Type)
	{
	case 1://非压缩单图
	{
		width = pal->PAL_RLEGetWidth((LPCBITMAPRLE)buf.data());
		height = pal->PAL_RLEGetHeight((LPCBITMAPRLE)buf.data());
		if (width < 80 && height < 80)
			magnification = 0;
		else if (width < 160 && height < 160)
			magnification = 1;
		else
			magnification = 2;
		m_ItemH = m_ItemW = 80 << magnification;
		QImage m(m_ItemW, m_ItemH, QImage::Format_Indexed8);
		m.setColorTable(m_Pal->sdl_PaltteToQColorList( m_Pal->pal->gpPalette->colors));
		m.fill(m_Pal->sdl_colorToUint( m_Pal->pal->gpPalette->colors[0]));
		int x = (m_ItemW - width) >> 1;
		int y = (m_ItemH - height) >> 1;
		m_Pal->PAL_RLEBlitToSurface((LPCBITMAPRLE)buf.data(), m, PAL_XY(x, y));
		m_Map = m.convertToFormat(colorFormat).scaled(320,200,Qt::KeepAspectRatio);
		break;
	}
	case 3://压缩多图
	{
		m_itemCount = pal->PAL_SpriteGetNumFrames((LPCSPRITE)buf.data());
		//取最大尺寸
		for (int i = 0; i < m_itemCount; i++)
		{
			LPCBITMAPRLE lpFrame = pal->PAL_SpriteGetFrame((LPCSPRITE)buf.data(), i);
			if (!lpFrame)
				continue;
            width = std::max((int)width, (int)pal->PAL_RLEGetWidth(lpFrame));
            height = std::max((int)height, (int)pal->PAL_RLEGetHeight(lpFrame));
		}
		if (width < 4 && height < 4)
		{
			m_NoDraw = 1;
			return;
		}
		else if (width < 80 && height < 80)
			magnification = 0;
		else if (width < 160 && height < 160)
			magnification = 1;
		else
			magnification = 2;
		m_ItemH = m_ItemW = 80 << magnification;
		if (m_itemCount < 5) m_colCount = 2;
		else if (m_itemCount < 10)m_colCount = 3;
		else if (m_itemCount < 16)m_colCount = 4;
		else m_colCount = 5;
		int rowCount = (m_itemCount + m_colCount - 1) / m_colCount;
		if (rowCount < m_colCount)rowCount = m_colCount;
		QImage m(m_ItemW * m_colCount, m_ItemH * rowCount, QImage::Format_Indexed8);
		m.setColorTable(m_Pal->sdl_PaltteToQColorList( m_Pal->pal->gpPalette->colors));
		m.fill(0);

		for (int i = 0; i < m_itemCount; i++)
		{
			int col = i % m_colCount;
			int row = i / m_colCount;
			//if (row < col)row = col;
			LPCBITMAPRLE lpFrame = pal->PAL_SpriteGetFrame((LPCSPRITE)buf.data(), i);
			if (!lpFrame)
				continue;
			width = pal->PAL_RLEGetWidth((LPCBITMAPRLE)lpFrame);
			height = pal->PAL_RLEGetHeight((LPCBITMAPRLE)lpFrame);
			int x = (m_ItemW - width) >> 1;
			int y = (m_ItemH - height) >> 1;
			DWORD pos = PAL_XY(col * m_ItemW + x, row * m_ItemH + y);

			m_Pal->PAL_RLEBlitToSurface(lpFrame, m, (PAL_POS)pos);
			//m_Map = std::move(m);
		}
		m_Map = m.convertToFormat(colorFormat);
		break;
	}
	case 4://单个大图320*200
	{
		m_ItemW = width = 320;
		m_ItemH = height = 200;
		if (buf.size() < 64000)
		{
			m_NoDraw = 1;
			return;
		}
		QImage m(m_ItemW, m_ItemH, QImage::Format_Indexed8);
		m.setColorTable(m_Pal->sdl_PaltteToQColorList( m_Pal->pal->gpPalette->colors));
		m.fill(0);
		memcpy(m.bits(), buf.data(), 64000);//直接拷贝
		m_Map = m.convertToFormat(colorFormat);
		break;
	}
	case 2://地图
	{
		auto &gopBuf = *lpGopBuf;
		QImage m = QImage(MapPixWidth, MapPixHight, QImage::Format_Indexed8);
		m.setColorTable(m_Pal->sdl_PaltteToQColorList( m_Pal->pal->gpPalette->colors));
		m.fill(0);
		m_Pal->m_pPalMap = pal->PAL_LoadMap(m_ListSelectRow, *lpfBuf, *lpGopBuf);
		if (!m_Pal->m_pPalMap)
		{
			m_NoDraw = 1;
			return;
		}
		PAL_Rect rect = { 0,0,MapPixWidth,MapPixHight };
		//画底层
		m_Pal->PAL_MapBlitToSurface(m_Pal->m_pPalMap, m, &rect, 0);
		//画顶层
		m_Pal->PAL_MapBlitToSurface(m_Pal->m_pPalMap, m, &rect, 1);
		m_Map = m.convertToFormat(colorFormat);
		//画障碍
		DrawObstacles(m_Map, &rect);
		m_ItemW = m_ItemH = MapPixHight;
		break;
	}
	default:
		break;
	}
}

void PackedPict_Dlg::ClickedRightSlot(QPoint pos)
{
	QMenu* pMenu = new QMenu(&m_ImageLabel);
	pMenu->setContextMenuPolicy(Qt::CustomContextMenu);
	for (int i = 0; pMenu_Str[i]; i++)
	{
		QString text;
		text = QString::asprintf(pMenu_Str[i]);

		auto act = pMenu->addAction(text);
		if (m_notEditable && i > 0)
			act->setEnabled(FALSE);
	}
	connect(pMenu, &QMenu::triggered, this, [&](QAction* action) {
		QString s = action->text();
		int msg = s.left(1).toInt() - 1;
		ObjectPopMenuRetuen(msg);
		});

	// 右键菜单显示的地方
	pMenu->exec(pos);
	delete pMenu;

}
//检查存储图像文件目录dir，不存在建立之， 返回 0 成功 其他失败
PalErr PackedPict_Dlg::TestSaverDir(QString& mDir)
{
	QString d = QString::asprintf("%s%s", m_Pal->pal->PalDir.c_str(), "image/");  
	QDir().mkdir(d);
	if (!CPalData::IsDirExist(d.toStdString()))
		return 1; // Failure  
	QString dir = d + mDir;
	QDir().mkdir(dir);
	if (!CPalData::IsDirExist(dir.toStdString()))
		return 1; // Failure  
	mDir = std::move(dir);
	return 0;
}

void PackedPict_Dlg::ObjectPopMenuRetuen(UINT msg)
{
	//弹出菜单返回值
	switch (msg)
	{
	case 0:
	{
		//L"存储本组图片到目录",同时存储打包块
		//建立目录
		QString sDir = QString::asprintf("%s%3.3d/", m_fileBaseName.toStdString().c_str(), m_ListSelectRow);
		if (TestSaverDir(sDir))
			return;
        ByteArray vChunk = (getFChunk)(m_ListSelectRow, *lpfBuf);
		LPBYTE lpSprite =(LPBYTE) vChunk.data();

		switch (m_Type)
		{
		case 3://压缩多图
		{
			for (int n = 0; n < m_itemCount; n++)
			{
				int width = m_Pal->pal->PAL_RLEGetWidth(m_Pal->pal->PAL_SpriteGetFrame(lpSprite, n));
				int height = m_Pal->pal->PAL_RLEGetHeight(m_Pal->pal->PAL_SpriteGetFrame(lpSprite, n));

				QImage m(width, height, QImage::Format_Indexed8);

				m.setColorTable(m_Pal->sdl_PaltteToQColorList( m_Pal->pal->gpPalette->colors));
				m.fill(m_Pal->sdl_colorToUint(m_Pal->pal->gpPalette->colors[0]));
				//写临时表数据
				//生成临时表面
				LPCBITMAPRLE lpFrame = m_Pal->pal->PAL_SpriteGetFrame(lpSprite, n);
				m_Pal->PAL_RLEBlitToSurface(lpFrame, m, 0);

				QString savename = QString::asprintf("%s%3.3d.bmp", sDir.toStdString().c_str(),n);
				m.save(savename, "BMP");
				//m_Map.save(savename, "BMP");
				//saveImage(m, savename);
			}
			break;
		}
		case 2://地图
			//分两部分，地图数据，地图图像
		{
			//图像
			auto count = *(WORD*)m_Pal->m_pPalMap->pTileSprite;
			if (count == 0)
				return;
			for (int i = 0; i < count; i++)
			{
				QString saveImage = QString::asprintf("%s%3.3d.bmp", sDir.toStdString().c_str(), i);
				QImage m(32, 15, QImage::Format_Indexed8);
				m.setColorTable(m_Pal->sdl_PaltteToQColorList( m_Pal->pal->gpPalette->colors));
				m.fill(m_Pal->sdl_colorToUint( m_Pal->pal->gpPalette->colors[0]));
				auto lpSprite = m_Pal->pal->PAL_SpriteGetFrame((LPBYTE)m_Pal->m_pPalMap->pTileSprite, i);
				if (lpSprite)
				{
					m_Pal->PAL_RLEBlitToSurface(lpSprite, m, 0);
				}
				m.save(saveImage,"BMP");
			}
			QString saveMap = QString::asprintf("%s%s", sDir.toStdString().c_str(),"map");
			auto s = (LPBYTE)m_Pal->m_pPalMap->MapTiles;
			QFile f(saveMap);
			f.open(QIODevice::WriteOnly);
			f.write((LPCSTR)s,sizeof(m_Pal->m_pPalMap->MapTiles));
			f.close();
			break;
		}
		case 1://非压缩单图
		{
			int width = m_Pal->pal->PAL_RLEGetWidth((LPCBITMAPRLE)lpSprite);
			int height = m_Pal->pal->PAL_RLEGetHeight((LPCBITMAPRLE)lpSprite);
			QImage m(width, height, QImage::Format_Indexed8);
			m.setColorTable(m_Pal->sdl_PaltteToQColorList( m_Pal->pal->gpPalette->colors));
			m.fill(m_Pal->sdl_colorToUint( m_Pal->pal->gpPalette->colors[0]));
			m_Pal->PAL_RLEBlitToSurface((LPCBITMAPRLE)lpSprite, m, 0);
			QString fname = QString::asprintf("%s000.bmp", sDir.toStdString().c_str());
			m.save(fname, "BMP");
			break;
		}
		case 4://单一大图320*200非压缩，尺寸64000 BYTE
		{
			QImage m(320, 200, QImage::Format_Indexed8);
			m.setColorTable(m_Pal->sdl_PaltteToQColorList( m_Pal->pal->gpPalette->colors));
			memcpy(m.bits(), lpSprite, 64000);
			QString fname = QString::asprintf("%s000.bmp", sDir.toStdString().c_str());
			m.save(fname, "BMP");
			break;
		}
		default:
			break;
		}
		break;
	}

	case 1:
	{
		//L"载入并替换本组图片"	
		QByteArray vChunk;
		QByteArray mapChunk;
		if (makeChunkfromAll(vChunk,mapChunk))
			return;//失败
		if (m_Type == 2)//地图
		{
			m_Pal->replaceMKFOne(lpfBuf, m_ListSelectRow, mapChunk.data(), mapChunk.length());
			m_Pal->replaceMKFOne(lpGopBuf, m_ListSelectRow, vChunk.data(), vChunk.length());
			m_Pal->ModifRecord[modRoc_fMkf] = TRUE;//标记改变
			m_Pal->ModifRecord[modRoc_gopMkf]= TRUE;//标记改变
		}
		else
		{
			m_Pal->replaceMKFOne(lpfBuf, m_ListSelectRow, vChunk.data(), vChunk.length());
			m_Pal->ModifRecord[modRoc_fMkf] = TRUE;//标记改变
		}
		//m_isUpdate = true;
		init();
		listSelect(m_ListSelectRow, 1);
		break;
	}
	case 2:
	{
		//L"载入图片到尾部"
		QByteArray vChunk;
		QByteArray mapChunk;
		if (makeChunkfromAll(vChunk, mapChunk))
			return;
		if (m_Type == 2)//地图
		{
			m_Pal->replaceMKFOne(lpfBuf, 99999, mapChunk.data(), mapChunk.length());
			m_Pal->replaceMKFOne(lpGopBuf, 99999, vChunk.data(), vChunk.length());
			m_Pal->ModifRecord[modRoc_fMkf] = TRUE;//标记改变
			m_Pal->ModifRecord[modRoc_gopMkf] = TRUE;//标记改变
		}
		else
		{
			m_Pal->replaceMKFOne(lpfBuf, 99999, vChunk.data(), vChunk.length());
			m_Pal->ModifRecord[modRoc_fMkf] = TRUE;//标记改变
		}
		//m_isUpdate = true;
		init();
		listSelect(m_ListSelectRow, 1);
		break;
	}
	default:
		break;
	}

}
//功能生成图像压缩片断
// 输入：压缩片断存储结构引用，打开文件结构指针
// 返回 成功返回0 不成功返回其他 成功时返回值在chunk中。
PalErr PackedPict_Dlg::makeChunkfromAll(QByteArray& chunk, QByteArray& mapChunk)
{
	std::string d = CPalEvent::va("%s%s", m_Pal->pal->PalDir.c_str(), "image/");
	//CFileDir f;
	if (!CPalData::IsDirExist(d))
		return 1;//失败
    QString sDir = QString("%1%2???").arg( d.c_str()).arg(m_fileBaseName);
	QString NewDir = QFileDialog::getExistingDirectory(nullptr, "打开目录", sDir);
	if (NewDir.isEmpty())
		return 1;
	if (NewDir.right(m_fileBaseName.size() + 3).left(m_fileBaseName.size())
		!= m_fileBaseName)
	{
		QString s = QString::asprintf("选择的目录 %1 不含有指定文件").arg(NewDir);
		QMessageBox::warning(this, "警告！", s, QMessageBox::Abort);
		return 1;
	}
	int chunkCount{};
	{
		QDir dir(NewDir);
		QStringList fileNames = dir.entryList(QDir::Files); // 获取文件夹中的所有文件名
		QRegExp re("[0-9]+$");
		for (int i = 0; i < fileNames.size(); i++)
		{
			if (fileNames.at(i).left(3).contains(re))
				chunkCount++;
		}
	}

	switch (m_Type)
	{
	case 1://非压缩单图,使用rle压缩
	{
		QImage m;
		auto s = QString::asprintf("%1/000.bmp").arg(NewDir);
		m.load(s, "BMP");
		int len{};
		LPVOID buf{};
		if (m_Pal->EncodeRLE(m.bits(), 0, m.bytesPerLine(), m.width(), m.height(), buf, len))
			return 1;
		chunk.resize(len);
		memcpy(chunk.data(), buf, chunk.size());
		free(buf);
		break;
	}
	case 2://地图
	case 3://压缩多图
	{
		chunk.resize(0x100000);//最大长度
		LPWORD lpTop = (LPWORD)chunk.data();
		int off = (chunkCount + 1) << 1;
		for (int i = 0; i < chunkCount; i++)//使用RLE压缩
		{
			auto v = QString::asprintf("%s/%3.3d.bmp", NewDir.toStdString().c_str(), i);
			QImage m;
			if (!m.load(v, "BMP"))
				break;
			int len{};
			LPVOID buf{};
			if (m_Pal->EncodeRLE(m.bits(), 0, m.bytesPerLine(), m.width(), m.height(), buf, len))
				return 1;
			lpTop[i] = (WORD)off >> 1;
			memcpy(chunk.data() + (lpTop[i] << 1), buf, len);
			off += len;
			lpTop[i + 1] = (WORD)off >> 1;
			free(buf);
		}
		chunk.resize(off);
		if (m_Type == 2)//载入Map数据并进行压缩
		{
			auto  s = QString::asprintf("%1/map").arg(NewDir);
			ByteArray b;
			m_Pal->pal->gpGlobals->loadData(s.toStdString().c_str(), b);
			//压缩
			UINT32 compressLen{};
			VOID* vBuf{};
			if (m_Pal->pal->gpGlobals->EnCompress(b.data(), b.size(), vBuf, compressLen, 0))
				return 1;
			mapChunk.resize(compressLen);
			memcpy(mapChunk.data(), vBuf, compressLen);
			free(vBuf);
		}
		else
		{
			//对数据进行压缩
			UINT32 compressLen{};
			VOID* vBuf{};
			if (m_Pal->pal->gpGlobals->EnCompress(chunk.data(), chunk.size(), vBuf, compressLen, 0))
				return 1;
			chunk.resize(compressLen);
			memcpy(chunk.data(), vBuf, compressLen);
			free(vBuf);
		}
		break;
	}
	case 4://单一大图320*200非压缩，尺寸64000 BYTE
	{
		QImage m;
		auto s = QString::asprintf("%1/000.bmp").arg(NewDir);
		m.load(s, "BMP");
		UINT32 compressLen{};
		VOID* vBuf{};
		if (m_Pal->pal->gpGlobals->EnCompress(m.bits(), 64000, vBuf, compressLen, 0))
			return 1;
		chunk.resize(compressLen);
		memcpy(chunk.data(), vBuf, compressLen);
		free(vBuf);
		break;
	}
	default:
		break;
	}

	return 0;
}

void PackedPict_Dlg::init()
{
	//生成列表
	const int totalCol = m_Type == 3 ? 3 : 2;
	auto& w_ColData = s_Data.c_Array;
	w_ColData.resize(totalCol);
	w_ColData[0].GetData("序号", 60, 0, ctrl_Null, tNull);
	w_ColData[1].GetData("大小", 60, 1, ctrl_Null, tNull);
	if (m_Type == 3)
		w_ColData[2].GetData("图片数", 60, 2, ctrl_Null, tSHORT);

	//数据
	DataArray& s_RowData = s_Data.d_Array;
	int count = m_Pal->pal->PAL_MKFGetChunkCount(*lpfBuf);
	s_RowData.resize(count);
	for (int i = 0; i < count; i++)
	{
		s_RowData[i].ColVarList.resize(totalCol);
		s_RowData[i].ColVarList[0] = i;
		s_RowData[i].ColVarList[1] = getFileChunkSize(i, *lpfBuf);
		if (m_Type == 3)
		{
            ByteArray buf = getFChunk(i, *lpfBuf);
			s_RowData[i].ColVarList[2] = buf.empty() ? 0 : 
				(m_Pal->pal->PAL_SpriteGetNumFrames)((LPSPRITE)buf.data());
		}
	}
	if (!m_ListModel)
		delete m_ListModel;
	m_ListModel = new CViewModel;
	m_ListModel->set_t_Data(&s_Data);
	m_List->setModel(m_ListModel);
	m_List->setColumnWidth(0, 50);
	m_List->setColumnWidth(1, 60);
	if (m_Type == 3)
		m_List->setColumnWidth(2, 50);
	m_List->setCurrentIndex(m_ListModel->index(m_ListSelectRow, 0));

	QObject::connect(m_List->selectionModel(), &QItemSelectionModel::selectionChanged,
		[&](const QItemSelection& selected, const QItemSelection& deselected)
		{
			auto i = selected.indexes();
			if (i.isEmpty())
				return;
			listSelect(i[0].row());
		});
	connect(this, &CPixEdit::RightButtonClicked, this, &PackedPict_Dlg::ClickedRightSlot);
}
