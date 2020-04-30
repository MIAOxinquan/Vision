#include "Record.h"
#include "PlotPad.h"

/*AddArrowLine*/
QString AddArrowLine::className() { return "AddArrowLine"; }
AddArrowLine::AddArrowLine(ArrowLine* arrowLine) { this->arrowLine = arrowLine; }

/*AddBlock*/
QString AddBlock::className() { return "AddBlock"; }
AddBlock::AddBlock(Block* block, QList<Block*>* belongingList) {
	this->block = block;
	this->belongingList = belongingList;
}

/*MoveBlock*/
QString MoveBlock::className() { return "MoveBlock"; }
MoveBlock::MoveBlock(Block* block)
{
	this->block = block;
}

/*RemoveArrowLine*/
QString RemoveArrowLine::className() { return "RemoveArrowLine"; }
RemoveArrowLine::RemoveArrowLine(ArrowLine* arrowLine) { this->arrowLine = arrowLine; }

/*RemoveBlock*/
QString RemoveBlock::className() { return "RemoveBlock"; }
RemoveBlock::RemoveBlock(Block* block, QList<Block*>* belongingList) {
	this->block = block;
	this->belongingList = belongingList;
}

/*ResetRoot*/
QString ResetRoot::className() { return "ResetRoot"; }
ResetRoot::ResetRoot(PlotPad* pad, Block* parentBlock, Block* oldRoot,Block*newRoot) {
	this->parentBlock = parentBlock;
	this->pad = pad;
	this->oldRoot = oldRoot;
	this->newRoot = newRoot;
}

/*RecordList*/
RecordList::RecordList(PlotPad* pad)
	: undoList(new QList<QList<Record*>*>())
	, redoList(new QList<QList<Record*>*>())
{
	this->pad = pad;
}

RecordList::~RecordList()
{

}
void RecordList::levelShow(Item* item) {
	int level = item->level;
	if (level > 0) {
		int stackLevel = pad->blockStack.count();
		if (level > pad->blockStack.count())item->hide();
		while (level < stackLevel) {
			pad->backLevel();
			stackLevel--;
		}
		if (level == stackLevel)item->show();
	}
}
void RecordList::InDo(Record* record)
{
	if (undoList->count() == MAX_REDO_STEP)//如果已经满了，则删除第一个并处理
	{
		QList<Record*>* handleRecords = undoList->first();
		undoList->removeFirst();
		delayedHandleUndoRecord(handleRecords);
	}
	QList<Record*>* records = new QList<Record*>();
	records->append(record);
	undoList->append(records);
}

void RecordList::InDo(QList<Record*>* records)
{
	if (undoList->count() == MAX_REDO_STEP)//如果已经满了，则删除第一个并处理
	{
		QList<Record*>* handleRecords = undoList->first();
		undoList->removeFirst();
		delayedHandleUndoRecord(handleRecords);
	}
	undoList->append(records);
}

void RecordList::Do(Record* record)
{
	InDo(record);
	//清空redoList
	clearRedoList();
}

void RecordList::Do(QList<Record*>* records)
{
	InDo(records);
	//清空redoList
	clearRedoList();
}

void RecordList::clearRedoList() {
	while (!redoList->empty()) {
		QList<Record*>* records = redoList->last(); redoList->removeLast();
		while (!records->empty()) {
			Record* record = records->last(); records->removeLast();
			delayedHandleRedoRecord(record);
			delete record;
		}
		delete records;
	}
}

void RecordList::Redo() {
	if (redoList->empty()) return;
	QList<Record*>* records = redoList->last(); redoList->removeLast();
	for (int i = records->count() - 1; i >= 0; --i) {
		QString recordName = records->at(i)->className();
		if (recordName == "AddArrowLine") {
			AddArrowLine* concreteRecord = (AddArrowLine*)records->at(i);
			concreteRecord->arrowLine->fromBlock->outArrow = concreteRecord->arrowLine;
			concreteRecord->arrowLine->toBlock->inArrow = concreteRecord->arrowLine;
			pad->scene->addItem(concreteRecord->arrowLine);
			levelShow(concreteRecord->arrowLine);
		}
		else if (recordName == "AddBlock") {
			AddBlock* concreteRecord = (AddBlock*)records->at(i);
			pad->scene->addItem(concreteRecord->block);
			concreteRecord->belongingList->append(concreteRecord->block);
			levelShow(concreteRecord->block);
			if (concreteRecord->block->inArrow)//应该不会执行
			{
				concreteRecord->block->inArrow->fromBlock->outArrow = concreteRecord->block->inArrow;
				pad->scene->addItem(concreteRecord->block->inArrow);
				levelShow(concreteRecord->block->inArrow);
				//delete concreteRecord->block->inArrow;//后面要修改，因为要Redo，所以此处不能删
			}
			if (concreteRecord->block->outArrow)//应该不会执行
			{
				concreteRecord->block->outArrow->toBlock->inArrow = concreteRecord->block->outArrow;
				pad->scene->addItem(concreteRecord->block->outArrow);
				levelShow(concreteRecord->block->outArrow);
				//delete concreteRecord->block->outArrow;//后面要修改，因为要Redo，所以此处不能删
			}
		}
		else if (recordName == "MoveBlock") {
			MoveBlock* concreteRecord = (MoveBlock*)records->at(i);

		}
		else if (recordName == "RemoveArrowLine") {
			RemoveArrowLine* concreteRecord = (RemoveArrowLine*)records->at(i);
			if (concreteRecord->arrowLine->fromBlock)
				concreteRecord->arrowLine->fromBlock->outArrow = Q_NULLPTR;
			if (concreteRecord->arrowLine->toBlock)
				concreteRecord->arrowLine->toBlock->inArrow = Q_NULLPTR;
			pad->scene->removeItem(concreteRecord->arrowLine);
			levelShow(concreteRecord->arrowLine);
		}
		else if (recordName == "RemoveBlock") {
			RemoveBlock* concreteRecord = (RemoveBlock*)records->at(i);
			if (concreteRecord->belongingList && concreteRecord->belongingList->count(concreteRecord->block) == 1)
				concreteRecord->belongingList->removeOne(concreteRecord->block);

			pad->scene->removeItem(concreteRecord->block);//不需要考虑箭头，箭头在别处考虑了
			concreteRecord->belongingList->removeOne(concreteRecord->block);
			levelShow(concreteRecord->block);
			if (concreteRecord->block->childrenBlock) {
				for (int i = 0; i < concreteRecord->block->childrenBlock->count(); ++i) {
					Block* block = concreteRecord->block->childrenBlock->at(i);
					pad->scene->removeItem(block);
					levelShow(block);
					if (block->inArrow) {
						pad->scene->removeItem(block->inArrow);
						levelShow(block->inArrow);
					}
				}
			}
		}
		else if (recordName == "ResetRoot") {
			ResetRoot* concreteRecord = (ResetRoot*)records->at(i);
			if (concreteRecord->pad) {
				concreteRecord->pad->setRoot(concreteRecord->newRoot);
			}
			else {
				concreteRecord->parentBlock->setChildRoot(concreteRecord->newRoot);
			}

		}
	}
	InDo(records);
}

void RecordList::Undo2Redu(QList<Record*>* records) {
	if (redoList->count() < MAX_REDO_STEP) {//应该是一定会满足的
		redoList->append(records);
	}
}

void RecordList::Undo() {
	if (undoList->empty()) return;
	QList<Record*>* records = undoList->last();
	undoList->removeLast();
	//处理undo事务
	for (int i = records->count() - 1; i >= 0; --i) {
		QString recordName = records->at(i)->className();
		if (recordName == "AddArrowLine") {
			AddArrowLine* concreteRecord = (AddArrowLine*)records->at(i);
			concreteRecord->arrowLine->fromBlock->outArrow = Q_NULLPTR;
			concreteRecord->arrowLine->toBlock->inArrow = Q_NULLPTR;
			pad->scene->removeItem(concreteRecord->arrowLine);
			levelShow(concreteRecord->arrowLine);
			//delete concreteRecord->arrowLine;//后面要修改，因为要Redo，所以此处不能删
		}
		else if (recordName == "AddBlock") {
			AddBlock* concreteRecord = (AddBlock*)records->at(i);
			pad->scene->removeItem(concreteRecord->block);
			concreteRecord->belongingList->removeOne(concreteRecord->block);
			levelShow(concreteRecord->block);
			if (concreteRecord->block->inArrow) {//应该不会执行
				concreteRecord->block->inArrow->fromBlock->outArrow = Q_NULLPTR;
				pad->scene->removeItem(concreteRecord->block->inArrow);
				levelShow(concreteRecord->block->inArrow);
				//delete concreteRecord->block->inArrow;//后面要修改，因为要Redo，所以此处不能删
			}
			if (concreteRecord->block->outArrow) {//应该不会执行
				concreteRecord->block->outArrow->toBlock->inArrow = Q_NULLPTR;
				pad->scene->removeItem(concreteRecord->block->outArrow);
				levelShow(concreteRecord->block->outArrow);
				//delete concreteRecord->block->outArrow;//后面要修改，因为要Redo，所以此处不能删
			}
			//delete concreteRecord->block;
		}
		else if (recordName == "MoveBlock") {
			MoveBlock* concreteRecord = (MoveBlock*)records->at(i);

		}
		else if (recordName == "RemoveArrowLine") {
			RemoveArrowLine* concreteRecord = (RemoveArrowLine*)records->at(i);
			if (concreteRecord->arrowLine->fromBlock)
				concreteRecord->arrowLine->fromBlock->outArrow = concreteRecord->arrowLine;
			if (concreteRecord->arrowLine->toBlock)
				concreteRecord->arrowLine->toBlock->inArrow = concreteRecord->arrowLine;
			pad->scene->addItem(concreteRecord->arrowLine);
			levelShow(concreteRecord->arrowLine);
		}
		else if (recordName == "RemoveBlock") {
			RemoveBlock* concreteRecord = (RemoveBlock*)records->at(i);
			concreteRecord->belongingList->push_back(concreteRecord->block);
			pad->scene->addItem(concreteRecord->block);
			concreteRecord->belongingList->append(concreteRecord->block);
			levelShow(concreteRecord->block);
			if (concreteRecord->block->childrenBlock) {
				for (int i = 0; i < concreteRecord->block->childrenBlock->count(); ++i) {
					Block* block = concreteRecord->block->childrenBlock->at(i);
					pad->scene->addItem(block);
					levelShow(block);
					if (block->inArrow) {
						pad->scene->addItem(block->inArrow);
						levelShow(block->inArrow);
					}
				}
			}
			
		}
		else if(recordName=="ResetRoot") {
			ResetRoot* concreteRecord = (ResetRoot*)records->at(i);
			if (concreteRecord->pad) {
				concreteRecord->pad->setRoot(concreteRecord->oldRoot);
			}
			else {
				concreteRecord->parentBlock->setChildRoot(concreteRecord->oldRoot);
			}
		}
	}
	Undo2Redu(records);
}

void RecordList::delayedHandleUndoRecord(Record* record) {
	QString recordName = record->className();
	if (recordName == "AddArrowLine") {
		AddArrowLine* concreteRecord = (AddArrowLine*)record;
		return;
	}
	else if (recordName == "AddBlock") {
		AddBlock* concreteRecord = (AddBlock*)record;
		return;
	}
	else if (recordName == "MoveBlock") {
		MoveBlock* concreteRecord = (MoveBlock*)record;
		return;
	}
	else if (recordName == "RemoveArrowLine") {
		RemoveArrowLine* concreteRecord = (RemoveArrowLine*)record;
		delete concreteRecord->arrowLine;
	}
	else if (recordName == "RemoveBlock") {
		RemoveBlock* concreteRecord = (RemoveBlock*)record;
		if (concreteRecord->block->childrenBlock) {
			for (int i = 0; i < concreteRecord->block->childrenBlock->count(); ++i) {
				Block* block = concreteRecord->block->childrenBlock->at(i);
				if (block->inArrow)
					delete block->inArrow;
				delete block;
			}
		}
	}
}

void RecordList::delayedHandleUndoRecord(QList<Record*>* records) {
	if (!records)return;
	for (int i = 0; i < records->count(); ++i)
		delayedHandleUndoRecord(records->at(i));
}

void RecordList::delayedHandleRedoRecord(Record* record) {
	QString recordName = record->className();
	if (recordName == "AddArrowLine") {
		AddArrowLine* concreteRecord = (AddArrowLine*)record;
		delete concreteRecord->arrowLine;
		return;
	}
	else if (recordName == "AddBlock") {
		AddBlock* concreteRecord = (AddBlock*)record;
		if (concreteRecord->block->childrenBlock) {
			for (int i = 0; i < concreteRecord->block->childrenBlock->count(); ++i) {
				Block* block = concreteRecord->block->childrenBlock->at(i);
				if (block->inArrow)
					delete block->inArrow;
				delete block;
			}
		}
		return;
	}
	else if (recordName == "MoveBlock") {
		MoveBlock* concreteRecord = (MoveBlock*)record;
		return;
	}
	else if (recordName == "RemoveArrowLine") {
		RemoveArrowLine* concreteRecord = (RemoveArrowLine*)record;
		return;
	}
	else if (recordName == "RemoveBlock") {
		RemoveBlock* concreteRecord = (RemoveBlock*)record;
		return;
	}
}

void RecordList::delayedHandleRedoRecord(QList<Record*>* records) {
	if (!records)return;
	for (int i = 0; i < records->count(); ++i)
		delayedHandleUndoRecord(records->at(i));
}
