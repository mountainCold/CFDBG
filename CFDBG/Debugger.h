#include <windows.h>

// 调试器类: 建立调试子系统，处理接收到的调试信息
//	获取用户的输入，并进行相应的输出反馈。
class Debugger
{
private:
	// 保存调试事件的结构体
	DEBUG_EVENT debug_event = { 0 };

	// 用于保存处理的结果
	DWORD ContinueStaus = DBG_CONTINUE;

	// 保存异常产生时对应的进程和线程句柄
	HANDLE thread_handle = NULL;
	HANDLE process_handle = NULL;

public:
	// 接受一个路径，以调试的方式创建进程
	void open(LPCSTR file_Path);

	// 接收并处理调试事件
	void run();

private:
	// 提供函数用于打开目标进程和关闭句柄
	void OpenHandles();
	void CloseHandles();

	// 用于处理接收到的所有异常事件
	void OnExceptionEvent();

	// 获取用户的输入
	void get_command();
};

