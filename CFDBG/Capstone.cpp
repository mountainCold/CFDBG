#include "Capstone.h"

csh Capstone::Handle = { 0 };
cs_opt_mem Capstone::OptMem = { 0 };

// ��ʼ�����������
void Capstone::Init()
{
	// ���öѿռ�Ļص�����
	OptMem.free = free;
	OptMem.calloc = calloc;
	OptMem.malloc = malloc;
	OptMem.realloc = realloc;
	OptMem.vsnprintf = (cs_vsnprintf_t)vsprintf_s;

	// ע��ѿռ�����麯��
	cs_option(NULL, CS_OPT_MEM, (size_t)& OptMem);

	// ��һ�����
	cs_open(CS_ARCH_X86, CS_MODE_32, &Capstone::Handle);
}


// �����ָ�����������
void Capstone::DisAsm(HANDLE Handle, LPVOID Addr, DWORD Count)
{
	// ������ȡָ��λ���ڴ�Ļ�������Ϣ
	cs_insn* ins = nullptr;
	PCHAR buff = new CHAR[Count * 16]{ 0 };

	// ��ȡָ�����ȵ��ڴ�ռ�
	DWORD dwWrite = 0;
	ReadProcessMemory(Handle, (LPVOID)Addr, buff, Count * 16, &dwWrite);
	int count = cs_disasm(Capstone::Handle, (uint8_t*)buff, Count * 16, (uint64_t)Addr, 0, &ins);
	
	// for ��������������»�������⣬������Ҫ�޸�����
	//	- ĳЩʱ�����öϵ�֮���������Чָ��
	for (DWORD i = 0; i < Count; ++i)
	{
		printf("%08X\t", (UINT)ins[i].address);
		for (uint16_t j = 0; j < 16; ++j)
		{
			if (j < ins[i].size)
				printf("%02X", ins[i].bytes[j]);
			else
				printf("  ");
		}
		// �����Ӧ�ķ����
		printf("\t%s %s\n", ins[i].mnemonic, ins[i].op_str);
	}
	printf("\n");
	// �ͷŶ�̬����Ŀռ�
	delete[] buff;
	cs_free(ins, count);
}
