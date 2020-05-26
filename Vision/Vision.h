#include <QtWidgets>
#include <QDomDocument>
#include <QtXml>
#include "ui_Vision.h"

namespace Ui {
	class Vision;
}
class ToolKit;
class PlotPad;
class SmartEdit;
class TipLabel;
class Block;
class Vision : public QMainWindow
{
	Q_OBJECT

public:
	QLabel* curDateTimeLabel;
	TipLabel* curNodePathLabel;
	Vision(QWidget *parent = Q_NULLPTR);
	~Vision();

protected:
	void closeEvent(QCloseEvent* event)override;

private:
	Ui::VisionClass visionUi;
	QSplitter* globalSplitter;
	ToolKit* toolKit;
	QTabWidget* padTab, * editTab;
	QTimer* timer;
	QStringList filePaths;//已打开文件路径
	QStringList fileNames;//已打开文件
	QList<PlotPad*>* pads;//plot指针列表
	QList<SmartEdit*>* edits;//edit指针列表

	bool tabNotEmpty();
	//处理当前XML节点 写此函数是为了递归
	void executeElementChilds(QDomElement e, PlotPad* pad, Block* parent, int _level);
	//保存文件
	int fSave(QString filePath,int index);
private slots:
	//大写是为避免和qt自带函数重名
	void Undo();
	void Redo();
	void BackLevel();
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
