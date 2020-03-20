#include "stdafx.h"
#include "Vision.h"

Vision::Vision(QWidget* parent)
	: QMainWindow(parent)
	, timer(new QTimer(this))
	, curDateTimeLabel(new QLabel())
	, globalSplitter(new QSplitter(Qt::Horizontal, this))
	, a(new QTextEdit("a", globalSplitter))
	, plotTab(new QTabWidget(globalSplitter))
	, codeTab(new QTabWidget(globalSplitter))
{
	visionUi.setupUi(this);//ui定义上区：菜单栏和工具栏
	curDateTimeLabel->setAlignment(Qt::AlignRight);
	visionUi.statusBar->addPermanentWidget(curDateTimeLabel);

	a->setMinimumWidth(200);
	plotTab->setMinimumWidth(200);
	codeTab->setMinimumWidth(200);
	//(index, stretch) 分割器内第index号框内元素stretch 0则不随窗体变化，1+则为比例系数
	//例如以下1号元素与2号元素宽度比为2：1
	globalSplitter->setStretchFactor(0, 0);
	globalSplitter->setStretchFactor(1, 2);
	globalSplitter->setStretchFactor(2, 1);


	this->setCentralWidget(globalSplitter);
	this->setMinimumSize(900, 540);
	timer->start(1000);
	
	//槽函数集群
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
	timer->stop();
	delete timer;
	delete curDateTimeLabel;
	delete a;
	delete plotTab;
	delete codeTab;
	delete globalSplitter;
}

void Vision::showCurDateTime() {
	QString curDateTime = QDate::currentDate().toString() + " / " + QTime::currentTime().toString();
	curDateTimeLabel->setText(curDateTime);
}
/*退出程序*/
int Vision::Quit() {
	int choose = -1;
	if (true) {
		QMessageBox* msgBox = new QMessageBox(
			QMessageBox::Question
			, QString::fromLocal8Bit("退出")
			, QString::fromLocal8Bit("是否保存所有文件？")
			, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		msgBox->button(QMessageBox::Yes)->setText(QString::fromLocal8Bit("保存退出"));
		msgBox->button(QMessageBox::No)->setText(QString::fromLocal8Bit("直接退出"));
		msgBox->button(QMessageBox::Cancel)->setText(QString::fromLocal8Bit("取消"));
		choose = msgBox->exec();

		if (QMessageBox::Yes == choose)  SaveAll();
		if (choose != QMessageBox::Cancel) {
			delete msgBox;
			qApp->quit();
		}
	}
	else {
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
	QMessageBox::information(this, QString::fromLocal8Bit("说明"), 
		QString::fromLocal8Bit("Vision为您服务！\n使用：\n开发团队：\n"));
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
























