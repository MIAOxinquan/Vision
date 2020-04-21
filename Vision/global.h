#include <QtCore>
#include <QtWidgets>
//默认路径
#define DEFAULT_PATH ".//product"
/*文本框高亮语法*/
const QList < QColor > colors = {
	QColor(160, 32, 240)
	, Qt::blue
	, Qt::black
	, QColor(205, 104, 57)
};
const QString sigQuote = "\'[^\']?\'";/*字符*/
const QString quote = "\"[^\"]*\"";/*字符串*/
const QString sigCmt = "//[^\n]*";/*单行注释*/
const QString mulCmtStart = "/\\*";/*多行注释开头*/
const QString mulCmtEnd = "\\*/";/*多行注释结尾*/
const QString nodeStart = "<\\@";/*节点头*/
const QString nodeEnd = "\\@>";/*节点尾*/
/*C++*/
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
	"if_else","do_while","try_catch"/*0,1,2, complex conditions*/
};
/*工具框关键字*/
const QStringList toolKeys = {
	"empty"/*0, empty block*/
	,"enum","union" ,"struct" ,"class"
	/*1,-4, blue , class\n{\n\n}, struct\n{\n\n}, space func()\n{\n\n}*/
	,"if"/*5, if()*/
	,"if_else"/*6, if()\n{\n\n}\nelse\n{\n\n}*/
	,"switch"/*7, switch()\n{\ndefault: break;\n}*/
	,"for"/*8, for(;;)\n{\n\n}*/
	,"while"/*9, while()\n{\n\n}*/
	,"do_while"/*10, do\n{\n\n}while();*/
	,"try_catch"/*11, try\n{\n\n}*/
	,"func"/*12, func()\n{\n\n}*/
};
/*文本框智能补充字段*/
const QStringList smarts = {
	"()"/*0,  sizeof if while catch*/
	,"\n{\n\n}"/*1, enum union struct class do try func*/
	,"(;;)\n{\n\n}"/*2,  for*/
	,"()\n{\ndefault:break;\n}"/*3, switch*/
	,"()\n{\n\n}\nelse\n{\n\n}"/*4, if_else*/
	,"\n{\n\n}while();"/*5, do_while*/
	,"\n{\n\n}\ncatch()\n{\nthrow;\n}"/*6, try_catch*/
};
const QStringList undefined = {
	"Name"/*0, for class, struct*/
	,"Statement"/*1, for if*2, for, while, do*/
	,"Input"/*2, specail for switch*/
	,"Type "/*3, special for func*/
};
/*加载QSS*/
void  loadStyleSheet(QWidget* ptr, QString fullFileName);