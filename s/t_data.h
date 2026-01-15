#pragma once

#include "../main/command.h"
#include <vector>
#include <functional>
#include <variant>
#include <qpixmap.h>


//using namespace std;
// 输入整数返回QString类型
//typedef std::function<std::string(int)>intToStr;
//输入整数，返回字符串
typedef std::function<QString(int i)>SrcToStr;
//输入字符串，返回整数
typedef std::function<int(const char *)>StrToSrc;
//编辑栏返回值校验，
typedef std::function<int(QString, int)>chuckRT;
//输入整数，返回Qimage类型，用于显示图像列表
typedef std::function<QPixmap(int)>intToQPixmap;
//表栏操作控制类型
typedef enum tagCellCTRL
{
	ctrl_Null = 0,//什么都不做
	ctrl_Fix,//列头固定(永远显示)
	ctrl_Edit,//Edit
	ctrl_Combo,//下拉框
	ctrl_List,//,listbox
	ctrl_Radio,//单选框
	//ctrl_MapEdit,//地图编辑
	//ctrl_InScript,//用于显示被调用脚本的次数
	ctrl_Pixmap,
}col_CTRL;
//表数据类型
typedef enum tagColtype
{
	tINT = 0,
	tWORD,//无符号
	tDWORD,//长无符号
	tSHORT,//短整数
	tHEX,//16进制数
	tFLOAT,//浮点数
	tSTR,//字符串
	tBOOL,//布尔数
	tIMAGE,//图像
	tNull,//同整数
	tNOTHING//空
}col_TYPE;

//下拉框单行数据结构
typedef struct tagSelect_Item_Data
{
	std::string s;
	int item;
	int row;
}Select_Item_Data;
//下拉框结构数组

typedef std::vector <Select_Item_Data> select_Item_ColArray;
//下拉框数据结构
typedef  struct  tagSelect_Item
{
	select_Item_ColArray data;
	int row = 0;
}*pSelect_Item;

typedef std::vector<tagSelect_Item>SELECT_ITEN_ARRAY;
typedef struct tagColData //每一栏属性结构
{
	QString ColTop;//表头文字 
	WORD ColWidth{ 0 };//列宽
	WORD Col{ 0 };//列号
	col_CTRL Ctrl{};//0 什么都不做，1，列头固定(永远显示)，2，Edit 3，下拉框 4,listbox 5 单选框 6 图片
	col_TYPE DataStat{};//原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串 6 布尔数
	pSelect_Item p_CtrlList = nullptr;//下拉框数据索引
	SrcToStr  f_InCol{ nullptr };//指向函数的指针，将源数据转化为显示数据字串
	StrToSrc  f_OutCol{ nullptr };//指向函数的指针，将显示字串转化为源
	INT  m_isScript{ 0 };//列标识，标识列是否是脚本
	chuckRT f_chuck{ nullptr };//输入校验函数指针
	intToQPixmap f_Pixmap{ nullptr };//图片生成函数指针
	//输入：表头文字 ;
	// 列宽;
	// 表头序号；
	// 控制：0 什么都不做，1，列头固定(永远显示)，2，Edit 3，下拉框 4,listbox 5 单选框
	//下拉数据索引；
	//函数指针将源数据转化为显示数据字串；
	//指向函数的指针，将显示字串转化为源
	//m_isScript = 1 时是脚本号 = 2时是脚本调用次数
    void GetData(LPCSTR t_ColTop, WORD Width, WORD t_col, col_CTRL t_Ctrl,
		col_TYPE  t_DataStat, pSelect_Item t_CtrlData = nullptr, SrcToStr t_SrcToStr = nullptr,
		StrToSrc t_StrToSrc = nullptr, INT isScript = 0)
	{
		ColTop = t_ColTop;
		ColWidth = Width; Col = t_col; Ctrl = t_Ctrl; DataStat = t_DataStat;
		p_CtrlList = t_CtrlData, f_InCol = t_SrcToStr; f_OutCol = t_StrToSrc;
		m_isScript = isScript;
	}

}COLDATA;

typedef std::vector<COLDATA> ColArray;//表头数据数组
using COLVAR = std::variant<int, double, std::string>;//一栏数据，
using  COLVARLIST = std::vector<COLVAR>;//由一行栏数据结成的数组

//表行数据结构，存一行数据
typedef struct tagColumnClass
{
	COLVARLIST ColVarList;
	int  oldRow{ -1 };//插入时缺省，删除时记录旧行号
}ColumnClass, * LPColumnClass;

typedef struct tagUnDoData
{
	ColumnClass undoCellData; // 记录一栏或一行重做数据
	ColumnClass redoCellData; // 记录一栏或一行恢复数据
	int uRow{ 0 };
	int uCol{ -1 };
	//int rOldRow{ -1 };
}UnDoData;


typedef std::vector<ColumnClass>DataArray;//表数据数组结构


//设置行更新时的操作函数指针
typedef VOID(*SET_COL_DATA)(ColumnClass* d, int row);

class T_DATA//与主表和模型共享数据结构
{
public:
	ColArray  c_Array;//表栏结构数组
	DataArray d_Array;//表数据数组
	std::vector<UnDoData> u_Array;//用于恢复和重做操作的数组
	int  UndoCount{};//undo 计数
	SET_COL_DATA Set_Col_Data{}; //行更新时的操作函数指针,将特定的列更新为指定的值
	//将原数据转换成显示字符串，格式化
	QVariant framCellData(int row, int col, int role = 0);
};
typedef class T_DATA * LPT_DATA;

Q_DECLARE_METATYPE(T_DATA*)
Q_DECLARE_METATYPE(pSelect_Item)
