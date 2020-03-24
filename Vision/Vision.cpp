#include "stdafx.h"
#include "Vision.h"
//last version 0.24
Vision::Vision(QWidget* parent)
	: QMainWindow(parent)
	, timer(new QTimer(this))
	, curDateTimeLabel(new QLabel())
	, globalSplitter(new QSplitter(Qt::Horizontal, this))
	, toolKit(new ToolKit(globalSplitter))
	, plotTab(new QTabWidget(globalSplitter))
	, editTab(new QTabWidget(globalSplitter))
{
	visionUi.setupUi(this);//ui�����������˵����͹�����
	curDateTimeLabel->setAlignment(Qt::AlignRight);
	statusBar()->addPermanentWidget(curDateTimeLabel);
	//(index, stretch) �ָ����ڵ�index�ſ���Ԫ��stretch 0���洰��仯��1+��Ϊ����ϵ��
	//��������1��Ԫ����2��Ԫ�ؿ�ȱ�Ϊ3��1
	int ratio[] = { 1,1,1 };
	for (int i = 0; i < 3; i++) {
		globalSplitter->setStretchFactor(i, ratio[i]);
	}
	toolKit->setMinimumWidth(60);
	toolKit->setMaximumWidth(120);
	plotTab->setMinimumWidth(180);
	editTab->setMinimumWidth(180);
	/*plot&edit test*/
	for (int i = 0; i < 5; i++) {
		PlotPad* pad = new PlotPad();
		SmartEdit* edit = new SmartEdit();
		plotTab->addTab(pad, QString::number(i));
		editTab->addTab(edit, QString::number(i));
	}

	this->setCentralWidget(globalSplitter);
	this->setMinimumSize(900, 600);
	showCurDateTime();
	statusBar()->showMessage("initailizition finished!", 5000);

	//����qss
	QFile file("./Resources/qss/global.qss");
	file.open(QFile::ReadOnly);
	setStyleSheet(file.readAll());
	file.close();
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
	timer->stop();
	delete timer;
	delete curDateTimeLabel;
	delete toolKit;
	delete plotTab;
	delete editTab;
	delete globalSplitter;
}

void Vision::showCurDateTime() {
	QString curDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd / hh:mm:ss");
	curDateTimeLabel->setText(curDateTime);
}
/*�˳�����*/
int Vision::Quit() {
	int choose = -1;
	if (true) {
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
			delete msgBox;
			qApp->quit();
		}
	}
	else {
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
		, QString::fromLocal8Bit("\nVision�����Ŷ�:\n<Students  &WHU>\n@Code: ������ / ���� / ˾���� / ������ / ����԰\n@Document: ������")
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







