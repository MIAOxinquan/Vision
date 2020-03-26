#include <QtWidgets>
#include <map>
class Vision;

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
	//void setVision(Vision* vision);
protected:
	void dragLeaveEvent(QDragLeaveEvent* e)override;
	void mousePressEvent(QMouseEvent* e)override;
	void mouseReleaseEvent(QMouseEvent* e)override;
	void mouseMoveEvent(QMouseEvent* e)override;

private://var
	QList<QListWidgetItem*> list;
	QListWidgetItem* if_item = NULL;
	QListWidgetItem* if_else_item = NULL;
	QListWidgetItem* selected = NULL;
	Vision* vision;
	bool down = false;
	QPoint m_dragPoint;
	QListWidgetItem* m_dragItem;
	std::map<int, int> itemIdxMap;
	std::map<QListWidgetItem*, QString> itemIconMap;
private://func
	void initWidgetItems();
	void initSlots();
	Vision* getVision();

public slots:
	void handleItemEntered(QListWidgetItem* item);
	void handleItemClicked(QListWidgetItem* item);
private slots:

};

