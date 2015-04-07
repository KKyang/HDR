#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    progressBar = new QProgressBar(ui->statusBar);
    progressBar->setAlignment(Qt::AlignRight);
    progressBar->setMaximumSize(180, 19);
    ui->statusBar->addPermanentWidget(progressBar);
    progressBar->setValue(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::uiControl(bool s)
{
    ui->groupBox_hdrProperties->setEnabled(s);
    ui->pushButton_hdr->setEnabled(s);
}

void MainWindow::on_pushButton_hdr_clicked()
{
    if(images.size() < 2)
    {
        QMessageBox::warning(this, "Warning", "Please open an image set.");
        return;
    }

    uiControl(false);

    ui->textBrowser_log->append("==============");
    HDR hdr;
    connect(&hdr, SIGNAL(sendProgress(QString&,int)), this, SLOT(progressBarStatus(QString&,int)));
    if(_hdr.empty() || ui->checkBox_MTB->isChecked() != MTB_status)
    {
        MTB_status = ui->checkBox_MTB->isChecked();
        hdr.process(images, list, _hdr, MTB_status);
    }

    ui->graphicsView->initialize(1, images[0].cols, images[0].rows);

    ui->statusBar->clearMessage();
    progressBar->setValue(0);
    if(ui->radioButton_durand->isChecked())
    {
        ui->textBrowser_log->append("Start creating tone mapping (Durand)");
        cv::Mat du;
        hdr.toneMappingDurand(_hdr, du);
        if(ui->checkBox_gammaC->isChecked())
        {
            ui->textBrowser_log->append("Correcting gamma");
            if(ui->checkBox_inverse->isChecked())
            {
                hdr.gammaCorrection(du, du, (1.0 / ui->doubleSpinBox_gamma->value()));
            }
            else
            {
                hdr.gammaCorrection(du, du, ui->doubleSpinBox_gamma->value());
            }
        }
        ui->graphicsView->setImage(du);
        tone_image = du.clone();
    }
    else if(ui->radioButton_reinhard->isChecked())
    {
        ui->textBrowser_log->append("Start creating tone mapping (Reinhard)");
        cv::Mat rei;

        hdr.toneMappingReinhard(_hdr, rei);
        if(ui->checkBox_gammaC->isChecked())
        {
            ui->textBrowser_log->append("Correcting gamma");
            if(ui->checkBox_inverse->isChecked())
            {
                hdr.gammaCorrection(rei, rei, (1.0 / ui->doubleSpinBox_gamma->value()));
            }
            else
            {
                hdr.gammaCorrection(rei, rei, ui->doubleSpinBox_gamma->value());
            }
        }
        ui->graphicsView->setImage(rei);
        tone_image = rei.clone();
    }

    uiControl(true);
    ui->textBrowser_log->append("Done");
    ui->textBrowser_log->append("==============");
    disconnect(&hdr, SIGNAL(sendProgress(QString&,int)), this, SLOT(progressBarStatus(QString&,int)));
}

void MainWindow::progressBarStatus(QString &content, int num)
{
    ui->statusBar->showMessage(content);
    ui->textBrowser_log->append(content);
    progressBar->setValue(num);
    ui->statusBar->update();
}

void MainWindow::on_actionOpen_image_triggered()
{
    list = QFileDialog::getOpenFileNames(this, "Open Images!");
    images.clear();
    cv::Mat tempimg;
    for(int i = 0; i < list.size(); i++)
    {
        tempimg = cv::imread(list.at(i).toStdString());
        images.push_back(tempimg.clone());
    }

    if(images.size() < 2)
    {
        QMessageBox::warning(this, "Warning", "Not enough images.");
        images.clear();
        return;
    }
    for(int i = 0; i < images.size(); i++)
    {
        if(images[i].empty())
        {
            QMessageBox::warning(this, "Warning", QString(QString::number(i) + " th image is empty!"));
            return;
        }
    }

    if(!_hdr.empty())
        _hdr.release();
}

void MainWindow::on_checkBox_MTB_stateChanged(int arg1)
{
    ui->checkBox_MTB->isChecked() ? ui->checkBox_MTB->setText("On") : ui->checkBox_MTB->setText("Off");
}

void MainWindow::on_checkBox_gammaC_stateChanged(int arg1)
{
    ui->checkBox_gammaC->isChecked() ? ui->checkBox_gammaC->setText("On") : ui->checkBox_gammaC->setText("Off");
}

void MainWindow::on_doubleSpinBox_gamma_valueChanged(double arg1)
{
    if(arg1 <= 0)
    {
        ui->doubleSpinBox_gamma->setValue(2.2);
    }
}

void MainWindow::on_actionSave_hdr_triggered()
{
    if(!_hdr.empty())
        cv::imwrite("result.hdr", _hdr);
}
