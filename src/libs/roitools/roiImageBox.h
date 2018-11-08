#pragma once

#include <QWidget>
#include "ui_roiImageBox.h"
#include "roiRenderWidgetBase.h"
#include "roiRenderWidgetStrip.h"
#include "roiRenderWidgetFull.h"
#include "roiRenderWidgetWave.h"

#include "common/june_errors.h"
#include "unitSwitchLabel.h"


class roiImageBox : public QWidget
{
	Q_OBJECT

public:
	enum RENDER_WIDGET_TYPE { RENDER_FULL, RENDER_WAVE, RENDER_STRIP };
	
	roiImageBox(QWidget *parent = Q_NULLPTR);
	~roiImageBox();

	void createWidget (RENDER_WIDGET_TYPE rType);
	void setpx2mmRatio(  double hRatio,double vRatio );
	bool hasImage() const { return _renderWidget->hasImage(); }

	void setC2CRoisLinedUp ( bool bVal ) const;

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
							, QRect leftStripRect
							, QRect rightStripRect
							, const int pageOffsetX
							, QSize i2sMargins
							, QSize c2cMargins
							, int c2cCircleDiameter  ) const
	{
		dynamic_cast<roiRenderWidgetFull*>(_renderWidget)->setROIs( true, 
																		is2sRcLeft, 
																		is2sRcRight, 
																		c2cRectsLeft, 
																		c2cRectsRight, 
																		leftStripRect,
																		rightStripRect,
																		pageOffsetX, 
																		i2sMargins,
																		c2cMargins, 
																		c2cCircleDiameter, 
																		true );
	}

	void updateROIs_Full(const QRect& is2sRcLeft
							, const QRect& is2sRcRight	
							, const QVector<QRect>& c2cRectsLeft
							, const QVector<QRect>& c2cRectsRight
							, QRect leftStripRect
							, QRect rightStripRect
							, const int pageOffsetX
							, QSize i2sMargins
							, QSize c2cMargins
							, int c2cCircleDiameter
							, bool updateBoth) const
	{
		dynamic_cast<roiRenderWidgetFull*>(_renderWidget)->setROIs( !_renderWidget->hasImage(), 
																		is2sRcLeft, 
																		is2sRcRight, 
																		c2cRectsLeft, 
																		c2cRectsRight, 
																		leftStripRect,
																		rightStripRect,
																		pageOffsetX, 
																		i2sMargins,
																		c2cMargins, 
																		c2cCircleDiameter, 
																		updateBoth );
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
	void setInitialROIs_Strip(
				  const QRect& is2sRc
				, const QVector<QRect>& c2cRects
				, int edgeX
				, QSize i2sMargins
				, QSize c2cMargins
				, int c2cCircleDiameter
				, bool bInteractive ) const
	{
		dynamic_cast<roiRenderWidgetStrip*>(_renderWidget)->setROIs( true, is2sRc, c2cRects, edgeX, i2sMargins, c2cMargins, c2cCircleDiameter, bInteractive);
	}

	void updateROIs_Strip(
				  const QRect& is2sRc
				, const QVector<QRect>& c2cRects
				, int edgeX
				, QSize i2sMargins
				, QSize c2cMargins
				, int c2cCircleDiameter
				, bool bInteractive) const
	{
		dynamic_cast<roiRenderWidgetStrip*>(_renderWidget)->setROIs( !_renderWidget->hasImage(), is2sRc, c2cRects, edgeX, i2sMargins, c2cMargins, c2cCircleDiameter, bInteractive);
	}
	
	void updateScaleFromExternal( double glScale, double imageScale ) const;
	void updateScrollsFromExternal( int hScrollVal, int vScrollVal ) const;
	void updateUnits ( int oldUnits, int newUnits );

protected:

	void createRenderWidget();
	void setImage ( const QString& strFilePath );
	void displayMetaData ();
	void setZoom ( int zoomPercentage );

	void updateCursorPositionText(QPoint pt, QSize size);

signals :

	void imageLoaded(QString filePath, LandaJune::CORE_ERROR error );

	// strip image signals
	void roiChanged_strip_edge	( const QPoint i2spt,  const int edgeX );
	void roiChanged_strip_i2s	( const QPoint i2spt );
	void roiChanged_strip_c2c	( const QPoint i2spt, const QVector<QPoint>& c2cPts );

	// full image signals
	void roiChanged_full_leftStripEdge	( const QPoint i2spt,  const int edgeX );
	void roiChanged_full_rightStripEdge	( const QPoint i2spt,  const int edgeX );
	void roiChanged_full_pageEdge	( const QPoint i2spt,  const int edgeX );
	void roiChanged_full_i2s	( const QPoint i2spt );
	void roiChanged_full_c2c	( const QPoint i2spt, const QVector<QPoint>& c2cPts );

	// full image signals
	void roiChanged_waveTriangle( QPoint newControlPoint );

	// general signals
	void scaleChanged(double glScale, double imageScale);
	void scrollValuesChanged( int hScrollVal, int vScrollVal);
	void unitsChanged ( int oldUnits, int newUnits);
	void doubleClick ( QPoint pos );

private slots:

	void onOpenFile();
	void onZoomAction();
	void onImageCursorPos(QPoint pt, QSize size);

	void onHorizonalScrollbarValueChanged(int);
	void onVerticalScrollbarValueChanged(int);

	void onScaleChanged(double newGLScale, double newImageScale);
	void onChangeUnits();

	void onDoubleClick (QPoint pos);

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

	double _hRatio = 1.0;
	double _vRatio = 1.0;
	QPoint _lastPostionPt = {};
	QSize _lastPostionSize = {};

	unitSwitchLabel::LABEL_UNITS _currentUnits = unitSwitchLabel::MM;
};
