//------------------------------------------------
//               Ch10_05.cpp
//------------------------------------------------

#include "stdafx.h"
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <random>
#include "AlignedMem.h"

using namespace std;

extern "C" bool Avx2CalcRgbMinMax_(uint8_t* rgb[3], size_t num_pixels, uint8_t min_vals[3], uint8_t max_vals[3]);

void Init(uint8_t* rgb[3], size_t n, unsigned int seed)
{
    uniform_int_distribution<> ui_dist {5, 250};
    default_random_engine rng {seed};

    for (size_t i = 0; i < n; i++)
    {
        rgb[0][i] = (uint8_t)ui_dist(rng);
        rgb[1][i] = (uint8_t)ui_dist(rng);
        rgb[2][i] = (uint8_t)ui_dist(rng);
    }

    // Set known min & max values for validation purposes
    rgb[0][n / 4] = 4;   rgb[1][n / 2] = 1;       rgb[2][3 * n / 4] = 3;
    rgb[0][n / 3] = 254; rgb[1][2 * n / 5] = 251; rgb[2][n - 1] = 252;
}

bool Avx2CalcRgbMinMaxCpp(uint8_t* rgb[3], size_t num_pixels, uint8_t min_vals[3], uint8_t max_vals[3])
{
    // Make sure num_pixels is valid
    if ((num_pixels == 0) || (num_pixels % 32 != 0))
        return false;

    if (!AlignedMem::IsAligned(rgb[0], 32))
        return false;
    if (!AlignedMem::IsAligned(rgb[1], 32))
        return false;
    if (!AlignedMem::IsAligned(rgb[2], 32))
        return false;

    // Find the min and max of each color plane
    min_vals[0] = min_vals[1] = min_vals[2] = 255;
    max_vals[0] = max_vals[1] = max_vals[2] = 0;

   for (size_t i = 0; i < 3; i++)
   {
        for (size_t j = 0; j < num_pixels; j++)
        {
            if (rgb[i][j] < min_vals[i])
                min_vals[i] = rgb[i][j];
            else if (rgb[i][j] > max_vals[i])
                max_vals[i] = rgb[i][j];
        }
    }

    return true;
}

int main(void)
{
    const size_t n = 1024;
    uint8_t* rgb[3];
    uint8_t min_vals1[3], max_vals1[3];
    uint8_t min_vals2[3], max_vals2[3];

    AlignedArray<uint8_t> r(n, 32);
    AlignedArray<uint8_t> g(n, 32);
    AlignedArray<uint8_t> b(n, 32);

    rgb[0] = r.Data();
    rgb[1] = g.Data();
    rgb[2] = b.Data();

    Init(rgb, n, 219);
    Avx2CalcRgbMinMaxCpp(rgb, n, min_vals1, max_vals1);
    Avx2CalcRgbMinMax_(rgb, n, min_vals2, max_vals2);

    cout << "Results for Avx2CalcRgbMinMax\n\n";
    cout << "                R   G   B\n";
    cout << "-------------------------\n";

    cout << "min_vals1: ";
    cout << setw(4) << (int)min_vals1[0]  << ' ';
    cout << setw(4) << (int)min_vals1[1]  << ' ';
    cout << setw(4) << (int)min_vals1[2]  << '\n';
    cout << "min_vals2: ";
    cout << setw(4) << (int)min_vals2[0]  << ' ';
    cout << setw(4) << (int)min_vals2[1]  << ' ';
    cout << setw(4) << (int)min_vals2[2]  << "\n\n";

    cout << "max_vals1: ";
    cout << setw(4) << (int)max_vals1[0]  << ' ';
    cout << setw(4) << (int)max_vals1[1]  << ' ';
    cout << setw(4) << (int)max_vals1[2]  << '\n';
    cout << "max_vals2: ";
    cout << setw(4) << (int)max_vals2[0]  << ' ';
    cout << setw(4) << (int)max_vals2[1]  << ' ';
    cout << setw(4) << (int)max_vals2[2]  << "\n\n";

    return 0;
}
