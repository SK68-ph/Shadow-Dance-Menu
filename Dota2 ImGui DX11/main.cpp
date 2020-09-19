#include "includes.h"
#include "MainHack.h"
#include "imgui/droidSans.h"
#include "imgui/vbeFont.h"


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//unsigned int ReadVBE(uintptr_t dynamicAddr, std::vector<unsigned int> offsets);
//std::vector<unsigned int> getOffsetFromText();
void MsgBox(const char* str, const char* caption, int type);
bool mainhackInit();

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


bool init = false;
bool Exit = false;
bool bShowMenu = true;
bool bVBE = false, bDrawRange = false, bParticleHack = false, bNoFog = false;
int camDistance = 1300;
int rangeVal = 1200;
static int item_current = 0;
int tempVal = 0;
std::vector<unsigned int> offsets;
MainHack hack;

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{

	if (Exit == true)
	{
		return oPresent(pSwapChain, SyncInterval, Flags);
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
			mainhackInit();
			init = true;
			IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");
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
		ImGui::Begin("Skrixx Dota2 Menu");
		ImGui::Text("Visuals");
		//VBE
		ImGui::Checkbox("Overlay Visible by enemy.", &bVBE);
		//Circle Range
		ImGui::Checkbox("Draw Dagger Circle Range.", &bDrawRange);
		ImGui::SliderInt("CameraDistance", &camDistance, 0, 3000, "%d");
		//Weather
		const char* items[] = { "Default", "Winter", "Rain", "MoonBeam", "Pestilence", "Harvest", "Sirocco", "Spring", "Ash", "Aurora" };

		ImGui::Dummy(ImVec2(1, 30));
		ImGui::Text("Weather");
		ImGui::ListBox("", &item_current, items, IM_ARRAYSIZE(items), 4);

		ImGui::Dummy(ImVec2(1,30));

		ImGui::Text("Hacks");
		ImGui::Checkbox("Map Fog.", &bNoFog);
		ImGui::Checkbox("Particle Map Hack.", &bParticleHack);
		ImGui::End();
		ImGui::PopFont();
	}

	if (bVBE)
	{
		//,NULL, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar  
		ImGui::PushFont(vbeFont);
		ImGui::SetNextWindowSize(ImVec2(320, 100));
		ImGui::Begin("VBE", NULL, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize |ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
		int VBE = hack.getVBE();
		//std::cout << std::dec << VBE << std::endl;
		if (VBE == 14) // Visible by enemy
		{
			ImGui::TextColored(ImVec4(255, 0, 0, 255), "Visible");
		}
		else if (VBE >= 6 && VBE <= 10) // Not visible by enemy
		{
			ImGui::TextColored(ImVec4(124, 252, 0, 255), "Not Visible");
		}
		else
		{
			bVBE = false;
			if (tempVal == 1) {
				MsgBox("Run VBE Ingame", "Warning!", 1);
				tempVal = 0;
			}
			tempVal++;
		}

		ImGui::End();
		ImGui::PopFont();
	}

	if (weather_item != item_current)
	{
		std::string tempCmnd = (hack.stringBuild(listCommands::d_cl_weather, item_current));
		const char* chrCommand = const_cast<char*>(tempCmnd.c_str());
		int bRes = hack.ExecuteCmd(chrCommand);
		if (bRes != 1) MsgBox("Command Error","Error",-1);
	}
	if (tempBDrawRange != bDrawRange)
	{
		bDrawRange ? rangeVal = 1200 : rangeVal = 0;
		std::string tempCmnd = (hack.stringBuild(listCommands::d_range_display, rangeVal));
		const char* chrCommand = const_cast<char*>(tempCmnd.c_str());
		int bRes = hack.ExecuteCmd(chrCommand);
		if (bRes != 1) MsgBox("Command Error", "Error", -1);
	}
	if (tempBParticleHack != bParticleHack)
	{
		std::string tempCmnd = (hack.stringBuild(listCommands::b_ParticleHasLimit, !bParticleHack));
		const char* chrCommand = const_cast<char*>(tempCmnd.c_str());
		int bRes = hack.ExecuteCmd(chrCommand);
		if (bRes != 1) MsgBox("Command Error", "Error", -1);
	}
	if (tempBNoFog != bNoFog)
	{
		std::string tempCmnd = (hack.stringBuild(listCommands::b_Fog, !bNoFog));
		const char* chrCommand = const_cast<char*>(tempCmnd.c_str());
		int bRes = hack.ExecuteCmd(chrCommand);
		if (bRes != 1) MsgBox("Command Error", "Error", -1);
	}
	if (tempBcamDistance != camDistance)
	{
		std::string tempCmnd = (hack.stringBuild(listCommands::d_CameraDistance, camDistance));
		const char* chrCommand = const_cast<char*>(tempCmnd.c_str());
		int bRes = hack.ExecuteCmd(chrCommand);
		if (bRes != 1) MsgBox("Command Error", "Error", -1);
	}

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
	//fclose(f);
	//FreeConsole();
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

bool mainhackInit()
{
	if (hack.Init() != 1)
	{
		MsgBox("Initialization Error", "ERROR", -1);
		Exit = true;
	}
	return true;
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


//std::vector<unsigned int> getOffsetFromText() {
//
//	std::fstream file;
//	std::string word;
//	std::vector<std::string> offsets;
//	std::vector<unsigned int> offsetsInt;
//
//	CHAR czTempPath[MAX_PATH] = { 0 };
//	GetTempPathA(MAX_PATH, czTempPath); // retrieving temp path
//	std::string sPath = czTempPath;
//	sPath += "offs.conf";
//
//	//Open text file
//	file.open(sPath, std::ios::out | std::ios::in);
//	if (file.fail()) {
//		MessageBox(NULL, "Config file not found", "ERROR", NULL);
//	}
//
//	int i = 0;
//	while (file >> word)
//	{
//		if (i >= 8) break;
//
//		offsets.push_back(word);
//		i++;
//	}
//	for (size_t i = 0; i < offsets.size(); i++)
//	{
//		std::istringstream buffer(offsets[i]);
//		unsigned long long value;
//		buffer >> std::hex >> value;
//		offsetsInt.push_back(value);
//	}
//	offsets.clear();
//	file.close();
//	return offsetsInt;
//}