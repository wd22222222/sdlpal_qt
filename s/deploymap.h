#pragma once

#include "cpixedit.h"
#include "t_data.h"

typedef struct ragSceneUndo
{
    DWORD tPos;
    EVENTOBJECT tOld;
    EVENTOBJECT tNew;
}SceneUndo;
typedef std::vector<SceneUndo> MapSceneUndoArray;

class deployMap :
    public CPixEdit
{
    Q_OBJECT;

    INT            iMapNum{ 0 };//地图号
    INT			   iScene{ 0 }; //场景号
    T_DATA         s_Data;      //地图列表数据
    DWORD          m_FixedPoint;//不动点，在窗口中始终显示
    QPoint         m_OriginPoint;//原点，原图显示在左上角的点   
    INT            m_TriggerMode{ 0 };
	T_DATA*        m_pData{ nullptr }; //数据指针
    MapSceneUndoArray m_UndoArray;
 
protected:
    //生成地图
    INT MakeMapImage(QImage& m, const DWORD flag);
    //生成全部
    void makeALL();
    //重画屏幕
    virtual void drawAllMap();
    void listSelected(int row);
    virtual int doUpdateMap() override;
public:
    deployMap(int scene,T_DATA* data , QWidget* para);
    virtual ~deployMap();
protected:
    virtual void doUndo()override;
    virtual void doRedo()override;
    void ClickedRightSlot(const QPoint pos);
    virtual void resizeEvent(QResizeEvent* event)override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event)override;
};

