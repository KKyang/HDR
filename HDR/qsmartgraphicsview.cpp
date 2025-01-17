/*M///////////////////////////////////////////////////////////////////////////////////////
// Written by Qt 5.3
// Original by Shih-Jhong Yu, Revised by KKyang.
// Protected under LPGLv3 license.
// https://github.com/KKyang/qtsmartgraphics
//M*/

#include "qsmartgraphicsview.h"
#include <QStyleOptionGraphicsItem>
#include <QMenu>
#include <QPixmap>
#include <QFileDialog>
#include <QGraphicsSceneMouseEvent>
#include <QtConcurrent/QtConcurrentMap>
#include <QTime>
#include <QDebug>
#include <QReadWriteLock>
extern QReadWriteLock lock;

QSmartGraphicsView::QSmartGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
	this->setTransformationAnchor(QGraphicsView::NoAnchor);
    this->setMouseTracking(true);    
    saveAction = new QAction("Save Image", this);
    connect(saveAction, SIGNAL(triggered()), this, SLOT(on_saveAction_triggered()));
    img_num = 0;
    scene = new QGraphicsScene;
    this->setScene(scene);
}

QSmartGraphicsView::~QSmartGraphicsView()
{
    scene->clear();
    delete scene;
}

void QSmartGraphicsView::initialize(const int _img_num, const int width, const int height, int changeRow)
{
    const int CHANGE = changeRow > _img_num ? _img_num : changeRow;
    img_num = _img_num;
    const size_t CAP_NUM = img_num;
    //Clear
    scene->clear();
    pix_item_vec.clear();
    //--
    // Add Pixmap Items
    for(size_t i = 0; i < CAP_NUM; ++i){
        QGraphicsPixmapItem *pix_item = scene->addPixmap(QPixmap(width, height));
        pix_item_vec.push_back(pix_item);
    }

    int hori_spacing = 0, verti_spacing = 0; //30 default
    pix_item_vec[0]->setPos(0, 0);
    QPointF p = pix_item_vec[0]->pos();
    for(size_t i = 1; i < CAP_NUM; i++)
    {        
        hori_spacing = (i % CHANGE) == 0 ? 0 : 30;
        verti_spacing = (i < CHANGE) ? 0 : 30;

        pix_item_vec[i]->setPos(p.x() + width * (i % CHANGE) + hori_spacing, p.y() + (int)(i / CHANGE) * height + verti_spacing);
    }
    this->fitInView(0, 0, width * CHANGE, height * CHANGE + verti_spacing * (CAP_NUM / CHANGE), Qt::KeepAspectRatio);
}

#ifdef HAVE_OPENCV
void QSmartGraphicsView::setImage(const cv::Mat &img)
{
    QImage img_temp(img.cols, img.rows, QImage::Format_RGB888);
    lock.lockForRead();
    for(int y = 0; y < img.rows; ++y){
        memcpy(img_temp.scanLine(y), img.data + y * img.cols * 3, img.cols * 3);
    }
    lock.unlock();
    pix_item_vec[0]->setPixmap(QPixmap::fromImage(img_temp.rgbSwapped()));
}

void QSmartGraphicsView::setImage(const std::vector<cv::Mat> &imgs)
{
    lock.lockForRead();
    for(size_t i = 0; i < imgs.size(); ++i){
        QImage img_temp(imgs[i].cols, imgs[i].rows, QImage::Format_RGB888);
        for(int y = 0; y < imgs[i].rows; ++y){
            memcpy(img_temp.scanLine(y), imgs[i].data + y * imgs[i].cols * 3, imgs[i].cols * 3);
        }
        pix_item_vec[i]->setPixmap(QPixmap::fromImage(img_temp.rgbSwapped()));
    }
    lock.unlock();
    QList<QGraphicsItem *> item_list = this->items(this->rect());
    for(int i = 0; i < item_list.size()/2; ++i){
        item_list.at(i)->update();
    }
}
#endif

void QSmartGraphicsView::setImagefromQImage(const QImage &qimg)
{
    pix_item_vec[0]->setPixmap(QPixmap::fromImage(qimg));
}

void QSmartGraphicsView::updateImg()
{
    QList<QGraphicsItem *> item_list = this->items(this->rect());
    for(int i = 0; i < item_list.size(); ++i){
        QGraphicsPixmapItem *item = dynamic_cast<QGraphicsPixmapItem*>(item_list[i]);
        if(item)
            item->update();        
    }
}

void QSmartGraphicsView::setImagefromQImage(const std::vector<QImage> &qimgs)
{
    lock.lockForRead();
    for(size_t i = 0; i < qimgs.size(); ++i)
        pix_item_vec[i]->setPixmap(QPixmap::fromImage(qimgs[i]));
    lock.unlock();
    QList<QGraphicsItem *> item_list = this->items(this->rect());
    for(int i = 0; i < item_list.size()/2; ++i){
        item_list.at(i)->update();
    }
}

void QSmartGraphicsView::wheelEvent(QWheelEvent *event)
{
	if(event->delta() == 0)
		return;
	QList<QGraphicsItem*> list = this->items();
	if(list.size() <= 0)
		return;

	QPointF pt = this->mapToScene(event->pos());
	double factor;
	if(event->delta() > 0)
		factor = 1.1;
	else if(event->delta() < 0)
		factor = 0.9;
	else
		factor = 1;
	this->scale(factor, factor);
	this->centerOn(pt);
}

void QSmartGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
	if(event->buttons() == Qt::LeftButton){
		this->translate(( -mou_x + event->x())/1.0, ( -mou_y + event->y())/1.0);
		this->setCursor(Qt::ClosedHandCursor);
		//QMessageBox::information(0, QString::number(mou_x - event->x()), QString::number(mou_y - event->y()));
	}
	this->setCursor(Qt::OpenHandCursor);
	mou_x = event->x();
	mou_y = event->y();

}

void QSmartGraphicsView::mousePressEvent(QMouseEvent *event)
{
    mou_x = event->x();
	mou_y = event->y();    
	if(event->button() == Qt::LeftButton)
		this->setCursor(Qt::ClosedHandCursor);
    else if(event->button() == Qt::MidButton)
        this->fitInView(0, 0, this->sceneRect().width(), this->sceneRect().height(), Qt::KeepAspectRatio);
    emit sendMousePress();
}

void QSmartGraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QGraphicsPixmapItem *item = dynamic_cast<QGraphicsPixmapItem *>(this->itemAt(event->pos()));
    if(!item)
        return;
    QPointF local_pt = item->mapFromScene(this->mapToScene(event->pos()));
    qDebug () << local_pt;
    if(this->pix_item_vec.size() == 1)
        emit sendItemMouXY(local_pt.x(), local_pt.y());
}

void QSmartGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    this->setCursor(Qt::ArrowCursor);
    if(event->button() == Qt::RightButton){
        QMenu m(this);
        m.addAction(saveAction);
        m.exec(event->globalPos());
    }
}

void QSmartGraphicsView::on_saveAction_triggered()
{
    bool isError = false;
    if(img_num == 0) {return;}
    QFileDialog d;
    if(img_num > 1)
        d.setConfirmOverwrite(false);
    QFileInfo file_name(d.getSaveFileName(0, "Img",0,"PNG (*.png);;BMP (*.bmp);;JPG (*.jpg)"));
    if(file_name.fileName().isNull()) {
        return;
    }
    if(img_num > 1)
        for(int i = 0; i < img_num; ++i)
        {
            if(!pix_item_vec[i]->pixmap().isNull())
            {
                int num_index = 0;
                if(file_name.exists())
                    ++num_index;
                while(QFile::exists(file_name.absolutePath() + "/" + file_name.completeBaseName() + "_" + QString::number(i + num_index)+"."+file_name.suffix()))
                    ++num_index;
                if(i == 0 && num_index == 0)
                    pix_item_vec[i]->pixmap().save(file_name.absoluteFilePath());
                else
                    pix_item_vec[i]->pixmap().save(file_name.absolutePath() + "/" + file_name.completeBaseName() +"_"+QString::number(i + num_index)+"."+file_name.suffix());
            }
            else{isError = true;}
        }
    else
    {
        if(!pix_item_vec[0]->pixmap().isNull()){pix_item_vec[0]->pixmap().save(file_name.absoluteFilePath());}
        else{isError = true;}
    }
    if(isError){QMessageBox::information(0, 0, "Can Not Save Image!!");}
}
