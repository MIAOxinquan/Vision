#include "global.h"
#include "PlotPad.h"
#include "SmartEdit.h"

/*TipLabel*/
TipLabel::TipLabel()
	:QLabel()
{
	setMinimumWidth(120);
	setMaximumWidth(540);
}
void TipLabel::setElidedText(QString fullText) {
	QFontMetrics fontMtc = fontMetrics();
	int showWidth = width();
	QString elidedText = fontMtc.elidedText(fullText, Qt::ElideRight, showWidth);//返回一个带有省略号的字符串
	setText(elidedText);
}
void TipLabel::enterEvent(QEvent* event) {
	if (QEvent::Enter == event->type()) {
		QToolTip::showText(QCursor::pos() - QPoint(width() / 2, 1.5 * height()), text(), this);
	}
}
void TipLabel::leaveEvent(QEvent* event) {
	if (QEvent::Leave == event->type()) {
		QToolTip::hideText();
	}
}

PlotPad::PlotPad(QGraphicsScene* scene)
	: QGraphicsView()
	, scene(Q_NULLPTR)
	, edit(Q_NULLPTR)
	, lastLine(Q_NULLPTR)
	, pathLabel(Q_NULLPTR)
	, root(Q_NULLPTR)
	, ctrlPressed(false)
	, leftBtnPressed(false)
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
	s.push(new QList<Block*>());
	loadStyleSheet(this,"plot.qss");
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
	QTransform transform;
	Item* pIt = (Item*)scene->itemAt(QPointF(p), transform);
	if (pIt && pIt->className() == "ArrowLine") return;
	Block* it = (Block*)pIt;
	Block* p1 = new Block(p.rx() , p.ry() , str);
	drawItems(p1);
	if (it) // it != NULL
	{
		qreal maxY = -1;
		for (int i = 0; i < it->childrenBlock->size(); ++i)
			if (it->childrenBlock->at(i)->pos().y() > maxY)
				maxY = it->childrenBlock->at(i)->pos().y();
		it->childrenBlock->push_back(p1);
		p1->setPos(200, maxY + 100);
		p1->hide();
	}
	else
		s.top()->push_back(p1);
	if (pathLabel)
		pathLabel->setElidedText(getNodesPath());
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
		ctrlPressed = true;
		e->accept();
	}
}
void PlotPad::keyReleaseEvent(QKeyEvent* e)
{
	if (e->key() == Qt::Key::Key_Control)
	{
		ctrlPressed = false;
		e->accept();
	}
}

//鼠标事件
void PlotPad::mouseMoveEvent(QMouseEvent* e)
{
	if (ctrlPressed && leftBtnPressed)
	{
		endPoint = e->pos();
		QPoint r = endPoint - startPoint;
		//Block* item = scene->itemAt()
		if (r.manhattanLength() >= 5)
		{
			if (lastLine)
				scene->removeItem(lastLine);
			lastLine = scene->addLine(startPoint.x(), startPoint.y(), endPoint.x(), endPoint.y());
		}
	}
	QGraphicsView::mouseMoveEvent(e);
}
void PlotPad::mousePressEvent(QMouseEvent* e)
{
	if (ctrlPressed) {//按下ctl
		if (e->button() == Qt::LeftButton) {//左键按下
			leftBtnPressed = true;
			startPoint = e->pos();
		}
	}
	else {
		QGraphicsView::mousePressEvent(e);
		if (itemAt(e->pos())) {
			Item* curItem = (Item*)itemAt(e->pos());
			if ("Block" == curItem->className()) {
				Block* curBlock = (Block*)curItem;
				edit->setPlainText(curBlock->content);
			}
			else
				return;
		}
		else {
			edit->setPlainText("Global");
		}
	}
}

void PlotPad::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		QTransform transform;
		Item* pIt = (Item*)scene->itemAt(QPointF(e->pos()), transform);
		if (pIt && pIt->className() == "ArrowLine") return;
		Block* it = (Block*)pIt;
		if (!it)return; // 如果it为NULL，则什么都不做
		QList<Block*>* topList = this->s.top();
		for (int i = 0; i < topList->size(); ++i)// 隐藏父亲节点同层节点
		{
			topList->at(i)->hide();
			if (topList->at(i) && topList->at(i)->inArrow && topList->at(i)->inArrow->isVisible())
				topList->at(i)->inArrow->hide();
			if (topList->at(i) && topList->at(i)->outArrow && topList->at(i)->outArrow->isVisible())
				topList->at(i)->outArrow->hide();
		}
		for (int i = 0; i < it->childrenBlock->size(); ++i)// 显示子层节点
		{
			it->childrenBlock->at(i)->show();
			if (it->childrenBlock->at(i)->inArrow && !it->childrenBlock->at(i)->inArrow->isVisible())
				it->childrenBlock->at(i)->inArrow->show();
			if (it->childrenBlock->at(i)->outArrow && !it->childrenBlock->at(i)->outArrow->isVisible())
				it->childrenBlock->at(i)->outArrow->show();
		}
		s.push(it->childrenBlock);
		nodesOnPath.append(it->head);
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
			
			if (topList->at(i) && topList->at(i)->inArrow && topList->at(i)->inArrow->isVisible())
				topList->at(i)->inArrow->hide();
			if (topList->at(i) && topList->at(i)->outArrow && topList->at(i)->outArrow->isVisible())
				topList->at(i)->outArrow->hide();
		}

		s.pop();
		nodesOnPath.removeLast();
		topList = this->s.top();
		for (int i = 0; i < topList->size(); ++i)
		{
			topList->at(i)->show();
			if (topList->at(i) && topList->at(i)->inArrow && !topList->at(i)->inArrow->isVisible())
				topList->at(i)->inArrow->show();
			if (topList->at(i) && topList->at(i)->outArrow && !topList->at(i)->outArrow->isVisible())
				topList->at(i)->outArrow->show();
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
		return;
	}
	scene->removeItem((QGraphicsItem*)i1);
	if (i1->className() == "Block") {
		Block* pItem = (Block*)i1;
		topList->removeOne(pItem);
		if (pItem->inArrow) {
			scene->removeItem(pItem->inArrow);
			pItem->inArrow->fromBlock->outArrow = NULL;
			delete pItem->inArrow;
			pItem->inArrow = NULL;
		}
		if (pItem->outArrow) {
			scene->removeItem(pItem->outArrow);
			pItem->outArrow->toBlock->inArrow = NULL;
			delete pItem->outArrow;
			pItem->outArrow = NULL;
		}
		pItem->removeItemAllSons(pItem);
		delete i1;
		i1 = NULL;
	}
	else if (i1->className() == "ArrowLine") {
		ArrowLine* pEdge = (ArrowLine*)i1;
		pEdge->deleteArrowLine(pEdge);
		i1 = NULL;
	}
}

QString PlotPad::getNodesPath() {
	QString nodesPath = "@PlotPad";
	int Count = nodesOnPath.count();
	for (int i = 0; i < Count; i++) {
		nodesPath += "//" + nodesOnPath.at(i);
	}
	return  nodesPath;
}

void PlotPad::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		leftBtnPressed = false;
		if (lastLine)
			scene->removeItem(lastLine);
		if (ctrlPressed)
		{
			QTransform transform;
			Block* fromItem = NULL;
			Block* toBlock = NULL;
			Item* fItem = (Item*)scene->itemAt(QPointF(startPoint), transform);
			Item* tItem = (Item*)scene->itemAt(QPointF(endPoint), transform);
			
			if (fItem && tItem && fItem->className() == "Block" && tItem->className() == "Block")
			{
				fromItem = (Block*)fItem;
				toBlock = (Block*)tItem;
			}
			if (fromItem && toBlock)
			{
				ArrowLine* line = new ArrowLine(fromItem, toBlock, QPointF(0, 0), QPointF(0, 0));
				scene->addItem(line);
				if (fromItem->outArrow)
				{
					fromItem->outArrow->toBlock->inArrow = NULL;
					scene->removeItem(fromItem->outArrow);
					delete fromItem->outArrow;
					fromItem->outArrow = NULL;
				}
				fromItem->outArrow = line;
				if (toBlock->inArrow)
				{
					toBlock->inArrow->fromBlock->outArrow = NULL;
					scene->removeItem(toBlock->inArrow);
					delete toBlock->inArrow;
					toBlock->inArrow = NULL;
				}
				toBlock->inArrow = line;
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
Block::Block(int x, int y, QString str)
	:Item()
	, outArrow(Q_NULLPTR)
	, inArrow(Q_NULLPTR)
	, childrenBlock(new QList<Block*>())
	, w(100)
	, h(40)
{
	this->head = str;
	this->setPos(QPointF(x, y));
}

QString Block::className() { return "Block"; }

void Block::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
	QWidget* widget)
{
	painter->setRenderHints(QPainter::Antialiasing
		| QPainter::SmoothPixmapTransform
		| QPainter::TextAntialiasing);
	painter->setBrush(QBrush("lightcyan"));
	painter->drawRoundedRect(boundingRect(), 10, 10);
	if (hasFocus()) {
		painter->setPen("blue");
		painter->setFont(QFont("微软雅黑", 12));
	}
	else {
		painter->setPen("darkslategray");
		painter->setFont(QFont("微软雅黑", 10));
	}
	painter->drawText(boundingRect(), Qt::AlignCenter, head);
}


QRectF Block::boundingRect() const
{
	QPointF p = this->pos();
	return QRectF(-w / 2, -h / 2, w, h);
}

//鼠标事件
void Block::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
	if (outArrow)
		outArrow->adjust();
	if (inArrow)
		inArrow->adjust();
	QGraphicsItem::mouseMoveEvent(e);
}

//删除节点的子孙节点
void Block::removeItemAllSons(Block* pItem) {
	if (!(pItem->childrenBlock)) {
		delete pItem;
		return;
	}
	QList<Block*>* p1 = pItem->childrenBlock;
	for (int i = 0; i < p1->size(); i++) {
		if (p1->at(i)->inArrow) {
			p1->at(i)->inArrow->deleteArrowLine(p1->at(i)->inArrow);
		}
		removeItemAllSons(p1->at(i));
	}
}


///////////////////////////////////////////////////// ArrowLine ////////////////////////////////////////

static const double Pi = 3.14159265358979323846264338327950288419717;
static double TwoPi = 2.0 * Pi;

ArrowLine::ArrowLine(Block* sourceNode, Block* destNode, QPointF pointStart, QPointF pointEnd)
	: Item()
	, fromBlock(Q_NULLPTR)
	, toBlock(Q_NULLPTR)
	, arrowSize(10)
{
	setFlags(ItemIsSelectable | ItemIsMovable | ItemIsFocusable);
	setAcceptedMouseButtons(0);
	m_pointStart = pointStart;//偏移量
	m_pointEnd = pointEnd;//偏移量
	fromBlock = sourceNode;
	toBlock = destNode;
	adjust();
}

QString ArrowLine::className() { return "ArrowLine"; }

qreal ArrowLine::min(qreal r1, qreal r2) { return r1 < r2 ? r1 : r2; }

qreal ArrowLine::abs(qreal r) { return (r >= 0) ? r : -r; }

void ArrowLine::adjust()
{
	if (!fromBlock || !toBlock)return;

	int sWidth = fromBlock->w, sHeight = fromBlock->h;
	int dWidth = toBlock->w, dHeight = toBlock->h;

	QPointF pS = fromBlock->pos(), pD = toBlock->pos();
	QRectF sRect = fromBlock->boundingRect();
	QRectF dRect = toBlock->boundingRect();

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
	QLineF line(mapFromItem(fromBlock, xS, yS), mapFromItem(toBlock, xD, yD));

	prepareGeometryChange();

	sourcePoint = line.p1();
	destPoint = line.p2();
}

QRectF ArrowLine::boundingRect() const
{
	if (!fromBlock || !toBlock)
		return QRectF();
	qreal extra = arrowSize / 2.0;
	return QRectF(sourcePoint + m_pointStart, QSizeF((destPoint + m_pointEnd).x() - (sourcePoint + m_pointStart).x(),
		(destPoint + m_pointEnd).y() - (sourcePoint + m_pointStart).y()))
		.normalized().adjusted(-extra, -extra, extra, extra);
}
QPainterPath ArrowLine::shape() const
{
	QPainterPath painterPath;
	painterPath.moveTo(sourcePoint);
	painterPath.lineTo(destPoint);
	QPainterPathStroker stroker;
	stroker.setWidth(1);
	painterPath = stroker.createStroke(painterPath);
	return painterPath;
}

void ArrowLine::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	painter->setRenderHints(QPainter::Antialiasing
		| QPainter::SmoothPixmapTransform
		| QPainter::TextAntialiasing);
	if (!fromBlock || !toBlock)return;
	QLineF line(sourcePoint + m_pointStart, destPoint + m_pointEnd);
	if (qFuzzyCompare(line.length(), qreal(0.)))
		return;
	if (hasFocus())
		painter->setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	else
		painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

	painter->drawLine(line);
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
}

void ArrowLine::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (event->modifiers() & Qt::ShiftModifier) {
		return;
	}
	QGraphicsItem::mouseMoveEvent(event);
}

void ArrowLine::deleteArrowLine(ArrowLine* pEdge) {
	pEdge->fromBlock->outArrow = Q_NULLPTR;
	pEdge->toBlock->inArrow = Q_NULLPTR;
	delete pEdge;
}