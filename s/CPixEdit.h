#pragma once
#include <qtableview.h>
#include <qdialog.h>
#include <qtextedit.h>
#include <qlabel.h>
#include "cgetpaldata.h"
#include "cviewmodel.h"
#include <qwidget.h>

#define MapPixWidth 2048
#define MapPixHight 2048
#define MaxScrollValue 600

//图片编辑基类
class CPixEdit :
    public QDialog
{
    Q_OBJECT;
protected:
	QTableView* m_List{};//列表
	CViewModel* m_ListModel{};
	QTextEdit	m_tEdit{};//文本区
	QLabel		m_ImageLabel{};//大地图显示
	QLabel		m_MiniMapLable{};//小地图显示
	QImage		m_Map{};//地图完整图像
	QTableView* m_ImageList{};//地图用图片列表

	QScrollBar* m_vScrollBar{};
	QScrollBar* m_hScrollBar{};
	QPoint		m_RightTopPos;//显示左上角的点
	CGetPalData* m_Pal{};
	DWORD		m_Flags{ 0b0111 };//显示标识

	//
	double  m_MapZoom{ 0.3 };//大地图缩放比率 0.2 -1.1
	int     m_Times{};//时间计数器，用于闪动显示
	int		m_NoDraw{};//不画图标志
	PalQRect m_MapPixSelectRect{};//选择地图上的点 title 的范围
	QPoint  m_lastScreenPoint;//上次屏幕点击点
	DWORD   m_MapSelectedTile{ };//上次鼠标点击选择的tile
	UINT 	m_ListSelectRow{ 0xffff };//列表控件选择的行
	int		m_UndoCount{};
protected:
	virtual void keyPressEvent(QKeyEvent* event)override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event)override{};
	virtual void resizeEvent(QResizeEvent* event)override;
	virtual void wheelEvent(QWheelEvent* event)override;
	virtual int	 doUpdateMap();
	virtual void closeEvent(QCloseEvent* event)override;

protected:
	virtual void drawAllMap() = 0;
	virtual void doUndo() = 0;
	virtual void doRedo() = 0;
	virtual void scrollValueChanged(int velue, int flags);
	//选中的行始终保持高亮
	// //Set header to select row color, lose focus to keep it highlighted
	void setSelectKeepHighlighted(QTableView* table);
	//图形拷贝
	void imageCopy(QImage& dst, const QRect& dRect, const QImage& src, const QRect& sRect);
	void imageCopy(QPixmap& dst, const QRect& dRect, const QImage& src, const QRect& sRect);
	void imageCopy(QPixmap& dst, const QRect& dRect, const QPixmap& src, const QRect& sRect);
	//坐标转换，原图片转显示
	PalQRect imageToLabelRect(const PAL_Rect& r, const QPoint& m, const double zoom);;
	PAL_Rect imageToLabelRect(const QRect& r, const QPoint& m, const double zoom);;
	//坐标转换，显示转图片
	PalQRect labelRectToImage(const PalQRect& r, const QPoint& m, double zoom);;
	PAL_Rect labelRectToImage(const QRect& r, const QPoint& m, double zoom)
	{
		return labelRectToImage(PalQRect(r), m, zoom);
	};
	//点转换 图片转显示
	QPoint imageToLabelPos(const QPoint& r, const QPoint& m, const double zoom)
	{
		int x = r.x(), y = r.y();
		x = (x - m.x()) / zoom; y = (y - m.y()) / zoom;
		return QPoint(x, y);
	};
	//点转换 显示转图片
	QPoint labelPosToImage(const QPoint& r, const QPoint& m, const double zoom)
	{
		int x = r.x(), y = r.y();
		x = x * zoom + m.x(); y = y * zoom + m.y();
		return QPoint(x, y);
	};
	// 画菱型Drawing rhomb shape
// 画菱形图块的边界，菱形起点为矩形的边界线段中点
//输入：图像结构指针，矩形宽度，矩形高度，线颜色=白色，线宽度=1
//返回，0 失败，非零成功
	BOOL DrawingRhombShape(QImage& image, int w, int h, int x = 0, int y = 0, QRgb r = qRgb(255, 255, 255), int width = 1);

	BOOL DrawingRhombShape(QPixmap* image, int w, int h, int x = 0, int y = 0, QRgb r = qRgb(255, 255, 255), int width = 1);
	//画矩形
	BOOL DrawSquare(QImage* image, int w, int h, int x = 0, int y = 0, QRgb r = qRgb(255, 255, 255), int width = 1);
	//画矩形
	BOOL DrawSquare(QPixmap* image, int w, int h, int x = 0, int y = 0, QRgb r = qRgb(255, 255, 255), int width = 1);
	void mousePressEvent(QMouseEvent* event) override;
	//通过点击的点，计算显示范围和位置
	PAL_Rect mapTitleRectFromPos(const QPoint& pt)
	{
		//计算在图片内部的实际偏移,移动到相对零点
		auto  pos = labelPosToImage(pt, m_RightTopPos, m_MapZoom);
		int sx{ -10 }, sy{ -10 }, sh{ -10 };
		m_MapSelectedTile = FindTileCoor(pos.x(), pos.y(), sx, sy, sh);
		int dx{ -1 }, dy{ -1 };
		GetMapTilePoint(sx, sy, sh, dx, dy);
		return PAL_Rect(dx, dy, 32, 16);
	}
	//取tile的坐标
//输入：x,y,h 指向坐标的指针
// 返回 1 成功，0 失败
	BOOL GetMapTilePoint(int x, int y, int h, int& dx, int& dy);
	//通过点击的点，计算显示范围和位置
//求dx,dy的title坐标 x,y，h
	DWORD FindTileCoor(int dx, int dy, int& sx, int& sy, int& sh);
	//将图片上的点显示出来
	PalQRect ImagePointToDraw(DWORD p, int sWidth = 0, int sHeight = 0);
	//将图片上的点移动至屏幕中部Move the dot on the image to the middle of the screen
	void moveDotToMiddle(DWORD p);
	//鼠标
	void mouseMoveEvent(QMouseEvent* event)override;
	//画障碍
	VOID DrawObstacles(QImage& m, const PAL_Rect* lpSrcRect);
signals:
	void RightButtonClicked(QPoint point);
public:
	explicit CPixEdit(QWidget* pare = nullptr);
	virtual ~CPixEdit();
};

