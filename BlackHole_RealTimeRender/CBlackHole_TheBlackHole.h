// CBlackHole_TheBlackHole.h
// 定义场景中的黑洞
#pragma once

class TheBlackHole {
private:
    float mass;           // 质量
    float spin;           // 自旋，现在先做施瓦西黑洞暂时不自旋

public:
    TheBlackHole() :mass(1.0), spin(0) {};
    TheBlackHole(float m, float s) :mass(m), spin(s) {};
    inline void set(float m, float s) {
        mass = m; spin = s;
    }
    inline float getMass() const { return mass; }
    inline float getSpin() const { return spin; }
};