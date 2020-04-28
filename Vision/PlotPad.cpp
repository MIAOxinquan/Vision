#include "global.h"
#include "PlotPad.h"
#include "SmartEdit.h"
#include "Record.h"

/*TipLabel*/
TipLabel::TipLabel()
	:QLabel()
{
	setMaximumWidth(400);
}
void TipLabel::setElidedText() {
	QFontMetrics fontMtc = fontMetrics();
	int pathStrWidth = fontMtc.width(blockPath), maxWidth = maximumWidth() - 20
		, showWidth = pathStrWidth <= maxWidth ? pathStrWidth : maxWidth;
	QString elidedText = fontMtc.elidedText(blockPath, Qt::ElideRight, showWidth);//����һ������ʡ�Ժŵ��ַ���
	setText(elidedText);
}
void TipLabel::enterEvent(QEvent* event) {
	if (QEvent::Enter == event->type()) {
		int offsetX = width() / 2, offsetY = height() * 1.5;
		QToolTip::showText(QCursor::pos() - QPoint(offsetX, offsetY), blockPath, this);
	}
}
void TipLabel::leaveEvent(QEvent* event) {
	if (QEvent::Leave == event->type()) {
		QToolTip::hideText();
	}
}
/*Item*/
Item::Item()
	:level(0)
{
	setFlags(ItemIsSelectable | ItemIsMovable | ItemIsFocusable);
}
/*PlotPad*/
PlotPad::PlotPad(QGraphicsScene* scene)
	: QGraphicsView()
	, scene(Q_NULLPTR)
	, edit(Q_NULLPTR)
	, pathLabel(Q_NULLPTR)
	, lastLine(Q_NULLPTR)
	, root(Q_NULLPTR)
	, blockOnPath(new QList<Block*>)
	, recordStack(new RecordStack(this))
	, ctrlPressed(false)
	, leftBtnPressed(false)
	, indexTotal(-1)
{
	this->scene = scene;
	QGraphicsView::setScene(scene);
	setSceneRect(0, 0, 1920, 1600);
	setAcceptDrops(true);
	//ȥ��������
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	startPoint = QPoint(-1, -1);
	endPoint = QPoint(-1, -1);
	blockStack.push(new QList<Block*>());
	/*����qss*/
	loadStyleSheet(this,"plot.qss");
}


void PlotPad::dropEvent(QDropEvent* event) {
	setFocus();
	QString type = event->mimeData()->text();//��ȡtext
	QPoint ePos = event->pos();//��ȡλ�� --> PlotPad�ڵ�λ��
	Item* itemAtPos = (Item*)scene->itemAt(QPointF(ePos), QTransform());
	if (itemAtPos && itemAtPos->className() == "ArrowLine") return;
	Block* oldBlock = (Block*)itemAtPos;
	Block* newBlock = new Block(ePos.rx() , ePos.ry() , type);
	newBlock->blockText = type;
	//content test start
	int index = toolKeys.indexOf(type);
	QString content;
	switch (index)
	{
	case 0:content = blockContent.at(0); break;
	case 1:case 2:case 3:case 4:
		content = type + blockContent.at(1); break;
	case 5:content = blockContent.at(2); break;
	case 6:content = blockContent.at(3); break;
	case 7:content = blockContent.at(4); break;
	case 8:content = blockContent.at(5); break;
	case 9:content = blockContent.at(6); break;
	case 10:content = blockContent.at(7); break;
	case 11:content = blockContent.at(8); break;
	case 12:content = blockContent.at(9); break;
	default:break;
	}
	newBlock->content = content+"\n";
	//content test end
	indexTotal++;
	newBlock->id = indexTotal;
	scene->addItem(newBlock);
	newBlock->setFocus();

	int tempLevel = blockStack.count();
	QList<Record*>* records = new QList<Record*>();
	if (oldBlock) // oldBlock != Q_NULLPTR
	{
		tempLevel++;
		newBlock->level = tempLevel;
		qreal maxY = -1;
		QList<Block*>* innerBlock = oldBlock->childrenBlock;
		int innerCount = innerBlock->count();
		for (int i = 0; i < innerCount; ++i)
			if (innerBlock->at(i)->pos().y() > maxY)
				maxY = innerBlock->at(i)->pos().y();
		newBlock->setPos(200, maxY + 100);
		newBlock->hide();
		innerBlock->append(newBlock);
		records->push_back(new AddBlock(newBlock, oldBlock->childrenBlock));
		if (1 == innerBlock->count()) 
			oldBlock->setChildRoot(newBlock);
		else {
			Block* tempBlock = oldBlock->childRoot;
			while (tempBlock->outArrow) {
				tempBlock = tempBlock->outArrow->toBlock;
			}
			ArrowLine* newArrow = new ArrowLine(tempBlock, newBlock, QPointF(0, 0), QPointF(0, 0));
			newArrow->hide();
			newArrow->level = tempLevel;
			tempBlock->outArrow = newArrow;
			newBlock->inArrow = newArrow;
			scene->addItem(newArrow);
			records->push_back(new AddArrowLine(newArrow));
		}
	}
	else {
		newBlock->level = tempLevel;
		QList<Block*>* topBlock = blockStack.top();
		topBlock->push_back(newBlock);
		records->push_back(new AddBlock(newBlock, blockStack.top()));
		if (1==topBlock->count()) {
			if (1 == tempLevel)
				setRoot(newBlock);
			else {
				Block* parentBlock = blockOnPath->last();
				parentBlock->setChildRoot(newBlock);
			}
			if (newBlock->inArrow) {
				scene->removeItem(newBlock->inArrow);
				newBlock->inArrow->fromBlock->outArrow = Q_NULLPTR;
				records->push_back(new DeleteArrowLine(newBlock->inArrow));
				newBlock->inArrow = Q_NULLPTR;
			}
		}
		else {
			Block* tempBlock = Q_NULLPTR;
			if (1 == tempLevel)
				tempBlock = root;
			else
				tempBlock = blockOnPath->last()->childRoot;
			while (tempBlock->outArrow) {
				tempBlock = tempBlock->outArrow->toBlock;
			}
			ArrowLine* newArrow = new ArrowLine(tempBlock, newBlock, QPointF(0, 0), QPointF(0, 0));
			newArrow->level = tempLevel;
			tempBlock->outArrow = newArrow;
			newBlock->inArrow = newArrow;
			scene->addItem(newArrow);
			records->push_back(new AddArrowLine(newArrow));
		}
	}
	recordStack->Do(records);
	if (1 == blockStack.count())
		edit->showContent(this);
	else {
		Block* parentBlock = blockOnPath->last();
		edit->showContent(parentBlock);
	}
	QGraphicsView::dropEvent(event);
}

void PlotPad::dragEnterEvent(QDragEnterEvent* event) { event->accept(); }
void PlotPad::dragMoveEvent(QDragMoveEvent* event) { event->accept(); }

//�����¼�
void PlotPad::keyPressEvent(QKeyEvent* e){
	e->accept();
	if (e->key() == Qt::Key::Key_Control) { ctrlPressed = true; }
}
void PlotPad::keyReleaseEvent(QKeyEvent* e){
	e->accept();
	if (e->key() == Qt::Key::Key_Control) { ctrlPressed = false; }
}

//����¼�
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
				edit->showContent(curBlock);
			}
		}
		else {
			if (1 == blockStack.count())
				edit->showContent(this);
			else {
				Block* parentBlock = blockOnPath->last();
				edit->showContent(parentBlock);
			}
		}
	}
}

void PlotPad::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		Item* itemAtPos = (Item*)scene->itemAt(QPointF(e->pos()), QTransform());
		if (itemAtPos && itemAtPos->className() == "ArrowLine") return;
		Block* blockAtPos = (Block*)itemAtPos;
		if (!blockAtPos)return; // ���itΪQ_NULLPTR����ʲô������
		QList<Block*>* topBlock = blockStack.top();
		for (int i = 0; i < topBlock->count(); ++i) {// ���ظ��׽ڵ�ͬ��ڵ�
			topBlock->at(i)->hide();
			if (topBlock->at(i) && topBlock->at(i)->inArrow && topBlock->at(i)->inArrow->isVisible())
				topBlock->at(i)->inArrow->hide();
			if (topBlock->at(i) && topBlock->at(i)->outArrow && topBlock->at(i)->outArrow->isVisible())
				topBlock->at(i)->outArrow->hide();
		}
		for (int i = 0; i < blockAtPos->childrenBlock->count(); ++i) {// ��ʾ�Ӳ�ڵ�
			blockAtPos->childrenBlock->at(i)->show();
			if (blockAtPos->childrenBlock->at(i)->inArrow && !blockAtPos->childrenBlock->at(i)->inArrow->isVisible())
				blockAtPos->childrenBlock->at(i)->inArrow->show();
			if (blockAtPos->childrenBlock->at(i)->outArrow && !blockAtPos->childrenBlock->at(i)->outArrow->isVisible())
				blockAtPos->childrenBlock->at(i)->outArrow->show();
		}
		if (blockAtPos->childrenBlock) {
			blockStack.push(blockAtPos->childrenBlock);
		}
		else {
			blockStack.push(new QList<Block*>());
		}
		blockOnPath->append(blockAtPos);
		if (pathLabel) {
			pathLabel->blockPath = getBlockPath();
			pathLabel->setElidedText();
		}
	}
	if (e->button() == Qt::RightButton) {
		if (scene->focusItem()) {
			Item* focusedItem = (Item*)scene->focusItem();
			if ("Block" == focusedItem->className()) {
				Block* focusedBlock = (Block*)focusedItem;
				if (1 == blockStack.count())
					setRoot(focusedBlock);
				else {
					Block* parentBlock = blockOnPath->last();
					parentBlock->setChildRoot(focusedBlock);
				}
				if (focusedBlock->inArrow) {
					scene->removeItem(focusedBlock->inArrow);
					focusedBlock->inArrow->fromBlock->outArrow = Q_NULLPTR;
					recordStack->Do(new DeleteArrowLine(focusedBlock->inArrow));
					focusedBlock->inArrow = Q_NULLPTR;
				}
			}
		}
	}
}

void PlotPad::backLevel() {
	if (blockStack.count() > 1)	{
		QList<Block*>* topBlock = blockStack.top();
		for (int i = 0; i < topBlock->size(); ++i) {
			topBlock->at(i)->hide();
			if (topBlock->at(i) && topBlock->at(i)->inArrow && topBlock->at(i)->inArrow->isVisible())
				topBlock->at(i)->inArrow->hide();
			if (topBlock->at(i) && topBlock->at(i)->outArrow && topBlock->at(i)->outArrow->isVisible())
				topBlock->at(i)->outArrow->hide();
		}
		blockStack.pop();
		blockOnPath->removeLast();
		topBlock = blockStack.top();
		for (int i = 0; i < topBlock->size(); ++i) {
			topBlock->at(i)->show();
			if (topBlock->at(i) && topBlock->at(i)->inArrow && !topBlock->at(i)->inArrow->isVisible())
				topBlock->at(i)->inArrow->show();
			if (topBlock->at(i) && topBlock->at(i)->outArrow && !topBlock->at(i)->outArrow->isVisible())
				topBlock->at(i)->outArrow->show();
		}
		if (pathLabel) {
			pathLabel->blockPath = getBlockPath();
			pathLabel->setElidedText();
		}
		if (1 == blockStack.count())
			edit->showContent(this);
		else {
			Block* parentBlock = blockOnPath->last();
			edit->showContent(parentBlock);
		}
	}
}
void PlotPad::removeItem()
{
	Item* focusedItem = (Item*)this->scene->focusItem();
	if (!focusedItem) return;
	if (focusedItem->className() == "Block") {
		removeBlock((Block*)focusedItem);
	}
	else {
		removeArrowLine((ArrowLine*)focusedItem);
	}
	if (1 == blockStack.count())
		edit->showContent(this);
	else {
		Block* parentBlock = blockOnPath->last();
		edit->showContent(parentBlock);
	}
}
// ������ֻremove��ǰBlock������������ArrowLine��ArrowLine��Block֮ǰ�Ƴ�
QList<Record*>* PlotPad::removeBlock(Block* block) {
	QList<Record*>* records = new QList<Record*>();
	QList<Block*>* topBlock = blockStack.top();
	topBlock->removeOne(block);
	int topCount = topBlock->count(), tempLevel = blockStack.count();
	for (int i = block->childrenBlock->count() - 1; i >= 0; --i) {
		records->append(*removeBlock(block->childrenBlock->at(i)));
	}
	if (block->inArrow)
	{
		scene->removeItem(block->inArrow);
		block->inArrow->fromBlock->outArrow = Q_NULLPTR;//��ͷ����ԴBlock��outArrowӦ����Ϊ��
		records->push_back(new DeleteArrowLine(block->inArrow));
		block->inArrow = Q_NULLPTR; // Ҫ��Ҫ��һ��?
	}
	if (block->outArrow)
	{
		scene->removeItem(block->outArrow);
		block->outArrow->toBlock->inArrow = Q_NULLPTR;
		records->push_back(new DeleteArrowLine(block->outArrow));
		block->outArrow = Q_NULLPTR;
	}
	if (1 == tempLevel) {
		if (0 == topCount)
			root = Q_NULLPTR;
		else {
			Block* topFirst = topBlock->first();
			setRoot(topFirst);
			if (topFirst->inArrow) {
				scene->removeItem(topFirst->inArrow);
				topFirst->inArrow->fromBlock->outArrow = Q_NULLPTR;
				records->push_back(new DeleteArrowLine(topFirst->inArrow));
				topFirst->inArrow = Q_NULLPTR;
			}
		}
	}
	else {
		Block* parentBlock = blockOnPath->last();
		if (block == parentBlock->childRoot) {
			if (0 == topCount)
				parentBlock->childRoot = Q_NULLPTR;
			else {
				Block* topFirst = topBlock->first();
				parentBlock->setChildRoot(topFirst);
				if (topFirst->inArrow) {
					scene->removeItem(topFirst->inArrow);
					topFirst->inArrow->fromBlock->outArrow = Q_NULLPTR;
					records->push_back(new DeleteArrowLine(topFirst->inArrow));
					topFirst->inArrow = Q_NULLPTR;
				}
			}
		}
	}
	scene->removeItem(block); 
	if (block->level == tempLevel) {
		records->push_back(new DeleteBlock(block, topBlock));
		recordStack->Do(records);
	}
	else {
		Block* parentBlock = (Block*)scene->focusItem();
		records->push_back(new DeleteBlock(block, parentBlock->childrenBlock));
	}
	return records;
}

void PlotPad::removeArrowLine(ArrowLine* arrowLine) {
	if (!arrowLine) return;
	scene->removeItem(arrowLine);
	arrowLine->toBlock->inArrow = Q_NULLPTR;
	arrowLine->fromBlock->outArrow = Q_NULLPTR;
	recordStack->Do(new DeleteArrowLine(arrowLine));
}

////ɾ��item
//void PlotPad::deleteItem() {
//	Item* focusedItem = (Item*)this->scene->focusItem();
//	if (focusedItem) {
//		if (focusedItem->className() == "Block") {
//			Block* focusedBlock = (Block*)focusedItem;
//			if (focusedBlock->inArrow) {
//				focusedBlock->inArrow->deleteSelf();
//			}
//			if (focusedBlock->outArrow) {
//				focusedBlock->outArrow->deleteSelf();
//			}
//			QList<Block*>* topBlock = blockStack.top();
//			blockStack.top()->removeOne(focusedBlock);
//			if (focusedBlock == root) {
//				if (0 == topBlock->count()) {
//					root = Q_NULLPTR;
//				}
//				else {
//					setRoot(topBlock->first());
//				}
//			}
//			focusedBlock->deleteSelf();
//		}
//		else {
//			ArrowLine* focusedArrow = (ArrowLine*)focusedItem;
//			focusedArrow->deleteSelf();
//		}
//		focusedItem = Q_NULLPTR;
//	}
//}

/*���ø��ڵ�*/
void PlotPad::setRoot(Block* newRoot) {
	if (newRoot != root) {
		if (root) {
			QString tempText = root->blockText;
			root->blockText = tempText.right(tempText.length() - 2);
			root->update();
		}
		root = newRoot;
		newRoot->blockText = "* " + newRoot->blockText;
		newRoot->update();
	}
}

QString PlotPad::getBlockPath() {
	QString nodesPath = title;
	int Count = blockOnPath->count();
	for (int i = 0; i < Count; i++) {
		nodesPath += ">>" + blockOnPath->at(i)->type;
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
				Block* endBlock = (Block*)toItem;
				if (1 == blockStack.count()) {
					if (endBlock == root)return;
				}
				else {
					Block* parentBlock = blockOnPath->last();
					if (endBlock == parentBlock->childRoot)return;
				}
				Block* startBlock = (Block*)fromItem;
				if (startBlock == endBlock)return;

				if (!startBlock->outArrow
					|| (startBlock->outArrow && startBlock->outArrow->toBlock != endBlock)) {
					QList<Record*>* records = new QList<Record*>();
					/*��Լ��*/
					if (startBlock->outArrow) {
						startBlock->outArrow->toBlock->inArrow = Q_NULLPTR;
						scene->removeItem(startBlock->outArrow);
						records->push_back(new DeleteArrowLine(startBlock->outArrow));
						startBlock->outArrow = Q_NULLPTR;
					}
					if (endBlock->inArrow) {
						endBlock->inArrow->fromBlock->outArrow = Q_NULLPTR;
						scene->removeItem(endBlock->inArrow);
						records->push_back(new DeleteArrowLine(endBlock->inArrow));
						endBlock->inArrow = Q_NULLPTR;
					}
					/*ǿԼ��*/
					if (endBlock->outArrow) {
						Block* temp = endBlock;
						while (temp->outArrow && temp->outArrow->toBlock != endBlock) {
							temp = temp->outArrow->toBlock;
						}
						if (temp == startBlock) {
							endBlock->outArrow->toBlock->inArrow = Q_NULLPTR;
							scene->removeItem(endBlock->outArrow);
							records->push_back(new DeleteArrowLine(endBlock->outArrow));
							endBlock->outArrow = Q_NULLPTR;
						}
					}
					ArrowLine* newArrow = new ArrowLine(startBlock, endBlock, QPointF(0, 0), QPointF(0, 0));
					newArrow->level = blockStack.count();
					startBlock->outArrow = newArrow;
					endBlock->inArrow = newArrow;
					scene->addItem(newArrow);
					records->push_back(new AddArrowLine(newArrow));
					recordStack->Do(records);
					newArrow->setFocus();
					if (1 == blockStack.count())
						edit->showContent(this);
					else {
						Block* parentBlock = blockOnPath->last();
						edit->showContent(parentBlock);
					}
				}
			}
		}
	}
	QGraphicsView::mouseReleaseEvent(e);
}
void PlotPad::undo() { recordStack->Undo(); }
void PlotPad::redo() { recordStack->Redo(); }

/*Block*/
Block::Block(int x, int y, QString type)
	:Item()
	, outArrow(Q_NULLPTR)
	, inArrow(Q_NULLPTR)
	, childRoot(Q_NULLPTR)
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
		painter->setFont(QFont("΢���ź�", 12, QFont::Bold));
		painter->setBrush(QBrush("orange"));
	}
	else {
		if (0 != zValue())setZValue(0);
		painter->setPen("darkslategray");
		painter->setFont(QFont("΢���ź�", 12, QFont::Normal));
		painter->setBrush(QBrush("lightBlue"));
	}
	painter->drawRoundedRect(boundingRect(), 12, 12);
	painter->drawText(boundingRect(), Qt::AlignCenter, blockText);
}


QRectF Block::boundingRect() const {
	return QRectF(-w / 2, -h / 2, w, h);
}

//����¼�
void Block::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
	if (outArrow)	outArrow->adjust();
	if (inArrow)inArrow->adjust();
	QGraphicsItem::mouseMoveEvent(e);
}

void Block::setChildRoot(Block* newChildRoot) {
	if (newChildRoot != childRoot) {
		if (childRoot) {
			QString tempText = childRoot->blockText;
			childRoot->blockText = tempText.right(tempText.length() - 2);
			childRoot->update();
		}
		childRoot = newChildRoot;
		newChildRoot->blockText = "* " + newChildRoot->blockText;
		newChildRoot->update();
	}
}
//ɾ���ڵ������ڵ�
//void Block::deleteSelf() {
//	if (childrenBlock) {
//		for (int i = 0; i < childrenBlock->count(); i++) {
//			if (childrenBlock->at(i)->inArrow) {
//				childrenBlock->at(i)->inArrow->deleteSelf();
//			}
//			childrenBlock->at(i)->deleteSelf();
//		}
//	}
//	delete this;
//}

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
	m_pointStart = pointStart;//ƫ����
	m_pointEnd = pointEnd;//ƫ����
	fromBlock = sourceNode;
	toBlock = destNode;
	adjust();
}

QString ArrowLine::className() { return "ArrowLine"; }

qreal ArrowLine::min(qreal r1, qreal r2) { return r1 < r2 ? r1 : r2; }
qreal ArrowLine::abs(qreal r) { return (r >= 0) ? r : -r; }

void ArrowLine::adjust() {
	if (!fromBlock || !toBlock)return;
	/*����Block�����ͬ*/
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

//void ArrowLine::deleteSelf() {
//	this->fromBlock->outArrow = Q_NULLPTR;
//	this->toBlock->inArrow = Q_NULLPTR;
//	delete this;
//}