#include "stdafx.h"
#include "Vision.h"

Vision::Vision(QWidget *parent)
	: QMainWindow(parent)
	, timer(new QTimer(this))
	, curDateTimeLabel(new QLabel())
	,horizonSplitter(new QSplitter(Qt::Horizontal , this))
	,a(new QTextEdit("a", horizonSplitter))
	,b(new QTextEdit("b", horizonSplitter))
	,c(new QTextEdit("c", horizonSplitter))
{
	visionUi.setupUi(this);//ui�����������˵����͹�����
	timer->start(1000);
	curDateTimeLabel->setAlignment(Qt::AlignRight);
	visionUi.statusBar->addPermanentWidget(curDateTimeLabel);

	//(index, stretch) �ָ����ڵ�index��Ԫ��stretch 0���洰��仯��1���洰��仯
	horizonSplitter->setStretchFactor(0, 0);
	horizonSplitter->setStretchFactor(1, 1);
	horizonSplitter->setStretchFactor(2, 1);


	this->setCentralWidget(horizonSplitter);
	this->setMinimumSize(900, 540);

}

Vision::~Vision() {
	timer->stop();
	delete timer;
	delete curDateTimeLabel;
	delete a;
	delete b;
	delete c;
	delete horizonSplitter;
}

void Vision::showCurDateTime() {

}