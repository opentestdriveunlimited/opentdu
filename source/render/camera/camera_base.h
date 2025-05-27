#pragma once

class CameraBase {
public:
    CameraBase();
    ~CameraBase();

    void updateMatrix(const Eigen::Vector4f& position, const Eigen::Vector4f& upVector, const Eigen::Vector4f& lookAt);

protected:
    float fov;
    float invAspectRatio;
    float aspectRatio;
    float cosHalfFov;
    float sinHalfFov;
    float tanHalfFov;
    float focalInfos[3];

    Eigen::Matrix4f worldToCam;
    Eigen::Matrix4f invWorldToCam;

protected:
    void computeMatrices(const Eigen::Vector4f& position, const Eigen::Vector4f& upVector, const Eigen::Vector4f& viewVector);
};
