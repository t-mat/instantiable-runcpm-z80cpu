// Instantiable RunCPM Z80 CPU
// Copyright (C) 2021, Takayuki Matsuoka.
// SPDX-License-Identifier: MIT
// ----
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <memory>
#include <functional>
#include "runcpm_cpu.hpp"

namespace RunCpmZ80Cpu {
	struct CpuImpl {
		MemReadFunc		_RamRead;
	 	MemWriteFunc	_RamWrite;
		IoInpFunc		cpu_in;
	 	IoOutFunc		cpu_out;

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
	    #define INSTANTIABLE_CPU_Z80RUN_SINGLE_CYCLE 1
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
	    #undef INSTANTIABLE_CPU_Z80RUN_SINGLE_CYCLE
	    #undef INSTANTIABLE_CPU_IN_OUT_DEFINED
	    #undef SET_HIGH_REGISTER
	    #undef SET_LOW_REGISTER
	    #if defined(_MSC_VER)
	    #pragma warning(pop)
	    #endif
	};

	Cpu::Cpu()
		: cpuImpl { new CpuImpl() }
	{
		cpuImpl->_RamRead = [&](uint16_t a) -> uint8_t { return memRead(a); };
		cpuImpl->_RamWrite = [&](uint16_t a, uint8_t d) { memWrite(a, d); };
		cpuImpl->cpu_in = [&](uint16_t a) -> uint8_t { return ioInp(a); };
		cpuImpl->cpu_out = [&](uint16_t a, uint8_t d) { ioOut(a, d); };
		reset();
	}

	Cpu::~Cpu() {
		delete cpuImpl;
	}

	void Cpu::reset() {
		cpuImpl->Z80reset();
	}

	bool Cpu::interrupt(bool nmi, uint8_t interruptVector) {
		{
			const uint8_t IFF1 = (cpuImpl->IFF & 1);
			const uint8_t EI_Delay = (cpuImpl->EI_Delay);
			if(nmi) {
				// - The state of IFF1 is used to inhibit interrupts while
				//   IFF2 is used as a temporary storage location for IFF1.
				cpuImpl->IFF = (IFF1 << 1);
			} else {
				// - When the IFF is reset, an interrupt cannot be accepted by the CPU.
				if(IFF1 == 0) {
					return false;
				}
				if(EI_Delay != 0) {
					return false;
				}
				// - When the CPU accepts a maskable interrupt, both IFF1 and IFF2 are automatically reset,
				//   inhibiting further interrupts until the programmer issues a new EI instruction.
				cpuImpl->IFF = 0;
//				cpuImpl->IFF &= ~1;
			}
		}

		uint16_t PC = cpuImpl->PC;
		if(cpuImpl->HALT != 0) {
			PC += 1;
			cpuImpl->HALT = 0;
		}

		// push PC
		cpuImpl->_RamWrite(--cpuImpl->SP, static_cast<uint8_t>((PC >> 8) & 0xff));
		cpuImpl->_RamWrite(--cpuImpl->SP, static_cast<uint8_t>((PC     ) & 0xff));

		const uint8_t IM   = cpuImpl->IM;
		const uint8_t I    = static_cast<uint8_t>(cpuImpl->IR >> 8);
		uint16_t jmpTo = 0;
		if(nmi) {
			jmpTo = 0x66;
		} else if(IM == 1) {
			jmpTo = 0x38;
		} else if(IM == 2) {
			const uint16_t ea = (I << 8) | interruptVector;
			jmpTo = memRead(ea) | (static_cast<uint16_t>(memRead(ea+1)) << 8);
		} else if(IM == 0 && (interruptVector & 0b11'000'111) == 0b11'000'111) {	// RST xx
			jmpTo = (interruptVector & 0b00'111'000);
		}

		// jp jmpTo
		cpuImpl->PC = jmpTo;
		cpuImpl->PCX = jmpTo;
		return true;
	}

	CpuStatus Cpu::getCpuStatus() const {
		CpuStatus cs;
		cs.PC	= static_cast<uint16_t>(cpuImpl->PCX);
//		cs.PC	= static_cast<uint16_t>(cpuImpl->PC);
		cs.AF	= static_cast<uint16_t>(cpuImpl->AF);
		cs.BC	= static_cast<uint16_t>(cpuImpl->BC);
		cs.DE	= static_cast<uint16_t>(cpuImpl->DE);
		cs.HL	= static_cast<uint16_t>(cpuImpl->HL);
		cs.IX	= static_cast<uint16_t>(cpuImpl->IX);
		cs.IY	= static_cast<uint16_t>(cpuImpl->IY);
		cs.SP	= static_cast<uint16_t>(cpuImpl->SP);
		cs.AF_	= static_cast<uint16_t>(cpuImpl->AF1);
		cs.BC_	= static_cast<uint16_t>(cpuImpl->BC1);
		cs.DE_	= static_cast<uint16_t>(cpuImpl->DE1);
		cs.HL_	= static_cast<uint16_t>(cpuImpl->HL1);
		cs.IR	= static_cast<uint16_t>(cpuImpl->IR);
		cs.IFFS	= 0;
		cs.setIFF1((cpuImpl->IFF >> 0) & 1);
		cs.setIFF2((cpuImpl->IFF >> 1) & 1);
		cs.setHALT(cpuImpl->HALT);
		cs.setIM  (cpuImpl->IM);
		cs.setEI  (cpuImpl->EI_Delay);
		return cs;
	}

	void Cpu::setCpuStatus(const CpuStatus& cpuStatus) {
		cpuImpl->PCX	= cpuStatus.PC;
		cpuImpl->PC		= cpuStatus.PC;
		cpuImpl->AF		= cpuStatus.AF;
		cpuImpl->BC		= cpuStatus.BC;
		cpuImpl->DE		= cpuStatus.DE;
		cpuImpl->HL		= cpuStatus.HL;
		cpuImpl->IX		= cpuStatus.IX;
		cpuImpl->IY		= cpuStatus.IY;
		cpuImpl->SP		= cpuStatus.SP;
		cpuImpl->AF1	= cpuStatus.AF_;
		cpuImpl->BC1	= cpuStatus.BC_;
		cpuImpl->DE1	= cpuStatus.DE_;
		cpuImpl->HL1	= cpuStatus.HL_;
		cpuImpl->IR		= cpuStatus.IR;		// Interrupt (upper) / Refresh (lower) register
		cpuImpl->IFF	= (cpuStatus.getIFF1() << 0) | (cpuStatus.getIFF2() << 1);
		cpuImpl->IM		= cpuStatus.getIM();
		cpuImpl->HALT	= cpuStatus.getHALT();
		cpuImpl->EI_Delay	= cpuStatus.getEI();
	}

	void Cpu::runSingleCycle() {
		cpuImpl->Z80run();
	}
}
