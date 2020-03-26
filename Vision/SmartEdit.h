#include <QtWidgets>
#include <QByteArray>

const QStringList keyWords = { "void","int","float","double","string","char","if","else","switch","for","while" };
/*
高亮器
必须写在SmartEdit前
*/
class SyntaxLit : public QSyntaxHighlighter {
	Q_OBJECT
public:
	SyntaxLit(QTextDocument* document = NULL);
protected:	void highlightBlock(const QString& text);
private:		QRegularExpression litRegExp;
};

class SmartEdit: public QPlainTextEdit {
	Q_OBJECT
public:
	SmartEdit(QTabWidget* parent = Q_NULLPTR);
	~SmartEdit();
	void init();
	int rowNumWidth();//行号块宽
	void rowNumPlot(QPaintEvent* event);//行号区绘画
	QString getCurPrefix() const;

protected:
	void resizeEvent(QResizeEvent* event)override;
	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);

private:
	QString completionPrefix;
	QTextCursor curTextCursor;
	QRect curTextCursorRect;
	QWidget* rowNumArea;
	QCompleter* kWordsCompleter;
	SyntaxLit* syntaxLit;
	
public slots:
	void completeKWord(const QString& completion);//补全关键字
	void updatePrefix(void);//更新前缀
private slots:
	void rowContentPlot();//单行文本块绘画
};

/*
行号区
必须写在SmartEdit后
*/
class RowNumArea : public QWidget {
public:
	explicit RowNumArea(SmartEdit* smartEdit) : QWidget(smartEdit) { smart = smartEdit; }
	QSize sizeHint() const override { return QSize(smart->rowNumWidth(), 0); }
protected:
	void paintEvent(QPaintEvent* event) override { smart->rowNumPlot(event); }
private: 	SmartEdit* smart;
};

