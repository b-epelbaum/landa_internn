#pragma once

#include <QVariant>
#include <QVector>

class ProviderPropsItem
{
public:
    explicit ProviderPropsItem(const QVector<QVariant> &data, ProviderPropsItem *parent = 0);
    ~ProviderPropsItem();

	ProviderPropsItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool insertChildren(int position, int count, int columns);
    bool insertColumns(int position, int columns);
	ProviderPropsItem *parent();
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int childNumber() const;
    bool setData(int column, const QVariant &value);

private:
    QList<ProviderPropsItem*> childItems;
    QVector<QVariant> itemData;
	ProviderPropsItem *parentItem;
};

