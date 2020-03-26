#include "SmartEdit.h"

/*������*/
SyntaxLit::SyntaxLit(QTextDocument* document)
	:QSyntaxHighlighter(document) {
	QString keyWordPattern;
	for (int i = 0; i < keyWords.count(); i++) {
		keyWordPattern.append(keyWords.at(i) + "|");
	}
	litRegExp.setPattern(keyWordPattern.trimmed());
}
/*
�﷨����
�麯����ͬ����������ʵ�֣����������ɸ���
*/
void SyntaxLit::highlightBlock(const QString& text) {
	QTextCharFormat hTextCharFormat;
	hTextCharFormat.setFontWeight(QFont::Bold);
	QRegularExpressionMatchIterator keyWordIterator = litRegExp.globalMatch(text);
	while (keyWordIterator.hasNext()) {
		QRegularExpressionMatch matchedWord = keyWordIterator.next();
		hTextCharFormat.setForeground(QBrush(QColor(0, 0, 255)));//160, 32, 240
		setFormat(matchedWord.capturedStart(), matchedWord.capturedLength(), hTextCharFormat);
	}
}

/*smart*/
SmartEdit::SmartEdit(QTabWidget* parent)
	:QPlainTextEdit(parent)
	, rowNumArea(new RowNumArea(this))
	, kWordsCompleter(NULL)
	, syntaxLit(new SyntaxLit(this->document()))
{
	init();//��ʼ��
	rowContentPlot();//��ʼ��ˢ���кſ�
	//�ۺ���
	connect(kWordsCompleter, SIGNAL(activated(QString)), this, SLOT(completeKWord(QString)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(rowContentPlot()));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updatePrefix()));

}
SmartEdit::~SmartEdit() {
	delete rowNumArea;	rowNumArea = NULL;
	delete kWordsCompleter;	kWordsCompleter = NULL;
}
/*��ʼ��*/
void SmartEdit::init() {
	rowNumArea->setFont(QFont("΢���ź�", 12, QFont::Bold));
	setFont(QFont("΢���ź�", 12));
	setWordWrapMode(QTextOption::NoWrap);  //ˮƽ����Ӧ������
	kWordsCompleter = new QCompleter(keyWords);
	kWordsCompleter->setWidget(this);
	kWordsCompleter->setCaseSensitivity(Qt::CaseSensitive); 
	kWordsCompleter->setCompletionMode(QCompleter::PopupCompletion);//ƥ������������,����
	kWordsCompleter->popup()->setFont(QFont("΢���ź�", 12));
	kWordsCompleter->setMaxVisibleItems(5);
	//����qss
	QFile file("./Resources/qss/smart.qss");
	file.open(QFile::ReadOnly);
	setStyleSheet(file.readAll());
	file.close();
}
/*��д��ˢ�´�С*/
void SmartEdit::resizeEvent(QResizeEvent* event){
	QRect rct = contentsRect();
	rowNumArea->setGeometry(QRect(rct.left(), rct.top(), rowNumWidth(), rct.height()));
}
/*�����¼�*/
void SmartEdit::keyPressEvent(QKeyEvent* event) {
	QPlainTextEdit::keyPressEvent(event);
	if (kWordsCompleter) {
		if (kWordsCompleter->popup()->isVisible()) {
			switch (event->key()) {
			case Qt::Key_Enter:
			case Qt::Key_Escape:
			case Qt::Key_Return:
			case Qt::Key_Tab:
				event->ignore(); return;
			default:break;
			}
		}
    	completionPrefix = getCurPrefix();
		kWordsCompleter->setCompletionPrefix(completionPrefix);
		if (completionPrefix == "") { return; }
		if (kWordsCompleter->completionCount() > 0) {
			curTextCursorRect = cursorRect();
			kWordsCompleter->complete(QRect(curTextCursorRect.left() + rowNumWidth()
				, curTextCursorRect.top() + fontMetrics().height(), 150, 1));
		} else {
			kWordsCompleter->popup()->setVisible(false);
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
int SmartEdit::rowNumWidth() {
	int digits = 4;//��ʼλ��
	int max = qMax(1, blockCount());//��ȡ���λ����Ĭ������1λ
	while (max >= 1000) {//2λ�����ȹ̶���3λ������������������
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
	setViewportMargins(rowNumWidth() - 3, -1, -3, 0);
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
/*��ȡǰ׺����괦����һ���ո�֮����ַ���*/
QString SmartEdit::getCurPrefix() const {
	QString selectedText;
	QTextCursor curTextCursor = textCursor();//ֻ���ñ�����ȡ��ǰ��꣬������textCursor()ֱ�Ӳ����������±���
	//���������ƶ�cursor����ѡ���ַ������鿴ѡ�еĵ������Ƿ��пո񡪡��ո���Ϊ���ʵķָ���
	while (curTextCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1)) {
		selectedText = curTextCursor.selectedText();
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
/*ǰ׺����*/
void SmartEdit::updatePrefix() {

	QString curPrefix = getCurPrefix();
	if (curPrefix == "") {
		kWordsCompleter->setCompletionPrefix("----");
		curTextCursorRect = cursorRect();
		kWordsCompleter->complete(QRect(curTextCursorRect.left(), curTextCursorRect.top(), 0, 0));
	}
}
/*��ȫ�ؼ���*/
void SmartEdit::completeKWord(const QString& completion) {
	QString curPrefix = getCurPrefix(),
		shouldInsertText = completion;
	curTextCursor = textCursor();
	if (!completion.contains(curPrefix)) {  // ɾ��֮ǰδ��ȫ���ַ�
		curTextCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, curPrefix.size());
		curTextCursor.clearSelection();
	}
	else {  // ��ȫ��Ӧ���ַ�
		shouldInsertText = shouldInsertText.replace(
			shouldInsertText.indexOf(curPrefix), curPrefix.size(), "");
	}curTextCursor.insertText(shouldInsertText);
}
