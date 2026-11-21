#pragma once
#include "../plants.hpp"

class DoomShroom : public Plants
{
public:
    DoomShroom(const int &_row, const int &_col);
    ~DoomShroom();
    bool is_night() const;
    /*Tick the fuse; returns true the tick the blast should go off.*/
    bool fuse_blow();
    /*Blast aftermath: dies once the explosion segment has played out.*/
    void aftermath();
    bool exploded() const { return has_exploded; }
    void display(const int &_row) override;
    void action() override;

private:
    bool has_exploded = false;
};
