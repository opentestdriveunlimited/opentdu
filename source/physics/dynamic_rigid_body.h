#pragma once

class PhysFactory;
	
struct BodyDesc {
    Eigen::Matrix4f InitialTransform;
    Eigen::Vector4f InertiaTensor[3];
    float Mass;
    uint32_t MotionFlags;
};

struct DynamicRigidBodyDesc {
    uint64_t Hashcode;
    uint32_t Index;
    uint32_t Flags;
};

class DynamicRigidBody {
public:

private:
    uint32_t index;
    uint32_t flags;
    uint64_t hashcode;
    uint32_t unknown;

    PhysFactory* pPhysFactory;
};
