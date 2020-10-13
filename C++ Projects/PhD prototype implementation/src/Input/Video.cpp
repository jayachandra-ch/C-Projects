#include "../../lib/Input/Video.h"

void Video::updateInterface(Ui_InterfaceClass & ui){
	ui.pushButton_Stop->setEnabled(true);
	ui.pushButton_Start->setEnabled(false);
	ui.radioButton_Camera->setEnabled(false);
	ui.radioButton_Movie->setEnabled(false);
	ui.radioButton_Picture->setEnabled(false);	
}