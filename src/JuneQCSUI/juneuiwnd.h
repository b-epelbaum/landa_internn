#pragma once

#include "ui_juneuiwnd.h"
#include "paramPropModel.h"

#include <QProgressBar>
#include <QTimer>
#include "common/june_exceptions.h"
#include "applog.h"
#include "onerunviewer.h"


namespace LandaJune
{
	namespace UI
	{
		enum RUN_MODE
		{
			RUN_UI = 0
			, RUN_NO_UI = 1
			, RUN_BATCH = 2
		};

		class JuneUIWnd : public QMainWindow
		{
			Q_OBJECT

		public:
			JuneUIWnd( QString runMode, QString logLevel, bool savelogToFile, QString strConfig, QWidget *parent = Q_NULLPTR);
			JuneUIWnd( RUN_MODE runMode, LOG_LEVEL logLevel, bool savelogToFile, QString strConfig, QWidget *parent = Q_NULLPTR);

			static RUN_MODE stringToMode (const QString& mode, bool& bOk);

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

			void initProcessParameters();
			void initProcessParametersUIMode();
			void initProcessParametersBatchMode();

			void saveExpandedOnLevel(const QModelIndex& index, QSet<int> & nodes, QTreeView * view, int& iLevel ) const;
			void restoreExpandedOnLevel(const QModelIndex& index, QSet<int> & nodes, QTreeView * view, int& iLevel) const;
			void saveExpandedState(QSet<int>& nodes, QTreeView * view) const;
			void restoreExpandedState(QSet<int>& nodes, QTreeView * view) const;

			void updateFrameProviderParamsView (int index);
			void updateAlgoRunnerParamsView ( int index);

			void addNewColor(const QString& colorName );

			bool isUIMode() const { return _runMode == RUN_UI; }
			bool isBatchMode() const { return _runMode == RUN_BATCH; }

			void handleException (LandaJune::BaseException& ex);

			void run( bool bAll );
			void runUIMode( bool bAll );
			void runBatchMode( bool bAll );

			std::unique_ptr<ParamPropModel> _providerParamModel;
			std::unique_ptr<ParamPropModel> _processParamModelEditable;
			std::unique_ptr<ParamPropModel> _processParamModelCalculated;

			oneRunViewer * _onRunViewer;

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

			RUN_MODE _runMode;
			LOG_LEVEL _logLevel;
			bool _savelogToFile;
			QString _strProcessingConfig;
		};
	}
}
