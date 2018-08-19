#ifndef QJSONMODEL_H
#define QJSONMODEL_H

#include <QAbstractItemModel>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QIcon>

class QJsonModel;
class QJsonItem;

class QJsonTreeItem
{
public:
	explicit QJsonTreeItem(const QVector<QVariant> &data, QJsonTreeItem *parent = nullptr);
	explicit QJsonTreeItem(QJsonTreeItem * parent = 0);
	~QJsonTreeItem();
	
	QJsonTreeItem *child(int row);
	QJsonTreeItem *parent();


	int childCount() const;
	int columnCount() const;
	
	int row() const;

	void setKey(const QString& key);
	void setValue(const QString& value);
	void setType(const QJsonValue::Type& type);
	QString key() const;
	QString value() const;
	QJsonValue::Type type() const;

	void appendChild(QJsonTreeItem * item);

	bool insertChildren(int position, int count, int columns);
	bool insertColumns(int position, int columns);
	bool removeChildren(int position, int count);
	bool removeColumns(int position, int columns);
	int childNumber() const;

	QVariant data(int column) const;
	bool setData(int column, const QVariant &value);


	static QJsonTreeItem* load(const QJsonValue& value, QJsonTreeItem * parent = 0);

protected:


private:
	QString mKey;
	QString mValue;
	QJsonValue::Type mType;
	QList<QJsonTreeItem*> childItems;
	QJsonTreeItem * parentItem;
	QVector<QVariant> itemData;


};

//---------------------------------------------------

class QJsonModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	explicit QJsonModel(QObject *parent = 0);
	~QJsonModel();

	void addRectangle(QRect& rc, QModelIndex);

	bool load(const QString& fileName);
	bool load(QIODevice * device);
	
	bool loadJson(const QByteArray& json);
	QString toJSON() const;
	
	QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;

	int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

	Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

	
	bool insertColumns(int position, int columns,
		const QModelIndex &parent = QModelIndex()) override;
	bool removeColumns(int position, int columns,
		const QModelIndex &parent = QModelIndex()) override;
	bool insertRows(int position, int rows,
		const QModelIndex &parent = QModelIndex()) override;
	bool removeRows(int position, int rows,
		const QModelIndex &parent = QModelIndex()) override;

	QJsonTreeItem *getItem(const QModelIndex &index) const;
	
	QJsonDocument json() const;
	QJsonValue getJSONObject() const {
		return genJson(mRootItem);
	}
	

private:
	QJsonValue genJson(QJsonTreeItem *) const;

	QJsonTreeItem * mRootItem;
	QStringList mHeaders;


};

#endif // QJSONMODEL_H