//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: patches.cpp,v 1.33 2006/05/07 17:32:29 kyeman Exp $
//
//----------------------------------------------------------

#include "../main.h"

void InstallSCMEventsProcessor();
void RelocateScanListHack();
void RelocatePedsListHack();
void RelocateObjectListHack();

void ApplyGameVehicleColorPatch();

extern int iGtaVersion;
extern CNetGame* pNetGame;
extern CGame* pGame;
extern DWORD dwUIMode;

DWORD dwSystemMemory;
DWORD dwStreamingMemory;

//----------------------------------------------------------

#define memadd(num,arr) dwAlloc+=num;cBytes=(char*)&dwAlloc;for(int i=0;i<4;i++)arr[i]=cBytes[i]
#define patch(a,b) _patch(a,b,sizeof(b))

//----------------------------------------------------------

void _patch(DWORD dwAddress,BYTE* bData,int iSize)
{
	DWORD dwProtect[2];
	VirtualProtect((PVOID)dwAddress,iSize,PAGE_EXECUTE_READWRITE,&dwProtect[0]);
	memcpy((PVOID)dwAddress,bData,iSize);
	VirtualProtect((PVOID)dwAddress,iSize,dwProtect[0],&dwProtect[1]);
}

//----------------------------------------------------------

void SetIPLs(int iIPLs, int iGtaVersion)
{
	DWORD dwAlloc = (DWORD) malloc(iIPLs+4);
	if(dwAlloc)
	{
		memset((void*)dwAlloc,0x0,iIPLs+4);
		char* cBytes = (char*)&dwAlloc;

		BYTE bIPLs[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };

		if(iGtaVersion == GTASA_VERSION_USA10)
		{
			patch(0x1569777,bIPLs);
			patch(0x15649FA,bIPLs);
			patch(0x1561160,bIPLs);
		}
		else
		{
			patch(0x1569717,bIPLs);
			patch(0x156495A,bIPLs);
			patch(0x156115C,bIPLs);
		}
		patch(0x40619B,bIPLs);
		patch(0x405C3D,bIPLs);
	}	
}

//----------------------------------------------------------

void SetTimedObjects(int iTimedObjects)
{
	DWORD dwAlloc = (DWORD) malloc((0x24*iTimedObjects)+4);
	if(dwAlloc)
	{
		memset((LPVOID)dwAlloc,0x00,(0x24*iTimedObjects)+4);
		for(DWORD i=dwAlloc+4;i<(dwAlloc+4+(0x24*iTimedObjects));i+=0x24)
		{
			*(BYTE*) i = 0xB0;
			*(BYTE*) (i+1) = 0xBC;
			*(BYTE*) (i+2) = 0x85;
			*(BYTE*) (i+10) = 0xFF;
			*(BYTE*) (i+11) = 0xFF;
			*(BYTE*) (i+34) = 0xFF;
			*(BYTE*) (i+35) = 0xFF;
		}
		char* cBytes = (char*)&dwAlloc;
		BYTE bTimedObjects[] = { cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x4C66B1,bTimedObjects);
		patch(0x4C66C2,bTimedObjects);
		patch(0x84BC51,bTimedObjects);
		patch(0x856261,bTimedObjects);
		patch(0x4C683B,bTimedObjects);
		memadd(4,bTimedObjects);
		patch(0x4C6464,bTimedObjects);
		patch(0x4C66BD,bTimedObjects);
		cBytes = (char*)&iTimedObjects;
		BYTE pushTimedObjects[] = { 0x68, cBytes[0], cBytes[1], cBytes[2], cBytes[3] };
		patch(0x4C58A5,pushTimedObjects);
	}
}

//----------------------------------------------------------

void UnFuckAndCheck(DWORD addr, int size, BYTE byteCheck)
{
	DWORD d;
	char s[256];
	VirtualProtect((PVOID)addr,size,PAGE_EXECUTE_READWRITE,&d);

	if(byteCheck != *(PBYTE)addr) {
#ifdef _DEBUG
		sprintf(s,"Failed Check At Addr: 0x%X",addr);
		OutputDebugString(s);
#endif
		while(byteCheck != *(PBYTE)addr) Sleep(1);

		VirtualProtect((PVOID)addr,size,PAGE_EXECUTE_READWRITE,&d);
	}
}

//----------------------------------------------------------

void UnFuck(DWORD addr, int size)
{
	DWORD d;
	VirtualProtect((PVOID)addr,size,PAGE_EXECUTE_READWRITE,&d);
}

//----------------------------------------------------------

void ApplyDebugLevelPatches()
{
	// Don't go back to player anims, use the peds IDE
	UnFuck(0x609A4E,6);
	memset((PVOID)0x609A4E, 0x90, 6);
}

//----------------------------------------------------------

BOOL ApplyPreGamePatches()
{	
	BYTE * pbyteVersionDetermination = (PBYTE)ADDR_BYPASS_VIDS_USA10;
	int iCounter=0;

	// MAIN VERSION DETERMINING LOGIC
	while( (*pbyteVersionDetermination != 0x89) &&
		   (*pbyteVersionDetermination != 0xC8) )
	{
		if (*(PBYTE)ADDR_GAME_STARTED == 1) {
			return FALSE;
		} else {
			Sleep(10);
			iCounter++;
			if(iCounter>6000) { // 60 seconds have passed
				return FALSE;
			}
		}
	}

	if(*pbyteVersionDetermination == 0x89) {
		iGtaVersion = GTASA_VERSION_USA10;
	} 
	else if(*pbyteVersionDetermination == 0xC8) {
		iGtaVersion = GTASA_VERSION_EU10;
	}

	// (skip to starting screen)
	if(iGtaVersion == GTASA_VERSION_USA10) {
		UnFuck(ADDR_BYPASS_VIDS_USA10,6);
		*(BYTE *)ADDR_ENTRY = 5;
		memset((PVOID)ADDR_BYPASS_VIDS_USA10,0x90,6);
	} 
	else if (iGtaVersion == GTASA_VERSION_EU10) {
		UnFuck(ADDR_BYPASS_VIDS_EU10,6);
		*(BYTE *)ADDR_ENTRY = 5;
		memset((PVOID)ADDR_BYPASS_VIDS_EU10,0x90,6);
	}

	// Loading screens
	UnFuck(0x866CD8,10);
	UnFuck(0x866CCC,10);
	strcpy((PCHAR)0x866CD8,"title");
	strcpy((PCHAR)0x866CCC,"title");

	// allow more resolutions by removing aspect ratio conditions
	UnFuck(0x745B87,68);
	memset((PVOID)0x745B87,0x90,68);

	// allow widescreen resolutions
	UnFuck(0x7459E1,2);
	memset((PVOID)0x7459E1,0x90,2);

	// change timer to rely on RenderWare's RsTimer()?
	UnFuckAndCheck(0x561872,30,0x85);
	*(PBYTE)0x561872 = 0x33;
	*(PBYTE)0x561873 = 0xC0;
	memset((PVOID)0x561874,0x90,27);

	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	
	dwSystemMemory = statex.ullTotalPhys / (1024 * 1024);

	if(dwSystemMemory > 4000)
		dwStreamingMemory = 0x40000000; // 1024MB
	else if(dwSystemMemory > 2000)
		dwStreamingMemory = 0x20000000; // 512MB
	else if(dwSystemMemory > 1000)
		dwStreamingMemory = 0x10000000; // 256MB
	else if(dwSystemMemory > 500)
		dwStreamingMemory = 0x08000000; // 128MB
	else
		dwStreamingMemory = 0x06000000; // 96MB

	// Modify the streaming memory hardcoded values
	UnFuck(0x5B8E6A,4);
	//*(DWORD *)0x5B8E6A = 134217728; // 128MB
	*(DWORD *)0x5B8E6A = dwStreamingMemory;

	// For SCM disable/enable
	// TODO: Comment this out. It's not present in the latest version
	// but this is required to get in the game
	UnFuck(0x469EF5,2);

	// prevent processing image files (GTA3.IMG, GTA_INT.IMG)
	// manually load these in default.dat
	UnFuckAndCheck(0x4083C0,1,0xB8);
	*(PBYTE)0x4083C0 = 0xC3;

	// Disable transitions/fading on loading screen
	UnFuck(0x590099,5);
	memset((PVOID)0x590099,0x90,5);

	// Frame sleep
	UnFuck(0x53E94C,1);
	*(PBYTE)0x53E94C = 2;

	// Increase TexDictionary
	UnFuck(0x731F60,4);
	*(DWORD*)0x731F60 = 20000;

	return TRUE;
}

//----------------------------------------------------------

BYTE pbyteVehiclePoolAllocPatch[] = {0x6A,0x00,0x68,0xC6,0x2,0x00,0x00}; // 710
BYTE pbyteCollisionPoolAllocPatch[] = { 0x68,0xFF,0x7E,0x00,0x00 }; // 32511
BYTE pbyteEntryInfoPoolAllocPatch[] = { 0x68,0x00,0x8,0x00,0x00 }; // 2048

BYTE pbyteTrainDelrailmentPatch[] = {
	0xB8, 0x89, 0x8F, 0x6F, 0x00, 0xFF, 0xE0
};

extern DWORD dwFarClipHookAddr;
extern DWORD dwFarClipReturnAddr;

void ApplyInGamePatches()
{	
	if(GTASA_VERSION_USA10 == iGtaVersion) {
		dwFarClipHookAddr = 0x7EE2A0;
		dwFarClipReturnAddr = dwFarClipHookAddr+9;
	} else {
		dwFarClipHookAddr = 0x7EE2E0;
		dwFarClipReturnAddr = dwFarClipHookAddr+9;
	}	

	// Rest of the stuff
	RelocateScanListHack();
	
	RelocatePedsListHack(); // allows us to use all 300 ped model slots
	
	ApplyGameVehicleColorPatch();

	RelocateObjectListHack();

	// APPLY THE DAMN NOP PATCH AND QUIT ASCIING QUESTIONS!

	/* Frame limiter default ~40 fps
	UnFuck(0xC1704C,1);
	*(PDWORD)0xC1704C = 55UL; // yes that means 40..*/

	// Increase the vehicle pool limit (see top of proc for patch)
	UnFuckAndCheck(0x551024,sizeof(pbyteVehiclePoolAllocPatch),0x68);
	memcpy((PVOID)0x551024,pbyteVehiclePoolAllocPatch,sizeof(pbyteVehiclePoolAllocPatch));

	/* ----THIS IS GTAU STUFF
	// Increase Buildings
	UnFuck(0x55105F,4);
	*(DWORD *)0x55105F = 160000;

	// Increase Dummys
	UnFuck(0x5510CF,4);
	*(DWORD *)0x5510CF = 16000;

	// Increase Ptr Node Double
	UnFuck(0x550F82,4);
	*(DWORD *)0x550F82 = 20000;

	// Increase IPL Files
	UnFuck(0x405F26,4);
	*(DWORD *)0x405F26 = 4096;

	// Increase Object Pool
	UnFuck(0x551097,4);
	*(DWORD *)0x551097 = 1000;

	// Increase IPLs
	SetIPLs(200,iGtaVersion);

	// Increase Timed Objects
	SetTimedObjects(1000);*/
	// ----END GTAU
	
	// Increase the ped pool limit (210)
	UnFuck(0x550FF2,1);
	*(PBYTE)0x550FF2 = 0xD2;

	// And we need 210 ped intelligence too plz
	UnFuck(0x551283,1);
	*(PBYTE)0x551283 = 0xD2; // thx
	
	// And a larger task pool
	UnFuck(0x551140,1);
	*(PBYTE)0x551140 = 0x05; // 1524

	// And a larger event pool
	UnFuck(0x551178,1);
	*(PBYTE)0x551178 = 0x01; // 456

	// And we'd definitely need some more matrices.
	// Who doesn't need more matrices?
	UnFuck(0x54F3A2,1);
	*(PBYTE)0x54F3A2 = 0x15; // 4228

	// (aru) Patch the VehicleStruct pool to have more than 50 vehicles
	//       We'll use 127 (max is 211 vehicles)
	//       We can't just patch the imm8 in the push due to it being unsigned
	//       so, we'll use our intelligence to hack it in :)
	UnFuck(0x5B8FDE, 7);
	*(BYTE*)0x5B8FDE = 0x6A;	// push imm8
	*(BYTE*)0x5B8FDF = 0x00;	// 0
	*(BYTE*)0x5B8FE0 = 0x68;	// push imm32
	*(BYTE*)0x5B8FE1 = 127;		// 127 types
	*(BYTE*)0x5B8FE2 = 0x00;
	*(BYTE*)0x5B8FE3 = 0x00;
	*(BYTE*)0x5B8FE4 = 0x00;
	
	// Increase the collision model ptr
	UnFuck(0x551106,sizeof(pbyteCollisionPoolAllocPatch));
	memcpy((PVOID)0x551106,pbyteCollisionPoolAllocPatch,sizeof(pbyteCollisionPoolAllocPatch));

	// Increase the entry info nodes
	UnFuck(0x550FB9,sizeof(pbyteEntryInfoPoolAllocPatch));
	memcpy((PVOID)0x550FB9,pbyteEntryInfoPoolAllocPatch,sizeof(pbyteEntryInfoPoolAllocPatch));

	//------------------
	
	/* clouds RenderEffects (needs checking)
	UnFuck(0x53E1AF,10);
	memset((PVOID)0x53E1AF,0x90,10);

	// 53E121 (15) low level cloud (needs checking)
	UnFuck(0x53E121,15);
	memset((PVOID)0x53E121,0x90,15);*/

	// Remove random procedural geometry like plants/rocks etc.
	UnFuck(0x53C159,5);
	memset((PVOID)0x53C159,0x90,5);
	
	if (dwUIMode != 0) 
	{
		// Move the radar to the top left of the screen
		UnFuck(0x866B70,4);
		*(float *)0x866B70 = 430.0f;
	    
		// Scale down the hud slightly
		//UnFuck(0x859520,8);
		//*(float *)0x859520 = 0.0011f;
		//*(float *)0x859524 = 0.00172f;
	}

	/* Cursor hiding
	UnFuck(0x7481CD,16);
	memset((PVOID)0x7481CD,0x90,16);
    
	UnFuck(0x747FEA,1);
	*(BYTE*)0x747FEA = 1;

	// A SetCursorPos proc internally
	UnFuck(0x6194A0,1);
	*(PBYTE)0x6194A0 = 0xC3;*/

	// No vehicle name rendering
	UnFuck(0x58FBE9,5);
    memset((PVOID)0x58FBE9,0x90,5);

	// No playidles anim loading.
	UnFuck(0x86D1EC,1);
	*(BYTE*)0x86D1EC = '\0';

	// Prevent replays
	UnFuck(0x53C090,5);
	memset((PVOID)0x53C090,0x90,5);

	// NO MORE INTERIOR PEDS
	UnFuck(0x440833,8);
	memset((PVOID)0x440833,0x90,8);

	// (ped shadows crash)
	UnFuck(0x53EA08,10);
	memset((PVOID)0x53EA08,0x90,10);

	// Anti-pause
	UnFuck(0x561AF0,7);
	memset((PVOID)0x561AF0,0x90,7);
	
	// Unknown from CPlayerPed::ProcessControl causes crash
	UnFuck(0x609C08,39);
	memset((PVOID)0x609C08,0x90,39);

	// FindPlayerVehicle (Always use nPlayerPed)
	UnFuck(0x56E0FA,18);
	memset((PVOID)0x56E0FA,0x90,18);

	// CMotorBike::ProcessControlInputs.. why oh why..
	UnFuck(0x6BC9EB,2);
	memset((PVOID)0x6BC9EB,0x90,2);

	// This removes the random weapon pickups (e.g. on the hill near chilliad)
	UnFuck(0x5B47B0,1);
	memset((PVOID)0x5B47B0,0xC3,1);

	// Removes the FindPlayerInfoForThisPlayerPed at these locations.
	UnFuck(0x5E63A6,19);
	memset((PVOID)0x5E63A6,0x90,19);

	UnFuck(0x621AEA,12);
	memset((PVOID)0x621AEA,0x90,12);

	UnFuck(0x62D331,11);
	memset((PVOID)0x62D331,0x90,11);

	UnFuck(0x741FFF,27);
	memset((PVOID)0x741FFF,0x90,27);

	/* temp testing vehicle horn
	UnFuck(0x500360,11);
	memset((PVOID)0x500360,0x90,11);*/
	
	// hack to remove motion blur in high speed vehicle
	UnFuck(0x704E8A,5);
	memset((PVOID)0x704E8A,0x90,5);

	// Respawn and Interior
	UnFuck(0x4090A0,1);
	*(BYTE*)0x4090A0 = 0xC3;
	
	// Respawn and Interior
	UnFuck(0x441482,5);
	memset((void*)0x441482, 0x90, 5);

	// No MessagePrint
	UnFuck(0x588BE0,1);
	*(BYTE*)0x588BE0 = 0xC3;

	// No IPL vehicle
	UnFuck(0x53C06A,5);
	memset((PVOID)0x53C06A,0x90,5);

	// SomeCarGenerator (0x41a8b3 crash)
	UnFuck(0x434272,5);
	memset((PVOID)0x434272,0x90,5);

	// CPlayerPed_CPlayerPed .. task system corrupts some shit
	UnFuck(0x60D64D,2);
	*(PBYTE)0x60D64E = 0x84; // jnz to jz
	
	// CPhysical Destructor (705b3b crash)
	UnFuck(0x542485,11);
	memset((PVOID)0x542485,0x90,11);
	
	// No peds kthx. (CPopulation::AddPed() nulled)
	UnFuck(0x612710,3);
	*(BYTE*)0x612710 = 0x33;
	*(BYTE*)0x612711 = 0xC0; // xor eax, eax
	*(BYTE*)0x612712 = 0xC3; // ret

	// Fuck the call to CPopulation::AddPed() for create_train off to kingdom kong	
    UnFuck(0x613BA7, 5);
	*(BYTE*)0x613BA7 = 0x90;
	*(BYTE*)0x613BA8 = 0x90;
	*(BYTE*)0x613BA9 = 0x90;
	*(BYTE*)0x613BAA = 0x90;
	*(BYTE*)0x613BAB = 0x90;

	// Don't go back to player anims, use the peds IDE
	UnFuck(0x609A4E,6);
	memset((PVOID)0x609A4E, 0x90, 6);

	// Train derailment 
	UnFuck(0x006F8CF8, 12);
	memset((PVOID)0x006F8CF8, 0x90, 5); // (Actual hook is installed in hooks.cpp)
	memcpy((PVOID)(0x006F8CF8+5), pbyteTrainDelrailmentPatch, sizeof(pbyteTrainDelrailmentPatch));

	// CarCtl::GenerateRandomCars nulled from CGame::Process (firetrucks etc)
	UnFuck(0x53C1C1,5);
	memset((PVOID)0x53C1C1,0x90,5);

	// (540040 bug), test ecx for 0 instead of [ecx+270]
	UnFuck(0x540040,10);
	// nop the first 8 bytes
	memset((PVOID)0x540040,0x90,6);
	*(PBYTE)0x540046 = 0x85;
	*(PBYTE)0x540047 = 0xC9; // test ecx, ecx
	*(PBYTE)0x540048 = 0x74; // jz
	
	// No wasted message
	UnFuck(0x56E5AD,5);
	memset((PVOID)0x56E5AD,0x90,5);

	// For the input disabling in CGame.
	UnFuck(0x541DF5,5);

	/* Ret at CCamera::ClearPlayerWeaponMode
	UnFuck(0x50AB10,2);
	*(PBYTE)0x50AB10 = 0xC3;*/
	UnFuck(0x609CB4,5);
	memset((PVOID)0x609CB4,0x90,5);

	// PlayerInfo checks in CPlayerPed::ProcessControl
	UnFuck(0x60F2C4,25);
	memset((PVOID)0x60F2C4,0x90,25);

	// No Vehicle Audio Processing (done manually from the hooks)
	UnFuck(0x6B18F1,5);
	memset((PVOID)0x6B18F1,0x90,5);
	UnFuck(0x6B9298,5);
	memset((PVOID)0x6B9298,0x90,5);
	UnFuck(0x6F1793,5);
	memset((PVOID)0x6F1793,0x90,5);
	UnFuck(0x6F86B6,5);
	memset((PVOID)0x6F86B6,0x90,5);

	// camera_on_actor patch, tsk tsk R*
	UnFuck(0x0047C477,1);
	*(BYTE*)0x0047C477 = 0xEB;

	// CPushBike fires set on CPed patch
	UnFuck(0x0053A984,2);
	*(BYTE*)0x0053A984 = 0xEB;  // jmp
	*(BYTE*)0x0053A985 = 0x77;  // +77h = 0x0053A9FD

	// Stop sniper clicking
	UnFuck(0x0060F289, 8);
	memset((PVOID)0x0060F289, 0x90, 8);
	UnFuck(0x0060F29D, 19);
	memset((PVOID)0x0060F29D, 0x90, 19);
	
	// Automatic go-to-menu on alt+tab
	//UnFuck(0x748063, 5);
	//memset((PVOID)0x748063, 0x90, 5);

	// Wanted level hook
	UnFuck(0x58DB5F, 9);
	*(BYTE*)0x58DB5F = 0xBD;
	*(BYTE*)0x58DB60 = 0x00;
	*(BYTE*)0x58DB61 = 0x00;
	*(BYTE*)0x58DB62 = 0x00;
	*(BYTE*)0x58DB63 = 0x00;
	*(BYTE*)0x58DB64 = 0x90;
	*(BYTE*)0x58DB65 = 0x90;
	*(BYTE*)0x58DB66 = 0x90;
	*(BYTE*)0x58DB67 = 0x90;
	
	// text_draw data
	//UnFuck(0xA913E8, MAX_TEXT_DRAWS * sizeof (TEXT_DRAW_DATA));
	//UnFuck(0xA44B68, 2);

	// Remove the blue(-ish) fog in the map
	UnFuck(0x00575B0E, 5);
	memset((PVOID)0x00575B0E, 0x90, 5);

	// Make the shadows slightly darker by increasing the alpha
	UnFuck(0x71162C,1);
	*(PBYTE)0x71162C = 80;

	// Remove the CReferences call from CTaskEnterVehicleDriver ctor
	UnFuck(0x63ADC8,6);
	memset((PVOID)0x63ADC8,0x90,6);

	// Stop ped rotations from the camera
	UnFuck(0x6884C4,6);
	memset((PVOID)0x6884C4,0x90,6);

	UnFuck(0x47BF54,4);
	InstallSCMEventsProcessor();
}

//----------------------------------------------------------

#pragma pack(1)
typedef struct _PED_MODEL
{
	DWORD func_tbl;
	BYTE  data[64];
} PED_MODEL;

PED_MODEL PedModelsMemory[319];

void RelocatePedsListHack()
{
    BYTE *aPedsListMemory = (BYTE*)&PedModelsMemory[0];

	// Init the mem
	int x=0;
	while(x!=319) {
		PedModelsMemory[x].func_tbl = 0x85BDC0;
		memset(PedModelsMemory[x].data,0,64);
		x++;
	}
	// Patch the GetPedsModelInfo to use us
	// instead of the gta_sa.exe mem.
	UnFuck(0x4C67AD,4);
	*(DWORD *)0x4C67AD = (DWORD)aPedsListMemory;
}

//----------------------------------------------------------

#pragma pack(1)
typedef struct _OBJECT_MODEL
{
	DWORD func_tbl;
	BYTE  data[28];
} OBJECT_MODEL;

OBJECT_MODEL ObjectModelsMemory[20000];

DWORD dwPatchAddrObjectModelsReloc[14] = {
	0x4C63F2,0x4C662D,0x4C6822,0x4C6829,0x4C6877,0x4C6881,0x4C6890,
	0x4C68A5,0x4C68F3,0x4C6932,0x4C6971,0x4C69B0,0x4C69EF,0x4C6A2E
};

void RelocateObjectListHack()
{
	BYTE *aObjectsListMemory = (BYTE*)&ObjectModelsMemory[0];

	// Init the mem
	int x=0;
	while(x!=20000) {
		ObjectModelsMemory[x].func_tbl = 0x85BBF0;
		memset(ObjectModelsMemory[x].data,0,28);
		x++;
	}

	x=0;
	while(x!=14) {
		UnFuck(dwPatchAddrObjectModelsReloc[x],4);
		*(DWORD*)dwPatchAddrObjectModelsReloc[x] = (DWORD)aObjectsListMemory;
		x++;
	}
}

//----------------------------------------------------------
// FOLLOWING IS TO RELOCATE THE SCANLIST MEMORY, A BIG
// HACK THAT ALLOWS US TO HAVE MORE THAN 2 CPlayerInfo STRUCTURES.

unsigned char ScanListMemory[8*20000];

// Pointers to actual code addresses to patch. The first list
// has taken into account the instruction bytes, second list
// does not. The second list is guaranteed to have 3 byte
// instructions before the new address.

DWORD dwPatchAddrScanReloc1USA[14] = {
0x5DC7AA,0x41A85D,0x41A864,0x408259,0x711B32,0x699CF8,
0x4092EC,0x40914E,0x408702,0x564220,0x564172,0x563845,
0x84E9C2,0x85652D };

DWORD dwPatchAddrScanReloc1EU[14] = {
0x5DC7AA,0x41A85D,0x41A864,0x408261,0x711B32,0x699CF8,
0x4092EC,0x40914E,0x408702,0x564220,0x564172,0x563845,
0x84EA02,0x85656D };

// Lots of hex.. that's why they call us a "determined group of hackers"

DWORD dwPatchAddrScanReloc2USA[56] = {
0x0040D68C,0x005664D7,0x00566586,0x00408706,0x0056B3B1,0x0056AD91,0x0056A85F,0x005675FA,
0x0056CD84,0x0056CC79,0x0056CB51,0x0056CA4A,0x0056C664,0x0056C569,0x0056C445,0x0056C341,
0x0056BD46,0x0056BC53,0x0056BE56,0x0056A940,0x00567735,0x00546738,0x0054BB23,0x006E31AA,
0x0040DC29,0x00534A09,0x00534D6B,0x00564B59,0x00564DA9,0x0067FF5D,0x00568CB9,0x00568EFB,
0x00569F57,0x00569537,0x00569127,0x0056B4B5,0x0056B594,0x0056B2C3,0x0056AF74,0x0056AE95,
0x0056BF4F,0x0056ACA3,0x0056A766,0x0056A685,0x0070B9BA,0x0056479D,0x0070ACB2,0x006063C7,
0x00699CFE,0x0041A861,0x0040E061,0x0040DF5E,0x0040DDCE,0x0040DB0E,0x0040D98C,0x01566855 };

DWORD dwPatchAddrScanReloc2EU[56] = {
0x0040D68C,0x005664D7,0x00566586,0x00408706,0x0056B3B1,0x0056AD91,0x0056A85F,0x005675FA,
0x0056CD84,0x0056CC79,0x0056CB51,0x0056CA4A,0x0056C664,0x0056C569,0x0056C445,0x0056C341,
0x0056BD46,0x0056BC53,0x0056BE56,0x0056A940,0x00567735,0x00546738,0x0054BB23,0x006E31AA,
0x0040DC29,0x00534A09,0x00534D6B,0x00564B59,0x00564DA9,0x0067FF5D,0x00568CB9,0x00568EFB,
0x00569F57,0x00569537,0x00569127,0x0056B4B5,0x0056B594,0x0056B2C3,0x0056AF74,0x0056AE95,
0x0056BF4F,0x0056ACA3,0x0056A766,0x0056A685,0x0070B9BA,0x0056479D,0x0070ACB2,0x006063C7,
0x00699CFE,0x0041A861,0x0040E061,0x0040DF5E,0x0040DDCE,0x0040DB0E,0x0040D98C,0x01566845 };

DWORD dwPatchAddrScanReloc3[11] = {
0x004091C5,0x00409367,0x0040D9C5,0x0040DB47,0x0040DC61,0x0040DE07,0x0040DF97,
0x0040E09A,0x00534A98,0x00534DFA,0x0071CDB0 };

// For End
// 0xB992B8 is reffed for checking end of scanlist... rewrite this to point to end of new list
DWORD dwPatchAddrScanRelocEnd[4] = { 0x005634A6, 0x005638DF, 0x0056420F, 0x00564283 };


//-----------------------------------------------------------

void RelocateScanListHack()
{
	DWORD oldProt;
	memset(&ScanListMemory[0], 0, sizeof(ScanListMemory));
	unsigned char *aScanListMemory = &ScanListMemory[0];

	// FIRST PREPARED LIST OF ACCESSORS
	int x=0;
	while(x!=14) {
		if(iGtaVersion == GTASA_VERSION_USA10) {
			VirtualProtect((PVOID)dwPatchAddrScanReloc1USA[x],4,PAGE_EXECUTE_READWRITE,&oldProt);
			*(PDWORD)dwPatchAddrScanReloc1USA[x] = (DWORD)aScanListMemory;
		}
		else if(iGtaVersion == GTASA_VERSION_EU10) {
			VirtualProtect((PVOID)dwPatchAddrScanReloc1EU[x],4,PAGE_EXECUTE_READWRITE,&oldProt);
			*(PDWORD)dwPatchAddrScanReloc1EU[x] = (DWORD)aScanListMemory;
		}
		x++;
	}

	// SECOND PREPARED LIST OF ACCESSORS <G>
	x=0;
	while(x!=56) {
		if(iGtaVersion == GTASA_VERSION_USA10) {
			VirtualProtect((PVOID)dwPatchAddrScanReloc2USA[x],8,PAGE_EXECUTE_READWRITE,&oldProt);
			*(PDWORD)(dwPatchAddrScanReloc2USA[x] + 3) = (DWORD)aScanListMemory;
		}
		else if(iGtaVersion == GTASA_VERSION_EU10) {
			VirtualProtect((PVOID)dwPatchAddrScanReloc2EU[x],8,PAGE_EXECUTE_READWRITE,&oldProt);
			*(PDWORD)(dwPatchAddrScanReloc2EU[x] + 3) = (DWORD)aScanListMemory;
		}
		x++;
	}

	// THIRD LIST THAT POINTS TO THE BASE SCANLIST MEMORY + 4
	x=0;
	while(x!=11) {
		VirtualProtect((PVOID)dwPatchAddrScanReloc3[x],8,PAGE_EXECUTE_READWRITE,&oldProt);
		*(PDWORD)(dwPatchAddrScanReloc3[x] + 3) = (DWORD)(aScanListMemory+4);
		x++;
	}	

	// FOURTH LIST THAT POINTS TO THE END OF THE SCANLIST
	x=0;
	while(x!=4) {
		VirtualProtect((PVOID)dwPatchAddrScanRelocEnd[x],4,PAGE_EXECUTE_READWRITE,&oldProt);
		*(PDWORD)(dwPatchAddrScanRelocEnd[x]) = (DWORD)(aScanListMemory+sizeof(ScanListMemory));
		x++;
	}	

	VirtualProtect((LPVOID)0x564DC7,4,PAGE_EXECUTE_READWRITE,&oldProt);
	*(PDWORD)0x564DC7 = (DWORD)(ScanListMemory+115200);

	// Others that didn't fit.
	VirtualProtect((PVOID)0x40936A,4,PAGE_EXECUTE_READWRITE,&oldProt);
	*(PDWORD)0x40936A = (DWORD)(aScanListMemory+4);

	// Reset the exe scanlist mem for playerinfo's
	memset((BYTE*)0xB7D0B8,0,8*14400);
}

//----------------------------------------------------------

RGBA VehicleColors[256] = {
{0x00,0x00,0x00,0xFF},{0xF5,0xF5,0xF5,0xFF},{0x2A,0x77,0xA1,0xFF},{0x84,0x04,0x10,0xFF},
{0x26,0x37,0x39,0xFF},{0x86,0x44,0x6E,0xFF},{0xD7,0x8E,0x10,0xFF},{0x4C,0x75,0xB7,0xFF},
{0xBD,0xBE,0xC6,0xFF},{0x5E,0x70,0x72,0xFF},{0x46,0x59,0x7A,0xFF},{0x65,0x6A,0x79,0xFF},
{0x5D,0x7E,0x8D,0xFF},{0x58,0x59,0x5A,0xFF},{0xD6,0xDA,0xD6,0xFF},{0x9C,0xA1,0xA3,0xFF},
{0x33,0x5F,0x3F,0xFF},{0x73,0x0E,0x1A,0xFF},{0x7B,0x0A,0x2A,0xFF},{0x9F,0x9D,0x94,0xFF},
{0x3B,0x4E,0x78,0xFF},{0x73,0x2E,0x3E,0xFF},{0x69,0x1E,0x3B,0xFF},{0x96,0x91,0x8C,0xFF},
{0x51,0x54,0x59,0xFF},{0x3F,0x3E,0x45,0xFF},{0xA5,0xA9,0xA7,0xFF},{0x63,0x5C,0x5A,0xFF},
{0x3D,0x4A,0x68,0xFF},{0x97,0x95,0x92,0xFF},{0x42,0x1F,0x21,0xFF},{0x5F,0x27,0x2B,0xFF},
{0x84,0x94,0xAB,0xFF},{0x76,0x7B,0x7C,0xFF},{0x64,0x64,0x64,0xFF},{0x5A,0x57,0x52,0xFF},
{0x25,0x25,0x27,0xFF},{0x2D,0x3A,0x35,0xFF},{0x93,0xA3,0x96,0xFF},{0x6D,0x7A,0x88,0xFF},
{0x22,0x19,0x18,0xFF},{0x6F,0x67,0x5F,0xFF},{0x7C,0x1C,0x2A,0xFF},{0x5F,0x0A,0x15,0xFF},
{0x19,0x38,0x26,0xFF},{0x5D,0x1B,0x20,0xFF},{0x9D,0x98,0x72,0xFF},{0x7A,0x75,0x60,0xFF},
{0x98,0x95,0x86,0xFF},{0xAD,0xB0,0xB0,0xFF},{0x84,0x89,0x88,0xFF},{0x30,0x4F,0x45,0xFF},
{0x4D,0x62,0x68,0xFF},{0x16,0x22,0x48,0xFF},{0x27,0x2F,0x4B,0xFF},{0x7D,0x62,0x56,0xFF},
{0x9E,0xA4,0xAB,0xFF},{0x9C,0x8D,0x71,0xFF},{0x6D,0x18,0x22,0xFF},{0x4E,0x68,0x81,0xFF},
{0x9C,0x9C,0x98,0xFF},{0x91,0x73,0x47,0xFF},{0x66,0x1C,0x26,0xFF},{0x94,0x9D,0x9F,0xFF},
{0xA4,0xA7,0xA5,0xFF},{0x8E,0x8C,0x46,0xFF},{0x34,0x1A,0x1E,0xFF},{0x6A,0x7A,0x8C,0xFF},
{0xAA,0xAD,0x8E,0xFF},{0xAB,0x98,0x8F,0xFF},{0x85,0x1F,0x2E,0xFF},{0x6F,0x82,0x97,0xFF},
{0x58,0x58,0x53,0xFF},{0x9A,0xA7,0x90,0xFF},{0x60,0x1A,0x23,0xFF},{0x20,0x20,0x2C,0xFF},
{0xA4,0xA0,0x96,0xFF},{0xAA,0x9D,0x84,0xFF},{0x78,0x22,0x2B,0xFF},{0x0E,0x31,0x6D,0xFF},
{0x72,0x2A,0x3F,0xFF},{0x7B,0x71,0x5E,0xFF},{0x74,0x1D,0x28,0xFF},{0x1E,0x2E,0x32,0xFF},
{0x4D,0x32,0x2F,0xFF},{0x7C,0x1B,0x44,0xFF},{0x2E,0x5B,0x20,0xFF},{0x39,0x5A,0x83,0xFF},
{0x6D,0x28,0x37,0xFF},{0xA7,0xA2,0x8F,0xFF},{0xAF,0xB1,0xB1,0xFF},{0x36,0x41,0x55,0xFF},
{0x6D,0x6C,0x6E,0xFF},{0x0F,0x6A,0x89,0xFF},{0x20,0x4B,0x6B,0xFF},{0x2B,0x3E,0x57,0xFF},
{0x9B,0x9F,0x9D,0xFF},{0x6C,0x84,0x95,0xFF},{0x4D,0x84,0x95,0xFF},{0xAE,0x9B,0x7F,0xFF},
{0x40,0x6C,0x8F,0xFF},{0x1F,0x25,0x3B,0xFF},{0xAB,0x92,0x76,0xFF},{0x13,0x45,0x73,0xFF},
{0x96,0x81,0x6C,0xFF},{0x64,0x68,0x6A,0xFF},{0x10,0x50,0x82,0xFF},{0xA1,0x99,0x83,0xFF},
{0x38,0x56,0x94,0xFF},{0x52,0x56,0x61,0xFF},{0x7F,0x69,0x56,0xFF},{0x8C,0x92,0x9A,0xFF},
{0x59,0x6E,0x87,0xFF},{0x47,0x35,0x32,0xFF},{0x44,0x62,0x4F,0xFF},{0x73,0x0A,0x27,0xFF},
{0x22,0x34,0x57,0xFF},{0x64,0x0D,0x1B,0xFF},{0xA3,0xAD,0xC6,0xFF},{0x69,0x58,0x53,0xFF},
{0x9B,0x8B,0x80,0xFF},{0x62,0x0B,0x1C,0xFF},{0x5B,0x5D,0x5E,0xFF},{0x62,0x44,0x28,0xFF},
{0x73,0x18,0x27,0xFF},{0x1B,0x37,0x6D,0xFF},{0xEC,0x6A,0xAE,0xFF},{0x00,0x00,0x00,0xFF},
{0x17,0x75,0x17,0xFF},{0x21,0x06,0x06,0xFF},{0x12,0x54,0x78,0xFF},{0x45,0x2A,0x0D,0xFF},
{0x57,0x1E,0x1E,0xFF},{0x01,0x07,0x01,0xFF},{0x25,0x22,0x5A,0xFF},{0x2C,0x89,0xAA,0xFF},
{0x8A,0x4D,0xBD,0xFF},{0x35,0x96,0x3A,0xFF},{0xB7,0xB7,0xB7,0xFF},{0x46,0x4C,0x8D,0xFF},
{0x84,0x88,0x8C,0xFF},{0x81,0x78,0x67,0xFF},{0x81,0x7A,0x26,0xFF},{0x6A,0x50,0x6F,0xFF},
{0x58,0x3E,0x6F,0xFF},{0x8C,0xB9,0x72,0xFF},{0x82,0x4F,0x78,0xFF},{0x6D,0x27,0x6A,0xFF},
{0x1E,0x1D,0x13,0xFF},{0x1E,0x13,0x06,0xFF},{0x1F,0x25,0x18,0xFF},{0x2C,0x45,0x31,0xFF},
{0x1E,0x4C,0x99,0xFF},{0x2E,0x5F,0x43,0xFF},{0x1E,0x99,0x48,0xFF},{0x1E,0x99,0x99,0xFF},
{0x99,0x99,0x76,0xFF},{0x7C,0x84,0x99,0xFF},{0x99,0x2E,0x1E,0xFF},{0x2C,0x1E,0x08,0xFF},
{0x14,0x24,0x07,0xFF},{0x99,0x3E,0x4D,0xFF},{0x1E,0x4C,0x99,0xFF},{0x19,0x81,0x81,0xFF},
{0x1A,0x29,0x2A,0xFF},{0x16,0x61,0x6F,0xFF},{0x1B,0x66,0x87,0xFF},{0x6C,0x3F,0x99,0xFF},
{0x48,0x1A,0x0E,0xFF},{0x7A,0x73,0x99,0xFF},{0x74,0x6D,0x99,0xFF},{0x53,0x38,0x7E,0xFF},
{0x22,0x24,0x07,0xFF},{0x3E,0x19,0x0C,0xFF},{0x46,0x21,0x0E,0xFF},{0x99,0x1E,0x1E,0xFF},
{0x8D,0x4C,0x8D,0xFF},{0x80,0x5B,0x80,0xFF},{0x7B,0x3E,0x7E,0xFF},{0x3C,0x17,0x37,0xFF},
{0x73,0x35,0x17,0xFF},{0x78,0x18,0x18,0xFF},{0x83,0x34,0x1A,0xFF},{0x8E,0x2F,0x1C,0xFF},
{0x7E,0x3E,0x53,0xFF},{0x7C,0x6D,0x7C,0xFF},{0x02,0x0C,0x02,0xFF},{0x07,0x24,0x07,0xFF},
{0x16,0x30,0x12,0xFF},{0x16,0x30,0x1B,0xFF},{0x64,0x2B,0x4F,0xFF},{0x36,0x84,0x52,0xFF},
{0x99,0x95,0x90,0xFF},{0x81,0x8D,0x96,0xFF},{0x99,0x99,0x1E,0xFF},{0x7F,0x99,0x4C,0xFF},
{0x83,0x92,0x92,0xFF},{0x78,0x82,0x22,0xFF},{0x2B,0x3C,0x99,0xFF},{0x3A,0x3A,0x0B,0xFF},
{0x8A,0x79,0x4E,0xFF},{0x0E,0x1F,0x49,0xFF},{0x15,0x37,0x1C,0xFF},{0x15,0x27,0x3A,0xFF},
{0x37,0x57,0x75,0xFF},{0x06,0x08,0x20,0xFF},{0x07,0x13,0x26,0xFF},{0x20,0x39,0x4B,0xFF},
{0x2C,0x50,0x89,0xFF},{0x15,0x42,0x6C,0xFF},{0x10,0x32,0x50,0xFF},{0x24,0x16,0x63,0xFF},
{0x69,0x20,0x15,0xFF},{0x8C,0x8D,0x94,0xFF},{0x51,0x60,0x13,0xFF},{0x09,0x0F,0x02,0xFF},
{0x8C,0x57,0x3A,0xFF},{0x52,0x88,0x8E,0xFF},{0x99,0x5C,0x52,0xFF},{0x99,0x58,0x1E,0xFF},
{0x99,0x3A,0x63,0xFF},{0x99,0x8F,0x4E,0xFF},{0x99,0x31,0x1E,0xFF},{0x0D,0x18,0x42,0xFF},
{0x52,0x1E,0x1E,0xFF},{0x42,0x42,0x0D,0xFF},{0x4C,0x99,0x1E,0xFF},{0x08,0x2A,0x1D,0xFF},
{0x96,0x82,0x1D,0xFF},{0x19,0x7F,0x19,0xFF},{0x3B,0x14,0x1F,0xFF},{0x74,0x52,0x17,0xFF},
{0x89,0x3F,0x8D,0xFF},{0x7E,0x1A,0x6C,0xFF},{0x0B,0x37,0x0B,0xFF},{0x27,0x45,0x0D,0xFF},
{0x07,0x1F,0x24,0xFF},{0x78,0x45,0x73,0xFF},{0x8A,0x65,0x3A,0xFF},{0x73,0x26,0x17,0xFF},
{0x31,0x94,0x90,0xFF},{0x56,0x94,0x1D,0xFF},{0x59,0x16,0x3D,0xFF},{0x1B,0x8A,0x2F,0xFF},
{0x38,0x16,0x0B,0xFF},{0x04,0x18,0x04,0xFF},{0x35,0x5D,0x8E,0xFF},{0x2E,0x3F,0x5B,0xFF},
{0x56,0x1A,0x28,0xFF},{0x4E,0x0E,0x27,0xFF},{0x70,0x6C,0x67,0xFF},{0x3B,0x3E,0x42,0xFF},
{0x2E,0x2D,0x33,0xFF},{0x7B,0x7E,0x7D,0xFF},{0x4A,0x44,0x42,0xFF},{0x28,0x34,0x4E,0xFF} };

RGBA UseVehicleColors[256];

void ApplyGameVehicleColorPatch()
{
	memcpy(UseVehicleColors,VehicleColors,sizeof(UseVehicleColors));

	UnFuck(0x44B1C1,4);
	*(PDWORD)0x44B1C1 = (DWORD)&UseVehicleColors;

	UnFuck(0x4C8390,4);
	*(PDWORD)0x4C8390 = (DWORD)&UseVehicleColors;

	UnFuck(0x4C8399,4);
	*(PDWORD)0x4C8399 = (DWORD)&UseVehicleColors->g;

	UnFuck(0x4C83A3,4);
	*(PDWORD)0x4C83A3 = (DWORD)&UseVehicleColors->b;

	UnFuck(0x5817CC,4);
	*(PDWORD)0x5817CC = (DWORD)&UseVehicleColors;

	UnFuck(0x582176,4);
	*(PDWORD)0x582176 = (DWORD)&UseVehicleColors;

	UnFuck(0x6A6FFA,4);
	*(PDWORD)0x6A6FFA = (DWORD)&UseVehicleColors;
}

//----------------------------------------------------------
