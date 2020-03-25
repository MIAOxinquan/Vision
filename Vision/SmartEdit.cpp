#include "SmartEdit.h"

SmartEdit::SmartEdit(QTabWidget* parent)
	:QPlainTextEdit(parent)
	, rowNumArea(new RowNumArea(this))
	, keyWordsCompleter(NULL)
{
	keyWordsCompleter = new QCompleter(keyWords);
	init();//初始化
	rowContentPlot();//初始化刷新行号块
	//槽函数
	connect(keyWordsCompleter, SIGNAL(activated(QString)), this, SLOT(completeKeyWord(QString)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(rowContentPlot()));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updatePrefix()));

}

SmartEdit::~SmartEdit() {
	delete rowNumArea;	rowNumArea = NULL;
	delete keyWordsCompleter;	keyWordsCompleter = NULL;
}
/*初始化*/
void SmartEdit::init() {
	rowNumArea->setFont(QFont("微软雅黑", 12, QFont::Bold));
	setFont(QFont("微软雅黑", 12));
	setWordWrapMode(QTextOption::NoWrap);  //水平自适应滚动条
	keyWordsCompleter->setWidget(this);
	keyWordsCompleter->setCaseSensitivity(Qt::CaseSensitive); 
	keyWordsCompleter->setCompletionMode(QCompleter::PopupCompletion);//匹配已输入内容,弹出
	keyWordsCompleter->popup()->setFont(QFont("微软雅黑", 12));
	keyWordsCompleter->setMaxVisibleItems(5);
	//加载qss
	QFile file("./Resources/qss/smart.qss");
	file.open(QFile::ReadOnly);
	setStyleSheet(file.readAll());
	file.close();
}
/*重写：刷新大小*/
void SmartEdit::resizeEvent(QResizeEvent* event){
	QRect rct = contentsRect();
	rowNumArea->setGeometry(QRect(rct.left(), rct.top(), rowNumWidth(), rct.height()));
}
/*按键事件*/
void SmartEdit::keyPressEvent(QKeyEvent* event) {
	QPlainTextEdit::keyPressEvent(event);
	if (keyWordsCompleter) {
		if (keyWordsCompleter->popup()->isVisible()) {
			switch (event->key()) {
			case Qt::Key_Enter:
			case Qt::Key_Escape:
			case Qt::Key_Return:
			case Qt::Key_Tab:
				event->ignore(); return;
			default:break;
			}
		} 
	}
	completionPrefix = getCompletionPrefix();
	keyWordsCompleter->setCompletionPrefix(completionPrefix);
	if (completionPrefix == "") { return; }
	if (keyWordsCompleter->completionCount() > 0) {
		QRect cursorRct = cursorRect();
		keyWordsCompleter->complete(QRect(cursorRct.left() + rowNumWidth(), cursorRct.top() + fontMetrics().height(), 150, 1));
	} else {
		keyWordsCompleter->popup()->setVisible(false);
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
/*文本区绘图*/
void SmartEdit::rowContentPlot(/*int*/) {
	//设置文本宽度，使之与行号区适配
	setViewportMargins(rowNumWidth() - 3, -1, -3, 0);
	//当前行高亮
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
/*获取前缀：光标处至上一个空格之间的字符串*/
QString SmartEdit::getCompletionPrefix() const {
	QString selectedText;
	//不断向左移动cursor，并选中字符，并查看选中的单词中是否含有空格——空格作为单词的分隔符
	while (textCursor().movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1)) {
		selectedText = textCursor().selectedText();
		if (selectedText.startsWith(QString(" "))
			|| selectedText.startsWith(QChar(0x422029))) {
			break;
		}
	}
	if (selectedText.startsWith(QChar(0x422029))) {
		selectedText.replace(0, 1, QChar(' '));
	}
	return selectedText.trimmed();
}
/*前缀更新*/
void SmartEdit::updatePrefix() {
	QString curPrefix = getCompletionPrefix();
	if (curPrefix == "") {
		keyWordsCompleter->setCompletionPrefix("----");
		QRect cursorRct = cursorRect();
		keyWordsCompleter->complete(QRect(cursorRct.left(), cursorRct.top(), 0, 0));
	}
}
/*补全关键字*/
void SmartEdit::completeKeyWord(const QString& completion) {
	QString curPrefix = getCompletionPrefix(),
		shouldInsertText = completion;
	if (!completion.contains(curPrefix)) {  // 删除之前未打全的字符
		textCursor().movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, curPrefix.size());
		textCursor().clearSelection();
	}
	else {  // 补全相应的字符
		shouldInsertText = shouldInsertText.replace(
			shouldInsertText.indexOf(curPrefix), curPrefix.size(), "");
	}
	textCursor().insertText(shouldInsertText);
}