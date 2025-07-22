#pragma once

#include <Eigen/Dense>

static Eigen::Vector4f Vector4DLerp( Eigen::Vector4f& vec1, Eigen::Vector4f& vec2, float param_4 )
{
    float fVar1;
    float fVar2;
    float fVar3;
    float fVar4;
    float fVar5;
    float fVar6;
    float fVar7;

    fVar1 = vec1.y();
    fVar2 = vec1.z();
    fVar3 = vec1.w();
    fVar4 = vec2.y();
    fVar7 = 1.0f - param_4;
    fVar5 = vec2.z();
    fVar6 = vec2.w();

    Eigen::Vector4f param_1;
    param_1.x() = vec2.x() * fVar7 + vec1.x() * param_4;
    param_1.y() = fVar4 * fVar7 + fVar1 * param_4;
    param_1.z() = fVar5 * fVar7 + fVar2 * param_4;
    param_1.w() = fVar6 * fVar7 + fVar3 * param_4;
    return param_1;
}

static Eigen::Vector4f& Vector4DSaturate( Eigen::Vector4f& vec1 )
{
    for ( int32_t i = 0; i < 4; i++ ) {
        if ( 1.0f <= vec1[i] ) {
            vec1[i] = 1.0f;
        }
        if ( 0.0 > vec1[i] ) {
            vec1[i] = 0.0f;
        }
    }

    return vec1;
}
