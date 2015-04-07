/*M///////////////////////////////////////////////////////////////////////////////////////
// Written by Qt 5.3
// Original by Shih-Jhong Yu, Revised by KKyang.
// Protected under LPGLv3 license.
// https://github.com/KKyang/qtsmartgraphics
//M*/

#ifndef QSMARTGRAPHICSVIEW_H
#define QSMARTGRAPHICSVIEW_H

#include <QFileInfo>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QList>
#include <QWheelEvent>
#include <QMessageBox>
#include <QVector>

#include <vector>
#ifdef HAVE_OPENCV
#include "opencv2/opencv.hpp"
#endif

class QSmartGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit QSmartGraphicsView(QWidget *parent = 0);
    ~QSmartGraphicsView();
    void initialize(const int _img_num, const int width, const int height, int changeRow = 4);
#ifdef HAVE_OPENCV
    void setImage(const cv::Mat &img);
    void setImage(const std::vector<cv::Mat> &imgs);
#endif
    void setImagefromQImage(const QImage &qimg);
    void setImagefromQImage(const std::vector<QImage> &qimgs);
    int getImgNum(){return img_num;}
protected:
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    void sendItemMouXY(const double x, const double y);
    void sendMousePress();
public slots:
    void updateImg();
private slots:
    void on_saveAction_triggered();
private:
    QAction *saveAction;
    QGraphicsScene *scene;
    QVector<QGraphicsPixmapItem*> pix_item_vec;
	int mou_x;
	int mou_y;
    int img_num;
};


#endif // SMARTGRAPHICSVIEW_H
