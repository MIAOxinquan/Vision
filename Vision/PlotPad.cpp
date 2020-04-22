#include "global.h"
#include "PlotPad.h"
#include "SmartEdit.h"
#include <QMap>

PlotPad::PlotPad(QGraphicsScene* scene) : QGraphicsView() {
	this->scene = scene;
	//scene->addRect(0, 0, 1000, 1000);
	//scene->addRect(-this->width() / 2, -this->height() / 2, this->width(), this->height());

	this->QGraphicsView::setScene(scene);
	this->setSceneRect(0, 0, 2000, 2000);
	setAcceptDrops(true);
	startPoint = QPoint(100, 100);
	endPoint = QPoint(200, 200);


	//去掉滚动条
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	s.push(new QList<Block*>());
	path.push("PlotPad//");
}

void PlotPad::drawItems(Block* it) {
	//it->setAcceptDrops(true);
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
	QTransform transform;
	Item* pIt = (Item*)scene->itemAt(QPointF(p), transform);
	if (pIt && pIt->className() == "PlotEdge") return;
	Block* it = (Block*)pIt;
	//PlotItem* it = (PlotItem*)scene->itemAt(QPointF(p), transform);
	Block* p1 = new Block(p.rx() , p.ry() , str);
	drawItems(p1);
	if (it) // it != NULL
	{
		qreal maxY = -1;
		for (int i = 0; i < it->sons->size(); ++i)
			if (it->sons->at(i)->pos().y() > maxY)
				maxY = it->sons->at(i)->pos().y();
		it->sons->push_back(p1);
		p1->setPos(200, maxY + 100);
		p1->hide();
	}
	else
		s.top()->push_back(p1);
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
		startPoint = e->pos();
		if (ctlPressed)//按下ctl
		{
			leftBtnClicked = true;
			qDebug() << "left pressed";
		}
	}
	if (!ctlPressed)QGraphicsView::mousePressEvent(e);
}

void PlotPad::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		QTransform transform;
		Item* pIt = (Item*)scene->itemAt(QPointF(e->pos()), transform);
		if (pIt && pIt->className() == "PlotEdge") return;
		Block* it = (Block*)pIt;
		if (!it)return; // 如果it为NULL，则什么都不做
		QList<Block*>* topList = this->s.top();
		for (int i = 0; i < topList->size(); ++i)// 隐藏父亲节点同层节点
		{
			topList->at(i)->hide();
			if (topList->at(i) && topList->at(i)->fromEdge && topList->at(i)->fromEdge->isVisible())
				topList->at(i)->fromEdge->hide();
			if (topList->at(i) && topList->at(i)->toEdge && topList->at(i)->toEdge->isVisible())
				topList->at(i)->toEdge->hide();
		}
		for (int i = 0; i < it->sons->size(); ++i)// 显示子层节点
		{
			it->sons->at(i)->show();
			if (it->sons->at(i)->fromEdge && !it->sons->at(i)->fromEdge->isVisible())
				it->sons->at(i)->fromEdge->show();
			if (it->sons->at(i)->toEdge && !it->sons->at(i)->toEdge->isVisible())
				it->sons->at(i)->toEdge->show();
		}
		s.push(it->sons);
		path.push(it->head.toStdString() + "//");
		QString dbugStr = QString("double click At (%1,%2)").arg(e->pos().x()).arg(e->pos().y());
		qDebug() << dbugStr;
	}
}

void PlotPad::backLevel()
{
	if (s.size() > 1)
	{
		QList<Block*>* topList = this->s.top();
		for (int i = 0; i < topList->size(); ++i)
		{
			topList->at(i)->hide();
			
			if (topList->at(i) && topList->at(i)->fromEdge && topList->at(i)->fromEdge->isVisible())
				topList->at(i)->fromEdge->hide();
			if (topList->at(i) && topList->at(i)->toEdge && topList->at(i)->toEdge->isVisible())
				topList->at(i)->toEdge->hide();
		}

		s.pop();
		path.pop();
		topList = this->s.top();
		for (int i = 0; i < topList->size(); ++i)
		{
			topList->at(i)->show();
			if (topList->at(i) && topList->at(i)->fromEdge && !topList->at(i)->fromEdge->isVisible())
				topList->at(i)->fromEdge->show();
			if (topList->at(i) && topList->at(i)->toEdge && !topList->at(i)->toEdge->isVisible())
				topList->at(i)->toEdge->show();
		}
	}
}

//删除item
void PlotPad::deleteItem() {
	QTransform transform;
	QList<Block*>* topList = this->s.top();
	//s.pop();
	Item* i1 = (Item*)scene->itemAt(QPointF(startPoint), transform);
	if (!i1) {
		qDebug() << "nothing to delete";
		//s.push(topList);
		return;
	}
	qDebug() << "try to delete";
	scene->removeItem((QGraphicsItem*)i1);
	if (i1->className() == "PlotItem") {
		Block* pItem = (Block*)i1;
		topList->removeOne(pItem);
		if (pItem->fromEdge) {
			scene->removeItem(pItem->fromEdge);
			pItem->fromEdge->getSrc()->toEdge = NULL;
			delete pItem->fromEdge;
			pItem->fromEdge = NULL;
		}
		if (pItem->toEdge) {
			scene->removeItem(pItem->toEdge);
			pItem->toEdge->getDest()->fromEdge = NULL;
			delete pItem->toEdge;
			pItem->toEdge = NULL;
		}
		pItem->removeItemAllSons(pItem);
		delete i1;
		i1 = NULL;
		qDebug() << "delete PlotItem successfully";
	}
	else if (i1->className() == "PlotEdge") {
		ArrowLine* pEdge = (ArrowLine*)i1;
		pEdge->deleteLine(pEdge);
		i1 = NULL;
		qDebug() << "delete PlotEdge successfully";
	}
	//s.push(topList);
}



Block* PlotPad::getRoot()//返回s.top()列表中的root节点
{
	QList<Block*>* list = s.top();
	int size = list->size();
	if (size == 0) {
	loop:
		qDebug() << "getBoot NULL";
		return NULL;
	}
	else if (size == 1) {
		qDebug() << "size = 1";
		return list->at(0);
	}
	QList<int> int_list;
	Block* tem = NULL;
	int_list.append((int)list->at(0));
	tem = list->at(0);
	//向后遍历
	while (tem->toEdge) {
		tem = tem->toEdge->getDest();
		if (tem == list->at(0)) {
			goto loop;
		}
		int_list.append((int)tem);
	}
	//向前遍历
	tem = list->at(0);
	while (tem->fromEdge) {
		tem = tem->fromEdge->getSrc();
		int_list.append((int)tem);
	}
	//如果int_list.count != size 说明不是连通图
	if (int_list.count() == size) {
		qDebug() << tem->head.toStdString().c_str();
		return tem;
	}
	else {
		goto loop;
	}
}

void PlotPad::myTest() {
	Block* p1 = getRoot();
	//qDebug() << QString::fromStdString(getPath());
}

std::string PlotPad::getPath() {
	std::string pth = "";
	for (int i = 0; i < path.size(); i++) {
		pth = pth + path.at(i);
	}
	return  pth.substr(0, pth.length() - 2);
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
			Block* fromItem = NULL;
			Block* toItem = NULL;
			Item* fItem = (Item*)scene->itemAt(QPointF(startPoint), transform);
			Item* tItem = (Item*)scene->itemAt(QPointF(endPoint), transform);
			
			if (fItem && tItem && fItem->className() == "PlotItem" && tItem->className() == "PlotItem")
			{
				fromItem = (Block*)fItem;
				toItem = (Block*)tItem;
			}
			if (fromItem && toItem)
			{
				ArrowLine* line = new ArrowLine(fromItem, toItem, QPointF(0, 0), QPointF(0, 0));
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
	//update();
}

//绘图事件
void PlotPad::paintEvent(QPaintEvent* e)
{
	QGraphicsView::paintEvent(e);
}












///////////////////////////////////////////////////// PItem ////////////////////////////////////////
Block::Block(int x, int y, QString str) :Item() {
	/*this->x = x;
	this->y = y;*/
	this->head = str;
	this->setPos(QPointF(x, y));
	this->sons = new QList<Block*>();
}

QString Block::className()
{
	return "PlotItem";
}

int Block::getWidth()
{
	return 80;
}

int Block::getHeight()
{
	return 40;
}

void Block::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
	QWidget* widget)
{
	//Q_UNUSED(option);
	//Q_UNUSED(widget);
	//QPointF p = this->pos();
	//painter->drawRoundedRect(p.x(), p.y(), 80, 40, 10, 10);
	painter->setBrush(QColor(0x80, 0xd6, 0xef));
	painter->drawRoundedRect(boundingRect(), 10, 10);
	painter->setPen(Qt::black);
	int size = 10;
	if (head.count() > 7)
		size = size - 3;
	if (hasFocus()) {
		painter->setPen(QColor(128, 0, 128));
		painter->setFont(QFont("华文琥珀", size + 2));
	}
	else {
		painter->setFont(QFont("Courier New", size));
	}
	painter->drawText(boundingRect(), Qt::AlignCenter, QObject::tr(head.toStdString().c_str()));
	drawToItem(painter);

	/*for (int i = 0; i < toItems.size(); ++i)
	{
		painter->drawLine(this->pos(), toItems[i]->pos());
	}*/
	//painter->drawText(sceneBoundingRect(), Qt::AlignCenter, QObject::tr(head.c_str()));
	//QGraphicsItem::paint(painter,option,widget);
}


void Block::drawToItem(QPainter* painter)
{
	if (toItem)
	{
		QPointF pf = this->pos(), pt = toItem->pos();
		painter->drawLine(QPointF(0, 0), pt - pf);
	}
}

QRectF Block::boundingRect() const
{
	qreal penWidth = 5;
	QPointF p = this->pos();
	//return QRectF(p.x() - penWidth / 2, p.y() - penWidth / 2,
	//	80 + penWidth, 40 + penWidth);
	return QRectF(-40 - penWidth / 2, -20 - penWidth / 2,
		80, 40 + penWidth);
	//return QRectF(0, 0, 80, 40);
}

//鼠标事件
void Block::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{

	/*foreach(PlotEdge * edge, edges) {
		qDebug() << "edge adjust begin";
		edge->adjust();
	}*/
	if (toEdge)
		toEdge->adjust();
	if (fromEdge)
		fromEdge->adjust();
	QGraphicsItem::mouseMoveEvent(e);
}

//删除节点的子孙节点
void Block::removeItemAllSons(Block* pItem) {
	if (!(pItem->sons)) {
		delete pItem;
		return;
	}
	QList<Block*>* p1 = pItem->sons;
	for (int i = 0; i < p1->size(); i++) {
		if (p1->at(i)->fromEdge) {
			p1->at(i)->fromEdge->deleteLine(p1->at(i)->fromEdge);
		}
		removeItemAllSons(p1->at(i));
	}
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

ArrowLine::ArrowLine(Block* sourceNode, Block* destNode, QPointF pointStart, QPointF pointEnd)
	: arrowSize(10)
{
	setFlags(ItemIsSelectable | ItemIsMovable);
	setAcceptedMouseButtons(0);
	//m_pointFlag = pointflag;

	m_pointStart = pointStart;//偏移量
	m_pointEnd = pointEnd;//偏移量
	source = sourceNode;
	dest = destNode;

	this->setFlags(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable
		| QGraphicsItem::GraphicsItemFlag::ItemIsFocusable);
	//source->addEdge(this);
	//dest->addEdge(this);
	adjust();

	//m_removeAction = new QAction(QStringLiteral("删除"), this);
	//connect(m_removeAction, SIGNAL(triggered()), this, SLOT(slotRemoveItem()));
}

Block* ArrowLine::getSrc()
{
	return source;
}

Block* ArrowLine::getDest()
{
	return dest;
}

QString ArrowLine::className()
{
	return "PlotEdge";
}

qreal ArrowLine::min(qreal r1, qreal r2)
{
	return r1 < r2 ? r1 : r2;
}

qreal ArrowLine::abs(qreal r)
{
	if (r >= 0) return r;
	else return -r;
}

void ArrowLine::adjust()
{
	if (!source || !dest)
		return;

	int sWidth = source->getWidth(), sHeight = source->getHeight();
	int dWidth = dest->getWidth(), dHeight = dest->getHeight();

	QPointF pS = source->pos(), pD = dest->pos();

	QRectF sRect = source->boundingRect();
	QRectF dRect = dest->boundingRect();

	qreal dx = abs(pS.x() - pD.x()), dy = abs(pS.y() - pD.y());
	qreal xS = 0, yS = 0, xD = 0, yD = 0;
	if (dx > dy) // 箭头的点应该在竖直的线上
	{
		if (pS.x() <= pD.x()) // source在左
		{
			xS = sWidth / 2;
			xD = -dWidth / 2;
		}
		else // source在右
		{
			xS = -sWidth / 2;
			xD = dWidth / 2;
		}
	}
	else // 箭头的点应该在水平的线上
	{
		if (pS.y() <= pD.y()) // source在上
		{
			yS = sHeight / 2;
			yD = -dHeight / 2;
		}
		else // source在下
		{
			yS = -sHeight / 2;
			yD = dHeight / 2;
		}
	}
	QLineF line(mapFromItem(source, xS, yS), mapFromItem(dest, xD, yD));
	//    qreal length = line.length();

	prepareGeometryChange();

	sourcePoint = line.p1();
	destPoint = line.p2();
	//    sourcePoint = mapFromItem(source, 0, 0);
	//    destPoint = mapFromItem(dest, 0, 0);
}

QRectF ArrowLine::boundingRect() const
{
	if (!source || !dest)
		return QRectF();
	qreal penWidth = 1;
	qreal extra = (penWidth + arrowSize) / 2.0;
	return QRectF(sourcePoint + m_pointStart, QSizeF((destPoint + m_pointEnd).x() - (sourcePoint + m_pointStart).x(),
		(destPoint + m_pointEnd).y() - (sourcePoint + m_pointStart).y()))
		.normalized().adjusted(-extra, -extra, extra, extra);
}
QPainterPath ArrowLine::shape() const
{

	int w = 5;
	QPainterPath path;
	//QPainterPath path = QGraphicsLineItem::shape();
	path.moveTo(sourcePoint);
	path.lineTo(destPoint);
	QPainterPathStroker stroker;
	stroker.setWidth(w);
	path = stroker.createStroke(path);
	return path;
}

void ArrowLine::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	if (!source || !dest)
		return;
	QLineF line(sourcePoint + m_pointStart, destPoint + m_pointEnd);
	if (qFuzzyCompare(line.length(), qreal(0.)))
		return;
	if (hasFocus())
		painter->setPen(QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	else
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
	if (hasFocus())
		painter->setPen(QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	else
		painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	painter->drawLine(QLineF(destArrowP1, destPoint + m_pointEnd));
	painter->drawLine(QLineF(destArrowP2, destPoint + m_pointEnd));
	/*painter->setPen(QPen(Qt::red, 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	painter->drawPath(this->shape());*/
	//    painter->setBrush(Qt::black);
	//    painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);
}

void ArrowLine::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	qDebug() << "-----move__----------";
	if (event->modifiers() & Qt::ShiftModifier) {
		update();
		return;
	}
	QGraphicsItem::mouseMoveEvent(event);
}

void ArrowLine::deleteLine(ArrowLine* pEdge) {
	pEdge->getSrc()->toEdge = NULL;
	pEdge->getDest()->fromEdge = NULL;
	delete pEdge;
}