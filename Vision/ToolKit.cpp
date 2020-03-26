#include "stdafx.h"
#include "ToolKit.h"
#include <qcursor.h>
#include "Vision.h"


ToolKit::ToolKit(QSplitter* parent)
	:QListWidget(parent)
{
	//QStringList strList{ tr("I4"), tr("I0"), tr("I2"), tr("I1") };
	//this->addItems(strList);

	initWidgetItems();
	//this->setMouseTracking(true);
	setDragEnabled(true);
	setFont(QFont("微软雅黑", 12, QFont::Bold));
	//加载qss
	QFile file("./Resources/qss/list.qss");
	file.open(QFile::ReadOnly);
	setStyleSheet(file.readAll());
	file.close();
	//initSlots();

}

void ToolKit::initSlots()
{
	connect(this, SIGNAL(itemEntered(QListWidgetItem*)), this, SLOT(handleItemEntered(QListWidgetItem*)));
	//connect(this, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(handleItemClicked(QListWidgetItem*)));
}


void ToolKit::initWidgetItems()
{
	int idx = 0;
	QListWidgetItem* item;
	QString picPath;
	// if
	picPath = QString(":/Vision/Resources/icon/IF_icon.jpg");//添加拖拽图片映射
	item = new QListWidgetItem(QString("if"), this);
	list.append(item);
	itemIdxMap[IF] = idx; idx++; //保存在list中的下标
	itemIconMap[item] = picPath;
	item->setIcon(QIcon(picPath));

	// if-else
	picPath = QString(":/Vision/Resources/icon/IF_ELSE_icon.png");
	item = new QListWidgetItem(QString("if-else"), this);
	list.append(item);
	itemIdxMap[IF_ELSE] = idx; idx++;
	itemIconMap[item] = picPath;
	item->setIcon(QIcon(picPath));

	// class
	picPath = QString(":/Vision/Resources/icon/CLASS_icon.jpg");
	item = new QListWidgetItem(QString("class"), this);
	list.append(item);
	itemIdxMap[CLASS] = idx; idx++;
	itemIconMap[item] = picPath;
	item->setIcon(QIcon(picPath));

	// func
	picPath = QString(":/Vision/Resources/icon/FUNC_icon.png");
	item = new QListWidgetItem(QString("func"), this);
	list.append(item);
	itemIdxMap[FUNC] = idx; idx++;
	itemIconMap[item] = picPath;
	item->setIcon(QIcon(picPath));

	// for
	picPath = QString(":/Vision/Resources/icon/FOR_icon.png");
	item = new QListWidgetItem(QString("for"), this);
	list.append(item);
	itemIdxMap[FOR] = idx; idx++;
	itemIconMap[item] = picPath;
	item->setIcon(QIcon(picPath));

	// while
	picPath = QString(":/Vision/Resources/icon/WHILE_icon.png");
	item = new QListWidgetItem(QString("while"), this);
	list.append(item);
	itemIdxMap[WHILE] = idx; idx++;
	itemIconMap[item] = picPath;
	item->setIcon(QIcon(picPath));

	// do-while
	picPath = QString(":/Vision/Resources/icon/DO_WHILE_icon.png");
	item = new QListWidgetItem(QString("do-while"), this);
	list.append(item);
	itemIdxMap[DO_WHILE] = idx; idx++;
	itemIconMap[item] = picPath;
	item->setIcon(QIcon(picPath));

	for (int i = 0; i < list.size(); i++)
	{
		list.at(i)->setFlags(Qt::NoItemFlags);
		list.at(i)->setFlags(Qt::ItemIsEnabled);
		addItem(list.at(i));
	}

}

ToolKit::~ToolKit() {

}

void ToolKit::dragLeaveEvent(QDragLeaveEvent* e)
{
	qDebug("dragLeaveEvent");
}

void ToolKit::mousePressEvent(QMouseEvent* e)
{
	/*QPoint p = e->globalPos();
	qDebug("Press G(%d,%d)", p.x(), p.y());
	qDebug("Press(%d,%d)", e->x(), e->y());*/
	//setCursor(Qt::CursorShape::CrossCursor);
	//setCursor(Qt::CursorShape::ForbiddenCursor);//设置鼠标样式

	/////////////////////////

	//确保左键拖拽.
	if (e->button() == Qt::LeftButton)
	{
		//先保存拖拽的起点.
		m_dragPoint = e->pos();
		//保留被拖拽的项.
		m_dragItem = this->itemAt(e->pos());
	}
	//保留原QListWidget部件的鼠标点击操作.
	QListWidget::mousePressEvent(e);
	down = true;
}


void ToolKit::mouseReleaseEvent(QMouseEvent* e)
{
	setCursor(Qt::CursorShape::ArrowCursor);//设置鼠标样式
	down = false;
}

void ToolKit::mouseMoveEvent(QMouseEvent* e)
{

	//确保按住左键移动.
	if (e->buttons() & Qt::LeftButton)
	{
		QPoint temp = e->pos() - m_dragPoint;
		//只有这个长度大于默认的距离,才会被系统认为是形成拖拽的操作.
		if (temp.manhattanLength() > QApplication::startDragDistance())
		{
			QDrag* drag = new QDrag(this);


			QIcon qI = QIcon(itemIconMap[m_dragItem]);

			QPixmap pixmap = qI.pixmap(QSize(24, 24));
			drag->setPixmap(pixmap);//设置拖拽时的图标


			QMimeData* mimeData = new QMimeData;

			mimeData->setText(m_dragItem->text());
			drag->setMimeData(mimeData);
			auto action = drag->exec(Qt::CopyAction | Qt::MoveAction);
		}
	}
	QListWidget::mouseMoveEvent(e);
}


Vision* ToolKit::getVision()
{
	QObject* Splitter = this->parent();
	QObject* vision = (QSplitter*)Splitter->parent();
	return (Vision*)vision;
}


//SLOTS
void ToolKit::handleItemEntered(QListWidgetItem* item)
{
	qDebug("handleItemEntered");
}

void ToolKit::handleItemClicked(QListWidgetItem* item)
{
	qDebug("handleItemClicked");
}