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
2.10
1.curSmartEdit syn to curPlotPad;
2.add icon of actions Default, NoPlot, Cpp and Java;
3.fix the crash caused by ~Vision(), details that tabs' clear() lead to paradox with slots SynTo();
4.drawing lines between plotitems occupies  CPU a lot, this is solved by removing all func update() in PlotPad.cpp;
5.make plotitem or plottext antialiasing;
6.remove all func init();
7.timer's cycle change from 1000ms to 100ms, so that datetime can be more specific;

*.escape character not supported;
*.support two patterns, you can choose to show plotpad or not;
*.support two languages, you can choose C++ or Java;
*/
const QString version = "2.10";

Vision::Vision(QWidget* parent)
	: QMainWindow(parent)
	, globalSplitter(new QSplitter(Qt::Horizontal, this))
	, toolKit(new ToolKit(globalSplitter))
	, plotTab(new QTabWidget(globalSplitter))
	, editTab(new QTabWidget(globalSplitter))
	, curDateTimeLabel(new QLabel())
	, timer(new QTimer(this))
	, plots(new QList<PlotPad*>())
	, edits(new QList<SmartEdit*>())
{
	//ȫ�ִ���
	setMinimumSize(900, 600);
	visionUi.setupUi(this);//ui�����������˵����͹�����

	//�Զ����м��������߿�ͼ�����������
	toolKit->setMinimumWidth(60);
	toolKit->setMaximumWidth(200);
	plotTab->setMinimumWidth(200);
	editTab->setMinimumWidth(200);
	//�Զ���������״̬����ʱ���ǩ
	curDateTimeLabel->setAlignment(Qt::AlignRight);
	statusBar()->addPermanentWidget(curDateTimeLabel);
	/*
	(index, stretch) �ָ����ڵ�index�ſ���Ԫ��stretch 0���洰��仯��1+��Ϊ����ϵ��
	��������1��Ԫ����2��Ԫ�ؿ�ȱ�Ϊ3��1
	*/
	for (int i = 0; i < 3; i++) {
		globalSplitter->setStretchFactor(i, 1);
	}
	setCentralWidget(globalSplitter);//���ڲ������
	//����qss
	loadStyleSheet(this, "global.qss");

	//������ʱ��
	timer->start(100);
	showCurDateTime();
	statusBar()->showMessage(QString::fromLocal8Bit("VisionΪ������"), 5000);
	//�ۺ���
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
	connect(plotTab, SIGNAL(currentChanged(int)), this, SLOT(editSynToPad(int)));
	connect(editTab, SIGNAL(currentChanged(int)), this, SLOT(padSynToEdit(int)));
}
Vision::~Vision() {
	timer->stop();	delete timer; timer = Q_NULLPTR;
	delete curDateTimeLabel;	curDateTimeLabel = Q_NULLPTR;
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

/*ʱ���ǩ*/
void Vision::showCurDateTime() {
	QString curDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd / hh:mm:ss");
	curDateTimeLabel->setText(curDateTime);
}
/*�˳�����*/
int Vision::Quit() {
	int choose = -1;
	if (plotTab->count() > 0) {
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
/*���ڰ�ť�˳�*/
void Vision::closeEvent(QCloseEvent* event) {
	if (QMessageBox::Cancel == Quit())event->ignore();
}
/*Ĭ��ģʽ*/
void Vision::Default() {
	if (plotTab->count() > 0) {

	}
}
/*��ͼģʽ*/
void Vision::NoPlot() {
	if (editTab->count() > 0) {

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

}
/*����*/
void Vision::Redo() {

}
/*����*/
void Vision::Cut() {

}
/*����*/
void Vision::Copy() {
	
}
/*ճ��*/
void Vision::Paste() {

}
/*ȫѡ*/
void Vision::SelectAll() {

}
/*ɾ��*/
void Vision::Delete() {

}
/*ȡ��*/
void Vision::getCode() {
	edits->at(editTab->currentIndex())->selectAll();
	edits->at(editTab->currentIndex())->copy();
}
/*�½��ļ�*/
void Vision::New() {
	QString defaultName = "#untitled@" + QString::number(plotTab->count()); 
	QGraphicsScene* scene = new QGraphicsScene;
	PlotPad* newPad = new PlotPad(scene);
	plots->append(newPad);
	plotTab->addTab(newPad, defaultName);
	int newIndex = plotTab->count() - 1;
	plotTab->setCurrentIndex(newIndex);
	SmartEdit* newEdit = new SmartEdit();
	edits->append(newEdit);
	editTab->addTab(newEdit, defaultName);
	editTab->setCurrentIndex(newIndex);
	filePaths.append("");

	visionUi.actionSave->setEnabled(true);
	visionUi.actionSaveAll->setEnabled(true);
	visionUi.actionSaveAs->setEnabled(true);
	visionUi.actionClose->setEnabled(true);
	visionUi.actionCopy->setEnabled(true);
	visionUi.actionCut->setEnabled(true);
	visionUi.actionDelete->setEnabled(true);
	visionUi.actionPaste->setEnabled(true);
	visionUi.actionRedo->setEnabled(true);
	visionUi.actionUndo->setEnabled(true);
}
/*���ļ�*/
void Vision::Open() {
	QString filePath = QFileDialog::getOpenFileName(this,
		QString::fromLocal8Bit("���ļ�"), DEFAULT_PATH, tr("XML (*.xml)"));
	if (filePath != NULL && !filePaths.contains(filePath, Qt::CaseSensitive)) {
		//��ȡ�ļ����ݣ���������ص�plotTab��editTab
	}
	else if (filePaths.contains(filePath, Qt::CaseSensitive)) {
		//��������ת����ͬ·����tab
		int index = filePaths.indexOf(filePath);
		plotTab->setCurrentIndex(index);
		editTab->setCurrentIndex(index);
	}
	if (plotTab->count() == 1) {
		visionUi.actionSave->setEnabled(true);
		visionUi.actionSaveAll->setEnabled(true);
		visionUi.actionSaveAs->setEnabled(true);
		visionUi.actionClose->setEnabled(true);
		visionUi.actionCopy->setEnabled(true);
		visionUi.actionCut->setEnabled(true);
		visionUi.actionDelete->setEnabled(true);
		visionUi.actionPaste->setEnabled(true);
		visionUi.actionRedo->setEnabled(true);
		visionUi.actionUndo->setEnabled(true);
	}
}
/*�����ļ�*/
void Vision::Save() {
	//����txt
	if (plotTab->count() > 0) {
		//QRegExp rx("&untitled@\S*");		
		int index = editTab->currentIndex();
		QString filePath = filePaths.at(index);
		if (filePath.isEmpty()) {
			filePath = QFileDialog::getSaveFileName(this,
				QString::fromLocal8Bit("�����ļ�"), DEFAULT_PATH, tr("XML (*.xml)"));
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
		visionUi.statusBar->showMessage(QString::fromLocal8Bit("�ɹ�������") + filePath, 3000);
	}
	//��XML
}
/*����ȫ��*/
void Vision::SaveAll() {
	if (plotTab->count() > 0) {
		int fileCounts = filePaths.count();
		for (int i = 0; i < fileCounts; i++) {
			if (filePaths.at(i).isEmpty()) {
				QString label = editTab->tabText(i);
				QString filePath = QFileDialog::getSaveFileName(this,
					QString::fromLocal8Bit("����") + label, DEFAULT_PATH, tr("XML (*.xml)"));
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
		visionUi.statusBar->showMessage(QString::fromLocal8Bit("ȫ������ɹ�"), 3000);
	}
}
/*���������Ϊ*/
void Vision::SaveAs() {

}
/*�ر��ļ�*/
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
				, QString::fromLocal8Bit("�ر�")
				, QString::fromLocal8Bit("�Ƿ񱣴浱ǰ�ļ���")
				, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
			msgBox->button(QMessageBox::Yes)->setText(QString::fromLocal8Bit("�����˳�"));
			msgBox->button(QMessageBox::No)->setText(QString::fromLocal8Bit("ֱ���˳�"));
			msgBox->button(QMessageBox::Cancel)->setText(QString::fromLocal8Bit("ȡ��"));
			int choose = msgBox->exec();
			if (QMessageBox::Yes == choose)
				Save();
			if (QMessageBox::Cancel == choose)
				return;
		}
		filePaths.removeAt(index);
		plots->removeAt(index);
		edits->removeAt(index);
		if (plots->count() == 0) {
			visionUi.actionSave->setEnabled(false);
			visionUi.actionSaveAll->setEnabled(false);
			visionUi.actionSaveAs->setEnabled(false);
			visionUi.actionClose->setEnabled(false);
			visionUi.actionCopy->setEnabled(false);
			visionUi.actionCut->setEnabled(false);
			visionUi.actionDelete->setEnabled(false);
			visionUi.actionPaste->setEnabled(false);
			visionUi.actionRedo->setEnabled(false);
			visionUi.actionUndo->setEnabled(false);
		}
	}
}
/*edit��plot��*/
void Vision::editSynToPad(int index) {
	if (index >= 0) {
		editTab->setCurrentIndex(index);
	}
}
void Vision::padSynToEdit(int index) {
	if (index >= 0) {
		plotTab->setCurrentIndex(index);
	}
}
