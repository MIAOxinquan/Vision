#include <QtCore>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
class SmartEdit;

class ArrowLine;
class Item :public QGraphicsItem {
public:    virtual QString className() = 0;
};
class Block :public Item {
public:
    Block(int x, int y, QString type);
    //~Block();
    int x, y;
    QString type;
    QString content;
    Block* nextBlock;
    ArrowLine* outArrow, * inArrow;
    QString className()override;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
protected:
    //键盘事件
    /*void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;*/
    //鼠标事件
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event)override;
    //void mousePressEvent(QGraphicsSceneMouseEvent* event)override;
    /*void mouseReleaseEvent(QGraphicsSceneMouseEvent* event)override;*/
private:
    bool ctrlPressed = false;
    void drawToItem(QPainter* painter);

};

class ArrowLine : public Item,public QObject {
    //Q_OBJECT
public:
    ArrowLine(Block* sourceItem, Block* destItem, QPointF, QPointF);
    //~ArrowLine();
    void adjust();
    QString className()override;
    Block* getSrc();
    Block* getDest();
protected:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    //void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
private:
    Block* source, * dest;
    QPointF sourcePoint;
    QPointF destPoint;
    qreal arrowSize;
    bool m_pointFlag;

    QPointF m_pointStart;//偏移量
    QPointF m_pointEnd;

};

class PlotPad :public QGraphicsView
{
	Q_OBJECT
public:
    PlotPad(QGraphicsScene* scene);//此处注意QGraphicsView的scene参数是子对象，第二个参数是父对象，如果只有QGraphicsScene参数，则父对象为空
    //~PlotPad();
    //鼠标释放时，绘制图像
    QGraphicsScene* scene;
    SmartEdit* edit;
    void drawItems(Block* it);

protected:
	void dropEvent(QDropEvent* event)override;
	void dragEnterEvent(QDragEnterEvent* event)override;
	void dragMoveEvent(QDragMoveEvent* event)override;
    //键盘事件
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    //鼠标事件
    void mouseMoveEvent(QMouseEvent* e)override;
    void mousePressEvent(QMouseEvent* e)override;
    void mouseReleaseEvent(QMouseEvent* e)override;
    //绘图事件
    void paintEvent(QPaintEvent* e) override;

private:
    bool ctrlPressed = false;
    bool m_leftBtnPressed = false;
    QPoint startPoint, endPoint;
    QGraphicsItem* lastLine ;

public slots:
private slots:
};

