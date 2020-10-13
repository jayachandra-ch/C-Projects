#include "../../lib/Algorithm/Process.h"

Process::Process(){
	mapping_focus = new FocusMapping;
	mapping_center = new CenterMapping;
	mapping_face = new FaceMapping;
}

void Process::init(Ui_InterfaceClass & ui){
	if(ui.checkBox_Focus->isChecked())
		attach(0);
	if(ui.checkBox_Center->isChecked())
		attach(1);
	if(ui.checkBox_Face->isChecked())
		attach(2);
}
void Process::attach(int runable){
	//Order in the array is important
	switch (runable){
	case 0:{
		lstRunable[runable]=mapping_focus;
		break;
		   }
	case 1:{
		lstRunable[runable]=mapping_center;
		break;
		   }
	case 2:{
		lstRunable[runable]=mapping_face;
		break;
		   }
	}
}

void Process::remove(int runable){
	lstRunable[runable]=NULL;
}

void Process::run(IplImage * imageOrig,IplImage * workingImage){
	for (unsigned i=0; i<NB_ALGO; ++i){
		if(lstRunable[i]!=NULL)
			lstRunable[i]->run(imageOrig,workingImage);
	}
}