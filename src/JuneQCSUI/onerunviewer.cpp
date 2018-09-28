#include "onerunviewer.h"

#include <QImageReader>
#include <QStandardItemModel>
#include <QTextStream>

oneRunViewer::oneRunViewer(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	//connect(ui.treeView, &QTreeView::clicked, this, &oneRunViewer::onTreeClicked);
}

oneRunViewer::~oneRunViewer()
{
}

void oneRunViewer::setTargetFolder(const QString& targetFolder)
{
	QStringList paths;

	disconnect(ui.treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &oneRunViewer::handleSelectionChanged);

    _filesModel = new QFileSystemModel(this);
	_filesModel->setFilter(QDir::AllDirs | QDir::AllEntries |QDir::NoDotAndDotDot);
    ui.treeView->setModel(_filesModel);

	connect(ui.treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &oneRunViewer::handleSelectionChanged);
	connect(_filesModel, &QFileSystemModel::directoryLoaded, this, &oneRunViewer::onDirectoryLoaded);
	connect(_filesModel, &QFileSystemModel::rootPathChanged, this, &oneRunViewer::onRootPathChanged);

	QModelIndex idx =  _filesModel->setRootPath(targetFolder);
	ui.treeView->setRootIndex(idx);


	ui.treeView->header()->setSectionHidden(1, true);
	ui.treeView->header()->setSectionHidden(2, true);
	ui.treeView->header()->setSectionHidden(3, true);


	_targetFolder = targetFolder;
	paths << targetFolder;
}

void oneRunViewer::onDirectoryLoaded(const QString &path)
{
	ui.treeView->expandAll();
}

void oneRunViewer::onRootPathChanged(const QString &newPath)
{
	
}

void oneRunViewer::onTreeClicked(const QModelIndex &index)
{
    // Get the full path of the item that's user clicked on
	if ( _filesModel->fileInfo(index).isDir() )
		return;
    
	showConent(_filesModel->fileInfo(index).absoluteFilePath());
}

void oneRunViewer::showConent(const QString& filePath )
{
	if ( QFileInfo(filePath).suffix().toLower() == "bmp" )
		showImage(filePath);
	else if ( QFileInfo(filePath).suffix().toLower() == "csv" )
		showCSV(filePath);
}

void oneRunViewer::showImage(const QString& filePath)
{
	ui.stackedWidget->setCurrentIndex(0);
	QImageReader reader(filePath);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (!newImage.isNull()) 
	{
		const auto& areaWidth = ui.scrollArea->width();
		const auto& areaHeight = ui.scrollArea->height();

		ui.imageBox->setScaledContents((newImage.width() < areaWidth && newImage.height() < areaHeight));
		if((newImage.width() < areaWidth || newImage.height() < areaHeight))
		{
			// set a scaled pixmap to a w x h window keeping its aspect ratio 
			ui.imageBox->setPixmap(QPixmap::fromImage(newImage).scaled(areaWidth,areaHeight,Qt::KeepAspectRatioByExpanding));
		}
		else
			ui.imageBox->setPixmap(QPixmap::fromImage(newImage));
	}
}

void oneRunViewer::handleSelectionChanged(const QItemSelection& newSel, const QItemSelection& oldSel)
{
	if ( newSel.size() )
		onTreeClicked(newSel.indexes().at(0));
}

void oneRunViewer::showCSV(const QString& filePath )
{
	ui.stackedWidget->setCurrentIndex(1);

	QStandardItemModel *model = new QStandardItemModel;
	QFile file(filePath);
	if (file.open(QIODevice::ReadOnly)) 
	{
	    int lineindex = 0;                     // file line counter
	    QTextStream in(&file);                 // read to text stream

	    while (!in.atEnd()) 
		{                           
			QString fileLine = in.readLine();  

			// parse the read line into separate pieces(tokens) with "," as the delimiter
			QStringList lineToken = fileLine.split(",", QString::SkipEmptyParts); 

			// load parsed data to model accordingly
			for (int j = 0; j < lineToken.size(); j++) 
			{
				QString value = lineToken.at(j);
				QStandardItem *item = new QStandardItem(value);
				model->setItem(lineindex, j, item);
			}
			lineindex++;   
		}
		file.close();
	}
	auto oldModel = ui.tableView->model();
	ui.tableView->setModel(model);

	delete oldModel;
}
