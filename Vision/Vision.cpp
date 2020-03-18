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
	visionUi.setupUi(this);//ui定义上区：菜单栏和工具栏
	curDateTimeLabel->setAlignment(Qt::AlignRight);
	visionUi.statusBar->addPermanentWidget(curDateTimeLabel);

	a->setMinimumWidth(200);
	b->setMinimumWidth(200);
	c->setMinimumWidth(200);
	//(index, stretch) 分割器内第index号框内元素stretch 0则不随窗体变化，1+则为比例系数
	//例如以下1号元素与2号元素宽度比为2：1
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