#pragma once

class BaseCamera;

class VehicleCameras {
public:
    VehicleCameras( uint32_t carID );
    ~VehicleCameras();

private:
    std::vector<BaseCamera*> cameras;
    uint32_t vehicleID;
};
