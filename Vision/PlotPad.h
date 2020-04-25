#include <QtCore>
#include <QtWidgets>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
class SmartEdit;
class ArrowLine;
class UndoRedoStack;
/*TipLabel*/
class TipLabel :public QLabel {
    Q_OBJECT
public:
    explicit TipLabel();
    void setElidedText(QString fullText);
private:
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
};
/*Item*/
class Item : public QGraphicsItem {
public:
    Item();
    virtual QString className() = 0;
};
/*Block*/
class Block :public Item{
public:
    Block(int x, int y, QString str);
    //~Block();
    int x, y, w, h, id;
    QString type;
    QString content;
    ArrowLine* inArrow, * outArrow;
    Block* childRoot;
    QList<Block*>* childrenBlock;

    QString className()override;
    QRectF boundingRect() const override;
    void deleteSelf();
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
/*ArrowLine*/
class ArrowLine : public Item{
public:
    ArrowLine(Block* sourceItem, Block* destItem, QPointF, QPointF);
    //~ArrowLine();
    Block* fromBlock, * toBlock;

    QString className()override;
    void adjust();
    void deleteSelf();
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
/*PlotPad*/
class PlotPad :public QGraphicsView
{
    Q_OBJECT
public:
    PlotPad(QGraphicsScene* scene);

    int indexTotal;
    QGraphicsScene* scene;
    SmartEdit* edit;
    TipLabel* pathLabel;
    Block* root;
    QStack<QList<Block*>*> blockStack; //blockStack 顶部的QList里面存的应当是当前层显示出来的Items的列表
    UndoRedoStack* undoRedoStack;
    QStringList nodesOnPath;

    void undo();
    void redo();
    void backLevel();
    void deleteItem();
    void setRoot(Block* root);
    QString getNodesPath();
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

private:
    bool ctrlPressed, leftBtnPressed;
    QPoint startPoint, endPoint;
    QGraphicsItem* lastLine ;

public slots:
private slots:
};

