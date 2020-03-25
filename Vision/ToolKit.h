#include <QtWidgets>
#include <map>

class ToolKit :public QListWidget
{
	Q_OBJECT
public:
	enum TOOL_TYPE {
		IF = 0,
		IF_ELSE,
		FOR,
		WHILE,
		DO_WHILE,
		FUNC,
		CLASS
	};
	ToolKit(QSplitter* parent = Q_NULLPTR);
	~ToolKit();
protected:
	void dragLeaveEvent(QDragLeaveEvent* e)override;
	void mousePressEvent(QMouseEvent* e)override;
	void mouseReleaseEvent(QMouseEvent* e)override;
	void mouseMoveEvent(QMouseEvent* e)override;
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)override;

private://var
	QList<QListWidgetItem*> list;
	QListWidgetItem* if_item = nullptr;
	QListWidgetItem* if_else_item = nullptr;
	QListWidgetItem* selected = nullptr;
	bool down = false;

	std::map<int, int> itemIdxMap;
private://func
	void initWidgetItems();
	//void initSlots();

public slots:
	void handleItemEntered(QListWidgetItem* item);
private slots:

};


