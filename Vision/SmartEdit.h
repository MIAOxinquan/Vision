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
������;
����д��SmartEditǰ
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
	//���������������У���ΪRowNumArea��Ҫ
	int getRowNumWidth();//�кſ��
	void rowNumPlot(QPaintEvent* event);//�к����滭

protected:
	void resizeEvent(QResizeEvent* event)override;
	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);

private:
	QString curPrefix;
	QTextCursor curTextCursor;
	QRect curTextCursorRect;/*��Ϊ�ں�����Ƶ������prefix��textCursor�Լ�rect�Ļ�ȡ�������ص�����������������Խ�ʡָ��*/
	QWidget* rowNumArea;
	QCompleter* keysCompleter;
	SyntaxLit* syntaxLit;

	QString getPrefix() const;
	
private slots:
	void rowContentPlot();//�����ı���滭
	void smartComplete(const QString& key);//���ܲ�ȫ�ؼ���
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

