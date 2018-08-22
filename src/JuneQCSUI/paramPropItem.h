#pragma once

#include <QVariant>
#include <QVector>

class ParamPropItem
{
public:
    explicit ParamPropItem(const QVector<QVariant> &data, ParamPropItem *parent = 0);
    ~ParamPropItem();

	ParamPropItem *child(int number) const;
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool insertChildren(int position, int count, int columns);
    bool insertColumns(int position, int columns);
	ParamPropItem *parent() const;
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int childNumber() const;
    bool setData(int column, const QVariant &value);
	void setUserData(const QVariant &value);
	QVariant userData() const { return _userData; }

private:
    QList<ParamPropItem*> childItems;
    QVector<QVariant> itemData;
	ParamPropItem *parentItem;
	QVariant _userData;
};

