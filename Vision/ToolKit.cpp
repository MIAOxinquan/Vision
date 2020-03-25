#include "ToolKit.h"

ToolKit::ToolKit(QSplitter* parent)
	:QListWidget(parent)
{
	//QStringList strList{ tr("I4"), tr("I0"), tr("I2"), tr("I1") };
	//this->addItems(strList);

	initWidgetItems();
	this->setMouseTracking(true);
	//initSlots();
	connect(this, SIGNAL(itemEntered(QListWidgetItem*)), this, SLOT(handleItemEntered(QListWidgetItem*)));
}
/*
void ToolKit::initSlots()
{
	connect(this, SIGNAL(itemEntered(QListWidgetItem*)), this, SLOT(handleItemEntered(QListWidgetItem*)));
}*/

void ToolKit::initWidgetItems()
{
	int idx = 0;
	// if
	list.append(new QListWidgetItem(QString("if_item"), this));
	itemIdxMap[IF] = idx; idx++; //保存在list中的下标

	// if-else
	list.append(new QListWidgetItem(QString("if_else_item"), this));
	itemIdxMap[IF_ELSE] = idx; idx++;
	//

	for (int i = 0; i < list.size(); i++)
	{
		list.at(i)->setFlags(Qt::NoItemFlags);
		list.at(i)->setFlags(Qt::ItemIsEnabled);
		addItem(list.at(i));
	}
	//list[0]->setFlags(list[0]->flags() & ~Qt::ItemIsEnabled);
}

ToolKit::~ToolKit() {

}

void ToolKit::dragLeaveEvent(QDragLeaveEvent* e)
{
	qDebug("e");
}

void ToolKit::mousePressEvent(QMouseEvent* e)
{
	qDebug("Press(%d,%d)", e->x(), e->y());
	setCursor(Qt::CursorShape::CrossCursor);
	//setCursor(Qt::CursorShape::ForbiddenCursor);//设置鼠标样式
	down = true;
}

void ToolKit::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	//selected.
}

void ToolKit::mouseReleaseEvent(QMouseEvent* e)
{
	setCursor(Qt::CursorShape::ArrowCursor);//设置鼠标样式
	down = false;
}

void ToolKit::mouseMoveEvent(QMouseEvent* e)
{
	//this->selecte
	//if(down && )
	//if_item->
	//qDebug("Move(%d,%d)", e->x(), e->y());
}
//SLOTS
void ToolKit::handleItemEntered(QListWidgetItem* item)
{
	qDebug("handleItemEntered");
}

