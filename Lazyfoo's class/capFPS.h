#ifndef capFPS_H
#define capFPS_H

#include <SDL.h>
#include "LTimer.h"

class FPScontrol
{
public:
    FPScontrol();
    ~FPScontrol();

private:
    // The frames per second timer
    LTimer fpsTimer;

    // The frames per second cap timer
    LTimer capTimer;

    // Start counting frames per second
    int countedFrames = 0;
};

#endif // capFPS_H