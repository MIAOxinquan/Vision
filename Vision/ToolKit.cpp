#include "global.h"
#include "ToolKit.h"

ToolKit::ToolKit(QSplitter* parent)
	:QListWidget(parent)
	, toolPressed(Q_NULLPTR)
{
	init();
}
ToolKit::~ToolKit() {
	delete toolPressed; toolPressed = Q_NULLPTR;
}
/*��ʼ��*/
void ToolKit::init() {
	//��������
	setDragEnabled(true);
	setFont(QFont("΢���ź�", 12, QFont::Bold));
	setIconSize(QSize(50, 25));
	//������ʼ��
	for (int i = 0; i < toolKeys.count(); i++) {
		//toolPressed���ڻ�ȡ���ѡ�е�tool���˴���ʼ��ʱ��ʱ�����Լ��ٱ�����������ʼ��������null�����ɣ�
		toolPressed = new QListWidgetItem(toolKeys.at(i), this);
		toolPressed->setIcon(QPixmap(":/Resources/icon/" + toolKeys.at(i) + ".png"));
		toolPressed->setForeground(getToolKeyColor(i));
		toolPressed->setFlags(Qt::ItemIsEnabled);
		addItem(toolPressed);
	}
	toolPressed = Q_NULLPTR;
	//����qss
	loadStyleSheet(this, "tools.qss");
}
/*��ɫת��*/
QColor ToolKit::getToolKeyColor(int index) {
	switch (index)
	{
	case 0:case 1:
		return Qt::blue; break;
	case 8:
		return Qt::darkYellow; break;
	default:
		return QColor(160, 32, 240); break;
	}
	return Qt::black;
}
/*
void ToolKit::dragLeaveEvent(QDragLeaveEvent* event)
{
	qDebug() << "dragLeave";
}*/

void ToolKit::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		pressPoint = event->pos();//�ȱ�����ק�����.
		toolPressed = itemAt(event->pos());//��������ק����.
		indexOfToolPressed = toolKeys.indexOf(toolPressed->text());
	}
	QListWidget::mousePressEvent(event);//����ԭQListWidget���������������.
}

/*
void ToolKit::mouseReleaseEvent(QMouseEvent* event)
{
	qDebug() << "mouseRelease";
}*/

void ToolKit::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton)//ȷ����ס����ƶ�.
	{
		QPoint curPoint = event->pos() - pressPoint;
		//ֻ��������ȴ���Ĭ�ϵľ���,�Żᱻϵͳ��Ϊ���γ���ק�Ĳ���.
		if (curPoint.manhattanLength() >= QApplication::startDragDistance()) {
			QDrag* drag = new QDrag(this);
			drag->setPixmap(toolPressed->icon().pixmap(60,30));//������קʱ��ͼ��

			QMimeData* mimeData = new QMimeData();//��ק��������ʱ����
			mimeData->setText(QString::number(indexOfToolPressed));//����ı�Ϊ�գ�������Ĭ�Ͻ��ղ����κ���Ϣ����ʾΪ��ֹ��־
			drag->setMimeData(mimeData);
			auto action = drag->exec(Qt::CopyAction | Qt::MoveAction);
		}
	}
	QListWidget::mouseMoveEvent(event);
}