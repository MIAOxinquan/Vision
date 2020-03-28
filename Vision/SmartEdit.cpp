#include "global.h"
#include "SmartEdit.h"

/*������*/
SyntaxLit::SyntaxLit(QTextDocument* document)
	:QSyntaxHighlighter(document) { 
	QString keysPattern;
	for (int i = 0; i < keys.count(); i++) {
		keysPattern.append(keys.at(i) + "|");
	}
	keysRegExp.setPattern(keysPattern.trimmed());
}
/*
�﷨����
�麯����ͬ����������ʵ�֣����������ɸ���
*/
void SyntaxLit::highlightBlock(const QString& rowText) {
	QTextCharFormat textCharFormat;
	textCharFormat.setFontWeight(QFont::Bold);
	/*
	globalMatch(rowText)�����ǣ���ÿ���ı�QString : rowText����ƥ���κη���������ʽ���ֶ�
	����һ�����֡�intint int func 123 111 int��������ʽ������int��
	��ô��ƥ�䵽�ĸ���int������ʹǰ������int������
	*/
	QRegularExpressionMatchIterator matchedItr = keysRegExp.globalMatch(rowText);
	while (matchedItr.hasNext()) {//ֻҪkeyIterator����
		QRegularExpressionMatch matchedWord = matchedItr.next();
		textCharFormat.setForeground(QBrush(getKeyColor(matchedWord.captured())));
		setFormat(matchedWord.capturedStart(), matchedWord.capturedLength(), textCharFormat);
	}
}
/*��ɫת��*/
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
	init();//��ʼ��
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
/*��ʼ��*/
void SmartEdit::init() {
	//this->setAcceptDrops(false);
	keysCompleter = new QCompleter(keys);//���ɸ�Ϊnew QCompleter(keys, this)
	keysCompleter->setWidget(this);
	keysCompleter->setCaseSensitivity(Qt::CaseSensitive); //���ִ�Сд
	keysCompleter->setCompletionMode(QCompleter::PopupCompletion);//ƥ������������,����
	keysCompleter->setMaxVisibleItems(5);
	keysCompleter->popup()->setFont(QFont("΢���ź�", 12, QFont::Bold));
	rowNumArea->setFont(QFont("΢���ź�", 12, QFont::Bold));
	setFont(QFont("΢���ź�", 12));
	setWordWrapMode(QTextOption::NoWrap);  //ˮƽ����Ӧ������
	//����qss
	loadStyleSheet(this, "smart.qss");
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
	moveCursor(QTextCursor::Left, QTextCursor::KeepAnchor);
	curTextCursor = textCursor();
	int index = curTextCursor.selectedText().toInt();
	curTextCursor.removeSelectedText();
	curTextCursor.insertText(toolSmarts.at(index));
	smartDrop(index);
}
/*��ק�Զ���ȫ*/
void SmartEdit::smartDrop(int index) {
	/*���ƶ����*/
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
	���������ı�
	�����ƶ�Ҫ���¹�긱��
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
	//��ǰ�и���
	if (!isReadOnly()) {
		QList<QTextEdit::ExtraSelection> extraSelections;
		QTextEdit::ExtraSelection selection;
		selection.format.setBackground(QColor(Qt::cyan).lighter(192));//���õ�ǰ�б���ɫ
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
		setExtraSelections(extraSelections);
	}
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
	QTextCursor curTextCursor = textCursor();
	while (curTextCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1)) {
		selectedText = curTextCursor.selectedText();//ֻҪ�������ո���߻��з����ͻ�ȡѡ���ı�
		if (selectedText.startsWith(' ') || selectedText.startsWith(QChar(0x422029))) {
			break;//���������ֿո���߻��з�ʱ��ֹ
		}
	}
	if (selectedText.startsWith(QChar(0x422029))) {
		selectedText.replace(0, 1, QChar(' '));//���з����滻Ϊ�ո�
	}
	if (keys.contains(selectedText.trimmed())) { 
		return "";//ȫƥ������Ҫ��ȫ��ǰ׺���ÿ�
	}
	return selectedText.trimmed();
}
/*���ܲ�ȫ*/
void SmartEdit::smartComplete(const QString& key) {
	QString curPrefix = getPrefix(), supplement = key, undefinedKey;
	int moveIndex = 0, moveLength = 0;
	curTextCursor = textCursor();
	//����Ҫ�жϽ��ޣ���Ϊ����һ����key
	int index = keys.indexOf(key);
	if (!key.contains(curPrefix)) { //ǰ׺��ƥ��ؼ�����ȡ��ѡ��
		curTextCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, curPrefix.size());
		curTextCursor.clearSelection();
	}
	else { //ǰ׺ƥ��ؼ������ȡ��δ����Ĳ���
		supplement = supplement.replace(supplement.indexOf(curPrefix), curPrefix.size(), "");
	}
	curTextCursor.insertText(supplement);
	//���������ܴ������
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