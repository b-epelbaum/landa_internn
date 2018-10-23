#pragma once

#include <QWidget>
#include "ui_roiWidget.h"

class roiWidget : public QWidget
{
	Q_OBJECT

public:
	roiWidget(QWidget *parent = Q_NULLPTR);
	~roiWidget();

	void setFileMetaInfo (QString strPrompt, QString strFileSaveKey)
	{
		_strPrompt = strPrompt;
		_strFileSaveKey = strFileSaveKey;
	}

	void setScales( float glScale, float imageScale );
	void setScrolls( int hScrollVal, int vScrollVal );

protected:

	void setImage ( const QString& strFilePath );
	void displayMetaData ();

	void setZoom ( int zoomPercentage );

signals :
	void scaleChanged(float glScale, float imageScale);
	void scrollValuesChanged ( int hScrollVal, int vScrollVal);

private slots:

	void onOpenFile();
	void onZoomAction();
	void onImageCursorPos(QPoint pt, QSize size);

	void horzValueChanged(int);
	void vertValueChanged(int);

	void onScaleChanged(double newGLScale, double newImageScale);

private:
	Ui::roiWidget ui;

	QString _strPrompt, _strFileSaveKey;
	QString _loadedImageFilePath;
	RenderWidget * _renderWidget = nullptr;

};
