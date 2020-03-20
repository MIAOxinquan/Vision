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
	visionUi.setupUi(this);//ui�����������˵����͹�����
	curDateTimeLabel->setAlignment(Qt::AlignRight);
	visionUi.statusBar->addPermanentWidget(curDateTimeLabel);

	a->setMinimumWidth(200);
	plotTab->setMinimumWidth(200);
	codeTab->setMinimumWidth(200);
	//(index, stretch) �ָ����ڵ�index�ſ���Ԫ��stretch 0���洰��仯��1+��Ϊ����ϵ��
	//��������1��Ԫ����2��Ԫ�ؿ�ȱ�Ϊ2��1
	globalSplitter->setStretchFactor(0, 0);
	globalSplitter->setStretchFactor(1, 2);
	globalSplitter->setStretchFactor(2, 1);


	this->setCentralWidget(globalSplitter);
	this->setMinimumSize(900, 540);
	timer->start(1000);
	
	//�ۺ�����Ⱥ
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
	QMessageBox::information(this, QString::fromLocal8Bit("˵��"), 
		QString::fromLocal8Bit("VisionΪ������\nʹ�ã�\n�����Ŷӣ�\n"));
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
























