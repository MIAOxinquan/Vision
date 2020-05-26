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
	setCacheMode(ItemCoordinateCache);//�����ܽ���ˢ���ʣ�����cpuʹ�ã�i7-7700HQ��5%����
}
/*PlotPad*/
PlotPad::PlotPad(QGraphicsScene* scene)
	: QGraphicsView()
	, scene(Q_NULLPTR)
	, actionUndo(Q_NULLPTR)
	, actionRedo(Q_NULLPTR)
	, actionDelete(Q_NULLPTR)
	, actionBackLevel(Q_NULLPTR)
	, edit(Q_NULLPTR)
	, pathLabel(Q_NULLPTR)
	, lastLine(Q_NULLPTR)
	, root(Q_NULLPTR)
	, focusedBlock(Q_NULLPTR)
	, blockOnPath(new QList<Block*>)
	, recordList(new RecordList(this))
	, isNew(true)
	, ctrlPressed(false)
	, leftBtnPressed(false)
	, indexTotal(-1)
{
	this->scene = scene;
	QGraphicsView::setScene(scene);
	setSceneRect(0, 0, 900, 600);
	setAcceptDrops(true);
	//ȥ��������
	//setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	startPoint = QPoint(-1, -1);
	endPoint = QPoint(-1, -1);
	blockStack.push(new QList<Block*>());
	/*����qss*/
	loadStyleSheet(this,"pad.qss");
}

void PlotPad::addBlock(Block* newBlock)
{
	QList<Record*>* records = new QList<Record*>();
	addBlock(newBlock, records);
	delete records;
}

void PlotPad::addBlock(Block* newBlock, QList<Record*>* records) {
	scene->addItem(newBlock); //��newBlock��ӵ�scene��
	newBlock->setFocus();
	if (isNew)focusedBlock = newBlock;
	//QList<Record*>* records = new QList<Record*>(); //���ڼ�¼�����Ϊ�����ø��ڵ㡢�Լ����ܷ�������ɾ����Ϊ

	int tempLevel = blockStack.count();//��ȡ��ǰ����ĵȼ�
	newBlock->level = tempLevel;
	QList<Block*>* topBlock = blockStack.top();  //��ȡ��ǰ������ʾ��Block����
	topBlock->append(newBlock); //��newBlock������
	records->append(new AddBlock(newBlock, blockStack.top()));  //��¼һ�����newBlock����Ϊ ����redo undo
	
}

void PlotPad::addBlockIntoBlock(Block* oldBlock, Block* newBlock, QList<Record*>* records) {
	if (!records) {
		records = new QList<Record*>();
	}
	this->scene->addItem(newBlock);
	newBlock->setFocus();
	//focusedBlock = newBlock;
	qreal maxY = -1;
	QList<Block*>* innerBlocks = oldBlock->childrenBlock;
	int innerCount = innerBlocks->count();
	for (int i = 0; i < innerCount; ++i)
		if (innerBlocks->at(i)->pos().y() > maxY)
			maxY = innerBlocks->at(i)->pos().y();
	//newBlock->parentBlock = oldBlock;
	newBlock->setPos(200, maxY + 100);
	newBlock->hide();  //��Ӻ�Ҫ��ʾ
	oldBlock->addChildBlock(newBlock, records);
	if (isNew)oldBlock->idMarker(newBlock);
}

ArrowLine* PlotPad::connectBlocks(Block* src, Block* des, int _level, QList<Record*>* records)
{
	ArrowLine* newArrow = new ArrowLine(src, des, QPointF(0, 0), QPointF(0, 0), _level);
	//newArrow->level = tempLevel;  ���Ӧ���ڹ��캯���� 2020.5.10Mashiro��
	src->outArrow = newArrow;
	des->inArrow = newArrow;
	this->scene->addItem(newArrow);
	records->push_back(new AddArrowLine(newArrow));
	return newArrow;
}

/*PlotPad�ϵķ����¼���Ӧ ��Ҫ�������Block������ ��Ϊֱ�ӷ��ú���Ϊ�ӽڵ�*/
void PlotPad::dropEvent(QDropEvent* event) {
	setFocus();
	QString type = event->mimeData()->text();//��ȡtext
	QPoint ePos = event->pos();//��ȡλ�� --> PlotPad�ڵ�λ��
	Item* itemAtPos = (Item*)itemAt(ePos);
	if (itemAtPos && itemAtPos->className() == "ArrowLine") return;
	Block* oldBlock = (Block*)itemAtPos;
	Block* newBlock = new Block(ePos.rx(), ePos.ry(), type, getIndexTotal());
	DeleteCtrl();
	int tempLevel = blockStack.count();//��ȡ��ǰ����ĵȼ�
	//����һ����Ϣ
	QList<Record*>* records = new QList<Record*>();
	if (oldBlock) // oldBlock != Q_NULLPTR
	{
		//���������ӽڵ����
		newBlock->parentBlock = oldBlock;	//512
		tempLevel = tempLevel + 1;
		newBlock->level = tempLevel;
		//qreal maxY = -1;
		addBlockIntoBlock(oldBlock, newBlock, records);
		QList<Block*>* innerBlocks = oldBlock->childrenBlock;
		focusedBlock = Q_NULLPTR;
		if (1 == innerBlocks->count()) {
			//����ӽڵ�ֻ��һ���սڵ� ��ô Ӧ������  
			oldBlock->setChildRoot(newBlock);
			records->push_back(new ResetRoot(Q_NULLPTR, oldBlock, oldBlock->childRoot, newBlock));
		}
		else {
			Block* tempBlock = oldBlock->childRoot;
			while (tempBlock->outArrow) {
				tempBlock = tempBlock->outArrow->toBlock;
			}
			//ArrowLine* newArrow = new ArrowLine(tempBlock, newBlock, QPointF(0, 0), QPointF(0, 0));
			ArrowLine* newArrow = connectBlocks(tempBlock, newBlock, tempLevel, records); //��������
			newArrow->hide();
		}
	}
	else {
		//�������ӽڵ㵽��ǰ����
		if (this->blockOnPath->count() == 0) {	//512
			newBlock->parentBlock = Q_NULLPTR;
		}
		else {
			newBlock->parentBlock = this->blockOnPath->last();
		}
		addBlock(newBlock, records);//��ӽڵ㵽scene ����records��¼
		QList<Block*>* topBlock = blockStack.top();  //��ȡ��ǰ������ʾ��Block����
		focusedBlock = Q_NULLPTR;
		if (1 == topBlock->count()) {
			//topBlock����һ���սڵ���?
			if (1 == tempLevel) {
				//���newBlock�Ƕ�����ĵ�һ���ڵ�
				records->push_back(new ResetRoot(this, Q_NULLPTR, Q_NULLPTR, newBlock));
				setRoot(newBlock);  //����Ϊ�����ڵ�
			}
			else {
				//���newBlock�ǷǶ�����ĵ�һ���ڵ�
				Block* parentBlock = blockOnPath->last();  //��·����ʽ��ȡ���ڵ�
				records->push_back(new ResetRoot(Q_NULLPTR, parentBlock, parentBlock->childRoot, newBlock));
				parentBlock->setChildRoot(newBlock);
				newBlock->parentBlock = parentBlock;
			}
		}
		else {
			//��ǰ���治��һ���ڵ� ��ô�Ͳ�����������ڵ�Ϊ���ڵ� ������һ���ڵ����ӵ��������newBlock
			Block* tempBlock = Q_NULLPTR;
			//����һ�����ҵ���ǰ������ڵ�
			if (1 == tempLevel)
				tempBlock = root;
			else {
				Block* parentBlock = blockOnPath->last();
				tempBlock = parentBlock->childRoot;
				newBlock->parentBlock = parentBlock;
			}
			//���ø��ڵ� �������� �ҵ����һ���ڵ�
			while (tempBlock->outArrow) {
				tempBlock = tempBlock->outArrow->toBlock;
			}
			//����һ����
			ArrowLine* newArrow = connectBlocks(tempBlock, newBlock, tempLevel, records);
			//newArrow->level = tempLevel;  ���Ӧ���ڹ��캯���� 2020.5.10Mashiro��
		}
	}
	recordList->Do(records);
	UndoRedoCtrl();
	if (1 == blockStack.count()) {
		edit->setPlainText(edit->showContent(this));
	}
	else {
		edit->showContent(blockOnPath->last());
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
			edit->setReadOnly(false);
			Item* curItem = (Item*)itemAt(e->pos());
			if ("Block" == curItem->className()) {
				focusedBlock = (Block*)curItem;
				edit->showContent(focusedBlock);
			}
		}
		else {
			focusedBlock = Q_NULLPTR;
			if (1 == blockStack.count()) {
				edit->setPlainText(edit->showContent(this));
			}
			else {
				edit->showContent(blockOnPath->last());
			}
			edit->setReadOnly(true);
		}
		DeleteCtrl();
	}
}

void PlotPad::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (!focusedBlock)focusedBlock = Q_NULLPTR;
	if (e->button() == Qt::LeftButton)
	{
		Item* itemAtPos = (Item*)itemAt(e->pos());
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
		BackLevelCtrl();
		DeleteCtrl();
		if (pathLabel) {
			pathLabel->blockPath = getBlockPath();
			pathLabel->setElidedText();
		}
	}
	if (e->button() == Qt::RightButton) {
		if (scene->focusItem()) {
			Item* focusedItem = (Item*)scene->focusItem();
			if ("Block" == focusedItem->className()) {
				QList<Record*>* records = new QList<Record*>();
				Block* focusedBlock = (Block*)focusedItem;
				if (1 == blockStack.count()) {
					records->append(new ResetRoot(this, Q_NULLPTR, this->root, focusedBlock));
					setRoot(focusedBlock);
				}
				else {
					Block* parentBlock = blockOnPath->last();
					records->append(new ResetRoot(Q_NULLPTR, parentBlock, parentBlock->childRoot, focusedBlock));
					parentBlock->setChildRoot(focusedBlock);
				}
				if (focusedBlock->inArrow) {
					scene->removeItem(focusedBlock->inArrow);
					focusedBlock->inArrow->fromBlock->outArrow = Q_NULLPTR;
					records->append(new RemoveArrowLine(focusedBlock->inArrow));
					focusedBlock->inArrow = Q_NULLPTR;
				}
				recordList->Do(records);
				UndoRedoCtrl();
			}
		}
	}
}

void PlotPad::backLevel() {
	setFocus();
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
		BackLevelCtrl();
		DeleteCtrl();
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
		if (1 == blockStack.count()) {
			edit->setPlainText(edit->showContent(this));
		}
		else {
			edit->showContent(blockOnPath->last());
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
		edit->setPlainText(edit->showContent(this));
	else {
		edit->showContent(blockOnPath->last());
	}
}
// ������ֻremove��ǰBlock������������ArrowLine��ArrowLine��Block֮ǰ�Ƴ�
QList<Record*>* PlotPad::removeBlock(Block* block) {
	QList<Record*>* records = new QList<Record*>();
	//pzy�޸�
	QList<Block*>* listOn;
	if (1 == blockStack.top()->count(block))
		listOn = blockStack.top();
	else {
		Block* focusBlock = (Block*)this->scene->focusItem();
		listOn = focusBlock->childrenBlock;
	}
	listOn->removeOne(block);
	int listCount = listOn->count();
	for (int i = block->childrenBlock->count() - 1; i >= 0; --i) {
		records->append(*removeBlock(block->childrenBlock->at(i)));
	}
	if (block->inArrow)
	{
		scene->removeItem(block->inArrow);
		block->inArrow->fromBlock->outArrow = Q_NULLPTR;//��ͷ����ԴBlock��outArrowӦ����Ϊ��
		records->append(new RemoveArrowLine(block->inArrow));
		block->inArrow = Q_NULLPTR; // Ҫ��Ҫ��һ��?
	}
	if (block->outArrow)
	{
		scene->removeItem(block->outArrow);
		block->outArrow->toBlock->inArrow = Q_NULLPTR;
		records->append(new RemoveArrowLine(block->outArrow));
		block->outArrow = Q_NULLPTR;
	}
	if (1 == block->level) {
		if (0 == listCount) {	//�Ƴ������ڵ�
			root = Q_NULLPTR;
		}
		else {		//�Ƴ���һ��Ǹ��ڵ�
			Block* topFirst = listOn->first();
			records->append(new ResetRoot(this, Q_NULLPTR, this->root, topFirst));
			setRoot(topFirst);
			if (topFirst->inArrow) {
				scene->removeItem(topFirst->inArrow);
				topFirst->inArrow->fromBlock->outArrow = Q_NULLPTR;
				records->append(new RemoveArrowLine(topFirst->inArrow));
				topFirst->inArrow = Q_NULLPTR;
			}
		}
	}
	else {
		//Block* parentBlock = blockOnPath->last();
		Block* parentBlock = block->parentBlock;	//512
		if (block == parentBlock->childRoot) {
			if (0 == listCount) {
				parentBlock->childRoot = Q_NULLPTR;
			}
			else {
				Block* topFirst = listOn->first();
				records->append(new ResetRoot(Q_NULLPTR, parentBlock, parentBlock->childRoot, topFirst));
				parentBlock->setChildRoot(topFirst);
				if (topFirst->inArrow) {	//ɾ����root�����ͷ
					scene->removeItem(topFirst->inArrow);
					topFirst->inArrow->fromBlock->outArrow = Q_NULLPTR;
					records->append(new RemoveArrowLine(topFirst->inArrow));
					topFirst->inArrow = Q_NULLPTR;
				}
			}
		}
	}
	scene->removeItem(block); 
	if (block->level == blockStack.count()) {	//512
		records->append(new RemoveBlock(block, listOn));
		recordList->Do(records);
		UndoRedoCtrl();
	}
	else {
		records->append(new RemoveBlock(block, block->parentBlock->childrenBlock));
	}
	//qDebug() << root->id;
	return records;
}

/*ɾ����*/
void PlotPad::removeArrowLine(ArrowLine* arrowLine) {
	if (!arrowLine) return;
	scene->removeItem(arrowLine);
	arrowLine->toBlock->inArrow = Q_NULLPTR;
	arrowLine->fromBlock->outArrow = Q_NULLPTR;
	recordList->Do(new RemoveArrowLine(arrowLine));
	UndoRedoCtrl();
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
	//if (newRoot==Q_NULLPTR || newRoot->blockText.startsWith("*")) return;
	if (newRoot != root) {
		if (root && root->blockText.startsWith('*')) {
			QString tempText = root->blockText;
			root->blockText = tempText.right(tempText.length() - 2);
			root->update();
		}
		root = newRoot;
		if (newRoot && !newRoot->blockText.startsWith('*')) {
			newRoot->blockText = "* " + newRoot->blockText;
			newRoot->update();
		}
	}
}

/*PlotPad��������Block�����XML����*/
void PlotPad::outport(QString path)//Ҫ��֤root�ǵ�һ��Block�ӽڵ�
{
	//�򿪻򴴽��ļ�
	QFile file(path); //���·��������·������Դ·��������
	if (!file.open(QFile::WriteOnly | QFile::Truncate)) //������QIODevice��Truncate��ʾ���ԭ��������
		return;

	QDomDocument doc;
	QDomProcessingInstruction instruction;
	QDomText text;
	//д��xmlͷ��
	instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);
	//д����ڵ� pad
	QDomElement pad = doc.createElement("pad");
	pad.setAttribute("indexTotal", indexTotal);
	doc.appendChild(pad);
	QList<Block*>* blocks = blockStack.at(0);
	if (blocks)
	{
		QSet<Block*> Set;
		Block* root = this->root;
		qDebug() << root->id;
		for (int i = 0; i < blocks->size(); ++i)
		{
			Block* block = blocks->at(i);
			qDebug() << block->id;
			if (block && !block->inArrow && block != root) Set.insert(block);
		}
		if (root) root->outport(doc, pad);
		for each (Block* b in Set)
		{
			b->outport(doc, pad);
		}
	}
	//������ļ�
	QTextStream out_stream(&file);
	doc.save(out_stream, 4); //����4��
	file.close();
}

/*�õ���ǰ����·��*/
QString PlotPad::getBlockPath() {
	QString nodesPath = title;
	int Count = blockOnPath->count();
	for (int i = 0; i < Count; i++) {
		nodesPath += ">" + blockOnPath->at(i)->type;
	}
	return  nodesPath;
}

/*PlotPad�ϵ���ק�ͷź��� ����*/
void PlotPad::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		leftBtnPressed = false;
		if (lastLine) scene->removeItem(lastLine);
		if (ctrlPressed)
		{
			//�ⲿ������������Block  TODO ���ϴ���Ϊһ������ 2020.5.9
			QTransform transform;
			Item* fromItem = (Item*)itemAt(startPoint);
			Item* toItem = (Item*)itemAt(endPoint);
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
						records->append(new RemoveArrowLine(startBlock->outArrow));
						startBlock->outArrow = Q_NULLPTR;
					}
					if (endBlock->inArrow) {
						endBlock->inArrow->fromBlock->outArrow = Q_NULLPTR;
						scene->removeItem(endBlock->inArrow);
						records->append(new RemoveArrowLine(endBlock->inArrow));
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
							records->append(new RemoveArrowLine(endBlock->outArrow));
							endBlock->outArrow = Q_NULLPTR;
						}
					}
					ArrowLine* newArrow = new ArrowLine(startBlock, endBlock, QPointF(0, 0), QPointF(0, 0));
					newArrow->level = blockStack.count();
					startBlock->outArrow = newArrow;
					endBlock->inArrow = newArrow;
					scene->addItem(newArrow);
					records->append(new AddArrowLine(newArrow));
					recordList->Do(records);
					UndoRedoCtrl();
					newArrow->setFocus();
					focusedBlock = Q_NULLPTR;
					DeleteCtrl();
					if (1 == blockStack.count())
						edit->setPlainText(edit->showContent(this));
					else 
						edit->showContent(blockOnPath->last());
				}
			}
		}
	}
	QGraphicsView::mouseReleaseEvent(e);
}

/*���ư�ť ��ť�Ƿ���ʹ��*/
void PlotPad::UndoRedoCtrl() {
	if (actionUndo && actionRedo) {
		if (0 == recordList->undoList->count())
			actionUndo->setEnabled(false);
		else
			actionUndo->setEnabled(true);
		if (0 == recordList->redoList->count())
			actionRedo->setEnabled(false);
		else
			actionRedo->setEnabled(true);
	}
}
void PlotPad::BackLevelCtrl() {
	if (actionBackLevel) {
		if (0 == blockOnPath->count())
			actionBackLevel->setEnabled(false);
		else
			actionBackLevel->setEnabled(true);
	}
}
void PlotPad::DeleteCtrl() {
	if (actionDelete) {
		if (scene->focusItem())
			actionDelete->setEnabled(true);
		else
			actionDelete->setEnabled(false);
	}
}
void PlotPad::ActionCtrl() {
	UndoRedoCtrl();
	BackLevelCtrl();
	DeleteCtrl();
}
void PlotPad::undo() { 
	recordList->Undo(); 
	ActionCtrl();
}
void PlotPad::redo() { 
	recordList->Redo();
	ActionCtrl();
}

/******************************************************************/
/*Block*/
QString Block::className() { return "Block"; }
Block::Block(int x, int y, QString type)
	:Item()
	, outArrow(Q_NULLPTR)
	, inArrow(Q_NULLPTR)
	, childRoot(Q_NULLPTR)
	, parentBlock(Q_NULLPTR)
	, childrenBlock(new QList<Block*>())
	, w(150)
	, h(60)
	, id(-1)
{
	//����type
	this->type = type;
	//����ͼ�ο��е�����Ӧ����ʾʲô
	this->blockText = type;

	/*����content*/
	int index = toolKeys.indexOf(type);
	switch (index)
	{
	case 0:this->content = blockContent.at(0); break;
	case 1:case 2:case 3:case 4:
		this->content = type + blockContent.at(1); break;
	case 5:case 6:case 7:case 8:case 9:case 10:case 11:case 12:
		this->content = blockContent.at(index - 3); break;
	default: break;
	}
	//����λ��XY
	setPos(QPointF(x, y));
	setZValue(0);
}

Block::Block(int x, int y, QString str, int _id) 
	:Block(x,y,str) //C11���� �ò�����ʽ���ù��캯��
{
	this->id = _id;
}
Block::Block(QDomElement element)
	:Block(0,0,"untype")
{
	int x = element.attribute("x").toInt();
	int y = element.attribute("y").toInt();
	setPos(QPointF(x, y));
	this->content = element.attribute("content");
	this->type = element.attribute("type");
	//this->blockText = element.attribute("blockText");
	this->blockText = type;
	this->id = element.attribute("id").toInt();
}

void Block::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setRenderHints(QPainter::Antialiasing
		| QPainter::SmoothPixmapTransform
		| QPainter::TextAntialiasing);
	if (hasFocus()) {
		if (1 != zValue())	setZValue(1);
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
	if (outArrow)outArrow->adjust();
	if (inArrow)inArrow->adjust();
	QGraphicsItem::mouseMoveEvent(e);
}

void Block::outport(QDomDocument& doc, QDomElement& parent)//Ҫ��֤root�ǵ�һ��Block�ӽڵ�
{
	QDomElement me = doc.createElement("Block");
	double hasArrowLineDVal = this->outArrow ? 1 : 0;
	me.setAttribute("id", this->id); // ����id
	me.setAttribute("hasArrowLine", hasArrowLineDVal);//���� �Ƿ�����һ��
	QDomText text;//�������ű�ǩ�м��ֵ
	// ����X
	QDomElement X = doc.createElement("x");
	text = doc.createTextNode(QString::number(this->pos().x()));
	me.appendChild(X);
	X.appendChild(text);
	 //����X����
	me.setAttribute("x", QString::number(this->pos().x()));

	// ����Y
	QDomElement Y = doc.createElement("y");
	text = doc.createTextNode(QString::number(this->pos().y()));
	me.appendChild(Y);
	Y.appendChild(text);
	  //����Y����
	me.setAttribute("y", QString::number(this->pos().y()));

	// ����type
	QDomElement type = doc.createElement("type");
	text = doc.createTextNode(this->type);
	me.appendChild(type);
	type.appendChild(text);
	  //����type����
	me.setAttribute("type", this->type);

	// ����blockText
	QDomElement blockText = doc.createElement("blockText");
	text = doc.createTextNode(this->blockText);
	me.appendChild(blockText);
	blockText.appendChild(text);
	  //����blockText����
	me.setAttribute("blockText", this->blockText);

	// ����content
	QDomElement content = doc.createElement("content");
	text = doc.createTextNode(this->content);
	me.appendChild(content);
	content.appendChild(text);
	 //����content����
	me.setAttribute("content", this->content);
	// ������block
	if (childrenBlock)
	{
		QSet<Block*> Set;
		Block* root = this->childRoot;
		for (int i = 0; i < childrenBlock->size(); ++i)
		{
			Block* block = childrenBlock->at(i);
			if (block && !block->inArrow && block != root) Set.insert(block);
		}
		if(root) root->outport(doc, me);
		for each (Block* b in Set) b->outport(doc, me);
	}
	parent.appendChild(me);
	if (this->outArrow && this->outArrow->toBlock)
	{
		Block* b = this->outArrow->toBlock;
		b->outport(doc, parent);
	}
}

void Block::setChildRoot(Block* newChildRoot) {
	if (newChildRoot != childRoot) {
		if (childRoot && childRoot->blockText.startsWith('*')) {
			QString tempText = childRoot->blockText;
			childRoot->blockText = tempText.right(tempText.length() - 2);
			childRoot->update();
		}
		childRoot = newChildRoot;
		if (newChildRoot && !newChildRoot->blockText.startsWith('*')) {
			newChildRoot->blockText = "* " + newChildRoot->blockText;
			newChildRoot->update();
		}
	}
}
//��װ�����ӿ�
void Block::addChildBlock(Block*newChild,QList<Record*>*records) {
	newChild->parentBlock = this;
	newChild->level = this->level + 1;
	this->childrenBlock->append(newChild);
	records->append(new AddBlock(newChild, this->childrenBlock));
}
//��block�����ݲ���id��ʶ
void Block::idMarker(Block* newChild) {
	int index = toolKeys.indexOf(this->type), length = this->content.length();
	QString insertID = "#" + QString::number(newChild->id) + "\n";
	switch (index)
	{
	case 0://empty
		this->content.append(insertID); break;
	case 10://do_while
		this->content.insert(length - 9, insertID); break;
	case 1:case 2:case 3:case 4:
		this->content.insert(length - 2, insertID); break;
	default:
		this->content.insert(length - 1, insertID); break;
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

/*****************************************************************************/
/*ArrowLine*/
static const double Pi = 3.14159265358979323846264338327950288419717;
static double TwoPi = 2.0 * Pi;

QString ArrowLine::className() { return "ArrowLine"; }
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

ArrowLine::ArrowLine(Block* sourceItem, Block* destItem, QPointF pointStart, QPointF pointEnd, int _level)
	:ArrowLine(sourceItem, destItem, pointStart, pointEnd)
{
	this->level = _level;
}

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
	fromPoint = line.p1();
	toPoint = line.p2();
}

QRectF ArrowLine::boundingRect() const
{
	if (!fromBlock || !toBlock)
		return QRectF();
	qreal extra = arrowSize / 2.0;
	return QRectF(fromPoint + m_pointStart, QSizeF((toPoint + m_pointEnd).x() - (fromPoint + m_pointStart).x(),
		(toPoint + m_pointEnd).y() - (fromPoint + m_pointStart).y()))
		.normalized().adjusted(-extra, -extra, extra, extra);
}
QPainterPath ArrowLine::shape() const
{
	QPainterPath painterPath;
	painterPath.moveTo(fromPoint);
	painterPath.lineTo(toPoint);
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
	QLineF line(fromPoint + m_pointStart, toPoint + m_pointEnd);
	if (qFuzzyCompare(line.length(), qreal(0.)))
		return;
	double angle = ::acos(line.dx() / line.length());
	if (line.dy() >= 0)
		angle = TwoPi - angle;
	QPointF destArrowP1 = toPoint + m_pointEnd + QPointF(sin(angle - Pi / 3) * arrowSize,
		cos(angle - Pi / 3) * arrowSize);
	QPointF destArrowP2 = toPoint + m_pointEnd + QPointF(sin(angle - Pi + Pi / 3) * arrowSize,
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
	painter->drawLine(QLineF(destArrowP1, toPoint + m_pointEnd));
	painter->drawLine(QLineF(destArrowP2, toPoint + m_pointEnd));
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