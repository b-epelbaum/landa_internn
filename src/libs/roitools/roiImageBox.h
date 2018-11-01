#pragma once

#include <QWidget>
#include "ui_roiImageBox.h"
#include "roiRenderWidget.h"


class roiImageBox : public QWidget
{
	Q_OBJECT

public:
	roiImageBox(QWidget *parent = Q_NULLPTR);
	~roiImageBox();

	void setFileMetaInfo (QString strPrompt, QString strFileSaveKey)
	{
		_strPrompt = strPrompt;
		_strFileSaveKey = strFileSaveKey;
	}

	void setInitialROIs(const QRect& is2sRc, const QVector<QRect>& c2cRects, QSize i2sMargins, QSize c2cMargins, int c2cCircleDiameter ) const { _renderWidget->setInitialROIs(is2sRc, c2cRects, i2sMargins, c2cMargins, c2cCircleDiameter);}
	void updateROIs(const QRect& rc, const QVector<QRect>& rects ) const { _renderWidget->updateROIs(rc, rects);}
	
	void updateScaleFromExternal( double glScale, double imageScale );
	void updateScrollsFromExternal( int hScrollVal, int vScrollVal );

protected:

	LandaJune::CORE_ERROR setImage ( const QString& strFilePath );
	void displayMetaData ();

	void setZoom ( int zoomPercentage );

signals :
	
	void i2sPosChanged(QPoint pt);
	void c2cPosChanged(int idx, QPoint pt);

	void scaleChanged(double glScale, double imageScale);
	void scrollValuesChanged ( int hScrollVal, int vScrollVal);

private slots:

	void onOpenFile();
	void onZoomAction();
	void onImageCursorPos(QPoint pt, QSize size);

	void onHorizonalScrollbarValueChanged(int);
	void onVerticalScrollbarValueChanged(int);

	void onScaleChanged(double newGLScale, double newImageScale);

private:
	Ui::roiImageFullBox ui;

	QScrollBar * _horizontalBar = nullptr;
	QScrollBar * _verticalBar = nullptr;

	QString _strPrompt, _strFileSaveKey;
	QString _loadedImageFilePath;
	roiRenderWidget * _renderWidget = nullptr;

	QToolButton * _zoomButt = nullptr;

};
