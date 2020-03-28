#include <QtWidgets>
#include <QByteArray>

/*
������;
����д��SmartEditǰ
*/
class SyntaxLit : public QSyntaxHighlighter {
	Q_OBJECT
public:
	SyntaxLit(QTextDocument* document = Q_NULLPTR);
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
	//���������������У���ΪClass RowNumArea��Ҫ
	int getRowNumWidth();//�кſ��
	void rowNumPlot(QPaintEvent* event);//�к����滭

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
	void smartDrop(int index);
	
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

