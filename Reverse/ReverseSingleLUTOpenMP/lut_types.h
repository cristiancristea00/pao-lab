#ifndef LUT_TYPES_H
#define LUT_TYPES_H

#include <cstdint>

#define PACKED    __attribute__((packed))


using lut32_t = union LUT32_STRUCT
{
    uint32_t value;
    struct PACKED
    {
        uint16_t lsb16 : 16;
        uint16_t msb16 : 16;
    };
};

using lut16_t = union LUT16_STRUCT
{
    uint16_t value;
    struct PACKED
    {
        uint8_t lsb8 : 8;
        uint8_t msb8 : 8;
    };
};

using lut8_t = union LUT8_STRUCT
{
    uint8_t value;
    struct PACKED
    {
        uint8_t lsb4 : 4;
        uint8_t msb4 : 4;
    };
};

using lut4_t = union LUT4_STRUCT
{
    uint8_t value: 4;
    struct PACKED
    {
        uint8_t lsb2: 2;
        uint8_t msb2: 2;
    };
};

#endif // LUT_TYPES_H

