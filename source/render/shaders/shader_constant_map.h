#pragma once


struct ConstantMap
{
    uint32_t IsPixelShader;
    uint32_t Type;
    int32_t* pConstants;

    ConstantMap()
        : IsPixelShader( 3 )
        , Type( 0 )
        , pConstants( nullptr ) {

    }

    void initialize(uint32_t param_2, uint32_t param_3) {
        // FUN_00603390
        pConstants = (int32_t*)AllocateFromRenderPool(0xec0);
        IsPixelShader = param_3;
        Type = param_2;

        for (uint32_t uVar2 = 0; uVar2 < 0xec0; uVar2 += 0x10) {
            pConstants[uVar2] = 0xffffffff;
        }
    }
};

struct ShaderConstantsMap {
    ConstantMap FloatPS;
    ConstantMap FlagsPS;
    ConstantMap IntegerPS;
    ConstantMap FloatVS;
    ConstantMap FlagsVS;
    ConstantMap IntegerVS;

    ShaderConstantsMap()
        : FloatPS()
        , FlagsPS()
        , IntegerPS()
        , FloatVS()
        , FlagsVS()
        , IntegerVS() {
        
    }

    void initialize() {
        // FUN_006033f0
        FloatVS.initialize(0, 0);
        FlagsVS.initialize(0, 2);
        IntegerVS.initialize(0, 1);
        
        FloatPS.initialize(1, 0);
        FlagsPS.initialize(1, 2);
        IntegerPS.initialize(1, 1);
    }

    void fillVertexFloatMap() {
        // FUN_006067d0
        FloatVS.pConstants[0x338] = 0;
        FloatVS.pConstants[0x339] = 0;
        FloatVS.pConstants[0x33a] = 0;
        FloatVS.pConstants[0x33c] = 1;
        FloatVS.pConstants[0x33d] = 0;
        FloatVS.pConstants[0x33e] = 0;
        FloatVS.pConstants[0x340] = 2;
        FloatVS.pConstants[0x341] = 0;
        FloatVS.pConstants[0x342] = 0;
        FloatVS.pConstants[0x344] = 3;
        FloatVS.pConstants[0x345] = 0;
        FloatVS.pConstants[0x346] = 0;
        FloatVS.pConstants[900] = 4;
        FloatVS.pConstants[0x385] = 0;
        FloatVS.pConstants[0x386] = 0;
        FloatVS.pConstants[0x388] = 5;
        FloatVS.pConstants[0x389] = 0;
        FloatVS.pConstants[0x38a] = 0;
        FloatVS.pConstants[0x38c] = 6;
        FloatVS.pConstants[0x38d] = 0;
        FloatVS.pConstants[0x38e] = 0;
        FloatVS.pConstants[0x390] = 7;
        FloatVS.pConstants[0x391] = 0;
        FloatVS.pConstants[0x392] = 0;
        *FloatVS.pConstants = 8;
        FloatVS.pConstants[1] = 0;
        FloatVS.pConstants[2] = 0;
        FloatVS.pConstants[4] = 0xc;
        FloatVS.pConstants[5] = 0;
        FloatVS.pConstants[6] = 0;
        FloatVS.pConstants[8] = 0x10;
        FloatVS.pConstants[9] = 0;
        FloatVS.pConstants[10] = 0;
        FloatVS.pConstants[0xc] = 0x14;
        FloatVS.pConstants[0xd] = 0;
        FloatVS.pConstants[0xe] = 0;
        FloatVS.pConstants[0x10] = 0x18;
        FloatVS.pConstants[0x11] = 0;
        FloatVS.pConstants[0x12] = 0;
        FloatVS.pConstants[0x14] = 0x1c;
        FloatVS.pConstants[0x15] = 0;
        FloatVS.pConstants[0x16] = 0;
        FloatVS.pConstants[0x18] = 0x20;
        FloatVS.pConstants[0x19] = 0;
        FloatVS.pConstants[0x1a] = 0;
        FloatVS.pConstants[0x24] = 0x24;
        FloatVS.pConstants[0x25] = 0;
        FloatVS.pConstants[0x26] = 0;
        FloatVS.pConstants[0x1c] = 0x28;
        FloatVS.pConstants[0x1d] = 0;
        FloatVS.pConstants[0x1e] = 0;
        FloatVS.pConstants[0x28] = 0x2c;
        FloatVS.pConstants[0x29] = 0;
        FloatVS.pConstants[0x2a] = 0;
        FloatVS.pConstants[0x20] = 0x30;
        FloatVS.pConstants[0x21] = 0;
        FloatVS.pConstants[0x22] = 0;
        FloatVS.pConstants[0x2c] = 0x34;
        FloatVS.pConstants[0x2d] = 0;
        FloatVS.pConstants[0x2e] = 0;
    }
};

static void FillShaderConstantsMap(ShaderConstantsMap& param_1)
{
    // FUN_006036f0
    param_1.initialize();
    param_1.fillVertexFloatMap();
    param_1.FloatVS.pConstants[0x30] = 0x54;
    param_1.FloatVS.pConstants[0x31] = 0;
    param_1.FloatVS.pConstants[0x32] = 0;
    param_1.FloatVS.pConstants[0x34] = 0x50;
    param_1.FloatVS.pConstants[0x35] = 0;
    param_1.FloatVS.pConstants[0x36] = 0;
    param_1.FloatVS.pConstants[0x38] = 0x4c;
    param_1.FloatVS.pConstants[0x39] = 0;
    param_1.FloatVS.pConstants[0x3a] = 0;
    param_1.FloatVS.pConstants[0x3c] = 0x3e;
    param_1.FloatVS.pConstants[0x3d] = 0;
    param_1.FloatVS.pConstants[0x3e] = 0;
    param_1.FloatVS.pConstants[0x40] = 0x44;
    param_1.FloatVS.pConstants[0x41] = 0;
    param_1.FloatVS.pConstants[0x42] = 0;
    param_1.FloatVS.pConstants[0x44] = 0x40;
    param_1.FloatVS.pConstants[0x45] = 0;
    param_1.FloatVS.pConstants[0x46] = 0;
    param_1.FloatVS.pConstants[0x48] = 0x32;
    param_1.FloatVS.pConstants[0x49] = 0;
    param_1.FloatVS.pConstants[0x4a] = 0;
    param_1.FloatVS.pConstants[0x4c] = 0x38;
    param_1.FloatVS.pConstants[0x4d] = 0;
    param_1.FloatVS.pConstants[0x4e] = 0;
    param_1.FloatVS.pConstants[0x50] = 0x70;
    param_1.FloatVS.pConstants[0x51] = 0;
    param_1.FloatVS.pConstants[0x52] = 0;
    param_1.FloatVS.pConstants[0xf8] = 0x58;
    param_1.FloatVS.pConstants[0xf9] = 0;
    param_1.FloatVS.pConstants[0xfa] = 0;
    param_1.FloatVS.pConstants[0x10c] = 0x59;
    param_1.FloatVS.pConstants[0x10d] = 0;
    param_1.FloatVS.pConstants[0x10e] = 0;
    param_1.FloatVS.pConstants[0x114] = 0x5a;
    param_1.FloatVS.pConstants[0x115] = 0;
    param_1.FloatVS.pConstants[0x116] = 0;
    param_1.FloatVS.pConstants[0x104] = 0x5b;
    param_1.FloatVS.pConstants[0x105] = 0;
    param_1.FloatVS.pConstants[0x106] = 0;
    param_1.FloatVS.pConstants[0x394] = 0xf1;
    param_1.FloatVS.pConstants[0x395] = 0;
    param_1.FloatVS.pConstants[0x396] = 0;
    param_1.FloatVS.pConstants[0x54] = 0x5c;
    param_1.FloatVS.pConstants[0x55] = 0;
    param_1.FloatVS.pConstants[0x56] = 0;
    param_1.FloatVS.pConstants[0x58] = 0x5d;
    param_1.FloatVS.pConstants[0x59] = 0;
    param_1.FloatVS.pConstants[0x5a] = 0;
    param_1.FloatVS.pConstants[0x5c] = 0x61;
    param_1.FloatVS.pConstants[0x5d] = 0;
    param_1.FloatVS.pConstants[0x5e] = 0;
    param_1.FloatVS.pConstants[0x60] = 0x5e;
    param_1.FloatVS.pConstants[0x61] = 0;
    param_1.FloatVS.pConstants[0x62] = 0;
    param_1.FloatVS.pConstants[100] = 0x5f;
    param_1.FloatVS.pConstants[0x65] = 0;
    param_1.FloatVS.pConstants[0x66] = 0;
    param_1.FloatVS.pConstants[0x68] = 0x60;
    param_1.FloatVS.pConstants[0x69] = 0;
    param_1.FloatVS.pConstants[0x6a] = 0;
    param_1.FloatVS.pConstants[0x6c] = 0x62;
    param_1.FloatVS.pConstants[0x6d] = 0;
    param_1.FloatVS.pConstants[0x6e] = 0;
    param_1.FloatVS.pConstants[0x74] = 100;
    param_1.FloatVS.pConstants[0x75] = 0;
    param_1.FloatVS.pConstants[0x76] = 0;
    param_1.FloatVS.pConstants[0x78] = 0x65;
    param_1.FloatVS.pConstants[0x79] = 0;
    param_1.FloatVS.pConstants[0x7a] = 0;
    param_1.FloatVS.pConstants[0x7c] = 0x69;
    param_1.FloatVS.pConstants[0x7d] = 0;
    param_1.FloatVS.pConstants[0x7e] = 0;
    param_1.FloatVS.pConstants[0x80] = 0x66;
    param_1.FloatVS.pConstants[0x81] = 0;
    param_1.FloatVS.pConstants[0x82] = 0;
    param_1.FloatVS.pConstants[0x84] = 0x67;
    param_1.FloatVS.pConstants[0x85] = 0;
    param_1.FloatVS.pConstants[0x86] = 0;
    param_1.FloatVS.pConstants[0x88] = 0x68;
    param_1.FloatVS.pConstants[0x89] = 0;
    param_1.FloatVS.pConstants[0x8a] = 0;
    param_1.FloatVS.pConstants[0x8c] = 0x6a;
    param_1.FloatVS.pConstants[0x8d] = 0;
    param_1.FloatVS.pConstants[0x8e] = 0;
    param_1.FloatVS.pConstants[0x3a8] = 0xf6;
    param_1.FloatVS.pConstants[0x3a9] = 0;
    param_1.FloatVS.pConstants[0x3aa] = 0;
    param_1.FloatVS.pConstants[0x3ac] = 0xf7;
    param_1.FloatVS.pConstants[0x3ad] = 0;
    param_1.FloatVS.pConstants[0x3ae] = 0;
    param_1.FloatVS.pConstants[0x1e8] = 0x72;
    param_1.FloatVS.pConstants[0x1e9] = 0;
    param_1.FloatVS.pConstants[0x1ea] = 0;
    param_1.FloatVS.pConstants[0x1ec] = 0x73;
    param_1.FloatVS.pConstants[0x1ed] = 0;
    param_1.FloatVS.pConstants[0x1ee] = 0;
    param_1.FloatVS.pConstants[0x1f0] = 0x74;
    param_1.FloatVS.pConstants[0x1f1] = 0;
    param_1.FloatVS.pConstants[0x1f2] = 0;
    param_1.FloatVS.pConstants[0x328] = 0x75;
    param_1.FloatVS.pConstants[0x329] = 0;
    param_1.FloatVS.pConstants[0x32a] = 0;
    param_1.FloatVS.pConstants[0x32c] = 0x76;
    param_1.FloatVS.pConstants[0x32d] = 0;
    param_1.FloatVS.pConstants[0x32e] = 0;
    param_1.FloatVS.pConstants[0x330] = 0x77;
    param_1.FloatVS.pConstants[0x331] = 0;
    param_1.FloatVS.pConstants[0x332] = 0;
    param_1.FloatVS.pConstants[0x36c] = 0xe1;
    param_1.FloatVS.pConstants[0x36d] = 0;
    param_1.FloatVS.pConstants[0x36e] = 0;
    param_1.FloatVS.pConstants[0x378] = 0xee;
    param_1.FloatVS.pConstants[0x379] = 0;
    param_1.FloatVS.pConstants[0x37a] = 0;
    param_1.FloatVS.pConstants[0x144] = 0x81;
    param_1.FloatVS.pConstants[0x145] = 0;
    param_1.FloatVS.pConstants[0x146] = 0;
    param_1.FloatVS.pConstants[0x13c] = 0x6b;
    param_1.FloatVS.pConstants[0x13d] = 0;
    param_1.FloatVS.pConstants[0x13e] = 0;
    param_1.FloatVS.pConstants[0x200] = 0x7b;
    param_1.FloatVS.pConstants[0x201] = 0;
    param_1.FloatVS.pConstants[0x202] = 0;
    param_1.FloatVS.pConstants[0x140] = 0x71;
    param_1.FloatVS.pConstants[0x141] = 0;
    param_1.FloatVS.pConstants[0x142] = 0;
    param_1.FloatVS.pConstants[0x334] = 0x78;
    param_1.FloatVS.pConstants[0x335] = 0;
    param_1.FloatVS.pConstants[0x336] = 0;
    param_1.FloatVS.pConstants[0x348] = 0x79;
    param_1.FloatVS.pConstants[0x349] = 0;
    param_1.FloatVS.pConstants[0x34a] = 0;
    param_1.FloatVS.pConstants[0x34c] = 0x7a;
    param_1.FloatVS.pConstants[0x34d] = 0;
    param_1.FloatVS.pConstants[0x34e] = 0;
    param_1.FloatVS.pConstants[0x364] = 0x7c;
    param_1.FloatVS.pConstants[0x365] = 0;
    param_1.FloatVS.pConstants[0x366] = 0;
    param_1.FloatVS.pConstants[0x398] = 0xf2;
    param_1.FloatVS.pConstants[0x399] = 0;
    param_1.FloatVS.pConstants[0x39a] = 0;
    param_1.FloatVS.pConstants[0x39c] = 0xf3;
    param_1.FloatVS.pConstants[0x39d] = 0;
    param_1.FloatVS.pConstants[0x39e] = 0;
    param_1.FloatVS.pConstants[0x3a0] = 0xf4;
    param_1.FloatVS.pConstants[0x3a1] = 0;
    param_1.FloatVS.pConstants[0x3a2] = 0;
    param_1.FloatVS.pConstants[0x3a4] = 0xf5;
    param_1.FloatVS.pConstants[0x3a5] = 0;
    param_1.FloatVS.pConstants[0x3a6] = 0;
    param_1.FloatVS.pConstants[0x2f4] = 0x7d;
    param_1.FloatVS.pConstants[0x2f5] = 0;
    param_1.FloatVS.pConstants[0x2f6] = 0;
    param_1.FloatVS.pConstants[0x2ec] = 0xea;
    param_1.FloatVS.pConstants[0x2ed] = 0;
    param_1.FloatVS.pConstants[0x2ee] = 0;
    param_1.FloatVS.pConstants[0x370] = 0xe2;
    param_1.FloatVS.pConstants[0x371] = 0;
    param_1.FloatVS.pConstants[0x372] = 0;
    param_1.FloatVS.pConstants[0x374] = 0xe6;
    param_1.FloatVS.pConstants[0x375] = 0;
    param_1.FloatVS.pConstants[0x376] = 0;
    param_1.FloatVS.pConstants[0x148] = 0x6d;
    param_1.FloatVS.pConstants[0x149] = 0;
    param_1.FloatVS.pConstants[0x14a] = 0;
    param_1.FloatVS.pConstants[0x37c] = 0xef;
    param_1.FloatVS.pConstants[0x37d] = 0;
    param_1.FloatVS.pConstants[0x37e] = 0;
    param_1.FloatVS.pConstants[0x380] = 0xf0;
    param_1.FloatVS.pConstants[0x381] = 0;
    param_1.FloatVS.pConstants[0x382] = 0;
    param_1.IntegerVS.pConstants[0x19c] = 0;
    param_1.IntegerVS.pConstants[0x19d] = 0;
    param_1.IntegerVS.pConstants[0x19e] = 0;
    param_1.IntegerVS.pConstants[0x1a0] = 1;
    param_1.IntegerVS.pConstants[0x1a1] = 0;
    param_1.IntegerVS.pConstants[0x1a2] = 0;
    param_1.IntegerVS.pConstants[0x1a4] = 2;
    param_1.IntegerVS.pConstants[0x1a5] = 0;
    param_1.IntegerVS.pConstants[0x1a6] = 0;
    param_1.FlagsVS.pConstants[400] = 1;
    param_1.FlagsVS.pConstants[0x191] = 0;
    param_1.FlagsVS.pConstants[0x192] = 0;
    param_1.FlagsVS.pConstants[0x194] = 0;
    param_1.FlagsVS.pConstants[0x195] = 0;
    param_1.FlagsVS.pConstants[0x196] = 0;
    param_1.FloatPS.pConstants[0x138] = 0xe;
    param_1.FloatPS.pConstants[0x139] = 0;
    param_1.FloatPS.pConstants[0x13a] = 0;
    param_1.FloatPS.pConstants[100] = 1;
    param_1.FloatPS.pConstants[0x65] = 0;
    param_1.FloatPS.pConstants[0x66] = 0;
    param_1.FloatPS.pConstants[0x68] = 2;
    param_1.FloatPS.pConstants[0x69] = 0;
    param_1.FloatPS.pConstants[0x6a] = 0;
    param_1.FloatPS.pConstants[0x60] = 0;
    param_1.FloatPS.pConstants[0x61] = 0;
    param_1.FloatPS.pConstants[0x62] = 0;
    param_1.FloatPS.pConstants[0x6c] = 3;
    param_1.FloatPS.pConstants[0x6d] = 0;
    param_1.FloatPS.pConstants[0x6e] = 0;
    param_1.FloatPS.pConstants[0xf8] = 4;
    param_1.FloatPS.pConstants[0xf9] = 0;
    param_1.FloatPS.pConstants[0xfa] = 0;
    param_1.FloatPS.pConstants[0x84] = 6;
    param_1.FloatPS.pConstants[0x85] = 0;
    param_1.FloatPS.pConstants[0x86] = 0;
    param_1.FloatPS.pConstants[0x88] = 7;
    param_1.FloatPS.pConstants[0x89] = 0;
    param_1.FloatPS.pConstants[0x8a] = 0;
    param_1.FloatPS.pConstants[0x80] = 5;
    param_1.FloatPS.pConstants[0x81] = 0;
    param_1.FloatPS.pConstants[0x82] = 0;
    param_1.FloatPS.pConstants[0x378] = 0x1b;
    param_1.FloatPS.pConstants[0x379] = 0;
    param_1.FloatPS.pConstants[0x37a] = 0;
    param_1.FloatPS.pConstants[0x1e8] = 0xf;
    param_1.FloatPS.pConstants[0x1e9] = 0;
    param_1.FloatPS.pConstants[0x1ea] = 0;
    param_1.FloatPS.pConstants[0x1ec] = 0x10;
    param_1.FloatPS.pConstants[0x1ed] = 0;
    param_1.FloatPS.pConstants[0x1ee] = 0;
    param_1.FloatPS.pConstants[0x1f0] = 0x11;
    param_1.FloatPS.pConstants[0x1f1] = 0;
    param_1.FloatPS.pConstants[0x1f2] = 0;
    param_1.FloatPS.pConstants[8] = 8;
    param_1.FloatPS.pConstants[9] = 0;
    param_1.FloatPS.pConstants[10] = 0;
    param_1.FloatPS.pConstants[0x350] = 0x12;
    param_1.FloatPS.pConstants[0x351] = 0;
    param_1.FloatPS.pConstants[0x352] = 0;
    param_1.FloatPS.pConstants[0x354] = 0x13;
    param_1.FloatPS.pConstants[0x355] = 0;
    param_1.FloatPS.pConstants[0x356] = 0;
    param_1.FloatPS.pConstants[0x358] = 0x14;
    param_1.FloatPS.pConstants[0x359] = 0;
    param_1.FloatPS.pConstants[0x35a] = 0;
    param_1.FloatPS.pConstants[0x35c] = 0x15;
    param_1.FloatPS.pConstants[0x35d] = 0;
    param_1.FloatPS.pConstants[0x35e] = 0;
    param_1.FloatPS.pConstants[0x360] = 0x16;
    param_1.FloatPS.pConstants[0x361] = 0;
    param_1.FloatPS.pConstants[0x362] = 0;
    param_1.FloatPS.pConstants[0x13c] = 0xc;
    param_1.FloatPS.pConstants[0x13d] = 0;
    param_1.FloatPS.pConstants[0x13e] = 0;
    param_1.FloatPS.pConstants[0x200] = 0xd;
    param_1.FloatPS.pConstants[0x201] = 0;
    param_1.FloatPS.pConstants[0x202] = 0;
    param_1.FloatPS.pConstants[0x2ec] = 0x17;
    param_1.FloatPS.pConstants[0x2ed] = 0;
    param_1.FloatPS.pConstants[0x2ee] = 0;
    param_1.FloatPS.pConstants[0x2f0] = 0x18;
    param_1.FloatPS.pConstants[0x2f1] = 0;
    param_1.FloatPS.pConstants[0x2f2] = 0;
    param_1.FloatPS.pConstants[0x2f8] = 0x19;
    param_1.FloatPS.pConstants[0x2f9] = 0;
    param_1.FloatPS.pConstants[0x2fa] = 0;
    param_1.FloatPS.pConstants[0x2fc] = 0x1a;
    param_1.FloatPS.pConstants[0x2fd] = 0;
    param_1.FloatPS.pConstants[0x2fe] = 0;
    param_1.FloatPS.pConstants[0x3a8] = 0x1c;
    param_1.FloatPS.pConstants[0x3a9] = 0;
    param_1.FloatPS.pConstants[0x3aa] = 0;
    param_1.FloatPS.pConstants[0x3ac] = 0x1d;
    param_1.FloatPS.pConstants[0x3ad] = 0;
    param_1.FloatPS.pConstants[0x3ae] = 0;
}

static void FillShaderConstantsMap1(ShaderConstantsMap& param_1)
{
    // FUN_006034c0
    param_1.initialize();
    param_1.fillVertexFloatMap();                                                        
    param_1.FloatVS.pConstants[0x1f8] = 0x38;
    param_1.FloatVS.pConstants[0x1f9] = 0;
    param_1.FloatVS.pConstants[0x1fa] = 0;
    param_1.FloatVS.pConstants[600] = 0x39;
    param_1.FloatVS.pConstants[0x259] = 0;
    param_1.FloatVS.pConstants[0x25a] = 0;
    param_1.FloatVS.pConstants[0x22c] = 0x3a;
    param_1.FloatVS.pConstants[0x22d] = 0;
    param_1.FloatVS.pConstants[0x22e] = 0;
    param_1.FloatVS.pConstants[0x230] = 0x3b;
    param_1.FloatVS.pConstants[0x231] = 0;
    param_1.FloatVS.pConstants[0x232] = 0;
    param_1.FloatVS.pConstants[0x234] = 0x3c;
    param_1.FloatVS.pConstants[0x235] = 0;
    param_1.FloatVS.pConstants[0x236] = 0;
    param_1.FloatVS.pConstants[0x238] = 0x3d;
    param_1.FloatVS.pConstants[0x239] = 0;
    param_1.FloatVS.pConstants[0x23a] = 0;
    param_1.FloatPS.pConstants[0x1fc] = 0;
    param_1.FloatPS.pConstants[0x1fd] = 0;
    param_1.FloatPS.pConstants[0x1fe] = 0;
    param_1.FloatPS.pConstants[0x220] = 0xe;
    param_1.FloatPS.pConstants[0x221] = 0;
    param_1.FloatPS.pConstants[0x222] = 0;
    param_1.FloatPS.pConstants[0x23c] = 3;
    param_1.FloatPS.pConstants[0x23d] = 0;
    param_1.FloatPS.pConstants[0x23e] = 0;
    param_1.FloatPS.pConstants[0x240] = 4;
    param_1.FloatPS.pConstants[0x241] = 0;
    param_1.FloatPS.pConstants[0x242] = 0;
    param_1.FloatPS.pConstants[0x244] = 5;
    param_1.FloatPS.pConstants[0x245] = 0;
    param_1.FloatPS.pConstants[0x246] = 0;
    param_1.FloatPS.pConstants[0x248] = 6;
    param_1.FloatPS.pConstants[0x249] = 0;
    param_1.FloatPS.pConstants[0x24a] = 0;
    param_1.FloatPS.pConstants[0x224] = 1;
    param_1.FloatPS.pConstants[0x225] = 0;
    param_1.FloatPS.pConstants[0x226] = 0;
    param_1.FloatPS.pConstants[0x228] = 2;
    param_1.FloatPS.pConstants[0x229] = 0;
    param_1.FloatPS.pConstants[0x22a] = 0;
    param_1.FloatPS.pConstants[0x24c] = 7;
    param_1.FloatPS.pConstants[0x24d] = 0;
    param_1.FloatPS.pConstants[0x24e] = 0;
    param_1.FloatPS.pConstants[0x250] = 8;
    param_1.FloatPS.pConstants[0x251] = 0;
    param_1.FloatPS.pConstants[0x252] = 0;
    param_1.FloatPS.pConstants[0x254] = 0xf;
    param_1.FloatPS.pConstants[0x255] = 0;
    param_1.FloatPS.pConstants[0x256] = 0;
}