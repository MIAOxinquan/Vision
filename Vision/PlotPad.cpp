#include "global.h"
#include "PlotPad.h"

PlotPad::PlotPad(QGraphicsScene* scene) : QGraphicsView() {
	this->scene = scene;
	//scene->addRect(0, 0, 1000, 1000);
	//scene->addRect(-this->width() / 2, -this->height() / 2, this->width(), this->height());
	
	this->QGraphicsView::setScene(scene);
	this->setSceneRect(0, 0, 2000, 2000);
	setAcceptDrops(true);
	startPoint = QPoint(100, 100);
	endPoint = QPoint(200, 200);

	vBar = this->verticalScrollBar();
	hBar = this->horizontalScrollBar();


	//去掉滚动条
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void PlotPad::drawItems(PlotItem* it) {
	it->setAcceptDrops(true);
	it->setFlags(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable
		| QGraphicsItem::GraphicsItemFlag::ItemIsMovable
		| QGraphicsItem::GraphicsItemFlag::ItemIsFocusable);
	scene->addItem(it);
}


void PlotPad::dropEvent(QDropEvent* event)
{
	setFocus();
	QString str = event->mimeData()->text();//获取text
	QPoint p = event->pos();//获取位置 --> PlotPad内的位置
	QString dbugStr = QString("dropEvent At (%1,%2),data: %3").arg(p.x()).arg(p.y()).arg(str);
	qDebug() << dbugStr;

	//PlotItem* p1 = new PlotItem(p.rx(), p.ry(),toolKeys.at(index).toStdString());
	PlotItem* p1 = new PlotItem(p.rx() + hBar->value(), p.ry() + vBar->value(), str);

	drawItems(p1);
	QGraphicsView::dropEvent(event);
}


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

//键盘事件
void PlotPad::keyPressEvent(QKeyEvent* e)
{
	if (e->key() == Qt::Key::Key_Control)
	{
		ctlPressed = true;
		qDebug() << "ctl pressed";
		e->accept();
	}
}
void PlotPad::keyReleaseEvent(QKeyEvent* e)
{
	if (e->key() == Qt::Key::Key_Control)
	{
		ctlPressed = false;
		qDebug() << "ctl released";
		e->accept();
	}
}

//鼠标事件
void PlotPad::mouseMoveEvent(QMouseEvent* e)
{
	if (ctlPressed && leftBtnClicked)
	{
		endPoint = e->pos();
		QPoint r = endPoint - startPoint;
		//PlotItem* item = scene->itemAt()
		if (r.manhattanLength() >= 5)
		{
			if (lastLine)
				scene->removeItem(lastLine);
			lastLine = scene->addLine(startPoint.x(), startPoint.y(), endPoint.x(), endPoint.y());
		}
	}
	QGraphicsView::mouseMoveEvent(e);
	//update();
}
void PlotPad::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)//左键按下
	{
		if (ctlPressed)//按下ctl
		{
			leftBtnClicked = true;
			startPoint = e->pos();
			qDebug() << "left pressed";
		}
	}
	if(!ctlPressed)QGraphicsView::mousePressEvent(e);
	
}
void PlotPad::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		leftBtnClicked = false;
		qDebug() << "left released";
		if (lastLine)
			scene->removeItem(lastLine);
		if (ctlPressed)
		{
			QTransform transform;
			PlotItem* fromItem = NULL;
			PlotItem* toItem = NULL;
			Item* fItem = (Item*)scene->itemAt(QPointF(startPoint), transform);
			Item* tItem = (Item*)scene->itemAt(QPointF(endPoint), transform);
			if (fItem && tItem && fItem->className() == "PlotItem" && tItem->className() == "PlotItem")
			{
				fromItem = (PlotItem*)fItem;
				toItem = (PlotItem*)tItem;
			}
			if (fromItem && toItem)
			{
				PlotEdge* line = new PlotEdge(fromItem, toItem, QPointF(0,0), QPointF(0, 0));
				scene->addItem(line);
				if (fromItem->toEdge)
				{
					qDebug() << "enter if";
					fromItem->toEdge->getDest()->fromEdge = NULL;
					scene->removeItem(fromItem->toEdge);
					qDebug() << "remove";
					delete fromItem->toEdge;
					fromItem->toEdge = NULL;
				}
				fromItem->toEdge = line;
				if (toItem->fromEdge)
				{
					toItem->fromEdge->getSrc()->toEdge = NULL;
					scene->removeItem(toItem->fromEdge);
					delete toItem->fromEdge;
					toItem->fromEdge = NULL;
				}
				toItem->fromEdge = line;
			}
		}
	}
	QGraphicsView::mouseReleaseEvent(e);
	update();
}

//绘图事件
void PlotPad::paintEvent(QPaintEvent* e)
{
	QGraphicsView::paintEvent(e);
}












///////////////////////////////////////////////////// PItem ////////////////////////////////////////
PlotItem::PlotItem(int x, int y, QString str) :Item() {
	/*this->x = x;
	this->y = y;*/
	this->head = str;
	this->setPos(QPointF(x, y));
}

QString PlotItem::className()
{
	return "PlotItem";
}

void PlotItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
	QWidget* widget)
{
	//Q_UNUSED(option);
	//Q_UNUSED(widget);
	//QPointF p = this->pos();
	//painter->drawRoundedRect(p.x(), p.y(), 80, 40, 10, 10);
	painter->drawRoundedRect(boundingRect(), 10, 10);
	painter->setPen(Qt::black);
	painter->setFont(QFont("华文琥珀", 12));
	painter->drawText(boundingRect(), Qt::AlignCenter, QObject::tr(head.toStdString().c_str()));
	drawToItem(painter);
	
	
	/*for (int i = 0; i < toItems.size(); ++i)
	{
		painter->drawLine(this->pos(), toItems[i]->pos());
	}*/
	//painter->drawText(sceneBoundingRect(), Qt::AlignCenter, QObject::tr(head.c_str()));
	//QGraphicsItem::paint(painter,option,widget);
}


void PlotItem::drawToItem(QPainter* painter)
{
	if (toItem)
	{
		QPointF pf = this->pos(), pt = toItem->pos();
		painter->drawLine(QPointF(0, 0), pt - pf);
	}
}

QRectF PlotItem::boundingRect() const
{
	qreal penWidth = 5;
	QPointF p = this->pos();
	//return QRectF(p.x() - penWidth / 2, p.y() - penWidth / 2,
	//	80 + penWidth, 40 + penWidth);
	return QRectF(-40 - penWidth / 2, -20 - penWidth / 2,
		80 + penWidth, 40 + penWidth);
	//return QRectF(0, 0, 80, 40);
}

//鼠标事件
void PlotItem::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{

	/*foreach(PlotEdge * edge, edges) {
		qDebug() << "edge adjust begin";
		edge->adjust();
	}*/
	if(toEdge)
		toEdge->adjust();
	if (fromEdge)
		fromEdge->adjust();
	QGraphicsItem::mouseMoveEvent(e);
}
//void PlotItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
//{
//
//}
//void PlotItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
//{
//
//}




///////////////////////////////////////////////////// PlotEdge ////////////////////////////////////////

static const double Pi = 3.14159265358979323846264338327950288419717;
static double TwoPi = 2.0 * Pi;

PlotEdge::PlotEdge(PlotItem* sourceNode, PlotItem* destNode, QPointF pointStart, QPointF pointEnd)
	: arrowSize(10)
{
	setFlags(ItemIsSelectable | ItemIsMovable);
	setAcceptedMouseButtons(0);
	//m_pointFlag = pointflag;

	m_pointStart = pointStart;//偏移量
	m_pointEnd = pointEnd;//偏移量
	source = sourceNode;
	dest = destNode;
	//source->addEdge(this);
	//dest->addEdge(this);
	adjust();

	//m_removeAction = new QAction(QStringLiteral("删除"), this);
	//connect(m_removeAction, SIGNAL(triggered()), this, SLOT(slotRemoveItem()));
}

PlotItem* PlotEdge::getSrc()
{
	return source;
}

PlotItem* PlotEdge::getDest()
{
	return dest;
}

QString PlotEdge::className()
{
	return "PlotEdge";
}
void PlotEdge::adjust()
{
	if (!source || !dest)
		return;

	QLineF line(mapFromItem(source, 0, 0), mapFromItem(dest, 0, 0));
	//    qreal length = line.length();

	prepareGeometryChange();

	sourcePoint = line.p1();
	destPoint = line.p2();
	//    sourcePoint = mapFromItem(source, 0, 0);
	//    destPoint = mapFromItem(dest, 0, 0);
}

QRectF PlotEdge::boundingRect() const
{
	if (!source || !dest)
		return QRectF();
	qreal penWidth = 1;
	qreal extra = (penWidth + arrowSize) / 2.0;
	return QRectF(sourcePoint + m_pointStart, QSizeF((destPoint + m_pointEnd).x() - (sourcePoint + m_pointStart).x(),
		(destPoint + m_pointEnd).y() - (sourcePoint + m_pointStart).y()))
		.normalized().adjusted(-extra, -extra, extra, extra);
}
void PlotEdge::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	if (!source || !dest)
		return;
	QLineF line(sourcePoint + m_pointStart, destPoint + m_pointEnd);
	if (qFuzzyCompare(line.length(), qreal(0.)))
		return;
	painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	update();
	painter->drawLine(line);

	// Draw the arrows
	//if (m_pointFlag)//
	//	return;
	double angle = ::acos(line.dx() / line.length());
	if (line.dy() >= 0)
		angle = TwoPi - angle;
	QPointF destArrowP1 = destPoint + m_pointEnd + QPointF(sin(angle - Pi / 3) * arrowSize,
		cos(angle - Pi / 3) * arrowSize);
	QPointF destArrowP2 = destPoint + m_pointEnd + QPointF(sin(angle - Pi + Pi / 3) * arrowSize,
		cos(angle - Pi + Pi / 3) * arrowSize);
	painter->setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	painter->drawLine(QLineF(destArrowP1, destPoint + m_pointEnd));
	painter->drawLine(QLineF(destArrowP2, destPoint + m_pointEnd));
	//    painter->setBrush(Qt::black);
	//    painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);
}
void PlotEdge::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	qDebug() << "-----move__----------";
	if (event->modifiers() & Qt::ShiftModifier) {
		update();
		return;
	}
	QGraphicsItem::mouseMoveEvent(event);
}
