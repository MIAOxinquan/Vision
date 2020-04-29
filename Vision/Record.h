#pragma once
#include <QtCore>

class Item;
class Block;
class ArrowLine;
class PlotPad;

class Record
{
public:
	virtual QString className() = 0;
};

class AddArrowLine:public Record
{
public:
	virtual QString className();
	ArrowLine* arrowLine;
	AddArrowLine(ArrowLine* arrowLine);
};


class AddBlock:public Record
{
public:
	virtual QString className();
	Block* block;
	QList<Block*>* belongingList;
	AddBlock(Block* block, QList<Block*>* belongingList);
};

class MoveBlock :public Record
{
public:
	virtual QString className();
	QPointF fromPoint, toPoint;
	Block* block;
	MoveBlock(Block* block);// fromPoint和toPoint怎么处理
};

class RemoveArrowLine :public Record
{
public:
	virtual QString className();
	ArrowLine* arrowLine;
	RemoveArrowLine(ArrowLine* arrowLine);
};

class RemoveBlock :public Record
{
public:
	virtual QString className();
	Block* block;
	QList<Block*>* belongingList;
	RemoveBlock(Block* block, QList<Block*>* belongingList);
};

class ResetRoot :public Record 
{
public:
	virtual QString className();
	Block* parentBlock, * oldRoot, * newRoot;
	PlotPad* pad;
	ResetRoot(PlotPad* pad, Block* parentBlock, Block* oldRoot, Block* newRoot);
};

class RecordList
{
public:
	QList<QList<Record*>*>* undoList;
	QList<QList<Record*>*>* redoList;
private:
	const int  MAX_REDO_STEP = 30;//最大重做数量
	PlotPad* pad;

	void delayedHandleUndoRecord(Record* record);
	void delayedHandleUndoRecord(QList<Record*>* records);
	void delayedHandleRedoRecord(Record* record);
	void delayedHandleRedoRecord(QList<Record*>* records);

	void Undo2Redu(QList<Record*>* records);
	void clearRedoList();
	void levelShow(Item* item);
	void InDo(Record* record);
	void InDo(QList<Record*>* records);

public:
	void Redo();
	void Undo();
	void Do(Record* record);
	void Do(QList<Record*>* records);
	
	RecordList(PlotPad* pad);
	virtual ~RecordList();
};
