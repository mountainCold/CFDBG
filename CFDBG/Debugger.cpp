#include <iostream>
#include "Debugger.h"
#include "Capstone.h"


// 提供函数用于打开目标进程和关闭句柄
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

// 接受一个路径，以调试的方式创建进程
void Debugger::open(LPCSTR file_Path)
{
	// 如果进程创建成功，用于接收进程线程的句柄和id
	PROCESS_INFORMATION process_info = { 0 };
	STARTUPINFOA startup_info = { sizeof(STARTUPINFOA) };

	// 调试方式创建进程
	BOOL result =  CreateProcessA(file_Path, nullptr, NULL, NULL, FALSE,
		DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,
		NULL, NULL, &startup_info, &process_info);

	// DEBUG_PROCESS 表示以调试的方式打开目标进程，并且
	//	当被调试创建新的进程时，同样接收新进程的调试信息。
	// DEBUG_ONLY_THIS_PROCESS 只调试目标进程，不调试
	//	目标进程创建的新的进程
	// CREATE_NEW_CONSOLE 表示新创建的 CUI 程序会使用一
	//	个独立的控制台运行，如果不写就和调试器共用控制台

	// 如果进程创建成功了，就关闭对应的句柄，防止句柄泄露
	if (result == TRUE)
	{
		CloseHandle(process_info.hThread);
		CloseHandle(process_info.hProcess);
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// 初始化反汇编引擎，必须在使用反汇编的函数前调用
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	Capstone::Init();
}

// 接收并处理调试事件
void Debugger::run()
{
	// 通过循环不断的从调试对象中获取到调试信息
	while (WaitForDebugEvent(&debug_event, INFINITE))
	{
		// 打开对应的进程和线程的句柄
		OpenHandles();

		// dwDebugEventCode 表示当前接收到的事件类型
		switch (debug_event.dwDebugEventCode)
		{
		case EXCEPTION_DEBUG_EVENT:     // 异常调试事件
			OnExceptionEvent(); 
			break;
		}

		// 在处理模块加载事件和进程创建事件的时候，对应的结构体
		// 中会提供两个字段，lpImageName 和 fUnicode，理论上
		// lpImageName 是一个指向目标进程内存空间指针，地址上
		// 保存了模块的名称，fUnicode用于标识名称是否是宽字符。
		// 但是，实际上这两个值没有任何的意义。可以通过搜索引擎
		// 搜索通过文件句柄找到模块名称(路径)获取。

		// 为了防止句柄泄露，应该关闭
		CloseHandles();

		// 向调试子系统返回当前的处理结果: 参数中的进程 id  和
		// 线程 id 必须是通过 WaitForDebugEvent 获取到的 id。
		// 因为被调试的可能是多个进程中的多个线程，需要进行区分。
		// 参数三是处理结果，处理成功了就应该返回 DBG_CONTINUE，
		// 假设处理失败，或者没有处理就应该返回 DBG_EXCEPTION_NOT_HANDLED   
		ContinueDebugEvent(
			debug_event.dwProcessId,
			debug_event.dwThreadId,
			ContinueStaus);
	}
}


// 用于处理接收到的所有异常事件
void Debugger::OnExceptionEvent()
{
	// 获取异常产生的地址以及异常的类型
	DWORD code = debug_event.u.Exception.ExceptionRecord.ExceptionCode;
	LPVOID addr = debug_event.u.Exception.ExceptionRecord.ExceptionAddress;

	// 输出异常的信息和产生的位置
	printf("Type(%08X): %p\n", code, addr);

	switch (code) 
	{
		// 设备访问异常: 和内存断点相关
	case EXCEPTION_ACCESS_VIOLATION:
		break;

		// 断点异常: int 3(0xCC) 会触发的异常
	case EXCEPTION_BREAKPOINT: 
	{
		// 当进程被创建的时候，操作系统会检测当前的
		// 进程是否处于被调试状态，如果被调试了，就
		// 会通过 int 3 设置一个软件断点，这个断点
		// 通常不需要处理。
		break;
	}

		// 硬件断点事件: TF单步  DrN断点
	case EXCEPTION_SINGLE_STEP:      
		break;
	}

	// 应该查看的是 eip 指向的位置，而不是异常的位置
	Capstone::DisAsm(process_handle, addr, 10);
	get_command();
}

// 获取用户的输入
void Debugger::get_command()
{
	char input[0x100] = { 0 };

	while (true)
	{
		// 获取指令，指令应该是事先考虑好的
		scanf_s("%s", input, 0x100);

		// 根据输入的指令执行不同的操作
		if (!strcmp(input, "g"))
		{
			// 结束输入，让程序继续执行，直到运行
			// 结束或者遇到下一个异常
			break;
		}
		else if (!strcmp(input, "u"))
		{
			// 查看指定位置的指定行汇编指令
			int addr = 0, lines = 0;
			scanf_s("%x %d", &addr, &lines);
			Capstone::DisAsm(process_handle, (LPVOID)addr, lines);
		}
	}
}
