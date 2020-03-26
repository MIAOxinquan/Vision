#include "Vision.h"
#include "ToolKit.h"
#include "PlotPad.h"
#include "SmartEdit.h"
/*
neighbour version means one of last few versions or current version
,for example versions 1.0, 1.01, 1.02 are regarded as neighbours
,if current version is 1.02
,so as neighbour version can be 1.0, 1.01 or 1.02 

neighbour@
version 1.07 forward to 1.1
1.add completer.qss & list.qss;
2.remould toolbar separator style in globle.qss 
3.slightly change init function;
4.ToolKit & PlotPad constructing, dragEvent basically finished, a keyword will appear where it's placed if the revelant tool is drag into smartEdit;
5.add key "do" with pattern "\n{\n\n}while();", change pattern of key "while" from "()\n{\n\n}" to "()";
*/
Vision::Vision(QWidget* parent)
	: QMainWindow(parent)
	, timer(new QTimer(this))
	, curDateTimeLabel(new QLabel())
	, globalSplitter(new QSplitter(Qt::Horizontal, this))
	, toolKit(new ToolKit(globalSplitter))
	, plotTab(new QTabWidget(globalSplitter))
	, editTab(new QTabWidget(globalSplitter))
{
	//��ʼ��
	init();
	//������ʱ��
	timer->start(1000);
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
/*��ʼ��*/
void Vision::init() {
	visionUi.setupUi(this);//ui�����������˵����͹�����
	//�Զ����м��������߿�ͼ�����������
	toolKit->setMinimumWidth(60);
	toolKit->setMaximumWidth(200);
	plotTab->setMinimumWidth(200);
	editTab->setMinimumWidth(200);
	/*plot & edit test*/
	PlotPad* pad = new PlotPad();
	plotTab->addTab(pad, "plotPad");
	SmartEdit* edit = new SmartEdit();
	editTab->addTab(edit, "smartEdit");
	/*
	(index, stretch) �ָ����ڵ�index�ſ���Ԫ��stretch 0���洰��仯��1+��Ϊ����ϵ��
	��������1��Ԫ����2��Ԫ�ؿ�ȱ�Ϊ3��1
	*/
	for (int i = 0; i < 3; i++) {
		globalSplitter->setStretchFactor(i, 1);
	}
	//�Զ���������״̬����ʱ���ǩ
	statusBar()->showMessage("initailizition finished!", 5000);
	curDateTimeLabel->setAlignment(Qt::AlignRight);
	statusBar()->addPermanentWidget(curDateTimeLabel);
	showCurDateTime();
	//ȫ�ִ���
	setCentralWidget(globalSplitter);
	setMinimumSize(900, 600);
	//����qss
	QFile file("./Resources/qss/global.qss");
	file.open(QFile::ReadOnly);
	setStyleSheet(file.readAll());
	file.close();
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
/*���ڰ�ť�˳�*/
void Vision::closeEvent(QCloseEvent* event) {
	if (QMessageBox::Cancel == Quit())event->ignore();
}
/*����*/
void Vision::About() {
	QMessageBox* msgBox = new QMessageBox(
		QMessageBox::Information
		, QString::fromLocal8Bit("˵��")
		, QString::fromLocal8Bit("\nVision�����Ŷ�:\n<Students  &WHU>\
                                                    \n@Code: ������ / ���� / ˾���� / ������ / ����԰\
                                                    \n@Document: ������\
                                                    \n@Version: 1.1")
		, QMessageBox::Ok);
	msgBox->button(QMessageBox::Ok)->setText(QString::fromLocal8Bit("�ر�"));
	QFile file("./Resources/qss/msgBox.qss");
	file.open(QFile::ReadOnly);
	msgBox->setStyleSheet(file.readAll());
	file.close();
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
/*���ƴ���*/
void Vision::getCode() {

}
/*�½��ļ�*/
void Vision::New() {

}
/*���ļ�*/
void Vision::Open() {

}
/*�����ļ�*/
void Vision::Save() {

}
/*����ȫ��*/
void Vision::SaveAll() {

}
/*���������Ϊ*/
void Vision::SaveAs() {

}
/*�ر��ļ�*/
void Vision::Close() {

}







