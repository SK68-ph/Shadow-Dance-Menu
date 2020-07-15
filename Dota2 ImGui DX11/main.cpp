#include "includes.h"
#include "MemIn.h"
#include "DOTA_Convar.h"



extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
std::vector<unsigned int> getOffsetFromText() {

	std::fstream file;
	std::string word;
	std::vector<std::string> offsets;
	std::vector<unsigned int> offsetsInt;

	//Open text file
	file.open("C:\\Users\\Skrixx\\Desktop\\Dota2Overlay\\offs.conf", std::ios::out | std::ios::in);
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
	return offsetsInt;
}


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
bool bShowMenu = true;
bool bVBE = false,bDrawRange = false,bParticleHack = false,bNoFog = false;
int camDistance = 1300;


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

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//Menu logic
	if (bShowMenu)
	{
		if (GetAsyncKeyState(VK_END) & 1)
		{
			kiero::shutdown();
			exit(0);
			return 0;
		}

		ImGui::Begin("Dota2 Menu");
		ImGui::SetWindowFontScale(1.20f);
		ImGui::Text("Visuals");
		ImGui::Checkbox("Overlay Visible by enemy.", &bVBE);
		ImGui::Checkbox("Draw Dagger Circle Range.", &bDrawRange);
		ImGui::SliderInt("CameraDistance", &camDistance, 0, 3000, "%d");
		ImGui::Dummy(ImVec2(1,30));
		ImGui::Text("Hacks");
		ImGui::Checkbox("Map Fog.", &bNoFog);
		ImGui::Checkbox("Particle Map Hack.", &bParticleHack);
		ImGui::End();

	}

	//Hack logic
	if (bVBE)
	{
		uintptr_t dynamicAddr = engine2BaseAddr + offsets[0];
		uintptr_t vbEAddr = meme.ReadMultiLevelPointer(dynamicAddr, offsets,true);
		int* vbEVal = (int*)vbEAddr;
		if (vbEVal != 0)
		{
			if (*vbEVal == 14) // Visible by enemy
			{
				cout << "visible" << endl;

			}
			else if (*vbEVal >= 6 && *vbEVal <= 10) // Not visible by enemy
			{
				cout << "not visible" << endl;
			}
		}
		else 
		{
			cout << "not found" << endl;
		}
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

DWORD WINAPI MainThread(LPVOID lpReserved)
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
	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	hModule = hMod;
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		CloseHandle(CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr));
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}