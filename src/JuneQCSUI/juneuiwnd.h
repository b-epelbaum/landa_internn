#pragma once

#include "ui_juneuiwnd.h"
#include "paramPropModel.h"

#include <QProgressBar>
#include <QTimer>
#include "common/june_exceptions.h"

namespace LandaJune
{
	namespace UI
	{
		class JuneUIWnd : public QMainWindow
		{
			Q_OBJECT

		public:
			JuneUIWnd(QWidget *parent = Q_NULLPTR);

		private:
			Ui::JuneUIWndClass ui{};

		private slots:

			void onCoreStopped();
			void onCoreException(const LandaJune::BaseException& ex);

			//void browseForFolder();
			void zoomIn();
			void zoomOut();
			void normalSize();
			void fitToWindow();
			void adjustScrollBar(QScrollBar* scrollBar, double factor);
			void scaleImage(double factor);
			void updateActions() const;

			void runAll();
			void runOnce();

			void stop();
			void onAboutToQuit();

			void onFrameProviderComboChanged(int index);
			void onAlgoRunnerComboChanged(int index);
			void updateStats() const;

			void onProviderPropChanged(QString propName, const QVariant& newVal);
			void onProcessParameterChanged(QString propName, const QVariant& newVal);

			void onUpdateProcessParams();
			void onUpdateCalculatedParams();

			void onSaveConfig();
			void onLoadConfig();

			void onAddColor();
			void onRemoveColor();

			void onTimerTick();

			void processParamSelectionChanged(const  QModelIndex&, const  QModelIndex&);

		signals :

			void processingFinished();

		private :

			void init();
			void initUI();
			void initCore();
			void createActions();
			void createStatusBar();

			void enumerateFrameProviders() ;
			void enumerateAlgoRunners() ;
			void enableUIForProcessing(bool bEnable);

			void initProcessParameters() const;

			void saveExpandedOnLevel(const QModelIndex& index, QSet<int> & nodes, QTreeView * view, int& iLevel ) const;
			void restoreExpandedOnLevel(const QModelIndex& index, QSet<int> & nodes, QTreeView * view, int& iLevel) const;
			void saveExpandedState(QSet<int>& nodes, QTreeView * view) const;
			void restoreExpandedState(QSet<int>& nodes, QTreeView * view) const;

			void addNewColor(const QString& colorName );

			void run( bool bAll );

			std::unique_ptr<ParamPropModel> _providerParamModel;
			std::unique_ptr<ParamPropModel> _processParamModelEditable;
			std::unique_ptr<ParamPropModel> _processParamModelCalculated;

			QAction *zoomInAct{};
			QAction *zoomOutAct{};
			QAction *normalSizeAct{};
			QAction *fitToWindowAct{};

			QAction * startAct = nullptr;
			QAction * startOnceAct = nullptr;
			QAction * stopAct = nullptr;

			QAction * addColor = nullptr;
			QAction * removeColor = nullptr;

			QAction * loadConfig = nullptr;
			QAction * saveConfig = nullptr;

			bool _bRunning = false;

			QLabel *_imageBox = nullptr;
			QScrollArea *_scrollArea = nullptr;
			double _scaleFactor = 1.0;
			QTimer * _updateStatsTimer = nullptr;

			QStringList _processParamsExpandedStatesList;

			QLabel * iconGeneral, * iconRecipe, * iconProvider, *iconAlgoRunner, *iconFramesHandled;
			QLabel * statusGeneral, *statusRecipeName, *statusFrameProv, *statusAlgoRunner, *statusFramesHandled;
			QProgressBar *statusProgressBar;
			QTimer _progressBarTimer;

			QPixmap  _greyLed, _greenLed;
		};
	}
}
