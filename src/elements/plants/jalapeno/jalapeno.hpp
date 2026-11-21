#pragma once
#include "../plants.hpp"

class Jalapeno : public Plants
{
private:
    int fuse; // ticks until the row burns (GOTY mDoSpecialCountdown = 100)

public:
    Jalapeno(const int &_row, const int &_col);
    ~Jalapeno();
    void burn_row();
    void display(const int &_row) override;
    void action() override;
};
