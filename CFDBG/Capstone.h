#pragma once
#include <windows.h>
#include "Capstone/include/capstone.h"
#pragma comment(lib,"capstone/capstone.lib")
#pragma comment(linker, "/NODEFAULTLIB:\"libcmtd.lib\"")

// ����������ࣨ�����ࣩ����Ҫ������ͨ������ĵ�ַ���ر�
//                     �����汣��Ĵ�����Ϣ�����Զ���
//                     ����ʽ���зḻ��

class Capstone
{
private:
	// ���ڳ�ʼ�����ڴ����ľ��
	static csh Handle;
	static cs_opt_mem OptMem;

public:
	// ����ΪĬ�Ϲ��캯��
	Capstone() = default;
	~Capstone() = default;

	// ���ڳ�ʼ���ĺ���
	static void Init();

	// ����ִ�з����ĺ���
	static void DisAsm(HANDLE Handle, LPVOID Addr, DWORD Count);
};

