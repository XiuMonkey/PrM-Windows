#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <ocidl.h>
#include <olectl.h>
#include <assert.h>
#include <direct.h>
#include <string>
#include <shlwapi.h>
#include <iostream>
#include <gdiplus.h>
#include "LauncherCore.h"
#include "resource.h"
#include "wincodec.h"
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#pragma  comment(lib,"Windowscodecs.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"gdiplus.lib")
#define SAFE_RELEASE(p) if(p){p->Release() ; p = NULL ;} 
#define LAUNCH 150
#define SHIFT 151
int ad2 = 0;
int ad = 0;
int mbnum2 = 0;
HDC hdc;
HDC mdc;
HDC hdcbutton;
HDC hdcbutton2;
HDC JP_edit_HDC;
HWND hWnd;//�����ھ��
HWND MBHWND;//��ʼ��ť���
HWND MBHWND2;//���ذ�ť���
HWND javapathedit;//java·���༭��
HFONT hFont;//������
WNDCLASS wc = { 0 };//��������
char javapath[1000];
HINSTANCE hInst;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
ID2D1Factory* pD2DFactory = NULL;
ID2D1Factory* pD2DFactory2 = NULL;
ID2D1Factory* pD2DFactory3 = NULL;
ID2D1HwndRenderTarget* pRenderTarget = NULL;//�����ڴ����н�����Ⱦ 
ID2D1SolidColorBrush* pBlackBrush = NULL;//���廭ˢ����������ͼ��
ID2D1RadialGradientBrush* pRadialGradientBrush = NULL;
ID2D1Bitmap* m_pD2d1Bitmap;
IWICBitmap* m_pWicBitmap;
IWICImagingFactory* m_pWicImagingFactory;
IWICBitmapDecoder* m_pWicDecoder;
IWICBitmapFrameDecode* m_pWicFrameDecoder;


VOID MyDraw(HWND hwnd,LPCWSTR c,ID2D1Factory* fac,LONG facx,LONG facy)
{
	RECT rc{ 0,0,facx,facy };
	pD2DFactory3->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			hwnd,
			D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
		&pRenderTarget);
	CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWicImagingFactory));
	if (m_pWicImagingFactory != nullptr)
	{
		m_pWicImagingFactory->CreateDecoderFromFilename(c, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &m_pWicDecoder);
		m_pWicDecoder->GetFrame(0, &m_pWicFrameDecoder);
		IWICBitmapSource* pWicSource = nullptr;
		m_pWicFrameDecoder->QueryInterface(IID_PPV_ARGS(&pWicSource));
		IWICFormatConverter* pCovert = nullptr;
		m_pWicImagingFactory->CreateFormatConverter(&pCovert);
		pCovert->Initialize(
			pWicSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeCustom
		);

		m_pWicImagingFactory->CreateBitmapFromSource(pCovert, WICBitmapCacheOnDemand, &m_pWicBitmap);
		SAFE_RELEASE(pCovert);
		UINT pixelWidth = 0, pixelHeight = 0;
		m_pWicBitmap->GetSize(&pixelWidth, &pixelHeight);
	}
	pRenderTarget->CreateBitmapFromWicBitmap(m_pWicBitmap, NULL, &m_pD2d1Bitmap);

	pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &pBlackBrush);


	pRenderTarget->BeginDraw();
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White, 0));

	/*****************����ͼƬ*************************/
	D2D1_SIZE_F rtSize = pRenderTarget->GetSize();
	if (m_pD2d1Bitmap != nullptr)
	{
		D2D1_SIZE_U sizeU = m_pD2d1Bitmap->GetPixelSize();

		D2D1_RECT_F rectangle3 = D2D1::RectF(
			(rtSize.width - sizeU.width) * 0.5f,
			(rtSize.height - sizeU.height) * 0.5f,
			sizeU.width + (rtSize.width - sizeU.width) * 0.5f,
			sizeU.height + (rtSize.height - sizeU.height) * 0.5f
		);

		pRenderTarget->DrawBitmap(m_pD2d1Bitmap, &rectangle3, 1.0f);
	}
	pRenderTarget->EndDraw();
	SAFE_RELEASE(m_pWicBitmap);//���ͼƬ��Դ�ǵ��ͷţ���Ȼ�᲻�������ڴ棬��Ȼ������԰���������󣬾Ͳ�Ҫ�ٴ�������������������ͷš�
	SAFE_RELEASE(pBlackBrush);
	SAFE_RELEASE(pRadialGradientBrush);
	SAFE_RELEASE(m_pD2d1Bitmap);
	SAFE_RELEASE(pRenderTarget);
	SAFE_RELEASE(m_pWicFrameDecoder);
	SAFE_RELEASE(m_pWicDecoder);
	SAFE_RELEASE(m_pWicImagingFactory);
}


using namespace std;
using namespace Gdiplus;
BOOL FreeMyResource(UINT uiResouceName, char* lpszResourceType, char* lpszSaveFileName)
{
	HRSRC hRsrc = ::FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(uiResouceName), lpszResourceType);
	LPTSTR szBuffer = new TCHAR[1024];
	if (hRsrc == NULL)
	{
		return FALSE;
	}
	DWORD dwSize = ::SizeofResource(NULL, hRsrc);
	if (0 >= dwSize)
	{
		return FALSE;
	}
	HGLOBAL hGlobal = ::LoadResource(NULL, hRsrc);
	if (NULL == hGlobal)
	{
		return FALSE;
	}
	LPVOID lpVoid = ::LockResource(hGlobal);
	if (NULL == lpVoid)
	{
		return FALSE;
	}
	FILE* fp = NULL;
	fopen_s(&fp, lpszSaveFileName, "wb+");
	if (NULL == fp)
	{
		return FALSE;
	}
	fwrite(lpVoid, sizeof(char), dwSize, fp);
	fclose(fp);
	return TRUE;
}
#pragma region ԭShowPic
/*
HRESULT ShowPic(char* lpstrFile, HWND hWnd, int nScrWidth, int nScrHeight)
{
	HDC hDC_Temp = GetDC(hWnd);

	IPicture* pPic;
	IStream* pStm;

	BOOL bResult;

	HANDLE hFile = NULL;
	DWORD dwFileSize, dwByteRead;

	//��Ӳ���е�ͼ���ļ�
	hFile = CreateFile(lpstrFile, GENERIC_READ,FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		dwFileSize = GetFileSize(hFile, NULL);//��ȡ�ļ��ֽ���

		if (dwFileSize == 0xFFFFFFFF)
			return E_FAIL;
	}
	else
	{
		return E_FAIL;
	}


	//����ȫ�ִ洢�ռ�
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwFileSize);
	LPVOID pvData = NULL;

	if (hGlobal == NULL)
		return E_FAIL;

	if ((pvData = GlobalLock(hGlobal)) == NULL)//���������ڴ��
		return E_FAIL;


	ReadFile(hFile, pvData, dwFileSize, &dwByteRead, NULL);//���ļ������ڴ滺����

	CreateStreamOnHGlobal(hGlobal, TRUE, &pStm);

	//װ��ͼ���ļ�
	bResult = OleLoadPicture(pStm, dwFileSize, TRUE, IID_IPicture, (LPVOID*)&pPic);


	if (FAILED(bResult))
		return E_FAIL;

	OLE_XSIZE_HIMETRIC hmWidth;//ͼƬ����ʵ���
	OLE_YSIZE_HIMETRIC hmHeight;//ͼƬ����ʵ�߶�
	pPic->get_Width(&hmWidth);
	pPic->get_Height(&hmHeight);


	//��ͼ���������Ļ�ϣ��е���BitBlt��
	bResult = pPic->Render(hDC_Temp, 0, 0, nScrWidth, nScrHeight,
		0, hmHeight, hmWidth, -hmHeight, NULL);

	pPic->Release();
	CloseHandle(hFile);//�رմ򿪵��ļ�
	GlobalUnlock(hGlobal);
	GlobalFree(hGlobal);


	if (SUCCEEDED(bResult))
	{
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}
*/

#pragma endregion
void OnPaint(HDC hdc) {
	FreeMyResource(IDR_JPG1, "PNG", "data\\01\\back.jpg");
	MyDraw(hWnd,L"data\\01\\back.jpg", pD2DFactory,987,629);
}
void OnPaintButton1(HDC hdc) {
	FreeMyResource(IDB_PNG2, "PNG", "data\\01\\launch_main.png");
	MyDraw(MBHWND,  L"data\\01\\launch_main.png", pD2DFactory2,140,90);
}
void OnPaintButton4(HDC hdc) {
	FreeMyResource(IDR_JPG2, "JPG", "data\\01\\Download.jpg");
	MyDraw(MBHWND2,  L"data\\01\\Download.jpg", pD2DFactory3,106,146);
}
void OnPaintButton3(HDC hdc) {
	FreeMyResource(IDR_JPG3, "JPG", "data\\01\\Download2.jpg");
	MyDraw(MBHWND2, L"data\\01\\Download2.jpg", pD2DFactory3,106,146);
}
void OnPaintButton2(HDC hdc) {
	FreeMyResource(IDB_PNG1, "PNG", "data\\01\\launch_main2.png");
	MyDraw(MBHWND,  L"data\\01\\launch_main2.png", pD2DFactory2,140,90);
}
void OnPaintButton5(HDC hdc) {
	MyDraw(javapathedit, L"data\\01\\edit-javapath.jpg", pD2DFactory2, 237, 25);
}

LRESULT CALLBACK ChildWndProc2(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	POINT pt2;
	POINT pt;
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE;
	tme.dwHoverTime = HOVER_DEFAULT;
	tme.hwndTrack = hwnd;
	PAINTSTRUCT ps;
	LONG x;
	switch (Message)
	{
	case WM_MOUSEMOVE:
		mbnum2++;
		GetCursorPos(&pt2);
		TrackMouseEvent(&tme);
	case WM_MOUSELEAVE:
		GetCursorPos(&pt);
		if (!((pt.x==pt2.x)&&(pt.y==pt2.y))) {
			mbnum2 = 0;
		}
	case WM_PAINT:
		if (mbnum2==0){
			hdcbutton2 = BeginPaint(MBHWND2, &ps);
			OnPaintButton4(hdcbutton2);
			EndPaint(MBHWND2, &ps);
		}
		else{
			hdcbutton2 = BeginPaint(MBHWND2, &ps);
			OnPaintButton3(hdcbutton2);
			EndPaint(MBHWND2, &ps);
		}
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);   //��ϵͳ������Ϣ���������һ��Ҫ����
	}
	return 0;
}
LRESULT CALLBACK ChildWndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	RECT rt;
	POINT pt;
	POINT pt2;
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE;
	tme.dwHoverTime = HOVER_DEFAULT;
	tme.hwndTrack = hwnd;
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.bottom = 90;
	rect.right = 160;
	int aaa;
	switch (Message)
	{
	case WM_MOUSEMOVE:
		ad++;
		GetCursorPos(&pt);
		TrackMouseEvent(&tme);
	case WM_MOUSELEAVE:
		GetCursorPos(&pt2);
		if (!((pt.x == pt2.x)&&(pt.y == pt2.y))){	
			ad = 0;
		}
	case WM_PAINT:
		if (ad == 0) {
			hdcbutton = BeginPaint(MBHWND, &ps);
			OnPaintButton1(hdcbutton);
			EndPaint(MBHWND, &ps);
		}
		else {
			hdcbutton = BeginPaint(MBHWND, &ps);
			OnPaintButton2(hdcbutton);
			EndPaint(MBHWND, &ps);
		}
		break;
	case WM_LBUTTONDOWN://�����
		GetWindowText(javapathedit, javapath, 1000);//��ȡ�༭������
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);   //��ϵͳ������Ϣ���������һ��Ҫ����
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam);

int CALLBACK WinMain(HINSTANCE hIns, HINSTANCE hPreIns, LPSTR lpCmdLine, int nCmdShow) {
	ULONG_PTR token;
	GdiplusStartupInput gsi;
	GdiplusStartup(&token, &gsi, 0);//����GDIPLUS
#pragma region ����������
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor = NULL;
	wc.hIcon = LoadIcon(hIns, MAKEINTRESOURCE(IDI_ICON1));
	wc.hInstance = hIns;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = "main";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&wc);//д������
	wc.cbClsExtra = sizeof(long);
	wc.hIcon = NULL;
	wc.lpfnWndProc = ChildWndProc;
	wc.lpszClassName = TEXT("ChildWindow");
	RegisterClass(&wc);
	wc.cbClsExtra = sizeof(long);
	wc.hIcon = NULL;
	wc.lpfnWndProc = ChildWndProc2;
	wc.lpszClassName = TEXT("ChildWindow2");
	RegisterClass(&wc);
	getopath(opath);
	_mkdir("data");
	_mkdir("data\\01");
	_mkdir("data\\02");
	FreeMyResource(IDR_JPG1, "JPG", "data\\01\\back.jpg");
	FreeMyResource(IDR_JPG4, "JPG", "data\\01\\edit-javapath.jpg");
	FreeMyResource(IDR_JAR1, "JAR", "data\\02\\log4j-patch-agent-1.0.jar");
	hWnd = CreateWindow("main", "PrM Alpha1.0", WS_CLIPCHILDREN | WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX| WS_BORDER, 100, 100, 750,500, NULL, NULL, hIns, NULL);//����������
#pragma endregion
	WNDCLASS Bt = { 0 };
	UpdateWindow(hWnd);
	MSG nMsg = { 0 };
	hInst = hIns;
	while (GetMessage(&nMsg, NULL, 0, 0)) { //ץ��Ϣ
		TranslateMessage(&nMsg);//������Ϣ
		DispatchMessage(&nMsg);//�ɷ���Ϣ������Ϣ�������ڴ�����������
	}
	GdiplusShutdown(token);
	return 0;                                                                                                            
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam)
{
	static int cxClient, cyClient, x = 0, cx, cy;
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	switch (msgID)
	{
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_CREATE:
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory2);
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory3);
		ShowWindow(hWnd, SW_SHOW);
		MBHWND = CreateWindowEx(0, TEXT("ChildWindow"), NULL, WS_CHILD | WS_VISIBLE,30 , 350 ,140, 90,hWnd, (HMENU)(x++), hInst, NULL);
		MBHWND2 = CreateWindowEx(0, TEXT("ChildWindow2"), NULL, WS_CHILD | WS_VISIBLE,171 , 163 ,106, 146,hWnd, (HMENU)(x++), hInst, NULL);
#pragma region �����༭����λͼ
		javapathedit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL| ES_NOHIDESEL, 450, 10, 237, 25, hWnd, NULL, NULL, NULL);
		hFont = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_SWISS, "����");
		SendMessage(javapathedit, WM_SETFONT, (WPARAM)hFont, TRUE);
#pragma endregion
		break;
		//���ٴ���
	case WM_DESTROY:
		remove("data\\01\\back.jpg");
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		OnPaint(hdc);
		hdc = BeginPaint(javapathedit, &ps);
		OnPaintButton5(JP_edit_HDC);
		EndPaint(hWnd, &ps);
	
	default:
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		DefWindowProc(hWnd, msgID, wParam, lParam);
	}
	return 1;
}



