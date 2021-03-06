//------------------------------------------------
//               Ch11_01.cpp
//------------------------------------------------

#include "stdafx.h"
#include <iostream>
#include <iomanip>
#include <memory>
#include <fstream>
#include <stdexcept>
#include "Ch11_01.h"

using namespace std;

extern "C" const int c_NumPtsMin = 32;
extern "C" const int c_NumPtsMax = 16 * 1024 * 1024;
extern "C" const int c_KernelSizeMin = 3;
extern "C" const int c_KernelSizeMax = 15;
unsigned int g_RngSeedVal = 97;

void Convolve1(void)
{
    const int n1 = 512;
    const float kernel[] { 0.0625f, 0.25f, 0.375f, 0.25f, 0.0625f };
    const int ks = sizeof(kernel) / sizeof(float);
    const int ks2 = ks / 2;
    const int n2 = n1 + ks2 * 2;

    // Create signal array
    unique_ptr<float[]> x1_up {new float[n1]};
    unique_ptr<float[]> x2_up {new float[n2]};
    float* x1 = x1_up.get();
    float* x2 = x2_up.get();

    CreateSignal(x1, n1, ks, g_RngSeedVal);
    PadSignal(x2, n2, x1, n1, ks2);

    // Perform convolutions
    const int num_pts = n1;
    unique_ptr<float[]> y1_up {new float[num_pts]};
    unique_ptr<float[]> y2_up {new float[num_pts]};
    unique_ptr<float[]> y3_up {new float[num_pts]};
    unique_ptr<float[]> y4_up {new float[num_pts]};
    float* y1 = y1_up.get();
    float* y2 = y2_up.get();
    float* y3 = y3_up.get();
    float* y4 = y4_up.get();

    bool rc1 = Convolve1Cpp(y1, x2, num_pts, kernel, ks);
    bool rc2 = Convolve1_(y2, x2, num_pts, kernel, ks);
    bool rc3 = Convolve1Ks5Cpp(y3, x2, num_pts, kernel, ks);
    bool rc4 = Convolve1Ks5_(y4, x2, num_pts, kernel, ks);

    cout << "Results for Convolve1\n";
    cout << "  rc1 = " << boolalpha << rc1 << '\n';
    cout << "  rc2 = " << boolalpha << rc2 << '\n';
    cout << "  rc3 = " << boolalpha << rc3 << '\n';
    cout << "  rc4 = " << boolalpha << rc4 << '\n';

    if (!rc1 || !rc2 || !rc3 || !rc4)
        return;

    // Save data
    const char* fn = "Ch11_01_Convolve1Results.csv";
    ofstream ofs(fn);

    if (ofs.bad())
        cout << "File create error - " << fn << '\n';
    else
    {
        const char* delim = ", ";

        ofs << fixed << setprecision(7);
        ofs << "i, x1, y1, y2, y3, y4\n";

        for (int i = 0; i < num_pts; i++)
        {
            ofs << setw(5) << i << delim;
            ofs << setw(10) << x1[i] << delim;
            ofs << setw(10) << y1[i] << delim;
            ofs << setw(10) << y2[i] << delim;
            ofs << setw(10) << y3[i] << delim;
            ofs << setw(10) << y4[i] << '\n';
        }

        ofs.close();
        cout << "\nConvolution results saved to file " << fn << '\n';
    }
}

bool Convolve1Cpp(float* y, const float* x, int num_pts, const float* kernel, int kernel_size)
{
    int ks2 = kernel_size / 2;

    if ((kernel_size & 1) == 0)
        return false;

    if (kernel_size < c_KernelSizeMin || kernel_size > c_KernelSizeMax)
        return false;

    if (num_pts < c_NumPtsMin || num_pts > c_NumPtsMax)
        return false;

    x += ks2;   // x points to first signal point

    for (int i = 0; i < num_pts; i++)
    {
        float sum = 0;

        for (int k = -ks2; k <= ks2; k++)
        {
            float x_val = x[i - k];
            float kernel_val = kernel[k + ks2];

            sum += kernel_val * x_val;
        }

        y[i] = sum;
    }

    return true;
}

bool Convolve1Ks5Cpp(float* y, const float* x, int num_pts, const float* kernel, int kernel_size)
{
    int ks2 = kernel_size / 2;

    if (kernel_size != 5)
        return false;

    if (num_pts < c_NumPtsMin || num_pts > c_NumPtsMax)
        return false;

    x += ks2;   // x points to first signal point

    for (int i = 0; i < num_pts; i++)
    {
        float sum = 0;
        int j = i + ks2;

        sum += x[j] * kernel[0];
        sum += x[j - 1] * kernel[1];
        sum += x[j - 2] * kernel[2];
        sum += x[j - 3] * kernel[3];
        sum += x[j - 4] * kernel[4];

        y[i] = sum;
    }

    return true;
}

int main()
{
    int ret_val = 1;

    try
    {
        Convolve1();
        Convolve1_BM();
        ret_val = 0;
    }

    catch (runtime_error& rte)
    {
        cout << "run_time exception has occurred\n";
        cout << rte.what() << '\n';
    }

    catch (...)
    {
        cout << "Unexpected exception has occurred\n";
    }

    return ret_val;
}
