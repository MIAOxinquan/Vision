#include <QtWidgets>
#include <QObject>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QScrollBar>
#include <qset.h>
#include <QStack>
#include <string>
class SmartEdit;
class ArrowLine;
class Item : public QGraphicsItem{
public:    virtual QString className() = 0;
};

class Block :public Item{
public:
    Block(int x, int y, QString str);
    //~Block();
    int x, y, w, h;
    QString head;
    QString content;
    ArrowLine* inArrow, * outArrow;
    QList<Block*>* childrenBlock;

    QString className()override;
    QRectF boundingRect() const override;
    void removeItemAllSons(Block* pItem);
protected:
    //键盘事件
    /*void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;*/
    //鼠标事件
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event)override;
    //void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    /*void mousePressEvent(QGraphicsSceneMouseEvent* event)override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event)override;*/
private:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
};

class ArrowLine : public Item{
    //Q_OBJECT
public:
    ArrowLine(Block* sourceItem, Block* destItem, QPointF, QPointF);
    //~ArrowLine();
    Block* fromBlock, * toBlock;

    QString className()override;
    void adjust();
    void deleteArrowLine(ArrowLine* pEdge);
protected:
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

private:
    QPointF sourcePoint, destPoint
        //偏移量,
        , m_pointStart, m_pointEnd;

    qreal arrowSize;
    bool m_pointFlag;
    static qreal min(qreal r1, qreal r2);
    static qreal abs(qreal r);
};

class PlotPad :public QGraphicsView
{
    Q_OBJECT
public:
    PlotPad(QGraphicsScene* scene);

    //鼠标释放时，绘制图像
    QGraphicsScene* scene;
    SmartEdit* edit;

    void drawItems(Block* it);
    void backLevel();
    void deleteItem();
    void myTest();
    QStack<QList<Block*>*> s;//s 顶部的QList里面存的应当是当前层显示出来的Items的列表
    QStack<std::string> path;
    std::string getPath();
    Block* getRoot();//返回s.top()列表中的root节点
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
    void mouseDoubleClickEvent(QMouseEvent* e)override;
    

    //绘图事件
    void paintEvent(QPaintEvent* e) override;
private:
    bool ctrlPressed, leftBtnPressed;
    QPoint startPoint, endPoint;
    QGraphicsItem* lastLine ;

public slots:
private slots:
};

