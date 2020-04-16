#include "global.h"
#include "SmartEdit.h"

/*高亮器*/
SyntaxLit::SyntaxLit(QTextDocument* document)
	:QSyntaxHighlighter(document) {
	QRegularExpression lit;
	QString keysPattern = "";
	for each (const QString key in keys_cpp_purple) {
		keysPattern.append(key + "|");
	}
	lit.setPattern(keysPattern);
	lits.append(lit);
	keysPattern = "";
	for each (const QString  key in keys_cpp_blue) {
		keysPattern.append(key + "|");
	}
	lit.setPattern(keysPattern);	
	lits.append(lit);
	keysPattern = "";
	for each (const QString key in keys_cpp_normal) {
		keysPattern.append(key + "|");
	}
	for each (const QString key in (keys_cpp_purple + keys_cpp_blue)) {
		if ("signed" == key) {
			keysPattern.append(
				"\\w*[a-mo-zA-Z0-9_]" + key + "|"
				+ "\\w*[a-tv-zA-Z0-9_]n" + key + "|"
				+ key + "\\w+|");
		}
		else if ("do"==key) {
			keysPattern.append(
				"\\w+" + key + "|"
				+ key + "[a-tv-zA-Z0-9_]\\w*|"
				+ key + "u[ac-zA-Z0-9_]\\w*|"
				+ key + "ub[a-km-zA-Z0-9_]\\w*|"
				+ key + "ubl[a-df-zA-Z0-9_]\\w*|");
		}
		else {
			keysPattern.append("\\w+" + key + "|" + key + "\\w+|");
		}
	}
	lit.setPattern(keysPattern);
	lits.append(lit);
	lit.setPattern(sigQuote + "|" + quote);
	lits.append(lit);
	lit.setPattern(sigCmt);	
	lits.append(lit);
	lit_mulCmtStart.setPattern(mulCmtStart);
	lit_mulCmtEnd.setPattern(mulCmtEnd);
}
/*
语法高亮
虚函数，同名函数必须实现，函数名不可更改
*/
/*
globalMatch(rowText)作用是，在文本QString : rowText遍历匹配任何符合正则表达式的字段
比如一段文字“intint int func 123 111 int“，正则式包含”int“
那么将匹配到四个”int“，即使前两个”int“相连
*/
void SyntaxLit::highlightBlock(const QString& rowText) {
	QTextCharFormat textCharFormat;
	QRegularExpressionMatchIterator matchedItr;
	for (int i = 0; i < lits.count(); i++) {
		matchedItr = lits.at(i).globalMatch(rowText);
		textCharFormat.setForeground(colors.at(i));
		if (2 == i) {
			textCharFormat.setFontWeight(QFont::Normal);
		}
		else {
			textCharFormat.setFontWeight(QFont::Bold);
		}
		while (matchedItr.hasNext()) {//只要keyIterator存在
			QRegularExpressionMatch matchedWord = matchedItr.next();
			setFormat(matchedWord.capturedStart(), matchedWord.capturedLength(), textCharFormat);
		}
	}
	/*多行注释高亮*/
	setCurrentBlockState(0);	
	int startIndex = 0;
	if (previousBlockState() != 1)
		startIndex = lit_mulCmtStart.indexIn(rowText);
	while (startIndex >= 0) {
		int endIndex = lit_mulCmtEnd.indexIn(rowText, startIndex);
		int commentLength;
		if (endIndex == -1) {
			setCurrentBlockState(1);
			commentLength = rowText.length() - startIndex;
		}
		else {
			commentLength = endIndex - startIndex
				+ lit_mulCmtEnd.matchedLength();
		}
		setFormat(startIndex, commentLength, textCharFormat);
		startIndex = lit_mulCmtStart.indexIn(rowText, startIndex + commentLength);
	}
}

/*smart*/
SmartEdit::SmartEdit(QTabWidget* parent)
	:QPlainTextEdit(parent)
	, syntaxLit(new SyntaxLit(this->document()))
	, rowNumArea(new RowNumArea(this))
	, keysCompleter(Q_NULLPTR)
{
	//this->setAcceptDrops(false);
	setContextMenuPolicy(Qt::NoContextMenu);
	setFont(QFont("微软雅黑", 12));
	setWordWrapMode(QTextOption::NoWrap);  //水平自适应滚动条
	rowNumArea->setFont(QFont("微软雅黑", 12, QFont::Bold));
	keysCompleter = new QCompleter(keys_cpp_blue + keys_cpp_purple + keys_cpp_normal);//不可改为new QCompleter(keys_cpp*, this)
	keysCompleter->setWidget(this);
	keysCompleter->setCaseSensitivity(Qt::CaseSensitive); //区分大小写
	keysCompleter->setCompletionMode(QCompleter::PopupCompletion);//匹配已输入内容,弹出
	keysCompleter->setMaxVisibleItems(8);
	keysCompleter->popup()->setFont(QFont("微软雅黑", 12, QFont::Bold));
	//加载qss
	loadStyleSheet(this, "smart.qss");

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
	smartCore(event->mimeData()->text());
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
	curTextCursor = textCursor();
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
	QTextCursor tempCursor = textCursor();
	while (tempCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1)) {
		selectedText = tempCursor.selectedText();//只要不遇到空格或者换行符，就获取选择文本
		if (selectedText.startsWith(' ') || selectedText.startsWith(QChar(0x422029))) {
			break;//左移至出现空格或者换行符时终止
		}
	}
	if (selectedText.startsWith(QChar(0x422029))) {
		selectedText.replace(0, 1, QChar(' '));//换行符则替换为空格
	}
	selectedText = selectedText.trimmed();
	if (keys_cpp_purple.contains(selectedText)
		|| keys_cpp_blue.contains(selectedText)
		|| keys_cpp_normal.contains(selectedText)) {
		return "";//全匹配则不需要补全，前缀归零
	}
	return selectedText;
}
/*智能补全*/
void SmartEdit::smartComplete(const QString& key) {
	QString curPrefix = getPrefix(), supplement = key;
	//不需要判断界限，因为参数一定是key
	if (curPrefix != "") { //前缀匹配关键字则获取还未输入的部分
		supplement = supplement.replace(supplement.indexOf(curPrefix), curPrefix.size(), "");
	}
	curTextCursor.insertText(supplement);
	smartCore(key);
}
/*智能核心*/
void SmartEdit::smartCore(QString key) {
	QString undefinedKey;
	int moveIndex = 0, moveStep = 0;
	if (keys_cpp_blue.contains(key)) {
		int index = keys_cpp_blue.indexOf(key);
		switch (index)
		{
		default:
			curTextCursor.insertText(" ");	break;
		case 24:case 25:case 26:case 27: break;/*true, false, this, nullptr*/
		case 30:/*sizeof*/
			curTextCursor.insertText(smarts.at(0));
			moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
			break;
		case 9:case 10:case 11:case 12:/*enum, union, struct, class*/
			curTextCursor.insertText(smarts.at(1) + ";");
			while (moveIndex < 3) {
				moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
				moveIndex++;
			}
			moveIndex = 0;
			moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
			undefinedKey = undefined.at(0);
			moveStep = undefinedKey.length();
			textCursor().insertText(" " + undefinedKey);
			while (moveIndex < moveStep) {
				moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
				moveIndex++;
			}
			break;
		}
	}
	moveIndex = 0; moveStep = 0;
	if (keys_cpp_purple.contains(key)) {
		int index = keys_cpp_purple.indexOf(key);
		switch (index)
		{
		default:	break;/*throw*/
		case 1:case 4:case 10:case 14:curTextCursor.insertText(" "); break;/*else, case, return, goto*/
		case 3:curTextCursor.insertText(": "); break;/*default*/
		case 5:case 6:curTextCursor.insertText("; "); break;/*continue, break*/
		case 0:case 9:case 12:/*if, while, catch*/
			curTextCursor.insertText(smarts.at(0));
			moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
			break;
		case 8:case 11:/*do, try*/
			curTextCursor.insertText(smarts.at(1));
			moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
			break;
		case 7:/*for*/
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
			moveStep = undefinedKey.length();
			textCursor().insertText(undefinedKey);
			while (moveIndex < moveStep) {
				moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
				moveIndex++;
			}
			break;
		case 2:/*switch*/
			curTextCursor.insertText(smarts.at(3));
			while (moveIndex < 3) {
				moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
				moveIndex++;
			}
			moveIndex = 0;
			moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
			moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
			undefinedKey = undefined.at(2);
			moveStep = undefinedKey.length();
			textCursor().insertText(undefinedKey);
			while (moveIndex < moveStep) {
				moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
				moveIndex++;
			}
			break;
		}
	}
	moveIndex = 0; moveStep = 0;
	if (keys_cpp_normal.contains(key)) {
		int index = keys_cpp_normal.indexOf(key);
		switch (index)
		{
		case 0:/*if_else*/
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
			moveStep = undefinedKey.length();
			textCursor().insertText(undefinedKey);
			while (moveIndex < moveStep) {
				moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
				moveIndex++;
			}
			break;
		case 1:/*do_while*/
			while (moveIndex < 6) {
				curTextCursor.deletePreviousChar();
				moveIndex++;
			}
			moveIndex = 0;
			curTextCursor.insertText(smarts.at(5));
			moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
			undefinedKey = undefined.at(1);
			moveStep = undefinedKey.length();
			textCursor().insertText(undefinedKey);
			while (moveIndex < moveStep) {
				moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
				moveIndex++;
			}
			break;
		case 2:/*try_catch*/
			while (moveIndex < 6) {
				curTextCursor.deletePreviousChar();
				moveIndex++;
			}
			moveIndex = 0;
			curTextCursor.insertText(smarts.at(6));
			while (moveIndex < 5) {
				moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
				moveIndex++;
			}
			moveIndex = 0;
			undefinedKey = undefined.at(1);
			moveStep = undefinedKey.length();
			textCursor().insertText(undefinedKey);
			while (moveIndex < moveStep) {
				moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
				moveIndex++;
			}
			break;
		default:	break;
		}
	}
	moveIndex = 0; moveStep = 0;
	if ("func" == key) {/*func*/
		curTextCursor.insertText(smarts.at(0) + smarts.at(1));
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
		moveIndex = 0;
		undefinedKey = undefined.at(3);
		moveStep = undefinedKey.length();
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
		while (moveIndex < moveStep) {
			moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
			moveIndex++;
		}
	}
}