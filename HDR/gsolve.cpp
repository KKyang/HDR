#include "gsolve.h"
#include <iomanip>
#include <fstream>

cv::Mat GSolve::G_lnE(vector< vector<int> > Z,vector<double> ln_time,double lambda)
{
    int n = 256;
    vector<vector<double>> A;
    vector<double> b;
    for (int i = 0; i < Z.size() * Z.at(0).size() + n + 1; i++) {
        vector< double > temp(n + Z.size(), 0);
        A.push_back(temp);
    }
    b.resize(A.size());

    int k = 0;
    for (int i = 0; i < Z.size(); i++) {
        for (int j = 0; j < Z.at(0).size()  ; j++) {
            double wij = Weighting(Z[i][j] + 1);
            A[k][Z[i][j]] = wij;
            A[k][n + i] = -wij;

            b[k] = wij *log(ln_time[j])/log(2.718281828) ;  //換成ln
            k++;
        }
    }
    std::cout << "\n";

    A[k][128] = 1;//?????
    k++;

    //x = A\b;  (matlab)

    for (int i = 0; i < n-1; i++) {
        A[k][i] = lambda * Weighting(i + 2);
        A[k][i + 1] = -2 * lambda * Weighting(i + 2);
        A[k][i + 2] = lambda * Weighting(i + 2);
        k++;
    }

    cv::Mat AA(A.size(), A[0].size(),CV_64F);
    for (int i = 0; i < AA.rows; i++) {
        for (int j = 0; j < AA.cols; j++) {
            AA.at<double>(i, j) = A[i][j];
        }
    }
//Example: http://stackoverflow.com/questions/13699901/convert-vector-to-mat-in-opencv
//    int main ( ... )
//    {
//         cv::vector < Component > components;
//         cv::Mat componentMat ( components, true );
//         std::cout << componentMat;
//         return 0;
//    }  // Copy vector to the cv::Mat
    cv::Mat bb(b,true);

    ofstream file;
    file.open("fugu.txt");
    for(int i= 0;i<A.size();i++)
    {
        bb.at<double>(i) = b[i];
    }
    for(int i = 0; i < A.size(); i++)
    {
        for(int j = 0; j < A[0].size(); j++)
        {
                file << std::setw(5) << (double)A[i][j] << ",";
        }
        file << "\n";
    }
    file.close();

    cv::Mat x(AA.rows, 1,CV_64F);
    cv::solve(AA, bb, x, cv::DECOMP_SVD);//AA bb  x is result   cv::DECOMP_SVD:SVD method
    //輸出coef的地方

//    for(int i =0;i<n;i++)
//    {
//        qDebug()<<"coef "<<i<<" "<<x.at<double>(i,0);
//    }

    return x;
}

double GSolve::Weighting(int z)
{
    //HDR ppt page32 w defination
    if (z <= 0.5*(ZMIN+ZMAX + 1))
        return (z - ZMIN) / 128.0;
    else
        return (ZMAX + 1 - z) / 128.0;
}

