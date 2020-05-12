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
    QString className()override;
    QRectF boundingRect() const override;
protected:
    //�����¼�
    /*void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;*/
    //����¼�
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
        //ƫ����,
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
    QStack<QList<Block*>*> blockStack; //blockStack ������QList������Ӧ���ǵ�ǰ����ʾ������Items���б�
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
    //���������ڵ�
    void setRoot(Block* root);
    //�������PlotPad������Block��XML�ṹ
    void outport(QString path);
    //���һ���ڵ� �� ��ǰ������
    void addBlockIntoPad(Block* newBlock);
    void addBlockIntoPad(Block* newBlock, QList<Record*>* records); //��records��¼��Ϊ
    //���һ���ڵ㵽��һ���ڵ���
    void addBlockIntoBlock(Block* oldBlock, Block* newBlock, QList<Record*>* records);
    //���������ڵ㲢��������ӵ�scene�� �� ����������
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

    //�����¼�
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    //����¼�
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

