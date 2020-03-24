#include "stdafx.h"
#include "SmartEdit.h"

SmartEdit::SmartEdit(QTabWidget* parent)
	:QPlainTextEdit(parent)
	, rowNumArea(new RowNumArea(this))
	, keyWordsCompleter(new QCompleter(this))
{
	rowNumArea->setFont(QFont("微软雅黑", 12, QFont::Bold));
	setFont(QFont("微软雅黑", 12));
	setWordWrapMode(QTextOption::NoWrap);  //水平自适应滚动条
	rowContentPlot();//初始化刷新行号块

	//加载qss
	QFile file("./Resources/qss/smart.qss");
	file.open(QFile::ReadOnly);
	setStyleSheet(file.readAll());
	file.close();

	//槽函数
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(rowContentPlot()));

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
		textCursor().insertText(")");
		moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
		break;
	case  Qt::Key_BracketLeft:
		textCursor().insertText("]");
		moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
		break;
	case  Qt::Key_BraceLeft:
		textCursor().insertText("\n\n}");
		moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
		break;
	case Qt::Key_Apostrophe:
		textCursor().insertText("\'");
		moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
		break;
	case Qt::Key_QuoteDbl:
		textCursor().insertText("\"");
		moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
		break;
	default: break;
	}
}
/*行号块宽*/
int SmartEdit::rowNumWidth() {
	int digits = 4;//初始位数
	int max = qMax(1, blockCount());//获取最大位数，默认至少1位
	while (max >= 1000) {//2位数则宽度固定，3位数及其以上则宽度自增
		max /= 10;
		++digits;
	}
	//字体应当和rowNumberArea一致，fontMetrics()
	int space = rowNumArea->fontMetrics().width(QChar('0')) * digits;
	return space;
}
/*行号块绘图*/
void SmartEdit::rowNumPlot(QPaintEvent* event) {
	QPainter painter(rowNumArea);
	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber()
		, top = (int)blockBoundingGeometry(block).translated(contentOffset()).top()//直接赋值为0则行号和文本高度不对齐
		, height = (int)blockBoundingRect(block).height()//块高设置为字体高也会导致行号与字体高度不对齐
		, bottom = top + height;
	
	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString num = QString::number(blockNumber + 1);
			painter.drawText(0, top, rowNumArea->width() - 3, height, Qt::AlignRight, num);
			rowNumArea->update();//每次绘制字体后刷新
		}
		block = block.next();
		top = bottom;
		bottom = top + height;
		blockNumber++;
	}
}
/*设置文本框左距以适配行号*/
void SmartEdit::rowContentPlot(/*int*/) {
	setViewportMargins(rowNumWidth() - 3, -1, -3, 0);
	if (!isReadOnly()) {
		QList<QTextEdit::ExtraSelection> extraSelections;
		QTextEdit::ExtraSelection selection;
		selection.format.setBackground(QColor(Qt::cyan).lighter(192));//设置当前行背景色
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
		setExtraSelections(extraSelections);
	}
}