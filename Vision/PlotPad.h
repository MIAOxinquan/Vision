#include <QtCore>
#include <QtWidgets>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDomDocument>
#include <QtXml>
class SmartEdit;
class ArrowLine;
class Record;
class RecordList;
/*TipLabel*/
class TipLabel :public QLabel {
public:
    TipLabel();
    QString blockPath;
    void setElidedText();
private:
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
};
/*Item*/
class Item : public QGraphicsItem {
public:
    Item();
    virtual QString className() = 0;
    int  level;
};
/*Block*/
class Block :public Item{
public:
    Block(int x, int y, QString str);
    Block(int x, int y, QString str, int _id);
    Block(QDomElement element);
    //~Block();
    int x, y, w, h, id;
    QString type, blockText, content;
    ArrowLine* inArrow, * outArrow;
    Block* childRoot, *parentBlock;
    QList<Block*>* childrenBlock;

    void outport(QDomDocument& doc, QDomElement& parent);
    //void deleteSelf();
    void setChildRoot(Block* newChildRoot);
    void addChildBlock(Block*newChild,QList<Record*>*records);
    void idMarker(Block* newChild);
    QString className()override;
    QRectF boundingRect() const override;
protected:
    //键盘事件
    /*void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;*/
    //鼠标事件
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event)override;
    //void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    //void mousePressEvent(QGraphicsSceneMouseEvent* event)override;
    //void mouseReleaseEvent(QGraphicsSceneMouseEvent* event)override;
private:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
};
/*ArrowLine*/
class ArrowLine : public Item{
public:
    ArrowLine(Block* sourceItem, Block* destItem, QPointF, QPointF);
    ArrowLine(Block* sourceItem, Block* destItem, QPointF, QPointF, int _level);
    //~ArrowLine();
    Block* fromBlock, * toBlock;

    QString className()override;
    void adjust();
    //void deleteSelf();
protected:
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

private:
    QPointF fromPoint, toPoint
        //偏移量,
        , m_pointStart, m_pointEnd;

    qreal arrowSize;
    bool m_pointFlag;
    static qreal abs(qreal r);
};
/*PlotPad*/
class PlotPad :public QGraphicsView
{
    Q_OBJECT
public:
    PlotPad(QGraphicsScene* scene);

    int indexTotal;
    QString title;
    QGraphicsScene* scene;
    QAction* actionUndo, * actionRedo, * actionDelete, * actionBackLevel;
    SmartEdit* edit;
    TipLabel* pathLabel;

    Block* root;
    QStack<QList<Block*>*> blockStack; //blockStack 顶部的QList里面存的应当是当前层显示出来的Items的列表
    RecordList* recordList;
    QList<Block*>*blockOnPath;

    int getIndexTotal() {
        int ans = ++indexTotal;
        return ans;
    }

    void undo();
    void redo();
    void removeItem();
    QList<Record*>* removeBlock(Block* block);
    void removeArrowLine(ArrowLine* arrowLine);
    void backLevel();
    //void deleteItem();
    //设置主根节点
    void setRoot(Block* root);
    //用于输出PlotPad内所有Block的XML结构
    void outport(QString path);
    //添加一个节点 到 当前界面内
    void addBlock(Block* newBlock);
    void addBlock(Block* newBlock, QList<Record*>* records); //用records记录行为
    //添加一个节点到另一个节点里
    void addBlockIntoBlock(Block* oldBlock, Block* newBlock, QList<Record*>* records);
    //连接两个节点并将连线添加到scene中 并 返回连接线
    ArrowLine* connectBlocks(Block* src, Block* des, int _level, QList<Record*>* records);

    /*
    ActionCTRL
    0 for undoredo's true\false
    1 for backlevel's true\false
    2 for delete's true\false
    */
    void UndoRedoCtrl();
    void BackLevelCtrl();
    void DeleteCtrl();
    void ActionCtrl();
    QString getBlockPath();
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

