// winmain.cpp (Shift-JIS)
//	リソースコンパイラのwindressはShift-Jis形式のファイルでないとコンパイルできないので、他のファイルもShift-JISで統一した
//	2023/05/15 by kepohon
//	2023/05/20 key by kepohon
//	2023/05/21 keyUp,keyDown,mouse by kepohon

#define WIN32_LEAN_AND_MEAN	// 不要なヘッダファイルのインクルードを抑止してコンパイル時間を短縮できる

#include	<windows.h>
#include	<stdlib.h>
#include	<time.h>
#include	<mmsystem.h>	// libwinmm.a を使うためのヘッダーファイル 
#include	"resource.h"
#include	"apri.h"

#define	ID_MYTIMER	100

// プロトタイプ宣言
int WINAPI		WinMain(HINSTANCE, HINSTANCE, LPSTR, int); 
bool			CreateMainWindow(HINSTANCE, int);
LRESULT WINAPI	WindowProc(HWND, UINT, WPARAM, LPARAM); 
int				TypeStart(HWND);
void			DrawRect(HWND, POINTS, POINTS);


// グローバル変数
HINSTANCE	hInst;
TCHAR		szMondai[32], szInput[32], szCheck[32];
int			iMon;
DWORD		dwStart, dwEnd;
BOOL		bStart = FALSE, bSeikai = TRUE;
TCHAR ch = ' ';             // character entered
RECT rect;                  // rectangle
bool vkKeys[256];           // state of virtual keys, false or true
POINTS		start, end, old_end;	// 矩形の始点終点

// 定数
const char CLASS_NAME[]  = "WinMain";
const char WIN_TITLE[]   = "タイピング練習";   // タイトルバーに表示される文字列
const int  WINDOW_WIDTH  = 640;             // ウィンドウの幅
const int  WINDOW_HEIGHT = 640;             // ウィンドウの高さ

// ウィンドウズプログラムのエントリーポイント
int WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpCmdLine,
                    int       nCmdShow)
{
    MSG	 msg;
	hInst = hInstance;

    // ウィンドウの作成
    if (!CreateMainWindow(hInstance, nCmdShow)){
        return false;
	}

    // メイン メッセージループ
    int done = 0;
    while (!done)
    {
        // PeekMessage is a non-blocking method for checking for Windows messages.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)		// QUITメッセージなら終了
                done = 1;

            TranslateMessage(&msg);			//decode and pass messages on to WindowProc
            DispatchMessage(&msg);
        } else {
			// ゲームのようなリアルタイム処理は、ここで行う
		}
    }
    return msg.wParam;
}

//=============================================================================
// ウィンドウクラスの登録とウィンドウの作成
// 戻り値: エラー発生時false
//=============================================================================
bool CreateMainWindow(HINSTANCE hInstance, int nCmdShow) 
{ 
    WNDCLASSEX	wcx; 
    HWND		hWnd;
 
    // ウィンドウクラスの登録 
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
 
    if (RegisterClassEx(&wcx) == 0)    // エラー発生時はfalseを返す
        return false;


    // ウィンドウ作成
    hWnd = CreateWindow(
			CLASS_NAME,
			WIN_TITLE,				// タイトルバーの文字列
			WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,    // windowスタイル
			CW_USEDEFAULT,          // ウィンドウの水平位置
			CW_USEDEFAULT,          // ウィンドウの垂直位置
			WINDOW_WIDTH,           // ウィンドウの幅
			WINDOW_HEIGHT,          // ウィンドウの高さ
			(HWND) NULL,            // 親ウィンドウ
			(HMENU) NULL,           // メニューバー
			hInstance,              // インスタンスハンドル
			(LPVOID) NULL			// no window parameters
		);

    if (!hWnd)		// エラー発生時はfalseを返す
        return false;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);    // Send a WM_PAINT message to the window procedure
	
    return true;
}

//=============================================================================
// ウィンドウイベントのコールバック関数
//=============================================================================
LRESULT WINAPI WindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	HDC				hdc, hdc_mem;
	PAINTSTRUCT		ps;
	static HMENU	hMenu;
	int				wmID;
	int				id;
	static TCHAR	szTime[64], szDate[64];
	SYSTEMTIME		st;			// 現在日時保存用
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
    case WM_CREATE:   // ウィンドウが作成されたら
        hMenu = LoadMenu(NULL, "WIN_MENU"); // メニューバーリソースをロード
        SetMenu(hWnd, hMenu);               // ウィンドウにメニューリソース割り当て
		SetTimer(hWnd, ID_MYTIMER, 1000, NULL);	// 時計用タイマーセット
		srand((unsigned)time(NULL));		// 乱数の種をまく、タイピング練習用
		
		// get the text metrics
		hdc = GetDC(hWnd);
		GetTextMetrics(hdc, &tm);
		ReleaseDC(hWnd, hdc);
		chWidth = tm.tmAveCharWidth;        // average character width
		chHeight = tm.tmHeight;             // character height
        return 0;
	
    case WM_CLOSE:    // ウィンドウが閉じられるとき
        id = MessageBox(hWnd, TEXT("終了しますか?"),
            TEXT("終了"), MB_YESNO | MB_ICONEXCLAMATION);
        if (id == IDYES){
            DestroyWindow(hWnd);        SetMenu(hWnd, NULL);  // ウィンドウからメニュー削除
			DestroyMenu(hMenu);   // メニューバーリソース破棄
			hMenu = NULL;
		}
        break;                // ウィンドウを閉じる処理はDefWindowProc()に任せる

    case WM_DESTROY:
        PostQuitMessage(0);			// メッセージループでWM_QUITメッセージを発生させる
        return 0;
		
	case WM_TIMER:
		if (wParam != ID_MYTIMER)	// 自分のタイマーでなければ、処理をウィンドウズに渡す
			return (DefWindowProc(hWnd, msg, wParam, lParam));
		GetLocalTime(&st);		// 現在日時の取得
		wsprintf(szTime, TEXT("%02d:%02d:%02d"), st.wHour, st.wMinute, st.wSecond);	// 画面表示用データ
		wsprintf(szDate, TEXT("%02d年%02d月%02d日"), st.wYear, st.wMonth, st.wDay);
		InvalidateRect(hWnd, NULL, TRUE);	// 描画領域を再描画する指示を出す
		//SetWindowText(hWnd, szTime);
		break;
		
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);	// hdc取得
		
		// ビットマップ画像表示
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
		
		// 時計表示
		TextOut(hdc, 5, 5, szDate, lstrlen(szDate));
		TextOut(hdc, 10, 20, szTime, lstrlen(szTime));
		
		// タイピング練習の問題表示
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
		
    case WM_COMMAND:  // メニューが選択されたら
        wmID = LOWORD(wParam);
        switch (wmID) {
        case IDM_OPEN: MessageBox(NULL, "開く", "メッセージ", MB_OK);
            break;
        case IDM_SAVE: MessageBox(NULL, "保存", "メッセージ", MB_OK);
            break;
        case IDM_END:
            PostMessage(hWnd, WM_CLOSE, 0, 0);            //終了メッセージを発生させる
			// DestroyWindow(hWnd); // 終了
            break;
        case IDM_CUT: MessageBox(NULL, "切り取り", "メッセージ", MB_OK);
            break;
        case IDM_COPY: MessageBox(NULL, "コピー", "メッセージ", MB_OK);
            break;
        case IDM_PASTE: MessageBox(NULL, "貼\り付け", "メッセージ", MB_OK);
            break;
        case IDM_TOOLBAR: MessageBox(NULL, "ツールバー", "メッセージ", MB_OK);
            break;
        case IDM_DISP1:
			PlaySound(TEXT("cat.wav"), NULL, SND_ASYNC | SND_FILENAME);	// libwinmm.aとのリンクが必要
			MessageBox(NULL, "イ", "メッセージ", MB_OK);
            break;
        case IDM_DISP2: MessageBox(NULL, "ロ", "メッセージ", MB_OK);
            break;
        case IDM_DISP3: MessageBox(NULL, "ハ", "メッセージ", MB_OK);
            break;
        case IDM_SOURCE: MessageBox(NULL, "ソ\ース", "メッセージ", MB_OK);
            break;
        case IDM_HELP: MessageBox(NULL, "ヘルプ", "メッセージ", MB_OK);
            break;
        case IDM_VERSION: MessageBox(NULL, "ver1.00", "バージョン情報", MB_OK);
            break;
        }
        break;
	
	case WM_CHAR:		// 文字入力があったときの処理
		ch = (TCHAR) wParam;    // get the character
		if (wParam == VK_SPACE && !bStart) {	// スペースキーが押されたらタイピング開始
			bStart = TRUE;
			TypeStart(hWnd);
			break;
		}
		if (bStart == FALSE)		// タイピング練習中でなければ処理をウィンドウズに任せる
			return DefWindowProc(hWnd, msg, wParam, lParam);
		if (wParam == VK_ESCAPE) {		// エスケープキーが押されたらタイピング終了
			lstrcpy(szMondai, TEXT(""));
			lstrcpy(szInput, TEXT(""));
			lstrcpy(szCheck, TEXT(""));
			InvalidateRect(hWnd, NULL, TRUE);
			bStart = FALSE;
			break;
		}
		wsprintf(szInput, TEXT("あなたの入力=\"%c\""), (int)wParam);
		if (iMon == (int)wParam) {
			bSeikai = TRUE;
			
			mm.wType = TIME_MS;
			timeGetSystemTime(&mm, sizeof(MMTIME));
			dwEnd = mm.u.ms;
			wsprintf(szCheck, TEXT("反応時間[%dミリ秒]"), dwEnd - dwStart);
			TypeStart(hWnd);
		} else {
			bSeikai = FALSE;
			MessageBeep(MB_OK);
			lstrcpy(szCheck, TEXT("タイプミス！"));
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
		PlaySound(TEXT("cat.wav"), NULL, SND_ASYNC | SND_FILENAME);	// libwinmm.aとのリンクが必要
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

	case WM_LBUTTONDBLCLK:		// ダブルクリックされたら WM_CLOSEメッセージを出す
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
	wsprintf(szMondai, TEXT("問題=\"%c\""), iMon);
	
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


