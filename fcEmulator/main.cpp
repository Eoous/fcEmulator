#include <windows.h>
#include <D2D1.h>

#define SAFE_RELEASE(P) if(P){P->Release() ; P = NULL ;}

ID2D1Factory* pD2DFactory = NULL; // Direct2D factory
ID2D1HwndRenderTarget* pRenderTarget = NULL;   // Render target
ID2D1SolidColorBrush* pBlackBrush = NULL; // A black brush, reflect the line color

RECT rc; // Render area
HWND g_Hwnd; // Window handle

void createD2DResource(HWND hWnd) {
	if (!pRenderTarget) {
		HRESULT hr;
		//创建工厂对象
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);

		if (FAILED(hr)) {
			MessageBox(hWnd, "Create D2D factory failed!", "Error", 0);
			return;
		}

		GetClientRect(hWnd, &rc);
		//创建Render target
		hr = pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),		//第一个参数 属性  ，这个函数是默认属性
			D2D1::HwndRenderTargetProperties(	//第二个参数是Hwnd类型 有三个参数，第一个是窗口句柄；第二个是Render target大小；第三个是Present选项，这里使用默认值
				hWnd,
				D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
			&pRenderTarget);
		if (FAILED(hr))
		{
			MessageBox(hWnd, "Create render target failed!", "Error", 0);
			return;
		}
		//创建Brush
		hr = pRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Red),
			&pBlackBrush);
		if (FAILED(hr)) {
			MessageBox(hWnd, "Create brush failed!", "Error", 0);
			return;
		}
	}
}

//绘制矩形
void drawRectangle() {
	createD2DResource(g_Hwnd);

	pRenderTarget->BeginDraw();

	//clear bg color white
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	//draw rc
	pRenderTarget->DrawRectangle(D2D1::RectF(100.f, 100.f, 500.f, 500.f),
		pBlackBrush);
	HRESULT hr = pRenderTarget->EndDraw();

}

void cleanUp() {
	SAFE_RELEASE(pRenderTarget);
	SAFE_RELEASE(pBlackBrush);
	SAFE_RELEASE(pD2DFactory);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpcmdLine, int nCmdShow)
{
	WNDCLASSEX winClass;
	winClass.lpszClassName = "Direct2D";
	winClass.cbSize = sizeof(WNDCLASSEX);
	winClass.style = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc = WndProc;
	winClass.hInstance = hInstance;
	winClass.hIcon = NULL;
	winClass.hIconSm = NULL;
	winClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = NULL;
	winClass.lpszMenuName = NULL;
	winClass.cbClsExtra = 0;
	winClass.cbWndExtra = 0;



	return 0;
}