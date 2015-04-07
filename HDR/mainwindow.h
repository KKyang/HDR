#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressBar>
#include "opencv2/opencv.hpp"
#include "hdr.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_hdr_clicked();

    void on_actionOpen_image_triggered();

    void on_checkBox_MTB_stateChanged(int arg1);

    void on_checkBox_gammaC_stateChanged(int arg1);

    void on_doubleSpinBox_gamma_valueChanged(double arg1);

private slots:
    void progressBarStatus(QString &content, int num);
    void on_actionSave_hdr_triggered();

private:
    Ui::MainWindow *ui;
    QProgressBar *progressBar;

    QStringList list;
    std::vector<cv::Mat> images;
    cv::Mat _hdr;
    cv::Mat tone_image;
    bool MTB_status = true;
};

#endif // MAINWINDOW_H
