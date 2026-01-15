#pragma once

#include "CPixEdit.h"
#include "t_data.h"
class PackedPict_Dlg : public CPixEdit
{
    Q_OBJECT;
public:
    PackedPict_Dlg(CGetPalData* pPal, std::string fName, QWidget* para = nullptr);
    virtual ~PackedPict_Dlg();
protected:
    virtual void doUndo()override {};
    virtual void doRedo()override {};
    virtual void drawAllMap() override;
    virtual void mousePressEvent(QMouseEvent* event) override ;
    
    void listSelected(const QModelIndex & index) { listSelect(index.row()); };
    void listSelect(int row,int refresh = 0);
    void makeImage();
    void ClickedRightSlot(QPoint pos);
    PalErr TestSaverDir(QString& dir);
    void ObjectPopMenuRetuen(UINT msg);
    PalErr makeChunkfromAll(QByteArray& chunk, QByteArray& mapChunk);
    void init();
    //void saveImage(QImage& image, QString &file);
protected:
    BOOL	OldCompressMode{};
    ByteArray fBuf;
    ByteArray gopBuf;
    ByteArray* lpfBuf{};
    ByteArray* lpGopBuf{};
    int m_UseYj1{ -1 };
    int m_Type{};//文件类型标识,1 单图，不压缩 2地图 3 多图压缩 4，fbp 压缩
    T_DATA s_Data{};
    INT		m_ItemW{};//栏宽,,设置与栏高相等
    INT		m_ItemH{};//栏高
    INT		m_colCount{};//栏数
    INT		m_itemCount{};
    INT     m_notEditable{};//不可编辑
    QString m_fileName;//路径名
    std::string m_fileDir;//路径名
    QString m_fileBaseName;
    QString m_file;//完整文件名

signals:
    void SendMessages(const QString& msg);//发送信号到 m_Edit
};
