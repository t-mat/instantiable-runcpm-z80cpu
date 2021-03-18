// Instantiable RunCPM Z80 CPU
// Copyright (C) 2021, Takayuki Matsuoka.
// SPDX-License-Identifier: MIT
// ----
#pragma once
#include <memory>
#include <functional>

namespace RunCpmZ80Cpu {
	using MemReadFunc 	= std::function<uint8_t(uint16_t)>;
	using MemWriteFunc	= std::function<void(uint16_t, uint8_t)>;
	using IoInpFunc 	= std::function<uint8_t(uint16_t)>;
	using IoOutFunc 	= std::function<void(uint16_t, uint8_t)>;

	struct CpuStatus {
		uint16_t	PC, SP, IX, IY;
		uint16_t	AF,  BC,  DE,  HL;
		uint16_t	AF_, BC_, DE_, HL_;
		uint16_t	IR;		// Interrupt (upper) / Refresh (lower) register
		uint16_t	IFFS;	// &0x01=IFF1, &0x02=IFF2, &0x04=EI, &0x08=HALT, &0x30=IM

		uint8_t getIFF1() const { return (IFFS >> 0) & 1; }
		uint8_t getIFF2() const { return (IFFS >> 1) & 1; }
		uint8_t getEI()   const { return (IFFS >> 2) & 1; }
		uint8_t getHALT() const { return (IFFS >> 3) & 1; }
		uint8_t getIM()   const { return (IFFS >> 4) & 3; }

		void setIFF1(uint8_t v) { IFFS &= ~(1 << 0); IFFS |= (v & 1) << 0; }
		void setIFF2(uint8_t v) { IFFS &= ~(1 << 1); IFFS |= (v & 1) << 1; }
		void setEI(uint8_t v)   { IFFS &= ~(1 << 2); IFFS |= (v & 1) << 2; }
		void setHALT(uint8_t v) { IFFS &= ~(1 << 3); IFFS |= (v & 1) << 3; }
		void setIM(uint8_t v)   { IFFS &= ~(3 << 4); IFFS |= (v & 3) << 4; }
	};

	struct CpuImpl;

	struct Cpu {
		Cpu();
		~Cpu();

		void reset();
		CpuStatus getCpuStatus() const;
		void setCpuStatus(const CpuStatus& cpuStatus);
		void runSingleCycle();

		bool nonMaskableInterrupt() { return interrupt(true, 0); }
		bool maskableInterrupt(uint8_t iv) { return interrupt(false, iv); }
		bool interrupt(bool nmi, uint8_t interruptVector);

		MemReadFunc		memRead;
	 	MemWriteFunc	memWrite;
		IoInpFunc		ioInp;
	 	IoOutFunc		ioOut;
		CpuImpl*		cpuImpl = nullptr;
	};
}
