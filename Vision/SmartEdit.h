#include <QtWidgets>

class SmartEdit: public QPlainTextEdit
{
	Q_OBJECT
public:
	SmartEdit(QTabWidget* parent = Q_NULLPTR);
	~SmartEdit();
	int rowNumWidth();//�кſ��
	void rowNumPlot(QPaintEvent* event);//�кſ�滭

protected:
	void resizeEvent(QResizeEvent* event)override;
	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);

private:
	QWidget* rowNumArea;
	QCompleter* keyWordsCompleter;

private slots:
	void rowNumsPlot(const QRect&, int);//�к����滭
	void rowNumWidthPlot(int);
	void curRowLit();//��ǰ�и���
	void curEntityCodeLit();//��ǰʵ��������
};

class RowNumArea : public QWidget {
public:
	explicit RowNumArea(SmartEdit* smartEdit) : QWidget(smartEdit) {
		smart = smartEdit;
	}
	QSize sizeHint() const override {
		return QSize(smart->rowNumWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent* event) override {
		smart->rowNumPlot(event);
	}

private:
	SmartEdit* smart;
};