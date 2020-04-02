#include "global.h"
#include "Vision.h"
#include "ToolKit.h"
#include "PlotPad.h"
#include "SmartEdit.h"
/*
neighbour version means one of several lastest versions
for example, versions 1.0, 1.01, 1.02 are regarded as neighbour group, if current version is 1.02
,so as neighbour version can be 1.0 or 1.01 or 1.02

neighbour@
version 1.47
1.start constructing PlotPad;
2.support highlightRegExp singleLineComment  in darkGreen, quotation and character in brown;
3.cancel smartEdit rightButton menu;
4.add menu Setting, within menu PlotPattern and menu Language;
5.restruct C++ keys_cpp with a lot new items;
6.make multilinesComment highlight in darkGreen;
7.highlightblock displayed normal when a string contains repeated keys;
8.escape character not supported;
9.support two patterns, you can choose to show plotpad or not;
10.support two languages, you can choose C++ or Java;
*/
const QString version = "1.47";

Vision::Vision(QWidget* parent)
	: QMainWindow(parent)
	, globalSplitter(new QSplitter(Qt::Horizontal, this))
	, toolKit(new ToolKit(globalSplitter))
	, plotTab(new QTabWidget(globalSplitter))
	, editTab(new QTabWidget(globalSplitter))
	, curDateTimeLabel(new QLabel())
	, timer(new QTimer(this))
{
	//初始化
	init();
	//启动计时器
	timer->start(1000);
	showCurDateTime();
	statusBar()->showMessage("initailizition finished!", 5000);
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
	connect(visionUi.actionDefault, SIGNAL(triggered()), this, SLOT(Default()));
	connect(visionUi.actionNoPlot, SIGNAL(triggered()), this, SLOT(NoPlot()));
	connect(visionUi.actionAbout, SIGNAL(triggered()), this, SLOT(Cpp()));
	connect(visionUi.actionJava, SIGNAL(triggered()), this, SLOT(Java()));


}
Vision::~Vision() {
	timer->stop();	delete timer; timer = Q_NULLPTR;
	delete curDateTimeLabel;	curDateTimeLabel = Q_NULLPTR;
	delete toolKit;	toolKit = Q_NULLPTR;
	if (plotTab->count() > 0)plotTab->clear();
	delete plotTab;	plotTab = Q_NULLPTR;
	if (editTab->count() > 0)editTab->clear();
	delete editTab;	editTab = Q_NULLPTR;
	delete globalSplitter; globalSplitter = Q_NULLPTR;
}
/*初始化*/
void Vision::init() {
	//全局窗口
	setMinimumSize(900, 600);
	visionUi.setupUi(this);//ui定义上区：菜单栏和工具栏
	//自定义中间区：工具框、图形区、代码框
	toolKit->setMinimumWidth(60);
	toolKit->setMaximumWidth(200);
	plotTab->setMinimumWidth(200);
	editTab->setMinimumWidth(200);
	//自定义下区：状态栏、时间标签
	curDateTimeLabel->setAlignment(Qt::AlignRight);
	statusBar()->addPermanentWidget(curDateTimeLabel);
	//变量初始化
	PlotPad* pad = new PlotPad();
	plotTab->addTab(pad, "plotPad");
	SmartEdit* edit = new SmartEdit();
	editTab->addTab(edit, "smartEdit");

	/*
	(index, stretch) 分割器内第index号框内元素stretch 0则不随窗体变化，1+则为比例系数
	例如以下1号元素与2号元素宽度比为3：1
	*/
	for (int i = 0; i < 3; i++) {
		globalSplitter->setStretchFactor(i, 1);
	}
	setCentralWidget(globalSplitter);//放在布局最后
	//加载qss
	loadStyleSheet(this, "global.qss");
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
		loadStyleSheet(msgBox, "msgBox.qss");
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
/*默认模式*/
void Vision::Default() {
	globalSplitter->widget(1)->show();
}
/*无图模式*/
void Vision::NoPlot() {
	globalSplitter->widget(1)->close();
}
/*C++*/
void Vision::Cpp() {

}
/*Java*/
void Vision::Java() {

}
/*关于*/
void Vision::About() {
	QMessageBox* msgBox = new QMessageBox(
		QMessageBox::Information
		, QString::fromLocal8Bit("说明")
		, QString::fromLocal8Bit("\nVision开发团队:\n<Students  &WHU>\
                                                    \n@Code: 王浩旭 / 邹鑫 / 司若愚 / 杨肇欣 / 彭中园\
                                                    \n@Document: 杨天舒\
                                                    \n@Version: ") + version
		, QMessageBox::Ok);
	msgBox->button(QMessageBox::Ok)->setText(QString::fromLocal8Bit("关闭")); 
	loadStyleSheet(msgBox, "msgBox.qss");
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







