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
version 1.50
1.add try_catch in keys_cpp and make it smart;
2.add try_catch in toolKeys and make it smart;
3.add func smartCore() to support func smartComplete() & func dropEvent() in the meanwhile, remove func smartDrop() and QStringList: toolSmarts;
4.fix more repeated keys' wrong displayed highlightblock;

8.escape character not supported;
9.support two patterns, you can choose to show plotpad or not;
10.support two languages, you can choose C++ or Java;
*/
const QString version = "1.50";

Vision::Vision(QWidget* parent)
	: QMainWindow(parent)
	, globalSplitter(new QSplitter(Qt::Horizontal, this))
	, toolKit(new ToolKit(globalSplitter))
	, plotTab(new QTabWidget(globalSplitter))
	, editTab(new QTabWidget(globalSplitter))
	, curDateTimeLabel(new QLabel())
	, timer(new QTimer(this))
{
	//��ʼ��
	init();
	//������ʱ��
	timer->start(1000);
	showCurDateTime();
	statusBar()->showMessage("initailizition finished!", 5000);
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
/*��ʼ��*/
void Vision::init() {
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
	//������ʼ��
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
	setCentralWidget(globalSplitter);//���ڲ������
	//����qss
	loadStyleSheet(this, "global.qss");
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
	globalSplitter->widget(1)->show();
}
/*��ͼģʽ*/
void Vision::NoPlot() {
	globalSplitter->widget(1)->close();
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







