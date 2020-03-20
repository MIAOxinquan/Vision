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
	void About();
	void showCurDateTime();

protected:
	void closeEvent(QCloseEvent* event)override;

private:
	Ui::VisionClass visionUi;
	QSplitter* globalSplitter;//左右竖直分割器
	QTextEdit* a;//test
	QTabWidget* plotTab, * codeTab;

private slots:
	//大写是为避免和qt自带函数重名
	void Undo();
	void Redo();
	void Cut();
	void Copy();
	void Paste();
	void SelectAll();
	void Delete();
	void New();
	void Open();
	void Save();
	void SaveAll();
	void SaveAs();
	void Close();
	int Quit();
	void getCode();


};
