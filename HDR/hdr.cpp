#include "hdr.h"
#include <fstream>
#include <iomanip>
#include <QDebug>

HDR::HDR(QObject *parent): QObject(parent)
{
    srand(time(NULL));
}

void HDR::process(std::vector<cv::Mat> inputArrays, QStringList &list, cv::Mat &_hdr, bool MTB)
{
    emit sendProgress(QString("Getting exposure information"), 1);
    getExposureTime(list);
    if(MTB)
    {
        emit sendProgress(QString("Start MTB"), 3);
        MTBA(inputArrays, inputArrays);
    }
    emit sendProgress(QString("Start calculating radiance map"), 20);
    radianceMap(inputArrays, _hdr);
    emit sendProgress(QString("Done"), 100);
}

void HDR::MTBA(std::vector<cv::Mat> &inputArrays, std::vector<cv::Mat> &outputArrays)
{
    cv::Mat sample = inputArrays[0].clone();
    MTB(sample, sample);
    std::vector<int> move_length_x(inputArrays.size(), 0);
    std::vector<int> move_length_y(inputArrays.size(), 0);

    //cut the noise frome the base (first pic)


    for(int j = 1; j < inputArrays.size(); j++)
    {
        cv::Mat temp1 = inputArrays[j].clone();
        MTB(temp1, temp1);
        for(int i = 5; i >= 0; i--)
        {
            cv::Mat temp_samp = sample.clone();
            cv::Mat temp = temp1.clone();

            cv::Mat sampimg = sample.clone();
            cv::Mat otherimg = temp1.clone();

            cv::Mat samp = temp_samp.clone();
            cv::Mat other = temp1.clone();
            cv::resize(samp,samp,cv::Size((int)(sample.cols / pow(2, i+1)), (int)(sample.rows / pow(2, i+1))));
            cv::resize(samp,samp,cv::Size((int)(sample.cols / pow(2, i)), (int)(sample.rows / pow(2, i))));
            cv::resize(other,other,cv::Size((int)(sample.cols / pow(2, i+1)), (int)(sample.rows / pow(2, i+1))));
            cv::resize(other,other,cv::Size((int)(sample.cols / pow(2, i)), (int)(sample.rows / pow(2, i))));

//            cv::imshow("samp",samp);
//            cv::imshow("temp_samp",temp_samp);
            //cv::imshow("temp",temp);

            for(int b = 0; b < samp.rows; b++)
            {
                for(int a = 0; a < samp.cols; a++)
                {
                    if(samp.at<uchar>(b,a)!=sampimg.at<uchar>(b,a))
                    {
                        temp_samp.at<uchar>(b,a) = 0;
                    }
                    if(other.at<uchar>(b,a)!=otherimg.at<uchar>(b,a))
                    {
                        temp.at<uchar>(b,a) = 0;
                    }
                    if(sampimg.at<uchar>(b,a)!=otherimg.at<uchar>(b,a))
                    {
                        temp.at<uchar>(b,a) = 0;
                    }

                }
            }
//            cv::imshow("sample",sample);
//            cv::imshow("samp",samp);
//            cv::imshow("temp_samp",temp_samp);
//            cv::imshow("temp",temp);

            //cv::resize(temp, temp, cv::Size((int)(sample.cols / pow(2, i)), (int)(sample.rows / pow(2, i))));
            //cv::resize(temp_samp, temp_samp, cv::Size((int)(sample.cols / pow(2, i)), (int)(sample.rows / pow(2, i))));

            unsigned double total[9] = {0};

            for(int b = 0; b < temp_samp.rows; b++)
            {
                for(int a = 0; a < temp_samp.cols; a++)
                {
                    int count = 0;
                    for(int y = -1; y <= 1; y++)
                    {
                        for(int x = -1; x <= 1; x++)
                        {
                            if(a + move_length_x[j] + x > 0 && a + move_length_x[j] + x < temp.cols && b + move_length_y[j] + y > 0 && b + move_length_y[j] + y < temp.rows)
                            {
                                if((int)temp_samp.at<uchar>(b, a) != (int)temp.at<uchar>(b + move_length_y[j] + y, a + move_length_x[j] + x))
                                    total[count]++;
                            }
                            count++;
                        }
                    }
                }
            }

            int a = 0;
//            qDebug () << total[0];
            for(int b = 1; b < 9; b++)
            {
                if(total[a] > total[b])
                {
                    a = b;
                }
//                qDebug () << total[b];
            }

//            qDebug() << "=========";
            if(total[a] * 1.5 >= total[4])
            {
                a = 4;
            }

//            qDebug () << "A:" << a;
//            qDebug () << "==========";


            switch(a)
            {
            case 0:
                move_length_y[j] = (move_length_y[j] - 1) * 2;
                move_length_x[j] = (move_length_x[j] - 1) * 2;
                break;
            case 1:
                move_length_y[j] = (move_length_y[j] - 1) * 2;
                break;
            case 2:
                move_length_y[j] = (move_length_y[j] - 1) * 2;
                move_length_x[j] = (move_length_x[j] + 1) * 2;
                break;
            case 3:
                move_length_x[j] = (move_length_x[j] - 1) * 2;
                break;
            case 4:
                break;
            case 5:
                move_length_x[j] = (move_length_x[j] + 1) * 2;
                break;
            case 6:
                move_length_y[j] = (move_length_y[j] + 1) * 2;
                move_length_x[j] = (move_length_x[j] - 1) * 2;
                break;
            case 7:
                move_length_y[j] = (move_length_y[j] + 1) * 2;
                break;
            case 8:
                move_length_y[j] = (move_length_y[j] + 1) * 2;
                move_length_x[j] = (move_length_x[j] + 1) * 2;
                break;
            }
//            qDebug() << move_length_x[j] << " " << move_length_y[j];
//            qDebug() << "@@@@@@@";
        }


    }

    int minX = move_length_x[0], maxX = move_length_x[0];
    int minY = move_length_y[0], maxY = move_length_y[0];

    for(int i = 1; i < inputArrays.size(); i++)
    {
        if(minX > move_length_x[i]){minX = move_length_x[i];}
        if(maxX < move_length_x[i]){maxX = move_length_x[i];}
        if(minY > move_length_y[i]){minY = move_length_y[i];}
        if(maxY < move_length_y[i]){maxY = move_length_y[i];}
    }
    std::vector<cv::Mat> tempdest(inputArrays.size());

int a, j, i, k;
#pragma parallel for private(a, j, i, k)
    for(a=0; a < inputArrays.size(); a++)
    {
        cv::Mat&& dest = cv::Mat::zeros(sample.rows + abs(minY) + abs(maxY), sample.cols + abs(minX) + abs(maxX), CV_8UC3);
        for(j = 0; j < inputArrays[a].rows; j++)
        {
            for(i = 0; i < inputArrays[a].cols; i++)
            {
                for(k = 0; k < 3; k++)
                {
                        dest.at<cv::Vec3b>(j + abs(minY) + move_length_y[a], i + abs(minX) + move_length_x[a])[k] = inputArrays[a].at<cv::Vec3b>(j, i)[k];
                }
            }
        }
        tempdest[a] = dest.clone();
    }

    cv::Mat&& dest1 = cv::Mat::zeros(sample.rows + abs(minY) + abs(maxY), sample.cols + abs(minX) + abs(maxX), CV_8UC3);

    int tmp = 0;
//#pragma parallel for private(a, j, i, k) firstprivate(tmp)
    for(j = 0; j < tempdest[0].rows; j++)
    {
        for(i = 0; i < tempdest[0].cols; i++)
        {
            for(k = 0; k < 3; k++)
            {
                tmp = 0;
                for(a=0; a < tempdest.size(); a++)
                {
                     tmp += tempdest[a].at<cv::Vec3b>(j, i)[k];
                }

                dest1.at<cv::Vec3b>(j, i)[k] = tmp / tempdest.size();
            }
        }
    }

    outputArrays.clear();
    outputArrays = tempdest;
}

void HDR::MTB(cv::Mat &inputArray, cv::Mat &outputArray)
{
    int color_count[256] = {0};
    int i, j;
    cv::Mat temp;
    if(inputArray.type() == CV_8UC3)
    {
        cv::cvtColor(inputArray, temp, cv::COLOR_BGR2GRAY);
    }
    else
    {
        temp = inputArray.clone();
    }

//#pragma omp parallel for private(i, j)
    for(j = 0; j < inputArray.rows; j++)
    {
        for(i = 0; i < inputArray.cols; i++)
        {
            color_count[temp.at<uchar>(j, i)]++;
        }
    }

    int threshold_total = 0;
    double thresh;
    for(j = 0; j < inputArray.rows; j++)
    {
        threshold_total += color_count[j];

        if(threshold_total >= (temp.cols * temp.rows)/2)
        {
            thresh = j;
            break;
        }
    }

    cv::Mat&& dest = cv::Mat::zeros(temp.rows, temp.cols, CV_8UC1);
#pragma omp parallel for private(i, j)
    for(j = 0; j < inputArray.rows; j++)
    {
        for(i = 0; i < inputArray.cols; i++)
        {
            temp.at<uchar>(j, i) > thresh ? dest.at<uchar>(j, i) = 255 : dest.at<uchar>(j, i) = 0;
        }
    }
    outputArray.release();
    outputArray = dest.clone();
    temp.release();
}

void HDR::getExposureTime(QStringList &list)
{
    //以下開始曝光時間的擷取
    QFileInfo * name = new QFileInfo[list.count()];

    for(int i = 0;i<list.count();i++)
    {
        name[i].setFile(list.at(i));
        name[i].absoluteFilePath().replace(QString("\\"),QString("\\\\" ));
    }
    for(int i=0;i<list.count();i++)
    {
        QByteArray ba = name[i].absoluteFilePath().toLatin1();
        const char *c_str2 = ba.data();
        //FILE *fp = fopen("D:\\Dropbox\\sen2\\ve\\test_exif_qt\\test_exif_qt\\0032.jpg", "rb");
        FILE *fp = fopen(c_str2, "rb");
        if (!fp) { printf("Can't open file.\n"); return; }
        fseek(fp, 0, SEEK_END);
        unsigned long fsize = ftell(fp);
        rewind(fp);
        unsigned char *buf = new unsigned char[fsize];
        if (fread(buf, 1, fsize, fp) != fsize) {
            printf("Can't read file.\n");
            return;
        }
        fclose(fp);

        // Parse exif
        EXIFInfo result;
        ParseEXIF(buf, fsize, result);
        if (result.exposureTime)
        {
            //printf("Exposure          : 1/%gs\n", 1.0 / result.exposureTime);
            //exposureTimes[i] = result.exposureTime;
            exposureTimes.push_back(result.exposureTime);

        }
        delete[] buf;
    }
        for(int i=0;i<list.count();i++)
        {
            //qDebug()<<name[i].absoluteFilePath()<<" T = "<<exposureTimes[i];
            emit sendProgress(QString("Exposure info: " + name[i].fileName() + " T = " + QString::number(exposureTimes[i])), 1);
        }

    //曝光時間擷取結束
}

void HDR::radianceMap(std::vector<cv::Mat> &inputArrays, cv::Mat &outputArray)
{
    int re_w = 10, re_h = 10;

    emit sendProgress(QString("Calculating pixels"), 20);
    std::vector<std::vector<hdr_points>> points(re_w * re_h);

    for(int j = 0; j < re_h; j++)
    {
        for(int i = 0; i < re_w; i++)
        {
            for(int a = 0; a < inputArrays.size(); a++)
            {
                cv::Mat temp;
                cv::resize(inputArrays[a], temp, cv::Size(inputArrays[a].cols, inputArrays[a].rows));
                cv::resize(temp, temp, cv::Size(re_w, re_h));
                hdr_points p;
                p.position_x = i;
                p.position_y = j;
                p.b = temp.at<cv::Vec3b>(j, i)[0];
                p.g = temp.at<cv::Vec3b>(j, i)[1];
                p.r = temp.at<cv::Vec3b>(j, i)[2];
                points[j * re_w + i].push_back(p);
            }
        }
    }

    std::vector<std::vector<int>> BZ;
    std::vector<std::vector<int>> RZ;
    std::vector<std::vector<int>> GZ;

    for(int c= 0; c < points.size(); c++){
        vector<int> value_v;
        for (int a = 0; a < points[0].size(); a++)
        {
            value_v.push_back(points[c][a].r);
        }
        RZ.push_back(value_v);
    }
     for(int c= 0; c < points.size(); c++){
        vector<int> value_v;
        for (int a = 0; a < points[0].size(); a++)
        {
            value_v.push_back(points[c][a].g);
        }
        GZ.push_back(value_v);
    }
     for(int c= 0; c < points.size(); c++){
        vector<int> value_v;
        for (int a = 0; a < points[0].size(); a++)
        {
            value_v.push_back(points[c][a].b);
        }
        BZ.push_back(value_v);
    }

    int lambda = 10;
    GSolve value;
    //value.
    //這邊才真的呼叫了
//    ofstream file;
//    file.open("fugus.txt");

//    for(int i = 0; i < RZ.size(); i++)
//    {
//        for(int j = 0; j < RZ[0].size(); j++)
//        {
//                file << std::setw(5) << (double)RZ[i][j] << ",";
//        }
//        file << "\n";
//    }
//    file.close();

    std::vector<cv::Mat> MZ(3);
    emit sendProgress(QString("Recovering R channel response curve"), 25);
    MZ[2] = value.G_lnE(RZ, exposureTimes, lambda).clone();  //value.G_lnE( vector of B layer, exposureTimes,lambda)
    emit sendProgress(QString("Recovering G channel response curve"), 45);
    MZ[1] = value.G_lnE(GZ, exposureTimes, lambda).clone();
    emit sendProgress(QString("Recovering B channel response curve"), 65);
    MZ[0] = value.G_lnE(BZ, exposureTimes, lambda).clone();

//    for(int i = 0; i< MZ[0].rows; i++)
//    {
//        qDebug()<<"coef "<<i<<" "<<MZ[2].at<double>(i,0);
//    }
    emit sendProgress(QString("Reconstructing radiance map"), 85);
    cv::Mat hdr_img = cv::Mat(inputArrays[0].rows, inputArrays[0].cols, CV_32FC3).clone();
    for(int b = 0; b < 3; b++)
    {
        for(int j = 0; j < inputArrays[0].rows; j++)
        {
            for(int i = 0; i < inputArrays[0].cols; i++)
            {
                double weight_sum = 0;
                double total = 0;
                for(int a = 0; a < inputArrays.size(); a++)
                {
                    int color = (int)inputArrays[a].at<cv::Vec3b>(j, i)[b];
                    double w = value.Weighting(color + 1);

                    total += (double)(w * (MZ[b].at<double>(color, 0) - (log(exposureTimes[a])/log(2.718281828))));
                    weight_sum += w;
                }
                if(!IsFiniteNumber((float)exp(total / weight_sum)))
                {
                    hdr_img.at<cv::Vec3f>(j, i)[b] = 0.0;
                }
                else
                {
                    hdr_img.at<cv::Vec3f>(j, i)[b] = (float)exp(total / weight_sum); //multiply 10, seems have better result. Still a bug.
                }
            }
        }
    }

    outputArray.release();
    outputArray = hdr_img.clone();
}



void HDR::toneMappingReinhard(cv::Mat &inputArray, cv::Mat &outputArray)
{
    cv::Mat intensity(inputArray.rows, inputArray.cols, CV_32FC1);
    for(int j = 0; j < inputArray.rows; j++)
    {
        for(int i = 0; i < inputArray.cols; i++)
        {
            intensity.at<float>(j, i) = 0.2126 * inputArray.at<cv::Vec3f>(j, i)[2] + 0.7152 * inputArray.at<cv::Vec3f>(j, i)[1] + 0.0722 * inputArray.at<cv::Vec3f>(j, i)[0];
        }
    }

    double lwMean_total = 0;
    for(int j = 0; j < inputArray.rows; j++)
    {
        for(int i = 0; i < inputArray.cols; i++)
        {
            double lw = intensity.at<float>(j, i);
            lwMean_total += log(0.00001 + lw);
        }
    }

    const float lwMean = exp(lwMean_total / (double)(inputArray.rows * inputArray.cols)); //Reinhard's source code is correct. PAPER IS WRONG!!!
    const float a = 0.36; // From Reinhard a 0.18-0.36. Bigger, lighter
    const float l_white = 3.0; //From Reinhard fig. 6

    cv::Mat L(inputArray.rows, inputArray.cols, CV_32FC1);
    for(int j = 0; j < inputArray.rows; j++)
    {
        for(int i = 0; i < inputArray.cols; i++)
        {
            float lw = intensity.at<float>(j, i);
            float ld = a * lw / lwMean;
            if(ld >= l_white)
            {
                L.at<float>(j, i) = 1;
            }
            else
            {
                float c = 1 + (ld / (l_white * l_white));
                L.at<float>(j, i) = ld * c / (1.0 + ld);
            }
        }
    }


    cv::Mat result(inputArray.rows, inputArray.cols, CV_8UC3);
    for(int c = 0; c < 3; c++)
    {
        for(int j = 0; j < inputArray.rows; j++)
        {
            for(int i = 0; i < inputArray.cols; i++)
            {
                float lw = intensity.at<float>(j, i);
                result.at<cv::Vec3b>(j, i)[c] = cv::saturate_cast<uchar>(inputArray.at<cv::Vec3f>(j, i)[c] * (L.at<float>(j, i) * 255.0 / lw));
            }
        }
    }

    outputArray.release();
    outputArray = result.clone();
}

void HDR::toneMappingDurand(cv::Mat &inputArray, cv::Mat &outputArray)
{
    cv::Mat intensity(inputArray.rows, inputArray.cols, CV_32FC1);
    cv::Mat intensity_log(inputArray.rows, inputArray.cols, CV_32FC1);
    for(int j = 0; j < inputArray.rows; j++)
    {
        for(int i = 0; i < inputArray.cols; i++)
        {
            intensity.at<float>(j, i) = 0.2126f * inputArray.at<cv::Vec3f>(j, i)[2] + 0.7152f * inputArray.at<cv::Vec3f>(j, i)[1] + 0.0722f * inputArray.at<cv::Vec3f>(j, i)[0];
            intensity_log.at<float>(j, i) = log(intensity.at<float>(j, i)) / log(10.0);
        }
    }


    cv::Mat base;

    cv::bilateralFilter(intensity_log, base, -1, 0.4, 0.02 * std::min(inputArray.rows, inputArray.cols)); //0.4 0.02
    cv::Mat detail(inputArray.rows, inputArray.cols, CV_32FC1);
    for(int j = 0; j < inputArray.rows; j++)
    {
        for(int i = 0; i < inputArray.cols; i++)
        {
            detail.at<float>(j, i) = intensity_log.at<float>(j, i) - base.at<float>(j, i);
        }
    }

    double min_log = base.at<float>(0,0), max_log = base.at<float>(0,0);
    for(int j = 0; j < inputArray.rows; j++)
    {
        for(int i = 0; i < inputArray.cols; i++)
        {
            if(base.at<float>(j,i) > max_log){max_log = base.at<float>(j,i);}
            if(base.at<float>(j,i) < min_log){min_log = base.at<float>(j,i);}
        }
    }

    double compression_factor = (log(5) / log(10.0)) /(max_log - min_log);
    double log_abs_scale = 1.0 / pow(10.0, compression_factor * max_log);

    cv::Mat final_intensity(inputArray.rows, inputArray.cols, CV_32FC1);

    for(int j = 0; j < inputArray.rows; j++)
    {
        for(int i = 0; i < inputArray.cols; i++)
        {
            final_intensity.at<float>(j, i) = pow(10.0, base.at<float>(j, i) * compression_factor + detail.at<float>(j, i));
        }
    }

    cv::Mat result(inputArray.rows, inputArray.cols, CV_8UC3);

    for(int c = 0; c < 3; c++)
    {
        for(int j = 0; j < inputArray.rows; j++)
        {
            for(int i = 0; i < inputArray.cols; i++)
            {
                result.at<cv::Vec3b>(j, i)[c] = cv::saturate_cast<uchar>(log_abs_scale * inputArray.at<cv::Vec3f>(j, i)[c] * (final_intensity.at<float>(j, i)/ intensity.at<float>(j, i)) * 255.0f);
            }
        }
    }

    outputArray.release();
    outputArray = result.clone();
}

void HDR::gammaCorrection(cv::Mat &inputArray, cv::Mat &outputArray, double gamma_value)
{
    cv::Mat result(inputArray.rows, inputArray.cols, CV_8UC3);
    for(int c = 0; c < 3; c++)
    {
        for(int j = 0; j < inputArray.rows; j++)
        {
            for(int i = 0; i < inputArray.cols; i++)
            {
                result.at<cv::Vec3b>(j, i)[c] = cv::saturate_cast<uchar>(pow((double)(inputArray.at<cv::Vec3b>(j, i)[c] / 255.0), gamma_value) * 255.0);
            }
        }
    }

    outputArray.release();
    outputArray = result.clone();
}
