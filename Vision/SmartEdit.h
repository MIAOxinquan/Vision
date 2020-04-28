#include <QtWidgets>
#include <QByteArray>
class Block;
class PlotPad;
/*
������;
����д��SmartEditǰ
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

	int getRowNumWidth();//�кſ��
	void rowNumPlot(QPaintEvent* event);//�к����滭
	QString smartCore(QString key); 
	QStringList getChildNodeContent();
	QString getParentNodeContent();
	void showContent(Block* block);
	void showContent(PlotPad* plot);
	QString getContent(Block* block);

protected:
	void resizeEvent(QResizeEvent* event)override;
	void keyPressEvent(QKeyEvent* event)override;
	void keyReleaseEvent(QKeyEvent* event)override;
	void dropEvent(QDropEvent* event)override;

private:
	/*����*/
	QString curPrefix;
	QTextCursor curTextCursor;
	QRect curTextCursorRect;/*��Ϊ�ں�����Ƶ������prefix��textCursor�Լ�rect�Ļ�ȡ�������ص�����������������Խ�ʡָ��*/
	QWidget* rowNumArea;
	QCompleter* keysCompleter;
	SyntaxLit* syntaxLit;
	/*����*/
	QString getPrefix() const;

private slots:
	void rowContentPlot();//�����ı���滭
	void smartComplete(const QString& key);//���ܲ�ȫ�ؼ���
	void update(const QRect& rect, int dy);//ˢ��
};

/*
�к���;
����д��SmartEdit��
*/
class RowNumArea : public QWidget {
public:
	explicit RowNumArea(SmartEdit* smartEdit) : QWidget(smartEdit) { smart = smartEdit; }
	QSize sizeHint() const override { return QSize(smart->getRowNumWidth(), 0); }
protected:
	void paintEvent(QPaintEvent* event) override { smart->rowNumPlot(event); }
private: 	SmartEdit* smart;
};

