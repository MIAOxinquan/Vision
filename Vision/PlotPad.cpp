#include "PlotPad.h"

PlotPad::PlotPad(QTabWidget* parent)
	:QWidget(parent) {
	setAcceptDrops(true);

}

PlotPad::~PlotPad() {

}

void PlotPad::dropEvent(QDropEvent* event)
{
	setFocus();
	QString str = event->mimeData()->text();//��ȡtext
	QPoint p = event->pos();//��ȡλ�� --> PlotPad�ڵ�λ��
	QString dbugStr = QString("dropEvent At (%1,%2),data: %3").arg(p.x()).arg(p.y()).arg(str);
	qDebug() << dbugStr;
	QWidget::dropEvent(event);
}

//Vision* PlotPad::getVision()
//{
//	QObject* plotTab = this->parent();
//	Vision* vision = (Vision*)plotTab->parent();
//	return vision;
//}

void PlotPad::dragEnterEvent(QDragEnterEvent* event)
{
	
	//���ö���Ϊ�ƶ�����.
	//event->setDropAction(Qt::MoveAction);
	//Ȼ������¼�.���һ��Ҫд.
	event->accept();
}

void PlotPad::dragMoveEvent(QDragMoveEvent* event)
{
	//event->setDropAction(Qt::MoveAction);
	event->accept();
}