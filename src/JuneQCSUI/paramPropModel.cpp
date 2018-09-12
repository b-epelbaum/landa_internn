#include <QtWidgets>
#include <QComboBox>

#include "paramPropItem.h"
#include "paramPropModel.h"

const QString GROUP_CLASS_NAME = "LandaJune::Parameters::PARAM_GROUP_HEADER";

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


ParamPropModel::ParamPropModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QVector<QVariant> rootData;
	rootData << "Parameter Name" << "Value";

	_rootItem = new ParamPropItem(rootData);

	_iconInt = QIcon(":/JuneUIWnd/Resources/integer.png");
	_iconFloat = QIcon(":/JuneUIWnd/Resources/float.png");
	_iconRect = QIcon(":/JuneUIWnd/Resources/rect.png");
	_iconBoolean = QIcon(":/JuneUIWnd/Resources/boolean.png");
	_iconLiteral = QIcon(":/JuneUIWnd/Resources/literal.png");
	_iconData = QIcon(":/JuneUIWnd/Resources/data.png");
	_iconColors = QIcon(":/JuneUIWnd/Resources/colors.png");
	_iconColorArray = QIcon(":/JuneUIWnd/Resources/color_arr.png");
}

ParamPropModel::~ParamPropModel()
{
    delete _rootItem;
}


int ParamPropModel::columnCount(const QModelIndex & /* parent */) const
{
    return _rootItem->columnCount();
}

QVariant ParamPropModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
	
	const auto item = static_cast<ParamPropItem*>(index.internalPointer());
	const auto var = item->data(1);
	const auto tName = QString(var.typeName());
	
	if (role == Qt::FontRole)
	{
		if (tName == GROUP_CLASS_NAME)
		{
			QFont font;
			font.setBold(true);
			return font;
		}
	}

	if (role == Qt::BackgroundColorRole)
	{
		if (tName == GROUP_CLASS_NAME)
		{
			return QColor(110, 110, 110);
		}
	}

	if (role == Qt::TextColorRole)
	{
		if (tName == GROUP_CLASS_NAME)
		{
			return QColor(255, 255, 255);
		}

		if (index.column() == 0 )
		{
			return QColor(0, 0, 0);
		}
	}

	if (role == Qt::DecorationRole && index.column() == 0 )
	{
		if ( tName == GROUP_CLASS_NAME)
		{
			return _iconData;
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
		if (tName == "LandaJune::Parameters::COLOR_TRIPLET_SINGLE" || tName == "LandaJune::Parameters::COLOR_TRIPLET")
		{
			return _iconColors;
		}
		if (tName == "QVector<LandaJune::Parameters::COLOR_TRIPLET>")
		{
			return _iconColorArray;
		}
		return _iconData;
	}

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

	if (index.column() == 1) {
		if (var.canConvert<QPoint>()) {
			const auto point = var.toPoint();
			return QString("(%1, %2)").arg(QString::number(point.x()), QString::number(point.y()));
		}
		if (var.canConvert<QRect>()) {
			const auto rect = var.toRect();
			return QString("(%1, %2, %3, %4)").arg(QString::number(rect.x()), QString::number(rect.y())
													, QString::number(rect.width()), QString::number(rect.height()));
		}
		if (var.canConvert<COLOR_TRIPLET>())
			return var.value<COLOR_TRIPLET>().toDisplayString();
		if (var.canConvert<COLOR_TRIPLET_SINGLE>())
			return var.value<COLOR_TRIPLET_SINGLE>().toDisplayString();
	}

	const auto pItem = getItem(index);
    return pItem->data(index.column());
}

Qt::ItemFlags ParamPropModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.column() == 0)
        return QAbstractItemModel::flags(index);
	
	if ( _readOnlyView )
		return QAbstractItemModel::flags(index);

	const auto item = static_cast<ParamPropItem*>(index.internalPointer());
	const auto var = item->data(1);
	
	if (var.canConvert<COLOR_TRIPLET>() || var.canConvert<COLOR_TRIPLET_SINGLE>()
		|| var.canConvert<QVector<COLOR_TRIPLET>>() || var.canConvert<QPoint>()
		|| var.canConvert<QRect>())
		return Qt::ItemIsSelectable;
	
	const auto tName = QString(var.typeName());

	if (tName == GROUP_CLASS_NAME)
	{
		return QAbstractItemModel::flags(index);
	}

	return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}


ParamPropItem *ParamPropModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) 
	{
		const auto item = static_cast<ParamPropItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return _rootItem;
}

QVariant ParamPropModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return _rootItem->data(section);

    return QVariant();
}

QModelIndex ParamPropModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

	auto parentItem = getItem(parent);
	const auto childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

bool ParamPropModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
	beginInsertColumns(parent, position, position + columns - 1);
	const auto success = _rootItem->insertColumns(position, columns);
    endInsertColumns();

    return success;
}

bool ParamPropModel::insertRows(int position, int rows, const QModelIndex &parent)
{
	auto parentItem = getItem(parent);
	beginInsertRows(parent, position, position + rows - 1);
	const auto success = parentItem->insertChildren(position, rows, _rootItem->columnCount());
    endInsertRows();
    return success;
}

QModelIndex ParamPropModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

	const auto childItem = getItem(index);
	const auto parentItem = childItem->parent();

    if (parentItem == _rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

QString ParamPropModel::itemType(const QModelIndex& idx) const
{
	const auto item = static_cast<ParamPropItem*>(idx.internalPointer());
	const auto var = item->data(1);
	return QString(var.typeName());
}

bool ParamPropModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
	beginRemoveColumns(parent, position, position + columns - 1);
	const auto success = _rootItem->removeColumns(position, columns);
    endRemoveColumns();

    if (_rootItem->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

bool ParamPropModel::removeRows(int position, int rows, const QModelIndex &parent)
{
	ParamPropItem *parentItem = getItem(parent);

	beginRemoveRows(parent, position, position + rows - 1);
	const auto success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

void ParamPropModel::copyParam(const QModelIndex &index)
{
	if (index.isValid()) {
		if (const auto item = getItem(index)) {
			const auto data = item->data(1);
			const auto name = item->data(0).toString();

			beginResetModel();
			setupProp(item->parent(), { name, data, true });
			endResetModel();
			emit propAdd(name, data);			
		}
	}
	
}

void ParamPropModel::removeParam(const QModelIndex &index)
{
	if (index.isValid()) {
		if (const auto item = getItem(index)) {
			auto itemParent = item->parent();
			beginResetModel();
			itemParent->removeChild(item);
			endResetModel();
			emit propRemoved(item->data(0).toString());		
		}
	}
}

int ParamPropModel::rowCount(const QModelIndex &parent) const
{
    return getItem(parent)->childCount();
}

bool ParamPropModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

	auto item = getItem(index);
	const auto result = item->setData(index.column(), value);

	if (result)
	{
		emit dataChanged(index, index);
		
		const auto&[name, var, editable] = propertyValue(item);
		emit propChanged(name, var);
	}

    return true;
}

bool ParamPropModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

	const auto result = _rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

void ParamPropModel::setupModelData(LandaJune::IPropertyList propList, bool readOnly)
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
			const auto chItem = setupGroupHeader(_rootItem, propTuple);
			_currentRoot = chItem;
			continue;
		}

		setupProp(_currentRoot, propTuple);
	}
}

void ParamPropModel::setupProp(ParamPropItem *rootItem, const LandaJune::IPropertyTuple &propTuple)
{
	if (!rootItem)
		return;

	const auto&[name, var, editable] = propTuple;
	if (var.canConvert<QPoint>()) {
		setupQPoint(rootItem, propTuple);
	}
	else if (var.canConvert<QVector<QRect>>()) {
		setupQRectVector(rootItem, propTuple);
	}
	else if (var.canConvert<QRect>()) {
		setupQRect(rootItem, propTuple);
	}
	else if (var.canConvert<QVector<COLOR_TRIPLET>>()) {
		setupColorTripletVector(rootItem, propTuple);
	}
	else if (var.canConvert<COLOR_TRIPLET>()) {
		setupColorTriplet(rootItem, propTuple);
	}
	else if (var.canConvert<COLOR_TRIPLET_SINGLE>()) {
		setupColorTripletSingle(rootItem, propTuple);
	}
	else {
		insertChild(rootItem, name, var);
	}
}

LandaJune::IPropertyTuple ParamPropModel::propertyValue(ParamPropItem *child) const noexcept
{
	using namespace LandaJune;
	if (child->parent()->parent() == _rootItem) 
	{
		return { child->data(0).toString(), child->data(1), _readOnlyView };
	}

	const auto parentItem = child->parent();
	return [this, parentItem, child]() -> IPropertyTuple
	{
		if (const auto point = parentItem; point->data(1).canConvert<QPoint>()) {
			return { point->data(0).toString(), QVariant::fromValue(createQPoint(point)), _readOnlyView };
		}
		else if (const auto rect = parentItem; rect->data(1).canConvert<QRect>()) {
			return { rect->data(0).toString(), QVariant::fromValue(createQRect(rect)), _readOnlyView };
		}
		else if (const auto colorTripletVector = parentItem->parent()->parent(); colorTripletVector->data(1).canConvert<QVector<COLOR_TRIPLET>>()) {
			return { colorTripletVector->data(0).toString(), QVariant::fromValue(createColorTripletVector(colorTripletVector)), _readOnlyView };
		} 
		else if (const auto colorTriplet = parentItem->parent(); colorTriplet->data(1).canConvert<COLOR_TRIPLET>()) {
			return { colorTriplet->data(0).toString(), QVariant::fromValue(createColorTriplet(colorTriplet)), _readOnlyView };
		}
		else if (const auto colorTipletSingle = parentItem; colorTipletSingle->data(1).canConvert<COLOR_TRIPLET_SINGLE>()) {
			return { colorTipletSingle->data(0).toString(), QVariant::fromValue(createColorTripletSingle(colorTipletSingle)), _readOnlyView };
		}
		
		return { child->data(0).toString(), child->data(1), _readOnlyView };
	}();
}

QPoint ParamPropModel::createQPoint(ParamPropItem *item) const noexcept
{
	const auto x = item->child(0)->data(1).toInt();
	const auto y = item->child(1)->data(1).toInt();

	const QPoint point(x, y);
	item->setData(1, QVariant::fromValue(point));

	return point;
}

QRect ParamPropModel::createQRect(ParamPropItem *item) const noexcept
{
	const auto x = item->child(0)->data(1).toInt();
	const auto y = item->child(1)->data(1).toInt();
	const auto width = item->child(2)->data(1).toInt();
	const auto height = item->child(3)->data(1).toInt();

	const QRect rect(x, y, width, height);
	item->setData(1, QVariant::fromValue(rect));

	return rect;
}

QVector<COLOR_TRIPLET> ParamPropModel::createColorTripletVector(ParamPropItem *item) const noexcept
{
	const int count = item->childCount();
	QVector<COLOR_TRIPLET> vec;
	vec.reserve(count);

	for (int i = 0; i < count; ++i)
	{
		vec.push_back(createColorTriplet(item->child(i)));
	}
	item->setData(1, QVariant::fromValue(vec));

	return vec;
}

COLOR_TRIPLET ParamPropModel::createColorTriplet(ParamPropItem *item) const noexcept
{
	const auto min = createColorTripletSingle(item->child(0));
	const auto max = createColorTripletSingle(item->child(1));

	const auto val = COLOR_TRIPLET{ min, max, item->data(0).toString()};
	item->setData(1, QVariant::fromValue(val));

	return val;
}

COLOR_TRIPLET_SINGLE ParamPropModel::createColorTripletSingle(ParamPropItem *item) const noexcept
{
	const auto h = item->child(0)->data(1).toInt();
	const auto s = item->child(1)->data(1).toInt();
	const auto v = item->child(2)->data(1).toInt();

	const auto val = COLOR_TRIPLET_SINGLE{ h, s, v, item->data(0).toString() };
	item->setData(1, QVariant::fromValue(val));

	return val;
}


ParamPropItem *ParamPropModel::insertChild(ParamPropItem *parent, const QString &name, const QVariant &value) noexcept
{
	const int count = parent->childCount();
	parent->insertChildren(count, 1, parent->columnCount());
	
	auto child = parent->child(count);
	child->setData(0, name);
	child->setData(1, value);

	return child;
}

ParamPropItem * ParamPropModel::setupGroupHeader(ParamPropItem* parent, const LandaJune::IPropertyTuple &prop) noexcept
{
	auto&[name, var, editable] = prop;
	const auto groupName = var.value<PARAM_GROUP_HEADER>();
	return insertChild(parent, groupName.GroupName(), var);
}

void ParamPropModel::setupQPoint(ParamPropItem *parent, const LandaJune::IPropertyTuple &prop) noexcept
{
	const auto&[name, pointVal, editable] = prop;
	auto child = insertChild(parent, name, pointVal);

	const auto point = pointVal.toPoint();

	insertChild(child, "X", point.x());
	insertChild(child, "Y", point.y());
}

void ParamPropModel::setupQRect(ParamPropItem *parent, const LandaJune::IPropertyTuple &prop) noexcept
{
	const auto&[name, rectVal, editable] = prop;
	auto child = insertChild(parent, name, rectVal);
	
	const auto rect = rectVal.toRect();

	insertChild(child, "X", rect.x());
	insertChild(child, "Y", rect.y());
	insertChild(child, "Width", rect.width());
	insertChild(child, "Height", rect.height());
}

void ParamPropModel::setupQRectVector(ParamPropItem *parent, const LandaJune::IPropertyTuple &prop) noexcept
{
	const auto&[name, _colorVar, editable] = prop;

	const auto qRectVector = _colorVar.value<QVector<QRect>>();
	auto child = insertChild(parent, name, _colorVar);

	for (int i = 0, size = qRectVector.size(); i < size; ++i)
	{
		setupQRect(child, { QString("[%1]").arg(i) , QVariant::fromValue(qRectVector[i]), editable });
	}
}

void ParamPropModel::setupColorTripletSingle(ParamPropItem *parent, const LandaJune::IPropertyTuple &prop) noexcept
{
	const auto&[name, _colorVar, editable] = prop;

	const auto color = _colorVar.value<COLOR_TRIPLET_SINGLE>();
	auto child = insertChild(parent, name, _colorVar);

	insertChild(child, "H", color.H());
	insertChild(child, "S", color.S());
	insertChild(child, "V", color.V());
}

void ParamPropModel::setupColorTriplet(ParamPropItem *parent, const LandaJune::IPropertyTuple &prop) noexcept
{
	const auto&[name, _colorVar, editable] = prop;

	const auto colorTriplet = _colorVar.value<COLOR_TRIPLET>();
	auto child = insertChild(parent, colorTriplet.ColorName(), _colorVar);

	setupColorTripletSingle(child, { "Min", QVariant::fromValue(colorTriplet.Min()), editable });
	setupColorTripletSingle(child, { "Max", QVariant::fromValue(colorTriplet.Max()), editable });
}

void ParamPropModel::setupColorTripletVector(ParamPropItem *parent, const LandaJune::IPropertyTuple &prop) noexcept
{
	const auto&[name, _colorVar, editable] = prop;

	const auto colorTripletVector = _colorVar.value<QVector<COLOR_TRIPLET>>();
	auto child = insertChild(parent, name, _colorVar);

	for (int i = 0, size = colorTripletVector.size(); i < size; ++i)
	{
		setupColorTriplet(child, { QString("[%1]").arg(i) , QVariant::fromValue(colorTripletVector[i]), editable });	
	}
}

LandaJune::IPropertyTuple ParamPropModel::getPropertyTuple(const QModelIndex& idx) const
{
	return propertyValue(getItem(idx));
}


QModelIndexList ParamPropModel::findItem(QString name) const
{
	return match(index(0, 0), Qt::DisplayRole, QVariant::fromValue(name), 1, Qt::MatchRecursive);
}