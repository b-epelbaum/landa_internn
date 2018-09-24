#pragma once

#include <QAbstractItemModel>
#include <QVariant>
#include <QStyledItemDelegate>
#include "baseparam.h"

#include "common/type_usings.h"

class ParamPropItem;

class ComboBoxItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	ComboBoxItemDelegate(QObject* parent = nullptr);
	~ComboBoxItemDelegate();

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
	virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
};

class ParamPropModel : public QAbstractItemModel
{
    Q_OBJECT

public:
	ParamPropModel(QObject *parent = nullptr);
    ~ParamPropModel();
	
	void setupModelData(LandaJune::IPropertyList, bool readOnly );

	QModelIndexList getPersistentIndexList() const { return persistentIndexList(); }

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

	QString itemType(const QModelIndex& idx ) const;

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

	void copyParam(const QModelIndex &index);
	void removeParam(const QModelIndex &index);

	LandaJune::IPropertyTuple getPropertyTuple(const QModelIndex& idx) const;

	QModelIndexList findItem(QString name) const;

signals:

	void propChanged(QString propName, const QVariant& newVal);
	void propAdd(const QString &propName, const QVariant &newVal);
	void propRemoved(const QString &propName);

private:
	void setupProp(ParamPropItem *rootItem, const LandaJune::IPropertyTuple &propTuple);
	LandaJune::IPropertyTuple propertyValue(ParamPropItem *child) const noexcept;

	QPoint createQPoint(ParamPropItem *item) const noexcept;
	QRect createQRect(ParamPropItem *item) const noexcept;
	QVector<LandaJune::Parameters::COLOR_TRIPLET> createColorTripletVector(ParamPropItem *item) const noexcept;
	LandaJune::Parameters::COLOR_TRIPLET createColorTriplet(ParamPropItem *item) const noexcept;
	LandaJune::Parameters::COLOR_TRIPLET_SINGLE createColorTripletSingle(ParamPropItem *item) const noexcept;
		
	ParamPropItem *insertChild(ParamPropItem *parent, const QString &name, const QVariant &value) noexcept;
	
	// Setup custom type
	void setupQPoint(ParamPropItem *parent, const LandaJune::IPropertyTuple &prop) noexcept;
	void setupQRectVector(ParamPropItem *parent, const LandaJune::IPropertyTuple &prop) noexcept;
	void setupQRect(ParamPropItem *parent, const LandaJune::IPropertyTuple &prop) noexcept;
	void setupColorTripletSingle(ParamPropItem *parent, const LandaJune::IPropertyTuple &prop) noexcept;
	void setupColorTriplet(ParamPropItem *parent, const LandaJune::IPropertyTuple &prop) noexcept;
	void setupColorTripletVector(ParamPropItem *parent, const LandaJune::IPropertyTuple &prop) noexcept;
	ParamPropItem * setupGroupHeader(ParamPropItem *parent, const LandaJune::IPropertyTuple &prop) noexcept;

	ParamPropItem *getItem(const QModelIndex &index) const;
	ParamPropItem *_rootItem = nullptr;
	ParamPropItem * _currentRoot = nullptr;

	QIcon _iconInt, _iconFloat, _iconRect, _iconBoolean, _iconLiteral, _iconData, _iconColors, _iconColorArray;

	bool _readOnlyView = true;

};

Q_DECLARE_METATYPE(std::shared_ptr<LandaJune::Parameters::BaseParameters>)