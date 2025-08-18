#pragma once

class Camera {
public:
    inline float getFOV() const { return fov; }
    inline float getAspectRatio() const { return aspectRatio; }

public:
    Camera();
    ~Camera();

    void updateMatrix(const Eigen::Vector3f& position, const Eigen::Vector3f& upVector, const Eigen::Vector3f& lookAt);
    void setAspectRatio(const float width, const float height);

    void setDefaultMatrix2D();
    void setFOV(const float param_1);
    
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
    void computeMatrices(const Eigen::Vector3f& position, const Eigen::Vector3f& upVector, const Eigen::Vector3f& viewVector);
};
