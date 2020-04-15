#include <QtWidgets>
#include <QObject>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QScrollBar>
#include <qset.h>
class PlotEdge;
class Item :public QGraphicsItem
{
public:
    virtual QString className() = 0;
};
class PlotItem :public Item {
public:
    int x, y;
    QString head;
    QString content;
    PlotItem(int x, int y, QString str);
    QRectF boundingRect() const override;
    //QRectF sceneBoundingRec() const override;
    PlotItem* toItem = NULL;
    PlotEdge* toEdge = NULL;
    PlotEdge* fromEdge = NULL;
    QString className()override;
    //QSet<PlotEdge*> edges;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
protected:
    //�����¼�
    /*void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;*/
    //����¼�
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event)override;
    /*void mousePressEvent(QGraphicsSceneMouseEvent* event)override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event)override;*/
private:
    bool ctlPressed = false;
    void drawToItem(QPainter* painter);

};

class PlotEdge : public Item,public QObject {
    //Q_OBJECT
public:
    PlotEdge(PlotItem* sourceItem, PlotItem* destItem, QPointF, QPointF);
    void adjust();
    QString className()override;
    PlotItem* getDest();
    PlotItem* getSrc();
protected:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    //void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
private:
    PlotItem* source, * dest;

    QPointF sourcePoint;
    QPointF destPoint;

    qreal arrowSize;

    bool m_pointFlag;

    QPointF m_pointStart;//ƫ����
    QPointF m_pointEnd;

    //QAction* m_removeAction;
////signals:
////    //void remove(Edge*);
////private slots:
////    void slotRemoveItem();
};

class PlotPad :public QGraphicsView
{
	Q_OBJECT
public:
    PlotPad(QGraphicsScene* scene);
    
    //����ͷ�ʱ������ͼ��
    QGraphicsScene* scene;
    QScrollBar* hBar;
    QScrollBar* vBar;
    void drawItems(PlotItem* it);
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

    //��ͼ�¼�

    void paintEvent(QPaintEvent* e) override;
private:
    bool ctlPressed = false;
    QPoint startPoint, endPoint;
    bool leftBtnClicked = false;
    QGraphicsItem* lastLine = NULL;
public slots:
private slots:
};

