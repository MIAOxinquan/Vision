#include <QtWidgets>


class PlotPad :public QWidget
{
	Q_OBJECT
public:
	PlotPad(QTabWidget* parent = Q_NULLPTR);
	~PlotPad();
protected:
	void dropEvent(QDropEvent* event)override;
	void dragEnterEvent(QDragEnterEvent* event)override;
	void dragMoveEvent(QDragMoveEvent* event)override;

private:

public slots:
private slots:
};

