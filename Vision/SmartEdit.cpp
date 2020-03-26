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
	, keysCompleter(NULL)
	, syntaxLit(new SyntaxLit(this->document()))
{
	init();//��ʼ��
	rowContentPlot();//��ʼ��ˢ���кſ�
	//�ۺ���
	connect(keysCompleter, SIGNAL(activated(QString)), this, SLOT(smartComplete(QString)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(rowContentPlot()));
}
SmartEdit::~SmartEdit() {
	delete keysCompleter;	keysCompleter = NULL;
	delete syntaxLit; syntaxLit = NULL;
	delete rowNumArea;	rowNumArea = NULL;
}
/*��ʼ��*/
void SmartEdit::init() {
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
	QFile file("./Resources/qss/smart.qss");
	file.open(QFile::ReadOnly);
	setStyleSheet(file.readAll());
	file.close();
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
				//keysCompleter->completionModel()
				//����qss
				QFile file("./Resources/qss/completer.qss");
				file.open(QFile::ReadOnly);
				keysCompleter->popup()->setStyleSheet(file.readAll());
				file.close();
			}
		}
	}
}
/*�ͼ��¼�*/
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
			rowNumArea->update();//ÿ�λ��������ˢ��
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
	//initPrefix();
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
		if (selectedText.startsWith(QString(" ")) || selectedText.startsWith(QChar(0x422029))) {
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
	QString curPrefix = getPrefix(),
		supplement = key;
	curTextCursor = textCursor();
	if (!key.contains(curPrefix)) { //ǰ׺��ƥ��ؼ�����ȡ��ѡ��
		curTextCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, curPrefix.size());
		curTextCursor.clearSelection();
	}
	else { //ǰ׺ƥ��ؼ������ȡ��δ����Ĳ���
		supplement = supplement.replace(supplement.indexOf(curPrefix), curPrefix.size(), "");
	}
	//����Ҫ�жϽ��ޣ���Ϊ����һ����key
	int index = keys.indexOf(key);
	//���������ܴ������
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
	case 15:case 16:/*if, while*/
		curTextCursor.insertText(supplement + "()");
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
	case 19:/*do*/
		curTextCursor.insertText(supplement + "\n{\n\n}while();");
		moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);	break;
	}
	textCursor().insertText(" ");
}
