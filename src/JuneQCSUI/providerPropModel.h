#pragma once

#include <QAbstractItemModel>
#include <QVariant>
#include <QStyledItemDelegate>
#include "baseparam.h"

class ProviderPropsItem;


class ComboBoxItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	ComboBoxItemDelegate(QObject* parent = 0);
	~ComboBoxItemDelegate();

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
	virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
};

class ProvidePropsModel : public QAbstractItemModel
{
    Q_OBJECT

public:
	ProvidePropsModel(QObject *parent = 0);
    ~ProvidePropsModel();
	
	void setupModelData(LandaJune::Parameters::IPropertyList);

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::EditRole) override;

    bool insertColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex()) override;
    bool insertRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex()) override;

signals:

	void propChanged(QString propName, const QVariant& newVal);


private:
 
	ProviderPropsItem *getItem(const QModelIndex &index) const;
	ProviderPropsItem *_rootItem;
};
