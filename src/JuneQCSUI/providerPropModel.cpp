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

	_iconInt = QIcon(":/JuneUIWnd/Resources/integer.png");
	_iconFloat = QIcon(":/JuneUIWnd/Resources/float.png");
	_iconRect = QIcon(":/JuneUIWnd/Resources/rect.png");
	_iconBoolean = QIcon(":/JuneUIWnd/Resources/boolean.png");
	_iconLiteral = QIcon(":/JuneUIWnd/Resources/literal.png");
	_iconData = QIcon(":/JuneUIWnd/Resources/data.png");;
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
	
	const auto item = static_cast<ProviderPropsItem*>(index.internalPointer());
	auto var = item->data(1);
	const auto tName = QString(var.typeName());
	
	if (role == Qt::FontRole)
	{
		if (tName == "LandaJune::Parameters::ProcessParameter::PARAM_GROUP_HEADER")
		{
			QFont font;
			font.setBold(true);
			return font;
		}
	}

	if (role == Qt::BackgroundColorRole)
	{
		if (tName == "LandaJune::Parameters::ProcessParameter::PARAM_GROUP_HEADER")
		{
			return QColor(160, 160, 160);
		}
	}

	if (role == Qt::TextColorRole)
	{
		if (tName == "LandaJune::Parameters::ProcessParameter::PARAM_GROUP_HEADER")
		{
			return QColor(255, 255, 255);
		}
	}

	if (role == Qt::DecorationRole && index.column() == 0 )
	{
		if ( tName == "LandaJune::Parameters::ProcessParameter::PARAM_GROUP_HEADER")
		{
			return QVariant();
		}
		if (tName == "bool")
		{
			return _iconBoolean;
		}
		if (tName == "int")
		{
			return _iconInt;
		}
		if (tName == "double" || tName == "float")
		{
			return _iconFloat;
		}
		if (tName == "QString")
		{
			return _iconLiteral;
		}
		if (tName == "QRect")
		{
			return _iconRect;
		}
		return _iconData;
	}

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

	const auto pItem = getItem(index);
    return pItem->data(index.column());
}

Qt::ItemFlags ProvidePropsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.column() == 0)
        return QAbstractItemModel::flags(index);
	
	if ( _readOnlyView )
		return QAbstractItemModel::flags(index);

	const auto item = static_cast<ProviderPropsItem*>(index.internalPointer());
	auto var = item->data(1);
	const auto tName = QString(var.typeName());

	if (tName == "LandaJune::Parameters::ProcessParameter::PARAM_GROUP_HEADER")
	{
		return QAbstractItemModel::flags(index);
	}

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

	const auto childItem = getItem(index);
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
		
		const auto prop = propertyValue(item);
		
		auto&[name, var, editable] = prop;
		emit propChanged(name, var);
	}

    return true;
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

void ProvidePropsModel::setupModelData(LandaJune::IPropertyList propList, bool readOnly)
{
	_readOnlyView = readOnly;
	const auto& numRows = _rootItem->childCount();
	if (numRows)
		removeRows(0, _rootItem->childCount());

	_currentRoot = _rootItem;
	for (const auto& propTuple : propList)
	{
		auto&[name, var, editable] = propTuple;


		if ( 
				(_readOnlyView && editable)
			||  (!_readOnlyView && !editable)
			)
		{
			continue;
		}

		if (var.canConvert<PARAM_GROUP_HEADER>())
		{
			auto chItem = setupGroupHeader(_rootItem, propTuple);
			_currentRoot = chItem;
			continue;
		}


		if (var.canConvert<COLOR_TRIPLET>())
		{
			setupColorTriplet(_currentRoot, propTuple);
		} 
		else if (var.canConvert<COLOR_TRIPLET_SINGLE>()) 
		{
			setupColorTripletSingle(_currentRoot, propTuple);
		}
		else 
		{
			insertChild(_currentRoot, name, var);
		}
	}
}

LandaJune::IPropertyTuple ProvidePropsModel::propertyValue(const ProviderPropsItem *child) const noexcept
{
	if (child->parent()->parent() == _rootItem) 
	{
		return { child->data(0).toString(), child->data(1), _readOnlyView };
	}

	auto *topPropertyItem = child;
	while (topPropertyItem->parent() != _currentRoot)
	{
		topPropertyItem = topPropertyItem->parent();
	}

	const auto value = [this, topPropertyItem]() 
	{
		if (const auto data = topPropertyItem->data(1); topPropertyItem->data(1).canConvert<COLOR_TRIPLET>()) 
		{
			return QVariant::fromValue(createColorTriplet(topPropertyItem));
		} 
		else if (data.canConvert<COLOR_TRIPLET_SINGLE>()) 
		{
			return QVariant::fromValue(createColorTripletSingle(topPropertyItem));
		}
		
		return topPropertyItem->data(1);
	}();

	return { topPropertyItem->data(0).toString(), value, _readOnlyView };
}

COLOR_TRIPLET ProvidePropsModel::createColorTriplet(const ProviderPropsItem *item) const noexcept
{
	const auto min = createColorTripletSingle(item->child(0));
	const auto max = createColorTripletSingle(item->child(1));
	return { min, max, item->data(0).toString().toStdString() };
}

COLOR_TRIPLET_SINGLE ProvidePropsModel::createColorTripletSingle(const ProviderPropsItem *item) const noexcept
{
	const auto color = item->child(0)->data(1).toString();
	const auto h = item->child(1)->data(1).toInt();
	const auto s = item->child(2)->data(1).toInt();
	const auto v = item->child(3)->data(1).toInt();

	return COLOR_TRIPLET_SINGLE { h, s, v };
}


ProviderPropsItem *ProvidePropsModel::insertChild(ProviderPropsItem *parent, const QString &name, const QVariant &value) noexcept
{
	const int count = parent->childCount();
	parent->insertChildren(count, 1, parent->columnCount());
	
	auto child = parent->child(count);
	child->setData(0, name);
	child->setData(1, value);

	return child;
}

ProviderPropsItem * ProvidePropsModel::setupGroupHeader(ProviderPropsItem* parent, const LandaJune::IPropertyTuple &prop) noexcept
{
	auto&[name, var, editable] = prop;
	const auto groupName = var.value<PARAM_GROUP_HEADER>();
	return insertChild(parent, groupName._groupName, var);
}


void ProvidePropsModel::setupColorTripletSingle(ProviderPropsItem *parent, const LandaJune::IPropertyTuple &prop) noexcept
{
	auto&[name, _colorVar, editable] = prop;

	const auto color = _colorVar.value<COLOR_TRIPLET_SINGLE>();
	auto child = insertChild(parent, name, _colorVar);

	insertChild(child, "H", color._iH);
	insertChild(child, "S", color._iS);
	insertChild(child, "V", color._iV);
}

void ProvidePropsModel::setupColorTriplet(ProviderPropsItem *parent, const LandaJune::IPropertyTuple &prop) noexcept
{
	auto&[name, _colorVar, editable] = prop;

	const auto colorTriplet = _colorVar.value<COLOR_TRIPLET>();
	auto child = insertChild(parent, QString::fromStdString(colorTriplet._colorName), _colorVar);

	setupColorTripletSingle(child, { "Min", QVariant::fromValue(colorTriplet._min), editable });
	setupColorTripletSingle(child, { "Max", QVariant::fromValue(colorTriplet._max), editable });
}
