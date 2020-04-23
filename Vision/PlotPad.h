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
    int x, y;
    QString head;
    QString content;
    Block(int x, int y, QString str);
    QRectF boundingRect() const override;
    //QRectF sceneBoundingRec() const override;
    Block* toItem = NULL;
    ArrowLine* toEdge = NULL;
    ArrowLine* fromEdge = NULL;
    QString className()override;

    int getWidth();
    int getHeight();
    QList<Block*>* sons = NULL;
    //QSet<PlotEdge*> edges;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void removeItemAllSons(Block* pItem);
protected:
    //�����¼�
    /*void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;*/
    //����¼�
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event)override;
    //void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    /*void mousePressEvent(QGraphicsSceneMouseEvent* event)override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event)override;*/
private:
    void drawToItem(QPainter* painter);

};

class ArrowLine : public Item, public QObject{
    //Q_OBJECT
public:
    ArrowLine(Block* sourceItem, Block* destItem, QPointF, QPointF);
    void adjust();
    QString className()override;
    Block* getDest();
    Block* getSrc();
    void deleteLine(ArrowLine* pEdge);
protected:
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

    //void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
private:
    Block* source, * dest;
    QPointF sourcePoint;
    QPointF destPoint;

    qreal arrowSize;

    bool m_pointFlag;

    QPointF m_pointStart;//ƫ����
    QPointF m_pointEnd;

    static qreal min(qreal r1, qreal r2);
    static qreal abs(qreal r);
};

class PlotPad :public QGraphicsView
{
    Q_OBJECT
public:
    PlotPad(QGraphicsScene* scene);

    //����ͷ�ʱ������ͼ��
    QGraphicsScene* scene;
    SmartEdit* edit;

    void drawItems(Block* it);
    void backLevel();
    void deleteItem();
    void myTest();
    QStack<QList<Block*>*> s;//s ������QList������Ӧ���ǵ�ǰ����ʾ������Items���б�
    QStack<std::string> path;
    std::string getPath();
    Block* getRoot();//����s.top()�б��е�root�ڵ�
protected:
    void dropEvent(QDropEvent* event)override;
    void dragEnterEvent(QDragEnterEvent* event)override;
    void dragMoveEvent(QDragMoveEvent* event)override;

    //�����¼�
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    //����¼�
    void mouseMoveEvent(QMouseEvent* e)override;
    void mousePressEvent(QMouseEvent* e)override;
    void mouseReleaseEvent(QMouseEvent* e)override;
    void mouseDoubleClickEvent(QMouseEvent* e)override;
    

    //��ͼ�¼�
    void paintEvent(QPaintEvent* e) override;
private:
    bool ctrlPressed = false;
    bool leftBtnPressed = false;
    QPoint startPoint, endPoint;
    QGraphicsItem* lastLine = NULL;

public slots:
private slots:
};

