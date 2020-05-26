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
version  4.08
1.bug: a click on a new PlotPad will lead to crash within func getContent(Block*block) of SmartEdit;
2.make type of func showContent(PlotPad*pad) from void to QString;
3.add func fSave() in Vision.cpp to support func Save()&SaveAll();
4.return func executeElementChilds(QDomElement e, PlotPad* pad, Block* parent) to original version as executeElementChilds(QDomElement e, PlotPad* pad, Block* parent, int _level);
5.saving or openninng file as XML is supported;

*.Block type will show children count if needed;
*.escape character not supported;
*.support two patterns, you can choose to show plotpad or not;
*.support two languages, you can choose C++ or Java;
*/
const QString version = "4.08";

/*Vision*/
Vision::Vision(QWidget* parent)
	: QMainWindow(parent)
	, globalSplitter(new QSplitter(Qt::Horizontal, this))
	, toolKit(new ToolKit(globalSplitter))
	, padTab(new QTabWidget(globalSplitter))
	, editTab(new QTabWidget(globalSplitter))
	, curDateTimeLabel(new QLabel())
	, curNodePathLabel(new TipLabel())
	, timer(new QTimer(this))
	, pads(new QList<PlotPad*>())
	, edits(new QList<SmartEdit*>())
{
	//全局窗口
	setMinimumSize(900, 600);
	visionUi.setupUi(this);//ui定义上区：菜单栏和工具栏
	//自定义中间区：工具框、图形区、代码框
	toolKit->setMinimumWidth(60);
	toolKit->setMaximumWidth(200);
	padTab->setMinimumWidth(200);
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
	//启动计时器
	timer->start(100);
	showCurDateTime();
	//加载qss
	loadStyleSheet(this, "global.qss");
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
	connect(padTab, SIGNAL(currentChanged(int)), this, SLOT(TabSyn_EditFollowPad(int)));
	connect(editTab, SIGNAL(currentChanged(int)), this, SLOT(TabSyn_PadFollowEdit(int)));
}
Vision::~Vision() {
	delete toolKit;	toolKit = Q_NULLPTR;
	while (tabNotEmpty()) {
		int lastIndex = editTab->count() - 1;
		editTab->removeTab(lastIndex);
		delete edits->at(lastIndex);
		padTab->removeTab(lastIndex);
		delete pads->at(lastIndex);
	}
	delete edits; edits = Q_NULLPTR;
	delete editTab;	editTab = Q_NULLPTR;
	delete pads; pads = Q_NULLPTR;
	delete padTab;	padTab = Q_NULLPTR;
	timer->stop();	delete timer; timer = Q_NULLPTR;
	delete curDateTimeLabel;	curDateTimeLabel = Q_NULLPTR;
	delete curNodePathLabel; curNodePathLabel = Q_NULLPTR;
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
	if (padTab->count() > 0) {
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
	return editTab->count() > 0;
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
	if (tabNotEmpty()) {
		int index = editTab->currentIndex();
		pads->at(index)->undo();
		pads->at(index)->update();
		edits->at(index)->showContent(pads->at(index));
	}
}
/*重做*/
void Vision::Redo() {
	if (tabNotEmpty()) {
		int index = editTab->currentIndex();
		pads->at(index)->redo();
		pads->at(index)->update();
		edits->at(index)->showContent(pads->at(index));
	}
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
		pads->at(padTab->currentIndex())->backLevel();
}
/*删除*/
void Vision::Delete() {
	if (tabNotEmpty()) {
		pads->at(padTab->currentIndex())->removeItem();
		visionUi.actionDelete->setEnabled(false);
	}
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
	int index = padTab->count();
	QString defaultName = "#untitled@" + QString::number(index);
	PlotPad* newPad = new PlotPad(new QGraphicsScene());
	newPad->title = defaultName;
	pads->append(newPad);
	padTab->addTab(newPad, defaultName);
	padTab->setCurrentIndex(index);
	SmartEdit* newEdit = new SmartEdit();
	edits->append(newEdit);
	editTab->addTab(newEdit, defaultName);
	editTab->setCurrentIndex(index);
	newPad->edit = newEdit;
	newPad->pathLabel = curNodePathLabel;
	newPad->actionUndo = visionUi.actionUndo;
	newPad->actionRedo = visionUi.actionRedo;
	newPad->actionDelete = visionUi.actionDelete;
	newPad->actionBackLevel = visionUi.actionBackLevel;
	newPad->ActionCtrl();
	curNodePathLabel->blockPath = newPad->getBlockPath();
	curNodePathLabel->setElidedText();
	if (curNodePathLabel->isHidden())curNodePathLabel->show();
	//用空字符串占位
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
	//文件路径
	QString filePath = QFileDialog::getOpenFileName(this,
		QString::fromLocal8Bit("打开文件"), DEFAULT_PATH, tr("XML (*.xml)"));
	QStringList names = filePath.split('/');
	//文件名字
	QString fileName = *(names.end() - 1);
	if (filePath != "") {
		if (filePaths.contains(filePath, Qt::CaseSensitive)) {//将焦点跳转到相同路径的tab
			int index = filePaths.indexOf(filePath);
			padTab->setCurrentIndex(index);
			editTab->setCurrentIndex(index);
		}
		else {//读取文件内容，解析后加载到plotTab和editTab

			//新建一个面板
			int index = padTab->count();
			QString defaultName = fileName;
			PlotPad* newPad = new PlotPad(new QGraphicsScene());
			newPad->title = defaultName;
			pads->append(newPad);
			padTab->addTab(newPad, defaultName);
			padTab->setCurrentIndex(index);
			SmartEdit* newEdit = new SmartEdit();
			edits->append(newEdit);
			editTab->addTab(newEdit, defaultName);
			editTab->setCurrentIndex(index);
			newPad->edit = newEdit;
			newPad->pathLabel = curNodePathLabel;
			newPad->actionUndo = visionUi.actionUndo;
			newPad->actionRedo = visionUi.actionRedo;
			newPad->actionDelete = visionUi.actionDelete;
			newPad->actionBackLevel = visionUi.actionBackLevel;
			newPad->ActionCtrl();
			curNodePathLabel->blockPath = newPad->getBlockPath();
			curNodePathLabel->setElidedText();
			if (curNodePathLabel->isHidden())curNodePathLabel->show();


			//打开或创建文件
			QFile file(filePath); //相对路径、绝对路径、资源路径都行
			if (!file.open(QFile::ReadOnly))
				return;

			QDomDocument doc;
			if (!doc.setContent(&file))
			{
				file.close();
				filePaths.append(filePath);
				statusBar()->showMessage(QString::fromLocal8Bit("打开文件") + filePath, 3000);
				return;
			}
			file.close();

			QDomElement root = doc.documentElement(); //返回根节点
			qDebug() << root.nodeName();
			QDomNode node = root.firstChild(); //获得第一个子节点
			Block* preBlock = Q_NULLPTR;
			int nowLevel = 1;
			while (!node.isNull())  //如果节点不空
			{
				if (node.isElement()) //如果节点是元素
				{
					QDomElement e = node.toElement(); //转换为元素，注意元素和节点是两个数据结构，其实差不多
					Block* nowBlock = new Block(e);
					newPad->addBlock(nowBlock);
					if (e.attribute("blockText").contains("*")) {
						newPad->setRoot(nowBlock);
					}
					//如果前面有个节点 且hasArrowLine就连接
					if (preBlock) {
						newPad->connectBlocks(preBlock, nowBlock, nowLevel, new QList<Record*>());
						preBlock = Q_NULLPTR;
					}

					if (e.attribute("hasArrowLine") == "1") {
						preBlock = nowBlock;
					}

					qDebug() << e.tagName() << " " << e.attribute("id"); //打印键值对，tagName和nodeName是一个东西
					//下一句负责处理当前节点的所有子节点 并 递归后续
					executeElementChilds(e, newPad, nowBlock, nowLevel + 1);

				}
				node = node.nextSibling(); //下一个兄弟节点,nextSiblingElement()是下一个兄弟元素，都差不多
			}
			if (1 == newPad->blockStack.count())
				newEdit->setPlainText(newEdit->showContent(newPad));
			else {
				newEdit->showContent(newPad->blockOnPath->last());
			}

			//以下待补充


			//将filePath添到已打开文件们中
			filePaths.append(filePath);
			statusBar()->showMessage(QString::fromLocal8Bit("打开文件") + filePath, 3000);
		}
	}
}

/*处理当前XML节点的子节点*/
void Vision::executeElementChilds(QDomElement e, PlotPad* newPad, Block* parent, int _level) {
	QDomNodeList list = e.childNodes();
	Block* preBlock = Q_NULLPTR;
	for (int i = 0; i < list.count(); i++) //遍历子元素，count和size都可以用,可用于标签数计数
	{
		QDomNode n = list.at(i);
		if (n.isElement()) {
			qDebug() << n.nodeName() << ":" << n.toElement().text();
			QDomElement childElement = n.toElement();
			if (childElement.tagName() == "Block") {
				Block* nowBlock = new Block(childElement);
				newPad->addBlockIntoBlock(parent, nowBlock, Q_NULLPTR);
				if (childElement.attribute("blockText").contains("*")) {
					parent->setChildRoot(nowBlock);
				}
				//如果前面有个节点 且hasArrowLine就连接
				if (preBlock) {
					newPad->connectBlocks(preBlock, nowBlock, _level, new QList<Record*>())->hide(); //把这个连接的箭头隐藏掉
					preBlock = Q_NULLPTR;
				}
				if (childElement.attribute("hasArrowLine") == "1") {
					preBlock = nowBlock;
				}

				//处理当前节点的子节点们
				executeElementChilds(childElement, newPad, nowBlock, _level + 1);
			}
		}

	}
}
/*保存文件*/
void Vision::Save() { //存入txt
	if (padTab->count() > 0) {
		//QRegExp rx("&untitled@\S*");	
		int index = editTab->currentIndex();
		QString filePath = filePaths.at(index);
		if (fSave(filePath, index) == 1)
			statusBar()->showMessage(QString::fromLocal8Bit("成功保存至") + filePath, 3000);
	}
}
/*保存全部*/
void Vision::SaveAll() {
	if (padTab->count() > 0) {
		int fileCounts = filePaths.count();
		int n = 0;	//用来判断是不是所有文件都保存成功了
		for (int i = 0; i < fileCounts; i++) {
			QString filePath = filePaths.at(i);
			n += fSave(filePath, i);
		}
		if (n == fileCounts)
			statusBar()->showMessage(QString::fromLocal8Bit("全部保存成功"), 3000);
	}
}
/*保存文件具体操作*/
int Vision::fSave(QString filePath, int index) {
	if (filePath.isEmpty()) {
		QString defName = padTab->tabText(index);
		filePath = QFileDialog::getSaveFileName(this,
			QString::fromLocal8Bit("保存") + defName, DEFAULT_PATH + tr("/") + defName, "Xml(*.xml);;Txt(*.txt)");
		if (filePath.isEmpty())
			return -1;
		filePaths[index] = filePath;
	}
	QFileInfo fileInfo = QFileInfo(filePath);
	QString fsuffix = fileInfo.suffix();
	if (fsuffix == "txt") {	//存txt
		QFile file(filePath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			return -1;
		QTextStream out(&file);
		out.setCodec(QTextCodec::codecForName("utf-8"));
		PlotPad* pad = pads->at(index);
		QString text = pad->edit->showContent(pad);
		out << text;
		file.close();
	}
	else if (fsuffix == "xml") {	//存xml
		pads->at(index)->outport(filePath);
	}
	//将路径保存至已有文件路径中
	filePaths.replace(index, filePath);
	padTab->setTabText(index, fileInfo.fileName());
	editTab->setTabText(index, fileInfo.fileName());
	return 1;
}
/*导出，另存为*/
void Vision::SaveAs() {
	if (tabNotEmpty()) {
		int index = editTab->currentIndex();
		pads->at(index)->outport("test.xml");
	}
}
/*关闭文件*/
void Vision::Close() {
	if (tabNotEmpty()) {
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
		pads->removeAt(index);
		edits->removeAt(index);
		if (!tabNotEmpty()) {
			visionUi.actionSave->setEnabled(false);
			visionUi.actionSaveAll->setEnabled(false);
			visionUi.actionSaveAs->setEnabled(false);
			visionUi.actionClose->setEnabled(false);
			visionUi.actionGetCode->setEnabled(false);
			visionUi.actionUndo->setEnabled(false);
			visionUi.actionRedo->setEnabled(false);
			visionUi.actionDelete->setEnabled(false);
			visionUi.actionBackLevel->setEnabled(false);
		}
	}
}
/*edit和plot绑定*/
void Vision::TabSyn_EditFollowPad(int index) {
	if (index >= 0 && index != editTab->currentIndex()) {
		editTab->setCurrentIndex(index);
		PlotPad* pad = pads->at(index);
		pad->ActionCtrl();
		curNodePathLabel->blockPath = pad->getBlockPath();
		curNodePathLabel->setElidedText();
	}
}
void Vision::TabSyn_PadFollowEdit(int index) {
	if (index >= 0&&index!=padTab->currentIndex()) { 
		padTab->setCurrentIndex(index);
		PlotPad* pad = pads->at(index);
		pad->ActionCtrl();
		curNodePathLabel->blockPath = pad->getBlockPath();
		curNodePathLabel->setElidedText();
	}
}