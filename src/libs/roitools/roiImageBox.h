#pragma once

#include <QWidget>
#include "ui_roiImageBox.h"
#include "roiRenderWidgetBase.h"
#include "roiRenderWidgetStrip.h"
#include "roiRenderWidgetFull.h"
#include "roiRenderWidgetWave.h"

#include "common/june_errors.h"


class roiImageBox : public QWidget
{
	Q_OBJECT

public:
	enum RENDER_WIDGET_TYPE { RENDER_FULL, RENDER_WAVE, RENDER_STRIP };
	
	roiImageBox(QWidget *parent = Q_NULLPTR);
	~roiImageBox();

	void createWidget (RENDER_WIDGET_TYPE rType);
	bool hasImage() const { return _renderWidget->hasImage(); }

	void setFileMetaInfo (QString strPrompt, QString strFileSaveKey)
	{
		_strPrompt = strPrompt;
		_strFileSaveKey = strFileSaveKey;
	}
	void setAlias( const QString strAlias ) { _alias = strAlias; if (_renderWidget ) _renderWidget->setAlias(strAlias);}

	// full image functions
	void setInitialROIs_Full( const QRect& is2sRcLeft
							, const QRect& is2sRcRight	
							, const QVector<QRect>& c2cRectsLeft
							, const QVector<QRect>& c2cRectsRight
							, QSize i2sMargins
							, QSize c2cMargins
							, int c2cCircleDiameter  ) const
	{
		dynamic_cast<roiRenderWidgetFull*>(_renderWidget)->setROIs( true, is2sRcLeft, is2sRcRight, c2cRectsLeft, c2cRectsRight, i2sMargins,c2cMargins, c2cCircleDiameter );
	}

	void updateROIs_Full(const QRect& is2sRcLeft
							, const QRect& is2sRcRight	
							, const QVector<QRect>& c2cRectsLeft
							, const QVector<QRect>& c2cRectsRight
							, QSize i2sMargins
							, QSize c2cMargins
							, int c2cCircleDiameter) const
	{
		dynamic_cast<roiRenderWidgetFull*>(_renderWidget)->setROIs( !_renderWidget->hasImage(), is2sRcLeft, is2sRcRight, c2cRectsLeft, c2cRectsRight, i2sMargins,c2cMargins, c2cCircleDiameter );
	}

	// wave image functions
	void setInitialROIs_Wave( const QRect& waveTriangleRect
							, const QRect& waveROIRect	
							, QSize triangleMargins
							) const
	{
		dynamic_cast<roiRenderWidgetWave*>(_renderWidget)->setROIs( true, waveTriangleRect, waveROIRect, triangleMargins);
	}

	void updateROIs_Wave(	const QRect& waveTriangleRect
							, const QRect& waveROIRect	
							, QSize triangleMargins
						) const
	{
		dynamic_cast<roiRenderWidgetWave*>(_renderWidget)->setROIs( !_renderWidget->hasImage(), waveTriangleRect, waveROIRect, triangleMargins );
	}

	// strip image functions
	void setInitialROIs_Strip(const QRect& is2sRc, const QVector<QRect>& c2cRects, QSize i2sMargins, QSize c2cMargins, int c2cCircleDiameter ) const
	{
		dynamic_cast<roiRenderWidgetStrip*>(_renderWidget)->setROIs( true, is2sRc, c2cRects, i2sMargins, c2cMargins, c2cCircleDiameter);
	}

	void updateROIs_Strip(const QRect& is2sRc, const QVector<QRect>& c2cRects, QSize i2sMargins, QSize c2cMargins, int c2cCircleDiameter ) const
	{
		dynamic_cast<roiRenderWidgetStrip*>(_renderWidget)->setROIs( !_renderWidget->hasImage(), is2sRc, c2cRects, i2sMargins, c2cMargins, c2cCircleDiameter);
	}
	
	void updateScaleFromExternal( double glScale, double imageScale );
	void updateScrollsFromExternal( int hScrollVal, int vScrollVal );

protected:

	void createRenderWidget();
	void setImage ( const QString& strFilePath );
	void displayMetaData ();
	void setZoom ( int zoomPercentage );

signals :

	void imageLoaded(QString filePath, LandaJune::CORE_ERROR error );

	// strip image signals
	void roiChanged_strip( const QVector<QPoint>& c2cPts );
	void c2CrossMoved_strip( int idx, QPoint centerPointOnImage);

	// full image signals
	void roiChanged_full( const QVector<QPoint>& c2cPts );

	// full image signals
	void roiChanged_wave( const QVector<QPoint>& c2cPts );

	// general signals
	void scaleChanged(double glScale, double imageScale);
	void scrollValuesChanged( int hScrollVal, int vScrollVal);

private slots:

	void onOpenFile();
	void onZoomAction();
	void onImageCursorPos(QPoint pt, QSize size);

	void onHorizonalScrollbarValueChanged(int);
	void onVerticalScrollbarValueChanged(int);

	void onScaleChanged(double newGLScale, double newImageScale);

private:
	Ui::roiImageFullBox ui;

	RENDER_WIDGET_TYPE _renderWidgetType = RENDER_FULL; 
	QScrollBar * _horizontalBar = nullptr;
	QScrollBar * _verticalBar = nullptr;

	QString _strPrompt, _strFileSaveKey;
	QString _loadedImageFilePath;
	QString _alias;

	roiRenderWidgetBase * _renderWidget = nullptr;
	QToolButton * _zoomButt = nullptr;
};
