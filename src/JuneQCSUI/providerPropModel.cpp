#include <QtWidgets>
#include <QComboBox>

#include "providerPropItem.h"
#include "providerPropModel.h"

using namespace LandaJune::Parameters;

ComboBoxItemDelegate::ComboBoxItemDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{
}


ComboBoxItemDelegate::~ComboBoxItemDelegate()
{
}


QWidget* ComboBoxItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	// ComboBox ony in column 2
	if (index.column() != 1)
		return QStyledItemDelegate::createEditor(parent, option, index);

	// Create the combobox and populate it
	QComboBox* cb = new QComboBox(parent);
	int row = index.row();
	cb->addItem(QString("one in row %1").arg(row));
	cb->addItem(QString("two in row %1").arg(row));
	cb->addItem(QString("three in row %1").arg(row));
	return cb;
}


void ComboBoxItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	if (QComboBox* cb = qobject_cast<QComboBox*>(editor)) {
		// get the index of the text in the combobox that matches the current value of the itenm
		QString currentText = index.data(Qt::EditRole).toString();
		int cbIndex = cb->findText(currentText);
		// if it is valid, adjust the combobox
		if (cbIndex >= 0)
			cb->setCurrentIndex(cbIndex);
	}
	else {
		QStyledItemDelegate::setEditorData(editor, index);
	}
}


void ComboBoxItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	if (QComboBox* cb = qobject_cast<QComboBox*>(editor))
		// save the current text of the combo box as the current value of the item
		model->setData(index, cb->currentText(), Qt::EditRole);
	else
		QStyledItemDelegate::setModelData(editor, model, index);
}


ProvidePropsModel::ProvidePropsModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QVector<QVariant> rootData;
	rootData << "Property" << "Value";

	_rootItem = new ProviderPropsItem(rootData);
}

ProvidePropsModel::~ProvidePropsModel()
{
    delete _rootItem;
}


int ProvidePropsModel::columnCount(const QModelIndex & /* parent */) const
{
    return _rootItem->columnCount();
}

QVariant ProvidePropsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

	const auto item = getItem(index);
    return item->data(index.column());
}

Qt::ItemFlags ProvidePropsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.column() == 0)
        return Qt::NoItemFlags;
	return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

ProviderPropsItem *ProvidePropsModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) 
	{
		const auto item = static_cast<ProviderPropsItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return _rootItem;
}

QVariant ProvidePropsModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return _rootItem->data(section);

    return QVariant();
}

QModelIndex ProvidePropsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

	auto parentItem = getItem(parent);
	const auto childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

bool ProvidePropsModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
	beginInsertColumns(parent, position, position + columns - 1);
	const auto success = _rootItem->insertColumns(position, columns);
    endInsertColumns();

    return success;
}

bool ProvidePropsModel::insertRows(int position, int rows, const QModelIndex &parent)
{
	auto parentItem = getItem(parent);
	beginInsertRows(parent, position, position + rows - 1);
	const auto success = parentItem->insertChildren(position, rows, _rootItem->columnCount());
    endInsertRows();
    return success;
}

QModelIndex ProvidePropsModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

	auto childItem = getItem(index);
	const auto parentItem = childItem->parent();

    if (parentItem == _rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool ProvidePropsModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
	beginRemoveColumns(parent, position, position + columns - 1);
	const auto success = _rootItem->removeColumns(position, columns);
    endRemoveColumns();

    if (_rootItem->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

bool ProvidePropsModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    ProviderPropsItem *parentItem = getItem(parent);

	beginRemoveRows(parent, position, position + rows - 1);
	const auto success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int ProvidePropsModel::rowCount(const QModelIndex &parent) const
{
    return getItem(parent)->childCount();
}

bool ProvidePropsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

	auto item = getItem(index);
	const auto result = item->setData(index.column(), value);

	if (result)
	{
		emit dataChanged(index, index);
		emit propChanged(item->data(0).toString(), value);
	}

    return result;
}

bool ProvidePropsModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

	const auto result = _rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

void ProvidePropsModel::setupModelData(LandaJune::IPropertyList propList )
{
	const auto& numRows = _rootItem->childCount();
	if (numRows)
		removeRows(0, _rootItem->childCount());

	for (const auto& prop : propList)
	{
		_rootItem->insertChildren(_rootItem->childCount(), 1, _rootItem->columnCount());
		_rootItem->child(_rootItem->childCount() - 1)->setData(0, prop.first);
		_rootItem->child(_rootItem->childCount() - 1)->setData(1, prop.second);
	}
}
