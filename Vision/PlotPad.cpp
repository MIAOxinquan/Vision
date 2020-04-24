#include "global.h"
#include "PlotPad.h"
#include "SmartEdit.h"

/*TipLabel*/
TipLabel::TipLabel()
	:QLabel()
{
	setMinimumWidth(120);
	setMaximumWidth(420);
}
void TipLabel::setElidedText(QString fullText) {
	QFontMetrics fontMtc = fontMetrics();
	int showWidth = width();
	QString elidedText = fontMtc.elidedText(fullText, Qt::ElideRight, showWidth);//返回一个带有省略号的字符串
	setText(elidedText);
}
void TipLabel::enterEvent(QEvent* event) {
	if (QEvent::Enter == event->type()) {
		int offsetX = width() / 2, offsetY = height() * 1.5;
		QToolTip::showText(QCursor::pos() - QPoint(offsetX, offsetY), text(), this);
	}
}
void TipLabel::leaveEvent(QEvent* event) {
	if (QEvent::Leave == event->type()) {
		QToolTip::hideText();
	}
}
/*Item*/
Item::Item() {
	setFlags(ItemIsSelectable | ItemIsMovable | ItemIsFocusable);
}

/*PlotPad*/
PlotPad::PlotPad(QGraphicsScene* scene)
	: QGraphicsView()
	, scene(Q_NULLPTR)
	, edit(Q_NULLPTR)
	, lastLine(Q_NULLPTR)
	, pathLabel(Q_NULLPTR)
	, root(Q_NULLPTR)
	, ctrlPressed(false)
	, leftBtnPressed(false)
	, indexTotal(-1)
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

void PlotPad::blockPlot(Block* it) {
	indexTotal++;
	it->id = indexTotal;
	if (s.top()->count() == 0)	setRoot(it);
	scene->addItem(it);
	it->setFocus();
}


void PlotPad::dropEvent(QDropEvent* event)
{
	setFocus();
	QString str = event->mimeData()->text();//获取text
	QPoint p = event->pos();//获取位置 --> PlotPad内的位置
	Item* pIt = (Item*)scene->itemAt(QPointF(p), QTransform());
	if (pIt && pIt->className() == "ArrowLine") return;
	Block* it = (Block*)pIt;
	Block* p1 = new Block(p.rx() , p.ry() , str);
	blockPlot(p1);
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
			lastLine = scene->addLine(startPoint.x(), startPoint.y(), endPoint.x(), endPoint.y(), QPen(Qt::DotLine));
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
		Item* pIt = (Item*)scene->itemAt(QPointF(e->pos()), QTransform());
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
		nodesOnPath.append(it->type);
	}
	if (e->button() == Qt::RightButton) {
		if (scene->focusItem()) {
			Item* focusedItem = (Item*)scene->focusItem();
			if ("Block" == focusedItem->className()) {
				Block* focusedBlock = (Block*)focusedItem;
				root->type = root->type.right(root->type.length() - 1);
				root->update();
				setRoot(focusedBlock);
			}
		}
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
	QList<Block*>* topList = this->s.top();
	Item* focusedItem = (Item*)this->scene->focusItem();
	if (focusedItem) {
		if (focusedItem->className() == "Block") {
			Block* focusedBlock = (Block*)focusedItem;
			topList->removeOne(focusedBlock);
			if (focusedBlock->inArrow) {
				focusedBlock->inArrow->deleteSelf();
			}
			if (focusedBlock->outArrow) {
				focusedBlock->outArrow->deleteSelf();
			}
			if (focusedBlock == root)
				setRoot(topList->first());
			focusedBlock->deleteSelf();
			focusedItem = Q_NULLPTR;
		}
		else {
			ArrowLine* focusedArrow = (ArrowLine*)focusedItem;
			focusedArrow->deleteSelf();
			focusedArrow = Q_NULLPTR;
			focusedItem = Q_NULLPTR;
		}
	}
}

/*设置根节点*/
void PlotPad::setRoot(Block* newRoot) {
	if (newRoot->type.at(0) != '*'){
		this->root = newRoot;
		newRoot->type = "*" + newRoot->type;
	}
	if (newRoot->inArrow)
		newRoot->inArrow->deleteSelf();
	newRoot->update();
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
				toBlock = (Block*)tItem;
				if (toBlock == root)return;
				fromItem = (Block*)fItem;
			}
			if (fromItem && toBlock)
			{
				ArrowLine* line = new ArrowLine(fromItem, toBlock, QPointF(0, 0), QPointF(0, 0));
				scene->addItem(line);
				line->setFocus();
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
}

/*Block*/
Block::Block(int x, int y, QString type)
	:Item()
	, outArrow(Q_NULLPTR)
	, inArrow(Q_NULLPTR)
	, childrenBlock(new QList<Block*>())
	, w(100)
	, h(40)
	, id(-1)
{
	this->type = type;
	setPos(QPointF(x, y));
	setZValue(0);
}

QString Block::className() { return "Block"; }

void Block::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setRenderHints(QPainter::Antialiasing
		| QPainter::SmoothPixmapTransform
		| QPainter::TextAntialiasing);
	if (hasFocus()) {
		setZValue(1);
		painter->setPen("lightYellow");
		painter->setFont(QFont("微软雅黑", 12, QFont::Bold));
		painter->setBrush(QBrush("orange"));
	}
	else {
		setZValue(0);
		painter->setPen("darkslategray");
		painter->setFont(QFont("微软雅黑", 12, QFont::Normal));
		painter->setBrush(QBrush("lightcyan"));
	}
	painter->drawRoundedRect(boundingRect(), 12, 12);
	painter->drawText(boundingRect(), Qt::AlignCenter, type);
}


QRectF Block::boundingRect() const
{
	QPointF p = this->pos();
	return QRectF(-w / 2, -h / 2, w, h);
}

//鼠标事件
void Block::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
	if (outArrow)	outArrow->adjust();
	if (inArrow)inArrow->adjust();
	QGraphicsItem::mouseMoveEvent(e);
}

//删除节点的子孙节点
void Block::deleteSelf() {
	if (childrenBlock) {
		for (int i = 0; i < childrenBlock->count(); i++) {
			if (childrenBlock->at(i)->inArrow) {
				childrenBlock->at(i)->inArrow->deleteSelf();
			}
			childrenBlock->at(i)->deleteSelf();
		}
	}
	delete this;
}


/*ArrowLine*/
static const double Pi = 3.14159265358979323846264338327950288419717;
static double TwoPi = 2.0 * Pi;

ArrowLine::ArrowLine(Block* sourceNode, Block* destNode, QPointF pointStart, QPointF pointEnd)
	: Item()
	, fromBlock(Q_NULLPTR)
	, toBlock(Q_NULLPTR)
	, arrowSize(10)
{
	setAcceptedMouseButtons(0);
	setZValue(1);
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
	double angle = ::acos(line.dx() / line.length());
	if (line.dy() >= 0)
		angle = TwoPi - angle;
	QPointF destArrowP1 = destPoint + m_pointEnd + QPointF(sin(angle - Pi / 3) * arrowSize,
		cos(angle - Pi / 3) * arrowSize);
	QPointF destArrowP2 = destPoint + m_pointEnd + QPointF(sin(angle - Pi + Pi / 3) * arrowSize,
		cos(angle - Pi + Pi / 3) * arrowSize);

	if (hasFocus())
		painter->setPen(QPen(QColor("orange"), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	else
		painter->setPen(QPen(QColor("darkslategray"), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

	painter->drawLine(line);
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

void ArrowLine::deleteSelf() {
	this->fromBlock->outArrow = Q_NULLPTR;
	this->toBlock->inArrow = Q_NULLPTR;
	delete this;
}