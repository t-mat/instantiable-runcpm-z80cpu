// Instantiable RunCPM Z80 CPU
// Copyright (C) 2021, Takayuki Matsuoka.
// SPDX-License-Identifier: MIT
// ----
// g++ -std=c++17 main.cpp
// ./a.out zexall.com
#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <memory>
#include <functional>

#include "runcpm_cpu.hpp"

void setupMinimumBdosForZexall(uint16_t bdosAddr, uint8_t IO_char, const std::function<void(uint16_t, uint8_t)>& memWrite) {
	const uint8_t boot[] = {
			            //  .org 0x0000
		0x3e, 0x24,     // ld   a,0x24
		0xD3, IO_char,  // out  (IO_char),a
		0x76,           // halt
		0xc3,			// jp   bdosAddr
		static_cast<uint8_t>(bdosAddr & 0xff),
		static_cast<uint8_t>(bdosAddr >> 8),
	};

	const uint8_t bdos[] = {
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

	for(int i = 0; i < std::ssize(boot); ++i) { memWrite(i, boot[i]); }
	for(int i = 0; i < std::ssize(bdos); ++i) { memWrite(bdosAddr + i, bdos[i]); }
}


// Example computer system which connects CPU, RAM and I/O ports.
struct MySystem {
	MySystem() {
		ram.resize(64*1024);
		cpu.memRead 	= [&](uint16_t addr) { return memRead(addr); };
		cpu.memWrite	= [&](uint16_t addr, uint8_t data) { memWrite(addr, data); };
		cpu.ioInp 		= [&](uint16_t addr) { return ioInp(addr); };
		cpu.ioOut 		= [&](uint16_t addr, uint8_t data) { ioOut(addr, data); };
		cpu.reset();
		setupMinimumBdosForZexall(0xfc00, IO_char, [&](uint16_t addr, uint8_t data) { memWrite(addr, data); });
	}

	void loadCimFile(uint16_t addr, const uint8_t* file, size_t fileSizeInBytes) {
		loadData(addr, file, fileSizeInBytes);
		auto s = cpu.getCpuStatus();
		s.PC = addr;
		cpu.setCpuStatus(s);
	}

	void loadData(uint16_t addr, const uint8_t* data, size_t dataSizeInBytes) {
		for(size_t i = 0; i < dataSizeInBytes; ++i) {
			memWrite(static_cast<uint16_t>(addr + i), data[i]);
		}
	}

	uint8_t memRead(uint16_t addr) {
		return ram[addr];
	}

	void memWrite(uint16_t addr, uint8_t data) {
		ram[addr] = data;
	}

	uint8_t ioInp(uint16_t addr) {
		return 0;
	}

	void ioOut(uint16_t addr, uint8_t data) {
	    if(addr == IO_char) {
	        putchar(data);
	    }
	}

	void runSingleCycle() {
	    if((callbackCounter % (1024 * 1024 * 64)) == 0) {
			const auto s = cpu.getCpuStatus();
	        printf("Cycle=%16zd, PC=%04X, SP=%04X, AF=%04X, BC=%04X, DE=%04X, HL=%04X\n", callbackCounter, s.PC, s.SP, s.AF, s.BC, s.DE, s.HL);
	    }
		cpu.runSingleCycle();
		++callbackCounter;
		isHalt = (cpu.getCpuStatus().getHALT() != 0);
	}

	RunCpmZ80Cpu::CpuStatus getCpuStatus() const {
		return cpu.getCpuStatus();
	}

	RunCpmZ80Cpu::Cpu cpu;
	std::vector<uint8_t> ram;
	int64_t callbackCounter = 0;
	bool isHalt = false;
	const uint8_t IO_char = 0xf2;
};


std::vector<uint8_t> loadFile(const char* filename) {
	std::vector<uint8_t> buf;
	if(FILE* fp = fopen(filename, "rb")) {
	    fseek(fp, 0, SEEK_END);
		buf.resize(ftell(fp));
	    fseek(fp, 0, SEEK_SET);
	    const auto r = fread(buf.data(), 1, buf.size(), fp);
	    fclose(fp);
	}
	return buf;
}


int main(int argc, const char** argv) {
    const char* zexallFilename = "external/zexall/zexall.com";
    for(int i = 1; i < argc; ++i) {
        zexallFilename = argv[i];
    }

	const std::vector<uint8_t> zexallFile = loadFile(zexallFilename);

	MySystem s;
	s.loadCimFile(0x0100, zexallFile.data(), zexallFile.size());
	while(! s.isHalt) {
		s.runSingleCycle();
	}
}
