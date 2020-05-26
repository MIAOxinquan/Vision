#include <QtWidgets>
#include <QByteArray>
class Block;
class PlotPad;
/*
高亮器;
必须写在SmartEdit前
*/
class SyntaxLit : public QSyntaxHighlighter {
	Q_OBJECT
public:
	SyntaxLit(QTextDocument* document = Q_NULLPTR);
protected:	void highlightBlock(const QString& rowText);
private:	
	QList<QRegularExpression>lits;// lit_blue, lit_purple, lit_quotes, lit_sigCmt;
	QRegExp lit_sigCmt, lit_mulCmtStart, lit_mulCmtEnd, lit_nodeStart, lit_nodeEnd;
};

class SmartEdit: public QPlainTextEdit {
	Q_OBJECT
public:
	SmartEdit(QTabWidget* parent = Q_NULLPTR);
	~SmartEdit();

	int getRowNumWidth();//行号块宽
	void rowNumPlot(QPaintEvent* event);//行号区绘画
	QString smartCore(QString key); 
	QStringList getChildNodeContent();
	QString getParentNodeContent();
	void showContent(Block* block);
	QString showContent(PlotPad* plot);
	QString getContent(Block* block);

protected:
	void resizeEvent(QResizeEvent* event)override;
	void keyPressEvent(QKeyEvent* event)override;
	void keyReleaseEvent(QKeyEvent* event)override;
	void dropEvent(QDropEvent* event)override;

private:
	/*变量*/
	QString curPrefix;
	QTextCursor curTextCursor;
	QRect curTextCursorRect;/*因为在函数中频繁调用prefix、textCursor以及rect的获取函数，特地用三个变量来替代以节省指令*/
	QWidget* rowNumArea;
	QCompleter* keysCompleter;
	SyntaxLit* syntaxLit;
	/*函数*/
	QString getPrefix() const;

private slots:
	void rowContentPlot();//单行文本块绘画
	void smartComplete(const QString& key);//智能补全关键字
	void update(const QRect& rect, int dy);//刷新
};

/*
行号区;
必须写在SmartEdit后
*/
class RowNumArea : public QWidget {
public:
	explicit RowNumArea(SmartEdit* smartEdit) : QWidget(smartEdit) { smart = smartEdit; }
	QSize sizeHint() const override { return QSize(smart->getRowNumWidth(), 0); }
protected:
	void paintEvent(QPaintEvent* event) override { smart->rowNumPlot(event); }
private: 	SmartEdit* smart;
};

