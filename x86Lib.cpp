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
#include <iostream>
#include <x86Lib.h>
#include <sstream>


namespace x86Lib{
//The lack of indentation for namespaces is intentional...
using namespace std;


x86CPU::x86CPU(uint32_t cpu_level_,uint32_t flags){
	if(cpu_level_==0){
		cpu_level=CPU286_REAL; //The default CPU right now..
	}else{
		cpu_level=cpu_level_;
	}
	Init();
}
x86CPU::x86CPU(x86SaveData &load_data,uint32_t flags){
	Init();
	LoadState(load_data);
}

void x86CPU::Init(){
	Reset();
}


void x86CPU::Reset(){
	/**Initialize register pointers**/
	regs16[0]=&((volatile uint16_t*)regs32)[0];
	regs16[1]=&((volatile uint16_t*)regs32)[2];
	regs16[2]=&((volatile uint16_t*)regs32)[4];
	regs16[3]=&((volatile uint16_t*)regs32)[6];
	regs16[4]=&((volatile uint16_t*)regs32)[8];
	regs16[5]=&((volatile uint16_t*)regs32)[10];
	regs16[6]=&((volatile uint16_t*)regs32)[12];
	regs16[7]=&((volatile uint16_t*)regs32)[14];
	regs8[0]=&((volatile uint8_t*)regs32)[0];
	regs8[1]=&((volatile uint8_t*)regs32)[4];
	regs8[2]=&((volatile uint8_t*)regs32)[8];
	regs8[3]=&((volatile uint8_t*)regs32)[12], //now do all the highs
	regs8[4]=&((volatile uint8_t*)regs32)[1];
	regs8[5]=&((volatile uint8_t*)regs32)[5];
	regs8[6]=&((volatile uint8_t*)regs32)[9];
	regs8[7]=&((volatile uint8_t*)regs32)[13];
	busmaster=0;
	//assumes pmem and ports are still the same...
	InitOpcodes();
    Opcodes = opcodes_hosted; //for smart contracts, don't use this as a full VM, just straight to 32bit mode
    Opcodes_ext = opcodes_hosted_ext;
	uint32_t i;
	for(i=0;i<8;i++){
		regs32[i]=0;
	}
	for(i=0;i<7;i++){
		seg[i]=0;
	}
	ResetSegments();
	eip=0x1000;
	seg[cCS]=0;
	*(uint16_t*)&freg=0;
    regs32[ESP] = 0x1FF000; //set stack to reasonable address for Qtum
	string_compares=0;
	int_pending=0;
	cli_count=0;
    OperandSize16=false;
    AddressSize16=false;
    DoStop=false;
    PrefixCount = 0;
}


void x86CPU::SaveState(x86SaveData *save){
	uint32_t i;
	for(i=0;i<8;i++){
		save->reg32[i]=regs32[i];
	}
	for(i=0;i<7;i++){
		save->seg[i]=seg[i];
	}
	save->freg=*(uint16_t*)&freg;
	save->eip=eip;
	save->seg_route[cES]=ES;
	save->seg_route[cCS]=CS;
	save->seg_route[cDS]=DS;
	save->seg_route[cSS]=SS;
	save->seg_route[cFS]=FS;
	save->seg_route[cGS]=GS;
	save->cpu_level=cpu_level;
    save->opcode_mode = OPCODE_HOSTED_32;
}

void x86CPU::LoadState(x86SaveData &load){
	uint32_t i;
	cpu_level=load.cpu_level;
	Reset();
	for(i=0;i<8;i++){
		regs32[i]=load.reg32[i];
	}
	for(i=0;i<7;i++){
		seg[i]=load.seg[i];
	}
	*(uint16_t*)&freg=load.freg;
	eip=load.eip;
	ES=load.seg_route[cES];
	CS=load.seg_route[cCS];
	DS=load.seg_route[cDS];
	SS=load.seg_route[cSS];
	GS=load.seg_route[cGS];
	FS=load.seg_route[cFS];
    Opcodes = opcodes_hosted;
}




void x86CPU::DumpState(ostream &output){
	output << "EAX: "<< hex << regs32[EAX] <<endl;
	output << "ECX: "<< hex << regs32[ECX] <<endl;
	output << "EDX: "<< hex << regs32[EDX] <<endl;
	output << "EBX: "<< hex << regs32[EBX] <<endl;
	output << "ESP: "<< hex << regs32[ESP] <<endl;
	output << "EBP: "<< hex << regs32[EBP] <<endl;
	output << "ESI: "<< hex << regs32[ESI] <<endl;
	output << "EDI: "<< hex << regs32[EDI] <<endl;

	output << "CS: " << hex << seg[cCS] << endl;
	output << "SS: " << hex << seg[cSS] << endl;
	output << "DS: " << hex << seg[cDS] << endl;
	output << "ES: " << hex << seg[cES] << endl;
	output << "FS: " << hex << seg[cFS] << endl;
	output << "GS: " << hex << seg[cGS] << endl;
	output << "EIP: " << hex << eip << endl;

	output << "--Flags:" <<hex << *(uint16_t*)&freg<< endl;
	output << "CF: " << (int)freg.cf << endl;
	output << "PF: " << (int)freg.pf << endl;
	output << "AF: " << (int)freg.af << endl;
	output << "ZF: " << (int)freg.zf << endl;
	output << "SF: " << (int)freg.sf << endl;
	output << "TF: " << (int)freg.tf << endl;
	output << "IF: " << (int)freg._if << endl;
	output << "DF: " << (int)freg.df << endl;
	output << "OF: " << (int)freg.of << endl;
}

void x86CPU::Int(uint8_t num){ //external interrupt
	int_pending=1;
	int_number=num;
}

bool x86CPU::IntPending(){
	return int_pending;
}



int x86CPU::CheckInterrupts(){
	//possible bug here. What if we have a pop SS without an interrupt waiting? Won't interrupts be forever ignored?
	if(int_pending==0){return 0;} //quickly get out of this, this is in the main Cycle loop, so needs to be very speedy...
	if((int_pending==1) && (int_number==2)){ //NMI
		eip--;
		Int16(2);
		eip++;
		int_pending=0;
		return 1;
	}
	if(freg._if==0){
		if(cli_count>1){
			cli_count--;
		}else{
			if(cli_count!=0){
				freg._if=1;
			}
		}
	}else{
		if(int_pending==1){
			eip--;
			Int16(int_number);
			eip++;
			int_pending=0;
			return 1;
		}
	}
	return 0;
}

void x86CPU::Exec(int cyclecount){
	int i=0;
	bool done=false;
	while(!done){
		try{
			for(;i<cyclecount;i++){
				Cycle();
                if(DoStop){
                    DoStop=false;
                    return;
                }
			}
		}
		catch(CpuInt_excp err){
			err.code&=0x00FF;
			switch(err.code){
				case 0: //division by zero
				case 1: //debug exception
				case 2: //NMI
				case 3: //breakpoint
				case 4: //overflow
				
				case 7: //device unavailable
				Int16(err.code);
				eip++; //undo the decrement by Int
				break;
                case 6: //unknown opcode
                    throw CpuPanic_excp("Unknown opcode",(err.code|0xF000)|TRIPLE_FAULT_EXCP);
				case 5: //(186+ bounds check)
				if(cpu_level >= CPU186){
					Int16(err.code);
					eip++;
					break;
				}
				default:
				throw CpuPanic_excp("16bit Faults",(err.code|0xF000)|TRIPLE_FAULT_EXCP);
			}
		}
		catch(Mem_excp err){
            std::ostringstream oss;
            oss << "Memory error at 0x" << std::hex << err.address;
			throw CpuPanic_excp(oss.str(),TRIPLE_FAULT_EXCP);
		}
		if(i>=cyclecount){
			done=true;
		}
	}
}


void x86CPU::Cycle(){
#ifdef ENABLE_OPCODE_CALLBACK
	if(EachOpcodeCallback!=NULL){
		(*EachOpcodeCallback)(this);
	}
#endif
	CheckInterrupts();
	*(uint64_t*)&op_cache=ReadQword(cCS,eip);
    wherebeen.push_back((uint32_t)eip);
    //note this bit for 0x0F checking could probably be better in this very critical loop
    if(op_cache[0] == 0x0F){
        //two byte opcode
        eip++;
        *(uint64_t*)&op_cache=ReadQword(cCS,eip);
        (this->*Opcodes_ext[op_cache[0]])(); //if in 32-bit mode, then go to 16-bit opcode
    }else {
        //operate on the this class with the opcode functions in this class
        (this->*Opcodes[op_cache[0]])();
    }
	eip=eip+1;
}







void x86CPU::InstallOp(uint8_t num,opcode func, opcode *opcode_table){
    if(opcode_table){
        opcode_table[num]=func;
    }else {
        Opcodes[num] = func;
    }
}

void x86CPU::InitOpcodes(){
	Opcodes=opcodes_hosted;

    //init all to unknown
    for(int i=0;i<256;i++){
        InstallOp(i, &x86CPU::op_unknown, opcodes_hosted);
    }

	for(int i=0;i<=7;i++){
		InstallOp(0xB0+i,&x86CPU::op_mov_r8_imm8);
		InstallOp(0x58+i,&x86CPU::op_pop_rW);
		InstallOp(0x50+i,&x86CPU::op_push_rW);
		InstallOp(0x40+i,&x86CPU::op_inc_rW);
		InstallOp(0x48+i,&x86CPU::op_dec_rW);
		InstallOp(0xD8+i,&x86CPU::op_escape);
		InstallOp(0x90+i,&x86CPU::op_xchg_axW_rW);
		InstallOp(0xB8+i,&x86CPU::op_mov_rW_immW);
	}
	InstallOp(0xF4,&x86CPU::op_hlt);
	InstallOp(0x90,&x86CPU::op_nop);
	InstallOp(0xEB,&x86CPU::op_jmp_rel8);
	InstallOp(0x2C,&x86CPU::op_sub_al_imm8);
	InstallOp(0x2D,&x86CPU::op_sub_axW_immW);
	InstallOp(0x8E,&x86CPU::op_mov_sr_rm16);
	InstallOp(0x8C,&x86CPU::op_mov_rm16_sr);
	InstallOp(0x68,&x86CPU::op_push_immW);
	InstallOp(0x07,&x86CPU::op_pop_es);
	InstallOp(0x17,&x86CPU::op_pop_ss);
	InstallOp(0x1F,&x86CPU::op_pop_ds);
	InstallOp(0x06,&x86CPU::op_push_es);
	InstallOp(0x0E,&x86CPU::op_push_cs);
	InstallOp(0x16,&x86CPU::op_push_ss);
	InstallOp(0x1E,&x86CPU::op_push_ds);
	InstallOp(0x89,&x86CPU::op_mov_rmW_rW);
	InstallOp(0x8B,&x86CPU::op_mov_rW_rmW);
	InstallOp(0xE8,&x86CPU::op_call_relW);
	InstallOp(0xC3,&x86CPU::op_retn);
	InstallOp(0xE2,&x86CPU::op_loopcc_rel8);
	InstallOp(0x26,&x86CPU::op_pre_es_override);
	InstallOp(0x3E,&x86CPU::op_pre_ds_override);
	InstallOp(0x36,&x86CPU::op_pre_ss_override);
	InstallOp(0x2E,&x86CPU::op_pre_cs_override);
	InstallOp(0xA5,&x86CPU::op_movsW);
	InstallOp(0xA4,&x86CPU::op_movsb);
	InstallOp(0xF8,&x86CPU::op_clc);
	InstallOp(0xFC,&x86CPU::op_cld);
	InstallOp(0xFA,&x86CPU::op_cli);
	InstallOp(0xF9,&x86CPU::op_stc);
	InstallOp(0xFD,&x86CPU::op_std);
	InstallOp(0xFB,&x86CPU::op_sti);
	InstallOp(0xF2,&x86CPU::op_rep);
	InstallOp(0xF3,&x86CPU::op_rep); //different, but handled by the same function...
	InstallOp(0xE6,&x86CPU::op_out_imm8_al);
	InstallOp(0xE7,&x86CPU::op_out_imm8_axW);
	InstallOp(0x9A,&x86CPU::op_call_immF);
	InstallOp(0xCB,&x86CPU::op_retf); 
	//can't override these opcodes
	//InstallOp(0xCD,&x86CPU::op_int_imm8);
	//InstallOp(0xCF,&x86CPU::op_iret);
	//InstallOp(0xCC,&x86CPU::op_int3);
	//InstallOp(0xCE,&x86CPU::op_into);
	InstallOp(0xE4,&x86CPU::op_in_al_imm8);
	InstallOp(0xE5,&x86CPU::op_in_axW_imm8);
	InstallOp(0x04,&x86CPU::op_add_al_imm8);
	InstallOp(0x05,&x86CPU::op_add_axW_immW);
	InstallOp(0x28,&x86CPU::op_sub_rm8_r8);
	InstallOp(0x80,&x86CPU::op_group_80);
	InstallOp(0x29,&x86CPU::op_sub_rmW_rW);
	InstallOp(0x2A,&x86CPU::op_sub_r8_rm8);
	InstallOp(0x2B,&x86CPU::op_sub_rW_rmW);
	InstallOp(0x81,&x86CPU::op_group_81);
	InstallOp(0x00,&x86CPU::op_add_rm8_r8);
	InstallOp(0x01,&x86CPU::op_add_rmW_rW);
	InstallOp(0x02,&x86CPU::op_add_r8_rm8);
	InstallOp(0x03,&x86CPU::op_add_rW_rmW);
	InstallOp(0xA0,&x86CPU::op_mov_al_m8);
	InstallOp(0xA1,&x86CPU::op_mov_axW_mW);
	InstallOp(0x88,&x86CPU::op_mov_rm8_r8);
	InstallOp(0x8A,&x86CPU::op_mov_r8_rm8);
	InstallOp(0xA2,&x86CPU::op_mov_m8_al);
	InstallOp(0xA3,&x86CPU::op_mov_mW_axW);
	InstallOp(0xC6,&x86CPU::op_mov_m8_imm8);
	InstallOp(0xC7,&x86CPU::op_mov_mW_immW);
	InstallOp(0x38,&x86CPU::op_cmp_rm8_r8);
	InstallOp(0x39,&x86CPU::op_cmp_rmW_rW);
	InstallOp(0x3A,&x86CPU::op_cmp_r8_rm8);
	InstallOp(0x3B,&x86CPU::op_cmp_rW_rmW);
	InstallOp(0x3C,&x86CPU::op_cmp_al_imm8);
	InstallOp(0x3D,&x86CPU::op_cmp_axW_immW); 
	InstallOp(0x83,&x86CPU::op_group_83);
	InstallOp(0xFF,&x86CPU::op_group_FF);
	InstallOp(0xE9,&x86CPU::op_jmp_relW);
	InstallOp(0xEA,&x86CPU::op_jmp_immF);
	InstallOp(0x6A,&x86CPU::op_push_imm8);
	InstallOp(0x8F,&x86CPU::op_group_8F);
	InstallOp(0xD6,&x86CPU::op_salc);
	InstallOp(0xF5,&x86CPU::op_cmc);
	InstallOp(0x98,&x86CPU::op_cbw);
	InstallOp(0x37,&x86CPU::op_aaa);
	InstallOp(0x27,&x86CPU::op_daa);
	InstallOp(0x2F,&x86CPU::op_das);
	InstallOp(0x3F,&x86CPU::op_aas);
	InstallOp(0xD5,&x86CPU::op_aad_imm8);
	InstallOp(0xD4,&x86CPU::op_aam_imm8);
	InstallOp(0xFE,&x86CPU::op_group_FE);
	InstallOp(0xF6,&x86CPU::op_group_F6);
	InstallOp(0xF7,&x86CPU::op_group_F7);
	InstallOp(0x99,&x86CPU::op_cwE);
	InstallOp(0x20,&x86CPU::op_and_rm8_r8);
	InstallOp(0x21,&x86CPU::op_and_rmW_rW);
	InstallOp(0x22,&x86CPU::op_and_r8_rm8);
	InstallOp(0x23,&x86CPU::op_and_rW_rmW);
	InstallOp(0x24,&x86CPU::op_and_al_imm8);
	InstallOp(0x25,&x86CPU::op_and_axW_immW);
	InstallOp(0x08,&x86CPU::op_or_rm8_r8);
	InstallOp(0x09,&x86CPU::op_or_rmW_rW);
	InstallOp(0x0A,&x86CPU::op_or_r8_rm8);
	InstallOp(0x0B,&x86CPU::op_or_rW_rmW);
	InstallOp(0x0C,&x86CPU::op_or_al_imm8);
	InstallOp(0x0D,&x86CPU::op_or_axW_immW);
	InstallOp(0xA6,&x86CPU::op_cmpsb);
	InstallOp(0xA7,&x86CPU::op_cmpsW);
	InstallOp(0xE3,&x86CPU::op_jcxzW_rel8);
	InstallOp(0x14,&x86CPU::op_adc_al_imm8);
	InstallOp(0x15,&x86CPU::op_adc_axW_immW);
	InstallOp(0x10,&x86CPU::op_adc_rm8_r8);
	InstallOp(0x11,&x86CPU::op_adc_rmW_rW);
	InstallOp(0x12,&x86CPU::op_adc_r8_rm8);
	InstallOp(0x13,&x86CPU::op_adc_rW_rmW);
	InstallOp(0x9E,&x86CPU::op_sahf);
	InstallOp(0x9F,&x86CPU::op_lahf);
	InstallOp(0xE1,&x86CPU::op_loopcc_rel8);
	InstallOp(0xE0,&x86CPU::op_loopcc_rel8);
	InstallOp(0xC5,&x86CPU::op_lds);
	InstallOp(0xC4,&x86CPU::op_les);
	InstallOp(0x8D,&x86CPU::op_lea);
	InstallOp(0xF0,&x86CPU::op_lock);
	InstallOp(0x30,&x86CPU::op_xor_rm8_r8);
	InstallOp(0x31,&x86CPU::op_xor_rmW_rW);
	InstallOp(0x32,&x86CPU::op_xor_r8_rm8);
	InstallOp(0x33,&x86CPU::op_xor_rW_rmW);
	InstallOp(0x34,&x86CPU::op_xor_al_imm8);
	InstallOp(0x35,&x86CPU::op_xor_axW_immW);
	InstallOp(0x1C,&x86CPU::op_sbb_al_imm8);
	InstallOp(0x1D,&x86CPU::op_sbb_axW_immW);
	InstallOp(0x19,&x86CPU::op_sbb_rmW_rW);
	InstallOp(0x1A,&x86CPU::op_sbb_r8_rm8);
	InstallOp(0x1B,&x86CPU::op_sbb_rW_rmW);
	InstallOp(0x18,&x86CPU::op_sub_rm8_r8);
	InstallOp(0x84,&x86CPU::op_test_rm8_r8);
	InstallOp(0x85,&x86CPU::op_test_rmW_rW);
	InstallOp(0xA8,&x86CPU::op_test_al_imm8);
	InstallOp(0xA9,&x86CPU::op_test_axW_immW);
	InstallOp(0x86,&x86CPU::op_xchg_rm8_r8);
	InstallOp(0x87,&x86CPU::op_xchg_rmW_rW);
	InstallOp(0xD2,&x86CPU::op_group_D2);
	InstallOp(0xD3,&x86CPU::op_group_D3);
	InstallOp(0xD0,&x86CPU::op_group_D0);
	InstallOp(0xD1,&x86CPU::op_group_D1);
	InstallOp(0xAC,&x86CPU::op_lodsb);
	InstallOp(0xAD,&x86CPU::op_lodsW);
	InstallOp(0xAE,&x86CPU::op_scasb);
	InstallOp(0xAF,&x86CPU::op_scasW);
	InstallOp(0x9B,&x86CPU::op_wait);
	InstallOp(0xD7,&x86CPU::op_xlatb);
	InstallOp(0xEC,&x86CPU::op_in_al_dx);
	InstallOp(0xED,&x86CPU::op_in_axW_dx);
	InstallOp(0xEE,&x86CPU::op_out_dx_al);
	InstallOp(0xEF,&x86CPU::op_out_dx_axW);
	InstallOp(0xAA,&x86CPU::op_stosb);
	InstallOp(0xAB,&x86CPU::op_stosW);

    for(int i=0;i<16;i++){
        InstallOp(0x70+i, &x86CPU::op_jcc_rel8, opcodes_hosted);
        InstallOp(0x80+i, &x86CPU::op_jcc_relW, opcodes_hosted_ext);
    }

    //two byte extended opcodes (new as of i286)
    InstallOp(0xB6,&x86CPU::op_movzx_rW_rm8, opcodes_hosted_ext);
    InstallOp(0xB7,&x86CPU::op_movzx_r32_rmW, opcodes_hosted_ext);





}


bool x86CPU::IsLocked(){
	return Memory->IsLocked();
}

void x86CPU::Lock(){
	//nothing...
	Memory->Lock();
	busmaster=1;
}

void x86CPU::Unlock(){
	//still nothing...
	Memory->Unlock();
	busmaster=0;
}


void x86CPU::ReadMemory(uint32_t address, uint32_t size, void* buffer){
    Memory->WaitLock(busmaster);
    Memory->Read(address, size, buffer);
}
void x86CPU::WriteMemory(uint32_t address, uint32_t size, void* buffer){
    Memory->WaitLock(busmaster);
    Memory->Write(address, size, buffer);
}


};






