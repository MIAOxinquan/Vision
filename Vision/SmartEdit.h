#include <QtWidgets>

class SmartEdit: public QPlainTextEdit
{
	Q_OBJECT
public:
	SmartEdit(QTabWidget* parent = Q_NULLPTR);
	~SmartEdit();
	int rowNumWidth();//行号块宽
	void rowNumPlot(QPaintEvent* event);//行号块绘画

protected:
	void resizeEvent(QResizeEvent* event)override;
	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);

private:
	QWidget* rowNumArea;
	QCompleter* keyWordsCompleter;

	void curRowLit();//当前行高亮

private slots:
	void rowNumsPlot(const QRect&, int);//行号区绘画
	void rowNumWidthPlot(int);
	void curEntityCodeLit();//当前实体代码高亮
};

class RowNumArea : public QWidget {
public:
	explicit RowNumArea(SmartEdit* smartEdit) : QWidget(smartEdit) {
		smart = smartEdit; //setVisible(true);
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