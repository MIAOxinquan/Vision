#include "RecordObject.h"
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
MoveBlock::MoveBlock(Block* block) {
	this->block = block;
	this->fromPoint = block->pos();
}

/*DeleteArrowLine*/
QString DeleteArrowLine::className() { return "DeleteArrowLine"; }
DeleteArrowLine::DeleteArrowLine(ArrowLine* arrowLine) { this->arrowLine = arrowLine; }

/*DeleteBlock*/
QString DeleteBlock::className() { return "DeleteBlock"; }
DeleteBlock::DeleteBlock(Block* block, QList<Block*>* belongingList) {
	this->block = block;
	this->belongingList = belongingList;
}

/*UndoRedoStack*/
UndoRedoStack::UndoRedoStack(PlotPad* pad)
	: undoList(new QList<QList<RecordObject*>*>())
	, redoList(new QList<QList<RecordObject*>*>())
{
	this->pad = pad;
}

UndoRedoStack::~UndoRedoStack()
{

}

void UndoRedoStack::InDo(RecordObject* record)
{
	if (undoList->size() == MAX_REDO_STEP)//如果已经满了，则删除第一个并处理
	{
		QList<RecordObject*>* handleRecords = undoList->front();
		redoList->pop_front();
		delayedHandleUndoRecord(handleRecords);
	}
	QList<RecordObject*>* records = new QList<RecordObject*>();
	records->push_back(record);
	undoList->push_back(records);
}

void UndoRedoStack::InDo(QList<RecordObject*>* records)
{
	if (undoList->size() == MAX_REDO_STEP)//如果已经满了，则删除第一个并处理
	{
		QList<RecordObject*>* handleRecords = undoList->front();
		redoList->pop_front();
		delayedHandleUndoRecord(handleRecords);
	}
	undoList->push_back(records);
}

void UndoRedoStack::Do(RecordObject* record)
{
	InDo(record);
	//清空redoList
	clearReduList();
}

void UndoRedoStack::Do(QList<RecordObject*>* records)
{
	InDo(records);
	//清空redoList
	clearReduList();
}

void UndoRedoStack::clearReduList() {
	while (!redoList->empty()) {
		QList<RecordObject*>* records = redoList->back(); redoList->pop_back();
		while (!records->empty()) {
			RecordObject* record = records->back(); records->pop_back();
			delayedHandleRedoRecord(record);
			delete record;
		}
		delete records;
	}
}

void UndoRedoStack::Redo() {
	if (redoList->empty()) return;
	QList<RecordObject*>* records = redoList->back(); redoList->pop_back();
	for (int i = records->size() - 1; i >= 0; --i) {
		QString recordName = records->at(i)->className();
		if (recordName == "AddArrowLine") {
			AddArrowLine* concreteRecord = (AddArrowLine*)records->at(i);
			concreteRecord->arrowLine->fromBlock->outArrow = concreteRecord->arrowLine;
			concreteRecord->arrowLine->toBlock->inArrow = concreteRecord->arrowLine;
			pad->scene->addItem(concreteRecord->arrowLine);
		}
		else if (recordName == "AddBlock") {
			AddBlock* concreteRecord = (AddBlock*)records->at(i);
			pad->scene->addItem(concreteRecord->block);
			if (concreteRecord->block->inArrow)//应该不会执行
			{
				concreteRecord->block->inArrow->fromBlock->outArrow = concreteRecord->block->inArrow;
				pad->scene->addItem(concreteRecord->block->inArrow);
				//delete concreteRecord->block->inArrow;//后面要修改，因为要Redo，所以此处不能删
			}
			if (concreteRecord->block->outArrow)//应该不会执行
			{
				concreteRecord->block->outArrow->toBlock->inArrow = concreteRecord->block->outArrow;
				pad->scene->addItem(concreteRecord->block->outArrow);
				//delete concreteRecord->block->outArrow;//后面要修改，因为要Redo，所以此处不能删
			}
		}
		else if (recordName == "MoveBlock") {
			MoveBlock* concreteRecord = (MoveBlock*)records->at(i);

		}
		else if (recordName == "DeleteArrowLine") {
			DeleteArrowLine* concreteRecord = (DeleteArrowLine*)records->at(i);
			if (concreteRecord->arrowLine->fromBlock)
				concreteRecord->arrowLine->fromBlock->outArrow = NULL;
			if (concreteRecord->arrowLine->toBlock)
				concreteRecord->arrowLine->toBlock->inArrow = NULL;
			pad->scene->removeItem(concreteRecord->arrowLine);
		}
		else if (recordName == "DeleteBlock") {
			DeleteBlock* concreteRecord = (DeleteBlock*)records->at(i);
			if (concreteRecord->belongingList && concreteRecord->belongingList->count(concreteRecord->block) == 1)
				concreteRecord->belongingList->removeOne(concreteRecord->block);

			pad->scene->removeItem(concreteRecord->block);//不需要考虑箭头，箭头在别处考虑了
			if (concreteRecord->block->childrenBlock) {
				for (int i = 0; i < concreteRecord->block->childrenBlock->size(); ++i) {
					Block* block = concreteRecord->block->childrenBlock->at(i);
					pad->scene->removeItem(block);
					if (block->inArrow) {
						pad->scene->removeItem(block->inArrow);
					}
				}
			}
		}
	}
	InDo(records);
}

void UndoRedoStack::Undo2Redu(QList<RecordObject*>* records) {
	if (redoList->size() < MAX_REDO_STEP) {//应该是一定会满足的
		redoList->push_back(records);
	}
}

void UndoRedoStack::Undo() {
	if (undoList->empty()) return;
	QList<RecordObject*>* records = undoList->back();
	undoList->pop_back();
	//处理undo事务
	for (int i = records->size() - 1; i >= 0; --i) {
		QString recordName = records->at(i)->className();
		if (recordName == "AddArrowLine") {
			AddArrowLine* concreteRecord = (AddArrowLine*)records->at(i);
			concreteRecord->arrowLine->fromBlock->outArrow = NULL;
			concreteRecord->arrowLine->toBlock->inArrow = NULL;
			pad->scene->removeItem(concreteRecord->arrowLine);
			//delete concreteRecord->arrowLine;//后面要修改，因为要Redo，所以此处不能删
		}
		else if (recordName == "AddBlock") {
			AddBlock* concreteRecord = (AddBlock*)records->at(i);
			pad->scene->removeItem(concreteRecord->block);
			if (concreteRecord->block->inArrow) {//应该不会执行
				concreteRecord->block->inArrow->fromBlock->outArrow = NULL;
				pad->scene->removeItem(concreteRecord->block->inArrow);
				//delete concreteRecord->block->inArrow;//后面要修改，因为要Redo，所以此处不能删
			}
			if (concreteRecord->block->outArrow) {//应该不会执行
				concreteRecord->block->outArrow->toBlock->inArrow = NULL;
				pad->scene->removeItem(concreteRecord->block->outArrow);
				//delete concreteRecord->block->outArrow;//后面要修改，因为要Redo，所以此处不能删
			}
			//delete concreteRecord->block;
		}
		else if (recordName == "MoveBlock") {
			MoveBlock* concreteRecord = (MoveBlock*)records->at(i);

		}
		else if (recordName == "DeleteArrowLine") {
			DeleteArrowLine* concreteRecord = (DeleteArrowLine*)records->at(i);
			if (concreteRecord->arrowLine->fromBlock)
				concreteRecord->arrowLine->fromBlock->outArrow = concreteRecord->arrowLine;
			if (concreteRecord->arrowLine->toBlock)
				concreteRecord->arrowLine->toBlock->inArrow = concreteRecord->arrowLine;
			pad->scene->addItem(concreteRecord->arrowLine);
		}
		else if (recordName == "DeleteBlock") {
			DeleteBlock* concreteRecord = (DeleteBlock*)records->at(i);
			concreteRecord->belongingList->push_back(concreteRecord->block);
			pad->scene->addItem(concreteRecord->block);
			if (concreteRecord->block->childrenBlock) {
				for (int i = 0; i < concreteRecord->block->childrenBlock->size(); ++i) {
					Block* block = concreteRecord->block->childrenBlock->at(i);
					pad->scene->addItem(block);
					if (block->inArrow)
						pad->scene->addItem(block->inArrow);
				}
			}
		}
	}
	Undo2Redu(records);
}

void UndoRedoStack::delayedHandleUndoRecord(RecordObject* record) {
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
	else if (recordName == "DeleteArrowLine") {
		DeleteArrowLine* concreteRecord = (DeleteArrowLine*)record;
		delete concreteRecord->arrowLine;
	}
	else if (recordName == "DeleteBlock") {
		DeleteBlock* concreteRecord = (DeleteBlock*)record;
		if (concreteRecord->block->childrenBlock) {
			for (int i = 0; i < concreteRecord->block->childrenBlock->size(); ++i) {
				Block* block = concreteRecord->block->childrenBlock->at(i);
				if (block->inArrow)
					delete block->inArrow;
				delete block;
			}
		}
	}
}

void UndoRedoStack::delayedHandleUndoRecord(QList<RecordObject*>* records) {
	if (!records)return;
	for (int i = 0; i < records->size(); ++i)
		delayedHandleUndoRecord(records->at(i));
}

void UndoRedoStack::delayedHandleRedoRecord(RecordObject* record) {
	QString recordName = record->className();
	if (recordName == "AddArrowLine") {
		AddArrowLine* concreteRecord = (AddArrowLine*)record;
		delete concreteRecord->arrowLine;
		return;
	}
	else if (recordName == "AddBlock") {
		AddBlock* concreteRecord = (AddBlock*)record;
		if (concreteRecord->block->childrenBlock) {
			for (int i = 0; i < concreteRecord->block->childrenBlock->size(); ++i) {
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
	else if (recordName == "DeleteArrowLine") {
		DeleteArrowLine* concreteRecord = (DeleteArrowLine*)record;
		return;
	}
	else if (recordName == "DeleteBlock") {
		DeleteBlock* concreteRecord = (DeleteBlock*)record;
		return;
	}
}

void UndoRedoStack::delayedHandleRedoRecord(QList<RecordObject*>* records) {
	if (!records)return;
	for (int i = 0; i < records->size(); ++i)
		delayedHandleUndoRecord(records->at(i));
}
