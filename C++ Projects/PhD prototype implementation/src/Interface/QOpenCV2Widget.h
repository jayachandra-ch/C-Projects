
#ifndef QOPENCV2WIDGET_H
#define QOPENCV2WIDGET_H

#include <opencv/cv.h>
#include <QPixmap>
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QImage>

class QOpenCV2Widget : public QWidget {
    private:
        QLabel *imagelabel;
        QVBoxLayout *layout;
        
        QImage image;
        
    public:
        QOpenCV2Widget(QWidget *parent = 0);
        ~QOpenCV2Widget(void);
        void putImage(IplImage *);
}; 

#endif
