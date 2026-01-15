#pragma once
#include <qtableview.h>

class CPopView;

class CPopView :public QTableView
{
	Q_OBJECT;
public:
    CPopView(QWidget * pare = nullptr);

    void focusOutEvent(QFocusEvent* event) override;
	QString Text;
	const QString text() { return Text; };
};
//Q_DECLARE_METATYPE(const CPopView*)
