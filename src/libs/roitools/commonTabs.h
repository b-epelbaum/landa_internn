#pragma once
#include <QString>

class QWidget;

class commonTabs
{
public:
	commonTabs();
	virtual ~commonTabs();

	static 	QString selectBitmapFile(QWidget * parentWidget, QString strPrompt, QString strLastFileKey);
};

