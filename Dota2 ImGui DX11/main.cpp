#include "includes.h"
#include "CoreHack.h"
#include "imgui/droidSans.h"
#include "imgui/vbeFont.h"


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;
FILE* f;
HMODULE hModule;
ImFont* mainFont;
ImFont* vbeFont;

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	vbeFont = io.Fonts->AddFontFromMemoryCompressedTTF(vbeFont_compressed_data, vbeFont_compressed_size, 50);
	mainFont = io.Fonts->AddFontFromMemoryCompressedTTF(droidSans_compressed_data, droidSans_compressed_size, 15);
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

// Features Vars
bool bVBE = false, bVBEParticle = false, bDrawRange = false, bParticleHack = false, bNoFog = false;
const char* weatherList[] = { "Default", "Winter", " Rain", "MoonBeam", "Pestilence", "Harvest", "Sirocco", "Spring", "Ash", "Aurora" };
int camDistance = 1200, rangeVal = 1200;
int prevVbe;
int item_current = 0;

// Imgui Vars
bool init = false, Exit = false, bShowMenu = true;

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{

	if (Exit == true)
	{
		pDevice->Release();
		pContext->Release();
		pSwapChain->Release();
		oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)(oWndProc));
		oPresent(pSwapChain, SyncInterval, Flags);
		return 0;
	}

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
			IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");
			init = true;
		}

		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}

	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		bShowMenu = !bShowMenu;
	}

	//temporary vars
	int tempBVBE = -1, tempBDrawRange = -1, tempBParticleHack = -1, tempBNoFog = -1;
	int tempcamDistance = -1;
	int weather_item = -1;
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//Menu logic
	if (bShowMenu)
	{
		ImGui::PushFont(mainFont);
		ImGui::Begin("Simple Menu");
		ImGui::Text("Visuals");
		//if (ImGui::TreeNode("Visible by enemy")){
		ImGui::Checkbox("Overlay Text.", &bVBE);
		//ImGui::Checkbox("Particle.(Soon)", &bVBEParticle);
		//ImGui::TreePop();
		//}
		ImGui::Checkbox("Draw Blink Dagger Circle Range.", &bDrawRange);
		ImGui::Text("CameraDistance");
		ImGui::SliderInt("", &camDistance, 0, 3000, "%d");
		ImGui::SameLine();
		if (ImGui::Button("Reset", ImVec2(70, 20))) {
			camDistance = 1200;
		}

		ImGui::Dummy(ImVec2(1, 10));
		ImGui::Text("Weather");
		ImGui::ListBox("", &item_current, weatherList, IM_ARRAYSIZE(weatherList), 4);

		ImGui::Dummy(ImVec2(1,20));
		ImGui::Text("Hacks");
		ImGui::Checkbox("No Map Fog.", &bNoFog);
		ImGui::Checkbox("Particle Map Hack.", &bParticleHack);
		ImGui::End();
		ImGui::PopFont();
	}

	if (bVBE)
	{
		ImGui::PushFont(vbeFont);
		//ImGui::SetNextWindowSize(ImVec2(vbeFont->FontSize, vbeFont->FontSize ));
		if (!bShowMenu)
		{
			ImGui::Begin("VBE", NULL, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
		}
		else
		{
			ImGui::Begin("VBE", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
		}
		int VBE = getVBE();
		if (VBE == 0 && prevVbe == 0) // Visible by enemy
		{
			ImGui::TextColored(ImVec4(255, 0, 0, 255), "Visible");
		}
		else if (VBE == -1)
		{
			bVBE = false;
		}
		prevVbe = VBE;
		ImGui::End();
		ImGui::PopFont();
	}
	if (weather_item != item_current)
	{
		SetWeather(item_current);
	}
	if (tempBDrawRange != bDrawRange)
	{
		if (bDrawRange)
			rangeVal = 1200;
		else
			rangeVal = 0;
		SetDrawRange(rangeVal);
	}
	if (tempBParticleHack != bParticleHack)
	{
		SetParticleHack(!bParticleHack);
	}
	if (tempBNoFog != bNoFog)
	{
		SetNoFog(!bNoFog);
	}
	if (tempcamDistance != camDistance)
	{
		SetCamDistance(camDistance);

	}

	tempBVBE = bVBE, tempBDrawRange = bDrawRange, tempBParticleHack = bParticleHack, tempBNoFog = bNoFog;
	tempcamDistance = camDistance;
	weather_item = item_current;

	ImGui::Render();
	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oPresent(pSwapChain, SyncInterval, Flags);
}


DWORD WINAPI MainThread(HMODULE hModule)
{
	//AllocConsole();
	//FILE* f;
	//freopen_s(&f, "CONOUT$", "w", stdout);

	InitHack();
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)& oPresent, hkPresent);
			init_hook = true;
		}
	} while (!init_hook);
	MessageBeep(MB_OK);
	while (Exit == false)
	{
		if (GetAsyncKeyState(VK_END) & 1)
		{
			Exit = true;
			Sleep(1000);
			ResetConvars();
			RemoveVmtHooks();
			MessageBeep(MB_OK);
			kiero::shutdown();
			//fclose(f);
			//FreeConsole();
			FreeLibraryAndExitThread(hModule, 0);
		}
	}
	return 1;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr));
	}
	return TRUE;
}
