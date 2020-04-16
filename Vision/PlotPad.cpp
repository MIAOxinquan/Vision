#include "global.h"
#include "PlotPad.h"

PlotPad::PlotPad(QGraphicsScene* scene)
	: QGraphicsView()
	, scene(Q_NULLPTR)
	, lastLine(Q_NULLPTR)
{
	this->scene = scene;
	this->QGraphicsView::setScene(scene);
	this->setSceneRect(0, 0, 1920, 1600);
	setAcceptDrops(true);
	startPoint = QPoint(100, 100);
	endPoint = QPoint(200, 200);


	//去掉滚动条
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	loadStyleSheet(this, "plot.qss");
}

//PlotPad::~PlotPad() {
//	if (lastLine) {
//		delete lastLine;
//		lastLine = Q_NULLPTR;
//	}
//	/*禁止delete scene*/
//}

void PlotPad::drawItems(Block* it) {
	it->setAcceptDrops(true);
	it->setFlags(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable
		| QGraphicsItem::GraphicsItemFlag::ItemIsMovable
		| QGraphicsItem::GraphicsItemFlag::ItemIsFocusable);
	scene->addItem(it);
}


void PlotPad::dropEvent(QDropEvent* event) {
	setFocus();
	QString type = event->mimeData()->text();//获取text
	QPoint m_dropPos = event->pos();//获取位置 --> PlotPad内的位置
	Block* newBlock = new Block(m_dropPos.rx() /*+ hBar->value()*/, m_dropPos.ry() /*+ vBar->value()*/, type);
	drawItems(newBlock);
	QGraphicsView::dropEvent(event);
}


void PlotPad::dragEnterEvent(QDragEnterEvent* event) {
	//设置动作为移动动作.
	//event->setDropAction(Qt::MoveAction);
	//然后接受事件.这个一定要写.
	event->accept();
}

void PlotPad::dragMoveEvent(QDragMoveEvent* event) {
	//event->setDropAction(Qt::MoveAction);
	event->accept();
}

//键盘事件
void PlotPad::keyPressEvent(QKeyEvent* e) {
	if (e->key() == Qt::Key::Key_Control) {
		ctrlPressed = true;
		e->accept();
	}
}
void PlotPad::keyReleaseEvent(QKeyEvent* e) {
	if (e->key() == Qt::Key::Key_Control) {
		ctrlPressed = false;
		e->accept();
	}
}

//鼠标事件
void PlotPad::mouseMoveEvent(QMouseEvent* e) {
	if (ctrlPressed && m_leftBtnPressed) {
		endPoint = e->pos();
		QPoint r = endPoint - startPoint;
		if (r.manhattanLength() >= 5) {
			if (lastLine) scene->removeItem(lastLine);
			lastLine = scene->addLine(startPoint.x(), startPoint.y(), endPoint.x(), endPoint.y());
		}
	}
	QGraphicsView::mouseMoveEvent(e);
}
void PlotPad::mousePressEvent(QMouseEvent* e) {
	if (e->button() == Qt::LeftButton) {//左键按下
		if (ctrlPressed) {//按下ctl
			m_leftBtnPressed = true;
			startPoint = e->pos();
		}
	}
	if(!ctrlPressed)QGraphicsView::mousePressEvent(e);
}
void PlotPad::mouseReleaseEvent(QMouseEvent* e) {
	if (e->button() == Qt::LeftButton)	{
		m_leftBtnPressed = false;
		if (lastLine) scene->removeItem(lastLine);
		if (ctrlPressed) {
			QTransform transform;
			Block* fromItem = NULL;
			Block* toItem = NULL;
			Item* fItem = (Item*)scene->itemAt(QPointF(startPoint), transform);
			Item* tItem = (Item*)scene->itemAt(QPointF(endPoint), transform);
			if (fItem && tItem && fItem->className() == "Block" && tItem->className() == "Block") {
				fromItem = (Block*)fItem;
				toItem = (Block*)tItem;
			}
			if (fromItem && toItem) {
				ArrowLine* line = new ArrowLine(fromItem, toItem, QPointF(0,0), QPointF(0, 0));
				scene->addItem(line);
				if (fromItem->toEdge) {
					fromItem->toEdge->getDest()->fromEdge = NULL;
					scene->removeItem(fromItem->toEdge);
					delete fromItem->toEdge;
					fromItem->toEdge = NULL;
				}
				fromItem->toEdge = line;
				if (toItem->fromEdge) {
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
}

//绘图事件
void PlotPad::paintEvent(QPaintEvent* e) {
	QGraphicsView::paintEvent(e);
}

///////////////////////////////////////////////////// PItem ////////////////////////////////////////
Block::Block(int x, int y, QString head) 
	:Item()
	, toItem(Q_NULLPTR)
	, toEdge(Q_NULLPTR)
	, fromEdge(Q_NULLPTR) {
	this->type = head;
	this->setPos(QPointF(x, y));
}

//Block::~Block(){
//	if (toEdge) {
//		delete toEdge;
//		toEdge = Q_NULLPTR;
//	}
//	if (fromEdge) {
//		delete fromEdge;
//		fromEdge = Q_NULLPTR;
//	}
//	if (toItem) {
//		delete toItem;
//		toItem = Q_NULLPTR;
//	}
//}
QString Block::className() {
	return "Block";
}

void Block::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	painter->setPen("darkslategray");
	painter->setFont(QFont("微软雅黑", 12));
	painter->setBrush(QBrush("lightcyan"));
	painter->drawRoundedRect(boundingRect(), 10, 10);
	painter->drawText(boundingRect(), Qt::AlignCenter, type);
	drawToItem(painter);
}


void Block::drawToItem(QPainter* painter) {
	if (toItem)	{
		QPointF pf = this->pos(), pt = toItem->pos();
		painter->drawLine(QPointF(0, 0), pt - pf);
	}
}

QRectF Block::boundingRect() const {
	qreal penWidth = 5;
	QPointF p = this->pos();
	//return QRectF(m_dropPos.x() - penWidth / 2, m_dropPos.y() - penWidth / 2,
	//	80 + penWidth, 40 + penWidth);
	return QRectF(-50 - penWidth / 2, -20 - penWidth / 2,
		100 + penWidth, 40 + penWidth);
	//return QRectF(0, 0, 80, 40);
}

//鼠标事件
void Block::mouseMoveEvent(QGraphicsSceneMouseEvent* e) {
	if(toEdge)	toEdge->adjust();
	if (fromEdge) fromEdge->adjust();
	QGraphicsItem::mouseMoveEvent(e);
}
//void Block::mousePressEvent(QGraphicsSceneMouseEvent* e)
//{
//
//}
//void Block::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
//{
//
//}



///////////////////////////////////////////////////// ArrowLine ////////////////////////////////////////

static const double Pi = 3.14159265358979323846264338327950288419717;
static double circle = 2.0 * Pi;

ArrowLine::ArrowLine(Block* sourceNode, Block* destNode, QPointF pointStart, QPointF pointEnd)
	: arrowSize(10)
	, source(Q_NULLPTR)
	, dest(Q_NULLPTR) {
	setFlags(ItemIsSelectable | ItemIsMovable);
	setAcceptedMouseButtons(0);
	//m_pointFlag = pointflag;

	m_pointStart = pointStart;//偏移量
	m_pointEnd = pointEnd;//偏移量
	source = sourceNode;
	dest = destNode;
	adjust();
}

//ArrowLine::~ArrowLine() {
//	if (source) {
//		delete source;
//		source = Q_NULLPTR;
//	}
//	if (dest) {
//		delete dest;
//		dest = Q_NULLPTR;
//	}
//}

Block* ArrowLine::getSrc() { return source; }
Block* ArrowLine::getDest() { return dest; }

QString ArrowLine::className() { return "ArrowLine"; }
void ArrowLine::adjust()
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
void ArrowLine::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	if (!source || !dest)
		return;
	QLineF line(sourcePoint + m_pointStart, destPoint + m_pointEnd);
	if (qFuzzyCompare(line.length(), qreal(0.)))
		return;
	painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	painter->drawLine(line);

	double angle = ::acos(line.dx() / line.length());
	if (line.dy() >= 0)
		angle = circle - angle;
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
void ArrowLine::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (event->modifiers() & Qt::ShiftModifier) {
		return;
	}
	QGraphicsItem::mouseMoveEvent(event);
}
