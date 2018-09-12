#include <QStringList>
#include <utility>
#include "paramPropItem.h"

ParamPropItem::ParamPropItem(QVector<QVariant> data, ParamPropItem *parent)
	: itemData(std::move(data))
	, parentItem(parent)
{
}


ParamPropItem::~ParamPropItem()
{
    qDeleteAll(childItems);
}

ParamPropItem *ParamPropItem::child(int number) const
{
    return childItems.value(number);
}

int ParamPropItem::childCount() const
{
    return childItems.count();
}

int ParamPropItem::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<ParamPropItem*>(this));

    return 0;
}

int ParamPropItem::columnCount() const
{
    return itemData.count();
}

QVariant ParamPropItem::data(int column) const
{
    return itemData.value(column);
}

bool ParamPropItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > childItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        ParamPropItem *item = new ParamPropItem(data, this);
        childItems.insert(position, item);
    }

    return true;
}

bool ParamPropItem::insertColumns(int position, int columns)
{
    if (position < 0 || position > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.insert(position, QVariant());

    foreach (ParamPropItem *child, childItems)
        child->insertColumns(position, columns);

    return true;
}

ParamPropItem *ParamPropItem::parent() const
{
    return parentItem;
}

void ParamPropItem::removeChild(ParamPropItem *child)
{
	if (!child)
		return;

	childItems.removeOne(child);
}

bool ParamPropItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}

bool ParamPropItem::removeColumns(int position, int columns)
{
    if (position < 0 || position + columns > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.remove(position);

    foreach (ParamPropItem *child, childItems)
        child->removeColumns(position, columns);

    return true;
}

bool ParamPropItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= itemData.size())
        return false;

    itemData[column] = value;
    return true;
}

void ParamPropItem::setUserData(const QVariant &value)
{
	_userData = value;
}
