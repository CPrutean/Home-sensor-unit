#include <Arduino.h>
namespace CONFIG {

    const uint8_t CUMAC[6]{0x08, 0xa6, 0xf7, 0x70, 0x10, 0x04};
    const uint8_t SUMAC[6][6]{{0x00, 0x4b, 0x12, 0x3e, 0x87, 0x64},{0x3c, 0x8a, 0x1f, 0xd3, 0xd6, 0xec}};

    const char* PMKKEY{"UcSzRpCEYAZBUsfm"};
    const char* SUMLMKKEY{"ld6WkDi5ifJKdUzP"};

    const char* SULMKKEYS[6]{"JzBg3Wd66svOcEy1", "eL1DnlAkoeQw1hNi"};


}

