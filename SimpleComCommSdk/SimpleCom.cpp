// 시리얼 포트 직접 입출력

#include <Windows.h>
#include "ComDlg.h"

BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
DWORD RecvData(LPVOID );			// 수신 전용 스레드

HWND	hWndCopy;					// 스레드를 위한 핸들의 카피
HANDLE	hFile;						// 파일 핸들
int		nSwitch;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPreInst, 
				   LPSTR lpszCmdLine, int nCmdShow)
{
	// 다이알로그 표시
	DialogBox(hInst, "DLG_COM", HWND_DESKTOP, (DLGPROC)DlgProc);

	return TRUE;
}

BOOL CALLBACK DlgProc(HWND hWnd, UINT msg, 
					  WPARAM wParam, LPARAM lParam)
{
	char szBuffer[200]; 

	switch(msg){

	case WM_INITDIALOG:

		hWndCopy = hWnd;			// 다이알로그박스 핸들
		nSwitch = 1;
	
		// 컴포트 열기
		hFile = CreateFile("COM3",
					GENERIC_READ | GENERIC_WRITE,
					0,				// 비공유
					0,				// 시큐리티 속성:사용안함
					OPEN_EXISTING,	// 기존 파일 오픈
					0, 0 );			// 속성, 템플레이트
		
		if(hFile == INVALID_HANDLE_VALUE)
			MessageBox(0, "컴포트 열때 에러났어", "에공", MB_OK);
		
		DCB				dcb;
		
		// 지정한 통신 디바이스의 현재 DCB 설정 얻기
		// DCB : Device Control Block 디바이스 제어 블럭
		GetCommState(hFile , &dcb);			
												
		// 설정을 변경할 때는 코드 추가
		//		:
		
		// DCB의 지정에 따라 통신 디바이스 구성
		// 하드웨어와 제어 설정 초기화
		SetCommState(hFile , &dcb);			
		
		wsprintf(szBuffer, "BaudRate %d : ByteSize %d", dcb.BaudRate, dcb.ByteSize);
		
		// 다이알로그박스 캡션에 BaudRate:ByteSize 사이즈 표시
		SetWindowText(hWnd, szBuffer);

		COMMTIMEOUTS	cTimeout;

		// 현재 설정중인 타임아웃 자료 얻기
		//GetCommTimeouts(hFile, &cTimeout);	

		cTimeout.ReadIntervalTimeout         = 1000;	
		cTimeout.ReadTotalTimeoutMultiplier  = 0;
		cTimeout.ReadTotalTimeoutConstant    = 1000;
		cTimeout.WriteTotalTimeoutMultiplier = 0;
		cTimeout.WriteTotalTimeoutConstant   = 0;

		// 지정한 통신 디바이스의 읽기쓰기 타임아웃 설정
		SetCommTimeouts(hFile, &cTimeout);

		// 수신 스래드 생성하여 스타트
		DWORD			nThreadId;
		
		// 시리얼 통신 데이터 수신용 스레드 생성
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecvData, NULL, 0, &nThreadId);
		
		return TRUE;

	case WM_COMMAND:

		switch(LOWORD(wParam)){
		
		case ID_EDIT_SEND:		

			// 송신쪽 에디트 컨트롤이 변화할 때
			if(HIWORD(wParam) == EN_CHANGE){		

				char szSend[10];
				
				// 송신쪽 데이터 버퍼에서 읽어오기
				GetDlgItemText(hWnd, ID_EDIT_SEND, szBuffer, sizeof(szBuffer));	

				// 선택 문자 위치 얻기
				int nPos = (int)SendDlgItemMessage(hWnd, ID_EDIT_SEND, EM_GETSEL, 0, 0);
													
				// 선택 항목 개시 위치
				int nStart = LOWORD(nPos);				
				szSend[0] = szBuffer[nStart - 1];

				DWORD	nByte;

				// 한문자 송신
				WriteFile(hFile, szSend, 1, &nByte, 0);	
			}
			return TRUE;

		case IDCANCEL:

			nSwitch = 0;		// 스래드 종료용

			// 컴포트 닫기
			CloseHandle(hFile);

			// 다이알로그 닫기
			EndDialog(hWnd, 0);

			return TRUE;
		}
	}
	
	return FALSE;
}
// 읽어오기 전용 스레드
DWORD RecvData( VOID * dummy )					
{
	DWORD	dwByte;
	char	szRecv[10];
	int		nRet;

	while(nSwitch){

		// 한 문자 수신
		nRet = ReadFile(hFile, szRecv, 1, &dwByte, 0);
		
		// ReadFile()은 성공하면 0이외를 반환, 타임아웃도 성공
		if(dwByte == 1)
		{
			SetWindowText(hWndCopy, "수신했어");

			SendDlgItemMessage(hWndCopy, ID_EDIT_RECV, WM_CHAR, szRecv[0], NULL);
		}
		else
			SetWindowText(hWndCopy, "수신 대기중");
	}
	return 0;
}
