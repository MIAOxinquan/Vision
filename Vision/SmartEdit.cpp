#include "stdafx.h"
#include "SmartEdit.h"

SmartEdit::SmartEdit(QTabWidget* parent)
	:QPlainTextEdit(parent)
	, rowNumArea(new RowNumArea(this))
	, keyWordsCompleter(new QCompleter(this))
{
	setFont(QFont("微软雅黑", 12, QFont::Bold));
	rowNumArea->setFont(QFont("微软雅黑",12,QFont::Bold));
	setWordWrapMode(QTextOption::NoWrap);  //水平自适应滚动条
	rowNumWidthPlot(0);//初始化刷新行号块宽
	curRowLit();

	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(rowNumWidthPlot(int)));
	connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(rowNumsPlot(QRect, int)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(curRowLit()));

}

SmartEdit::~SmartEdit() {
	delete rowNumArea;
	delete keyWordsCompleter;
}

/*重写：刷新大小*/
void SmartEdit::resizeEvent(QResizeEvent* event){
	QPlainTextEdit::resizeEvent(event);
	QRect rct = contentsRect();
	rowNumArea->setGeometry(QRect(rct.left(), rct.top(), rowNumWidth(), rct.height()));
}
/*按键事件*/
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
/*释键事件*/
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
/*行号块宽*/
int SmartEdit::rowNumWidth() {
	int digits = 3;//初始位数
	int max = qMax(1, blockCount());//获取最大位数
	while (max >= 100) {//2位数则宽度固定，3位数及其以上则宽度自增
		max /= 10;
		++digits;
	}
	//字体应当和rowNumberArea一致
	int space = rowNumArea->fontMetrics().width(QChar('0')) * digits;
	return space;
}
/*设置文本框左距以适配行号*/
void SmartEdit::rowNumWidthPlot(int) {
	setViewportMargins(rowNumWidth(), 0, 0, 0);
}
/*行号区绘图*/
void SmartEdit::rowNumsPlot(const QRect& rct, int dy) {
	if (dy) rowNumArea->scroll(0, dy);
	else  rowNumArea->update(0, rct.y(), rowNumArea->width(), rct.height());
	if (rct.contains(viewport()->rect())) rowNumWidthPlot(0);
}
/*行号块绘图*/
void SmartEdit::rowNumPlot(QPaintEvent* event) {
	QPainter painter(rowNumArea);
	QTextBlock block = firstVisibleBlock();
	painter.fillRect(event->rect(), QColor(Qt::lightGray));
	painter.setBrush(Qt::black);
	int blockNumber = block.blockNumber()
		, top = (int)blockBoundingGeometry(block).translated(contentOffset()).top()
		//直接赋值为0则行号和文本高度不对齐,块高设置为字体高也会导致行号与字体高度不对齐
		, bottom = top + (int)blockBoundingRect(block).height()
		, height = fontMetrics().height();
	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString num = QString::number(blockNumber + 1);
			painter.drawText(0, top, rowNumArea->width(), height, Qt::AlignRight, num);
			this->update();
		}
		block = block.next();
		top = bottom;
		bottom = top + (int)blockBoundingRect(block).height();
		blockNumber++;
	}
}
/*当前行高亮*/
void SmartEdit::curRowLit() {
	if (!isReadOnly()) {
		QList<QTextEdit::ExtraSelection> extraSelections;
		QTextEdit::ExtraSelection selection;
		selection.format.setBackground(QColor(Qt::yellow).lighter(155));//设置当前行背景色
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
		setExtraSelections(extraSelections);
	}
}
/*当前实体代码高亮*/
void SmartEdit::curEntityCodeLit() {

}