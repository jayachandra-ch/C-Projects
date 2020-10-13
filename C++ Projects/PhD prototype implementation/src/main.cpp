#include "../../lib/Interface/Interface.h"
#include <QtWidgets/QApplication>
#include <QMainWindow>

// _DEBUG Memory leak
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG   
#ifndef DBG_NEW     
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )      
#define new DBG_NEW   
#endif
#endif  

int main(int argc, char *argv[])
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );//Memory leak

	QApplication a(argc, argv);
	Interface inter;// Start the application
	inter.show();
	return a.exec();

}


