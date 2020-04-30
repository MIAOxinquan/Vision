#include "global.h"
#include "SmartEdit.h"
#include "PlotPad.h"
//#include<algorithm>
/*������*/
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
�﷨����
�麯����ͬ����������ʵ�֣����������ɸ���
*/
/*
globalMatch(blockText)�����ǣ����ı�QString : rowText����ƥ���κη���������ʽ���ֶ�
����һ�����֡�intint int func 123 111 int��������ʽ������int��
��ô��ƥ�䵽�ĸ���int������ʹǰ������int������
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
		while (matchedItr.hasNext()) {//ֻҪkeyIterator����
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
	/*����ע��*/
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
	/*�ڵ�*/
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
{
	this->setAcceptDrops(false);
	setContextMenuPolicy(Qt::NoContextMenu);
	setFont(QFont("΢���ź�", 12));
	setWordWrapMode(QTextOption::NoWrap);  //ˮƽ����Ӧ������
	rowNumArea->setFont(QFont("΢���ź�", 12, QFont::Bold));
	keysCompleter = new QCompleter(keys_cpp_blue + keys_cpp_purple + keys_cpp_normal);//���ɸ�Ϊnew QCompleter(keys_cpp*, this)
	keysCompleter->setWidget(this);
	keysCompleter->setCaseSensitivity(Qt::CaseSensitive); //���ִ�Сд
	keysCompleter->setCompletionMode(QCompleter::PopupCompletion);//ƥ������������,����
	keysCompleter->setMaxVisibleItems(8);
	keysCompleter->popup()->setFont(QFont("΢���ź�", 12, QFont::Bold));
	//����qss
	loadStyleSheet(this, "edit.qss");

	rowContentPlot();//��ʼ��ˢ���кſ�
	//�ۺ���
	connect(keysCompleter, SIGNAL(activated(QString)), this, SLOT(smartComplete(QString)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(rowContentPlot()));
	connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(update(QRect, int)));
}
SmartEdit::~SmartEdit() {
	delete keysCompleter;	keysCompleter = Q_NULLPTR;
	delete syntaxLit; syntaxLit = Q_NULLPTR;
	delete rowNumArea;	rowNumArea = Q_NULLPTR;
}
/*��д��ˢ�´�С*/
void SmartEdit::resizeEvent(QResizeEvent* event){
	QRect rct = contentsRect();
	rowNumArea->setGeometry(QRect(rct.left(), rct.top(), getRowNumWidth(), rct.height()));
}
/*�����¼�*/
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
		QPlainTextEdit::keyPressEvent(event);//�̳��¼����Կ���һ�ֿ��أ��൱�ڸ���event���˴�λ�ò��ɵ�ǰ
		curPrefix = getPrefix();
		keysCompleter->setCompletionPrefix(curPrefix); // ����ǰ׺��ʹCompleterѰ�ҹؼ���
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
				//����qss
				loadStyleSheet(keysCompleter->popup(), "completer.qss");
			}
		}
	}
}
/*�ɼ��¼�*/
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
/*�ͷ��¼�*/
/*
drag = mousePress+mouseMove
drop=mouseMove+mouseRelease
*/
void SmartEdit::dropEvent(QDropEvent* event) {
	setFocus();
	QPlainTextEdit::dropEvent(event);
	smartCore(event->mimeData()->text());
}
/*�кſ��*/
int SmartEdit::getRowNumWidth() {
	int digits = 4;//��ʼλ��
	int max = qMax(1, blockCount());//��ȡ���λ����Ĭ������1λ
	while (max >= 1000) {//3λ�����ȹ̶���4λ������������������
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
			//rowNumArea->update();//ÿ�λ��������ˢ��,��������CPU
		}
		block = block.next();
		top = bottom;
		bottom = top + height;
		blockNumber++;
	}
}
/*�ı�����ͼ*/
void SmartEdit::rowContentPlot(/*int*/) {
	//�����ı���ȣ�ʹ֮���к�������
	setViewportMargins(getRowNumWidth() - 3, -1, -3, 0);
	curTextCursor = textCursor();
	/*  �ڵ�����ʶ����  */

	//��ǰ�и���
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
/*  �ڵ�����ʶ����  */
/*���ظ��ڵ�����ݣ������ӽڵ�����ֻ����id*/
QString SmartEdit::getParentNodeContent()
{
	QString str = this->toPlainText();
	QStringList childContents = getChildNodeContent();
	int num = childContents.count();
	for (int i = 0; i < num; i++) {
		str.replace("<@" + childContents[i] + "@>", " ");
	}
	this->setPlainText(str);	//for test
	return str;
}

/*  ɨ����벢���������ӽڵ�����  */
QStringList SmartEdit::getChildNodeContent()
{
	QStringList nodesContents;
	QString content = this->toPlainText();
	QString str;
	if (!content.isEmpty()) {
		QStringList splist1 = content.split("#");	//��һ�ΰ�#�ָ�,1-maxԪ�ر�Ϊ�ӽڵ�����+���ӽڵ�����
		int count1 = splist1.count();
		if (count1 > 1) {
			QStringList splist2;
			QStringList splist3;
			QRegExp rx("\\d+");
			for (int i = 1; i < count1; i++) {
				splist2 = splist1.at(i).split("<@");	//�ڶ��ΰ�<@�ָ����Ϊ0Ԫ����id��1Ԫ�����ӽڵ�����
				int count2 = splist2.count();
				if (count2 == 1) {
					nodesContents.append("BROKEN!");	//ֻ��һ��Ԫ��˵���û�ɾ���˱�ͷ<@���ڵ㱻�ƻ�
					continue;
				}
				else if (count2 > 2) {	//�ж����ͷ
					if (rx.exactMatch(splist2.at(0))) {		//ֻ�е�0��Ԫ�ؼ�#�͵�һ��<@֮�������ֽڵ���п��ܱ��ֲ����ƻ�
						str = splist2.at(1);
						for (int j = 2; j < count2; j++) {
							str = str + "<@" + splist2.at(j);
						}
						splist3 = str.split("@>");	//�����ΰ�@>�ָ�
						if (splist3.count() == 1) {		//˵���ýڵ�ı�β���ƻ�
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
						nodesContents.append(splist3.at(0));	//�����м���@>,����0��Ԫ����Ϊ�ڵ�����
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

/*���ؽڵ�����*/
void SmartEdit::showContent(Block* block)
{
	QString cont = block->content;
	//QString global;
	int index = cont.indexOf("#", 0);
	int n = 0;
	QRegExp rx("\\d");
	Block* bl;
	//QStringList idInContent;
	while (index != -1)	//ÿ��ѭ���Ȼ�ȡ#��������֣��������Ӧ�ӽڵ��id������ӽڵ�content����id֮��
	{
		int i = index;
		QString id = "";
		while (rx.exactMatch(cont.at(i))) {	//��ȡid
			id += cont.at(i);
			i++;
		}
		//idInContent.append(id);
		bl = block->childrenBlock->at(n);
		if (id == bl->id)	//�ж��Ƿ�Ϊ�ӽڵ�id
		{
			if (bl->childrenBlock->count() != 0) {
				QString str = bl->content;
				str.replace("#", "$");
				cont.insert(i, "<@\n" + str + "\n@>\n");
			}
		}
		n++;
		index = cont.indexOf("#", index + 1);
	}
	this->setPlainText(cont);
	//return global;
}

/*����ȫ�ִ���*/
void SmartEdit::showContent(PlotPad* plot)
{
	QString cont = "";
	if (plot->root) {
		cont = getContent(plot->root);	//�ȴ�����ڵ�
		ArrowLine* arr = plot->root->outArrow;
		Block* block = Q_NULLPTR;
		while (arr != Q_NULLPTR) {	//������ڵ�ĺ�̽ڵ�
			block = arr->toBlock;
			cont += getContent(block);
			arr = block->outArrow;
		}
	}
	this->setPlainText(cont);
}

/*ȫ����ʾʱ�������ڵ����ݣ�������������*/
QString SmartEdit::getContent(Block* block)
{
	QString cont = block->content;
	int index = cont.indexOf("#", 0);
	int n = 0;
	QRegExp rx("\\d");
	Block* bl;
	while (index != -1)
	{
		int i = index + 1;
		QString id = "";
		while (rx.exactMatch(cont.at(i))) {	//��ȡid
			id += cont.at(i);
			i++;
		}
		bl = block->childrenBlock->at(n);
		if (id == bl->id)	//�ж��Ƿ�Ϊ�ӽڵ�id
		{
			cont.remove(index, i - 1);
			cont.insert(index - 1, bl->content);
		}
		n++;
		index = cont.indexOf("#", index + 1);
	}
	for (int j = 0; j < block->childrenBlock->count(); j++) {
		if (!block->childrenBlock->at(j)->childrenBlock->isEmpty())
			cont = this->getContent(block->childrenBlock->at(j));	//�ݹ鴦������ڵ�
	}
	return cont;
}
/*
ˢ�½��棬ֻ��updateRequest�������paintEvent��󽵵�cpu����
*/
void SmartEdit::update(const QRect& rect, int dy) {
	if (dy) rowNumArea->scroll(0, dy);
	else  rowNumArea->update(0, rect.y(), rowNumArea->width(), rect.height());
	if (rect.contains(viewport()->rect())) setViewportMargins(getRowNumWidth() - 3, -1, -3, 0);
}
/*��ȡǰ׺����괦����һ���ո�֮����ַ���*/
QString SmartEdit::getPrefix() const {
	QString selectedText;
	/*
	��ȡtextCursor�ĸ�����ģ�����ƻ�ȡѡ���ı�����˲���Ӱ�쵽ʵ�ʹ���λ��
	������textCursor()ֱ�Ӳ����������±���
	*/
	QTextCursor tempCursor = textCursor();
	while (tempCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1)) {
		selectedText = tempCursor.selectedText();//ֻҪ�������ո���߻��з����ͻ�ȡѡ���ı�
		if (selectedText.startsWith(' ') || selectedText.startsWith(QChar(0x422029))) {
			break;//���������ֿո���߻��з�ʱ��ֹ
		}
	}
	if (selectedText.startsWith(QChar(0x422029))) {
		selectedText.replace(0, 1, QChar(' '));//���з����滻Ϊ�ո�
	}
	selectedText = selectedText.trimmed();
	if (keys_cpp_purple.contains(selectedText)
		|| keys_cpp_blue.contains(selectedText)
		|| keys_cpp_normal.contains(selectedText)) {
		return "";//ȫƥ������Ҫ��ȫ��ǰ׺����
	}
	return selectedText;
}
/*���ܲ�ȫ*/
void SmartEdit::smartComplete(const QString& key) {
	QString curPrefix = getPrefix(), supplement = key;
	//����Ҫ�жϽ��ޣ���Ϊ����һ����key
	if (curPrefix != "") { //ǰ׺ƥ��ؼ������ȡ��δ����Ĳ���
		supplement = supplement.replace(supplement.indexOf(curPrefix), curPrefix.size(), "");
	}
	curTextCursor.insertText(supplement);
	smartCore(key);
}
/*���ܺ���*/
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