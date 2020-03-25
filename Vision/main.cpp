#include <QtWidgets/QApplication>
#include "Vision.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Vision vision;
	vision.show();
	return a.exec();
}
