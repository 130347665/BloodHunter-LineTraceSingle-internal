
#include "includes.h"



namespace Data

{
	ULONG64 Module;
	float WindowWidth;
	float WindowHeight;
	float ScreenWidth;
	float ScreenHeight;
	_D3DMATRIX2 ViewWorld;
	ULONG64 Uworld = NULL;
	ULONG64 GNames = NULL;
	ULONG64 Matrix = NULL;//ImGui::GetColorU32({ 1.f, 0.f, 0.f,1.f })
	ULONG64 Ulever = NULL;
	ULONG64 Actor = NULL;
	ULONG64 Count = NULL;
	ULONG64 APawn = NULL;
	ULONG64 RootComponent = NULL;
	ULONG64 CameraManager = NULL;
	ULONG64 PlayerController = NULL;
	ULONG64 magicbullet_addr = NULL; //子弹追踪
	void DrawLine(int x, int y, int x1, int y1, ImU32 color, float thickness)
	{

		ImGui::GetOverlayDrawList()->AddLine(ImVec2(x, y), ImVec2(x1, y1), color, thickness);
	}

	void DrawRect(int x, int y, int w, int h, ImU32 color, float thickness)
	{
		ImGui::GetOverlayDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color, 0, 0, thickness);
	}
	void DrawTextA(int x, int y, ImU32 color, const char* text)
	{
		ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), color, text);
	}

	const char* GetFromFName(const int key)
	{
		DWORD chunkOffset = ((int)(key) >> 16); // Block
		WORD nameOffset = (WORD)key;
		auto namePoolChunk = Ram<uintptr_t>(GNames + ((chunkOffset + 2) * 8));
		auto entryOffset = namePoolChunk + (DWORD)(2 * nameOffset);
		INT16 nameLength =Ram<INT16>(entryOffset) >> 6;
		if (nameLength > 256)nameLength = 255;
		char cBuf[256] = {};
		RtlCopyMemory(&cBuf, (void*)(entryOffset + 2), nameLength);
		cBuf[nameLength] = 0;
		return cBuf;
	}

	bool WorldToScreen(const class Vector3& WorldLocation, Vector4* Rect)
	{
		Vector3 ViewW = Vector3(0.0f, 0.0f, 0.0f);
		float ViewY2 = 0.0f, ItemY = 0.0f;
		ViewW.z = ViewWorld._14 * WorldLocation.x + ViewWorld._24 * WorldLocation.y + ViewWorld._34 * WorldLocation.z + ViewWorld._44;
		if (ViewW.z <= 0.01f) { return false; }
		ViewW.z = 1 / ViewW.z;
		ViewW.x = ScreenWidth + (ViewWorld._11 * WorldLocation.x + ViewWorld._21 * WorldLocation.y + ViewWorld._31 * WorldLocation.z + ViewWorld._41) * ViewW.z * ScreenWidth;
		ViewW.y = ScreenHeight - (ViewWorld._12 * WorldLocation.x + ViewWorld._22 * WorldLocation.y + ViewWorld._32 * (WorldLocation.z - 90.0f) + ViewWorld._42) * ViewW.z * ScreenHeight;
		ViewY2 = ScreenHeight - (ViewWorld._12 * WorldLocation.x + ViewWorld._22 * WorldLocation.y + ViewWorld._32 * (WorldLocation.z + 90.0f) + ViewWorld._42) * ViewW.z * ScreenHeight;
		ItemY = ScreenHeight - (ViewWorld._12 * WorldLocation.x + ViewWorld._22 * WorldLocation.y + ViewWorld._32 * WorldLocation.z + ViewWorld._42) * ViewW.z * ScreenHeight;
		Rect->x = ViewW.x - (ViewY2 - ViewW.y) / 4;
		Rect->y = ViewW.y;
		Rect->w = (ViewY2 - ViewW.y) / 2;
		Rect->h = ViewY2 - ViewW.y;
		return true;
	}

	bool BoneToScreen(const class Vector3& WorldLocation, Vector2* Rect)
	{
		Vector3 ViewW = Vector3(0.0f, 0.0f, 0.0f);
		float ViewY2 = 0.0f, ItemY = 0.0f, Box = 0.0f;
		ViewW.z = ViewWorld._14 * WorldLocation.x + ViewWorld._24 * WorldLocation.y + ViewWorld._34 * WorldLocation.z + ViewWorld._44;
		if (ViewW.z <= 0.01f) { return false; }
		ViewW.z = 1 / ViewW.z;
		ViewW.x = ScreenWidth + (ViewWorld._11 * WorldLocation.x + ViewWorld._21 * WorldLocation.y + ViewWorld._31 * WorldLocation.z + ViewWorld._41) * ViewW.z * ScreenWidth;
		ViewW.y = ScreenHeight - (ViewWorld._12 * WorldLocation.x + ViewWorld._22 * WorldLocation.y + ViewWorld._32 * WorldLocation.z + ViewWorld._42) * ViewW.z * ScreenHeight;
		Rect->x = ViewW.x;
		Rect->y = ViewW.y;
		return true;
	}

	Vector3 GetBoneFTransform(ULONG64 Mesh, const int Id)
	{
		ULONG64 BoneActor = Ram<ULONG64>(Mesh + OFFSET_BonArray);
		FTransform lpFTransform = Ram<FTransform>(BoneActor + Id * 0x30);
		FTransform ComponentToWorld = Ram<FTransform>(Mesh + OFFSET_ComponentToWorld);
		D3DMATRIX2 Matrix = FTransform::MatrixMultiplication(lpFTransform.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());
		return Vector3(Matrix._41, Matrix._42, Matrix._43);
	}


	void DrawBone(ULONG64 Mesh, ImU32 Color)
	{

		static int bonelist[][2]
		{
			{27,9},//脊柱

			{27,109},//左手
			{109,110},
			{110,111},

			{27,64},//右手
			{64,65},
			{65,66},

			{9,17},//左腿
			{17,18},
			{18,19},

			{9,10}, //右腿
			{10,11},
			{11,12}

		};


		Vector2 rect_0 = Vector2(); Vector2 rect_1 = Vector2();

		//骨骼
		for (int i = 0; i < 14; ++i)
		{

			auto Start = GetBoneFTransform(Mesh, bonelist[i][0]);
			auto End = GetBoneFTransform(Mesh, bonelist[i][1]);
			if (BoneToScreen(Start, &rect_0) &&
				BoneToScreen(End, &rect_1))
			{
				if (LineTraceSingle(Ram<Vector3>(CameraManager + OFFSET_CameraPos), Start) ||
					LineTraceSingle(Ram<Vector3>(CameraManager + OFFSET_CameraPos), End)
					)
				{
					DrawLine(rect_0.x, rect_0.y, rect_1.x, rect_1.y, Color, 1.f);
	
				}
				else
				{
					DrawLine(rect_0.x, rect_0.y, rect_1.x, rect_1.y, ImGui::GetColorU32({ 0.f, 1.f, 0.f,1.f }), 1.f);
				}

			

			}

		}


		





	}
	bool LineOfSightTo(ULONG64 PlayerController,ULONG64 Other, Vector3 ViewPoint, bool bAlternateChecks)
	{
		//SCUM.exe+34AB110 - 40 55                 - push rbp
		auto function = reinterpret_cast<bool(__fastcall*)(ULONG64, ULONG64, Vector3, bool)>(Module + 0x34AB110);

		return function(PlayerController, Other, ViewPoint, bAlternateChecks);

	}
	//static bool LineTraceSingle(const UObject* WorldContextObject, const FVector Start, const FVector End, ETraceTypeQuery TraceChannel, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, FHitResult& OutHit, bool bIgnoreSelf, FLinearColor TraceColor = FLinearColor::Red, FLinearColor TraceHitColor = FLinearColor::Green, float DrawTime = 5.0f);

	bool LineTraceSingle(Vector3 Start, Vector3 End)
	{

		struct TArray
		{
			ULONG64 Actor;
			int Num;
			int MaxNum;

		};
	
		auto function = reinterpret_cast<bool(__fastcall*)(
			ULONG64 WorldContextObject,
			Vector3 Start,
			Vector3 End,
			ETraceTypeQuery TraceChannel,
			bool bTraceComplex,
			TArray*,
			EDrawDebugTrace DrawDebugType,
			FHitResult * OutHit,
			bool bIgnoreSelf,
			FLinearColor TraceColor,
			FLinearColor TraceHitColor,
			float DrawTime
			)>(Module + 0x2DC80A0);

		FHitResult lpFHitResult = {};

		return !function(Uworld, Start, End, ETraceTypeQuery::TraceTypeQuery1, true, (TArray*)(Ulever + OFFSET_Actor), EDrawDebugTrace::None, &lpFHitResult, true, FLinearColor(), FLinearColor(), 0.f);


	}

	bool Get()
	{	
		if (!Module)
			Module = (ULONG64)GetModuleHandle(NULL);
		Uworld = Ram<ULONG64>(Module + OFFSET_Uworld);												
		GNames = Module + OFFSET_GNames;															
		Ulever = Ram<ULONG64>(Uworld + OFFSET_Ulever);										
		Actor = Ram<uintptr_t>(Ulever + OFFSET_Actor);										
		Count =Ram<DWORD>(Ulever + OFFSET_Count);												
		ULONG64 GameInstance = Ram<uintptr_t>(Uworld + OFFSET_GameInstance);						
		ULONG64 ULocalPlayer = Ram<uintptr_t>(GameInstance + OFFSET_ULocalPlayer);					
		ULocalPlayer = Ram<uintptr_t>(ULocalPlayer);										
		PlayerController =Ram<uintptr_t>(ULocalPlayer + OFFSET_PlayerController);			
		CameraManager = Ram<uintptr_t>(PlayerController + OFFSET_CameraManager);			
		APawn =Ram<uintptr_t>(PlayerController + OFFSET_APawn);										
		ULONG64 RootComponent =Ram<uintptr_t>(APawn + OFFSET_RootComponent);						
		Matrix = Ram<uintptr_t>(Ram<uintptr_t>(Module + OFFSET_Matrix) + 0x20) + 0x280;

		return true;

	}


	void Draw()
	{

		if (!Get()) return;
		ViewWorld = Ram<_D3DMATRIX2>(Matrix);
		Vector4 Rect = Vector4();
		Vector2 Rect_0 = Vector2();
		//printf("Count:%d \n", Count);
		for (int i = 0; i < Count; i++)
		{
			ULONG64 Object = Ram<ULONG64>(Actor + i * 8);
			DWORD dwObjectId = Ram<ULONG64>(Object + OFFSET_ObjectId);
			const char* ClassName = GetFromFName(dwObjectId);
			ULONG64 ObjectRootComp = Ram<ULONG64>(Object + OFFSET_RootComponent);
			Vector3 ObjectVector = Ram<Vector3>(ObjectRootComp + OFFSET_Vector3d);
			Vector3 LocalVector = Ram<Vector3>(RootComponent + OFFSET_Vector3d);
			float Dist = LocalVector.Distance(ObjectVector) / 100.f;
			printf("第一次打印类名:%s \n", ClassName); //打印ClassName
			if (strcmp(ClassName, "TBP_ElysiumPlayer_C") == 0)//
			{

				printf("第二次打印类名:%s \n", ClassName); //打印ClassName
				DrawBone(Ram<ULONG64>(Object + OFFSET_MeshComponent), ImGui::GetColorU32({ 1.f, 0.f, 0.f,1.f }));//R   G  B  A   /255.f

				if (WorldToScreen(ObjectVector, &Rect))
				{
					char buffer[128] = {};
					sprintf_s(buffer, sizeof(buffer) / sizeof(char), "%llX  %s[%.fm]", Object, ClassName, Dist);
					DrawTextA(Rect.x, Rect.y, ImGui::GetColorU32({ 1.f, 0.f, 0.f,1.f }), buffer);
					DrawRect(Rect.x, Rect.y, Rect.w, Rect.h, ImGui::GetColorU32({ 1.f, 0.f, 0.f,1.f }), 1.f);
				}



			}
			if (Dist > 200.f) continue;
			if (BoneToScreen(ObjectVector, &Rect_0))
			{
				char buffer[128] = {};
				//bool IsVis = LineOfSightTo(PlayerController, Object, Ram<Vector3>(CameraManager + OFFSET_CameraPos), false);
				bool IsVis = TRUE;
				sprintf_s(buffer, sizeof(buffer) / sizeof(char), "%llX  %s[%.fm]", Object, ClassName,Dist);
				DrawTextA(Rect_0.x, Rect_0.y, IsVis ? ImGui::GetColorU32({ 0.f, 1.f, 0.f,1.f }) : ImGui::GetColorU32({ 0.f, 0.f, 1.f,1.f }), buffer);

			}

		}

	}

	//void hookMagicBullet()
	//{
	//	long long gStatus = NULL;
	//	static uint64_t JumpAddress;
	//	static uint64_t HookAddress = (uint64_t)Module + 0x2C38203;
	//	uintptr_t aaa = protectMemory<uintptr_t>(-1, HookAddress, 64);
	//	PthProtectMemory(GameVars.dwProcessId, HookAddress, 20, 64, &gStatus);
	//	Bytes HookB
	//	{
	//		 0x44,0x0F,0x11,0xA7,0xD0,0x01,0x00,0x00,0xB2,0x01,0x44,0x0F,0x11,0xBF,0xE0,0x01,0x00,0x00
	//	};
	//	Bytes HookA{};
	//	JumpAddress = HookAddress + 18;
	//	if (GameVars.magicbullet_addr != 0) {
	//		//WriteBytes(HookAddress, HookB, HookB.Length());
	//		return;
	//	}
	//	UINT64 VirtualAddress = PthAllocateMemory(GameVars.dwProcessId, 10000, 64, &gStatus);
	//	HookA += JMP_FF25(VirtualAddress, 4); // JMP 占用14字节 这里需要18所以 18 - 14
	//	Write<uint64_t>(VirtualAddress + 0x500, 8702406);
	//	HookB += {0x53, 0x52, 0x48, 0x8B, 0x57, 0x18, 0x48, 0x3B, 0x15, 0xE1, 0x04, 0x00, 0x00, 0x75, 0x0E, 0x0F, 0x10, 0x25, 0xD8, 0x03, 0x00, 0x00, 0x0F, 0x11, 0xA7, 0xD0, 0x01, 0x00, 0x00, 0x5A, 0x5B};
	//	HookB += JMP_FF25(JumpAddress, 0);
	//	GameVars.magicbullet_addr = VirtualAddress + 0x400;
	//	printf("VirtualAddress === %lld \n", VirtualAddress);
	//	WriteBytes(VirtualAddress, HookB, HookB.Length());
	//	WriteBytes(HookAddress, HookA, HookA.Length());
	//}

	//void MagicBullet() 
	//{
	//	Write<Vector3>(GameVars.magicbullet_addr, head_pos);
	//}


}