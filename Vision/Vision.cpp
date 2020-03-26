#include "Vision.h"
#include "ToolKit.h"
#include "PlotPad.h"
#include "SmartEdit.h"
/*
neighbour version means one of last few versions or current version
,for example versions 1.0, 1.01, 1.02 are regarded as neighbours
,if current version is 1.02
,so as neighbour version can be 1.0, 1.01 or 1.02 
*/
//neighbour version : 1.05

Vision::Vision(QWidget* parent)
	: QMainWindow(parent)
	, timer(new QTimer(this))
	, curDateTimeLabel(new QLabel())
	, globalSplitter(new QSplitter(Qt::Horizontal, this))
	, toolKit(new ToolKit(globalSplitter))
	, plotTab(new QTabWidget(globalSplitter))
	, editTab(new QTabWidget(globalSplitter))
{
	//初始化
	init();
	//启动计时器
	timer->start(1000);
	//槽函数
	connect(timer, SIGNAL(timeout()), this, SLOT(showCurDateTime()));
	connect(visionUi.actionUndo, SIGNAL(triggered()), this, SLOT(Undo()));
	connect(visionUi.actionRedo, SIGNAL(triggered()), this, SLOT(Redo()));
	connect(visionUi.actionCut, SIGNAL(triggered()), this, SLOT(Cut()));
	connect(visionUi.actionCopy, SIGNAL(triggered()), this, SLOT(Copy()));
	connect(visionUi.actionPaste, SIGNAL(triggered()), this, SLOT(Paste()));
	connect(visionUi.actionSelectAll, SIGNAL(triggered()), this, SLOT(SelectAll()));
	connect(visionUi.actionDelete, SIGNAL(triggered()), this, SLOT(Delete()));
	connect(visionUi.actionGetCode, SIGNAL(triggered()), this, SLOT(getCode()));
	connect(visionUi.actionNew, SIGNAL(triggered()), this, SLOT(New()));
	connect(visionUi.actionOpen, SIGNAL(triggered()), this, SLOT(Open()));
	connect(visionUi.actionSave, SIGNAL(triggered()), this, SLOT(Save()));
	connect(visionUi.actionSaveAll, SIGNAL(triggered()), this, SLOT(SaveAll()));
	connect(visionUi.actionSaveAs, SIGNAL(triggered()), this, SLOT(SaveAs()));
	connect(visionUi.actionClose, SIGNAL(triggered()), this, SLOT(Close()));
	connect(visionUi.actionQuit, SIGNAL(triggered()), this, SLOT(Quit()));
	connect(visionUi.actionAbout, SIGNAL(triggered()), this, SLOT(About()));

}
Vision::~Vision() {
	timer->stop();	delete timer; timer = NULL;
	delete curDateTimeLabel;	curDateTimeLabel = NULL;
	delete toolKit;	toolKit = NULL;
	if (plotTab->count() > 0)plotTab->clear();
	delete plotTab;	plotTab = NULL;
	if (editTab->count() > 0)editTab->clear();
	delete editTab;	editTab = NULL;
	delete globalSplitter; globalSplitter = NULL;
}
/*初始化*/
void Vision::init() {
	visionUi.setupUi(this);//ui定义上区：菜单栏和工具栏
	curDateTimeLabel->setAlignment(Qt::AlignRight);
	statusBar()->addPermanentWidget(curDateTimeLabel);

	//(index, stretch) 分割器内第index号框内元素stretch 0则不随窗体变化，1+则为比例系数
	//例如以下1号元素与2号元素宽度比为3：1
	int ratio[] = { 1,1,1 };
	for (int i = 0; i < 3; i++) {
		globalSplitter->setStretchFactor(i, ratio[i]);
	}
	toolKit->setMinimumWidth(60);
	toolKit->setMaximumWidth(200);
	plotTab->setMinimumWidth(200);
	editTab->setMinimumWidth(200);

	this->setCentralWidget(globalSplitter);
	this->setMinimumSize(900, 600);
	showCurDateTime();
	statusBar()->showMessage("initailizition finished!", 5000);

	/*plot&edit test*/
	for (int i = 0; i < 5; i++) {
		PlotPad* pad = new PlotPad();
		plotTab->addTab(pad, QString::number(i));
		SmartEdit* edit = new SmartEdit();
		editTab->addTab(edit, QString::number(i));
	}
	//加载qss
	QFile file("./Resources/qss/global.qss");
	file.open(QFile::ReadOnly);
	setStyleSheet(file.readAll());
	file.close();
}
/*时间标签*/
void Vision::showCurDateTime() {
	QString curDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd / hh:mm:ss");
	curDateTimeLabel->setText(curDateTime);
}
/*退出程序*/
int Vision::Quit() {
	int choose = -1;
	if (plotTab->count() > 0) {
		QMessageBox* msgBox = new QMessageBox(
			QMessageBox::Question
			, QString::fromLocal8Bit("退出")
			, QString::fromLocal8Bit("是否保存所有文件？")
			, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		msgBox->button(QMessageBox::Yes)->setText(QString::fromLocal8Bit("保存退出"));
		msgBox->button(QMessageBox::No)->setText(QString::fromLocal8Bit("直接退出"));
		msgBox->button(QMessageBox::Cancel)->setText(QString::fromLocal8Bit("取消"));
		QFile file("./Resources/qss/msgBox.qss");
		file.open(QFile::ReadOnly);
		msgBox->setStyleSheet(file.readAll());
		file.close();
		choose = msgBox->exec();
		if (QMessageBox::Yes == choose)  SaveAll();
		if (choose != QMessageBox::Cancel) {
			qApp->quit();
		}
	} else {
		qApp->quit();
	}
	return choose;
}
/*窗口按钮退出*/
void Vision::closeEvent(QCloseEvent* event) {
	if (QMessageBox::Cancel == Quit())event->ignore();
}
/*关于*/
void Vision::About() {
	QMessageBox* msgBox = new QMessageBox(
		QMessageBox::Information
		, QString::fromLocal8Bit("说明")
		, QString::fromLocal8Bit("\nVision开发团队:\n<Students  &WHU>\
                                                    \n@Code: 王浩旭 / 邹鑫 / 司若愚 / 杨肇欣 / 彭中园\
                                                    \n@Document: 杨天舒\
                                                    \n@Version: 1.02")
		, QMessageBox::Ok);
	msgBox->button(QMessageBox::Ok)->setText(QString::fromLocal8Bit("关闭"));
	QFile file("./Resources/qss/msgBox.qss");
	file.open(QFile::ReadOnly);
	msgBox->setStyleSheet(file.readAll());
	file.close();
	msgBox->exec();
}
/*撤回*/
void Vision::Undo() {

}
/*重做*/
void Vision::Redo() {

}
/*剪切*/
void Vision::Cut() {

}
/*复制*/
void Vision::Copy() {

}
/*粘贴*/
void Vision::Paste() {

}
/*全选*/
void Vision::SelectAll() {

}
/*删除*/
void Vision::Delete() {

}
/*复制代码*/
void Vision::getCode() {

}
/*新建文件*/
void Vision::New() {

}
/*打开文件*/
void Vision::Open() {

}
/*保存文件*/
void Vision::Save() {

}
/*保存全部*/
void Vision::SaveAll() {

}
/*导出，另存为*/
void Vision::SaveAs() {

}
/*关闭文件*/
void Vision::Close() {

}







