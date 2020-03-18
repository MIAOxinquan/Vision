#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Vision.h"
namespace Ui {
	class Vision;
}


class Vision : public QMainWindow
{
	Q_OBJECT

public:
	Vision(QWidget *parent = Q_NULLPTR);
	~Vision();
	QLabel* curDateTimeLabel;
	QTimer* timer;
public slots:
	void showCurDateTime();
private:
	Ui::VisionClass visionUi;
	QSplitter* horizonSplitter;//左右竖直分割器
	QTextEdit* a, * b, * c;//test

private slots://槽函数声明


};
