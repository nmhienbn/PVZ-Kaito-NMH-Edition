#include "support.hpp"
#include <chrono>
#include <ctime>
#include <random>
using namespace std;

static mt19937 rd(chrono::high_resolution_clock::now().time_since_epoch().count());

/*
random int in [L;R]
*/
int rand(int L, int R)
{
    if (L > R)
        exit(-10);
    return rd() % (R - L + 1) + L;
}

/*
Check if x in [L; R]
*/
bool is_in(const int &L, const int &x, const int &R)
{
    return L <= x && x <= R;
}