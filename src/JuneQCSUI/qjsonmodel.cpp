#include "qjsonmodel.h"
#include <QFile>
#include <QDebug>
#include <QFont>

QJsonTreeItem::QJsonTreeItem(QJsonTreeItem *parent)
{
	parentItem = parent;
}

QJsonTreeItem::QJsonTreeItem(const QVector<QVariant> &data, QJsonTreeItem *parent)
{
	parentItem = parent;
	itemData = data;
}


QJsonTreeItem::~QJsonTreeItem()
{
	qDeleteAll(childItems);
}

void QJsonTreeItem::appendChild(QJsonTreeItem *item)
{
	childItems.append(item);
}

QJsonTreeItem *QJsonTreeItem::child(int row)
{
	return childItems.value(row);
}

QJsonTreeItem *QJsonTreeItem::parent()
{
	return parentItem;
}

int QJsonTreeItem::childCount() const
{
	return childItems.count();
}

int QJsonTreeItem::childNumber() const
{
	if (parentItem)
		return parentItem->childItems.indexOf(const_cast<QJsonTreeItem*>(this));

	return 0;
}

int QJsonTreeItem::columnCount() const
{
	return itemData.count();
}

int QJsonTreeItem::row() const
{
	if (parentItem)
		return parentItem->childItems.indexOf(const_cast<QJsonTreeItem*>(this));

	return 0;
}

void QJsonTreeItem::setKey(const QString &key)
{
	mKey = key;
}

void QJsonTreeItem::setValue(const QString &value)
{
	mValue = value;
}

void QJsonTreeItem::setType(const QJsonValue::Type &type)
{
	mType = type;
}

QString QJsonTreeItem::key() const
{
	return mKey;
}

QString QJsonTreeItem::value() const
{
	return mValue;
}

QJsonValue::Type QJsonTreeItem::type() const
{
	return mType;
}


QVariant QJsonTreeItem::data(int column) const
{
	return itemData.value(column);
}

bool QJsonTreeItem::insertChildren(int position, int count, int columns)
{
	if (position < 0 || position > childItems.size())
		return false;

	for (int row = 0; row < count; ++row) {
		const QVector<QVariant> data(columns);
		QJsonTreeItem *item = new QJsonTreeItem(data, this);
		childItems.insert(position, item);
	}

	return true;
}

bool QJsonTreeItem::insertColumns(int position, int columns)
{
	if (position < 0 || position > itemData.size())
		return false;

	for (int column = 0; column < columns; ++column)
		itemData.insert(position, QVariant());

	foreach(QJsonTreeItem *child, childItems)
		child->insertColumns(position, columns);

	return true;
}

bool QJsonTreeItem::removeChildren(int position, int count)
{
	if (position < 0 || position + count > childItems.size())
		return false;

	for (int row = 0; row < count; ++row)
		delete childItems.takeAt(position);

	return true;
}

bool QJsonTreeItem::removeColumns(int position, int columns)
{
	if (position < 0 || position + columns > itemData.size())
		return false;

	for (int column = 0; column < columns; ++column)
		itemData.remove(position);

	foreach(QJsonTreeItem *child, childItems)
		child->removeColumns(position, columns);

	return true;
}

bool QJsonTreeItem::setData(int column, const QVariant &value)
{
	if (column < 0 || column >= itemData.size())
		return false;

	itemData[column] = value;
	return true;
}


QJsonTreeItem* QJsonTreeItem::load(const QJsonValue& value, QJsonTreeItem* parent)
{
	QJsonTreeItem * rootItem = new QJsonTreeItem(parent);
	rootItem->setKey("root");

	if (value.isObject())
	{

		//Get all QJsonValue childs
		for (QString key : value.toObject().keys()) {
			QJsonValue v = value.toObject().value(key);
			QJsonTreeItem * child = load(v, rootItem);
			child->setKey(key);
			child->setType(v.type());
			rootItem->appendChild(child);

		}

	}

	else if (value.isArray())
	{
		//Get all QJsonValue childs
		int index = 0;
		for (QJsonValue v : value.toArray()) {

			QJsonTreeItem * child = load(v, rootItem);
			child->setKey(QString::number(index));
			child->setType(v.type());
			rootItem->appendChild(child);
			++index;
		}
	}
	else
	{
		rootItem->setValue(value.toVariant().toString());
		rootItem->setType(value.type());
	}

	return rootItem;
}

//=========================================================================

QJsonModel::QJsonModel(QObject *parent) :
	QAbstractItemModel(parent)
{
	mRootItem = new QJsonTreeItem;
	mHeaders.append("Parameter");
	mHeaders.append("Value");



}

QJsonModel::~QJsonModel()
{
	delete mRootItem;
}

bool QJsonModel::load(const QString &fileName)
{
	QFile file(fileName);
	bool success = false;
	if (file.open(QIODevice::ReadOnly)) {
		success = load(&file);
		file.close();
	}
	else success = false;

	return success;
}

bool QJsonModel::load(QIODevice *device)
{
	return loadJson(device->readAll());
}

bool QJsonModel::loadJson(const QByteArray &json)
{
	auto const& jdoc = QJsonDocument::fromJson(json);

	if (!jdoc.isNull())
	{
		beginResetModel();
		delete mRootItem;
		if (jdoc.isArray()) {
			mRootItem = QJsonTreeItem::load(QJsonValue(jdoc.array()));
			mRootItem->setType(QJsonValue::Array);

		}
		else {
			mRootItem = QJsonTreeItem::load(QJsonValue(jdoc.object()));
			mRootItem->setType(QJsonValue::Object);
		}
		endResetModel();
		return true;
	}

	qDebug() << Q_FUNC_INFO << "cannot load json";
	return false;
}


QVariant QJsonModel::data(const QModelIndex &index, int role) const
{

	if (!index.isValid())
		return QVariant();


	QJsonTreeItem *item = static_cast<QJsonTreeItem*>(index.internalPointer());


	if (role == Qt::DisplayRole) {

		if (index.column() == 0)
			return QString("%1").arg(item->key());

		if (index.column() == 1)
			return QString("%1").arg(item->value());
	}
	else if (Qt::EditRole == role) 
	{
		if (index.column() == 1) 
		{
			return QString("%1").arg(item->value());
		}
		if (index.column() == 0) {
			return QString("%1").arg(item->key());
		}

	}



	return QVariant();

}

bool QJsonModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (Qt::EditRole == role) 
	{
		if (index.column() == 1  )
		{
			auto item = static_cast<QJsonTreeItem*>(index.internalPointer());
			item->setValue(value.toString());
			emit dataChanged(index, index, { Qt::EditRole });
			return true;
		}

		if (index.column() == 0)
		{
			auto item = static_cast<QJsonTreeItem*>(index.internalPointer());
			item->setKey(value.toString());
			emit dataChanged(index, index, { Qt::EditRole });
			return true;
		}
	}

	return false;
}

bool QJsonModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
	if (role != Qt::EditRole || orientation != Qt::Horizontal)
		return false;

	const auto result = mRootItem->setData(section, value);

	if (result)
		emit headerDataChanged(orientation, section, section);

	return result;
}

bool QJsonModel::insertColumns(int position, int columns, const QModelIndex& parent)
{
	beginInsertColumns(parent, position, position + columns - 1);
	const auto success = mRootItem->insertColumns(position, columns);
	endInsertColumns();

	return success;
}

bool QJsonModel::removeColumns(int position, int columns, const QModelIndex& parent)
{
	beginRemoveColumns(parent, position, position + columns - 1);
	const auto success = mRootItem->removeColumns(position, columns);
	endRemoveColumns();

	if (mRootItem->columnCount() == 0)
		removeRows(0, rowCount());

	return success;
}


QVariant QJsonModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	return (orientation == Qt::Horizontal) ? mHeaders.value(section) : QVariant();
}

QModelIndex QJsonModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	auto parentItem = (!parent.isValid()) ?  mRootItem : static_cast<QJsonTreeItem*>(parent.internalPointer());

	const auto childItem = parentItem->child(row);
	return childItem ? createIndex(row, column, childItem) : QModelIndex();
}

QModelIndex QJsonModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	const auto parentItem = static_cast<QJsonTreeItem*>(index.internalPointer())->parent();

	if (parentItem == mRootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

int QJsonModel::rowCount(const QModelIndex &parent) const
{
	QJsonTreeItem *parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = mRootItem;
	else
		parentItem = static_cast<QJsonTreeItem*>(parent.internalPointer());

	return parentItem->childCount();
}

int QJsonModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
		return 2;
}

Qt::ItemFlags QJsonModel::flags(const QModelIndex &index) const
{
	if (index.column() == 0 || index.column() == 1 )
	{
		return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
	}
	return QAbstractItemModel::flags(index);
}

QJsonDocument QJsonModel::json() const
{

	auto v = genJson(mRootItem);
	QJsonDocument doc;

	if (v.isObject()) {
		doc = QJsonDocument(v.toObject());
	}
	else {
		doc = QJsonDocument(v.toArray());
	}

	return doc;
}

QJsonValue  QJsonModel::genJson(QJsonTreeItem * item) const
{
	const auto type = item->type();
	const int  nchild = item->childCount();

	if (QJsonValue::Object == type) 
	{
		QJsonObject jo;
		for (auto i = 0; i < nchild; ++i) 
		{
			const auto ch = item->child(i);
			const auto key = ch->key();
			jo.insert(key, genJson(ch));
		}
		return  jo;
	}
	
	if (QJsonValue::Array == type) 
	{
		QJsonArray arr;
		for (auto i = 0; i < nchild; ++i) 
		{
			const auto ch = item->child(i);
			arr.append(genJson(ch));
		}
		return arr;
	}
	
	return QJsonValue (item->value());
}

QJsonTreeItem *QJsonModel::getItem(const QModelIndex &index) const
{
	if (index.isValid()) {
		QJsonTreeItem *item = static_cast<QJsonTreeItem*>(index.internalPointer());
		if (item)
			return item;
	}
	return mRootItem;
}

bool QJsonModel::insertRows(int position, int rows, const QModelIndex &parent)
{
	QJsonTreeItem *parentItem = getItem(parent);

	beginInsertRows(parent, position, position + rows - 1);
	const auto success = parentItem->insertChildren(position, rows, mRootItem->columnCount());
	endInsertRows();

	return success;
}

bool QJsonModel::removeRows(int position, int rows, const QModelIndex& parent)
{
	auto parentItem = getItem(parent);
	beginRemoveRows(parent, position, position + rows - 1);
	auto const success = parentItem->removeChildren(position, rows);
	endRemoveRows();

	return success;
}


QString QJsonModel::toJSON() const
{
	return json().toJson(QJsonDocument::Indented);
}

void QJsonModel::addRectangle(QRect& rc, QModelIndex currentIndex)
{
	auto rowsInParent = getItem(currentIndex)->childCount();
	insertRow(currentIndex.row(), currentIndex);
		rowsInParent++;

	auto newItem = getItem(index(rowsInParent-1, 0, currentIndex));
	newItem->setKey("Rectangle");
}
