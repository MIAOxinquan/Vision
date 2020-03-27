#include <QtWidgets>
#include "ui_Vision.h"

namespace Ui {
	class Vision;
}
class ToolKit;
class PlotPad;
class SmartEdit;
class Vision : public QMainWindow
{
	Q_OBJECT

public:
	QLabel* curDateTimeLabel;
	Vision(QWidget *parent = Q_NULLPTR);
	~Vision();
	void init();

protected:
	void closeEvent(QCloseEvent* event)override;

private:
	Ui::VisionClass visionUi;
	QSplitter* globalSplitter;//������ֱ�ָ���
	ToolKit* toolKit;
	QTabWidget* plotTab, * editTab;
	QTimer* timer;

private slots:
	//��д��Ϊ�����qt�Դ���������
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
	void About();
	void showCurDateTime();
	void getCode();
	int Quit();
};
