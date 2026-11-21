#pragma once
#include "../plants.hpp"

class HypnoShroom : public Plants
{
public:
    HypnoShroom(const int &_row, const int &_col);
    ~HypnoShroom();
    bool is_night() const;
    void display(const int &_row) override;
    void action() override;
};
