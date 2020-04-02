#include <QtWidgets>

class PlotItem :public QGraphicsTextItem {
public:
	PlotItem(QGraphicsScene*parent=Q_NULLPTR);
	/*
	PlotItem(PlotItem* parent);*/
	~PlotItem();

	QString type, info;
	QPoint center;
	int width, height;
protected:/*
	void paint();
	QRectF boundingRect();*/
private:

};
class PlotPad :public QWidget
{
	Q_OBJECT
public:
	PlotPad(QTabWidget* parent = Q_NULLPTR);
	~PlotPad();
	QGraphicsView* plotView;
	QGraphicsScene* plotScene;
	//PlotItem* item;
protected:
	void dropEvent(QDropEvent* event)override;
	void dragEnterEvent(QDragEnterEvent* event)override;
	void dragMoveEvent(QDragMoveEvent* event)override;

private:
public slots:
private slots:
};
