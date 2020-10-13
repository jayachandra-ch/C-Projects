#ifndef INTERFACE_H
#define INTERFACE_H

#include <QtWidgets/QWidget>
#include "ui_Interface.h"
#include "QOpenCV2Widget.h"
#include "Camera.h"
#include "Image.h"

class Interface : public QWidget
{
	Q_OBJECT

private:
	Ui::InterfaceClass ui;
	Camera * camera;
	Image * image;
	FaceMapping * mapping_face;
	FocusMapping * mapping_focus;

public slots:
	void start();

protected:
	void timerEvent(QTimerEvent*);  

public:
	Interface(QWidget *parent = 0);
	~Interface();
	


};

#endif // INTERFACE_H
