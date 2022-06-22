#include "includes.h"
#include "CoreHack.h"
#include "imgui/droidSans.h"
#include "imgui/vbeFont.h"


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//unsigned int ReadVBE(uintptr_t dynamicAddr, std::vector<unsigned int> offsets);
//std::vector<unsigned int> getOffsetFromText();
void MsgBox(const char* str, const char* caption, int type);


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

bool bVBE = false, bDrawRange = false, bParticleHack = false, bNoFog = false;		//Weather
const char* weatherList[] = { "Default", "Winter", " Rain", "MoonBeam", "Pestilence", "Harvest", "Sirocco", "Spring", "Ash", "Aurora" };
int camDistance = 1200;
int rangeVal = 1200;
static int item_current = 0;
Hack::ConVars convars;
bool init = false;
bool Exit = false;
bool bShowMenu = true;

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{

	if (Exit == true)
	{
		return oPresent(pSwapChain, SyncInterval, Flags);
	}

	if (!init)
	{
		convars.FindConVars();
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
		ImGui::SetNextWindowSize(ImVec2(320, 100));
		ImGui::Begin("VBE", NULL, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize |ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
		int VBE = Hack::getVBE();
		if (VBE == 0) // Visible by enemy
		{
			ImGui::TextColored(ImVec4(255, 0, 0, 255), "Visible");
		}
		else if(VBE == -1)
		{
			bVBE = false;
		}

		ImGui::End();
		ImGui::PopFont();
	}

	if (weather_item != item_current)
	{
		convars.cl_weather->SetValue(item_current);
	}
	if (tempBDrawRange != bDrawRange)
	{
		bDrawRange ? rangeVal = 1200 : rangeVal = 0;
		convars.range_display->SetValue(rangeVal);
	}
	if (tempBParticleHack != bParticleHack)
	{
		convars.particle_hack->SetValue(!bParticleHack);
	}
	if (tempBNoFog != bNoFog)
	{
		convars.fog_enable->SetValue(!bNoFog);
	}
	if (tempBcamDistance != camDistance)
	{
		convars.camera_distance->SetValue(camDistance);
		convars.r_farz->SetValue(camDistance * 2);
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
		}
	} while (!init_hook);

	while (!GetAsyncKeyState(VK_DELETE) && Exit == false)
	{
		Sleep(1);
	}

	Exit = true;
	kiero::shutdown();
	fclose(f);
	FreeConsole();
	//close main thread
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

void MsgBox(const char* str, const char* caption, int type) {
	switch (type)
	{
	case 0:
		MessageBox(NULL, str, caption, MB_OK | MB_ICONINFORMATION);
		break;
	case 1:
		MessageBox(NULL, str, caption, MB_OK | MB_ICONWARNING);
		break;
	case -1:
		MessageBox(NULL, str, caption, MB_ABORTRETRYIGNORE | MB_ICONERROR);
		break;
	default:
		break;
	}
		
}
