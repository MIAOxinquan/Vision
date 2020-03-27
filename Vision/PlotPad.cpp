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
	QString str = event->mimeData()->text();//获取text
	QPoint p = event->pos();//获取位置 --> PlotPad内的位置
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
	
	//设置动作为移动动作.
	//event->setDropAction(Qt::MoveAction);
	//然后接受事件.这个一定要写.
	event->accept();
}

void PlotPad::dragMoveEvent(QDragMoveEvent* event)
{
	//event->setDropAction(Qt::MoveAction);
	event->accept();
}