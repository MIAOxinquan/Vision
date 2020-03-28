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
/*初始化*/
void ToolKit::init() {
	//布局设置
	setDragEnabled(true);
	setFont(QFont("微软雅黑", 12, QFont::Bold));
	setIconSize(QSize(50, 25));
	//变量初始化
	for (int i = 0; i < toolKeys.count(); i++) {
		//toolPressed用于获取鼠标选中的tool，此处初始化时临时借用以减少变量声明，初始化结束后null化即可，
		toolPressed = new QListWidgetItem(toolKeys.at(i), this);
		toolPressed->setIcon(QPixmap(":/Resources/icon/" + toolKeys.at(i) + ".png"));
		toolPressed->setForeground(getToolKeyColor(i));
		toolPressed->setFlags(Qt::ItemIsEnabled);
		addItem(toolPressed);
	}
	toolPressed = Q_NULLPTR;
	//加载qss
	loadStyleSheet(this, "tools.qss");
}
/*词色转换*/
QColor ToolKit::getToolKeyColor(int index) {
	switch (index)
	{
	case 0:case 1:
		return Qt::blue; break;
	case 8:
		return QColor(205, 133, 63); break;
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
		pressPoint = event->pos();//先保存拖拽的起点.
		toolPressed = itemAt(event->pos());//保留被拖拽的项.
		indexOfToolPressed = toolKeys.indexOf(toolPressed->text());
	}
	QListWidget::mousePressEvent(event);//保留原QListWidget部件的鼠标点击操作.
}

/*
void ToolKit::mouseReleaseEvent(QMouseEvent* event)
{
	qDebug() << "mouseRelease";
}*/

void ToolKit::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton)//确保按住左键移动.
	{
		QPoint curPoint = event->pos() - pressPoint;
		//只有这个长度大于默认的距离,才会被系统认为是形成拖拽的操作.
		if (curPoint.manhattanLength() >= QApplication::startDragDistance()) {
			QDrag* drag = new QDrag(this);
			drag->setPixmap(toolPressed->icon().pixmap(60,30));//设置拖拽时的图标

			QMimeData* mimeData = new QMimeData();//拖拽产生的临时数据
			mimeData->setText(QString::number(indexOfToolPressed));//如果文本为空，则代码框默认接收不到任何信息，显示为禁止标志
			drag->setMimeData(mimeData);
			auto action = drag->exec(Qt::CopyAction | Qt::MoveAction);
		}
	}
	QListWidget::mouseMoveEvent(event);
}