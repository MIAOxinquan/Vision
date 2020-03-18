#include "stdafx.h"
#include "Vision.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Vision vision;
	vision.show();
	return a.exec();
}
