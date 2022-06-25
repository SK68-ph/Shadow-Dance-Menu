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
ConVars convar;

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
bool bVBE = false, bDrawRange = false, bParticleHack = false, bNoFog = false, bVbeScan = true;
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
		return oPresent(pSwapChain, SyncInterval, Flags);
	}

	if (!init)
	{
		InitHack();
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)& pDevice)))
		{
			std::cout << "Initialized ImGui" << std::endl;
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
			convar.InitConvars();
		}

		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}

	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		bShowMenu = !bShowMenu;
	}

	//temporary vars
	bool tempBVBE = bVBE, tempBDrawRange = bDrawRange, tempBParticleHack = bParticleHack, tempBNoFog = bNoFog;
	int tempBcamDistance = camDistance;
	int weather_item = item_current;
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//Menu logic
	if (bShowMenu)
	{
		ImGui::PushFont(mainFont);
		ImGui::Begin("Simple Dota2 Menu");

		ImGui::Text("Visuals");
		ImGui::Checkbox("Overlay Visible by enemy.", &bVBE);
		ImGui::Checkbox("Draw Blink Dagger Circle Range.", &bDrawRange);
		ImGui::SliderInt("CameraDistance", &camDistance, 0, 3000, "%d");

		ImGui::Dummy(ImVec2(1, 10));
		ImGui::Text("Weather");
		ImGui::ListBox("", &item_current, weatherList, IM_ARRAYSIZE(weatherList), 4);

		ImGui::Dummy(ImVec2(1,20));
		ImGui::Text("Hacks");
		ImGui::Checkbox("No Map Fog.", &bNoFog);
		ImGui::Checkbox("Particle Map Hack.", &bParticleHack);

		ImGui::Dummy(ImVec2(1, 5));
		if (ImGui::Button("Reset Options", ImVec2(90, 20))) {
			bVBE = false;
			bDrawRange = 0;
			camDistance = 1200;
			item_current = 0;
			bNoFog = false;
			bParticleHack = false;
		}
		ImGui::End();
		ImGui::PopFont();
	}

	if (bVBE)
	{
		ImGui::PushFont(vbeFont);
		ImGui::SetNextWindowSize(ImVec2(vbeFont->FontSize * 6, vbeFont->FontSize * 2));
		ImGui::Begin("VBE", NULL, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize |ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
		int VBE = getVBE();
		Sleep(1);
		if (VBE == 0 && prevVbe == 0) // Visible by enemy
		{
			ImGui::TextColored(ImVec4(255, 0, 0, 255), "Visible");
		}
		else if(VBE == -1)
		{
			bVBE = false;
			std::cout << "VBE failed, disabling" << std::endl;
		}
		prevVbe = VBE;
		ImGui::End();
		ImGui::PopFont();
	}

	if (weather_item != item_current)
	{
		convar.weather->SetValue(item_current);
	}
	if (tempBDrawRange != bDrawRange)
	{
		if (bDrawRange)
			rangeVal = 1200;
		else
			rangeVal = 0;
		convar.sv_cheats->SetValue(1);
		convar.drawrange->SetValue(rangeVal);
	}
	if (tempBParticleHack != bParticleHack)
	{
		convar.particle_hack->SetValue(!bParticleHack);
	}
	if (tempBNoFog != bNoFog)
	{
		convar.fog_enable->SetValue(!bNoFog);
	}
	if (tempBcamDistance != camDistance)
	{
		convar.camera_distance->SetValue(camDistance);
		convar.r_farz->SetValue(camDistance * 2);
	}

	ImGui::Render();
	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oPresent(pSwapChain, SyncInterval, Flags);
}


DWORD WINAPI MainThread(HMODULE hModule)
{
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);

	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)& oPresent, hkPresent);
			init_hook = true;
			std::cout << "Successfully Hooked Render Present" << std::endl;
		}
	} while (!init_hook);

	while (!GetAsyncKeyState(VK_DELETE) && Exit == false)
	{
		Sleep(1);
	}

	Exit = true;
	kiero::shutdown();
	ExitHack();
	fclose(f);
	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
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
