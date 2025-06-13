#pragma once

struct ColorRGBA {
    union {
        struct {
            uint8_t R;
            uint8_t G;
            uint8_t B;
            uint8_t A;
        };
        uint32_t Color;
    };

    constexpr ColorRGBA& operator = (const ColorRGBA other)
    {
        this->Color = other.Color;
        return *this;
    }

    constexpr uint8_t& operator[] (const int32_t i)
    {
        switch(i) {
        case 0: return R;
        case 1: return G;
        case 2: return B;
        case 3: return A;
        default: 
            OTDU_ASSERT(false);
            return R;
        }
    }
    constexpr ColorRGBA( const uint8_t r = 0x00,
                         const uint8_t g = 0x00,
                         const uint8_t b = 0x00,
                         const uint8_t a = 0xff )
        : R( r )
        , G( g )
        , B( b )
        , A( a )
    {

    }

    constexpr ColorRGBA( const uint32_t color )
        : Color( color )
    {

    }
};

static constexpr ColorRGBA gColorBlackNoAlpha( 0x00, 0x00, 0x00, 0x00 );
static constexpr ColorRGBA gColorBlack( 0x00, 0x00, 0x00 );
static constexpr ColorRGBA gColorRed(   0xff, 0x00, 0x00 );
static constexpr ColorRGBA gColorBlue(  0x00, 0xff, 0x00 );
static constexpr ColorRGBA gColorGreen( 0x00, 0x00, 0xff );
static constexpr ColorRGBA gColorWhite( 0xff, 0xff, 0xff );
