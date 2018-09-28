#pragma once

#include <QWidget>
#include "ui_onerunviewer.h"
#include <QFileSystemWatcher>
#include <QFileSystemModel>

class oneRunViewer : public QWidget
{
	Q_OBJECT

public:
	oneRunViewer(QWidget *parent = Q_NULLPTR);
	~oneRunViewer();

	void setTargetFolder( const QString& targetFolder );

public slots:
	
	void onTreeClicked(const QModelIndex& index);
	void handleSelectionChanged(const QItemSelection&, const QItemSelection&);
	void onDirectoryLoaded(const QString& path);
	void onRootPathChanged(const QString &newPath);

private:

	void showConent(const QString& filePath );

	void showImage(const QString& filePath );
	void showCSV(const QString& filePath );

	Ui::oneRunViewer ui;

	QIcon _iconFolder, _iconFolderRoot, _iconPlaylist;
	QString _targetFolder;
	QFileSystemModel   * _filesModel = nullptr;

};
