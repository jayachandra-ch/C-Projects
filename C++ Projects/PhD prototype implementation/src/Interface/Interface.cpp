#include "../../lib/Interface/Interface.h"

#include <QHBoxLayout>
#include <QFileDialog>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QCloseEvent>

Interface::Interface(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	QHBoxLayout * layout = new QHBoxLayout;

	QButtonGroup * groupCapture = new QButtonGroup;
	groupCapture->addButton(ui.radioButton_Camera);
	groupCapture->addButton(ui.radioButton_Movie);
	groupCapture->addButton(ui.radioButton_Picture);

	connect(ui.pushButton_Start, SIGNAL(clicked()),
                     this, SLOT(start()));
	connect(ui.pushButton_Stop, SIGNAL(clicked()),
                     this, SLOT(stop()));
	connect(ui.pushButton_browse, SIGNAL(clicked()),
                     this, SLOT(browse()));
	connect(groupCapture, SIGNAL(buttonClicked(int)),
                     this, SLOT(updateMod()));

	connect(ui.checkBox_Center, SIGNAL(stateChanged(int)),
                     this, SLOT(checkBox_Center(int)));
	connect(ui.checkBox_Face, SIGNAL(stateChanged(int)),
                     this, SLOT(checkBox_Face(int)));
	connect(ui.checkBox_Focus, SIGNAL(stateChanged(int)),
                     this, SLOT(checkBox_Focus(int)));

	process = new Process;
	process->init(ui);
	
	currentInput = new Camera;
	double width=600;
	double height=600;
	inputPict = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	cvSet(inputPict, cvScalar(0,0,0));

	isCamAvailable=currentInput->isAvailble();
	
	if(!isCamAvailable){
		ui.radioButton_Camera->setEnabled(false);
		ui.radioButton_Picture->setChecked(true);
		ui.pushButton_browse->setEnabled(true);
		ui.pushButton_Start->setEnabled(false);
		currentInput = new Picture;
	}
	ui.pushButton_Stop->setEnabled(false);
	update(width,height);
}

void Interface::start(){

	double width;
	double height;
	double timer; 

	if(ui.radioButton_Picture->isChecked()){
		currentInput->init(width, height, inputPict, timer, ui);
		image=new Image(inputPict, process);
		ui.widget_orig->putImage(inputPict);
		image->setImage(inputPict,ui.checkBox_Focus->isChecked(),ui.checkBox_Center->isChecked(),ui.checkBox_Face->isChecked());
		ui.widget_modi->putImage(image->display());
	}else{
		if(currentInput->init(width, height, inputPict, timer, ui)){
			image=new Image(inputPict, process);
			idTimer=startTimer(timer);
		}
	}
	update(width,height);
	resize(width*2+200, height+150);
}

void Interface::stop(){
	killTimer(idTimer);
	image->~Image();
	ui.pushButton_Stop->setEnabled(false);
	
	ui.pushButton_Start->setEnabled(true);
	if(isCamAvailable){
		ui.radioButton_Camera->setEnabled(true);
	}
	ui.radioButton_Movie->setEnabled(true);
	ui.radioButton_Picture->setEnabled(true);
}

void Interface::browse(){
	QString filePath;
	if(ui.radioButton_Picture->isChecked())
		filePath = QFileDialog::getOpenFileName(this, tr("Select a file"),"/home",tr("Picture (*.jpg *.bmp *.gif)"));
	else
		filePath = QFileDialog::getOpenFileName(this, tr("Select a file"),"/home",tr("Movie (*.avi *.mp4 *.wmv)"));

	if(!(filePath.isEmpty() || filePath.isNull())){
		ui.label->setText(filePath);
		ui.pushButton_Start->setEnabled(true);

		if(ui.radioButton_Picture->isChecked()){
			inputPict=cvLoadImage(filePath.toStdString().c_str());
			currentInput = new Picture;
		}else
			currentInput = new CompressedVideo(filePath.toStdString());
	}
}

void Interface::updateMod(){
	if(ui.radioButton_Camera->isChecked()){
		ui.pushButton_browse->setEnabled(false);
		ui.pushButton_Start->setEnabled(true);
		currentInput = new Camera;
	}
	if(ui.radioButton_Movie->isChecked()){
		ui.pushButton_browse->setEnabled(true);
		ui.pushButton_Start->setEnabled(false);
	}
	if(ui.radioButton_Picture->isChecked()){
		ui.pushButton_browse->setEnabled(true);
		ui.pushButton_Start->setEnabled(false);
	}
}

void Interface::timerEvent(QTimerEvent*) {
	if(currentInput->run(inputPict)){
		ui.widget_orig->putImage(inputPict);
		image->setImage(inputPict,ui.checkBox_Focus->isChecked(),ui.checkBox_Center->isChecked(),ui.checkBox_Face->isChecked());
		ui.widget_modi->putImage(image->display());
	}else
		stop();
}

void Interface::update(double width,double height){
	ui.widget_orig->setGeometry(QRect(150, 0, width, height));
	ui.widget_modi->setGeometry(QRect(150+width, 0,width, height));
	if(height<360+50){
		ui.pushButton_browse->setGeometry(QRect(10,430,110,23));
		ui.label->setGeometry(QRect(150,430,500,25));
	}else{
		ui.pushButton_browse->setGeometry(QRect(10,height,110,23));
		ui.label->setGeometry(QRect(150,height,500,25));
	}
}

void Interface::closeEvent(QCloseEvent *even){
	delete(currentInput);  
	qApp->quit();
}

void Interface::checkBox_Focus(int state){
	if(state)
		process->attach(0);
	else
		process->remove(0);
}
void Interface::checkBox_Center(int state){
	if(state)
		process->attach(1);
	else
		process->remove(1);
}
void Interface::checkBox_Face(int state){
	if(state)
		process->attach(2);
	else
		process->remove(2);
}
