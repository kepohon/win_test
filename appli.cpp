//
// winmain.cpp (Shift-JIS)
//	ウィンドウズプログラミングの基本雛形
//
//	2023/07/21 by kepohon	// 二重起動チェック
//	2023/07/20 by kepohon

#define WIN32_LEAN_AND_MEAN	// 不要なヘッダファイルのインクルードを抑止してコンパイル時間を短縮できる

#include	<windows.h>
//#include <Mmsystem.h>
#include	"appli.h"

// プロトタイプ宣言
bool	CreateMainWindow(HINSTANCE, int); 
LRESULT WINAPI	WindowProc( HWND, UINT, WPARAM, LPARAM );
bool	AnotherInstance();		// 二重起動チェック

/*
int WINAPI		WinMain(HINSTANCE, HINSTANCE, LPSTR, int); 
bool			CreateMainWindow(HINSTANCE, int);
LRESULT WINAPI	WindowProc(HWND, UINT, WPARAM, LPARAM); 
*/


// ウィンドウズプログラムのエントリーポイント
int WINAPI	WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpCmdLine,
                    int       nCmdShow)
{
    MSG	 msg;

	// 二重起動チェック
	if ( AnotherInstance() )
	{
		MessageBox(	NULL,
					TEXT("二重起動はできません"),
					TEXT("警告！"),
					MB_OK
		);
		return false;
	}

    // ウィンドウの作成
    if (!CreateMainWindow(hInstance, nCmdShow)){
        return false;
	}

	try {
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
	catch(...){
	}
	return  msg.wParam;
}

//=============================================================================
// ウィンドウクラスの登録とウィンドウの作成
// 戻り値: エラー発生時false
//=============================================================================
bool	CreateMainWindow(HINSTANCE hInstance, int nCmdShow) 
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
LRESULT WINAPI	WindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	int id;
	
    switch( msg )
    {
    case WM_CLOSE:    // ウィンドウが閉じられるとき
        id = MessageBox(hWnd, TEXT("終了しますか?"),
            TEXT("終了"), MB_YESNO | MB_ICONEXCLAMATION);
        if (id == IDYES){
            DestroyWindow(hWnd);        SetMenu(hWnd, NULL);  // ウィンドウからメニュー削除
		}
        break;                // ウィンドウを閉じる処理はDefWindowProc()に任せる

    case WM_DESTROY:
        PostQuitMessage(0);			// メッセージループでWM_QUITメッセージを発生させる
        return 0;
		
	default:
		return DefWindowProc( hWnd, msg, wParam, lParam );
    }
    return 0;
}

//=============================================================================
// 二重起動のチェック
//=============================================================================
bool AnotherInstance()
{
	HANDLE ourMutex;

	// Attempt to create a mutex using our unique string
	ourMutex = CreateMutex(	NULL,
							true,
							"Use_a_different_string_here_for_each_program_48161-XYZZY"
							);

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return true;            // another instance was found
	}

	return false;               // we are the only instance
}


