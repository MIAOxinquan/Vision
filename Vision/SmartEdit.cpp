#include "stdafx.h"
#include "SmartEdit.h"

SmartEdit::SmartEdit(QTabWidget* parent)
	:QPlainTextEdit(parent)
	, rowNumArea(new RowNumArea(this))
	, keyWordsCompleter(new QCompleter(this))
{
	setWordWrapMode(QTextOption::NoWrap);  //ˮƽ����Ӧ������
	//curEntityCodeLit();
	curRowLit();
	rowNumWidthPlot(0);

	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(rowNumWidthPlot(int)));
	connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(rowNumsPlot(QRect, int)));

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
	int digits = 3;//��ʼλ��
	int max = qMax(1, blockCount());//��ȡ���λ��
	while (max >= 100) {//2λ�����ȹ̶���3λ������������������
		max /= 10;
		++digits;
	}
	//����Ӧ����rowNumberAreaһ��
	int space = rowNumArea->fontMetrics().width(QChar('0')) * digits;
	return space;
}
/*�����ı�������������к�*/
void SmartEdit::rowNumWidthPlot(int) {
	setViewportMargins(rowNumWidth(), 0, 0, 0);
}
/*�к�����ͼ*/
void SmartEdit::rowNumsPlot(const QRect& rct, int dy) {
	if (dy) rowNumArea->scroll(0, dy);
	else  rowNumArea->update(0, rct.y(), rowNumArea->width(), rct.height());
	if (rct.contains(viewport()->rect())) rowNumWidthPlot(0);
}
/*�кſ��ͼ*/
void SmartEdit::rowNumPlot(QPaintEvent* event) {
	QPainter painter(rowNumArea);
	QTextBlock block = firstVisibleBlock();
	painter.fillRect(event->rect(), QColor(Qt::lightGray));
	painter.setBrush(Qt::black);
	int blockNumber = block.blockNumber()
		, top = (int)blockBoundingGeometry(block).translated(contentOffset()).top()
		//ֱ�Ӹ�ֵΪ0���кź��ı��߶Ȳ�����,�������Ϊ�����Ҳ�ᵼ���к�������߶Ȳ�����
		, bottom = top + (int)blockBoundingRect(block).height()
		, numHeight = fontMetrics().height();
	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString num = QString::number(blockNumber + 1);
			painter.drawText(0, top, rowNumArea->width(), numHeight, Qt::AlignRight, num);
			this->update();
		}
		block = block.next();
		top = bottom;
		bottom = top + (int)blockBoundingRect(block).height();
		blockNumber++;
	}
}
/*��ǰ�и���*/
void SmartEdit::curRowLit() {
	if (!isReadOnly()) {
		QList<QTextEdit::ExtraSelection> extraSelections;
		QTextEdit::ExtraSelection selection;
		selection.format.setBackground(QColor(Qt::yellow).lighter(160));//���õ�ǰ�б���ɫ
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
		setExtraSelections(extraSelections);
	}
}
/*��ǰʵ��������*/
void SmartEdit::curEntityCodeLit() {

}