#pragma once

struct OmegaTorqueTuple {
    float Omega;
    float Torque;
};

struct EngineSimulationDesc {
    float OmegaMaxTorque;
    float OmegaMax;
    float OmegaIdle;
    float OmegaMaxPower;

    float OmegaToStall;
    float TorqueMax;
    float TorqueMin;
    float Inertia;
    float FrictionLoss;
    float ThrottleIdle;

    float TimeIgnition;
    float TimeIgnitionFullThrottle;
    float TimeIgnitionNoThrottle;
    float TimeIgnitionThrottlePos;
    float TimeRevLimiterNoPower;

    float FakeCoef;
    float TorqueInertiaUp;
    float TorqueInertiaDown;
    float OmegaIgnition;
    float PowerMax;
    
    float TurboStart;
    float TurboEnd;
    float TurboOmegaMax;
    float TurboFriction;
    float TurboCoeff;
    float TurboAccelerationMax;
    float TurboPressureMax;
    float TurboInertia;
    float TurboOmegaPressureStable;
    float TurboOmegaPressureCoeff;
    
    float TempOptimum;
    float TempPowerCoeff;
    float TempCoolingCoeff;

    std::vector<OmegaTorqueTuple> Min;
    std::vector<OmegaTorqueTuple> Max;
};


enum class eEngineState {
    ES_Stalled = 0,
    ES_Igniting = 1,
    ES_Running = 2
};

enum class eIngitionStep {
    IS_Ignite = 0,
    IS_PressingThrottle = 1,
    IS_WaitingForTimer = 2
};

struct EngineSimulationSnapshot {
    eEngineState  State;
    eIngitionStep IgnitionState;
    float OmegaDelta;
    float IntertiaSum;
    float Omega;
    float OmegaUnknown;
    float PrevOmega;
    float RunningTime;
    float EngineTemp;
    float TurboOmega;
    float TurboAcceleration;
    float TurboPressure;
};

struct EngineSimulation {
    EngineSimulationDesc        Desc;
    EngineSimulationSnapshot    State;

    float InputThrottle;
    float InputResistiveTorque;
    
    float OutputTorque;
    float OutputTorqueInertial;
};
