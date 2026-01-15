#pragma once
#include "../main/cpalbase.h"
#include "../main/cpaldata.h"
#include "../main/cpalevent.h"
#include "../main/cscript.h"
#include "../main/palsurface.h"
#include <cassert>
#include <QMessageBox>
#include <map>
#include <qdialog.h>
#include <qevent.h>
#include <QGuiApplication.h>
#include <qlistwidget.h>
//#include <QOpenGLWidget>
#include <qscreen.h>
#include <qtextedit.h>
#include <qtimer.h>
#include <qwidget.h>
#include <qwindow.h>

using  MAPScript = std::map<WORD, WORD>;

class CGetPalData;

enum ragModifRecord
{
	modRoc_abcMkf,
	modRoc_ballMkf,
	modRoc_dataMkf,
	modRoc_fMkf,
	modRoc_fbpMkf,
	modRoc_fireMkf,
	modRoc_gopMkf,
	modRoc_mapMkf,
	modRoc_mgoMkf,
	modRoc_rgmMkf,
	modRoc_sssMkf,
	modRoc_mMsg,
	modRoc_wordDat,
	modRoc_descDat,
	modRoc_End,
};

struct  gMARK {
	WORD row{}; //所在表格的行
	char save{};//存储
	char from{}; //来源 1 = 对象 2 = 场景 3 = 事件对象 4 脚本
	char col{};//位置

	gMARK(const WORD& row, char save, char from, char col)
		: row(row), save(save), from(from), col(col)
	{
	}
};
struct	sgMark { std::vector<gMARK> s; };
using  PMARK = std::vector<sgMark>;
extern LPCSTR PalFileName[];

struct PAL_Surface:public SDL_Surface
{
	PAL_Surface() = delete;
	PAL_Surface(QImage& s) {
		memset(this, 0, sizeof(SDL_Surface));
		w = s.width(); h = s.height(); pitch = s.bytesPerLine(); pixels = s.bits();
		flags = 0;
	}
} ;


class PalQRect : public PAL_Rect  
{  
public:  
  PalQRect() : PAL_Rect() {}  
  PalQRect(int ax, int ay, int aw, int ah) : PAL_Rect(ax, ay, aw, ah) {}  
  PalQRect(const QRect& r) : PAL_Rect(r.x(), r.y(), r.width(), r.height()) {}  
  PalQRect(const PAL_Rect& r) : PAL_Rect(r.x, r.y, r.w, r.h) {} // Add this constructor  
  QRect toQRect() const { return QRect(x, y, w, h); }  
};

typedef enum tagObject_class
{
	kIsPlayer = (1 << 0),
	kIsItem = (1 << 1),
	kIsMagic = (1 << 2),
	kIsEnemy = (1 << 3),
	kIsPoison = (1 << 4)
} Object_Class;


class CGetPalData:public QObject
{
	Q_OBJECT
signals:

private:
	//INT isUseYj_1{-1};
	QTextEdit* m_sEdit{};
	CGetPalData() = delete;
public:
	INT nEnemy{};
	INT nItem{};
	INT nPoisonID{};
	INT nMagic{};
	INT gpObject_classify[MAX_OBJECTS]{ 0 };
	INT ModifRecord[modRoc_End]{0};
	PMARK pMark;//调用脚本地址表,二维结构，第一维指向脚本入口，第二维，调用该入口的对象地址
	CScript* pal{};
public:

	CGetPalData(int save, BOOL noRun,const CPalData* pf = nullptr);

	~CGetPalData();;

	// 封装显示中文消息框的函数，返回值为按钮位置  0 ，1，2 ……
	static int showChineseMessageBox(QWidget* parent,
		const QString& title,
		const QString& text,
		QMessageBox::Icon icon = QMessageBox::Information,
		const QStringList& buttons = QStringList());


	//返回是否有修改数据
	bool isSaveDataChaged() const;
	VOID clearSaveDataChangd();

	static QVector<uint> sdl_PaltteToQColorList(SDL_Color* colors);
	static uint sdl_colorToUint(const SDL_Color&);
	static PalErr setCorrectDir(std::string& dir,bool change = false );
	LPPALMAP m_pPalMap{};

	PalErr EncodeRLE(const void* Source, const UINT8 TransparentColor, INT32 Stride, INT32 Width, INT32 Height, void*& Destination, INT32& Length);
	VOID PAL_Object_classify(VOID);
	//标记脚本跳转地址
	//Mark The script jumps to the address
	//输入 跳转地址引用
	//返回下一跳地址，为0 结束，jumps 不为0返回三个跳转地址
	INT MarkSprictJumpsAddress(WORD spriptEntry, MAPScript& mMaps);

	INT PAL_MarkScriptAll();

	//标记具体的有效脚本地址
	//参数1 入口，参数2 要标记的脚本号列表
	VOID PAL_MarkScriptEntryAll(WORD Entry, const gMARK& mark, int save);
	// 替换mkf文件中的一个片段，
	int replaceMKFOne(ByteArray* f, int nNum, LPCVOID buf, int BufLen);
	//
	PalErr SingleScriptChange(int sOld, int sNew,MAPScript & st);
	//测试文件流是否使用YJ_!压缩
	INT isUseYJ_1(const ByteArray& fp);
	//
	void doSaveMapTilesToArray(int iMapNum);
	//重写函数
	void PAL_MapBlitToSurface(LPCPALMAP lpMap, QImage& m, const SDL_Rect* lpSrcRect, BYTE ucLayer)
	{
		PAL_Surface s(m);
		pal->PAL_MapBlitToSurface(lpMap, &s, lpSrcRect, ucLayer);
	}
	
	PalErr PAL_RLEBlitToSurface(LPCBITMAPRLE lpBitmapRLE,
		QImage& m, PAL_POS pos, BOOL bShadow = FALSE, BOOL d = TRUE)
	{
		PAL_Surface s(m);
		return pal->PAL_RLEBlitToSurface(lpBitmapRLE, &s, pos, bShadow, d);
	}
	
	VOID saveGameDataToCache();

	//初始化压缩标志
	void set_CompressFlag(int f);
	VOID Utf8ToSys(std::string& s);
	PalErr backupFile();
	PalErr restoreFile();
	PalErr delBackupFile();
	PalErr saveDataFile();

};

//运行并测试
class testRun :public QDialog
{
	Q_OBJECT
private:
	QTextEdit			m_sEdit;
	QListWidget			m_sList;
	CPalData*			p_PalData{};
	QTimer* timer{};
public:
	testRun(CPalData* p, QWidget* para = nullptr);
	~testRun();


protected:
	void resizeEvent(QResizeEvent* event) override;
	void closeEvent(QCloseEvent* event) override;
	//void reject() override {}
	void keyPressEvent(QKeyEvent* event)override;
	void onListItemChanged(QListWidgetItem* item);
	void runTest();
};

