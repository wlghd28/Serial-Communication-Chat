// �ø��� ��Ʈ ���� �����

#include <Windows.h>
#include "ComDlg.h"

BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
DWORD RecvData(LPVOID );			// ���� ���� ������

HWND	hWndCopy;					// �����带 ���� �ڵ��� ī��
HANDLE	hFile;						// ���� �ڵ�
int		nSwitch;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPreInst, 
				   LPSTR lpszCmdLine, int nCmdShow)
{
	// ���̾˷α� ǥ��
	DialogBox(hInst, "DLG_COM", HWND_DESKTOP, (DLGPROC)DlgProc);

	return TRUE;
}

BOOL CALLBACK DlgProc(HWND hWnd, UINT msg, 
					  WPARAM wParam, LPARAM lParam)
{
	char szBuffer[200]; 

	switch(msg){

	case WM_INITDIALOG:

		hWndCopy = hWnd;			// ���̾˷α׹ڽ� �ڵ�
		nSwitch = 1;
	
		// ����Ʈ ����
		hFile = CreateFile("COM3",
					GENERIC_READ | GENERIC_WRITE,
					0,				// �����
					0,				// ��ť��Ƽ �Ӽ�:������
					OPEN_EXISTING,	// ���� ���� ����
					0, 0 );			// �Ӽ�, ���÷���Ʈ
		
		if(hFile == INVALID_HANDLE_VALUE)
			MessageBox(0, "����Ʈ ���� ��������", "����", MB_OK);
		
		DCB				dcb;
		
		// ������ ��� ����̽��� ���� DCB ���� ���
		// DCB : Device Control Block ����̽� ���� ��
		GetCommState(hFile , &dcb);			
												
		// ������ ������ ���� �ڵ� �߰�
		//		:
		
		// DCB�� ������ ���� ��� ����̽� ����
		// �ϵ����� ���� ���� �ʱ�ȭ
		SetCommState(hFile , &dcb);			
		
		wsprintf(szBuffer, "BaudRate %d : ByteSize %d", dcb.BaudRate, dcb.ByteSize);
		
		// ���̾˷α׹ڽ� ĸ�ǿ� BaudRate:ByteSize ������ ǥ��
		SetWindowText(hWnd, szBuffer);

		COMMTIMEOUTS	cTimeout;

		// ���� �������� Ÿ�Ӿƿ� �ڷ� ���
		//GetCommTimeouts(hFile, &cTimeout);	

		cTimeout.ReadIntervalTimeout         = 1000;	
		cTimeout.ReadTotalTimeoutMultiplier  = 0;
		cTimeout.ReadTotalTimeoutConstant    = 1000;
		cTimeout.WriteTotalTimeoutMultiplier = 0;
		cTimeout.WriteTotalTimeoutConstant   = 0;

		// ������ ��� ����̽��� �б⾲�� Ÿ�Ӿƿ� ����
		SetCommTimeouts(hFile, &cTimeout);

		// ���� ������ �����Ͽ� ��ŸƮ
		DWORD			nThreadId;
		
		// �ø��� ��� ������ ���ſ� ������ ����
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecvData, NULL, 0, &nThreadId);
		
		return TRUE;

	case WM_COMMAND:

		switch(LOWORD(wParam)){
		
		case ID_EDIT_SEND:		

			// �۽��� ����Ʈ ��Ʈ���� ��ȭ�� ��
			if(HIWORD(wParam) == EN_CHANGE){		

				char szSend[10];
				
				// �۽��� ������ ���ۿ��� �о����
				GetDlgItemText(hWnd, ID_EDIT_SEND, szBuffer, sizeof(szBuffer));	

				// ���� ���� ��ġ ���
				int nPos = (int)SendDlgItemMessage(hWnd, ID_EDIT_SEND, EM_GETSEL, 0, 0);
													
				// ���� �׸� ���� ��ġ
				int nStart = LOWORD(nPos);				
				szSend[0] = szBuffer[nStart - 1];

				DWORD	nByte;

				// �ѹ��� �۽�
				WriteFile(hFile, szSend, 1, &nByte, 0);	
			}
			return TRUE;

		case IDCANCEL:

			nSwitch = 0;		// ������ �����

			// ����Ʈ �ݱ�
			CloseHandle(hFile);

			// ���̾˷α� �ݱ�
			EndDialog(hWnd, 0);

			return TRUE;
		}
	}
	
	return FALSE;
}
// �о���� ���� ������
DWORD RecvData( VOID * dummy )					
{
	DWORD	dwByte;
	char	szRecv[10];
	int		nRet;

	while(nSwitch){

		// �� ���� ����
		nRet = ReadFile(hFile, szRecv, 1, &dwByte, 0);
		
		// ReadFile()�� �����ϸ� 0�ܸ̿� ��ȯ, Ÿ�Ӿƿ��� ����
		if(dwByte == 1)
		{
			SetWindowText(hWndCopy, "�����߾�");

			SendDlgItemMessage(hWndCopy, ID_EDIT_RECV, WM_CHAR, szRecv[0], NULL);
		}
		else
			SetWindowText(hWndCopy, "���� �����");
	}
	return 0;
}
