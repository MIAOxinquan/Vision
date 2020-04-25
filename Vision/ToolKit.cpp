#include "global.h"
#include "ToolKit.h"

ToolKit::ToolKit(QSplitter* parent)
	:QListWidget(parent)
	, toolPressed(Q_NULLPTR)
{
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
ToolKit::~ToolKit() {
	if (toolPressed) {
		delete toolPressed;
		toolPressed = Q_NULLPTR;
	}
}
/*词色转换*/
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
		pressPoint = event->pos();//先保存拖拽的起点.
		toolPressed = itemAt(event->pos());//保留被拖拽的项.
	}
	QListWidget::mousePressEvent(event);//保留原QListWidget部件的鼠标点击操作.
}

void ToolKit::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton&& toolPressed)//确保按住左键移动.
	{
		QPoint curPoint = event->pos() - pressPoint;
		//只有这个长度大于默认的距离,才会被系统认为是形成拖拽的操作.
		if (curPoint.manhattanLength() >= QApplication::startDragDistance()) {
				QDrag* drag = new QDrag(this);
				drag->setPixmap(toolPressed->icon().pixmap(120, 60));//设置拖拽时的图标

				QMimeData* mimeData = new QMimeData();//拖拽产生的临时数据
				QString mimeText = toolPressed->text();
				if ("empty" == mimeText)mimeText = "";
				mimeData->setText(mimeText);//如果文本为空，则代码框默认接收不到任何信息，显示为禁止标志
				drag->setMimeData(mimeData);
				auto action = drag->exec(Qt::CopyAction | Qt::MoveAction);
		}
	}
	QListWidget::mouseMoveEvent(event);
}