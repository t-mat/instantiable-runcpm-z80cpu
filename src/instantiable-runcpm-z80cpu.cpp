// g++ -std=c++17 main.cpp
// ./a.out zexall.com
#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

struct RunCpmCpu {
    const uint8_t IO_char = 0xf2;

    uint8_t RAM[65536];

    uint8_t* _RamSysAddr(uint16_t address) {
        return &RAM[address];
    }

    uint8_t _RamRead(uint16_t address) {
        return *_RamSysAddr(address);
    }

    uint16_t _RamRead16(uint16_t address) {
        return _RamRead(address) + _RamRead(address + 1) * 256;
    }

    void _RamWrite(uint16_t address, uint8_t value) {
        *_RamSysAddr(address) = value;
    }

    void _RamWrite16(uint16_t address, uint16_t value) {
        _RamWrite(address, value & 0xff);
        _RamWrite(address + 1, (value >> 8) & 0xff);
    }

    void ramStore(uint16_t address, const uint8_t* src, size_t srcSizeInBytes) {
        for(size_t i = 0; i < srcSizeInBytes; ++i) {
            _RamWrite(static_cast<uint16_t>(address + i), src[i]);
        }
    }

    void cpu_out(uint32_t port, uint32_t value) {
        if(port == IO_char) {
            putchar(value);
        }
    }

    uint32_t cpu_in(uint32_t port) {
        return(HIGH_REGISTER(AF));
    }

    void setupPicoBdos() {
        const uint16_t bdosAddr = 0xfc00;

        const uint8_t boot[] = {
            0x3e, 0x24,     // ld   a,0x24
            0xD3, IO_char,  // out  (IO_char),a
            0x76,           // halt
            0xc3,
            bdosAddr & 0xff,
            bdosAddr >> 8,
        };

        const uint8_t picoBdos[] = {
                            //  .org 0xfc00
            0x79,           //      ld  a,c
            0xFE, 0x09,     //      cp  0x09
            0x28, 0x04,     //      jr  z,WRITESTR
            0x7B,           //      ld  a,e
            0xD3, IO_char,  //      out (IO_char),a
            0xC9,           //      ret
                            // WRITESTR:
            0xEB,           //      ex de,hl
                            // LP:
            0x7E,           //      ld  a,(hl)
            0xFE, 0x24,     //      cp  0x24
            0xC8,           //      ret z
            0xD3, IO_char,  //      out (IO_char),a
            0x23,           //      inc hl
            0x18, 0xF7      //      jr  LP
        };

        ramStore(0, boot, sizeof(boot));
        ramStore(bdosAddr, picoBdos, sizeof(picoBdos));
    }

    int callbackCounter = 0;

    void Z80run_callback() {
        if((++callbackCounter % (1024 * 1024 * 64)) == 0) {
            printf("PC=%04X, SP=%04X, AF=%04X, BC=%04X, DE=%04X, HL=%04X\n", PCX, SP, AF, BC, DE, HL);
        }
    }

    void _RamLoad(const char* filename, uint16_t address) {
        if(FILE* fp = fopen(filename, "rb")) {
            fseek(fp, 0, SEEK_END);
            const long l = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            const auto r = fread(_RamSysAddr(address), 1, l, fp);
            assert(l == r);
            fclose(fp);
        }
    }

    uint8_t LOW_DIGIT(uint32_t x) { return x & 0xf; }
    uint8_t HIGH_DIGIT(uint32_t x) { return (((x) >> 4) & 0xf); }
    uint8_t LOW_REGISTER(uint32_t x) { return ((x) & 0xff); }
    uint8_t HIGH_REGISTER(uint32_t x) { return (((x) >> 8) & 0xff); }

    #if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable : 5033)
    #endif
    #define SET_LOW_REGISTER(x, v)  x = (((x) & 0xff00) | ((v) & 0xff))
    #define SET_HIGH_REGISTER(x, v) x = (((x) & 0xff) | (((v) & 0xff) << 8))

    void SetA(uint8_t v) { SET_HIGH_REGISTER(AF, v); }
    void SetF(uint8_t v) { SET_LOW_REGISTER (AF, v); }
    void SetB(uint8_t v) { SET_HIGH_REGISTER(BC, v); }
    void SetC(uint8_t v) { SET_LOW_REGISTER (BC, v); }
    void SetD(uint8_t v) { SET_HIGH_REGISTER(DE, v); }
    void SetE(uint8_t v) { SET_LOW_REGISTER (DE, v); }
    void SetH(uint8_t v) { SET_HIGH_REGISTER(HL, v); }
    void SetL(uint8_t v) { SET_LOW_REGISTER (HL, v); }

    #define INSTANTIABLE_CPU_IN_OUT_DEFINED 1
    #define INSTANTIABLE_CPU_Z80RUN_CALLBACK 1
    #define INSTANTIABLE_CPU_INLINE inline
    #define INSTANTIABLE_CPU_NON_PUBLIC(x)
    #define register

    using int8 = int8_t;
    using uint8 = uint8_t;
    using int16 = int16_t;
    using uint16 = uint16_t;
    using int32 = int32_t;
    using uint32 = uint32_t;

    #include "runcpm/cpu.h"

    #undef register
    #undef INSTANTIABLE_CPU_NON_PUBLIC
    #undef INSTANTIABLE_CPU_INLINE
    #undef INSTANTIABLE_CPU_Z80RUN_CALLBACK
    #undef INSTANTIABLE_CPU_IN_OUT_DEFINED
    #undef SET_HIGH_REGISTER
    #undef SET_LOW_REGISTER
    #if defined(_MSC_VER)
    #pragma warning(pop)
    #endif
};

int main(int argc, const char** argv) {
    const char* zexallFilename = "external/zexall/zexall.com";
    for(int i = 1; i < argc; ++i) {
        zexallFilename = argv[i];
    }

    RunCpmCpu cpu;
    cpu.Status = 0;
    cpu._RamLoad(zexallFilename, 0x0100);
    cpu.setupPicoBdos();
    cpu.Z80reset();
    cpu.PC = 0x0100;
    cpu.Z80run();
}
