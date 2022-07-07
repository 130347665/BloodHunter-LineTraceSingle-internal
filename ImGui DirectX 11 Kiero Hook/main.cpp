#include "includes.h"
#include "Data.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;

bool P_Menu;//菜单开关
bool P_ESP;//透视开关
bool P_MagicBullet;//追踪开关

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool init = false;
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	
	if (!init)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)& pDevice)))
		{
			
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			init = true;
		}

		else
		{ 
			printf("我进来HOOK啦EAC HELLO \n");
			return oPresent(pSwapChain, SyncInterval, Flags);
		}
	}
	ImGuiIO& io = ImGui::GetIO();
	Data::ScreenWidth = io.DisplaySize.x / 2;
	Data::ScreenHeight = io.DisplaySize.y / 2;
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	if (P_ESP)
	{
		Data::Draw();
	}
	if (P_MagicBullet)
	{
		Data::Draw();
	}

	ImGui::SetNextWindowSize(ImVec2(400,300));
	if (GetAsyncKeyState(VK_HOME) & 1)P_Menu = !P_Menu;
	ImGui::GetIO().MouseDrawCursor = P_Menu; // 让鼠标穿透
	if(P_Menu){
		ImGui::Begin("ImGui Window",0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |ImGuiWindowFlags_NoScrollbar);
		ImGui::Checkbox("ESP", &P_ESP); ImGui::SameLine();
		ImGui::Checkbox("MagicBullet", &P_MagicBullet);
		ImGui::End();
	}

	ImGui::Render();

	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI MainThread()
{
	printf("%s", "EAC Bypass Fuck you");
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)& oPresent, hkPresent);
			init_hook = true;
		}
	} while (!init_hook);
	return TRUE;
}
//If Any Problem join my Discord discuss Relaxing#4205
//Created 2022/05/22
//No Rat
//Using FaceInjectorV2 
BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		AllocConsole();
		SetConsoleTitle("Debug");
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
		DisableThreadLibraryCalls(hMod);
		MainThread();
		//CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);  //EAC禁止创建线程
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}