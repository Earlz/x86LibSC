/**
Copyright (c) 2007 - 2010 Jordan "Earlz/hckr83" Earls  <http://www.Earlz.biz.tm>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.
   
THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This file is part of the x86Lib project.
**/

#include <stdint.h>

#ifndef X86LIB_INTERNAL_H
#define X86LIB_INTERNAL_H


namespace x86Lib{
	//32 bit register macros
static const int EAX=0;
static const int ECX=1;
static const int EDX=2;
static const int EBX=3;
static const int ESP=4;
static const int EBP=5;
static const int ESI=6;
static const int EDI=7;


//16 bit register macros
static const int AX=0;
static const int CX=1;
static const int DX=2;
static const int BX=3;
static const int SP=4;
static const int BP=5;
static const int SI=6;
static const int DI=7;

//8 bit register macros
static const int AL=0;
static const int CL=1;
static const int DL=2;
static const int BL=3;
static const int AH=4;
static const int CH=5;
static const int DH=6;
static const int BH=7;


//segment registers constants(the defaults)
static const int cES=0;
static const int cCS=1;
static const int cSS=2;
static const int cDS=3;
static const int cFS=4;
static const int cGS=5;
static const int cIS=6; //this is an imaginary segment only used for direct segment overrides
//for instance it would be used in mov [1000:bx],ax



typedef struct{
    unsigned char cf:1;
    unsigned char r0:1;
    unsigned char pf:1;
    unsigned char r1:1;
    unsigned char af:1;
    unsigned char r2:1;
    unsigned char zf:1;
    unsigned char sf:1;
    unsigned char tf:1;
    unsigned char _if:1;
    unsigned char df:1;
    unsigned char of:1;
    unsigned char iopl:2; //not yet used
    unsigned char nt:1;
    unsigned char r3:1;
}__attribute__((packed))FLAGS; //this is a better representation of flags(much easier to use)





typedef void (x86Lib::x86CPU::*opcode)(); /**You have no idea how hard it was to figure out how to do that!**/


static const uint32_t OPCODE_REAL_16=1;
static const uint32_t OPCODE_HOSTED_32=2;


//CPU Exceptions(interrupt handled)
static const uint32_t DIV0_IEXCP=0xF000; //Divide by zero exception
static const uint32_t DEBUG_IEXCP=0xF001; //Debug exception
static const uint32_t NMI_IEXCP=0xF002; //NMI
static const uint32_t BREAK_IEXCP=0xF003; //Breakpoint/int 3
static const uint32_t OVERFLOW_IEXCP=0xF004; //Overflow/into
static const uint32_t BOUNDS_IEXCP=0xF005; //Bounds Check
static const uint32_t UNK_IEXCP=0xF006; //unknown opcode
static const uint32_t UNDEV_IEXCP=0xF007; //Unknown device
static const uint32_t DOUBLE_FAULT_IEXCP=0xF008;
static const uint32_t SEG_OVERRUN_IEXCP=0xF009; //Co-processor segment overrun..(not used after i486
static const uint32_t ITSS_IEXCP=0xF00A; //Invalid TSS
static const uint32_t ISEG_IEXCP=0xF00B; //Invalid/non-existent segment
static const uint32_t STACK_IEXCP=0xF00C; //Stack Exception
static const uint32_t GPF_IEXCP=0xF00D; //GPF
static const uint32_t PAGE_FAULT_IEXCP=0xF00E;
static const uint32_t RESERVED_IEXCP=0xF00F; //Reserved by intel, so internal use?
static const uint32_t FLOAT_ERROR_IEXCP=0xF010; //Floating Point Error..
static const uint32_t ALIGN_IEXCP=0xF011; //Alignment Check...

static const uint32_t UNSUPPORTED_EXCP = 0xF0FF; //indicates something unsupported by the emulator

class CpuInt_excp{ //Used internally for handling interrupt exceptions...
	public:
	CpuInt_excp(uint32_t code_){
		code=code_;
	}
	uint32_t code;
};
















/**Random support functions that are static inline'd**/

static inline uint16_t SignExtend8(uint8_t val){ //sign extend a byte to a word
	if((val&0x80)!=0){
		return 0xFF00|val;
	}else{
		return val;
	}
}

//convert signed integer into unsigned, and store top bit in store
static inline uint64_t Unsign64(uint64_t val,bool &store){

    if(val>=0x8000000000000000){
        store=1;
        return (~(val))+1;
    }else{
        store=0;
        return val;
    }

}
static inline uint32_t Unsign32(uint32_t val,bool &store){

	if(val>=0x80000000){
		store=1;
		return (~(val))+1;
	}else{
		store=0;
		return val;
	}

}

static inline uint16_t Unsign16(uint16_t val,bool &store){
	if(val>=0x8000){
		store=1;
		return (~(val))+1;
	}else{
		store=0;
		return val;
	}

}

static inline uint8_t Unsign8(uint8_t val,bool &store){
	if(val>=0x80){
		store=1;
		return (~(val))+1;
	}else{
		store=0;
		return val;
	}

}

/**Resign an unsigned integer using the store as the sign bit.
--Note, in order to combine two sign bits, just bitwise XOR(^) them!*/
static inline uint64_t Resign64(uint64_t val,bool store1){
    if((store1)==1){
        return (~(val))+1;
    }else{
        return val;
    }
}
static inline uint32_t Resign32(uint32_t val,bool store1){
	if((store1)==1){
		return (~(val))+1;
	}else{
		return val;
	}
}

static inline uint16_t Resign16(uint16_t val,bool store1){
	if((store1)==1){
		return (~(val))+1;
	}else{
		return val;
	}
}

static inline uint8_t Resign8(uint8_t val,bool store1){
	if((store1)==1){
		return (~(val))+1;
	}else{
		return val;
	}
}


};


#endif


#ifdef X86_POST_CPU

#ifndef X86LIB_INTERNAL_H_POST_CPU
#define X86LIB_INTERNAL_H_POST_CPU
//namespace x86Lib{

	



	
inline uint16_t ModRM::GetRegD(){ //This returns the register displacement value
	switch(modrm.rm){
		case 0:
		return *this_cpu->regs16[BX]+*this_cpu->regs16[SI];
		break;
		case 1:
		return *this_cpu->regs16[BX]+*this_cpu->regs16[DI];
		break;
		case 2:
		use_ss=1;
		return *this_cpu->regs16[BP]+*this_cpu->regs16[SI];
		break;
		case 3:
		use_ss=1;
		return *this_cpu->regs16[BP]+*this_cpu->regs16[DI];
		break;
		case 4:
		return *this_cpu->regs16[SI];
		break;
		case 5:
		return *this_cpu->regs16[DI];
		case 6: //immediate Displacement only, so no register displace..
		return 0;
		break;
		case 7:
		return *this_cpu->regs16[BX];
		break;
	}
	return 0;
}

inline uint32_t ModRM::GetRegD32(){ //This returns the register displacement value
	switch(modrm.rm){
		case 0:
		return this_cpu->regs32[EAX];
		case 1:
		return this_cpu->regs32[ECX];
		case 2:
		return this_cpu->regs32[EDX];
		case 3:
		return this_cpu->regs32[EBX];
		break;
		case 4:
		//TODO SIB
		return 0;
		break;
		case 5: //immediate Displacement only, so no register displace..
		return 0;
		case 6:
		return this_cpu->regs32[ESI];
		case 7:
		return this_cpu->regs32[EDI];
	}
	return 0;
}

inline uint16_t ModRM::GetDisp(){
	uint16_t reg;
	reg=GetRegD();
	if(modrm.rm==6){ //Don't worry, it's safe...
		use_ss=1;
		reg=*this_cpu->regs16[BP];
	}
	switch(modrm.mod){
		case 0: //no displacement

		if(modrm.rm==6){ //if only word displacement...
			use_ss=0;
			//eip++;
			//eip++;
			return *(uint16_t*)&this_cpu->op_cache[1];
		}else{
			return reg;
		}
		break;
		case 1: //byte displacement(signed)
		//eip++;
		return (signed)reg+(signed)this_cpu->op_cache[1];
		break;
		case 2: //word displacement(signed)
		return (signed)reg+(signed)(*(uint16_t*)&this_cpu->op_cache[1]);
		break;
		case 3: //opcode specific...
		op_specific=1;
		return 0;
		break;
	}
	return 0;
}

inline uint32_t ModRM::GetDisp32(){
	uint32_t reg;
	reg=GetRegD32();
	switch(modrm.mod){
		case 0: //no displacement
		if(modrm.rm==5){ //only dword displacement...
			return this_cpu->ReadDword(this_cpu->CS, this_cpu->eip+1);
		}else if(modrm.rm == 4){ //if SIB
            return GetSIBDisp();
		}
        return reg;
		break;
		case 1: //byte displacement(signed)
		if(modrm.rm == 4){
            return GetSIBDisp() + (int32_t)this_cpu->op_cache[2];
		}else{
            return (int32_t)reg+(int32_t)this_cpu->op_cache[1];
		}
		break;
		case 2: //dword displacement(signed)
		if(modrm.rm == 4){
		    //make sure to use eip+2 here to account for SIB
            return (int32_t)GetSIBDisp() + (int32_t)this_cpu->ReadDword(this_cpu->CS, this_cpu->eip + 2);
		}else{
            return (int32_t)reg + (int32_t)this_cpu->ReadDword(this_cpu->CS, this_cpu->eip + 1);
		}
		break;
		case 3: //opcode specific...
		op_specific=1;
		return 0;
		break;
	}
	return 0;
}

inline uint32_t ModRM::GetSIBDisp(){
    int32_t regindex=0;
    int32_t regbase=0;
    if(sib.index == 4){
        regindex=0;
    }
    int32_t mul=1;
    switch(sib.ss){
        case 1: //skip 0
        mul = 2;
        break;
        case 2:
        mul = 4;
        break;
        case 3:
        mul = 8;
        break;
    }
    switch(sib.index){
        case 0:
            regindex = this_cpu->regs32[EAX];
            break;
        case 1:
            regindex = this_cpu->regs32[ECX];
            break;
        case 2:
            regindex = this_cpu->regs32[EDX];
            break;
        case 3:
            regindex = this_cpu->regs32[EBX];
            break;
        case 4:
            regindex = 0;
            break;
        case 5:
            regindex = this_cpu->regs32[EBP];
            break;
        case 6:
            regindex = this_cpu->regs32[ESI];
            break;
        case 7:
            regindex = this_cpu->regs32[EDI];
            break;
    }

    switch(sib.base){
        case 0:
            regbase = this_cpu->regs32[EAX];
            break;
        case 1:
            regbase = this_cpu->regs32[ECX];
            break;
        case 2:
            regbase = this_cpu->regs32[EDX];
            break;
        case 3:
            regbase = this_cpu->regs32[EBX];
            break;
        case 4:
            regbase = this_cpu->regs32[ESP];
            break;
        case 5:
            if(modrm.mod == 0){
                regbase = 0;
            }else{
                regbase = this_cpu->regs32[EBP];
            }
            break;
        case 6:
            regbase = this_cpu->regs32[ESI];
            break;
        case 7:
            regbase = this_cpu->regs32[EDI];
            break;
    }
    return regindex * mul + regbase;
}


inline ModRM::ModRM(x86CPU *this_cpu_){
	use_ss=0;
	op_specific=0;
	this_cpu=this_cpu_;
	*(uint32_t*)&this_cpu->op_cache=this_cpu->ReadDword(cCS,this_cpu->eip);
	*(uint8_t*)&modrm=this_cpu->op_cache[0];
	*(uint8_t*)&sib=this_cpu->op_cache[1];
}

inline ModRM::~ModRM(){
	this_cpu->eip+=GetLength()-1;
}

//The r suffix means /r, which means for op_specific=1, use general registers
inline uint8_t ModRM::ReadByter(){
    if(this_cpu->Use32BitAddress()){
        return ReadByter32();
    }
	use_ss=0;
	op_specific=0;
	uint16_t disp=GetDisp();
	if(op_specific==1){
		return *this_cpu->regs8[modrm.rm];
	}else{
		if(use_ss==1){
			return this_cpu->ReadByte(this_cpu->SS,disp);
		}else{
			return this_cpu->ReadByte(this_cpu->DS,disp);
		}
	}
}

inline uint16_t ModRM::ReadWordr(){
    if(this_cpu->Use32BitAddress()){
        return ReadWordr32();
    }
	use_ss=0;
	op_specific=0;
	uint16_t disp=GetDisp();
	if(op_specific==1){
		//cout << "h" << endl;
		return *this_cpu->regs16[modrm.rm];
	}else{

		if(use_ss==1){
			return this_cpu->ReadWord(this_cpu->SS,disp);
		}else{
			return this_cpu->ReadWord(this_cpu->DS,disp);
		}
	}
}
inline uint32_t ModRM::ReadDwordr(){ //make this for consistency. Need to refactor this whole class later..
    return ReadDword();
}
inline uint32_t ModRM::ReadDword(){
    if(this_cpu->Use32BitAddress()){
        return ReadDwordr32();
    }
	use_ss=0;
	op_specific=0;
	uint16_t disp=GetDisp();
	if(op_specific==1){
		return this_cpu->regs32[modrm.rm];
	}else{

		if(use_ss==1){
			return this_cpu->ReadDword(this_cpu->SS,disp);
		}else{
			return this_cpu->ReadDword(this_cpu->DS,disp);
		}
	}
}

inline void ModRM::WriteByter(uint8_t byte){
    if(this_cpu->Use32BitAddress()){
        return WriteByter32(byte);
    }
	use_ss=0;
	op_specific=0;
	uint16_t disp=GetDisp();
	if(op_specific==1){
		*this_cpu->regs8[modrm.rm]=byte;
	}else{

		if(use_ss==1){
			this_cpu->WriteByte(this_cpu->SS,disp,byte);
		}else{
			this_cpu->WriteByte(this_cpu->DS,disp,byte);
		}
	}
}
inline void ModRM::WriteWordr(uint16_t word){
    if(this_cpu->Use32BitAddress()){
        return WriteWordr32(word);
    }
	use_ss=0;
	op_specific=0;
	uint16_t disp=GetDisp();
	if(op_specific==1){
		*this_cpu->regs16[modrm.rm]=word;
	}else{

		if(use_ss==1){
			this_cpu->WriteWord(this_cpu->SS,disp,word);
		}else{
			this_cpu->WriteWord(this_cpu->DS,disp,word);
		}
	}
}
inline void ModRM::WriteDwordr(uint32_t dword){
    WriteDword(dword);
}
inline void ModRM::WriteDword(uint32_t dword){
    if(this_cpu->Use32BitAddress()){
        return WriteDword(dword);
    }
	use_ss=0;
	op_specific=0;
	uint16_t disp=GetDisp();
	if(op_specific==1){
		this_cpu->regs32[modrm.rm]=dword;
	}else{

		if(use_ss==1){
			this_cpu->WriteDword(this_cpu->SS,disp,dword);
		}else{
			this_cpu->WriteDword(this_cpu->DS,disp,dword);
		}
	}
}



//The r suffix means /r, which means for op_specific=1, use general registers
inline uint8_t ModRM::ReadByter32(){
	use_ss=0;
	op_specific=0;
	uint32_t disp=GetDisp32();
	if(op_specific==1){
		return *this_cpu->regs8[modrm.rm];
	}else{
        return this_cpu->ReadByte(this_cpu->DS,disp);
	}
}

inline uint16_t ModRM::ReadWordr32(){
	use_ss=0;
	op_specific=0;
	uint32_t disp=GetDisp32();
	if(op_specific==1){
	    //don't think this is actually possible in 32bit mode, but ok
		return *this_cpu->regs16[modrm.rm];
	}else{
        return this_cpu->ReadWord(this_cpu->DS,disp);
	}
}
inline uint32_t ModRM::ReadDwordr32(){
	use_ss=0;
	op_specific=0;
	uint32_t disp=GetDisp32();
	if(op_specific==1){
		return this_cpu->regs32[modrm.rm];
	}else{
        return this_cpu->ReadDword(this_cpu->DS,disp);
	}
}

inline void ModRM::WriteByter32(uint8_t byte){
	use_ss=0;
	op_specific=0;
	uint32_t disp=GetDisp32();
	if(op_specific==1){
		*this_cpu->regs8[modrm.rm]=byte;
	}else{
        this_cpu->WriteByte(this_cpu->DS,disp,byte);
	}
}
inline void ModRM::WriteWordr32(uint16_t word){
	use_ss=0;
	op_specific=0;
	uint32_t disp=GetDisp32();
	if(op_specific==1){
		*this_cpu->regs16[modrm.rm]=word;
	}else{
        this_cpu->WriteWord(this_cpu->DS,disp,word);
	}
}
inline void ModRM::WriteDwordr32(uint32_t dword){
	use_ss=0;
	op_specific=0;
	uint32_t disp=GetDisp32();
	if(op_specific==1){
		this_cpu->regs32[modrm.rm]=dword;
	}else{
        this_cpu->WriteDword(this_cpu->DS,disp,dword);
	}
}

inline uint8_t ModRM::GetLength(){ //This returns how many total bytes the modrm block consumes
    if(this_cpu->Use32BitAddress()){
        if((modrm.mod==0) && (modrm.rm==5)){
            return 5;
        }
        if(modrm.mod == 3){
            return 1;
        }
        int count=1; //1 for modrm byte
        if(modrm.rm == 4){
            count++; //SIB byte
        }
        switch(modrm.mod){
            case 0:
            count += 0;
            break;
            case 1:
            count += 1;
            break;
            case 2:
            count += 4;
            break;
        }
        return count;
    }else{
        if((modrm.mod==0) && (modrm.rm==6)){
            return 3;
        }
        switch(modrm.mod){
            case 0:
            return 1;
            break;
            case 1:
            return 2;
            break;
            case 2:
            return 3;
            break;
            case 3:
            return 1;
            break;
        }
	}
	return 1; //should never reach here, but to avoid warnings...

} //that was easier than I first thought it would be...
inline uint8_t ModRM::GetExtra(){ //Get the extra fied from mod_rm
	return modrm.extra;
}

inline uint16_t ModRM::ReadOffset(){ //This is only used by LEA. It will obtain the offset and not dereference it...
	use_ss=0;
	op_specific=0;
	uint16_t disp=GetDisp();
	if(op_specific==1){
		throw CpuInt_excp(UNK_IEXCP);
		//We can't return regs16 because it can't get address of a register!
		//return *regs16[modrm.rm];
	}else{
		return disp;
	}
}

inline uint32_t ModRM::ReadOffset32(){ //This is only used by LEA. It will obtain the offset and not dereference it...
	use_ss=0;
	op_specific=0;
	uint32_t disp=GetDisp32();
	if(op_specific==1){
		throw CpuInt_excp(UNK_IEXCP);
		//We can't return regs16 because it can't get address of a register!
		//return *regs16[modrm.rm];
	}else{
		return disp;
	}
}


#endif
#endif
