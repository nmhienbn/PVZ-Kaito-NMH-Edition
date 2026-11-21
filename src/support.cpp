#include "support.hpp"
#include <chrono>
#include <ctime>
#include <random>
using namespace std;

/*Process-wide RNG, lazily seeded: checks/tests can pin determinism by
setting the env var PVZ_RAND_SEED before the first rand() call (a global
constructor would run before main() and lock the wall-time seed in).*/
static mt19937 &rd()
{
    static mt19937 g = [] {
        if (const char *s = getenv("PVZ_RAND_SEED"))
            return mt19937((uint32_t)atoll(s));
        return mt19937(chrono::high_resolution_clock::now().time_since_epoch().count());
    }();
    return g;
}

/*
random int in [L;R]
*/
int rand(int L, int R)
{
    if (L > R)
        exit(-10);
    return rd()() % (R - L + 1) + L;
}

/*
Check if x in [L; R]
*/
bool is_in(const int &L, const int &x, const int &R)
{
    return L <= x && x <= R;
}