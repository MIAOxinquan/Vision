#pragma once
#include <QtCore>

class Block;
class ArrowLine;
class PlotPad;

class RecordObject
{
public:
	virtual QString className() = 0;
};

class AddArrowLine:public RecordObject
{
public:
	virtual QString className();
	ArrowLine* arrowLine;
	AddArrowLine(ArrowLine* arrowLine);
};


class AddBlock:public RecordObject
{
public:
	virtual QString className();
	Block* block;
	QList<Block*>* belongingList;
	AddBlock(Block* block, QList<Block*>* belongingList);
};

class MoveBlock :public RecordObject
{
public:
	virtual QString className();
	QPointF fromPoint, toPoint;
	Block* block;
	MoveBlock(Block* block);// fromPoint和toPoint怎么处理
};

class DeleteArrowLine :public RecordObject
{
public:
	virtual QString className();
	ArrowLine* arrowLine;
	DeleteArrowLine(ArrowLine* arrowLine);
};

class DeleteBlock :public RecordObject
{
public:
	virtual QString className();
	Block* block;
	QList<Block*>* belongingList;
	DeleteBlock(Block* block, QList<Block*>* belongingList);
};


class UndoRedoStack
{
private:
	const int  MAX_REDO_STEP = 30;//最大重做数量
	PlotPad* pad;
	QList<QList<RecordObject*>*>* undoList;
	QList<QList<RecordObject*>*>* redoList;

	void delayedHandleUndoRecord(RecordObject* record);
	void delayedHandleUndoRecord(QList<RecordObject*>* records);
	void delayedHandleRedoRecord(RecordObject* record);
	void delayedHandleRedoRecord(QList<RecordObject*>* records);

	void Undo2Redu(QList<RecordObject*>* records);
	void clearReduList();

	void InDo(RecordObject* record);
	void InDo(QList<RecordObject*>* records);

public:
	void Redo();
	void Undo();
	void Do(RecordObject* record);
	void Do(QList<RecordObject*>* records);
	
	UndoRedoStack(PlotPad* pad);
	virtual ~UndoRedoStack();
};
