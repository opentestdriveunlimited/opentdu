#pragma once

struct DBEntryRim
{
    uint64_t Hashcode;
    uint64_t MakeName;
    uint64_t RimName;
    uint64_t FriendlyName;
    int32_t FrontWidth;
    int32_t FrontHeight;
    int32_t FrontDiameter;
    int32_t RearWidth;
    int32_t RearHeight;
    int32_t RearDiameter;
    int32_t Price;
    int32_t SpokesCount;
    uint64_t ResourcePath;
    uint64_t ResourcePathFront;
    uint64_t ResourcePathRear;
    uint32_t Flags;
};
