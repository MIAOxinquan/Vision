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
	setFont(QFont("΢���ź�", 12, QFont::Bold));
	//����qss
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
	picPath = QString(":/Vision/Resources/icon/IF_icon.jpg");//�����קͼƬӳ��
	item = new QListWidgetItem(QString("if"), this);
	list.append(item);
	itemIdxMap[IF] = idx; idx++; //������list�е��±�
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
	//setCursor(Qt::CursorShape::ForbiddenCursor);//���������ʽ

	/////////////////////////

	//ȷ�������ק.
	if (e->button() == Qt::LeftButton)
	{
		//�ȱ�����ק�����.
		m_dragPoint = e->pos();
		//��������ק����.
		m_dragItem = this->itemAt(e->pos());
	}
	//����ԭQListWidget���������������.
	QListWidget::mousePressEvent(e);
	down = true;
}


void ToolKit::mouseReleaseEvent(QMouseEvent* e)
{
	setCursor(Qt::CursorShape::ArrowCursor);//���������ʽ
	down = false;
}

void ToolKit::mouseMoveEvent(QMouseEvent* e)
{

	//ȷ����ס����ƶ�.
	if (e->buttons() & Qt::LeftButton)
	{
		QPoint temp = e->pos() - m_dragPoint;
		//ֻ��������ȴ���Ĭ�ϵľ���,�Żᱻϵͳ��Ϊ���γ���ק�Ĳ���.
		if (temp.manhattanLength() > QApplication::startDragDistance())
		{
			QDrag* drag = new QDrag(this);


			QIcon qI = QIcon(itemIconMap[m_dragItem]);

			QPixmap pixmap = qI.pixmap(QSize(24, 24));
			drag->setPixmap(pixmap);//������קʱ��ͼ��


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