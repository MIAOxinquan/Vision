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
version  4.16
1.func Close(), Open() and Save() are basically supported, and qss for func Close() is available;
2.now content of focusedBlock will refresh properly;
3.block id now can be inserted in content properly;
4.bug that id-repeat-inserting when open xml files fixed;
5.add attribute "indexTotal" into xml's root "pad";
6.remove Action Copy, Paste and Cut;
7.bug that a xml just opened loss its operatility fixed;
8.now support two patterns, you can choose to show plotpad or not;

*.Block type will show children count if needed;
*.escape character not supported;
*.support two languages, you can choose C++ or Java;
*/
const QString version = "4.16";

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
	//ȫ�ִ���
	setMinimumSize(900, 600);
	visionUi.setupUi(this);//ui�����������˵����͹�����
	//�Զ����м��������߿�ͼ�����������
	toolKit->setMinimumWidth(60);
	toolKit->setMaximumWidth(200);
	padTab->setMinimumWidth(200);
	editTab->setMinimumWidth(200);
	//�Զ���������״̬����ʱ���ǩ���ڵ�·����ǩ
	statusBar()->addPermanentWidget(curNodePathLabel);
	curNodePathLabel->hide();
	statusBar()->addPermanentWidget(curDateTimeLabel);
	/*
	(index, stretch) �ָ����ڵ�index�ſ���Ԫ��stretch 0���洰��仯��1+��Ϊ����ϵ��
	��������1��Ԫ����2��Ԫ�ؿ�ȱ�Ϊ3��1
	*/
	for (int i = 0; i < 3; i++) {
		globalSplitter->setStretchFactor(i, 1);
	}
	setCentralWidget(globalSplitter);//���ڲ������
	//������ʱ��
	timer->start(100);
	showCurDateTime();
	//����qss
	loadStyleSheet(this, "global.qss");
	statusBar()->showMessage(QString::fromLocal8Bit("VisionΪ������"), 5000);
	//�ۺ���
	connect(timer, SIGNAL(timeout()), this, SLOT(showCurDateTime()));
	connect(visionUi.actionUndo, SIGNAL(triggered()), this, SLOT(Undo()));
	connect(visionUi.actionRedo, SIGNAL(triggered()), this, SLOT(Redo()));
	/*connect(visionUi.actionCut, SIGNAL(triggered()), this, SLOT(Cut()));
	connect(visionUi.actionCopy, SIGNAL(triggered()), this, SLOT(Copy()));
	connect(visionUi.actionPaste, SIGNAL(triggered()), this, SLOT(Paste()));*/
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

/*ʱ���ǩ*/
void Vision::showCurDateTime() {
	QString curDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd / hh:mm:ss");
	curDateTimeLabel->setText(curDateTime);
}
/*�˳�����*/
int Vision::Quit() {
	int choose = -1;
	if (padTab->count() > 0) {
		QMessageBox* msgBox = new QMessageBox(
			QMessageBox::Question
			, QString::fromLocal8Bit("�˳�")
			, QString::fromLocal8Bit("�Ƿ񱣴������ļ���")
			, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		msgBox->button(QMessageBox::Yes)->setText(QString::fromLocal8Bit("�����˳�"));
		msgBox->button(QMessageBox::No)->setText(QString::fromLocal8Bit("ֱ���˳�"));
		msgBox->button(QMessageBox::Cancel)->setText(QString::fromLocal8Bit("ȡ��"));
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
/*�ر��¼�-���ڰ�ť�˳�*/
void Vision::closeEvent(QCloseEvent* event) {
	if (QMessageBox::Cancel == Quit())event->ignore();
}
/*tab����*/
bool Vision::tabNotEmpty() {
	return editTab->count() > 0;
}
/*Ĭ��ģʽ*/
void Vision::Default() {
	if (tabNotEmpty()) {
		globalSplitter->widget(1)->hide();
		visionUi.actionDefault->setEnabled(false);
		visionUi.actionNoPlot->setEnabled(true);
	}
}
/*��ͼģʽ*/
void Vision::NoPlot() {
	if (tabNotEmpty()) {
		globalSplitter->widget(1)->show();
		visionUi.actionNoPlot->setEnabled(false);
		visionUi.actionDefault->setEnabled(true);
	}
}
/*C++*/
void Vision::Cpp() {

}
/*Java*/
void Vision::Java() {

}
/*����*/
void Vision::About() {
	QMessageBox* msgBox = new QMessageBox(
		QMessageBox::Information
		, QString::fromLocal8Bit("˵��")
		, QString::fromLocal8Bit("\nVision�����Ŷ�:\n<Students  &WHU>\
                                                    \n@Code: ������ / ���� / ˾���� / ������ / ����԰\
                                                    \n@Document: ������\
                                                    \n@Version: ") + version
		, QMessageBox::Ok);
	msgBox->button(QMessageBox::Ok)->setText(QString::fromLocal8Bit("�ر�")); 
	loadStyleSheet(msgBox, "msgBox.qss");
	msgBox->exec();
}
/*����*/
void Vision::Undo() {
	if (tabNotEmpty()) {
		int index = editTab->currentIndex();
		PlotPad* curPad = pads->at(index);
		SmartEdit* curEdit = edits->at(index);
		curPad->undo();
		curPad->update();
		curEdit->setPlainText(curEdit->showContent(curPad));
	}
}
/*����*/
void Vision::Redo() {
	if (tabNotEmpty()) {
		int index = editTab->currentIndex();
		PlotPad* curPad = pads->at(index);
		SmartEdit* curEdit = edits->at(index);
		curPad->redo();
		curPad->update();
		curEdit->setPlainText(curEdit->showContent(curPad));
	}
}
/*ȫѡ*/
void Vision::BackLevel() {
	if (tabNotEmpty())
		pads->at(padTab->currentIndex())->backLevel();
}
/*ɾ��*/
void Vision::Delete() {
	if (tabNotEmpty()) {
		pads->at(padTab->currentIndex())->removeItem();
		visionUi.actionDelete->setEnabled(false);
	}
}
/*ȡ��*/
void Vision::getCode() {
	if (tabNotEmpty()) {
		int curIndex = editTab->currentIndex();
		SmartEdit* curEdit = edits->at(curIndex);
		curEdit->selectAll();
		curEdit->copy();
	}
}
/*�½��ļ�*/
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
	newEdit->pad = newPad;
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
	//�ÿ��ַ���ռλ
	filePaths.append("");
}
/*���ļ�*/
void Vision::Open() {
	if (!tabNotEmpty()) {
		visionUi.actionSave->setEnabled(true);
		visionUi.actionSaveAll->setEnabled(true);
		visionUi.actionSaveAs->setEnabled(true);
		visionUi.actionClose->setEnabled(true);
		visionUi.actionGetCode->setEnabled(true);
	}
	//�ļ�·��
	QString filePath = QFileDialog::getOpenFileName(this,
		QString::fromLocal8Bit("���ļ�"), DEFAULT_PATH, tr("XML (*.xml)"));
	QStringList names = filePath.split('/');
	//�ļ�����
	QString fileName = *(names.end() - 1);
	if (filePath != "") {
		if (filePaths.contains(filePath, Qt::CaseSensitive)) {//��������ת����ͬ·����tab
			int index = filePaths.indexOf(filePath);
			padTab->setCurrentIndex(index);
			editTab->setCurrentIndex(index);
		}
		else {//��ȡ�ļ����ݣ���������ص�plotTab��editTab

			//�½�һ�����
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
			newEdit->pad = newPad;
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


			//�򿪻򴴽��ļ�
			QFile file(filePath); //���·��������·������Դ·������
			if (!file.open(QFile::ReadOnly))
				return;

			QDomDocument doc;
			if (!doc.setContent(&file))
			{
				file.close();
				filePaths.append(filePath);
				statusBar()->showMessage(QString::fromLocal8Bit("���ļ�") + filePath, 3000);
				return;
			}
			file.close();

			QDomElement root = doc.documentElement(); //���ظ��ڵ�
			qDebug() << root.nodeName();
			QDomNode node = root.firstChild(); //��õ�һ���ӽڵ�
			Block* preBlock = Q_NULLPTR;
			int nowLevel = 1;
			while (!node.isNull())  //����ڵ㲻��
			{
				if (node.isElement()) //����ڵ���Ԫ��
				{
					QDomElement e = node.toElement(); //ת��ΪԪ�أ�ע��Ԫ�غͽڵ����������ݽṹ����ʵ���
					Block* nowBlock = new Block(e);
					newPad->isNew = false;
					newPad->addBlock(nowBlock);
					newPad->isNew - true;
					newPad->indexTotal = root.attribute("indexTotal").toInt();
					if (e.attribute("blockText").contains("*")) {
						newPad->setRoot(nowBlock);
					}
					//���ǰ���и��ڵ� ��hasArrowLine������
					if (preBlock) {
						newPad->connectBlocks(preBlock, nowBlock, nowLevel, new QList<Record*>());
						preBlock = Q_NULLPTR;
					}

					if (e.attribute("hasArrowLine") == "1") {
						preBlock = nowBlock;
					}

					qDebug() << e.tagName() << " " << e.attribute("id"); //��ӡ��ֵ�ԣ�tagName��nodeName��һ������
					//��һ�为����ǰ�ڵ�������ӽڵ� �� �ݹ����
					newPad->isNew = false;
					executeElementChilds(e, newPad, nowBlock, nowLevel + 1);
					newPad->isNew = true;
				}
				node = node.nextSibling(); //��һ���ֵܽڵ�,nextSiblingElement()����һ���ֵ�Ԫ�أ������
			}
			if (1 == newPad->blockStack.count()) {
				newEdit->setPlainText(newEdit->showContent(newPad));
			}
			else {
				newEdit->showContent(newPad->blockOnPath->last());
			}

			//���´�����


			//��filePath���Ѵ��ļ�����
			filePaths.append(filePath);
			statusBar()->showMessage(QString::fromLocal8Bit("���ļ�") + filePath, 3000);
		}
	}
}

/*����ǰXML�ڵ���ӽڵ�*/
void Vision::executeElementChilds(QDomElement e, PlotPad* newPad, Block* parent, int _level) {
	QDomNodeList list = e.childNodes();
	Block* preBlock = Q_NULLPTR;
	for (int i = 0; i < list.count(); i++) //������Ԫ�أ�count��size��������,�����ڱ�ǩ������
	{
		QDomNode n = list.at(i);
		if (n.isElement()) {
			qDebug() << n.nodeName() << ":" << n.toElement().text();
			QDomElement childElement = n.toElement();
			if (childElement.tagName() == "Block") {
				//newPad->isNew = false;
				Block* nowBlock = new Block(childElement);
				newPad->addBlockIntoBlock(parent, nowBlock, Q_NULLPTR);
				//newPad->isNew = true;
				if (childElement.attribute("blockText").contains("*")) {
					parent->setChildRoot(nowBlock);
				}
				//���ǰ���и��ڵ� ��hasArrowLine������
				if (preBlock) {
					newPad->connectBlocks(preBlock, nowBlock, _level, new QList<Record*>())->hide(); //��������ӵļ�ͷ���ص�
					preBlock = Q_NULLPTR;
				}
				if (childElement.attribute("hasArrowLine") == "1") {
					preBlock = nowBlock;
				}
				//����ǰ�ڵ���ӽڵ���
				executeElementChilds(childElement, newPad, nowBlock, _level + 1);
			}
		}
	}
	newPad->isNew = true;
}
/*�����ļ�*/
void Vision::Save() { //����txt
	if (padTab->count() > 0) {
		//QRegExp rx("&untitled@\S*");	
		int index = editTab->currentIndex();
		QString filePath = filePaths.at(index);
		if (fSave(filePath, index) == 1)
			statusBar()->showMessage(QString::fromLocal8Bit("�ɹ�������") + filePath, 3000);
	}
}
/*����ȫ��*/
void Vision::SaveAll() {
	if (padTab->count() > 0) {
		int fileCounts = filePaths.count();
		int n = 0;	//�����ж��ǲ��������ļ�������ɹ���
		for (int i = 0; i < fileCounts; i++) {
			QString filePath = filePaths.at(i);
			n += fSave(filePath, i);
		}
		if (n == fileCounts)
			statusBar()->showMessage(QString::fromLocal8Bit("ȫ������ɹ�"), 3000);
	}
}
/*�����ļ��������*/
int Vision::fSave(QString filePath, int index) {
	if (filePath.isEmpty()) {
		QString defName = padTab->tabText(index);
		filePath = QFileDialog::getSaveFileName(this,
			QString::fromLocal8Bit("����") + defName, DEFAULT_PATH + tr("/") + defName, "Xml(*.xml);;Txt(*.txt)");
		if (filePath.isEmpty())
			return -1;
		filePaths[index] = filePath;
	}
	QFileInfo fileInfo = QFileInfo(filePath);
	QString fsuffix = fileInfo.suffix();
	if (fsuffix == "txt") {	//��txt
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
	else if (fsuffix == "xml") {	//��xml
		pads->at(index)->outport(filePath);
	}
	//��·�������������ļ�·����
	filePaths.replace(index, filePath);
	padTab->setTabText(index, fileInfo.fileName());
	editTab->setTabText(index, fileInfo.fileName());
	return 1;
}
/*���������Ϊ*/
void Vision::SaveAs() {
	if (tabNotEmpty()) {
		int index = editTab->currentIndex();
		QString defName = padTab->tabText(index)
			, pattern = (globalSplitter->widget(1)->isHidden()) ? "Txt(*.txt)" : "Xml(*.xml);;Txt(*.txt)";
		QString filePath = QFileDialog::getSaveFileName(this,
			QString::fromLocal8Bit("����") + defName, DEFAULT_PATH + tr("/") + defName, pattern);
		if (filePath.isEmpty())
			return ;
		filePaths[index] = filePath;
		
		QFileInfo fileInfo = QFileInfo(filePath);
		QString fsuffix = fileInfo.suffix();
		if (fsuffix == "txt") {	//��txt
			QFile file(filePath);
			if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
				return ;
			QTextStream out(&file);
			out.setCodec(QTextCodec::codecForName("utf-8"));
			PlotPad* pad = pads->at(index);
			QString text = pad->edit->showContent(pad);
			out << text;
			file.close();
		}
		else if (fsuffix == "xml") {	//��xml
			pads->at(index)->outport(filePath);
		}
		//��·�������������ļ�·����
		filePaths.replace(index, filePath);
		padTab->setTabText(index, fileInfo.fileName());
		editTab->setTabText(index, fileInfo.fileName());
		statusBar()->showMessage(QString::fromLocal8Bit("�ɹ�������") + filePath, 3000);
	}
}
/*�ر��ļ�*/
void Vision::Close() {
	if (tabNotEmpty()) {
		int index = editTab->currentIndex();
		QMessageBox* msgBox = new QMessageBox(
			QMessageBox::Question
			, QString::fromLocal8Bit("�ر�")
			, QString::fromLocal8Bit("�Ƿ񱣴浱ǰ�ļ���")
			, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		msgBox->button(QMessageBox::Yes)->setText(QString::fromLocal8Bit("���沢�ر�"));
		msgBox->button(QMessageBox::No)->setText(QString::fromLocal8Bit("ֱ�ӹر�"));
		msgBox->button(QMessageBox::Cancel)->setText(QString::fromLocal8Bit("ȡ��"));
		loadStyleSheet(msgBox, "msgBox.qss");
		int choose = msgBox->exec();
		if (QMessageBox::Cancel == choose)
			return;
		if (QMessageBox::Yes == choose)
			Save();
		filePaths.removeAt(index);
		pads->removeAt(index);
		edits->removeAt(index);
		padTab->removeTab(index);
		editTab->removeTab(index);
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
/*edit��plot��*/
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