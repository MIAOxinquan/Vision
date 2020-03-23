#include "stdafx.h"
#include "SmartEdit.h"

SmartEdit::SmartEdit(QTabWidget* parent)
	:QPlainTextEdit(parent)
	, rowNumArea(new RowNumArea(this))
	, keyWordsCompleter(new QCompleter(this))
{
	rowNumArea->setFont(QFont("Arial", 12, QFont::Bold));
	setFont(QFont("Arial", 12));
	setWordWrapMode(QTextOption::NoWrap);  //ˮƽ����Ӧ������
	rowContentPlot();//��ʼ��ˢ���кſ�

	//����qss
	QFile file("./Resources/qss/smart.qss");
	file.open(QFile::ReadOnly);
	setStyleSheet(file.readAll());
	file.close();

	//�ۺ���
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(rowContentPlot()));

}

SmartEdit::~SmartEdit() {
	delete rowNumArea;
	delete keyWordsCompleter;
}

/*��д��ˢ�´�С*/
void SmartEdit::resizeEvent(QResizeEvent* event){
	QPlainTextEdit::resizeEvent(event);
	QRect rct = contentsRect();
	rowNumArea->setGeometry(QRect(rct.left(), rct.top(), rowNumWidth(), rct.height()));
}
/*�����¼�*/
void SmartEdit::keyPressEvent(QKeyEvent* event) {
	QPlainTextEdit::keyPressEvent(event);
	if (keyWordsCompleter && keyWordsCompleter->popup()->isVisible()) {
		switch (event->key())
		{
		case Qt::Key_Enter:
		case Qt::Key_Escape:
		case Qt::Key_Return:
		case Qt::Key_Tab:
			event->ignore(); return;
		default:break;
		}
	}

}
/*�ͼ��¼�*/
void SmartEdit::keyReleaseEvent(QKeyEvent* event) {
	QPlainTextEdit::keyReleaseEvent(event);
	switch (event->key())
	{
	case Qt::Key_ParenLeft:
		this->textCursor().insertText(")");
		textCursor().movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
		textCursor().clearSelection(); break;
	case  Qt::Key_BracketLeft:
		this->textCursor().insertText("]"); break;
	case  Qt::Key_BraceLeft:
		this->textCursor().insertText("}"); break;
	default: break;
	}
}
/*�кſ��*/
int SmartEdit::rowNumWidth() {
	int digits = 4;//��ʼλ��
	int max = qMax(1, blockCount());//��ȡ���λ����Ĭ������1λ
	while (max >= 1000) {//2λ�����ȹ̶���3λ������������������
		max /= 10;
		++digits;
	}
	//����Ӧ����rowNumberAreaһ�£�fontMetrics()
	int space = rowNumArea->fontMetrics().width(QChar('0')) * digits;
	return space;
}
/*�кſ��ͼ*/
void SmartEdit::rowNumPlot(QPaintEvent* event) {
	QPainter painter(rowNumArea);
	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber()
		, top = (int)blockBoundingGeometry(block).translated(contentOffset()).top()//ֱ�Ӹ�ֵΪ0���кź��ı��߶Ȳ�����
		, height = (int)blockBoundingRect(block).height()//�������Ϊ�����Ҳ�ᵼ���к�������߶Ȳ�����
		, bottom = top + height;
	
	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString num = QString::number(blockNumber + 1);
			painter.drawText(0, top, rowNumArea->width() - 3, height, Qt::AlignRight, num);
			rowNumArea->update();//ÿ�λ��������ˢ��
		}
		block = block.next();
		top = bottom;
		bottom = top + height;
		blockNumber++;
	}
}
/*�����ı�������������к�*/
void SmartEdit::rowContentPlot(/*int*/) {
	qDebug() <<blockCount()<<":"<< verticalScrollBar()->sliderPosition();
	setViewportMargins(rowNumWidth() - 3, 0, -3, 0);
	if (!isReadOnly()) {
		QList<QTextEdit::ExtraSelection> extraSelections;
		QTextEdit::ExtraSelection selection;
		selection.format.setBackground(QColor(Qt::cyan).lighter(180));//���õ�ǰ�б���ɫ
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
		setExtraSelections(extraSelections);
	}
}