#pragma once

struct State
{
    int speedProfile = 1;
    bool fsdEnabled = false;
};

struct LegacyState : State
{
};

struct Hw3State : State
{
    int speedOffset = 0;
};

struct Hw4State : State
{
    bool isaSpeedChimeSuppress = true;
};
