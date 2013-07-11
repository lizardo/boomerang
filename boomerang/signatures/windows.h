/* NOTE: All functions in this file are assumed to be PASCAL calling convention
  (hard coded in FrontEnd::readLibraryCatalog()) */

typedef unsigned int UINT;
typedef int INT;
typedef unsigned int SIZE_T;
typedef unsigned int LONG;
typedef const void *LPCVOID;
typedef void *LPVOID;
typedef void *PVOID;
typedef const char LPCSTR[];
typedef char LPSTR[];
typedef const short LPCWSTR[];
typedef short LPWSTR[];
typedef int DWORD;
typedef int HWND;
typedef int HMENU;
typedef int HFONT;
typedef int HLOCAL;
typedef int HINSTANCE;
typedef int HICON;
typedef int HCURSOR;
typedef int HBRUSH;
typedef int HACCEL;
typedef int HDC;
typedef int HGDIOBJ;
typedef int WPARAM;
typedef int LPARAM;
typedef int LRESULT;
typedef int ATOM;
typedef int BOOL;
typedef unsigned char BYTE;
typedef char CHAR;
typedef LRESULT WndProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);
typedef WndProc *WNDPROC;

typedef int WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
);

HLOCAL LocalFree(HLOCAL hMem);
DWORD FormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list* Arguments);
int _write(int fd, char buf[], int size);
LPSTR CharNextA(
    LPCSTR lpsz
);
LPWSTR CharNextW(
    LPCWSTR lpsz
);
LPSTR GetCommandLineA(void);
LPWSTR GetCommandLineW(void);

typedef struct {
    UINT cbSize;
    UINT style;
    WNDPROC lpfnWndProc;
    int cbClsExtra;
    int cbWndExtra;
    HINSTANCE hInstance;
    HICON hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    LPCSTR lpszMenuName;
    LPCSTR lpszClassName;
    HICON hIconSm;
} WNDCLASSEX;
typedef struct {
    UINT cbSize;
    UINT style;
    WNDPROC lpfnWndProc;
    int cbClsExtra;
    int cbWndExtra;
    HINSTANCE hInstance;
    HICON hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    LPCWSTR lpszMenuName;
    LPCWSTR lpszClassName;
    HICON hIconSm;
} WNDCLASSEXw;


ATOM RegisterClassExW(WNDCLASSEXw *lpwcx);
ATOM RegisterClassExA(WNDCLASSEX *lpwcx);

int LoadStringA(
    HINSTANCE hInstance,
    UINT uID,
    LPSTR lpBuffer,
    int nBufferMax
);

int LoadStringW(
    HINSTANCE hInstance,
    UINT uID,
    LPWSTR lpBuffer,
    int nBufferMax
);

HACCEL LoadAcceleratorsA(
    HINSTANCE hInstance,
    LPCSTR lpTableName
);

HACCEL LoadAcceleratorsW(
    HINSTANCE hInstance,
    LPCWSTR lpTableName
);

typedef struct {
    int x;
    int y;
} POINT;
typedef POINT *LPPOINT;

typedef struct {
    int cx;
    int cy;
} SIZE;

typedef struct {
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
} MSG;
typedef MSG *LPMSG;

BOOL GetMessageA(
    LPMSG lpMsg,
    HWND hWnd,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax
);
BOOL GetMessageW(
    LPMSG lpMsg,
    HWND hWnd,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax
);

int TranslateAcceleratorA(
    HWND hWnd,
    HACCEL hAccTable,
    LPMSG lpMsg
);

int TranslateAcceleratorW(
    HWND hWnd,
    HACCEL hAccTable,
    LPMSG lpMsg
);

BOOL TranslateMessage(
    const MSG *lpMsg
);

LRESULT DispatchMessageA(
    const MSG *lpmsg
);

LRESULT DispatchMessageW(
    const MSG *lpmsg
);

HICON LoadIconW(
    HINSTANCE hInstance,
    LPCWSTR lpIconName
);

HCURSOR LoadCursorW(
    HINSTANCE hInstance,
    LPCWSTR lpCursorName
);

HICON LoadIconA(
    HINSTANCE hInstance,
    LPCSTR lpIconName
);

HCURSOR LoadCursorA(
    HINSTANCE hInstance,
    LPCSTR lpCursorName
);

HWND CreateWindowExA(
    DWORD dwExStyle,
    LPCSTR lpClassName,
    LPCSTR lpWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam
);

HWND CreateWindowExW(
    DWORD dwExStyle,
    LPCWSTR lpClassName,
    LPCWSTR lpWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam
);

BOOL ShowWindow(
    HWND hWnd,
    int nCmdShow
);

BOOL UpdateWindow(
    HWND hWnd
);

int MessageBoxA(
    HWND hWnd,
    LPCSTR lpText,
    LPCSTR lpCaption,
    UINT uType
);

int MessageBoxW(
    HWND hWnd,
    LPCWSTR lpText,
    LPCWSTR lpCaption,
    UINT uType
);

BOOL GetProcessDefaultLayout(DWORD *pdwDefaultLayout);
BOOL SetProcessDefaultLayout(DWORD dwDefaultLayout);

HLOCAL LocalAlloc(
    UINT uFlags,
    SIZE_T uBytes
);

BOOL IsDialogMessageA(HWND hDlg, LPMSG lpMsg);
BOOL IsDialogMessageW(HWND hDlg, LPMSG lpMsg);

BOOL IsChild(HWND hWndParent, HWND hWnd);

HBRUSH GetSysColorBrush(int nIndex);

BOOL SystemParametersInfoA(
    UINT uiAction,
    UINT uiParam,
    PVOID pvParam,
    UINT fWinIni
);

BOOL SystemParametersInfoW(
    UINT uiAction,
    UINT uiParam,
    PVOID pvParam,
    UINT fWinIni
);

BOOL SetMenu(HWND hWnd, HMENU hMenu);
BOOL DestroyWindow(HWND hWnd);
BOOL DestroyMenu(HMENU hMenu);

typedef int INT_PTR;
typedef INT_PTR DlgProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);
typedef DlgProc *DLGPROC;

HWND CreateDialogParamA(
    HINSTANCE hInstance,
    LPCSTR lpTemplateName,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam
);

HWND CreateDialogParamW(
    HINSTANCE hInstance,
    LPCWSTR lpTemplateName,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam
);

LRESULT DefWindowProcA(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
);

void PostQuitMessage(
    int nExitCode
);

typedef struct {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT;
typedef RECT *LPRECT;

typedef struct {
    HDC  hdc;
    BOOL fErase;
    RECT rcPaint;
    BOOL fRestore;
    BOOL fIncUpdate;
    BYTE rgbReserved[32];
} PAINTSTRUCT;
typedef PAINTSTRUCT *LPPAINTSTRUCT;

HDC BeginPaint(
    HWND hwnd,
    LPPAINTSTRUCT lpPaint
);

BOOL EndPaint(
    HWND hWnd,
    LPPAINTSTRUCT lpPaint
);

BOOL GetClientRect(
    HWND hWnd,
    LPRECT lpRect
);

int DrawTextA(
    HDC hDC,
    LPCSTR lpString,
    int nCount,
    LPRECT lpRect,
    UINT uFormat
);

int DrawTextW(
    HDC hDC,
    LPCWSTR lpString,
    int nCount,
    LPRECT lpRect,
    UINT uFormat
);

BOOL GetTextExtentPointA(
    HDC     hDC,
    LPCSTR  lpStr,
    int     len,
    SIZE*   sz
);

HGDIOBJ SelectObject(
    HDC hDC,
    HGDIOBJ hObj
);

INT_PTR DialogBoxParamA(
    HINSTANCE hInstance,
    LPCSTR lpTemplateName,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam
);

INT_PTR DialogBoxParamW(
    HINSTANCE hInstance,
    LPCWSTR lpTemplateName,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam
);

BOOL EndDialog(
    HWND hDlg,
    INT_PTR nResult
);

int wsprintfA(
    LPTSTR lpOut,
    LPCTSTR lpFmt,
    ...
);

char *_gcvt(double value, int digits, char *buffer);  /* Convert flt to str */
int MulDiv(int number, int numerator, int denominator);
HFONT CreateFontIndirectA(void* lf);
HFONT CreateFontA(
    int nHeight,               // height of font
    int nWidth,                // average character width
    int nEscapement,           // angle of escapement
    int nOrientation,          // base-line orientation angle
    int fnWeight,              // font weight
    DWORD fdwItalic,           // italic attribute option
    DWORD fdwUnderline,        // underline attribute option
    DWORD fdwStrikeOut,        // strikeout attribute option
    DWORD fdwCharSet,          // character set identifier
    DWORD fdwOutputPrecision,  // output precision
    DWORD fdwClipPrecision,    // clipping precision
    DWORD fdwQuality,          // output quality
    DWORD fdwPitchAndFamily,   // pitch and family
    LPCTSTR lpszFace           // typeface name
);

typedef DWORD LCID;
LCID GetThreadLocale();
HLOCAL LocalReAlloc(
    HLOCAL hMem,
    SIZE_T uBytes,
    UINT uFlags
);
UINT GetProfileIntA(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    INT nDefault
);
UINT GetProfileIntW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    INT nDefault
);
DWORD GetProfileStringA(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpDefault,
    LPSTR lpReturnedString,
    DWORD nSize
);
DWORD GetProfileStringW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR lpReturnedString,
    DWORD nSize
);
DWORD GetSysColor(
    int nIndex
);
BOOL GetWindowRect(
    HWND hWnd,
    LPRECT lpRect
);
BOOL InvalidateRect(
    HWND hWnd,
    LPRECT lpRect,
    BOOL bErase
);
HWND GetDlgItem(
    HWND hDlg,
    int nIDDlgItem
);
BOOL EnableWindow(
    HWND hWnd,
    BOOL bEnable
);
int MapWindowPoints(
    HWND hWndFrom,
    HWND hWndTo,
    LPPOINT lpPoints,
    UINT cPoints
);
BOOL OffsetRect(
    LPRECT lprc,
    int dx,
    int dy
);
BOOL SetWindowPos(
    HWND hWnd,
    HWND hWndInsertAfter,
    int X,
    int Y,
    int cx,
    int cy,
    UINT uFlags
);
HMENU GetMenu(
    HWND hWnd
);
HMENU GetSubMenu(
    HMENU hMenu,
    int nPos
);
DWORD CheckMenuItem(
    HMENU hmenu,
    UINT uIDCheckItem,
    UINT uCheck
);
BOOL SetDlgItemTextA(
    HWND hDlg,
    int nIDDlgItem,
    LPCSTR lpString
);
BOOL SetDlgItemTextW(
    HWND hDlg,
    int nIDDlgItem,
    LPCWSTR lpString
);
BOOL CheckRadioButton(
    HWND hDlg,
    int nIDFirstButton,
    int nIDLastButton,
    int nIDCheckButton
);
HCURSOR SetCursor(
    HCURSOR hCursor
);
BOOL TextOutA(
    HDC hDC,
    int x, int y,
    char* sz, int len);

int FillRect(
    HDC hDC,          // handle to DC
    RECT *lprc,       // rectangle
    HBRUSH hbr        // handle to brush
);
