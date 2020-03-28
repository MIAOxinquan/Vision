#include "global.h"
#include "SmartEdit.h"

/*高亮器*/
SyntaxLit::SyntaxLit(QTextDocument* document)
	:QSyntaxHighlighter(document) { 
	QString keysPattern;
	for (int i = 0; i < keys.count(); i++) {
		keysPattern.append(keys.at(i) + "|");
	}
	keysRegExp.setPattern(keysPattern.trimmed());
}
/*
语法高亮
虚函数，同名函数必须实现，函数名不可更改
*/
void SyntaxLit::highlightBlock(const QString& rowText) {
	QTextCharFormat textCharFormat;
	textCharFormat.setFontWeight(QFont::Bold);
	/*
	globalMatch(rowText)作用是，在每行文本QString : rowText遍历匹配任何符合正则表达式的字段
	比如一段文字“intint int func 123 111 int“，正则式包含”int“
	那么将匹配到四个”int“，即使前两个”int“相连
	*/
	QRegularExpressionMatchIterator matchedItr = keysRegExp.globalMatch(rowText);
	while (matchedItr.hasNext()) {//只要keyIterator存在
		QRegularExpressionMatch matchedWord = matchedItr.next();
		textCharFormat.setForeground(QBrush(getKeyColor(matchedWord.captured())));
		setFormat(matchedWord.capturedStart(), matchedWord.capturedLength(), textCharFormat);
	}
}
/*词色转换*/
QColor SyntaxLit::getKeyColor(QString matchedWord) {
	if (keys.contains(matchedWord)) {
		int index = keys.indexOf(matchedWord);
		if (index <= 7) { return Qt::blue; }
		else if (index <= keys.count()) { return QColor(160, 32, 240); }
	}
	return Qt::black;
}

/*smart*/
SmartEdit::SmartEdit(QTabWidget* parent)
	:QPlainTextEdit(parent)
	, rowNumArea(new RowNumArea(this))
	, keysCompleter(Q_NULLPTR)
	, syntaxLit(new SyntaxLit(this->document()))
{
	init();//初始化
	rowContentPlot();//初始化刷新行号块
	//槽函数
	connect(keysCompleter, SIGNAL(activated(QString)), this, SLOT(smartComplete(QString)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(rowContentPlot()));
	connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(update(QRect, int)));
}
SmartEdit::~SmartEdit() {
	delete keysCompleter;	keysCompleter = Q_NULLPTR;
	delete syntaxLit; syntaxLit = Q_NULLPTR;
	delete rowNumArea;	rowNumArea = Q_NULLPTR;
}
/*初始化*/
void SmartEdit::init() {
	//this->setAcceptDrops(false);
	keysCompleter = new QCompleter(keys);//不可改为new QCompleter(keys, this)
	keysCompleter->setWidget(this);
	keysCompleter->setCaseSensitivity(Qt::CaseSensitive); //区分大小写
	keysCompleter->setCompletionMode(QCompleter::PopupCompletion);//匹配已输入内容,弹出
	keysCompleter->setMaxVisibleItems(5);
	keysCompleter->popup()->setFont(QFont("微软雅黑", 12, QFont::Bold));
	rowNumArea->setFont(QFont("微软雅黑", 12, QFont::Bold));
	setFont(QFont("微软雅黑", 12));
	setWordWrapMode(QTextOption::NoWrap);  //水平自适应滚动条
	//加载qss
	loadStyleSheet(this, "smart.qss");
}
/*重写：刷新大小*/
void SmartEdit::resizeEvent(QResizeEvent* event){
	QRect rct = contentsRect();
	rowNumArea->setGeometry(QRect(rct.left(), rct.top(), getRowNumWidth(), rct.height()));
}
/*按键事件*/
void SmartEdit::keyPressEvent(QKeyEvent* event) {
	if (keysCompleter) {
		if (keysCompleter->popup()->isVisible()) {
			switch (event->key()) {
			case Qt::Key_Escape:
			case Qt::Key_Enter:
			case Qt::Key_Return:
			case Qt::Key_Tab:
				event->ignore(); return;
			default:break;
			}
		}
		QPlainTextEdit::keyPressEvent(event);//继承事件可以看作一种开关，相当于更新event，此处位置不可调前
		curPrefix = getPrefix();
		keysCompleter->setCompletionPrefix(curPrefix); // 设置前缀，使Completer寻找关键词
		if (curPrefix == "") { 
			keysCompleter->popup()->setVisible(false); return; 
		} else	{
			if (0==keysCompleter->completionCount()) {
				keysCompleter->popup()->setVisible(false); return;
			} else {
				curTextCursorRect = cursorRect();
				keysCompleter->complete(
					QRect(curTextCursorRect.left() + getRowNumWidth()
						, curTextCursorRect.top(), 200, fontMetrics().height()));
				//加载qss
				loadStyleSheet(keysCompleter->popup(), "completer.qss");
			}
		}
	}
}
/*松键事件*/
void SmartEdit::keyReleaseEvent(QKeyEvent* event) {
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
	QPlainTextEdit::keyReleaseEvent(event);
}
/*释放事件*/
/*
drag = mousePress+mouseMove
drop=mouseMove+mouseRelease
*/
void SmartEdit::dropEvent(QDropEvent* event) {
	setFocus();
	QPlainTextEdit::dropEvent(event);
	moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
	curTextCursor = textCursor();
	int index = curTextCursor.selectedText().toInt();
	curTextCursor.removeSelectedText();
	curTextCursor.insertText(toolSmarts.at(index));
	smartDrop(index);
}
/*拖拽自动补全*/
void SmartEdit::smartDrop(int index) {
	/*先移动光标*/
	int moveIndex = 0, moveLength = 0;
	QString undefinedKey;
	switch (index)
	{
	case 0:case 1:/*class, struct*/
		while (moveIndex < 3) {
			moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
			moveIndex++;
		}
		moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
		break;
	case 2:case 4:case 6:/*if, switch, while*/
		while (moveIndex < 3) {
			moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
			moveIndex++;
		}
		moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
		moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
		break;
	case 3:/*if_else*/
		while (moveIndex < 7) {
			moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
			moveIndex++;
		}
		moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
		moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
		break;
	case 5:/*for*/
		while (moveIndex < 3) {
			moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
			moveIndex++;
		}
		moveIndex = 0;
		moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
		while (moveIndex < 3) {
			moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
			moveIndex++;
		}
		break;
	case 7:/*do(while)*/
		moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor); break;
	case 8:/*func*/
		while (moveIndex < 3) {
			moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
			moveIndex++;
		}
		moveIndex = 0;
		moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
		while (moveIndex < 6) {
			moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
			moveIndex++;
		}
		break;
	default: break;
	}
	moveIndex = 0;
	/*
	再填充待定文本
	做完移动要更新光标副本
	*/
	curTextCursor = textCursor();
	switch (index)
	{
	case 0:case 1:/*class, struct*/
		undefinedKey = undefined.at(0);
		moveLength = undefinedKey.length();
		curTextCursor.insertText(" " + undefinedKey);
		while (moveIndex < moveLength) {
			moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
			moveIndex++;
		}
		break;
	case 2:case 3:case 5:case 6:case 7:/*if,if_else,for,while,do*/
		undefinedKey = undefined.at(1);
		moveLength = undefinedKey.length();
		curTextCursor.insertText(undefinedKey);
		while (moveIndex < moveLength) {
			moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
			moveIndex++;
		}
		break;
	case 4:/*switch*/
		undefinedKey = undefined.at(2);
		moveLength = undefinedKey.length();
		curTextCursor.insertText(undefinedKey);
		while (moveIndex < moveLength) {
			moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
			moveIndex++;
		}
		break;
	case 8:/*func*/
		undefinedKey = undefined.at(3);
		moveLength = undefinedKey.length();
		if (0 == curTextCursor.position()) {
			curTextCursor.insertText(undefinedKey);
		} 
		else {
			curTextCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
			QString selectedChar = curTextCursor.selectedText();
			curTextCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
			if (" " == selectedChar || QString(0x422029) == selectedChar) {
				curTextCursor.insertText(undefinedKey);
			}
			else {
				curTextCursor.insertText(" " + undefinedKey);
			}
		}
		while (moveIndex < moveLength) {
			moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
			moveIndex++;
		}
		break;
	default: break;
	}
}
/*行号块宽*/
int SmartEdit::getRowNumWidth() {
	int digits = 4;//初始位数
	int max = qMax(1, blockCount());//获取最大位数，默认至少1位
	while (max >= 1000) {//3位数则宽度固定，4位数及其以上则宽度自增
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
			//rowNumArea->update();//每次绘制字体后刷新,过于消耗CPU
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
	setViewportMargins(getRowNumWidth() - 3, -1, -3, 0);
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
/*
刷新界面，只和updateRequest关联相比paintEvent大大降低cpu消耗
*/
void SmartEdit::update(const QRect& rect, int dy) {
	if (dy) rowNumArea->scroll(0, dy);
	else  rowNumArea->update(0, rect.y(), rowNumArea->width(), rect.height());
	if (rect.contains(viewport()->rect())) setViewportMargins(getRowNumWidth() - 3, -1, -3, 0);
}
/*获取前缀：光标处至上一个空格之间的字符串*/
QString SmartEdit::getPrefix() const {
	QString selectedText;
	/*
	获取textCursor的副本以模拟左移获取选择文本，如此不会影响到实际光标的位置
	不能用textCursor()直接操作，否则导致崩溃
	*/
	QTextCursor curTextCursor = textCursor();
	while (curTextCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1)) {
		selectedText = curTextCursor.selectedText();//只要不遇到空格或者换行符，就获取选择文本
		if (selectedText.startsWith(' ') || selectedText.startsWith(QChar(0x422029))) {
			break;//左移至出现空格或者换行符时终止
		}
	}
	if (selectedText.startsWith(QChar(0x422029))) {
		selectedText.replace(0, 1, QChar(' '));//换行符则替换为空格
	}
	if (keys.contains(selectedText.trimmed())) { 
		return "";//全匹配则不需要补全，前缀设置空
	}
	return selectedText.trimmed();
}
/*智能补全*/
void SmartEdit::smartComplete(const QString& key) {
	QString curPrefix = getPrefix(), supplement = key, undefinedKey;
	int moveIndex = 0, moveLength = 0;
	curTextCursor = textCursor();
	//不需要判断界限，因为参数一定是key
	int index = keys.indexOf(key);
	if (!key.contains(curPrefix)) { //前缀不匹配关键字则取消选择
		curTextCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, curPrefix.size());
		curTextCursor.clearSelection();
	}
	else { //前缀匹配关键字则获取还未输入的部分
		supplement = supplement.replace(supplement.indexOf(curPrefix), curPrefix.size(), "");
	}
	curTextCursor.insertText(supplement);
	//以下是智能处理核心
	switch (index)
	{
	default:/*0-5,8-11,void, int, float, double, char, string, return, const, case, else*/
		textCursor().insertText(" "); break;
	case 12:/*default*/
		curTextCursor.insertText(": "); break;
	case 13:case 14:/*continue,break*/
		curTextCursor.insertText("; "); break;
	case 6:case 7:/*class, struct*/
		curTextCursor.insertText(smarts.at(0));
		while (moveIndex < 3) {
			moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
			moveIndex++;
		}
		moveIndex = 0;
		moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
		undefinedKey = undefined.at(0);
		moveLength = undefinedKey.length();
		textCursor().insertText(" " + undefinedKey);
		while (moveIndex < moveLength) {
			moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
			moveIndex++;
		}
		break;
	case 15:case 16:/*if, while*/
		curTextCursor.insertText(smarts.at(1));
		moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
		undefinedKey = undefined.at(1);
		moveLength = undefinedKey.length();
		textCursor().insertText(undefinedKey);
		while (moveIndex < moveLength) {
			moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
			moveIndex++;
		}
		break;
	case 17:/*for*/
		curTextCursor.insertText(smarts.at(2));
		while (moveIndex < 3) {
			moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
			moveIndex++;
		}
		moveIndex = 0;
		moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
		while (moveIndex < 3) {
			moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
			moveIndex++;
		}
		moveIndex = 0;
		undefinedKey = undefined.at(1);
		moveLength = undefinedKey.length();
		textCursor().insertText(undefinedKey);
		while (moveIndex < moveLength) {
			moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
			moveIndex++;
		}
		break;
	case 18:/*do(while)*/
		curTextCursor.insertText(smarts.at(3));
		moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
		undefinedKey = undefined.at(1);
		moveLength = undefinedKey.length();
		textCursor().insertText(undefinedKey);
		while (moveIndex < moveLength) {
			moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
			moveIndex++;
		}
		break;
	case 19:/*if_else*/
		while (moveIndex < 5) {
			curTextCursor.deletePreviousChar();
			moveIndex++;
		}
		moveIndex = 0;
		curTextCursor.insertText(smarts.at(4));
		while (moveIndex < 7) {
			moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
			moveIndex++;
		}
		moveIndex = 0;
		moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
		moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
		undefinedKey = undefined.at(1);
		moveLength = undefinedKey.length();
		textCursor().insertText(undefinedKey);
		while (moveIndex < moveLength) {
			moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
			moveIndex++;
		}
		break;
	case 20:/*switch*/
		curTextCursor.insertText(smarts.at(5));
		while (moveIndex < 3) {
			moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
			moveIndex++;
		}
		moveIndex = 0;
		moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
		moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
		undefinedKey = undefined.at(2);
		moveLength = undefinedKey.length();
		textCursor().insertText(undefinedKey);
		while (moveIndex < moveLength) {
			moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
			moveIndex++;
		}
		break;
	}
}