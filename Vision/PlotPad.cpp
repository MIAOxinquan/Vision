#include "global.h"
#include "PlotPad.h"
#include "SmartEdit.h"
#include "RecordObject.h"

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
	, undoRedoStack(new UndoRedoStack(this))
	, ctrlPressed(false)
	, leftBtnPressed(false)
	, indexTotal(-1)
{
	this->scene = scene;
	QGraphicsView::setScene(scene);
	setSceneRect(0, 0, 1920, 1600);
	setAcceptDrops(true);
	//去掉滚动条
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	startPoint = QPoint(-1, -1);
	endPoint = QPoint(-1, -1);
	blockStack.push(new QList<Block*>());
	/*加载qss*/
	loadStyleSheet(this,"plot.qss");
}


void PlotPad::dropEvent(QDropEvent* event)
{
	setFocus();
	QString type = event->mimeData()->text();//获取text
	QPoint ePos = event->pos();//获取位置 --> PlotPad内的位置
	Item* itemAtPos = (Item*)scene->itemAt(QPointF(ePos), QTransform());
	if (itemAtPos && itemAtPos->className() == "ArrowLine") return;
	Block* oldBlock = (Block*)itemAtPos;
	Block* newBlock = new Block(ePos.rx() , ePos.ry() , type);
	indexTotal++;
	newBlock->id = indexTotal;
	scene->addItem(newBlock);
	newBlock->setFocus();
	if (oldBlock) // oldBlock != NULL
	{
		qreal maxY = -1;
		for (int i = 0; i < oldBlock->childrenBlock->size(); ++i)
			if (oldBlock->childrenBlock->at(i)->pos().y() > maxY)
				maxY = oldBlock->childrenBlock->at(i)->pos().y();
		oldBlock->childrenBlock->append(newBlock);
		undoRedoStack->Do(new AddBlock(newBlock, oldBlock->childrenBlock));
		newBlock->setPos(200, maxY + 100);
		newBlock->hide();
	}
	else {
		if (0 == blockStack.top()->count()) {
			setRoot(newBlock);
			if (pathLabel)
				pathLabel->setElidedText(getNodesPath());
		}
		else {
			Block* temp = root;
			while (temp->outArrow) {
				temp = temp->outArrow->toBlock;
			}
			ArrowLine* newArrow = new ArrowLine(temp, newBlock, QPointF(0, 0), QPointF(0, 0));
			temp->outArrow = newArrow;
			newBlock->inArrow = newArrow;
			scene->addItem(newArrow);
		}
		blockStack.top()->push_back(newBlock);
		undoRedoStack->Do(new AddBlock(newBlock, blockStack.top()));
	}
	QGraphicsView::dropEvent(event);
}

void PlotPad::dragEnterEvent(QDragEnterEvent* event) { event->accept(); }
void PlotPad::dragMoveEvent(QDragMoveEvent* event) { event->accept(); }

//键盘事件
void PlotPad::keyPressEvent(QKeyEvent* e){
	e->accept();
	if (e->key() == Qt::Key::Key_Control) { ctrlPressed = true; }
}
void PlotPad::keyReleaseEvent(QKeyEvent* e){
	e->accept();
	if (e->key() == Qt::Key::Key_Control) { ctrlPressed = false; }
}

//鼠标事件
void PlotPad::mouseMoveEvent(QMouseEvent* e){
	if (ctrlPressed && leftBtnPressed){
		endPoint = e->pos();
		QPoint r = endPoint - startPoint;
		if (r.manhattanLength() >= 5){
			if (lastLine)scene->removeItem(lastLine);
			lastLine = scene->addLine(startPoint.x(), startPoint.y(), endPoint.x(), endPoint.y(), QPen(Qt::DotLine));
			lastLine->setZValue(1);
		}
	}
	QGraphicsView::mouseMoveEvent(e);
}
void PlotPad::mousePressEvent(QMouseEvent* e)
{
	if (ctrlPressed) {
		if (e->button() == Qt::LeftButton) {
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
		QList<Block*>* topList = this->blockStack.top();
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
		blockStack.push(it->childrenBlock);
		nodesOnPath.append(it->type);
	}
	if (e->button() == Qt::RightButton) {
		if (scene->focusItem()) {
			Item* focusedItem = (Item*)scene->focusItem();
			if ("Block" == focusedItem->className()) {
				Block* focusedBlock = (Block*)focusedItem;
				setRoot(focusedBlock);
			}
		}
	}
}

void PlotPad::backLevel()
{
	if (blockStack.size() > 1)
	{
		QList<Block*>* topList = this->blockStack.top();
		for (int i = 0; i < topList->size(); ++i)
		{
			topList->at(i)->hide();
			
			if (topList->at(i) && topList->at(i)->inArrow && topList->at(i)->inArrow->isVisible())
				topList->at(i)->inArrow->hide();
			if (topList->at(i) && topList->at(i)->outArrow && topList->at(i)->outArrow->isVisible())
				topList->at(i)->outArrow->hide();
		}

		blockStack.pop();
		nodesOnPath.removeLast();
		topList = this->blockStack.top();
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
	Item* focusedItem = (Item*)this->scene->focusItem();
	if (focusedItem) {
		if (focusedItem->className() == "Block") {
			Block* focusedBlock = (Block*)focusedItem;
			if (focusedBlock->inArrow) {
				focusedBlock->inArrow->deleteSelf();
			}
			if (focusedBlock->outArrow) {
				focusedBlock->outArrow->deleteSelf();
			}
			QList<Block*>* topBlock = blockStack.top();
			blockStack.top()->removeOne(focusedBlock);
			if (focusedBlock == root) {
				if (0 == topBlock->count()) {
					root = Q_NULLPTR;
				}
				else {
					setRoot(topBlock->first());
				}
			}
			focusedBlock->deleteSelf();
		}
		else {
			ArrowLine* focusedArrow = (ArrowLine*)focusedItem;
			focusedArrow->deleteSelf();
		}
		focusedItem = Q_NULLPTR;
	}
}

/*设置根节点*/
void PlotPad::setRoot(Block* newRoot) {
	if (root) {
		root->type = root->type.right(root->type.length() - 1);
		root->update();
	}
	if (newRoot->type.at(0) != '*'){
		root = newRoot;
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
		if (lastLine) scene->removeItem(lastLine);
		if (ctrlPressed)
		{
			QTransform transform;
			Item* fromItem = (Item*)scene->itemAt(QPointF(startPoint), transform);
			Item* toItem = (Item*)scene->itemAt(QPointF(endPoint), transform);
			if (fromItem && toItem
				&& fromItem->className() == "Block"
				&& toItem->className() == "Block") {
				/*强约束*/
				Block* endBlock = (Block*)toItem;
				if (endBlock == root)return;
				Block* startBlock = (Block*)fromItem;
				if (startBlock == endBlock)return;

				if (!startBlock->outArrow
					|| (startBlock->outArrow && startBlock->outArrow->toBlock != endBlock)) {
					qDebug() << "drawArrow";
					/*弱约束*/
					if (startBlock->outArrow) {
						startBlock->outArrow->toBlock->inArrow = Q_NULLPTR;
						scene->removeItem(startBlock->outArrow);
						delete startBlock->outArrow;
						startBlock->outArrow = Q_NULLPTR;
					}
					if (endBlock->inArrow) {
						endBlock->inArrow->fromBlock->outArrow = Q_NULLPTR;
						scene->removeItem(endBlock->inArrow);
						delete endBlock->inArrow;
						endBlock->inArrow = Q_NULLPTR;
					}
					/*强约束*/
					if (endBlock->outArrow) {
						int length = 0;
						Block* temp = endBlock;
						while (temp->outArrow && temp->outArrow->toBlock != endBlock) {
							temp = temp->outArrow->toBlock;
							length++;
						}
						if (temp == startBlock) {
							endBlock->outArrow->toBlock->inArrow = Q_NULLPTR;
							scene->removeItem(endBlock->outArrow);
							delete endBlock->outArrow;
							endBlock->outArrow = Q_NULLPTR;
						}
					}
					ArrowLine* newArrow = new ArrowLine(startBlock, endBlock, QPointF(0, 0), QPointF(0, 0));
					startBlock->outArrow = newArrow;
					endBlock->inArrow = newArrow;
					scene->addItem(newArrow);
					newArrow->setFocus();
				}
			}
		}
	}
	QGraphicsView::mouseReleaseEvent(e);
}
void PlotPad::undo() { undoRedoStack->Undo(); }
void PlotPad::redo() { undoRedoStack->Redo(); }

/*Block*/
Block::Block(int x, int y, QString type)
	:Item()
	, outArrow(Q_NULLPTR)
	, inArrow(Q_NULLPTR)
	, childrenBlock(new QList<Block*>())
	, w(150)
	, h(60)
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
		if (1 != zValue())setZValue(1);
		painter->setPen("lightYellow");
		painter->setFont(QFont("微软雅黑", 12, QFont::Bold));
		painter->setBrush(QBrush("orange"));
	}
	else {
		if (0 != zValue())setZValue(0);
		painter->setPen("darkslategray");
		painter->setFont(QFont("微软雅黑", 12, QFont::Normal));
		painter->setBrush(QBrush("lightBlue"));
	}
	painter->drawRoundedRect(boundingRect(), 12, 12);
	painter->drawText(boundingRect(), Qt::AlignCenter, type);
}


QRectF Block::boundingRect() const {
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
	setZValue(0);
	m_pointStart = pointStart;//偏移量
	m_pointEnd = pointEnd;//偏移量
	fromBlock = sourceNode;
	toBlock = destNode;
	adjust();
}

QString ArrowLine::className() { return "ArrowLine"; }

qreal ArrowLine::min(qreal r1, qreal r2) { return r1 < r2 ? r1 : r2; }
qreal ArrowLine::abs(qreal r) { return (r >= 0) ? r : -r; }

void ArrowLine::adjust() {
	if (!fromBlock || !toBlock)return;
	/*所有Block宽高相同*/
	int blockWidth = fromBlock->w, blockHeight = fromBlock->h
		, halfWidth = blockWidth / 2, halfHeight = blockHeight / 2;
	QPointF offsetPointF = fromBlock->pos() - toBlock->pos();
	qreal xF = 0, yF = 0, xT = 0, yT = 0
		, blockRatio = blockWidth / blockHeight
		, offsetX = offsetPointF.x(), offsetY = offsetPointF.y()
		, offsetRatio = abs(offsetX / offsetY);
	if (1 == fromBlock->zValue()) {
		if (offsetRatio >= blockRatio) {
			xF = (offsetX >= 0) ? -halfWidth : halfWidth;
			if (offsetX<-blockWidth || offsetX>blockWidth)xT = -xF;
			else if (offsetX > -halfWidth && offsetX < halfWidth)xT = xF;
			else yT = (offsetY >= 0) ? halfHeight : -halfHeight;
		}
		else {
			yF = (offsetY >= 0) ? -halfHeight : halfHeight;
			if (offsetY<-blockHeight || offsetY>blockHeight)yT = -yF;
			else if (offsetY > -halfHeight && offsetY < halfHeight)yT = yF;
			else xT = (offsetX >= 0) ? halfWidth : -halfWidth;
		}
	}
	else {
		if (offsetRatio >= blockRatio) {
			xT = (offsetX >= 0) ? halfWidth : -halfWidth;
			if (offsetX<-blockWidth || offsetX>blockWidth)xF = -xT;
			else if (offsetX > -halfWidth && offsetX < halfWidth)xF = xT;
			else yF = (offsetY >= 0) ? -halfHeight : halfHeight;
		}
		else {
			yT = (offsetY >= 0) ? halfHeight : -halfHeight;
			if (offsetY<-blockHeight || offsetY>blockHeight)yF = -yT;
			else if (offsetY > -halfHeight && offsetY < halfHeight)yF = yT;
			else xF = (offsetX >= 0) ? -halfWidth : halfWidth;
		}
	}
	QLineF line(mapFromItem(fromBlock, xF, yF), mapFromItem(toBlock, xT, yT));
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

	if (hasFocus()) {
		if (1 != zValue())setZValue(1);
		painter->setPen(QPen(QColor("orange"), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	}
	else {
		if (0 != zValue())setZValue(0);
		painter->setPen(QPen(QColor("darkslategray"), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	}
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