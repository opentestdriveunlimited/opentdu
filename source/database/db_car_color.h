#pragma once

struct DBEntryCarColor
{
    uint64_t CarHashcode;
    uint64_t ColorID1;
    uint64_t ColorName;
    uint64_t ColorID2;
    uint64_t CallipersColor;
    int32_t Price;
    int32_t __PADDING__;
    uint64_t InteriorColors[15];
};
