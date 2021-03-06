
// rosINTERNALesp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Windows.h"
#include "intrin.h"
#include "d3d9.h"
#include <iostream>
#include "d3dx9.h"
#include <vector>
#include "Colors.h"
#pragma comment(lib, "D:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x86\\d3d9.lib")
#pragma comment(lib, "D:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x86\\d3dx9.lib")
#include <assert.h>
#include <cstdint>
#include <type_traits>
#include <fstream>      // std::ofstream
#include <thread>
#include "CMenu.h"
#include "CSound.h"
#include "D:\\Downloads\\detour\\Include\\detours.h"
#pragma comment(lib, "D:\\Downloads\\detour\\Library\\detours.lib")

using namespace std;


void drawesp(IDirect3DDevice9* pDevice);

#define OFFSET_CLIENT 0x20E8590
#define OFFSET_RENDERER 0x2083518
#define OFFSET_VIEWMATRIX 0x1EDB470
#define OFFSET_LOCALPLAYER 0x20EA28C
#define OFFSET_PYGAME 0x20DCA48

#define LANG_HUMAN "Human"
#define LANG_ROBOT "Robot"
#define LANG_VEHICLE  "Vehicle"
#define LANG_ITEM  "Item"
#define LANG_SUPPLY  "Supply"
#define LANG_PLANE  "Plane"
#define LANG_WEAPON  "Weapon"

bool Player, Bot, Vehicle, Item, SuplyBox, Plane, weapon;
static bool wallhacks = true;
static int color = 1;
static bool multi = true;
static bool lines = false;

static bool ingame = true;


static bool esp = true;
static bool menu = true;

UINT Stride;
bool InitOnce = true;

D3DVIEWPORT9 Viewport;
static HMODULE Hand;
static DWORD id;
static HANDLE processHandle;
static DWORD BAddress;
DWORD addPrim;

typedef HRESULT(APIENTRY *Present)(LPDIRECT3DDEVICE9, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*); //17
HRESULT APIENTRY Present_hook(LPDIRECT3DDEVICE9, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*);
Present Present_orig = 0;


typedef HRESULT(APIENTRY *SetStreamSource_t)(IDirect3DDevice9*, UINT, IDirect3DVertexBuffer9*, UINT, UINT);
HRESULT APIENTRY SetStreamSource_hook(IDirect3DDevice9*, UINT, IDirect3DVertexBuffer9*, UINT, UINT);
SetStreamSource_t SetStreamSource_orig = 0;

typedef HRESULT(APIENTRY *SetVertexShaderConstantF_t)(IDirect3DDevice9*, UINT, const float*, UINT);
HRESULT APIENTRY SetVertexShaderConstantF_hook(IDirect3DDevice9*, UINT, const float*, UINT);
SetVertexShaderConstantF_t SetVertexShaderConstantF_orig = 0;


typedef HRESULT(APIENTRY *SetTexture_t)(IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9 *);
HRESULT APIENTRY SetTexture_hook(IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9 *);
SetTexture_t SetTexture_orig = 0;




typedef HRESULT(APIENTRY *Reset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
HRESULT APIENTRY Reset_hook(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
Reset_t Reset_orig = 0;

LPDIRECT3DTEXTURE9 Red, Green, Blue, Yellow, Orange, Purple;


struct Vector3
{
	float x;
	float y;
	float z;
};



void log(string text, bool newline = true)
{
	if (false)
	{
		std::ofstream outfile;
		outfile.open("D:\\log.txt", std::ios_base::app);
		if (newline)
			outfile << text.c_str() << endl;
		else
			outfile << text.c_str();
	}
}
static bool xdxd = true;
static LPD3DXFONT font = NULL;
UINT mStartregister;
UINT mVectorCount;
class Memory
{
public:
	template <typename Type, typename Base, typename Offset>
	static inline Type Ptr(Base base, Offset offset)
	{
		static_assert(std::is_pointer<Type>::value || std::is_integral<Type>::value, "Type must be a pointer or address");
		static_assert(std::is_pointer<Base>::value || std::is_integral<Base>::value, "Base must be a pointer or address");
		static_assert(std::is_pointer<Offset>::value || std::is_integral<Offset>::value, "Offset must be a pointer or address");

		return base ? reinterpret_cast<Type>((reinterpret_cast<uint64_t>(base) + static_cast<uint64_t>(offset))) : nullptr;
	}

	template <typename Type>
	static bool IsValidPtr(Type* ptr)
	{
		return (ptr && sizeof(ptr)) ? true : false;
	}

	static bool IsValidPtr(void* ptr)
	{
		return (ptr && sizeof(ptr)) ? true : false;
	}
	template<typename T>
	static T ReadMemory(SIZE_T address)
	{
		T buffer;
		if (!ReadProcessMemory(processHandle, (LPCVOID)address, &buffer, sizeof(T), NULL))
		{
			log("FAIL RPM");

		}
		return buffer;
	}

	template<typename T>
	static bool ReadMemoryC(SIZE_T address)
	{
		T buffer;
		if (!ReadProcessMemory(processHandle, (LPCVOID)address, &buffer, sizeof(T), NULL))
		{
			return false;

		}
		return true;
	}


	static string ReadString(SIZE_T address, int Size)
	{

		string buffer = "";
		if (!ReadProcessMemory(processHandle, (LPCVOID)address, &buffer, Size, NULL))
			log("FAIL STR");
		return buffer;
	}
};


HRESULT APIENTRY SetStreamSource_hook(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT sStride)
{
	if (StreamNumber == 0)
		Stride = sStride;

	return SetStreamSource_orig(pDevice, StreamNumber, pStreamData, OffsetInBytes, sStride);
}


HRESULT APIENTRY SetVertexShaderConstantF_hook(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, const float *pConstantData, UINT Vector4fCount)
{
	if (pConstantData != NULL)
	{
		mStartregister = StartRegister;
		mVectorCount = Vector4fCount;
	}

	return SetVertexShaderConstantF_orig(pDevice, StartRegister, pConstantData, Vector4fCount);
}


HRESULT GenerateTexture(IDirect3DDevice9 *pDevice, IDirect3DTexture9 **ppD3Dtex, DWORD colour32)
{
	if (FAILED(pDevice->CreateTexture(8, 8, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, ppD3Dtex, NULL)))
		return E_FAIL;

	WORD colour16 = ((WORD)((colour32 >> 28) & 0xF) << 12)
		| (WORD)(((colour32 >> 20) & 0xF) << 8)
		| (WORD)(((colour32 >> 12) & 0xF) << 4)
		| (WORD)(((colour32 >> 4) & 0xF) << 0);

	D3DLOCKED_RECT d3dlr;
	(*ppD3Dtex)->LockRect(0, &d3dlr, 0, 0);
	WORD *pDst16 = (WORD*)d3dlr.pBits;

	for (int xy = 0; xy < 8 * 8; xy++)
		*pDst16++ = colour16;

	(*ppD3Dtex)->UnlockRect(0);

	return S_OK;
}

IDirect3DVertexShader9* vShader;

UINT vSize;
bool Direct3DInitialize = false;

IDirect3DPixelShader9 *Front, *Back;

HRESULT APIENTRY SetTexture_hook(LPDIRECT3DDEVICE9 pDevice, DWORD Sampler, IDirect3DBaseTexture9 *pTexture)
{
	if (InitOnce)
	{
		InitOnce = false;
		GenerateTexture(pDevice, &Green, D3DCOLOR_RGBA(0, 255, 0, 255));
		GenerateTexture(pDevice, &Red, D3DCOLOR_ARGB(255, 255, 0, 0));
		GenerateTexture(pDevice, &Blue, D3DCOLOR_ARGB(255, 0, 0, 255));
		GenerateTexture(pDevice, &Yellow, D3DCOLOR_ARGB(255, 255, 255, 0));
		GenerateTexture(pDevice, &Orange, D3DCOLOR_ARGB(255, 255, 165, 0));
		GenerateTexture(pDevice, &Purple, D3DCOLOR_ARGB(255, 102, 0, 153));
	}

	//get vSize
	if (SUCCEEDED(pDevice->GetVertexShader(&vShader)))
		if (vShader != NULL)
			if (SUCCEEDED(vShader->GetFunction(NULL, &vSize)))
				if (vShader != NULL) { vShader->Release(); vShader = NULL; }


	if (opt.d3d.Chams) {
		//wallhack
		pDevice->SetRenderState(D3DRS_DEPTHBIAS, 0);
		if (vSize == 2300 || vSize == 900 ||
			vSize == 1952 || vSize == 640 || Stride == 48 || vSize == 2300 || vSize == 900 ||
			vSize == 1952 || vSize == 640)//vSize == 1436
		{

			float r = 0, g = 0, b = 0,a=0;
			if (opt.chamsopt.green)
				r = 1.0f;
			if (opt.chamsopt.blue)
				g = 1.0f;
			if (opt.chamsopt.alpha)
				b = 1.0f;
			if (opt.chamsopt.alpha)
				a = 1.0f;

			float sColor[4] = { a,r,g,b};
			pDevice->SetPixelShaderConstantF(0, sColor, 4);

			float bias = 1000.0f;
			float bias_float = static_cast<float>(-bias);
			bias_float /= 10000.0f;
			pDevice->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&bias_float);
		}



	}


	return SetTexture_orig(pDevice, Sampler, pTexture);
}





//nSeven
HRESULT DrawString(LPD3DXFONT Font, INT X, INT Y, DWORD dColor, CONST PCHAR cString, ...)
{
	HRESULT hRet;

	CHAR buf[512] = { NULL };
	va_list ArgumentList;
	va_start(ArgumentList, cString);
	_vsnprintf_s(buf, sizeof(buf), sizeof(buf) - strlen(buf), cString, ArgumentList);
	va_end(ArgumentList);

	RECT rc[2];
	SetRect(&rc[0], X, Y, X, 0);
	SetRect(&rc[1], X, Y, X + 50, 50);

	hRet = D3D_OK;

	if (SUCCEEDED(hRet))
	{
		Font->DrawTextA(NULL, buf, -1, &rc[0], DT_NOCLIP, 0xFF000000);
		hRet = Font->DrawTextA(NULL, buf, -1, &rc[1], DT_NOCLIP, dColor);
	}

	return hRet;
}

//lucastx


class ClientApp
{
public:
	static bool WorldToScreen(Vector3 pos, D3DXVECTOR3 &screen, D3DXMATRIX matrix, int windowWidth, int windowHeight)
	{
		D3DXVECTOR4 clipCoords;
		clipCoords.x = pos.x*matrix[0] + pos.y*matrix[4] + pos.z*matrix[8] + matrix[12];
		clipCoords.y = pos.x*matrix[1] + pos.y*matrix[5] + pos.z*matrix[9] + matrix[13];
		clipCoords.z = pos.x*matrix[2] + pos.y*matrix[6] + pos.z*matrix[10] + matrix[14];
		clipCoords.w = pos.x*matrix[3] + pos.y*matrix[7] + pos.z*matrix[11] + matrix[15];

		if (clipCoords.w < 0.1f)
			return false;

		D3DXVECTOR3 NDC;
		NDC.x = clipCoords.x / clipCoords.w;
		NDC.y = clipCoords.y / clipCoords.w;
		NDC.z = clipCoords.z / clipCoords.w;

		screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
		screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);
		return true;
	}
	static bool WorldToScreen(D3DXVECTOR3 pos, D3DXVECTOR3 &screen, D3DXMATRIX matrix, int windowWidth, int windowHeight)
	{
		D3DXVECTOR4 clipCoords;
		clipCoords.x = pos.x*matrix[0] + pos.y*matrix[4] + pos.z*matrix[8] + matrix[12];
		clipCoords.y = pos.x*matrix[1] + pos.y*matrix[5] + pos.z*matrix[9] + matrix[13];
		clipCoords.z = pos.x*matrix[2] + pos.y*matrix[6] + pos.z*matrix[10] + matrix[14];
		clipCoords.w = pos.x*matrix[3] + pos.y*matrix[7] + pos.z*matrix[11] + matrix[15];

		if (clipCoords.w < 0.1f)
			return false;

		D3DXVECTOR3 NDC;
		NDC.x = clipCoords.x / clipCoords.w;
		NDC.y = clipCoords.y / clipCoords.w;
		NDC.z = clipCoords.z / clipCoords.w;

		screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
		screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);
		return true;
	}
	
};



static int GetDistance(Vector3 v1, Vector3 v2, int Divide)
{
	D3DXVECTOR3 vector3;
	vector3.x = v1.x - v2.x;
	vector3.y = v1.y - v2.y;
	vector3.z = v1.z - v2.z;
	return (int)sqrt(pow((double)vector3.x, 2.0) + pow((double)vector3.y, 2.0) + pow((double)vector3.z, 2.0)) / Divide;
}

const char* getitemname(int a) {
	switch (a)
	{
	case 1: return "Lv 1 helmet";
	case 2: return "Lv 2 helmet";
	case 3:  return "Lv 3 helmet";
	case 5:  return "Lv 1 Armor";
	case 6:  return "Lv 2 Armor";
	case 7:  return "Lv 3 Armor";
	case 8:  return "Lv 1 Backpack";
	case 9:  return "Lv 2 Backpack";
	case 10:  return "Lv 3 Backpack";
	case 19:  return "Ghillie Suit";
	case 50:  return "Fuel Barrel";
	case 101:  return "Bandage";
	case 102:  return "Med Kit";
	case 103:  return "First Aid Kit";
	case 105:  return "Sports Drink";
	case 106:  return "Cardio Tonic";
	case 1001:  return "M4A1 Rifle";
	case 1002:  return "AKM Rifle";
	case 1003:  return "M870 SG";
	case 1004:  return "M1887 SG";
	case 1005:  return "AA12 SG";
	case 1006:  return "AWM SR";
	case 1007:  return "Barett SR";
	case 1008:  return "M249 MG";
	case 1009:  return "M14EBR Rifle";
	case 1010:  return "AR15 Rifle";
	case 1011:  return "MP7 SMG";
	case 1012:  return "PP19 SMG";
	case 1013:  return "Thompson SMG";
	case 1014:  return "G18C Pistol";
	case 1015:  return "Desert Eagle";
	case 1017:  return "Vector";
	case 1018:  return "P90 SMG";
	case 1019:  return "SAIGA-12 Shotgun";
	case 1020:  return "WRO Hunting Rifle";
	case 1021:  return "SVD SR";
	case 1022:  return "M110 Sniper Rifle";
	case 1023:  return "ACR Rifle";
	case 1024:  return "AN94 Rifle";
	case 1025:  return "MP5 SMG";
	case 1026:  return "AUG Rifle";
	case 1027:  return "QBU Sniper Rifle";
	case 1031:  return "RPG Rocket Tube";
	case 1101:  return "SG Ammo";
	case 1102:  return "Rifle Ammo";
	case 1103:  return "SR Ammo";
	case 1104:  return "SMG Ammo";
	case 1105:  return "Pistol Ammo";
	case 1106:  return "RPG Ammo";
	case 1201:  return "Rifle Silencer";
	case 1202:  return "SR Silence";
	case 1204:  return "SMG Silencer";
	case 1211:  return "Rifle Compensator";
	case 1212:  return "SR Compensator";
	case 1213:  return "SMG Compensator";
	case 1221:  return "Rifle Flash Hider";
	case 1222:  return "SR Flash Hider";
	case 1223:  return "SMG Smoke Hider";
	case 1231:  return "SG Choke";
	case 1241:  return "Triangle Grip";
	case 1242:  return "Vertical Foregrip";
	case 1251:  return "Rifle QD-Mag";
	case 1252:  return "Rifle EX-Mag";
	case 1253:  return "Rifle EX-QD-Mag";
	case 1261:  return "SR Cheek Pad";
	case 1262:  return "Tactical Stock";
	case 1263:  return "SG Bullet Loop";
	case 1264:  return "SR EX-Mag";
	case 1265:  return "SR QD-Mag";
	case 1266:  return "SR EX-QD-Mag";
	case 1271:  return "Red Dot Sight";
	case 1272:  return "Holo-Sight";
	case 1273:  return "2x Scope";
	case 1274:  return "4x Scope";
	case 1275:  return "8x Scope";
	case 1276:  return "SMG QD-Mag";
	case 1277:  return "SMG EX-Mag";
	case 1278:  return "SMG QD-EX-Mag";
	case 1279:  return "Collapsible Stock";
	case 1303:  return "Crowbar";
	case 1306:  return "Frying Pan";
	case 1307:  return "Rubber Chicken";
	case 1401:  return "Grenade";
	case 1402:  return "Stun Grenade";
	case 1403:  return "Smoke Grenade";
	case 1404:  return "Molotov Cocktail";
	case 1405:  return "Chicken Grenade";
	case 1406:  return "Flash Grenade";

	default:return "no name";
		break;
	}
}



static void dieForever()
{
	DetourRemove((PBYTE)SetStreamSource_orig, (PBYTE)SetStreamSource_hook);
	DetourRemove((PBYTE)SetTexture_orig, (PBYTE)SetTexture_hook);
	DetourRemove((PBYTE)SetVertexShaderConstantF_orig, (PBYTE)SetVertexShaderConstantF_hook);
	DetourRemove((PBYTE)Reset_orig, (PBYTE)Reset_hook);
	DetourRemove((PBYTE)Present_orig, (PBYTE)Present_hook);
}


//nSeven
void DrawBox(IDirect3DDevice9 *pDevice, float x, float y, float w, float h, D3DCOLOR Color)
{
	struct Vertex
	{
		float x, y, z, ht;
		DWORD Color;
	}
	V[4] = { { x, y + h, 0.0f, 0.0f, Color },{ x, y, 0.0f, 0.01f, Color },
	{ x + w, y + h, 0.0f, 0.0f, Color },{ x + w, y, 0.0f, 0.0f, Color } };
	pDevice->SetTexture(0, NULL);
	pDevice->SetPixelShader(0);
	pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, V, sizeof(Vertex));
}

class D3DTLVERTEX
{
public:
	FLOAT X, Y, X2, Y2;
	DWORD Color;
};
IDirect3DPixelShader9* oldppShader;

void DrawLine(IDirect3DDevice9* pDevice, float X, float Y, float X2, float Y2, float Width, D3DCOLOR Color)
{
	D3DTLVERTEX qV[2] = {
		{ (float)X , (float)Y, 0.0f, 1.0f, Color },
	{ (float)X2 , (float)Y2 , 0.0f, 1.0f, Color },
	};
	const DWORD D3DFVF_TL = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pDevice->SetFVF(D3DFVF_TL);
	pDevice->SetTexture(0, NULL);
	pDevice->SetPixelShader(0);
	pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 2, qV, sizeof(D3DTLVERTEX));
}

void DrawFilledRect(int x, int y, int w, int h, D3DCOLOR color, IDirect3DDevice9* pDevice)
{
	if (w < 0)w = 1;
	if (h < 0)h = 1;
	if (x < 0)x = 1;
	if (y < 0)y = 1;

	D3DRECT Rect = { x,y, x + w, y + h };
	pDevice->Clear(1, &Rect, D3DCLEAR_TARGET | D3DCLEAR_TARGET, color, NULL, NULL);
}


void DrawBorder(int x, int y, int w, int h, int px, D3DCOLOR border_col, IDirect3DDevice9* pDevice)
{
	DrawFilledRect(x, (y + h - px), w, px, border_col, pDevice);
	DrawFilledRect(x, y, px, h, border_col, pDevice);
	DrawFilledRect(x, y, w, px, border_col, pDevice);
	DrawFilledRect((x + w - px), y, px, h, border_col, pDevice);
}

static bool InsideCircle(int xc, int yc, int r, int x, int y)
{
	int dx = xc - x;
	int dy = yc - y;
	return dx * dx + dy * dy <= r * r;
}


void Box2D(D3DXVECTOR3 head, D3DXVECTOR3 root, DWORD dwColor, LPDIRECT3DDEVICE9 pDevice)
{
	D3DXVECTOR3 Box = head - root;
	if (Box.y < 0) Box.y *= -1;
	int BoxWidth = (int)Box.y / 2;
	int DrawX = (int)head.x - (BoxWidth / 2);
	int DrawY = (int)head.y;
	DrawBorder(DrawX, DrawY, BoxWidth, (int)Box.y, 2, dwColor, pDevice);
	DrawBorder(DrawX, DrawY, BoxWidth, (int)Box.y, 1, 0xFF000000, pDevice);
}


//hay , needle 
static bool findnew(string str1, string str2)
{
	//log("checking '"+str1+"' == '"+str2+"'",false);
	if (strstr(str1.c_str(), str2.c_str()))
	{
		//log("YES");
		return true;
	}
	//log("Nope.avi");
	return false;
}
D3DXVECTOR3 GetMidPoint(Vector3 V1, Vector3 V2)
{
	return D3DXVECTOR3((V1.x + V2.x) / 2, (V1.y + V2.y) / 2, (V1.z + V2.z) / 2);
}

static bool once = false;

static bool freee = true;


void drawesp(IDirect3DDevice9* pDevice)
{

	if (opt.options.reset)
	{
		dieForever();
	}
	int m_pWorld = Memory::ReadMemory<int>(BAddress + OFFSET_PYGAME + 0x410);
	int m_pSceneContext = Memory::ReadMemory<int>(m_pWorld + 0x8);
	int cameraBase = Memory::ReadMemory<int>(m_pSceneContext + 0x4);
	if (!Memory::ReadMemoryC<D3DXMATRIX>(cameraBase + 0xC4) && ingame == true)
	{
		//dieForever();
		ingame = false;
	}

	D3DXMATRIX worldMatrix = Memory::ReadMemory<D3DXMATRIX>(cameraBase + 0xC4);
	int visibleCount = Memory::ReadMemory<int>(m_pWorld + 0x278);// 'As CModelFactory->CModelSkeletal 0x28 '0x27c
	int pLocalModel = Memory::ReadMemory<int>(m_pWorld + 0x27C);
	int pSkeletonList = Memory::ReadMemory<int>(m_pWorld + 0x290);// 'As CModelFactory->CModelSkeletal 0x3C 'pSkeletonList 0x28c
	D3DXVECTOR3 vScreen;
	pDevice->GetViewport(&Viewport);
	int width = (int)Viewport.Width;
	int height = (int)Viewport.Height;
	int num1 = Memory::ReadMemory<int>(BAddress + OFFSET_LOCALPLAYER);
	int numnya = Memory::ReadMemory<int>(BAddress + 34);
	Vector3 MyPosition;
	MyPosition.x = Memory::ReadMemory<float>(num1 + 16);
	MyPosition.y = Memory::ReadMemory<float>(num1 + 16 + 4);
	MyPosition.z = Memory::ReadMemory<float>(num1 + 16 + 8);
	for (int i = 0; i < visibleCount; i += 4)
	{
		

		int r_pModel = Memory::ReadMemory<int>(pSkeletonList + i);

		int SpaceNode = Memory::ReadMemory<int>(r_pModel + 0x1C);
		int m_pAnimator = Memory::ReadMemory<int>(r_pModel + 0x328);
		D3DXMATRIX m_Position = Memory::ReadMemory<D3DXMATRIX>(r_pModel + 0x3B0);

		string m_pModelName = Memory::ReadString(Memory::ReadMemory<int>(m_pAnimator + 0x528), 16);
		string str = m_pModelName;
		bool isPlayer = findnew(str, "character");
		bool isBot = findnew(str, "Robot");// (str.find("Robot") != std::string::npos);
		bool isVehicle = findnew(str, "vehicle");// (str.find("Land") != std::string::npos);
		bool isItem = findnew(str, "item");// (str.find("DtsProp") != std::string::npos);
		bool isSupplyBox = findnew(str, "DtsPlayerHeritage");// (str.find("DtsPlayerHeritage") != std::string::npos);
		bool isPlane = findnew(str, "Plane");// (str.find("Plane") != std::string::npos);w
		bool isWeapon = findnew(str, "ClientWeaponEntity");// (str.find("ClientWeaponEntity") != std::string::npos);
		Vector3 EnemyPos;
		EnemyPos.x = m_Position[12];
		EnemyPos.y = m_Position[13];
		EnemyPos.z = m_Position[14];
		if (i == 0)
		{
			MyPosition.x = EnemyPos.x;
			MyPosition.y = EnemyPos.y;
			MyPosition.z = EnemyPos.z;
			continue;

		}
		if (ClientApp::WorldToScreen(EnemyPos, vScreen, worldMatrix, width, height))
		{
			if (isPlayer)
			{

				DrawString(font, vScreen.x, vScreen.y - 20, RED(255), (PCHAR)"PLAYER");

				if (opt.espopt.lines)
					DrawLine(pDevice, (float)(width / 2) - 1, 10, EnemyPos.x, EnemyPos.y,20, D3DCOLOR_ARGB(255, 0, 255, 255));

				float entityHeight = 21.5f;
				D3DXVECTOR3 pRoot;
				ClientApp::WorldToScreen(EnemyPos, pRoot, worldMatrix, width, height);
				Vector3 eHead;
				eHead.x = EnemyPos.x;
				eHead.y = EnemyPos.y + entityHeight;
				eHead.z = EnemyPos.z;
				float dist2 = GetDistance(MyPosition, EnemyPos, 10);
				D3DXVECTOR3 pHead;
				ClientApp::WorldToScreen(eHead, pHead, worldMatrix, width, height);
				struct Rectangle
				{
					int width;
					int height;
					int X;
					int Y;
				};
				Rectangle rect;
				rect.width = (int)(700 / dist2);
				if (rect.width > 100)
				{
					rect.width = 100;
				}
				rect.height = (int)(pRoot.y - pHead.y);
				rect.X = (int)pRoot.x - rect.width / 2;
				rect.Y = (int)pRoot.y - rect.height;
				if (dist2 > 25)
				{
					rect.Y = (int)pRoot.y - rect.height + 15;
				}
				if (opt.espopt.box3)
				{
					D3DXVECTOR3 Pos0, Pos1, Pos2, Pos3, Pos4, Pos5, Pos6, Pos7, Pos8;
					Vector3 EnemyPos1 = EnemyPos;
					EnemyPos1.y -= 10;
					Vector3 eHead1 = eHead;
					eHead1.y += 10;
					Pos0 = GetMidPoint(eHead1, EnemyPos1);
					Pos1 = Pos0 + D3DXVECTOR3(-5, 17, -5);
					Pos2 = Pos0 + D3DXVECTOR3(-5, -17, -5);
					Pos3 = Pos0 + D3DXVECTOR3(5, -17, -5);
					Pos4 = Pos0 + D3DXVECTOR3(5, 17, -5);
					Pos5 = Pos0 + D3DXVECTOR3(-5, 17, 5);
					Pos6 = Pos0 + D3DXVECTOR3(-5, -17, 5);
					Pos7 = Pos0 + D3DXVECTOR3(5, -17, 5);
					Pos8 = Pos0 + D3DXVECTOR3(5, 17, 5);
					D3DXVECTOR3 bone1;
					D3DXVECTOR3 bone2;
					D3DXVECTOR3 bone3;
					D3DXVECTOR3 bone4;
					D3DXVECTOR3 bone5;
					D3DXVECTOR3 bone6;
					D3DXVECTOR3 bone7;
					D3DXVECTOR3 bone8;
					if (true)
					{
						if (ClientApp::WorldToScreen(Pos1, bone1, worldMatrix, width, height)
							&& ClientApp::WorldToScreen(Pos2, bone2, worldMatrix, width, height)
							&& ClientApp::WorldToScreen(Pos3, bone3, worldMatrix, width, height)
							&& ClientApp::WorldToScreen(Pos4, bone4, worldMatrix, width, height)
							&& ClientApp::WorldToScreen(Pos5, bone5, worldMatrix, width, height)
							&& ClientApp::WorldToScreen(Pos6, bone6, worldMatrix, width, height)
							&& ClientApp::WorldToScreen(Pos7, bone7, worldMatrix, width, height)
							&& ClientApp::WorldToScreen(Pos8, bone8, worldMatrix, width, height))
						{
							DrawLine(pDevice, bone1.x, bone1.y - 38, bone2.x, bone2.y - 38, 1, BLUE(255));
							DrawLine(pDevice, bone2.x, bone2.y - 38, bone3.x, bone3.y - 38, 1, BLUE(255));
							DrawLine(pDevice, bone3.x, bone3.y - 38, bone4.x, bone4.y - 38, 1, BLUE(255));
							DrawLine(pDevice, bone4.x, bone4.y - 38, bone1.x, bone1.y - 38, 1, BLUE(255));
							DrawLine(pDevice, bone5.x, bone5.y - 38, bone6.x, bone6.y - 38, 1, BLUE(255));
							DrawLine(pDevice, bone6.x, bone6.y - 38, bone7.x, bone7.y - 38, 1, BLUE(255));
							DrawLine(pDevice, bone7.x, bone7.y - 38, bone8.x, bone8.y - 38, 1, BLUE(255));
							DrawLine(pDevice, bone8.x, bone8.y - 38, bone5.x, bone5.y - 38, 1, BLUE(255));
							DrawLine(pDevice, bone1.x, bone1.y - 38, bone5.x, bone5.y - 38, 1, BLUE(255));
							DrawLine(pDevice, bone2.x, bone2.y - 38, bone6.x, bone6.y - 38, 1, BLUE(255));
							DrawLine(pDevice, bone3.x, bone3.y - 38, bone7.x, bone7.y - 38, 1, BLUE(255));
							DrawLine(pDevice, bone4.x, bone4.y - 38, bone8.x, bone8.y - 38, 1, BLUE(255));
						}
					}
				}
				//POINT pt;
				if (opt.espopt.box2)
					Box2D(pHead, pRoot, WHITE(255), pDevice); 

				if (GetAsyncKeyState(VK_LSHIFT) && opt.d3d.aimbot)
				{
					if (InsideCircle(width / 2, height / 2, 30, vScreen.x, vScreen.y))
						//	//mouse_event(MOUSEEVENTF_MOVE , vScreen.x, vScreen.y, 0, 0); //move
					{
						DrawLine(pDevice, vScreen.x, vScreen.y, width / 2.0, height - 15.0f,20, RED(255));
						SetCursorPos(pHead.x, pHead.y);


					}
				}
				char result[20] = "";
				sprintf(result, "%.1f m", (dist2));
				if(opt.espopt.dist)
					DrawString(font, vScreen.x, vScreen.y - 10, WHITE(255), (PCHAR)result);
				int num6 = Memory::ReadMemory<int>(Memory::ReadMemory<int>(Memory::ReadMemory<int>(Memory::ReadMemory<int>(r_pModel + 256) + 20) + 152) + 8);
				//int num6 = 100; //TODO fix
				if (opt.espopt.health)
				{
					if (num6 >= 100)
					{
						DrawString(font, vScreen.x - 10, vScreen.y + 15, GREEN(255), (PCHAR)"++++++++++");
					}
					if (num6 >= 90)
					{
						DrawString(font, vScreen.x - 10, vScreen.y + 15, GREEN(255), (PCHAR)"+++++++++");
					}
					if (num6 >= 80)
					{
						DrawString(font, vScreen.x - 10, vScreen.y + 15, YELLOW(255), (PCHAR)"++++++++");
					}
					if (num6 >= 70)
					{
						DrawString(font, vScreen.x - 10, vScreen.y + 15, YELLOW(255), (PCHAR)"+++++++");
					}
					if (num6 >= 60)
					{
						DrawString(font, vScreen.x - 10, vScreen.y + 15, ORANGE(255), (PCHAR)"++++++");
					}
					if (num6 >= 50)
					{
						DrawString(font, vScreen.x - 10, vScreen.y + 15, DARKORANGE(255), (PCHAR)"+++++");
					}
					if (num6 >= 40)
					{
						DrawString(font, vScreen.x - 10, vScreen.y + 15, GRAY(255), (PCHAR)"++++");
					}
					if (num6 >= 30)
					{
						DrawString(font, vScreen.x - 10, vScreen.y + 15, GRAY(255), (PCHAR)"+++");
					}
					if (num6 >= 20)
					{
						DrawString(font, vScreen.x - 10, vScreen.y + 15, RED(255), (PCHAR)"++");
					}
					if (num6 >= 10)
					{
						DrawString(font, vScreen.x - 10, vScreen.y + 15, RED(255), (PCHAR)"+");
					}
				}
				//dist2 = GetDistance(MyPosition, EnemyPos, 10);
				//sprintf(result, "%.1f", dist2);
				//DrawString(font, vScreen.x - 10, vScreen.y + 30, WHITE(255), (PCHAR)result);
			}
			else
				if (isBot && opt.espopt.player)
					DrawString(font, vScreen.x, vScreen.y, GREEN(255), (PCHAR)"BOT");
				else
					if (isItem && opt.espopt.item)
					{
						/*int cliententitytable = Memory::ReadMemory<int>(Adress3 + 0x100);
						int cliententitytableptr = Memory::ReadMemory<int>(cliententitytable + 0x14);
						int propID = Memory::ReadMemory<int>(Memory::ReadMemory<int>(cliententitytableptr + 0x2C) + 0x8);
						*/string itemName;
			DrawString(font, vScreen.x, vScreen.y, YELLOW(255), (PCHAR)"ITEM"/*getitemname(propID)*/);
					}
					else
						if (isVehicle && opt.espopt.vehicle)
						{
							DrawString(font, vScreen.x, vScreen.y, BLUE(255), (PCHAR)"VEHICLE");
						}
						else
							if (isWeapon &&opt.espopt.item)
								DrawString(font, vScreen.x, vScreen.y, BLACK(100), (PCHAR)"WEAPON");
							else
								if (opt.espopt.type)
									DrawString(font, vScreen.x, vScreen.y, WHITE(255), (PCHAR)str.c_str());

		}
	}

}


cMenu Menu;
copt opt;
cSound Sound;

float ScreenCenterX = 0.0f;
float ScreenCenterY = 0.0f;

void PreReset(LPDIRECT3DDEVICE9 pDevice)
{
	Menu.PreReset();
	return;
}

void PostReset(LPDIRECT3DDEVICE9 pDevice)
{
	Menu.PostReset(pDevice);
	return;
}


LPD3DXFONT Font;
static cMenu mensu;


HRESULT CreateMyShader(IDirect3DPixelShader9 **pShader, IDirect3DDevice9 *Device, float red, float green, float blue, float alpha)
{
	ID3DXBuffer *MyBuffer = NULL;
	char MyShader[256];
	sprintf(MyShader, "ps.1.1\ndef c0, %f, %f, %f, %f\nmov r0,c0", red / 255, green / 255, blue / 255, alpha / 255);
	D3DXAssembleShader(MyShader, sizeof(MyShader), NULL, NULL, 0, &MyBuffer, NULL);
	if (FAILED(Device->CreatePixelShader((const DWORD*)MyBuffer->GetBufferPointer(), pShader)))return E_FAIL;
	return S_OK;
}

HRESULT APIENTRY Present_hook(LPDIRECT3DDEVICE9 pDevice, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
	
	/*if (!Direct3DInitialize)
	{
		Direct3DInitialize = true;

		CreateMyShader(&Front, pDevice, 255, 0, 0, 255);
		CreateMyShader(&Back, pDevice, 255, 255, 0, 255);
	}*/
	
	if (font == NULL)
		D3DXCreateFont(pDevice, 14, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Italic"), &font);

	if (xdxd)
	{
		//AllocConsole();
		//freopen("CONOUT$", "w", stdout);
		id = GetCurrentProcessId();
		processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, id);
		BAddress = (DWORD)GetModuleHandle(NULL);
		/*std::thread t1(drawesp, pDevice);
		t1.detach();*/
		xdxd = false;
	}

	//if (font && menu)
	//{

	//	DrawString(font, 190, 100, D3DCOLOR_ARGB(255, 255, 255, 255), (PCHAR)"Venom v3 by Ashesh [unknowncheats.me]");
	//	/*	DrawString(font, 190, 150, D3DCOLOR_ARGB(255, 255, 255, 255), (PCHAR)" [[Insert -  Toggle Menu]]");
	//		DrawString(font, 190, 200, D3DCOLOR_ARGB(255, 255, 255, 255), (PCHAR)"Arrow Keys - Switch Colors");
	//		DrawString(font, 190, 250, D3DCOLOR_ARGB(255, 255, 255, 255), (PCHAR)"F1 - Toggle Chams");
	//		DrawString(font, 190, 270, D3DCOLOR_ARGB(255, 255, 255, 255), (PCHAR)"F2 - Toggle ESP");
	//		DrawString(font, 190, 230, D3DCOLOR_ARGB(255, 255, 255, 255), (PCHAR)"Delete - Toggle Lines");
	//		DrawString(font, 190, 300, D3DCOLOR_ARGB(255, 255, 255, 255), (PCHAR)"F4 - Toggle MultiColor");*/

	//}
	if (!Tools.Init)
	{
		D3DXCreateLine(pDevice, &Tools.pLine);
		Tools.Init = TRUE;
	}
	PostReset(pDevice);
	Menu.ShowMenu(pDevice);
	PreReset(pDevice);

	pDevice->GetViewport(&Tools.g_ViewPort);
	ScreenCenterX = (float)Tools.g_ViewPort.Width / 2;
	ScreenCenterY = (float)Tools.g_ViewPort.Height / 2;

	if (opt.d3d.cross)
	{
		Tools.FillRGB(ScreenCenterX - 14, ScreenCenterY, 10, 1, D3DXCOLOR(1.0, 0.0, 0.0, 1.0), pDevice);//Left line
		Tools.FillRGB(ScreenCenterX + 5, ScreenCenterY, 10, 1, D3DXCOLOR(1.0, 0.0, 0.0, 1.0), pDevice);//Right line
		Tools.FillRGB(ScreenCenterX, ScreenCenterY - 14, 1, 10, D3DXCOLOR(1.0, 0.0, 0.0, 1.0), pDevice);//Top line
		Tools.FillRGB(ScreenCenterX, ScreenCenterY + 5, 1, 10, D3DXCOLOR(1.0, 0.0, 0.0, 1.0), pDevice);//Bottom line
		Tools.DrawCircle(D3DXVECTOR2(ScreenCenterX, ScreenCenterY), 8, 60, D3DXCOLOR(1.0, 0.0, 0.0, 1.0));//Circle
		Tools.DrawPoint(ScreenCenterX, ScreenCenterY, 1, 1, D3DXCOLOR(0.0, 1.0, 0.0, 1.0), pDevice);//Point
	}
	if (opt.d3d.esp)
		drawesp(pDevice);
	return Present_orig(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion); // return anywhere in this function crash xD
}


HRESULT APIENTRY Reset_hook(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	if (Font)
		Font->OnLostDevice();

	if (Tools.pLine)
		Tools.pLine->OnLostDevice();
	HRESULT ResetReturn = Reset_orig(pDevice, pPresentationParameters);

	if (SUCCEEDED(ResetReturn))
	{
		if (Font)
			Font->OnResetDevice();
		if (Tools.pLine)
			Tools.pLine->OnResetDevice();
	}


	return ResetReturn;
}

class Renderer
{
public:
	class RenderInfo
	{
	public:
		char _0x0000[48];
		__int32 m_FrameCounter; //0x0030 
	};

	char _0x0000[8];
	RenderInfo* m_pRenderInfo; //0x0008 
	char _0x000C[56];
	__int32 m_Width; //0x0044 
	__int32 m_Height; //0x0048 
	char _0x004C[68];
	__int32 m_PrimCount; //0x0090 
	__int32 m_ModelPrimCount; //0x0094 
	__int32 m_ModelDPCount; //0x0098 
	char _0x009C[44];
	float m_LogicFPS; //0x00C8 
	float m_RenderFPS; //0x00CC 

	static Renderer* GetInstance()
	{
		uintptr_t uBase = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));
		return *reinterpret_cast<Renderer**>(uBase + OFFSET_RENDERER);
	}
};


DWORD WINAPI RosD3D(__in LPVOID lpParameter)
{
	HMODULE dDll = NULL;
	while (!dDll)
	{
		dDll = GetModuleHandleA("d3d9.dll");
		Sleep(100);
	}
	CloseHandle(dDll);

	IDirect3D9* d3d = NULL;
	IDirect3DDevice9* d3ddev = NULL;

	HWND tmpWnd = CreateWindowA("BUTTON", "RosD3D", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, Hand, NULL);
	if (tmpWnd == NULL)
	{

		return 0;
	}

	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d == NULL)
	{
		DestroyWindow(tmpWnd);

		return 0;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = tmpWnd;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	HRESULT result = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, tmpWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);
	if (result != D3D_OK)
	{
		d3d->Release();
		DestroyWindow(tmpWnd);

		return 0;
	}


#if defined _M_X64
	DWORD64* dVtable = (DWORD64*)d3ddev;
	dVtable = (DWORD64*)dVtable[0];
#elif defined _M_IX86
	DWORD* dVtable = (DWORD*)d3ddev;
	dVtable = (DWORD*)dVtable[0];
#endif
	SetStreamSource_orig = (SetStreamSource_t)dVtable[100];
	SetVertexShaderConstantF_orig = (SetVertexShaderConstantF_t)dVtable[94];
	SetTexture_orig = (SetTexture_t)dVtable[65];
	Reset_orig = (Reset_t)dVtable[16];
	Present_orig = (Present)dVtable[17];

	Present_orig = (Present)DetourFunction((PBYTE)Present_orig, (PBYTE)Present_hook);
	SetStreamSource_orig = (SetStreamSource_t)DetourFunction((PBYTE)SetStreamSource_orig, (PBYTE)SetStreamSource_hook);
	SetTexture_orig = (SetTexture_t)DetourFunction((PBYTE)SetTexture_orig, (PBYTE)SetTexture_hook);
	SetVertexShaderConstantF_orig = (SetVertexShaderConstantF_t)DetourFunction((PBYTE)SetVertexShaderConstantF_orig, (PBYTE)SetVertexShaderConstantF_hook);
	Reset_orig = (Reset_t)DetourFunction((PBYTE)Reset_orig, (PBYTE)Reset_hook);


	while (!GetAsyncKeyState(VK_END))
	{


		if (GetAsyncKeyState(VK_UP))
		{
			color = 1; Sleep(100);
		}
		if (GetAsyncKeyState(VK_DOWN))
		{
			color = 2; Sleep(100);
		}
		if (GetAsyncKeyState(VK_RIGHT))
		{
			color = 3; Sleep(100);
		}
		if (GetAsyncKeyState(VK_LEFT))
		{
			color = 4; Sleep(100);
		}
	}
	Sleep(1000);
	dieForever();
	return 0;
}



BOOL WINAPI DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:

		CreateThread(0, 0, RosD3D, 0, 0, 0);
		break;

	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
	}