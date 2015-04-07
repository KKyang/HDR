#ifndef MYHDR_H
#define MYHDR_H
#include <vector>
#include <opencv/cv.h>
#include "opencv2/opencv.hpp"
#include <QDebug>
using namespace std;



class GSolve
{
private:
public:
    cv::Mat G_lnE(std::vector<vector<int> > Zij,vector<double> ln_time,double lambda);
    void PrintMatrix(CvMat *Matrix,int Rows,int Cols);
    double Weighting(int  z);
    int ZMAX = 255, ZMIN = 0;

};

#endif // MYHDR_H
