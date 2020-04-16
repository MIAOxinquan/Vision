#include <QtWidgets>
#include <QtGui>

class ToolKit :public QListWidget
{
	Q_OBJECT
public:
	ToolKit(QSplitter* parent = Q_NULLPTR);
	~ToolKit();
	QColor getToolKeyColor(int index);

protected:
	//void dragLeaveEvent(QDragLeaveEvent* event)override;
	void mousePressEvent(QMouseEvent* event)override;
	//void mouseReleaseEvent(QMouseEvent* e)override;
	void mouseMoveEvent(QMouseEvent* e)override;

private:
	QPoint pressPoint;
	QListWidgetItem* toolPressed;
};

