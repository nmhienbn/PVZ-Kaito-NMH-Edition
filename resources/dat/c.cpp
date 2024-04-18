#include <bits/stdc++.h>
#include <nlohmann/json.hpp>
using namespace std;
using namespace nlohmann;
ifstream zombies_json(".\\dat\\zombies.json");
json zombie_json = json::parse(zombies_json);

class A
{
public:
    virtual void doSt() { cout << 1; }
    void doSt2() { doSt(); }
};

class B : public A
{
public:
    void doSt() { cout << 2; }
};

int main()
{
    const int DISPLAY_LOGO_TIME = 250;
    for (int clk = 0; clk < DISPLAY_LOGO_TIME; clk++)
    {
        int alpha = max(510 * (clk <= (DISPLAY_LOGO_TIME / 2) ? clk : DISPLAY_LOGO_TIME - clk) / DISPLAY_LOGO_TIME, 10);
        cout << alpha << '\n';
    }
}