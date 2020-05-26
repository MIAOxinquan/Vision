#include "global.h"
#include "SmartEdit.h"
#include "PlotPad.h"
class Item;
//#include<algorithm>
/*高亮器*/
SyntaxLit::SyntaxLit(QTextDocument* document)
	:QSyntaxHighlighter(document)
{
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
				+ key + "\\w{1,2,3}|"
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
	lit_sigCmt.setPattern(sigCmt);
	lit_mulCmtStart.setPattern( mulCmtStart);
	lit_mulCmtEnd.setPattern(mulCmtEnd);
	lit_nodeStart.setPattern(nodeStart);
	lit_nodeEnd.setPattern(nodeEnd);
}
/*
语法高亮
虚函数，同名函数必须实现，函数名不可更改
*/
/*
globalMatch(blockText)作用是，在文本QString : rowText遍历匹配任何符合正则表达式的字段
比如一段文字“intint int func 123 111 int“，正则式包含”int“
那么将匹配到四个”int“，即使前两个”int“相连
*/
void SyntaxLit::highlightBlock(const QString &blockText) {
	QTextCharFormat litFormat;
	QRegularExpressionMatchIterator matchedItr;
	for (int i = 0; i < lits.count(); i++) {
		matchedItr = lits.at(i).globalMatch(blockText);
		litFormat.setForeground(colors.at(i));
		if (2 == i) {
			litFormat.setFontWeight(QFont::Normal);
		}
		else {
			litFormat.setFontWeight(QFont::Bold);
		}
		while (matchedItr.hasNext()) {//只要keyIterator存在
			QRegularExpressionMatch matchedWord = matchedItr.next();
			setFormat(matchedWord.capturedStart(), matchedWord.capturedLength(), litFormat);
		}
	}
	litFormat.setForeground(Qt::darkGreen);
	setCurrentBlockState(0);
	int mulCmtStartIndex = 0, nodeStartIndex = 0, nextSig = 0
		, sigCmtIndex = lit_sigCmt.indexIn(blockText);
	if (previousBlockState() != 1) 	mulCmtStartIndex = lit_mulCmtStart.indexIn(blockText);
	if (previousBlockState() != 2) 	nodeStartIndex = lit_nodeStart.indexIn(blockText);
	if (sigCmtIndex >= 0) {
		if (sigCmtIndex < mulCmtStartIndex)mulCmtStartIndex = -1;
		if (sigCmtIndex < nodeStartIndex)nodeStartIndex = -1;
	}
	/*qDebug() << "<Out_Before>Sig:" << sigCmtIndex
		<< "/MulStart:" << mulCmtStartIndex
		<< "/NodeStart:" << nodeStartIndex
		<< "/NextSig" << nextSig;*/
	/*多行注释*/
	while (mulCmtStartIndex >= 0) {
		int nextStartIndex = 0
			, rowMulCmtStartMark = lit_mulCmtStart.indexIn(blockText)
			, mulCmtEndIndex = lit_mulCmtEnd.indexIn(blockText
				, (-1 == rowMulCmtStartMark) ? mulCmtStartIndex : mulCmtStartIndex + 2);
		if (-1 == mulCmtEndIndex) {
			setCurrentBlockState(1);
			nextStartIndex = blockText.length();
			sigCmtIndex = -1;
		}
		else {
			nextStartIndex = mulCmtEndIndex + lit_mulCmtEnd.matchedLength(); 
		}
		if (mulCmtStartIndex > 0) {
			sigCmtIndex = lit_sigCmt.indexIn(blockText);
		}
		if (sigCmtIndex >= 0) {
			if (sigCmtIndex >= mulCmtEndIndex + 2 && sigCmtIndex < mulCmtStartIndex) {
				setFormat(sigCmtIndex, lit_sigCmt.matchedLength(), litFormat);
				mulCmtStartIndex = -1;
			}
		}
		/*qDebug() << "<InMul_After>MulStart:" << mulCmtStartIndex
			<< "/Sig:" << sigCmtIndex
			<< "/MulEnd:" << mulCmtEndIndex
			<< "/NextMulStart:" << nextStartIndex;*/
		nextSig = nextStartIndex;
		int length = nextStartIndex - mulCmtStartIndex;
		setFormat(mulCmtStartIndex, length, litFormat);
		mulCmtStartIndex = lit_mulCmtStart.indexIn(blockText,  nextStartIndex);
	}
	/*节点*/
	while (nodeStartIndex >= 0) {
		int nextStartIndex = 0
			, rowSigMark = lit_sigCmt.indexIn(blockText)
			, rowNodeStartMark = lit_nodeStart.indexIn(blockText)
			, nodeEndIndex = lit_nodeEnd.indexIn(blockText
				, (-1 == rowNodeStartMark) ? nodeStartIndex : nodeStartIndex + 2);
		if (-1 == nodeEndIndex) {
			setCurrentBlockState(2);
			nextStartIndex = blockText.length();
			sigCmtIndex = -1;
		}
		else {
			nextStartIndex = nodeEndIndex + lit_nodeEnd.matchedLength(); 
			if (nodeStartIndex > 0)
				sigCmtIndex = lit_sigCmt.indexIn(blockText, nodeStartIndex);
			if (sigCmtIndex >= 0) {
				if (sigCmtIndex > nodeEndIndex + 2 && sigCmtIndex < nodeStartIndex
					&& sigCmtIndex < mulCmtStartIndex) {
					litFormat.setForeground(Qt::darkGreen);
					setFormat(sigCmtIndex, lit_sigCmt.matchedLength(), litFormat);
				}
			}
		}
		nextSig = nextStartIndex;
		int length = nextStartIndex - nodeStartIndex;
		litFormat.setForeground(Qt::darkGray);
		setFormat(nodeStartIndex, length, litFormat);
		nodeStartIndex = lit_nodeStart.indexIn(blockText, nextStartIndex);
	}
	sigCmtIndex = lit_sigCmt.indexIn(blockText, nextSig);
	if (sigCmtIndex >= nextSig && -1 == mulCmtStartIndex && -1 == nodeStartIndex) {
		litFormat.setForeground(Qt::darkGreen);
		setFormat(sigCmtIndex, lit_sigCmt.matchedLength(), litFormat);
	}
	/*qDebug() << "<Out_After>Sig:" << sigCmtIndex
		<< "/MulStart:" << mulCmtStartIndex
		<< "/NodeStart:" << nodeStartIndex
		<< "/NextSig:" << nextSig;*/
}
/*edit*/
SmartEdit::SmartEdit(QTabWidget* parent)
	:QPlainTextEdit(parent)
	, syntaxLit(new SyntaxLit(this->document()))
	, rowNumArea(new RowNumArea(this))
	, keysCompleter(Q_NULLPTR)
	, pad(Q_NULLPTR)
{
	this->setAcceptDrops(false);
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
	loadStyleSheet(this, "edit.qss");

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
	/*  节点内容识别函数  */
	//当前行高亮
	if (!isReadOnly()) {
		QList<QTextEdit::ExtraSelection> extraSelections;
		QTextEdit::ExtraSelection selection;
		selection.format.setBackground(QColor(Qt::cyan).lighter(180));//设置当前行背景色
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
		setExtraSelections(extraSelections);
	}
	if (pad && pad->focusedBlock) {
		qDebug() << "focused";
		pad->focusedBlock->content = getParentNodeContent();
	}
}
/*  节点内容识别函数  */
/*返回父节点的内容，其中子节点内容只保留id*/
QString SmartEdit::getParentNodeContent()
{
	//stringList
	QString str = this->toPlainText();
	QStringList childContents = getChildNodeContent();
	int num = childContents.count();
	for (int i = 0; i < num; i++) {
		//stringList.append(childContents[i]);
		str.replace("<@" + childContents[i] + "@>", "");
	}
	//stringList.append(str);
	return str;
}

/*  扫描代码并返回所有子节点内容  */
QStringList SmartEdit::getChildNodeContent()
{
	QStringList nodesContents;
	QString content = this->toPlainText();
	QString str;
	if (!content.isEmpty()) {
		QStringList splist1 = content.split("#");	//第一次按#分割,1-max元素必为子节点内容+非子节点内容
		int count1 = splist1.count();
		if (count1 > 1) {
			QStringList splist2;
			QStringList splist3;
			QRegExp rx("\\d+");
			for (int i = 1; i < count1; i++) {
				splist2 = splist1.at(i).split("<@");	//第二次按<@分割，理想为0元素是id，1元素是子节点内容
				int count2 = splist2.count();
				if (count2 == 1) {
					nodesContents.append("BROKEN!");	//只有一个元素说明用户删除了标头<@，节点被破坏
					continue;
				}
				else if (count2 > 2) {	//有多个标头
					if (rx.exactMatch(splist2.at(0))) {		//只有第0个元素即#和第一个<@之间是数字节点才有可能保持不被破坏
						str = splist2.at(1);
						for (int j = 2; j < count2; j++) {
							str = str + "<@" + splist2.at(j);
						}
						splist3 = str.split("@>");	//第三次按@>分割
						if (splist3.count() == 1) {		//说明该节点的标尾被破坏
							nodesContents.append("BROKEN!");
							continue;
						}
						else {
							str = splist3.at(0);
							nodesContents.append(str);
						}
					}
					else {
						nodesContents.append("BROKEN!");
						continue;
					}
				}
				else if (count2 == 2) {
					if (rx.exactMatch(splist2.at(0))) {
						str = splist2.at(1);
						splist3 = str.split("@>");
						nodesContents.append(splist3.at(0));	//不管有几个@>,都以0号元素作为节点内容
					}
					else {
						nodesContents.append("BROKEN!");
						continue;
					}
				}
				else {
					nodesContents.append("BROKEN!");
					continue;
				}
			}
		}
	}
	return nodesContents;
}

/*加载节点内容*/
void SmartEdit::showContent(Block* block)
{
	QString cont = block->content;
	//QString global;
	int index = cont.indexOf("#", 0);
	int n = 0;
	QRegExp rx("\\d");
	Block* bl;
	//QStringList idInContent;
	while (index != -1)	//每次循环先获取#后面的数字，若其与对应子节点的id相等则将子节点content插入id之后
	{
		int i = index + 1;
		QString id = "";
		while (rx.exactMatch(cont.at(i))) {	//获取id
			id += cont.at(i);
			i++;
		}
		//idInContent.append(id);
		bl = block->childrenBlock->at(n);
		if (id.toInt() == bl->id)	//判断是否为子节点id
		{
			QString str = bl->content;
			if (bl->childrenBlock->count() != 0) {				
				str.replace("#", "$");				
			}
			cont.insert(i, "<@\n" + str + "\n@>");
		}
		n++;
		index = cont.indexOf("#", index + 1);
	}
	this->setPlainText(cont);
	//return global;
}

/*加载全局代码*/
QString SmartEdit::showContent(PlotPad* plot)
{
	QString cont = "";
	if (plot->root) {
		cont = getContent(plot->root);	//先处理根节点
		ArrowLine* arr = plot->root->outArrow;
		Block* block = Q_NULLPTR;
		while (arr != Q_NULLPTR) {	//处理根节点的后继节点
			block = arr->toBlock;
			cont += "\n" + getContent(block);
			arr = block->outArrow;
		}
	}
	//this->setPlainText(cont);
	return cont;
}

/*全局显示时处理单个节点内容，返回正常代码*/
QString SmartEdit::getContent(Block* block)
{
	if (!block)return "";
	QString cont = block->content;
	if (block->childrenBlock->count() > 0) {
		int index = cont.indexOf("#", 0);
		int n = 0;
		QRegExp rx("\\d");
		Block* bl;
		while (index != -1)
		{
			QString id = "";
			int i = index + 1;
			while (rx.exactMatch(cont.at(i))) {	//获取id
				id += cont.at(i);
				i++;
			}
			bl = block->childrenBlock->at(n);
			if (id.toInt() == bl->id)	//判断是否为子节点id
			{
				cont.remove(index, id.length() + 2);
				cont.insert(index - 1, bl->content);
			}
			n++;
			index = cont.indexOf("#", index + bl->content.length());
		}
		for (int j = 0; j < block->childrenBlock->count(); j++) {//递归处理子孙节点
			if (!block->childrenBlock->at(j)->childrenBlock->isEmpty()) {
				QString subCont = this->getContent(block->childrenBlock->at(j));
				cont.replace(block->childrenBlock->at(j)->content, subCont);
			}
		}
	}
	return cont;
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
QString SmartEdit::smartCore(QString key) {
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
	QString retSmart = "";
	if (toolKeys.contains(key)) {
		retSmart = key;
		int index = toolKeys.indexOf(key);
		switch (index)
		{
		case 1:case 2:case 3:case 4:/*enum union struct class*/
			retSmart += " "+undefined.at(0) + smarts.at(1) + ";"; break;
		case 5:case 9:/*if while*/
			retSmart += smarts.at(0); break;
		case 6:/*if_else*/
			retSmart = "if(Statement)\n{\n\n}\nelse\n{\n\n}"; break;
		case 7:/*switch*/
			retSmart += smarts.at(3); break;
		case 8:/*for*/
			retSmart += "(Statement;;)\n{\n\n}"; break;
		case 10:/*do_while*/
			retSmart = "do\n{\nStatement\n}while();" ; break;
		case 11:/*try_catch*/
			retSmart += smarts.at(6); break;
		case 12:/*func*/
			retSmart = "Type func()\n{\n\n}"; break;
		default:	 break;
		}
	}
	return retSmart;
}