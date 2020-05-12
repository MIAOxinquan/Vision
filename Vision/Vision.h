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
	QStringList filePaths;//�Ѵ��ļ�·��
	QStringList fileNames;//�Ѵ��ļ�
	QList<PlotPad*>* pads;//plotָ���б�
	QList<SmartEdit*>* edits;//editָ���б�

	bool tabNotEmpty();
	//����ǰXML�ڵ� д�˺�����Ϊ�˵ݹ�
	void executeElementChilds(QDomElement e, PlotPad* pad, Block* parent, int _level);
private slots:
	//��д��Ϊ�����qt�Դ���������
	void Undo();
	void Redo();
	void Cut();
	void Copy();
	void Paste();
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
