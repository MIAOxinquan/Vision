#include "global.h"
#include "ToolKit.h"

ToolKit::ToolKit(QSplitter* parent)
	:QListWidget(parent)
	, toolPressed(Q_NULLPTR)
{
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
ToolKit::~ToolKit() {
	if (toolPressed) {
		delete toolPressed;
		toolPressed = Q_NULLPTR;
	}
}
/*��ɫת��*/
QColor ToolKit::getToolKeyColor(int index) {
	switch (index)
	{
	case 0:
		return Qt::darkGray; break;
	case 1:case 2:case 3:case 4:/*enum, union, struct, class*/
		return Qt::blue; break;
	case 12:
		return QColor(205, 133, 63); break;
	default:
		return QColor(160, 32, 240); break;
	}
}

void ToolKit::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		pressPoint = event->pos();//�ȱ�����ק�����.
		toolPressed = itemAt(event->pos());//��������ק����.
	}
	QListWidget::mousePressEvent(event);//����ԭQListWidget���������������.
}

void ToolKit::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton&& toolPressed)//ȷ����ס����ƶ�.
	{
		QPoint curPoint = event->pos() - pressPoint;
		//ֻ��������ȴ���Ĭ�ϵľ���,�Żᱻϵͳ��Ϊ���γ���ק�Ĳ���.
		if (curPoint.manhattanLength() >= QApplication::startDragDistance()) {
				QDrag* drag = new QDrag(this);
				drag->setPixmap(toolPressed->icon().pixmap(120, 60));//������קʱ��ͼ��

				QMimeData* mimeData = new QMimeData();//��ק��������ʱ����
				QString mimeText = toolPressed->text();
				mimeData->setText(mimeText);//����ı�Ϊ�գ�������Ĭ�Ͻ��ղ����κ���Ϣ����ʾΪ��ֹ��־
				drag->setMimeData(mimeData);
				auto action = drag->exec(Qt::CopyAction | Qt::MoveAction);
		}
	}
	QListWidget::mouseMoveEvent(event);
}