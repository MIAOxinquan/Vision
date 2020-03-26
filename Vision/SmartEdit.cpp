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
		else if (index <= 18) { return QColor(160, 32, 240); }
	}
	return Qt::black;
}

/*smart*/
SmartEdit::SmartEdit(QTabWidget* parent)
	:QPlainTextEdit(parent)
	, rowNumArea(new RowNumArea(this))
	, keysCompleter(NULL)
	, syntaxLit(new SyntaxLit(this->document()))
{
	init();//初始化
	rowContentPlot();//初始化刷新行号块
	//槽函数
	connect(keysCompleter, SIGNAL(activated(QString)), this, SLOT(smartComplete(QString)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(rowContentPlot()));

}
SmartEdit::~SmartEdit() {
	delete keysCompleter;	keysCompleter = NULL;
	delete syntaxLit; syntaxLit = NULL;
	delete rowNumArea;	rowNumArea = NULL;
}
/*初始化*/
void SmartEdit::init() {
	rowNumArea->setFont(QFont("微软雅黑", 12, QFont::Bold));
	setFont(QFont("微软雅黑", 12));
	setWordWrapMode(QTextOption::NoWrap);  //水平自适应滚动条
	keysCompleter = new QCompleter(keys);//不可更改为new QCompleter(keys, this)
	keysCompleter->setWidget(this);
	keysCompleter->setCaseSensitivity(Qt::CaseSensitive); 
	keysCompleter->setCompletionMode(QCompleter::PopupCompletion);//匹配已输入内容,弹出
	keysCompleter->popup()->setFont(QFont("微软雅黑", 12));
	keysCompleter->setMaxVisibleItems(5);
	//加载qss
	QFile file("./Resources/qss/smart.qss");
	file.open(QFile::ReadOnly);
	setStyleSheet(file.readAll());
	file.close();
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
						/*cursorRect()并未随rowContent与rowNum兼容宽度，因此左界加上行号块宽，使之看起来更舒适*/
						, curTextCursorRect.top(), 150, fontMetrics().height()));
			}
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
int SmartEdit::getRowNumWidth() {
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
	//initPrefix();
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
		if (selectedText.startsWith(QString(" ")) || selectedText.startsWith(QChar(0x422029))) {
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
	QString curPrefix = getPrefix(),
		supplement = key;
	curTextCursor = textCursor();
	if (!key.contains(curPrefix)) { //前缀不匹配关键字则取消选择
		curTextCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, curPrefix.size());
		curTextCursor.clearSelection();
	}
	else { //前缀匹配关键字则获取还未输入的部分
		supplement = supplement.replace(supplement.indexOf(curPrefix), curPrefix.size(), "");
	}
	//不需要判断界限，因为参数一定是key
	int index = keys.indexOf(key);
	//以下是智能处理核心
	switch (index)
	{
	default:/*0-5,8-11,void, int, float, double, char, string, return, const, case, else*/
		curTextCursor.insertText(supplement); break;
	case 6:case 7:/*class, struct*/
		curTextCursor.insertText(supplement + "\n{\n\n};");
		for (int i = 0; i < 3; i++) {
			moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
		}
		moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
		break;
	case 12:/*default*/
		curTextCursor.insertText(supplement + ":"); break;
	case 13:case 14:/*continue,break*/
		curTextCursor.insertText(supplement + ";"); break;
	case 15:/*if*/
		curTextCursor.insertText(supplement + "()");
		moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
		break;
	case 16:/*while*/
		curTextCursor.insertText(supplement + "()\n{\n\n}");
		for (int i = 0; i < 3; i++) {
			moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
		}
		moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
		moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
		break;
	case 17:/*for*/
		curTextCursor.insertText(supplement + "(;;)\n{\n\n}");
		for (int i = 0; i < 3; i++) {
			moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
		}
		moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
		for (int i = 0; i < 3; i++) {
			moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
		}
		break;
	case 18:/*switch*/
		curTextCursor.insertText(supplement + "()\n{\ndefault: break;\n}");
		for (int i = 0; i < 3; i++) {
			moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
		}
		moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
		moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
		break;
	}
	textCursor().insertText(" ");
}
