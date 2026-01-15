#pragma once
#include "cpixedit.h"
class NoPackedPict_Dlg :
    public CPixEdit
{
    Q_OBJECT;
public:
    NoPackedPict_Dlg(CGetPalData* pPal, QWidget* para = nullptr);
    ~NoPackedPict_Dlg() {};
    virtual void drawAllMap() override {};
protected:
    virtual void doUndo()override {};
    virtual void doRedo()override {};
};

