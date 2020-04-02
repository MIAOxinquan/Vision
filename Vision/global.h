#include <QtCore>
#include <QtWidgets>
/*�ı�������﷨*/
const QList < QColor > colors = {
	QColor(160, 32, 240)
	, Qt::blue
	, Qt::black
	, QColor(205, 104, 57)
	, Qt::darkGreen
};
const QString sigQuote = "\'[^\']?\'";/*�ַ�*/
const QString quote = "\"[^\"]*\"";/*�ַ���*/
const QString sigCmt = "//[^\n]*";/*����ע��*/
const QString mulCmtStart = "/\\*";/*����ע�Ϳ�ͷ*/
const QString mulCmtEnd = "\\*/";/*����ע�ͽ�β*/
//C++
const QStringList keys_cpp_blue = {
	"operator","void","bool","char"
	,"short","int","float","double","long"
	,"enum","union","struct","class"/*0-12, var types*/
	,"static","extern"
	,"public","protected","private"
	,"virtual","explicit"
	,"const","volatile"
	,"signed","unsigned"/*13-23, modifiers and qulifiers*/
	,"true","false","this","nullptr"/*24-27, values*/
	,"new","delete","sizeof"/*28-30, functions*/
};
const QStringList keys_cpp_purple = {
	"if","else","switch","default","case","continue","break"/*0-6, conditions*/
	,"for","do","while","return"/*7-10, loops*/
	,"try","catch","throw"/*11-13, try-catch*/
	,"goto"/*14, goto*/
};
const QStringList keys_cpp_normal = {
	"if_else","do_while"/*0,1, complex conditions*/
};
/*�ı������ܲ����ֶ�*/
const QStringList smarts = {
	"()"/*0,  sizeof if while catch*/
	,"\n{\n\n}"/*1, enum union struct class do try*/
	,"(;;)\n{\n\n}"/*2,  for*/
	,"()\n{\ndefault:break;\n}"/*3, switch*/
	,"()\n{\n\n}\nelse\n{\n\n}"/*4, if_else*/
	,"\n{\n\n}while();"/*5, do_while*/
};
/*���߿�ؼ���*/
const QStringList toolKeys = {
	"enum","union" ,"struct" ,"class"
	/*0,-3, blue , class\n{\n\n}, struct\n{\n\n}, space func()\n{\n\n}*/
	,"if"/*4, if()*/
	,"if_else"/*5, if()\n{\n\n}\nelse\n{\n\n}*/
	,"switch"/*6, switch()\n{\ndefault: break;\n}*/
	,"for"/*7, for(;;)\n{\n\n}*/
	,"while"/*8, while()\n{\n\n}*/
	,"do_while"/*9, do\n{\n\n}while();*/
	,"func"/*10, func()\n{\n\n}*/
};
/*���߿�ȫ�ֶ�*/
const QStringList toolSmarts = {
	"enum\n{\n\n};"
	,"union\n{\n\n};"
	,"struct\n{\n\n};"
	,"class\n{\n\n};"
	,"if()\n{\n\n}"
	,"if()\n{\n\n}\nelse\n{\n\n}"
	,"switch()\n{\ndefault:break;\n}"
	,"for(;;)\n{\n\n}"
	,"while()\n{\n\n}"
	,"do\n{\n\n}while();"
	,"func()\n{\n\n}"
};
const QStringList undefined = {
	"Name"/*0, for class, struct*/
	,"Statement"/*1, for if*2, for, while, do*/
	,"Input"/*2, specail for switch*/
	,"Type "/*3, special for func*/
};
/*����QSS*/
void  loadStyleSheet(QWidget* ptr, QString fullFileName);