#include <QtCore>
#include <QtWidgets\qmainwindow.h>
#include <QtWidgets\qplaintextedit.h>
#include <QtWidgets\qwidget.h>
#include <QtWidgets\qlistwidget.h>
/*文本框关键字*/
const QStringList keys = {
	"void","int","float","double","char","string"/*0-5,with space*/
	,"class","struct"/*6,7, with  \n{\n\n};*/
	/*0-7,blue above and 8-18,purple below*/
	,"return","const","case","else"/*8,9,10,11,with space*/
	,"default"/*12,with :*/
	,"continue","break"/*13,14,with ;*/
	,"if","while"/*15,16, with ()*/
	,"for"/*17,with (;;)\n{\n\n}*/
	,"do"/*18,with \n{\n\n} while();*/
	,"if_else"/*19, with if()\n{\n\n}\nelse\n{\n\n}*/
	,"switch"/*20,with ()\n{\ndefault: break;\n}*/
};
/*文本框智能补充字段*/
const QStringList smarts = {
	"\n{\n\n};"/*0, for class struct*/
	,"()"/*1, for if while*/
	,"(;;)\n{\n\n}"/*2, for for*/
	,"\n{\n\n}while();"/*3, for do*/
	,"()\n{\n\n}\nelse\n{\n\n}"/*4 ,for if_else*/
	,"()\n{\ndefault:break;\n}"/*5, for switch*/
};
/*工具框关键字*/
const QStringList toolKeys = {
	"class","struct"
	/*0,1, blue , class\n{\n\n}, struct\n{\n\n}, space func()\n{\n\n}*/
	,"if"/*2, if()*/
	,"if_else"/*3, if()\n{\n\n}\nelse\n{\n\n}*/
	,"switch"/*4, switch()\n{\ndefault: break;\n}*/
	,"for"/*5, for(;;)\n{\n\n}*/
	,"while"/*6, while()\n{\n\n}*/
	,"do_while"/*7, do\n{\n\n}while();*/
	,"func"/*8, func()\n{\n\n}*/
};
/*工具框全字段*/
const QStringList toolSmarts = {
	"class\n{\n\n}"
	,"struct\n{\n\n}"
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
	,"Input"/*2, for switch*/
	,"Type"/*3, for func*/
};
/*加载QSS*/
void  loadStyleSheet(QWidget* ptr, QString fullFileName);