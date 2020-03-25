#include "SmartEdit.h"

SmartEdit::SmartEdit(QTabWidget* parent)
	:QPlainTextEdit(parent)
	, rowNumArea(new RowNumArea(this))
	, keyWordsCompleter(NULL)
{
	keyWordsCompleter = new QCompleter(keyWords);
	init();//��ʼ��
	rowContentPlot();//��ʼ��ˢ���кſ�
	//�ۺ���
	connect(keyWordsCompleter, SIGNAL(activated(QString)), this, SLOT(completeKeyWord(QString)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(rowContentPlot()));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updatePrefix()));

}

SmartEdit::~SmartEdit() {
	delete rowNumArea;	rowNumArea = NULL;
	delete keyWordsCompleter;	keyWordsCompleter = NULL;
}
/*��ʼ��*/
void SmartEdit::init() {
	rowNumArea->setFont(QFont("΢���ź�", 12, QFont::Bold));
	setFont(QFont("΢���ź�", 12));
	setWordWrapMode(QTextOption::NoWrap);  //ˮƽ����Ӧ������
	keyWordsCompleter->setWidget(this);
	keyWordsCompleter->setCaseSensitivity(Qt::CaseSensitive); 
	keyWordsCompleter->setCompletionMode(QCompleter::PopupCompletion);//ƥ������������,����
	keyWordsCompleter->popup()->setFont(QFont("΢���ź�", 12));
	keyWordsCompleter->setMaxVisibleItems(5);
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
QString SmartEdit::getCompletionPrefix() const {
	QString selectedText;
	//���������ƶ�cursor����ѡ���ַ������鿴ѡ�еĵ������Ƿ��пո񡪡��ո���Ϊ���ʵķָ���
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
/*ǰ׺����*/
void SmartEdit::updatePrefix() {
	QString curPrefix = getCompletionPrefix();
	if (curPrefix == "") {
		keyWordsCompleter->setCompletionPrefix("----");
		QRect cursorRct = cursorRect();
		keyWordsCompleter->complete(QRect(cursorRct.left(), cursorRct.top(), 0, 0));
	}
}
/*��ȫ�ؼ���*/
void SmartEdit::completeKeyWord(const QString& completion) {
	QString curPrefix = getCompletionPrefix(),
		shouldInsertText = completion;
	if (!completion.contains(curPrefix)) {  // ɾ��֮ǰδ��ȫ���ַ�
		textCursor().movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, curPrefix.size());
		textCursor().clearSelection();
	}
	else {  // ��ȫ��Ӧ���ַ�
		shouldInsertText = shouldInsertText.replace(
			shouldInsertText.indexOf(curPrefix), curPrefix.size(), "");
	}
	textCursor().insertText(shouldInsertText);
}