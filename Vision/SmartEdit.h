#include <QtWidgets>
#include <QByteArray>

const QStringList keyWords = { "void","int","float","double","string","char","if","else","switch","for","while" };
/*
������
����д��SmartEditǰ
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
	int rowNumWidth();//�кſ��
	void rowNumPlot(QPaintEvent* event);//�к����滭
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
	void completeKWord(const QString& completion);//��ȫ�ؼ���
	void updatePrefix(void);//����ǰ׺
private slots:
	void rowContentPlot();//�����ı���滭
};

/*
�к���
����д��SmartEdit��
*/
class RowNumArea : public QWidget {
public:
	explicit RowNumArea(SmartEdit* smartEdit) : QWidget(smartEdit) { smart = smartEdit; }
	QSize sizeHint() const override { return QSize(smart->rowNumWidth(), 0); }
protected:
	void paintEvent(QPaintEvent* event) override { smart->rowNumPlot(event); }
private: 	SmartEdit* smart;
};

