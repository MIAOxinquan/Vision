#include <QtWidgets/QMainWindow>
#include "ui_Vision.h"
#include "SmartEdit.h"
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

protected:
	void closeEvent(QCloseEvent* event)override;

private:
	Ui::VisionClass visionUi;
	QSplitter* globalSplitter;//左右竖直分割器
	QTextEdit* a;//test
	QTabWidget* plotTab, * codeTab;

public slots:
	void About();
	void showCurDateTime();

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
