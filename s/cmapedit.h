#pragma once
#include <qtableview.h>
#include <vector>
#include "cviewmodel.h"
#include "CPixEdit.h"
#include "cgetpaldata.h"
#include <Windows.h>
#include <cstdint>
#include <qabstractitemmodel.h>
#include <qpoint.h>
#if QT_VERSION >= QT_VERSION_CHECK( 6,0, 0)
#include <qtmetamacros.h>
#endif
#include <qevent.h>
#include <qimage.h>
#include <qwidget.h>
#include "t_data.h"


enum tagMapFlags
{
	map_Bottom = 1 << 0,//下层标识
	map_Top = 1 << 1,//上层标识
	map_Barrier = 1 << 2,//障碍标识
	map_AddObstacle = 1 << 3,//添加障碍
	map_ClearObstacle = 1 << 4,//清除障碍
	map_ClearBottom = 1 << 5,//清除底层
	map_ClearTop = 1 << 6,//清除上层
	map_ReplaceBottom = 1 << 7, //	使用%3.3d替换底层
	map_ReplaceTop = 1 << 8, //	使用%3.3d替换上层
};


typedef struct ragUndo
{
	uint32_t tPos{};
	uint32_t tOld{};
	uint32_t tNew{};
}MAPUNDO;

typedef std::vector<MAPUNDO> MapUndoArray;

typedef std::vector<INT32>TestData;

class PAL_Rect;

class CMapEdit :public CPixEdit
{
	Q_OBJECT
protected:
	CViewModel* m_ImageModel{};//图片数据模型
	T_DATA s_Data;//地图列表数据
	T_DATA s_ImageData;//图片列表数据
	BOOL    m_NoDrawPoint{};//不显示上次点击
	PAL_Rect m_MapRect;//地图数据显示区域
	int     m_MapEdited;//已经选择编辑的地图号
	MapUndoArray m_undoArray;

protected:
	virtual void mouseDoubleClickEvent(QMouseEvent* event)override;
	virtual void resizeEvent(QResizeEvent* event)override;
	void init(int map);
	//生成地图
	int MakeMapImage(QImage& m, const DWORD flag);
	//根据几个关键变量绘制大地图和小地图
	// Plot large and small maps based on a few key variables
    void drawAllMap()override;
	//弹出菜单设置
	void ClickedRightSlot(QPoint pos);
	//点击地图表
	void mapListClicked(const QModelIndex& index);
	//undo
	virtual void doUndo()override;
	virtual void doRedo()override;
	void doAddUndo(MAPUNDO &);
	//更新地图，
	virtual int doUpdateMap() override;
public:
	CMapEdit(CGetPalData* pPal,QWidget * para = nullptr);
	virtual ~CMapEdit();
signals:
	void RightButtonClicked(QPoint point);

};
