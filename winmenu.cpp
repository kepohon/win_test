// winmain.cpp (Shift-JIS)
//	���\�[�X�R���p�C����windress��Shift-Jis�`���̃t�@�C���łȂ��ƃR���p�C���ł��Ȃ��̂ŁA���̃t�@�C����Shift-JIS�œ��ꂵ��
//	2023/05/15 by kepohon
//	2023/05/20 key by kepohon
//	2023/05/21 keyUp,keyDown,mouse by kepohon

#define WIN32_LEAN_AND_MEAN	// �s�v�ȃw�b�_�t�@�C���̃C���N���[�h��}�~���ăR���p�C�����Ԃ�Z�k�ł���

#include	<windows.h>
#include	<stdlib.h>
#include	<time.h>
#include	<mmsystem.h>	// libwinmm.a ���g�����߂̃w�b�_�[�t�@�C�� 
#include	"resource.h"
#include	"apri.h"

#define	ID_MYTIMER	100

// �v���g�^�C�v�錾
int WINAPI		WinMain(HINSTANCE, HINSTANCE, LPSTR, int); 
bool			CreateMainWindow(HINSTANCE, int);
LRESULT WINAPI	WindowProc(HWND, UINT, WPARAM, LPARAM); 
int				TypeStart(HWND);
void			DrawRect(HWND, POINTS, POINTS);


// �O���[�o���ϐ�
HINSTANCE	hInst;
TCHAR		szMondai[32], szInput[32], szCheck[32];
int			iMon;
DWORD		dwStart, dwEnd;
BOOL		bStart = FALSE, bSeikai = TRUE;
TCHAR ch = ' ';             // character entered
RECT rect;                  // rectangle
bool vkKeys[256];           // state of virtual keys, false or true
POINTS		start, end, old_end;	// ��`�̎n�_�I�_

// �萔
const char CLASS_NAME[]  = "WinMain";
const char WIN_TITLE[]   = "�^�C�s���O���K";   // �^�C�g���o�[�ɕ\������镶����
const int  WINDOW_WIDTH  = 640;             // �E�B���h�E�̕�
const int  WINDOW_HEIGHT = 640;             // �E�B���h�E�̍���

// �E�B���h�E�Y�v���O�����̃G���g���[�|�C���g
int WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpCmdLine,
                    int       nCmdShow)
{
    MSG	 msg;
	hInst = hInstance;

    // �E�B���h�E�̍쐬
    if (!CreateMainWindow(hInstance, nCmdShow)){
        return false;
	}

    // ���C�� ���b�Z�[�W���[�v
    int done = 0;
    while (!done)
    {
        // PeekMessage is a non-blocking method for checking for Windows messages.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)		// QUIT���b�Z�[�W�Ȃ�I��
                done = 1;

            TranslateMessage(&msg);			//decode and pass messages on to WindowProc
            DispatchMessage(&msg);
        } else {
			// �Q�[���̂悤�ȃ��A���^�C�������́A�����ōs��
		}
    }
    return msg.wParam;
}

//=============================================================================
// �E�B���h�E�N���X�̓o�^�ƃE�B���h�E�̍쐬
// �߂�l: �G���[������false
//=============================================================================
bool CreateMainWindow(HINSTANCE hInstance, int nCmdShow) 
{ 
    WNDCLASSEX	wcx; 
    HWND		hWnd;
 
    // �E�B���h�E�N���X�̓o�^ 
    wcx.cbSize = sizeof(wcx);
    wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS; 
    wcx.lpfnWndProc = WindowProc; 
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 0;
    wcx.hInstance = hInstance;          // handle to instance 
    wcx.hIcon = NULL; 
    wcx.hCursor = LoadCursor(NULL,IDC_ARROW);
    wcx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wcx.lpszMenuName =  NULL;
    wcx.lpszClassName = CLASS_NAME;
    wcx.hIconSm = NULL;
 
    if (RegisterClassEx(&wcx) == 0)    // �G���[��������false��Ԃ�
        return false;


    // �E�B���h�E�쐬
    hWnd = CreateWindow(
			CLASS_NAME,
			WIN_TITLE,				// �^�C�g���o�[�̕�����
			WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,    // window�X�^�C��
			CW_USEDEFAULT,          // �E�B���h�E�̐����ʒu
			CW_USEDEFAULT,          // �E�B���h�E�̐����ʒu
			WINDOW_WIDTH,           // �E�B���h�E�̕�
			WINDOW_HEIGHT,          // �E�B���h�E�̍���
			(HWND) NULL,            // �e�E�B���h�E
			(HMENU) NULL,           // ���j���[�o�[
			hInstance,              // �C���X�^���X�n���h��
			(LPVOID) NULL			// no window parameters
		);

    if (!hWnd)		// �G���[��������false��Ԃ�
        return false;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);    // Send a WM_PAINT message to the window procedure
	
    return true;
}

//=============================================================================
// �E�B���h�E�C�x���g�̃R�[���o�b�N�֐�
//=============================================================================
LRESULT WINAPI WindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	HDC				hdc, hdc_mem;
	PAINTSTRUCT		ps;
	static HMENU	hMenu;
	int				wmID;
	int				id;
	static TCHAR	szTime[64], szDate[64];
	SYSTEMTIME		st;			// ���ݓ����ۑ��p
	HBITMAP			hBmp;
	BITMAP			bmp_info;
	int				w, h;
	MMTIME			mm;
	
    short nVirtKey;                 // virtual-key code 
    const short SHIFTED = (short)0x8000; 
    TEXTMETRIC tm;                  // structure for text metrics 
    DWORD chWidth = 20;             // width of characters
    DWORD chHeight = 20;            // height of characters
	
	HBRUSH			hBrush;
	static BOOL		bDraw;
	TCHAR			szBuf[32];
	
    switch( msg )
    {
    case WM_CREATE:   // �E�B���h�E���쐬���ꂽ��
        hMenu = LoadMenu(NULL, "WIN_MENU"); // ���j���[�o�[���\�[�X�����[�h
        SetMenu(hWnd, hMenu);               // �E�B���h�E�Ƀ��j���[���\�[�X���蓖��
		SetTimer(hWnd, ID_MYTIMER, 1000, NULL);	// ���v�p�^�C�}�[�Z�b�g
		srand((unsigned)time(NULL));		// �����̎���܂��A�^�C�s���O���K�p
		
		// get the text metrics
		hdc = GetDC(hWnd);
		GetTextMetrics(hdc, &tm);
		ReleaseDC(hWnd, hdc);
		chWidth = tm.tmAveCharWidth;        // average character width
		chHeight = tm.tmHeight;             // character height
        return 0;
	
    case WM_CLOSE:    // �E�B���h�E��������Ƃ�
        id = MessageBox(hWnd, TEXT("�I�����܂���?"),
            TEXT("�I��"), MB_YESNO | MB_ICONEXCLAMATION);
        if (id == IDYES){
            DestroyWindow(hWnd);        SetMenu(hWnd, NULL);  // �E�B���h�E���烁�j���[�폜
			DestroyMenu(hMenu);   // ���j���[�o�[���\�[�X�j��
			hMenu = NULL;
		}
        break;                // �E�B���h�E����鏈����DefWindowProc()�ɔC����

    case WM_DESTROY:
        PostQuitMessage(0);			// ���b�Z�[�W���[�v��WM_QUIT���b�Z�[�W�𔭐�������
        return 0;
		
	case WM_TIMER:
		if (wParam != ID_MYTIMER)	// �����̃^�C�}�[�łȂ���΁A�������E�B���h�E�Y�ɓn��
			return (DefWindowProc(hWnd, msg, wParam, lParam));
		GetLocalTime(&st);		// ���ݓ����̎擾
		wsprintf(szTime, TEXT("%02d:%02d:%02d"), st.wHour, st.wMinute, st.wSecond);	// ��ʕ\���p�f�[�^
		wsprintf(szDate, TEXT("%02d�N%02d��%02d��"), st.wYear, st.wMonth, st.wDay);
		InvalidateRect(hWnd, NULL, TRUE);	// �`��̈���ĕ`�悷��w�����o��
		//SetWindowText(hWnd, szTime);
		break;
		
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);	// hdc�擾
		
		// �r�b�g�}�b�v�摜�\��
		hBmp = LoadBitmap(hInst, TEXT("MYBMP"));
		GetObject(hBmp, (int)sizeof(BITMAP), &bmp_info);
		w = bmp_info.bmWidth;
		h = bmp_info.bmHeight;
		hdc_mem = CreateCompatibleDC(hdc);
		SelectObject(hdc_mem, hBmp);
		BitBlt(hdc, 0, 0, w, h, hdc_mem, 0, 0, SRCCOPY);
		StretchBlt(hdc, w, 0, w * 2, h * 2, hdc_mem, 0, 0, w, h, SRCCOPY);
		DeleteDC(hdc_mem);
		DeleteObject(hBmp);
		
		// ���v�\��
		TextOut(hdc, 5, 5, szDate, lstrlen(szDate));
		TextOut(hdc, 10, 20, szTime, lstrlen(szTime));
		
		// �^�C�s���O���K�̖��\��
		TextOut(hdc, 100, 100, szMondai, lstrlen(szMondai));
		TextOut(hdc, 100, 140, szInput, lstrlen(szInput));
		if (bSeikai)
			SetTextColor(hdc, RGB(0, 0, 0));
		else
			SetTextColor(hdc, RGB(255, 0, 0));
		TextOut(hdc, 100, 180, szCheck, lstrlen(szCheck));
		
            TextOut(hdc, 0, 0, &ch, 1);     // display the character
            
            // Display the state of vkKeys array
            // Display 'T' if key is down and 'F' is key is up
            for (int r=0; r<16; r++)
            {
                for (int c=0; c<16; c++)
                {
                    if (vkKeys[r*16+c])
                    {
                        SetBkMode(hdc, OPAQUE);         // opaque text background
                        TextOut(hdc,c*chWidth+chWidth*2,r*chHeight+chHeight*2,"T ", 2);
                    } else {
                        SetBkMode(hdc, TRANSPARENT);    // transparent text background
                        TextOut(hdc,c*chWidth+chWidth*2,r*chHeight+chHeight*2,"F ", 2);
                    }
                }
            }

		
		EndPaint(hWnd, &ps);
		break;
		
    case WM_COMMAND:  // ���j���[���I�����ꂽ��
        wmID = LOWORD(wParam);
        switch (wmID) {
        case IDM_OPEN: MessageBox(NULL, "�J��", "���b�Z�[�W", MB_OK);
            break;
        case IDM_SAVE: MessageBox(NULL, "�ۑ�", "���b�Z�[�W", MB_OK);
            break;
        case IDM_END:
            PostMessage(hWnd, WM_CLOSE, 0, 0);            //�I�����b�Z�[�W�𔭐�������
			// DestroyWindow(hWnd); // �I��
            break;
        case IDM_CUT: MessageBox(NULL, "�؂���", "���b�Z�[�W", MB_OK);
            break;
        case IDM_COPY: MessageBox(NULL, "�R�s�[", "���b�Z�[�W", MB_OK);
            break;
        case IDM_PASTE: MessageBox(NULL, "�\\��t��", "���b�Z�[�W", MB_OK);
            break;
        case IDM_TOOLBAR: MessageBox(NULL, "�c�[���o�[", "���b�Z�[�W", MB_OK);
            break;
        case IDM_DISP1:
			PlaySound(TEXT("cat.wav"), NULL, SND_ASYNC | SND_FILENAME);	// libwinmm.a�Ƃ̃����N���K�v
			MessageBox(NULL, "�C", "���b�Z�[�W", MB_OK);
            break;
        case IDM_DISP2: MessageBox(NULL, "��", "���b�Z�[�W", MB_OK);
            break;
        case IDM_DISP3: MessageBox(NULL, "�n", "���b�Z�[�W", MB_OK);
            break;
        case IDM_SOURCE: MessageBox(NULL, "�\\�[�X", "���b�Z�[�W", MB_OK);
            break;
        case IDM_HELP: MessageBox(NULL, "�w���v", "���b�Z�[�W", MB_OK);
            break;
        case IDM_VERSION: MessageBox(NULL, "ver1.00", "�o�[�W�������", MB_OK);
            break;
        }
        break;
	
	case WM_CHAR:		// �������͂��������Ƃ��̏���
		ch = (TCHAR) wParam;    // get the character
		if (wParam == VK_SPACE && !bStart) {	// �X�y�[�X�L�[�������ꂽ��^�C�s���O�J�n
			bStart = TRUE;
			TypeStart(hWnd);
			break;
		}
		if (bStart == FALSE)		// �^�C�s���O���K���łȂ���Ώ������E�B���h�E�Y�ɔC����
			return DefWindowProc(hWnd, msg, wParam, lParam);
		if (wParam == VK_ESCAPE) {		// �G�X�P�[�v�L�[�������ꂽ��^�C�s���O�I��
			lstrcpy(szMondai, TEXT(""));
			lstrcpy(szInput, TEXT(""));
			lstrcpy(szCheck, TEXT(""));
			InvalidateRect(hWnd, NULL, TRUE);
			bStart = FALSE;
			break;
		}
		wsprintf(szInput, TEXT("���Ȃ��̓���=\"%c\""), (int)wParam);
		if (iMon == (int)wParam) {
			bSeikai = TRUE;
			
			mm.wType = TIME_MS;
			timeGetSystemTime(&mm, sizeof(MMTIME));
			dwEnd = mm.u.ms;
			wsprintf(szCheck, TEXT("��������[%d�~���b]"), dwEnd - dwStart);
			TypeStart(hWnd);
		} else {
			bSeikai = FALSE;
			MessageBeep(MB_OK);
			lstrcpy(szCheck, TEXT("�^�C�v�~�X�I"));
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;
		
	case WM_KEYDOWN:                                // key down
		vkKeys[wParam] = true;
		switch(wParam)
		{
			case VK_SHIFT:                          // shift key
				nVirtKey = GetKeyState(VK_LSHIFT);  // get state of left shift
				if (nVirtKey & SHIFTED)             // if left shift
					vkKeys[VK_LSHIFT] = true;
				nVirtKey = GetKeyState(VK_RSHIFT);  // get state of right shift
				if (nVirtKey & SHIFTED)             // if right shift
					vkKeys[VK_RSHIFT] = true;
				break;
				
			case VK_CONTROL:                        // control key
				nVirtKey = GetKeyState(VK_LCONTROL);
				if (nVirtKey & SHIFTED)             // if left control
					vkKeys[VK_LCONTROL] = true;
				nVirtKey = GetKeyState(VK_RCONTROL);
				if (nVirtKey & SHIFTED)             // if right control
					vkKeys[VK_RCONTROL] = true;
				break;
				
		}
		InvalidateRect(hWnd, NULL, TRUE);           // force WM_PAINT
			return 0;
		break;
		
	case WM_KEYUP:                                  // key up
		vkKeys[wParam] = false;
		switch(wParam)
		{
			case VK_SHIFT:                          // shift key
				nVirtKey = GetKeyState(VK_LSHIFT); 
				if ((nVirtKey & SHIFTED) == 0)      // if left shift
					vkKeys[VK_LSHIFT] = false;
				nVirtKey = GetKeyState(VK_RSHIFT); 
				if ((nVirtKey & SHIFTED) == 0)      // if right shift
					vkKeys[VK_RSHIFT] = false;
				break;
				
			case VK_CONTROL:                        // control key
				nVirtKey = GetKeyState(VK_LCONTROL);
				if ((nVirtKey & SHIFTED) == 0)      // if left control
					vkKeys[VK_LCONTROL] = false;
				nVirtKey = GetKeyState(VK_RCONTROL);
				if ((nVirtKey & SHIFTED) == 0)      // if right control
					vkKeys[VK_RCONTROL] = false;
				break;
		}
		InvalidateRect(hWnd, NULL, TRUE);    // force WM_PAINT
		return 0;
		break;
	
	case WM_LBUTTONDOWN:
		PlaySound(TEXT("cat.wav"), NULL, SND_ASYNC | SND_FILENAME);	// libwinmm.a�Ƃ̃����N���K�v
		bDraw = TRUE;
		old_end = start = MAKEPOINTS(lParam);
		DrawRect(hWnd, start, old_end);
		break;
		
	case WM_MOUSEMOVE:
		if (bDraw) {
			end = MAKEPOINTS(lParam);
			DrawRect(hWnd, start, old_end);
			DrawRect(hWnd, start, end);
			old_end = end;
		} else {
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		break;
		
	case WM_LBUTTONUP:
		if (bDraw) {
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			DrawRect(hWnd, start, end);
			bDraw = FALSE;
			wsprintf(szBuf, TEXT("(%d, %d) - (%d, %d)"), start.x, start.y, end.x, end.y);
			SetWindowText(hWnd, szBuf);
			hdc = GetDC(hWnd);
			hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
			SelectObject(hdc, hBrush);
			Ellipse(hdc, start.x, start.y, end.x, end.y);
			ReleaseDC(hWnd, hdc);
		} else {
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		break;

	case WM_LBUTTONDBLCLK:		// �_�u���N���b�N���ꂽ�� WM_CLOSE���b�Z�[�W���o��
		SendMessage(hWnd, WM_CLOSE, 0, 0);
		break;
		
	default:
		return DefWindowProc( hWnd, msg, wParam, lParam );
    }
    return 0;
}

int	TypeStart(HWND hWnd)
{
	int		n;
	MMTIME	mm;
	
	n = rand() % 26;
	iMon = 'a' + n;
	wsprintf(szMondai, TEXT("���=\"%c\""), iMon);
	
	mm.wType = TIME_MS;
	timeGetSystemTime(&mm, sizeof(MMTIME));
	dwStart = mm.u.ms;
	InvalidateRect(hWnd, NULL, TRUE);
	return 0;
}

void DrawRect(HWND hWnd, POINTS beg, POINTS end)
{
	HDC		hdc;
	
	hdc = GetDC(hWnd);
	SetROP2(hdc, R2_NOT);
	
	MoveToEx(hdc, beg.x, beg.y, NULL);
	LineTo(hdc, end.x, beg.y);
	LineTo(hdc, end.x, end.y);
	LineTo(hdc, beg.x, end.y);
	LineTo(hdc, beg.x, beg.y);
	LineTo(hdc, end.x, end.y);
	
	ReleaseDC(hWnd, hdc);
	return;
}


