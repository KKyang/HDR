#ifndef HDR_H
#define HDR_H

#include <QObject>
#include <QFileInfo>
#include <QString>
#include <QByteArray>
#include <time.h>
#include <math.h>
#include "exif.h"
#include "gsolve.h"
#include "opencv2/opencv.hpp"

struct hdr_points{
    int position_x;
    int position_y;
    int r;
    int g;
    int b;
    void operator = (const hdr_points &p){position_x = p.position_x; position_y = p.position_y; r = p.r; g = p.g; b = p.b;}
};


class HDR : public QObject
{
    Q_OBJECT
public:
    explicit HDR(QObject *parent = 0);
    void process(std::vector<cv::Mat> inputArrays, QStringList &list, cv::Mat &_hdr, bool MTB=true);
    void toneMappingReinhard(cv::Mat &inputArray, cv::Mat &outputArray);
    void toneMappingDurand(cv::Mat &inputArray, cv::Mat &outputArray);

    void gammaCorrection(cv::Mat &inputArray, cv::Mat &outputArray, double gamma_value);
signals:
    void sendProgress(QString &pro, int num);
private:
    std::vector<double> exposureTimes;

    void MTBA(std::vector<cv::Mat> &inputArrays, std::vector<cv::Mat> &outputArrays);
    void MTB(cv::Mat &inputArray, cv::Mat &outputArray);
    void getExposureTime(QStringList &list);
    void radianceMap(std::vector<cv::Mat> &inputArrays, cv::Mat &outputArray = cv::Mat());


    //Overflow problem. Fix here: http://www.johndcook.com/blog/IEEE_exceptions_in_cpp/
    bool IsNumber(float x)
    {
        // This looks like it should always be true,
        // but it's false if x is a NaN.
        return (x == x);
    }

    bool IsFiniteNumber(double x)
    {
        return (x <= DBL_MAX && x >= -DBL_MAX);
    }
};

#endif // HDR_H
