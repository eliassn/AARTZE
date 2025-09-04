#pragma once

struct AnimationBlendComponent
{
    int clipA{-1};
    int clipB{-1};
    float alpha{0.0f}; // 0..1 blend weight
    float speed{1.0f};
    bool loop{true};
    float time{0.0f};
};

