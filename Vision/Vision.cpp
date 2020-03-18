#include "stdafx.h"
#include "Vision.h"

Vision::Vision(QWidget *parent)
	: QMainWindow(parent)
	, timer(new QTimer(this))
	, curDateTimeLabel(new QLabel())
	,VSplitter(new QSplitter(Qt::Horizontal , this))
	,a(new QTextEdit("a", VSplitter))
	,b(new QTextEdit("b", VSplitter))
	,c(new QTextEdit("c", VSplitter))
{
	visionUi.setupUi(this);//ui�����������˵����͹�����
	curDateTimeLabel->setAlignment(Qt::AlignRight);
	visionUi.statusBar->addPermanentWidget(curDateTimeLabel);

	a->setMinimumWidth(200);
	b->setMinimumWidth(200);
	c->setMinimumWidth(200);
	//(index, stretch) �ָ����ڵ�index�ſ���Ԫ��stretch 0���洰��仯��1+��Ϊ����ϵ��
	//��������1��Ԫ����2��Ԫ�ؿ�ȱ�Ϊ2��1
	VSplitter->setStretchFactor(0, 0);
	VSplitter->setStretchFactor(1, 2);
	VSplitter->setStretchFactor(2, 1);


	this->setCentralWidget(VSplitter);
	this->setMinimumSize(900, 540);
	timer->start(1000);

	connect(timer, SIGNAL(timeout()), this, SLOT(showCurDateTime()));
}

Vision::~Vision() {
	timer->stop();
	delete timer;
	delete curDateTimeLabel;
	delete a;
	delete b;
	delete c;
	delete VSplitter;
}

void Vision::showCurDateTime() {
	QString curDateTime = QDate::currentDate().toString() + " / " + QTime::currentTime().toString();
	curDateTimeLabel->setText(curDateTime);
}