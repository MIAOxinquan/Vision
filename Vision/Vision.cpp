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
version 3.17
1.ArrowLine now supports strong ajust;
2.PlotPad now supports auto ArrowLine;
3.remove Action "SelectAll", add Action "BackLevel";
4.add class RecordObject with .h & .cpp;
5.make logic of ToolBar Buttons more clear;

*.escape character not supported;
*.support two patterns, you can choose to show plotpad or not;
*.support two languages, you can choose C++ or Java;
*/
const QString version = "3.17";

/*Vision*/
Vision::Vision(QWidget* parent)
	: QMainWindow(parent)
	, globalSplitter(new QSplitter(Qt::Horizontal, this))
	, toolKit(new ToolKit(globalSplitter))
	, plotTab(new QTabWidget(globalSplitter))
	, editTab(new QTabWidget(globalSplitter))
	, curDateTimeLabel(new QLabel())
	, curNodePathLabel(new TipLabel())
	, timer(new QTimer(this))
	, plots(new QList<PlotPad*>())
	, edits(new QList<SmartEdit*>())
{
	//全局窗口
	setMinimumSize(900, 600);
	visionUi.setupUi(this);//ui定义上区：菜单栏和工具栏
	//自定义中间区：工具框、图形区、代码框
	toolKit->setMinimumWidth(60);
	toolKit->setMaximumWidth(200);
	plotTab->setMinimumWidth(200);
	editTab->setMinimumWidth(200);
	//自定义下区：状态栏、时间标签、节点路径标签
	statusBar()->addPermanentWidget(curNodePathLabel);
	curNodePathLabel->hide();
	statusBar()->addPermanentWidget(curDateTimeLabel);
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
	//启动计时器
	timer->start(100);
	showCurDateTime();
	statusBar()->showMessage(QString::fromLocal8Bit("Vision为您服务"), 5000);
	//槽函数
	connect(timer, SIGNAL(timeout()), this, SLOT(showCurDateTime()));
	connect(visionUi.actionUndo, SIGNAL(triggered()), this, SLOT(Undo()));
	connect(visionUi.actionRedo, SIGNAL(triggered()), this, SLOT(Redo()));
	connect(visionUi.actionCut, SIGNAL(triggered()), this, SLOT(Cut()));
	connect(visionUi.actionCopy, SIGNAL(triggered()), this, SLOT(Copy()));
	connect(visionUi.actionPaste, SIGNAL(triggered()), this, SLOT(Paste()));
	connect(visionUi.actionBackLevel, SIGNAL(triggered()), this, SLOT(BackLevel()));
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
	connect(plotTab, SIGNAL(currentChanged(int)), this, SLOT(TabSyn_EditFollowPad(int)));
	connect(editTab, SIGNAL(currentChanged(int)), this, SLOT(TabSyn_PadFollowEdit(int)));
}
Vision::~Vision() {
	timer->stop();	delete timer; timer = Q_NULLPTR;
	delete curDateTimeLabel;	curDateTimeLabel = Q_NULLPTR;
	delete curNodePathLabel; curNodePathLabel = Q_NULLPTR;
	delete toolKit;	toolKit = Q_NULLPTR;
	while (plotTab->count() > 0) {
		int lastIndex = plotTab->count() - 1;
		plotTab->removeTab(lastIndex);
		editTab->removeTab(lastIndex);
	}
	delete plotTab;	plotTab = Q_NULLPTR;
	delete editTab;	editTab = Q_NULLPTR;
	delete globalSplitter; globalSplitter = Q_NULLPTR;
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
/*关闭事件-窗口按钮退出*/
void Vision::closeEvent(QCloseEvent* event) {
	if (QMessageBox::Cancel == Quit())event->ignore();
}
/*tab不空*/
bool Vision::tabNotEmpty() {
	return editTab->count() >= 0;
}
/*默认模式*/
void Vision::Default() {
	if (tabNotEmpty()) {

	}
}
/*无图模式*/
void Vision::NoPlot() {
	if (tabNotEmpty()) {

	}
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
	if(tabNotEmpty())
		plots->at(plotTab->currentIndex())->undo();
}
/*重做*/
void Vision::Redo() {
	if (tabNotEmpty())
		plots->at(plotTab->currentIndex())->redo();
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
void Vision::BackLevel() {
	if (tabNotEmpty())
		plots->at(plotTab->currentIndex())->backLevel();
}
/*删除*/
void Vision::Delete() {
	if (tabNotEmpty())
		plots->at(plotTab->currentIndex())->deleteItem();
}
/*取码*/
void Vision::getCode() {
	if (tabNotEmpty()) {
		int curIndex = editTab->currentIndex();
		SmartEdit* curEdit = edits->at(curIndex);
		curEdit->selectAll();
		curEdit->copy();
	}
}
/*新建文件*/
void Vision::New() {
	if (!tabNotEmpty()) {
		visionUi.actionSave->setEnabled(true);
		visionUi.actionSaveAll->setEnabled(true);
		visionUi.actionSaveAs->setEnabled(true);
		visionUi.actionClose->setEnabled(true);
		visionUi.actionGetCode->setEnabled(true);
	}
	int index = plotTab->count();
	QString defaultName = "#untitled@" + QString::number(index);
	PlotPad* newPad = new PlotPad(new QGraphicsScene());
	plots->append(newPad);
	plotTab->addTab(newPad, defaultName);
	plotTab->setCurrentIndex(index);
	SmartEdit* newEdit = new SmartEdit();
	edits->append(newEdit);
	editTab->addTab(newEdit, defaultName);
	editTab->setCurrentIndex(index);
	newPad->edit = newEdit;
	newPad->pathLabel = curNodePathLabel;
	curNodePathLabel->setElidedText(newPad->getNodesPath());
	if (curNodePathLabel->isHidden())curNodePathLabel->show();
	//以下待补充
	filePaths.append("");
}
/*打开文件*/
void Vision::Open() {
	if (!tabNotEmpty()) {
		visionUi.actionSave->setEnabled(true);
		visionUi.actionSaveAll->setEnabled(true);
		visionUi.actionSaveAs->setEnabled(true);
		visionUi.actionClose->setEnabled(true);
		visionUi.actionGetCode->setEnabled(true);
	}
	QString filePath = QFileDialog::getOpenFileName(this,
		QString::fromLocal8Bit("打开文件"), DEFAULT_PATH, tr("XML (*.xml)"));
	if (filePath != "" ) {
		if (filePaths.contains(filePath, Qt::CaseSensitive)) {//将焦点跳转到相同路径的tab
			int index = filePaths.indexOf(filePath);
			plotTab->setCurrentIndex(index);
			editTab->setCurrentIndex(index);
		}
		else {//读取文件内容，解析后加载到plotTab和editTab

			statusBar()->showMessage(QString::fromLocal8Bit("打开文件") + filePath, 3000);
		}
	}
}
/*保存文件*/
void Vision::Save() { //存入txt
	if (plotTab->count() > 0) {
		//QRegExp rx("&untitled@\S*");	
		int index = editTab->currentIndex();
		QString filePath = filePaths.at(index);
		if (filePath.isEmpty()) {
			filePath = QFileDialog::getSaveFileName(this,
				QString::fromLocal8Bit("保存文件"), DEFAULT_PATH, tr("XML (*.xml)"));
			filePaths[index] = filePath;
		}
		QFile file(filePath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			return;
		QTextStream out(&file);
		out.setCodec(QTextCodec::codecForName("utf-8"));
		QString text = edits->at(index)->toPlainText();
		out << text;
		file.close();
		statusBar()->showMessage(QString::fromLocal8Bit("成功保存至") + filePath, 3000);
	}
	//存XML
}
/*保存全部*/
void Vision::SaveAll() {
	if (plotTab->count() > 0) {
		int fileCounts = filePaths.count();
		for (int i = 0; i < fileCounts; i++) {
			if (filePaths.at(i).isEmpty()) {
				QString label = editTab->tabText(i);
				QString filePath = QFileDialog::getSaveFileName(this,
					QString::fromLocal8Bit("保存") + label, DEFAULT_PATH, tr("XML (*.xml)"));
				filePaths[i] = filePath;
			}
			QFile file(filePaths.at(i));
			if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
				return;
			QTextStream out(&file);
			out.setCodec(QTextCodec::codecForName("utf-8"));
			out << edits->at(i)->toPlainText();
			file.close();
		}
		statusBar()->showMessage(QString::fromLocal8Bit("全部保存成功"), 3000);
	}
}
/*导出，另存为*/
void Vision::SaveAs() {

}
/*关闭文件*/
void Vision::Close() {
	if (plotTab->count() > 0) {
		int index = editTab->currentIndex();
		QString filePath = filePaths.at(index);
		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
		QTextStream in(&file);
		in.setCodec(QTextCodec::codecForName("utf-8"));
		QString all = in.readAll();
		file.close();
		QString text = edits->at(index)->toPlainText();
		if (QString::compare(all, text) != 0) {
			QMessageBox* msgBox = new QMessageBox(
				QMessageBox::Question
				, QString::fromLocal8Bit("关闭")
				, QString::fromLocal8Bit("是否保存当前文件？")
				, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
			msgBox->button(QMessageBox::Yes)->setText(QString::fromLocal8Bit("保存退出"));
			msgBox->button(QMessageBox::No)->setText(QString::fromLocal8Bit("直接退出"));
			msgBox->button(QMessageBox::Cancel)->setText(QString::fromLocal8Bit("取消"));
			int choose = msgBox->exec();
			if (QMessageBox::Yes == choose)
				Save();
			if (QMessageBox::Cancel == choose)
				return;
		}
		filePaths.removeAt(index);
		plots->removeAt(index);
		edits->removeAt(index);
		if (!tabNotEmpty()) {
			visionUi.actionSave->setEnabled(false);
			visionUi.actionSaveAll->setEnabled(false);
			visionUi.actionSaveAs->setEnabled(false);
			visionUi.actionClose->setEnabled(false);
			visionUi.actionGetCode->setEnabled(false);
		}
	}
}
/*edit和plot绑定*/
void Vision::TabSyn_EditFollowPad(int index) {
	if (index >= 0) { editTab->setCurrentIndex(index); }
}
void Vision::TabSyn_PadFollowEdit(int index) {
	if (index >= 0) { plotTab->setCurrentIndex(index); }
}