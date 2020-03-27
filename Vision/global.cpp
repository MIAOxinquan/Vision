#include "global.h"
/*¼ÓÔØQSS*/
void  loadStyleSheet(QWidget* ptr, QString fullFileName) {
	QFile file(":/Resources/qss/" + fullFileName);
	file.open(QFile::ReadOnly);
	ptr->setStyleSheet(file.readAll());
	file.close();
}