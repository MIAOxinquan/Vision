#include <QtWidgets>
#include <QByteArray>

class SmartEdit: public QPlainTextEdit
{
	Q_OBJECT
public:
	SmartEdit(QTabWidget* parent = Q_NULLPTR);
	~SmartEdit();
	void init();
	QString getCompletionPrefix() const;
	int rowNumWidth();//�кſ��
	void rowNumPlot(QPaintEvent* event);//�к����滭

protected:
	void resizeEvent(QResizeEvent* event)override;
	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);

private:
	QString completionPrefix;
	QStringList keyWords = { "int","float","double","string","char","if","else","switch","for","while" };
	QWidget* rowNumArea;
	QCompleter* keyWordsCompleter;

private slots:
	void completeKeyWord(const QString& completion);//��ȫ�ؼ���
	void updatePrefix(void);//����ǰ׺
	void rowContentPlot();//�����ı���滭
};

//�к���
class RowNumArea : public QWidget {
public:
	explicit RowNumArea(SmartEdit* smartEdit) : QWidget(smartEdit) {	smart = smartEdit;}
	QSize sizeHint() const override {	return QSize(smart->rowNumWidth(), 0);}

protected:
	void paintEvent(QPaintEvent* event) override {	smart->rowNumPlot(event);}

private: 	SmartEdit* smart;
};
