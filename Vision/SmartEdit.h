#include <QtWidgets>
#include <QByteArray>

const QStringList keys = {
	"void","int","float","double","char","string"/*0-5,with space*/
	,"class","struct"/*6,7, with  \n{\n\n};*/
	/*0-7,blue above and 8-18,purple below*/
	,"return","const","case","else"/*8,9,10,11,with space*/
	,"default"/*12,with :*/
	,"continue","break"/*13,14,with ;*/
	,"if"/*15,with ()*/
	,"while"/*16,with ()\n{\n\n}*/
	,"for"/*17,with (;;)\n{\n\n}*/
	,"switch"/*18,with ()\n{\ndefault: break;\n}*/ };
/*
高亮器;
必须写在SmartEdit前
*/
class SyntaxLit : public QSyntaxHighlighter {
	Q_OBJECT
public:
	SyntaxLit(QTextDocument* document = NULL);
	QColor getKeyColor(QString key);
protected:	void highlightBlock(const QString& rowText);
private:		QRegularExpression keysRegExp;
};

class SmartEdit: public QPlainTextEdit {
	Q_OBJECT
public:
	SmartEdit(QTabWidget* parent = Q_NULLPTR);
	~SmartEdit();
	void init();
	//以下两个函数公有，因为RowNumArea需要
	int getRowNumWidth();//行号块宽
	void rowNumPlot(QPaintEvent* event);//行号区绘画

protected:
	void resizeEvent(QResizeEvent* event)override;
	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);

private:
	QString curPrefix;
	QTextCursor curTextCursor;
	QRect curTextCursorRect;/*因为在函数中频繁调用prefix、textCursor以及rect的获取函数，特地用三个变量来替代以节省指令*/
	QWidget* rowNumArea;
	QCompleter* keysCompleter;
	SyntaxLit* syntaxLit;

	QString getPrefix() const;
	
private slots:
	void rowContentPlot();//单行文本块绘画
	void smartComplete(const QString& key);//智能补全关键字
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

