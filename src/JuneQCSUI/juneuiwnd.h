#pragma once

#include "ui_juneuiwnd.h"
#include "paramPropModel.h"

#include <QProgressBar>
#include <QTimer>

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

			//void browseForFolder();
			void zoomIn();
			void zoomOut();
			void normalSize();
			void fitToWindow();
			void adjustScrollBar(QScrollBar* scrollBar, double factor);
			void scaleImage(double factor);
			void updateActions() const;

			void start();
			void stop();
			void onAboutToQuit();

			void onFrameProviderComboChanged(int index);
			void onAlgoHandlerComboChanged(int index);
			void updateStats() const;

			void onProviderPropChanged(QString propName, const QVariant& newVal);
			void onBatchPropChanged(QString propName, const QVariant& newVal);

			void onUpdateProcessParams();
			void onUpdateCalculatedParams();
	                void onBtnAddPropClicked() noexcept;
			void onBtnRemovePropClicked() noexcept;

			void onSaveConfig();
			void onLoadConfig();

			void onTimerTick();

		signals :

			void processingFinished();

		private :

			void init();
			void initUI();
			void initCore();
			void createActions();
			void createStatusBar();

			void enumerateFrameProviders() ;
			void enumerateAlgoHandlers() ;
			void enableUIForProcessing(bool bEnable);

			void initBatchParameters() const;

			void saveExpandedOnLevel(const QModelIndex& index, QSet<int> & nodes, QTreeView * view, int& iLevel ) const;
			void restoreExpandedOnLevel(const QModelIndex& index, QSet<int> & nodes, QTreeView * view, int& iLevel) const;
			void saveExpandedState(QSet<int>& nodes, QTreeView * view) const;
			void restoreExpandedState(QSet<int>& nodes, QTreeView * view) const;

			std::unique_ptr<ParamPropModel> _providerParamModel;
			std::unique_ptr<ParamPropModel> _processParamModelEditable;
			std::unique_ptr<ParamPropModel> _processParamModelCalculated;

			QAction *zoomInAct{};
			QAction *zoomOutAct{};
			QAction *normalSizeAct{};
			QAction *fitToWindowAct{};

			QAction * startAct = nullptr;
			QAction * stopAct = nullptr;

			QAction * loadConfig = nullptr;
			QAction * saveConfig = nullptr;

			bool _bRunning = false;

			QLabel *_imageBox = nullptr;
			QScrollArea *_scrollArea = nullptr;
			double _scaleFactor = 1.0;
			QTimer * _updateStatsTimer = nullptr;

			QStringList _processParamsExpandedStatesList;

			QLabel * statusGeneral, *statusFrameProv, *statusAlgoHandler, *statusFramesHandled, * statusFramesDropped;
			QProgressBar *statusProgressBar;
			QTimer _progressBarTimer;
		};
	}
}
