#include <iostream>
#include "Debugger.h"
#include "Capstone.h"


// �ṩ�������ڴ�Ŀ����̺͹رվ��
void Debugger::OpenHandles()
{
	thread_handle = OpenThread(THREAD_ALL_ACCESS, FALSE, debug_event.dwThreadId);
	process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, debug_event.dwProcessId);
}


void Debugger::CloseHandles()
{
	CloseHandle(thread_handle);
	CloseHandle(process_handle);
}

// ����һ��·�����Ե��Եķ�ʽ��������
void Debugger::open(LPCSTR file_Path)
{
	// ������̴����ɹ������ڽ��ս����̵߳ľ����id
	PROCESS_INFORMATION process_info = { 0 };
	STARTUPINFOA startup_info = { sizeof(STARTUPINFOA) };

	// ���Է�ʽ��������
	BOOL result =  CreateProcessA(file_Path, nullptr, NULL, NULL, FALSE,
		DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,
		NULL, NULL, &startup_info, &process_info);

	// DEBUG_PROCESS ��ʾ�Ե��Եķ�ʽ��Ŀ����̣�����
	//	�������Դ����µĽ���ʱ��ͬ�������½��̵ĵ�����Ϣ��
	// DEBUG_ONLY_THIS_PROCESS ֻ����Ŀ����̣�������
	//	Ŀ����̴������µĽ���
	// CREATE_NEW_CONSOLE ��ʾ�´����� CUI �����ʹ��һ
	//	�������Ŀ���̨���У������д�ͺ͵��������ÿ���̨

	// ������̴����ɹ��ˣ��͹رն�Ӧ�ľ������ֹ���й¶
	if (result == TRUE)
	{
		CloseHandle(process_info.hThread);
		CloseHandle(process_info.hProcess);
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// ��ʼ����������棬������ʹ�÷����ĺ���ǰ����
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	Capstone::Init();
}

// ���ղ���������¼�
void Debugger::run()
{
	// ͨ��ѭ�����ϵĴӵ��Զ����л�ȡ��������Ϣ
	while (WaitForDebugEvent(&debug_event, INFINITE))
	{
		// �򿪶�Ӧ�Ľ��̺��̵߳ľ��
		OpenHandles();

		// dwDebugEventCode ��ʾ��ǰ���յ����¼�����
		switch (debug_event.dwDebugEventCode)
		{
		case EXCEPTION_DEBUG_EVENT:     // �쳣�����¼�
			OnExceptionEvent(); 
			break;
		}

		// �ڴ���ģ������¼��ͽ��̴����¼���ʱ�򣬶�Ӧ�Ľṹ��
		// �л��ṩ�����ֶΣ�lpImageName �� fUnicode��������
		// lpImageName ��һ��ָ��Ŀ������ڴ�ռ�ָ�룬��ַ��
		// ������ģ������ƣ�fUnicode���ڱ�ʶ�����Ƿ��ǿ��ַ���
		// ���ǣ�ʵ����������ֵû���κε����塣����ͨ����������
		// ����ͨ���ļ�����ҵ�ģ������(·��)��ȡ��

		// Ϊ�˷�ֹ���й¶��Ӧ�ùر�
		CloseHandles();

		// �������ϵͳ���ص�ǰ�Ĵ�����: �����еĽ��� id  ��
		// �߳� id ������ͨ�� WaitForDebugEvent ��ȡ���� id��
		// ��Ϊ�����ԵĿ����Ƕ�������еĶ���̣߳���Ҫ�������֡�
		// �������Ǵ�����������ɹ��˾�Ӧ�÷��� DBG_CONTINUE��
		// ���账��ʧ�ܣ�����û�д����Ӧ�÷��� DBG_EXCEPTION_NOT_HANDLED   
		ContinueDebugEvent(
			debug_event.dwProcessId,
			debug_event.dwThreadId,
			ContinueStaus);
	}
}


// ���ڴ�����յ��������쳣�¼�
void Debugger::OnExceptionEvent()
{
	// ��ȡ�쳣�����ĵ�ַ�Լ��쳣������
	DWORD code = debug_event.u.Exception.ExceptionRecord.ExceptionCode;
	LPVOID addr = debug_event.u.Exception.ExceptionRecord.ExceptionAddress;

	// ����쳣����Ϣ�Ͳ�����λ��
	printf("Type(%08X): %p\n", code, addr);

	switch (code) 
	{
		// �豸�����쳣: ���ڴ�ϵ����
	case EXCEPTION_ACCESS_VIOLATION:
		break;

		// �ϵ��쳣: int 3(0xCC) �ᴥ�����쳣
	case EXCEPTION_BREAKPOINT: 
	{
		// �����̱�������ʱ�򣬲���ϵͳ���⵱ǰ��
		// �����Ƿ��ڱ�����״̬������������ˣ���
		// ��ͨ�� int 3 ����һ������ϵ㣬����ϵ�
		// ͨ������Ҫ����
		break;
	}

		// Ӳ���ϵ��¼�: TF����  DrN�ϵ�
	case EXCEPTION_SINGLE_STEP:      
		break;
	}

	// Ӧ�ò鿴���� eip ָ���λ�ã��������쳣��λ��
	Capstone::DisAsm(process_handle, addr, 10);
	get_command();
}

// ��ȡ�û�������
void Debugger::get_command()
{
	char input[0x100] = { 0 };

	while (true)
	{
		// ��ȡָ�ָ��Ӧ�������ȿ��Ǻõ�
		scanf_s("%s", input, 0x100);

		// ���������ָ��ִ�в�ͬ�Ĳ���
		if (!strcmp(input, "g"))
		{
			// �������룬�ó������ִ�У�ֱ������
			// ��������������һ���쳣
			break;
		}
		else if (!strcmp(input, "u"))
		{
			// �鿴ָ��λ�õ�ָ���л��ָ��
			int addr = 0, lines = 0;
			scanf_s("%x %d", &addr, &lines);
			Capstone::DisAsm(process_handle, (LPVOID)addr, lines);
		}
	}
}
