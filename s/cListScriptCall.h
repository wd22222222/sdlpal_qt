#pragma once
#include <qtableview.h>
#include  "t_data.h"

class CListScriptCall:public QTableView
{
    Q_OBJECT;

    class MainWindow* m_parat{};
    class CViewModel* m_Model{};
    class T_DATA s_Data;
public:
    CListScriptCall(class MainWindow* para, unsigned short scriptEntry);;
	virtual ~CListScriptCall();;

	void focusOutEvent(QFocusEvent* event) override;;
	void keyPressEvent(QKeyEvent* event)override;;
};

