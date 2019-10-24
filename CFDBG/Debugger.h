#include <windows.h>

// ��������: ����������ϵͳ��������յ��ĵ�����Ϣ
//	��ȡ�û������룬��������Ӧ�����������
class Debugger
{
private:
	// ��������¼��Ľṹ��
	DEBUG_EVENT debug_event = { 0 };

	// ���ڱ��洦��Ľ��
	DWORD ContinueStaus = DBG_CONTINUE;

	// �����쳣����ʱ��Ӧ�Ľ��̺��߳̾��
	HANDLE thread_handle = NULL;
	HANDLE process_handle = NULL;

public:
	// ����һ��·�����Ե��Եķ�ʽ��������
	void open(LPCSTR file_Path);

	// ���ղ���������¼�
	void run();

private:
	// �ṩ�������ڴ�Ŀ����̺͹رվ��
	void OpenHandles();
	void CloseHandles();

	// ���ڴ�����յ��������쳣�¼�
	void OnExceptionEvent();

	// ��ȡ�û�������
	void get_command();
};

