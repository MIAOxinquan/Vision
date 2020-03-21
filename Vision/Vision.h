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
	QSplitter* globalSplitter;//������ֱ�ָ���
	QTextEdit* a;//test
	QTabWidget* plotTab, * codeTab;

public slots:
	void About();
	void showCurDateTime();

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
	int Quit();
	void getCode();


};
