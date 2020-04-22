#include <QtWidgets>
#include "ui_Vision.h"

namespace Ui {
	class Vision;
}
class ToolKit;
class PlotPad;
class SmartEdit;

//class TipLabel :public QLabel {
//protected:
//	void mouseMoveEvent(QMouseEvent* event)override;
//};
class Vision : public QMainWindow
{
	Q_OBJECT

public:
	QLabel* curDateTimeLabel, * curNodePathLabel;
	Vision(QWidget *parent = Q_NULLPTR);
	~Vision();

protected:
	void closeEvent(QCloseEvent* event)override;

private:
	Ui::VisionClass visionUi;
	QSplitter* globalSplitter;//左右竖直分割器
	ToolKit* toolKit;
	QTabWidget* plotTab, * editTab;
	QTimer* timer;
	QStringList filePaths;//已打开文件路径
	QStringList fileNames;//已打开文件
	QList<PlotPad*>* plots;//plot指针列表
	QList<SmartEdit*>* edits;//edit指针列表

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
	void About();
	void Default();
	void NoPlot();
	void Cpp();
	void Java();
	void showCurDateTime();
	void getCode();
	int Quit();
	void TabSyn_EditFollowPad(int index);
	void TabSyn_PadFollowEdit(int index); 
	
};
