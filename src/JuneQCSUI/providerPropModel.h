#pragma once

#include <QAbstractItemModel>
#include <QVariant>
#include <QStyledItemDelegate>
#include "ProcessParameter.h"

#include "interfaces/type_usings.h"

using COLOR_TRIPLET = LandaJune::Parameters::ProcessParameter::COLOR_TRIPLET;
using COLOR_TRIPLET_SINGLE = LandaJune::Parameters::ProcessParameter::COLOR_TRIPLET_SINGLE;
using PARAM_GROUP_HEADER = LandaJune::Parameters::ProcessParameter::PARAM_GROUP_HEADER;

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
	
	void setupModelData(LandaJune::IPropertyList, bool readOnly );

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
	LandaJune::IPropertyTuple propertyValue(const ProviderPropsItem *child) const noexcept;
	
	COLOR_TRIPLET createColorTriplet(const ProviderPropsItem *item) const noexcept;
	COLOR_TRIPLET_SINGLE createColorTripletSingle(const ProviderPropsItem *item) const noexcept;
		
	ProviderPropsItem *insertChild(ProviderPropsItem *parent, const QString &name, const QVariant &value) noexcept;
	
	// Setup custom type
	void setupColorTripletSingle(ProviderPropsItem *parent, const LandaJune::IPropertyTuple &prop) noexcept;
	void setupColorTriplet(ProviderPropsItem *parent, const LandaJune::IPropertyTuple &prop) noexcept;
	ProviderPropsItem * setupGroupHeader(ProviderPropsItem *parent, const LandaJune::IPropertyTuple &prop) noexcept;

	ProviderPropsItem *getItem(const QModelIndex &index) const;
	ProviderPropsItem *_rootItem;
	ProviderPropsItem * _currentRoot;

	QIcon _iconInt, _iconFloat, _iconRect, _iconBoolean, _iconLiteral, _iconData;

	bool _readOnlyView = true;

};
