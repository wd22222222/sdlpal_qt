#pragma once
//cviewmodel.h
#include <string>
#include <vector>
#include <variant>
#include <functional>
#include <QAbstractTableModel>
#include "t_data.h"

class CViewModel : public QAbstractTableModel
{
	Q_OBJECT
	//表数据模型
private:
	T_DATA* t_pData{};//指向表数据的指针
public:
	CViewModel(QObject* parent = 0);;
	virtual ~CViewModel() {};
public:
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;;
	virtual QVariant data(const QModelIndex& index, int role) const override;;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;;
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
	//返回表头数据
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	//
	virtual int insertRow(int row, const QModelIndex& aparent = QModelIndex()) ;
	virtual int removeRow(int row, const QModelIndex& aparent = QModelIndex()) ;
	//引入数据
	void set_t_Data(T_DATA* t_Data);
	//返回数据 供代理使用
	T_DATA& get_t_Data();;

private:
	//根据定义的转换规则，并将QString 转换成COLVAR
	COLVAR getCellData(QString val,int n_col);//col所在的列
	COLVAR getCellData(QString val, COLDATA & t);//t 格式
};


//model 供listedit 使用
class CEditlistModel : public  QAbstractTableModel
{
	Q_OBJECT
	pSelect_Item m_List;
public:
	CEditlistModel(pSelect_Item list,  QObject* parent = 0)
		: QAbstractTableModel(parent),m_List(list) {};
	virtual ~CEditlistModel() {};
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;;
	virtual QVariant data(const QModelIndex& index, int role) const override;;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;;

};

