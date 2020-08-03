#include "includes.h"
#include "MemIn.h"
#include "DOTA_Convar.h"



extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
unsigned int ReadVBE(uintptr_t dynamicAddr, std::vector<unsigned int> offsets);
std::vector<unsigned int> getOffsetFromText();

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;
FILE* f;
HMODULE hModule;
using std::cout;
using std::endl;

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

//Read offsets from config file.



MemIn meme;
std::vector<unsigned int> offsets;
uintptr_t engine2BaseAddr;
uintptr_t clientBaseAddr;
UnrestrictedCMD cmd = UnrestrictedCMD();
bool mainhackInit() 
{
	offsets = getOffsetFromText();
	engine2BaseAddr = meme.GetModuleBase("engine2.dll");
	clientBaseAddr = meme.GetModuleBase("client.dll");
	cmd.Init();
	return true;
}

bool init = false;
bool Exit = false;
bool bShowMenu = true;
bool bVBE = false,bDrawRange = false,bParticleHack = false,bNoFog = false;
int camDistance = 1300;
static int item_current = 0;

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

		ImGui::Begin("Skrixx Dota2 Menu");
		ImGui::SetWindowFontScale(1.20f);

		ImGui::Text("Visuals");
		//VBE
		ImGui::Checkbox("Overlay Visible by enemy.", &bVBE);
		//Circle Range
		ImGui::Checkbox("Draw Dagger Circle Range.", &bDrawRange);
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
	}


	if (weather_item != item_current)
	{
		std::string tempCmnd = (cmd.stringBuild(listCommands::d_cl_weather, item_current));
		const char* chrCommand = const_cast<char*>(tempCmnd.c_str());
		cmd.ExecuteCmd(chrCommand);
	}
	if (tempBDrawRange != bDrawRange)
	{
		std::string tempCmnd = (cmd.stringBuild(listCommands::d_range_display, 1200));
		const char* chrCommand = const_cast<char*>(tempCmnd.c_str());
		cmd.ExecuteCmd(chrCommand);
	}
	if (tempBParticleHack != bParticleHack)
	{
		std::string tempCmnd = (cmd.stringBuild(listCommands::b_ParticleHasLimit, !bParticleHack));
		const char* chrCommand = const_cast<char*>(tempCmnd.c_str());
		cmd.ExecuteCmd(chrCommand);
	}
	if (tempBNoFog != bNoFog)
	{
		std::string tempCmnd = (cmd.stringBuild(listCommands::b_Fog, !bNoFog));
		const char* chrCommand = const_cast<char*>(tempCmnd.c_str());
		cmd.ExecuteCmd(chrCommand);
	}
	if (tempBcamDistance != camDistance)
	{
		std::string tempCmnd = (cmd.stringBuild(listCommands::d_CameraDistance, camDistance));
		const char* chrCommand = const_cast<char*>(tempCmnd.c_str());
		cmd.ExecuteCmd(chrCommand);
	}

	ImGui::Render();
	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI MainThread(HMODULE hModule)
{
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)& oPresent, hkPresent);
			init_hook = true;
		}
	} while (!init_hook);

	while (!GetAsyncKeyState(VK_DELETE))
	{
		Sleep(1);
	}
	Exit = true;
	kiero::shutdown();

	FreeLibraryAndExitThread(hModule, 0);
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr));

	}
	return TRUE;
}
//Read Address of Visible by enemy tag in memory
unsigned int ReadVBE(uintptr_t dynamicAddr, std::vector<unsigned int> offsets)
{
	uintptr_t vbEAddr = meme.ReadMultiLevelPointer(dynamicAddr, offsets, true);
	if (vbEAddr == 0)
	{
		return 3;
	}
	int* vbEVal = (int*)vbEAddr;
	if (*vbEVal == 14) // Visible by enemy
	{
		return 1;
	}
	else if (*vbEVal >= 6 && *vbEVal <= 10) // Not visible by enemy
	{
		return 0;
	}
	return 3;
}
std::vector<unsigned int> getOffsetFromText() {

	std::fstream file;
	std::string word;
	std::vector<std::string> offsets;
	std::vector<unsigned int> offsetsInt;

	CHAR czTempPath[MAX_PATH] = { 0 };
	GetTempPathA(MAX_PATH, czTempPath); // retrieving temp path
	std::string sPath = czTempPath;
	sPath += "offs.conf";

	//Open text file
	file.open(sPath, std::ios::out | std::ios::in);
	if (file.fail()) {
		MessageBox(NULL, "Config file not found", "ERROR", NULL);
	}

	int i = 0;
	while (file >> word)
	{
		if (i >= 8) break;

		offsets.push_back(word);
		i++;
	}
	for (size_t i = 0; i < offsets.size(); i++)
	{
		std::istringstream buffer(offsets[i]);
		unsigned long long value;
		buffer >> std::hex >> value;
		offsetsInt.push_back(value);
	}
	offsets.clear();
	file.close();
	return offsetsInt;
}