#include <QStringList>
#include "providerPropItem.h"

ProviderPropsItem::ProviderPropsItem(const QVector<QVariant> &data, ProviderPropsItem *parent)
{
    parentItem = parent;
    itemData = data;
}


ProviderPropsItem::~ProviderPropsItem()
{
    qDeleteAll(childItems);
}

ProviderPropsItem *ProviderPropsItem::child(int number)
{
    return childItems.value(number);
}

int ProviderPropsItem::childCount() const
{
    return childItems.count();
}

int ProviderPropsItem::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<ProviderPropsItem*>(this));

    return 0;
}

int ProviderPropsItem::columnCount() const
{
    return itemData.count();
}

QVariant ProviderPropsItem::data(int column) const
{
    return itemData.value(column);
}

bool ProviderPropsItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > childItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        ProviderPropsItem *item = new ProviderPropsItem(data, this);
        childItems.insert(position, item);
    }

    return true;
}

bool ProviderPropsItem::insertColumns(int position, int columns)
{
    if (position < 0 || position > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.insert(position, QVariant());

    foreach (ProviderPropsItem *child, childItems)
        child->insertColumns(position, columns);

    return true;
}

ProviderPropsItem *ProviderPropsItem::parent()
{
    return parentItem;
}

bool ProviderPropsItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}

bool ProviderPropsItem::removeColumns(int position, int columns)
{
    if (position < 0 || position + columns > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.remove(position);

    foreach (ProviderPropsItem *child, childItems)
        child->removeColumns(position, columns);

    return true;
}

bool ProviderPropsItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= itemData.size())
        return false;

    itemData[column] = value;
    return true;
}
