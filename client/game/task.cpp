//----------------------------------------------------------
//
//   SA:CM Multiplayer Modification For GTA:SA
//   Copyright 2004-2006 SA:CM team
//
//----------------------------------------------------------

#include "../main.h"
#include "task.h"

//==========================================================
// GENERIC TASK

CTask::CTask()
{
	m_pTaskType = NULL;
	m_pPlayerPed = NULL;
}

//----------------------------------------------------------

CTask::CTask(DWORD dwSize)
{
	m_pTaskType = NULL;
	m_pPlayerPed = NULL;
	Create(dwSize);
}

//----------------------------------------------------------

CTask::CTask(BYTE *pTaskType)
{
	m_pTaskType = NULL;
	m_pPlayerPed = NULL;
	Create(pTaskType);	
}

//----------------------------------------------------------

CTask::~CTask()
{
}

//----------------------------------------------------------

void CTask::Create(DWORD dwSize)
{
	BYTE *pTaskType;
	__asm
	{
		push dwSize;
		mov eax, 0x61A5A0;	// CTask_New
		call eax;
		add esp, 4;
		mov pTaskType, eax;
	}
	m_bSelfCreated = TRUE;
	m_pTaskType = pTaskType;
}

//----------------------------------------------------------

void CTask::Create(BYTE *pTaskType)
{
	m_bSelfCreated = FALSE;
	m_pTaskType = pTaskType;
}

//----------------------------------------------------------

CTask* CTask::CreateCopy()
{
	BYTE *pTaskType = m_pTaskType;
	__asm
	{
		push ecx;
		mov ecx, pTaskType;
		mov eax, [ecx];
		call [eax+0x4];
		mov pTaskType, eax;
		pop ecx;
	}
	return new CTask(pTaskType);
}

//----------------------------------------------------------

void CTask::Destroy()
{
	if (!IsDestroyed())
	{
		BYTE *pTaskType = m_pTaskType;
		if (m_bSelfCreated)
		{
			__asm
			{
				push ecx;
				mov ecx, pTaskType;
				mov eax, [ecx];
				push 0;		// remove from pool?
				call [eax+0];
				pop ecx;
			}
		}
		else
		{
			__asm
			{
				push ecx;
				mov ecx, pTaskType;
				mov eax, [ecx];
				push 1;		// remove from pool?
				call [eax+0];
				pop ecx;
			}
		}
		m_pTaskType = NULL;
		m_pPlayerPed = NULL;
	}
}

//----------------------------------------------------------

void CTask::ApplyToPed(CActorPed *pPed)
{
	//m_pPlayerPed = pPed;
	PED_TASKS_TYPE *pPedTasksType = pPed->m_pPed->Tasks;
	BYTE *pTaskType = m_pTaskType;

	__asm
	{
		push ecx;

		mov ecx, pPedTasksType;
		push 0;
		push 3;
		push pTaskType;
		add ecx, 4;
		mov eax, 0x681AF0;	
		call eax;			// AddTaskToActor

		pop ecx;
	}
}

void CTask::ApplyToPed(CPlayerPed *pPed)
{
	extern BYTE *pbyteCurrentPlayer;
	m_pPlayerPed = pPed;
	PED_TASKS_TYPE *pPedTasksType = pPed->m_pPed->Tasks;
	BYTE *pTaskType = m_pTaskType;

	*pbyteCurrentPlayer = pPed->m_bytePlayerNumber;

	__asm
	{
		push ecx;
		
		mov ecx, pPedTasksType;
		push 0;
		push 3;
		push pTaskType;
		add ecx, 4;
		mov eax, 0x681AF0;	
		call eax;			// AddTaskToActor

		pop ecx;
	}

	*pbyteCurrentPlayer = 0;


	/*
	(aru) Disabling this messy thing... The one above works just fine for now.

	// Call a default applier here, but can be overloaded for special tasks
	PED_TYPE *pPedType = pPed->m_pPed;
	PED_TASKS_TYPE *pPedTasksType = pPedType->Tasks;
	BYTE *pTaskType = m_pTaskType;
	
	BYTE TaskAppBlock[24];

	DWORD dwSomeSpecialArg = 0;

	__asm
	{
		push ecx;
		push edi;

		push 0;
		push pTaskType;
		push 3;
		lea ecx, TaskAppBlock;
		mov eax, 0x4B0A00;
		call eax;			// Initializes the TaskAppBlock

		lea eax, TaskAppBlock;
		push eax;
		mov eax, pPedTasksType;
		lea ecx, [eax+0x68];
		mov eax, 0x4AB420;
		call eax;			// Does something with CPedIntelligence+0x68

		mov edi, eax;
		test edi, edi;
		jz skipExtended;	// Skip out of the extended stuff

		mov eax, 0x608540;	// This gets something and returns it in eax (3)
		call eax;

		mov ecx, dwSomeSpecialArg;
		push edi;
		push ecx;
		lea ecx, [eax+eax*4];
		push pPedType;
		lea ecx, ds:0xC0B1E8[ecx*4];
		mov eax, 0x608390;
		call eax;			// No idea what this does. (10)

skipExtended:

		lea ecx, TaskAppBlock;
		mov eax, 0x4B0A50;
		call eax;			// No idea what this does either (11)

		pop edi;
		pop ecx;
	}
	*/

}

//----------------------------------------------------------

DWORD CTask::GetID()
{
	BYTE *pTaskType = m_pTaskType;
	DWORD dwID;
	__asm
	{
		push ecx;
		mov ecx, pTaskType;
		mov eax, [ecx];
		call [eax+0x10];
		mov dwID, eax;
		pop ecx;
	}
	return dwID;
}

//----------------------------------------------------------

BOOL CTask::IsDestroyed()
{
	if (m_pTaskType == NULL)
		return TRUE;

	DWORD dwVTbl = *((DWORD*)m_pTaskType);

	if (dwVTbl == 0x86D48C)		// CTask_vtbl
		return TRUE;

	return FALSE;
}

//----------------------------------------------------------

BOOL CTask::IsSimple()
{
	BYTE *pTaskType = m_pTaskType;
	BOOL bRet;
	__asm
	{
		push ecx;
		mov ecx, pTaskType;
		mov eax, [ecx];
		call [eax+0xC];
		movzx eax, al;
		mov bRet, eax;
		pop ecx;
	}
	return bRet;
}

//==========================================================
// JETPACK TASK

CTaskJetpack::CTaskJetpack()
{
	m_pPlayerPed = NULL;
	
	Create(112);

	BYTE *pTaskType = m_pTaskType;
	__asm
	{
		push 0;
		push 0;
		push 0x41200000;		// 10.0f
		push 0;
		mov ecx, pTaskType;
		mov eax, 0x67B4E0;	//  CTaskJetpack_CreateJetpack
		call eax;
	}
}

//----------------------------------------------------------

CTaskJetpack::CTaskJetpack(BYTE *pTaskType)
{
	m_pPlayerPed = NULL;

	Create(pTaskType);
}

//----------------------------------------------------------

CTaskJetpack::~CTaskJetpack()
{
	if (m_pPlayerPed)
		m_pPlayerPed->m_pPed->Tasks->pdwJumpJetPack = NULL;
	CTask::Destroy();
}

//==========================================================
// TAKE_DAMAGE_FALL TASK

CTaskTakeDamageFall::CTaskTakeDamageFall(DWORD dwFallType, DWORD dwNum)
{
	m_pPlayerPed = NULL;

	Create(24);

	BYTE *pTaskType = m_pTaskType;
	__asm
	{
		push dwNum;
		push 0;
		push dwFallType;
		mov ecx, pTaskType;
		mov eax, 0x6786C0;
		call eax;				// CTaskTakeDamageFall_ctor2
	}
}

//==========================================================
// ACTIVATE_GOGGLES

CTaskGoggles::CTaskGoggles()
{
	m_pPlayerPed = NULL;
	Create(12);

	BYTE *pTaskType = m_pTaskType;
	_asm push ecx
	_asm mov ecx, pTaskType
	_asm mov eax, 0x634EF0
	_asm call eax
	_asm pop ecx
}

//-----------------------------------------------------------

CTaskGoToPoint::CTaskGoToPoint(int unk, VECTOR* vecPos, float unk2, int unk3, int unk4 )
{
	m_pPlayerPed = NULL;
	Create(48);

	BYTE* pTaskType = m_pTaskType;
	__asm
	{
		push unk4
		push unk3
		push unk2
		push vecPos
		push unk
		mov ecx, pTaskType
		mov eax, 0x667CD0
		call eax
	}	
}

//-----------------------------------------------------------

CTaskKillPedOnFootArmed::CTaskKillPedOnFootArmed( int unk, int unk2, int unk3, int unk4, PED_TYPE* pToShoot )
{
	m_pPlayerPed = NULL;
	Create(92);

	BYTE* pTaskType = m_pTaskType;
	__asm
	{
		PUSH unk
		PUSH unk3
		PUSH unk2
		PUSH unk4
		push pToShoot
		mov ecx, pTaskType
		mov eax, 0x00621190
		call eax
	}
}

//==========================================================

char szTaskNames[][64] = {
"TASK_SIMPLE_PLAYER_ON_FOOT",
"TASK_SIMPLE_PLAYER_IN_CAR",
"[2]",
"[3]",
"[4]",
"[5]",
"[6]",
"[7]",
"[8]",
"[9]",
"[10]",
"[11]",
"[12]",
"[13]",
"[14]",
"[15]",
"[16]",
"[17]",
"[18]",
"[19]",
"[20]",
"[21]",
"[22]",
"[23]",
"[24]",
"[25]",
"[26]",
"[27]",
"[28]",
"[29]",
"[30]",
"[31]",
"[32]",
"[33]",
"[34]",
"[35]",
"[36]",
"[37]",
"[38]",
"[39]",
"[40]",
"[41]",
"[42]",
"[43]",
"[44]",
"[45]",
"[46]",
"[47]",
"[48]",
"[49]",
"[50]",
"[51]",
"[52]",
"[53]",
"[54]",
"[55]",
"[56]",
"[57]",
"[58]",
"[59]",
"[60]",
"[61]",
"[62]",
"[63]",
"[64]",
"[65]",
"[66]",
"[67]",
"[68]",
"[69]",
"[70]",
"[71]",
"[72]",
"[73]",
"[74]",
"[75]",
"[76]",
"[77]",
"[78]",
"[79]",
"[80]",
"[81]",
"[82]",
"[83]",
"[84]",
"[85]",
"[86]",
"[87]",
"[88]",
"[89]",
"[90]",
"[91]",
"[92]",
"[93]",
"[94]",
"[95]",
"[96]",
"[97]",
"[98]",
"[99]",
"TASK_COMPLEX_MEDIC_TREAT_INJURED_PED",
"TASK_COMPLEX_TREAT_ACCIDENT",
"TASK_SIMPLE_GIVE_CPR",
"TASK_COMPLEX_COP_ARREST_PED",
"TASK_COMPLEX_COP_HASSLE_PED",
"TASK_COMPLEX_HASSLED_BY_COP",
"TASK_COMPLEX_PRESENT_ID_TO_COP",
"TASK_COMPLEX_DRIVE_FIRE_TRUCK",
"TASK_COMPLEX_USE_SWAT_ROPE",
"TASK_COMPLEX_USE_WATER_CANNON",
"TASK_COMPLEX_EXTINGUISH_FIRE_ON_FOOT",
"[111]",
"[112]",
"[113]",
"[114]",
"[115]",
"[116]",
"[117]",
"[118]",
"[119]",
"[120]",
"[121]",
"[122]",
"[123]",
"[124]",
"[125]",
"[126]",
"[127]",
"[128]",
"[129]",
"[130]",
"[131]",
"[132]",
"[133]",
"[134]",
"[135]",
"[136]",
"[137]",
"[138]",
"[139]",
"[140]",
"[141]",
"[142]",
"[143]",
"[144]",
"[145]",
"[146]",
"[147]",
"[148]",
"[149]",
"[150]",
"[151]",
"[152]",
"[153]",
"[154]",
"[155]",
"[156]",
"[157]",
"[158]",
"[159]",
"[160]",
"[161]",
"[162]",
"[163]",
"[164]",
"[165]",
"[166]",
"[167]",
"[168]",
"[169]",
"[170]",
"[171]",
"[172]",
"[173]",
"[174]",
"[175]",
"[176]",
"[177]",
"[178]",
"[179]",
"[180]",
"[181]",
"[182]",
"[183]",
"[184]",
"[185]",
"[186]",
"[187]",
"[188]",
"[189]",
"[190]",
"[191]",
"[192]",
"[193]",
"[194]",
"[195]",
"[196]",
"[197]",
"[198]",
"[199]",
"TASK_NONE",
"TASK_SIMPLE_UNINTERRUPTABLE",
"TASK_SIMPLE_PAUSE",
"TASK_SIMPLE_STAND_STILL",
"TASK_SIMPLE_SET_STAY_IN_SAME_PLACE",
"TASK_SIMPLE_GET_UP",
"TASK_COMPLEX_GET_UP_AND_STAND_STILL",
"TASK_SIMPLE_FALL",
"TASK_COMPLEX_FALL_AND_GET_UP",
"TASK_COMPLEX_FALL_AND_STAY_DOWN",
"TASK_SIMPLE_JUMP",
"TASK_COMPLEX_JUMP",
"TASK_SIMPLE_DIE",
"TASK_SIMPLE_DROWN",
"TASK_SIMPLE_DIE_IN_CAR",
"TASK_COMPLEX_DIE_IN_CAR",
"TASK_SIMPLE_DROWN_IN_CAR",
"TASK_COMPLEX_DIE",
"TASK_SIMPLE_DEAD",
"TASK_SIMPLE_TIRED",
"TASK_SIMPLE_SIT_DOWN",
"TASK_SIMPLE_SIT_IDLE",
"TASK_SIMPLE_STAND_UP",
"TASK_COMPLEX_SIT_DOWN_THEN_IDLE_THEN_STAND_UP",
"TASK_COMPLEX_OBSERVE_TRAFFIC_LIGHTS",
"TASK_COMPLEX_OBSERVE_TRAFFIC_LIGHTS_AND_ACHIEVE_HEADING",
"TASK_NOT_USED",
"TASK_COMPLEX_CROSS_ROAD_LOOK_AND_ACHIEVE_HEADING",
"TASK_SIMPLE_TURN_180",
"TASK_SIMPLE_HAIL_TAXI",
"TASK_COMPLEX_HIT_RESPONSE",
"TASK_COMPLEX_HIT_BY_GUN_RESPONSE",
"TASK_UNUSED_SLOT",
"TASK_COMPLEX_USE_EFFECT",
"TASK_COMPLEX_WAIT_AT_ATTRACTOR",
"TASK_COMPLEX_USE_ATTRACTOR",
"TASK_COMPLEX_WAIT_FOR_DRY_WEATHER",
"TASK_COMPLEX_WAIT_FOR_BUS",
"TASK_SIMPLE_WAIT_FOR_BUS",
"TASK_SIMPLE_WAIT_FOR_PIZZA",
"TASK_COMPLEX_IN_AIR_AND_LAND",
"TASK_SIMPLE_IN_AIR",
"TASK_SIMPLE_LAND",
"TASK_COMPLEX_BE_IN_GROUP",
"TASK_COMPLEX_SEQUENCE",
"TASK_SIMPLE_CALL_FOR_BACKUP",
"TASK_COMPLEX_USE_PAIRED_ATTRACTOR",
"TASK_COMPLEX_USE_ATTRACTOR_PARTNER",
"TASK_COMPLEX_ATTRACTOR_PARTNER_WAIT",
"TASK_COMPLEX_USE_SCRIPTED_ATTRACTOR",
"TASK_COMPLEX_ON_FIRE",
"TASK_SIMPLE_BE_DAMAGED",
"TASK_SIMPLE_TRIGGER_EVENT",
"TASK_SIMPLE_RAGDOLL",
"TASK_SIMPLE_CLIMB",
"TASK_SIMPLE_PLAYER_ON_FIRE",
"TASK_COMPLEX_PARTNER",
"TASK_COMPLEX_STARE_AT_PED",
"TASK_COMPLEX_USE_CLOSEST_FREE_SCRIPTED_ATTRACTOR",
"TASK_COMPLEX_USE_EFFECT_RUNNING",
"TASK_COMPLEX_USE_EFFECT_SPRINTING",
"TASK_COMPLEX_USE_CLOSEST_FREE_SCRIPTED_ATTRACTOR_RUN",
"TASK_COMPLEX_USE_CLOSEST_FREE_SCRIPTED_ATTRACTOR_SPRINT",
"TASK_SIMPLE_CHOKING",
"TASK_SIMPLE_IK_CHAIN",
"TASK_SIMPLE_IK_MANAGER",
"TASK_SIMPLE_IK_LOOK_AT",
"TASK_COMPLEX_CLIMB",
"TASK_COMPLEX_IN_WATER",
"TASK_SIMPLE_TRIGGER_LOOK_AT",
"TASK_SIMPLE_CLEAR_LOOK_AT",
"TASK_SIMPLE_SET_CHAR_DECISION_MAKER",
"TASK_SIMPLE_IK_POINT_R_ARM",
"TASK_SIMPLE_IK_POINT_L_ARM",
"TASK_COMPLEX_BE_STILL",
"TASK_COMPLEX_USE_SEQUENCE",
"TASK_SIMPLE_SET_KINDA_STAY_IN_SAME_PLACE",
"TASK_COMPLEX_FALL_TO_DEATH",
"TASK_WAIT_FOR_MATCHING_LEADER_AREA_CODES",
"[279]",
"[280]",
"[281]",
"[282]",
"[283]",
"[284]",
"[285]",
"[286]",
"[287]",
"[288]",
"[289]",
"[290]",
"[291]",
"[292]",
"[293]",
"[294]",
"[295]",
"[296]",
"[297]",
"[298]",
"[299]",
"TASK_SIMPLE_LOOK_AT_ENTITY_OR_COORD",
"TASK_SIMPLE_SAY",
"TASK_SIMPLE_SHAKE_FIST",
"TASK_SIMPLE_FACIAL",
"TASK_COMPLEX_CHAINED_FACIAL",
"TASK_COMPLEX_FACIAL",
"TASK_SIMPLE_AFFECT_SECONDARY_BEHAVIOUR",
"TASK_SIMPLE_HOLD_ENTITY",
"TASK_SIMPLE_PICKUP_ENTITY",
"TASK_SIMPLE_PUTDOWN_ENTITY",
"TASK_COMPLEX_GO_PICKUP_ENTITY",
"TASK_SIMPLE_DUCK_WHILE_SHOTS_WHIZZING",
"[312]",
"[313]",
"[314]",
"[315]",
"[316]",
"[317]",
"[318]",
"[319]",
"[320]",
"[321]",
"[322]",
"[323]",
"[324]",
"[325]",
"[326]",
"[327]",
"[328]",
"[329]",
"[330]",
"[331]",
"[332]",
"[333]",
"[334]",
"[335]",
"[336]",
"[337]",
"[338]",
"[339]",
"[340]",
"[341]",
"[342]",
"[343]",
"[344]",
"[345]",
"[346]",
"[347]",
"[348]",
"[349]",
"[350]",
"[351]",
"[352]",
"[353]",
"[354]",
"[355]",
"[356]",
"[357]",
"[358]",
"[359]",
"[360]",
"[361]",
"[362]",
"[363]",
"[364]",
"[365]",
"[366]",
"[367]",
"[368]",
"[369]",
"[370]",
"[371]",
"[372]",
"[373]",
"[374]",
"[375]",
"[376]",
"[377]",
"[378]",
"[379]",
"[380]",
"[381]",
"[382]",
"[383]",
"[384]",
"[385]",
"[386]",
"[387]",
"[388]",
"[389]",
"[390]",
"[391]",
"[392]",
"[393]",
"[394]",
"[395]",
"[396]",
"[397]",
"[398]",
"[399]",
"TASK_SIMPLE_ANIM",
"TASK_SIMPLE_NAMED_ANIM",
"TASK_SIMPLE_TIMED_ANIM",
"TASK_SIMPLE_HIT_BACK",
"TASK_SIMPLE_HIT_FRONT",
"TASK_SIMPLE_HIT_LEFT",
"TASK_SIMPLE_HIT_RIGHT",
"TASK_SIMPLE_HIT_BY_GUN_BACK",
"TASK_SIMPLE_HIT_BY_GUN_FRONT",
"TASK_SIMPLE_HIT_BY_GUN_LEFT",
"TASK_SIMPLE_HIT_BY_GUN_RIGHT",
"TASK_SIMPLE_HIT_WALL",
"TASK_SIMPLE_COWER",
"TASK_SIMPLE_HANDS_UP",
"TASK_SIMPLE_HIT_BEHIND",
"TASK_SIMPLE_DUCK",
"TASK_SIMPLE_CHAT",
"TASK_COMPLEX_SUNBATHE",
"TASK_SIMPLE_SUNBATHE",
"TASK_SIMPLE_DETONATE",
"TASK_SIMPLE_USE_ATM",
"TASK_SIMPLE_SCRATCH_HEAD",
"TASK_SIMPLE_LOOK_ABOUT",
"TASK_SIMPLE_ABSEIL",
"TASK_SIMPLE_ANIM_LOOPED_MIDDLE",
"TASK_SIMPLE_HANDSIGNAL_ANIM",
"TASK_COMPLEX_HANDSIGNAL_ANIM",
"TASK_SIMPLE_DUCK_FOREVER",
"TASK_SIMPLE_START_SUNBATHING",
"TASK_SIMPLE_IDLE_SUNBATHING",
"TASK_SIMPLE_STOP_SUNBATHING",
"[431]",
"[432]",
"[433]",
"[434]",
"[435]",
"[436]",
"[437]",
"[438]",
"[439]",
"[440]",
"[441]",
"[442]",
"[443]",
"[444]",
"[445]",
"[446]",
"[447]",
"[448]",
"[449]",
"[450]",
"[451]",
"[452]",
"[453]",
"[454]",
"[455]",
"[456]",
"[457]",
"[458]",
"[459]",
"[460]",
"[461]",
"[462]",
"[463]",
"[464]",
"[465]",
"[466]",
"[467]",
"[468]",
"[469]",
"[470]",
"[471]",
"[472]",
"[473]",
"[474]",
"[475]",
"[476]",
"[477]",
"[478]",
"[479]",
"[480]",
"[481]",
"[482]",
"[483]",
"[484]",
"[485]",
"[486]",
"[487]",
"[488]",
"[489]",
"[490]",
"[491]",
"[492]",
"[493]",
"[494]",
"[495]",
"[496]",
"[497]",
"[498]",
"[499]",
"TASK_SIMPLE_HIT_HEAD",
"TASK_SIMPLE_EVASIVE_STEP",
"TASK_COMPLEX_EVASIVE_STEP",
"TASK_SIMPLE_EVASIVE_DIVE",
"TASK_COMPLEX_EVASIVE_DIVE_AND_GET_UP",
"TASK_COMPLEX_HIT_PED_WITH_CAR",
"TASK_SIMPLE_KILL_PED_WITH_CAR",
"TASK_SIMPLE_HURT_PED_WITH_CAR",
"TASK_COMPLEX_WALK_ROUND_CAR",
"TASK_COMPLEX_WALK_ROUND_BUILDING_ATTEMPT",
"TASK_COMPLEX_WALK_ROUND_OBJECT",
"TASK_COMPLEX_MOVE_BACK_AND_JUMP",
"TASK_COMPLEX_EVASIVE_COWER",
"TASK_COMPLEX_DIVE_FROM_ATTACHED_ENTITY_AND_GET_UP",
"TASK_COMPLEX_WALK_ROUND_FIRE",
"TASK_COMPLEX_STUCK_IN_AIR",
"[516]",
"[517]",
"[518]",
"[519]",
"[520]",
"[521]",
"[522]",
"[523]",
"[524]",
"[525]",
"[526]",
"[527]",
"[528]",
"[529]",
"[530]",
"[531]",
"[532]",
"[533]",
"[534]",
"[535]",
"[536]",
"[537]",
"[538]",
"[539]",
"[540]",
"[541]",
"[542]",
"[543]",
"[544]",
"[545]",
"[546]",
"[547]",
"[548]",
"[549]",
"[550]",
"[551]",
"[552]",
"[553]",
"[554]",
"[555]",
"[556]",
"[557]",
"[558]",
"[559]",
"[560]",
"[561]",
"[562]",
"[563]",
"[564]",
"[565]",
"[566]",
"[567]",
"[568]",
"[569]",
"[570]",
"[571]",
"[572]",
"[573]",
"[574]",
"[575]",
"[576]",
"[577]",
"[578]",
"[579]",
"[580]",
"[581]",
"[582]",
"[583]",
"[584]",
"[585]",
"[586]",
"[587]",
"[588]",
"[589]",
"[590]",
"[591]",
"[592]",
"[593]",
"[594]",
"[595]",
"[596]",
"[597]",
"[598]",
"[599]",
"TASK_COMPLEX_INVESTIGATE_DEAD_PED",
"TASK_COMPLEX_REACT_TO_GUN_AIMED_AT",
"TASK_COMPLEX_WAIT_FOR_BACKUP",
"TASK_COMPLEX_GET_OUT_OF_WAY_OF_CAR",
"TASK_COMPLEX_EXTINGUISH_FIRES",
"[605]",
"[606]",
"[607]",
"[608]",
"[609]",
"[610]",
"[611]",
"[612]",
"[613]",
"[614]",
"[615]",
"[616]",
"[617]",
"[618]",
"[619]",
"[620]",
"[621]",
"[622]",
"[623]",
"[624]",
"[625]",
"[626]",
"[627]",
"[628]",
"[629]",
"[630]",
"[631]",
"[632]",
"[633]",
"[634]",
"[635]",
"[636]",
"[637]",
"[638]",
"[639]",
"[640]",
"[641]",
"[642]",
"[643]",
"[644]",
"[645]",
"[646]",
"[647]",
"[648]",
"[649]",
"[650]",
"[651]",
"[652]",
"[653]",
"[654]",
"[655]",
"[656]",
"[657]",
"[658]",
"[659]",
"[660]",
"[661]",
"[662]",
"[663]",
"[664]",
"[665]",
"[666]",
"[667]",
"[668]",
"[669]",
"[670]",
"[671]",
"[672]",
"[673]",
"[674]",
"[675]",
"[676]",
"[677]",
"[678]",
"[679]",
"[680]",
"[681]",
"[682]",
"[683]",
"[684]",
"[685]",
"[686]",
"[687]",
"[688]",
"[689]",
"[690]",
"[691]",
"[692]",
"[693]",
"[694]",
"[695]",
"[696]",
"[697]",
"[698]",
"[699]",
"TASK_COMPLEX_ENTER_CAR_AS_PASSENGER",
"TASK_COMPLEX_ENTER_CAR_AS_DRIVER",
"TASK_COMPLEX_STEAL_CAR",
"TASK_COMPLEX_DRAG_PED_FROM_CAR",
"TASK_COMPLEX_LEAVE_CAR",
"TASK_COMPLEX_LEAVE_CAR_AND_DIE",
"TASK_COMPLEX_LEAVE_CAR_AND_FLEE",
"TASK_COMPLEX_LEAVE_CAR_AND_WANDER",
"TASK_COMPLEX_SCREAM_IN_CAR_THEN_LEAVE",
"TASK_SIMPLE_CAR_DRIVE",
"TASK_COMPLEX_CAR_DRIVE_TO_POINT",
"TASK_COMPLEX_CAR_DRIVE_WANDER",
"TASK_COMPLEX_ENTER_CAR_AS_PASSENGER_TIMED",
"TASK_COMPLEX_ENTER_CAR_AS_DRIVER_TIMED",
"TASK_COMPLEX_LEAVE_ANY_CAR",
"TASK_COMPLEX_ENTER_BOAT_AS_DRIVER",
"TASK_COMPLEX_LEAVE_BOAT",
"TASK_COMPLEX_ENTER_ANY_CAR_AS_DRIVER",
"TASK_COMPLEX_ENTER_CAR_AS_PASSENGER_WAIT",
"TASK_SIMPLE_CAR_DRIVE_TIMED",
"TASK_COMPLEX_SHUFFLE_SEATS",
"TASK_COMPLEX_CAR_DRIVE_POINT_ROUTE",
"TASK_COMPLEX_CAR_OPEN_DRIVER_DOOR",
"TASK_SIMPLE_CAR_SET_TEMP_ACTION",
"TASK_COMPLEX_CAR_DRIVE_MISSION",
"TASK_COMPLEX_CAR_DRIVE",
"TASK_COMPLEX_CAR_DRIVE_MISSION_FLEE_SCENE",
"TASK_COMPLEX_ENTER_LEADER_CAR_AS_PASSENGER",
"TASK_COMPLEX_CAR_OPEN_PASSENGER_DOOR",
"TASK_COMPLEX_CAR_DRIVE_MISSION_KILL_PED",
"TASK_COMPLEX_LEAVE_CAR_AS_PASSENGER_WAIT",
"[731]",
"[732]",
"[733]",
"[734]",
"[735]",
"[736]",
"[737]",
"[738]",
"[739]",
"[740]",
"[741]",
"[742]",
"[743]",
"[744]",
"[745]",
"[746]",
"[747]",
"[748]",
"[749]",
"[750]",
"[751]",
"[752]",
"[753]",
"[754]",
"[755]",
"[756]",
"[757]",
"[758]",
"[759]",
"[760]",
"[761]",
"[762]",
"[763]",
"[764]",
"[765]",
"[766]",
"[767]",
"[768]",
"[769]",
"[770]",
"[771]",
"[772]",
"[773]",
"[774]",
"[775]",
"[776]",
"[777]",
"[778]",
"[779]",
"[780]",
"[781]",
"[782]",
"[783]",
"[784]",
"[785]",
"[786]",
"[787]",
"[788]",
"[789]",
"[790]",
"[791]",
"[792]",
"[793]",
"[794]",
"[795]",
"[796]",
"[797]",
"[798]",
"[799]",
"TASK_COMPLEX_GO_TO_CAR_DOOR_AND_STAND_STILL",
"TASK_SIMPLE_CAR_ALIGN",
"TASK_SIMPLE_CAR_OPEN_DOOR_FROM_OUTSIDE",
"TASK_SIMPLE_CAR_OPEN_LOCKED_DOOR_FROM_OUTSIDE",
"TASK_SIMPLE_BIKE_PICK_UP",
"TASK_SIMPLE_CAR_CLOSE_DOOR_FROM_INSIDE",
"TASK_SIMPLE_CAR_CLOSE_DOOR_FROM_OUTSIDE",
"TASK_SIMPLE_CAR_GET_IN",
"TASK_SIMPLE_CAR_SHUFFLE",
"TASK_SIMPLE_CAR_WAIT_TO_SLOW_DOWN",
"TASK_SIMPLE_CAR_WAIT_FOR_DOOR_NOT_TO_BE_IN_USE",
"TASK_SIMPLE_CAR_SET_PED_IN_AS_PASSENGER",
"TASK_SIMPLE_CAR_SET_PED_IN_AS_DRIVER",
"TASK_SIMPLE_CAR_GET_OUT",
"TASK_SIMPLE_CAR_JUMP_OUT",
"TASK_SIMPLE_CAR_FORCE_PED_OUT",
"TASK_SIMPLE_CAR_SET_PED_OUT",
"TASK_SIMPLE_CAR_QUICK_DRAG_PED_OUT",
"TASK_SIMPLE_CAR_QUICK_BE_DRAGGED_OUT",
"TASK_SIMPLE_CAR_SET_PED_QUICK_DRAGGED_OUT",
"TASK_SIMPLE_CAR_SLOW_DRAG_PED_OUT",
"TASK_SIMPLE_CAR_SLOW_BE_DRAGGED_OUT",
"TASK_SIMPLE_CAR_SET_PED_SLOW_DRAGGED_OUT",
"TASK_COMPLEX_CAR_SLOW_BE_DRAGGED_OUT",
"TASK_COMPLEX_CAR_SLOW_BE_DRAGGED_OUT_AND_STAND_UP",
"TASK_COMPLEX_CAR_QUICK_BE_DRAGGED_OUT",
"TASK_SIMPLE_BIKE_JACKED",
"TASK_SIMPLE_SET_PED_AS_AUTO_DRIVER",
"TASK_SIMPLE_GO_TO_POINT_NEAR_CAR_DOOR_UNTIL_DOOR_NOT_IN_USE",
"TASK_SIMPLE_WAIT_UNTIL_PED_OUT_CAR",
"TASK_COMPLEX_GO_TO_BOAT_STEERING_WHEEL",
"TASK_COMPLEX_GET_ON_BOAT_SEAT",
"TASK_SIMPLE_CREATE_CAR_AND_GET_IN",
"TASK_SIMPLE_WAIT_UNTIL_PED_IN_CAR",
"TASK_SIMPLE_CAR_FALL_OUT",
"[835]",
"[836]",
"[837]",
"[838]",
"[839]",
"[840]",
"[841]",
"[842]",
"[843]",
"[844]",
"[845]",
"[846]",
"[847]",
"[848]",
"[849]",
"[850]",
"[851]",
"[852]",
"[853]",
"[854]",
"[855]",
"[856]",
"[857]",
"[858]",
"[859]",
"[860]",
"[861]",
"[862]",
"[863]",
"[864]",
"[865]",
"[866]",
"[867]",
"[868]",
"[869]",
"[870]",
"[871]",
"[872]",
"[873]",
"[874]",
"[875]",
"[876]",
"[877]",
"[878]",
"[879]",
"[880]",
"[881]",
"[882]",
"[883]",
"[884]",
"[885]",
"[886]",
"[887]",
"[888]",
"[889]",
"[890]",
"[891]",
"[892]",
"[893]",
"[894]",
"[895]",
"[896]",
"[897]",
"[898]",
"[899]",
"TASK_SIMPLE_GO_TO_POINT",
"TASK_COMPLEX_GO_TO_POINT_SHOOTING",
"TASK_SIMPLE_ACHIEVE_HEADING",
"TASK_COMPLEX_GO_TO_POINT_AND_STAND_STILL",
"TASK_COMPLEX_GO_TO_POINT_AND_STAND_STILL_AND_ACHIEVE_HEADING",
"TASK_COMPLEX_FOLLOW_POINT_ROUTE",
"TASK_COMPLEX_FOLLOW_NODE_ROUTE",
"TASK_COMPLEX_SEEK_ENTITY",
"TASK_COMPLEX_FLEE_POINT",
"TASK_COMPLEX_FLEE_ENTITY",
"TASK_COMPLEX_SMART_FLEE_POINT",
"TASK_COMPLEX_SMART_FLEE_ENTITY",
"TASK_COMPLEX_WANDER",
"TASK_COMPLEX_FOLLOW_LEADER_IN_FORMATION",
"TASK_COMPLEX_FOLLOW_SEXY_PED",
"TASK_COMPLEX_GO_TO_ATTRACTOR",
"TASK_COMPLEX_LEAVE_ATTRACTOR",
"TASK_COMPLEX_AVOID_OTHER_PED_WHILE_WANDERING",
"TASK_COMPLEX_GO_TO_POINT_ANY_MEANS",
"TASK_COMPLEX_WALK_ROUND_SHOP",
"TASK_COMPLEX_TURN_TO_FACE_ENTITY",
"TASK_COMPLEX_AVOID_BUILDING",
"TASK_COMPLEX_SEEK_ENTITY_ANY_MEANS",
"TASK_COMPLEX_FOLLOW_LEADER_ANY_MEANS",
"TASK_COMPLEX_GO_TO_POINT_AIMING",
"TASK_COMPLEX_TRACK_ENTITY",
"TASK_SIMPLE_GO_TO_POINT_FINE",
"TASK_COMPLEX_FLEE_ANY_MEANS",
"TASK_COMPLEX_FLEE_SHOOTING",
"TASK_COMPLEX_SEEK_ENTITY_SHOOTING",
"TASK_UNUSED1",
"TASK_COMPLEX_FOLLOW_PATROL_ROUTE",
"TASK_COMPLEX_GOTO_DOOR_AND_OPEN",
"TASK_COMPLEX_SEEK_ENTITY_AIMING",
"TASK_SIMPLE_SLIDE_TO_COORD",
"TASK_COMPLEX_INVESTIGATE_DISTURBANCE",
"TASK_COMPLEX_FOLLOW_PED_FOOTSTEPS",
"TASK_COMPLEX_FOLLOW_NODE_ROUTE_SHOOTING",
"TASK_COMPLEX_USE_ENTRYEXIT",
"TASK_COMPLEX_AVOID_ENTITY",
"TASK_SMART_FLEE_ENTITY_WALKING",
"[941]",
"[942]",
"[943]",
"[944]",
"[945]",
"[946]",
"[947]",
"[948]",
"[949]",
"[950]",
"[951]",
"[952]",
"[953]",
"[954]",
"[955]",
"[956]",
"[957]",
"[958]",
"[959]",
"[960]",
"[961]",
"[962]",
"[963]",
"[964]",
"[965]",
"[966]",
"[967]",
"[968]",
"[969]",
"[970]",
"[971]",
"[972]",
"[973]",
"[974]",
"[975]",
"[976]",
"[977]",
"[978]",
"[979]",
"[980]",
"[981]",
"[982]",
"[983]",
"[984]",
"[985]",
"[986]",
"[987]",
"[988]",
"[989]",
"[990]",
"[991]",
"[992]",
"[993]",
"[994]",
"[995]",
"[996]",
"[997]",
"[998]",
"[999]",
"TASK_COMPLEX_KILL_PED_ON_FOOT",
"TASK_COMPLEX_KILL_PED_ON_FOOT_MELEE",
"TASK_COMPLEX_KILL_PED_ON_FOOT_ARMED",
"TASK_COMPLEX_DESTROY_CAR",
"TASK_COMPLEX_DESTROY_CAR_MELEE",
"TASK_COMPLEX_DESTROY_CAR_ARMED",
"TASK_COMPLEX_REACT_TO_ATTACK",
"TASK_SIMPLE_BE_KICKED_ON_GROUND",
"TASK_SIMPLE_BE_HIT",
"TASK_SIMPLE_BE_HIT_WHILE_MOVING",
"TASK_COMPLEX_SIDE_STEP_AND_SHOOT",
"TASK_SIMPLE_DRIVEBY_SHOOT",
"TASK_SIMPLE_DRIVEBY_WATCH_FOR_TARGET",
"TASK_COMPLEX_DO_DRIVEBY",
"TASK_KILL_ALL_THREATS",
"TASK_KILL_PED_GROUP_ON_FOOT",
"TASK_SIMPLE_FIGHT",
"TASK_SIMPLE_USE_GUN",
"TASK_SIMPLE_THROW",
"TASK_SIMPLE_FIGHT_CTRL",
"TASK_SIMPLE_GUN_CTRL",
"TASK_SIMPLE_THROW_CTRL",
"TASK_SIMPLE_GANG_DRIVEBY",
"TASK_COMPLEX_KILL_PED_ON_FOOT_TIMED",
"TASK_COMPLEX_KILL_PED_ON_FOOT_STAND_STILL",
"TASK_UNUSED2",
"TASK_KILL_PED_ON_FOOT_WHILE_DUCKING",
"TASK_SIMPLE_STEALTH_KILL",
"TASK_COMPLEX_KILL_PED_ON_FOOT_STEALTH",
"TASK_COMPLEX_KILL_PED_ON_FOOT_KINDA_STAND_STILL",
"TASK_COMPLEX_KILL_PED_AND_REENTER_CAR",
"TASK_COMPLEX_ROAD_RAGE",
"TASK_KILL_PED_FROM_BOAT",
"TASK_SIMPLE_SET_CHAR_IGNORE_WEAPON_RANGE_FLAG",
"TASK_SEEK_COVER_UNTIL_TARGET_DEAD",
"[1035]",
"[1036]",
"[1037]",
"[1038]",
"[1039]",
"[1040]",
"[1041]",
"[1042]",
"[1043]",
"[1044]",
"[1045]",
"[1046]",
"[1047]",
"[1048]",
"[1049]",
"[1050]",
"[1051]",
"[1052]",
"[1053]",
"[1054]",
"[1055]",
"[1056]",
"[1057]",
"[1058]",
"[1059]",
"[1060]",
"[1061]",
"[1062]",
"[1063]",
"[1064]",
"[1065]",
"[1066]",
"[1067]",
"[1068]",
"[1069]",
"[1070]",
"[1071]",
"[1072]",
"[1073]",
"[1074]",
"[1075]",
"[1076]",
"[1077]",
"[1078]",
"[1079]",
"[1080]",
"[1081]",
"[1082]",
"[1083]",
"[1084]",
"[1085]",
"[1086]",
"[1087]",
"[1088]",
"[1089]",
"[1090]",
"[1091]",
"[1092]",
"[1093]",
"[1094]",
"[1095]",
"[1096]",
"[1097]",
"[1098]",
"[1099]",
"TASK_SIMPLE_ARREST_PED",
"TASK_COMPLEX_ARREST_PED",
"TASK_SIMPLE_BE_ARRESTED",
"TASK_COMPLEX_POLICE_PURSUIT",
"TASK_COMPLEX_BE_COP",
"TASK_COMPLEX_KILL_CRIMINAL",
"TASK_COMPLEX_COP_IN_CAR",
"[1107]",
"[1108]",
"[1109]",
"[1110]",
"[1111]",
"[1112]",
"[1113]",
"[1114]",
"[1115]",
"[1116]",
"[1117]",
"[1118]",
"[1119]",
"[1120]",
"[1121]",
"[1122]",
"[1123]",
"[1124]",
"[1125]",
"[1126]",
"[1127]",
"[1128]",
"[1129]",
"[1130]",
"[1131]",
"[1132]",
"[1133]",
"[1134]",
"[1135]",
"[1136]",
"[1137]",
"[1138]",
"[1139]",
"[1140]",
"[1141]",
"[1142]",
"[1143]",
"[1144]",
"[1145]",
"[1146]",
"[1147]",
"[1148]",
"[1149]",
"[1150]",
"[1151]",
"[1152]",
"[1153]",
"[1154]",
"[1155]",
"[1156]",
"[1157]",
"[1158]",
"[1159]",
"[1160]",
"[1161]",
"[1162]",
"[1163]",
"[1164]",
"[1165]",
"[1166]",
"[1167]",
"[1168]",
"[1169]",
"[1170]",
"[1171]",
"[1172]",
"[1173]",
"[1174]",
"[1175]",
"[1176]",
"[1177]",
"[1178]",
"[1179]",
"[1180]",
"[1181]",
"[1182]",
"[1183]",
"[1184]",
"[1185]",
"[1186]",
"[1187]",
"[1188]",
"[1189]",
"[1190]",
"[1191]",
"[1192]",
"[1193]",
"[1194]",
"[1195]",
"[1196]",
"[1197]",
"[1198]",
"[1199]",
"TASK_SIMPLE_INFORM_GROUP",
"TASK_COMPLEX_GANG_LEADER",
"TASK_COMPLEX_PARTNER_DEAL",
"TASK_COMPLEX_PARTNER_GREET",
"TASK_COMPLEX_PARTNER_CHAT",
"TASK_COMPLEX_GANG_HASSLE_VEHICLE",
"TASK_COMPLEX_WALK_WITH_PED",
"TASK_COMPLEX_GANG_FOLLOWER",
"TASK_COMPLEX_WALK_ALONGSIDE_PED",
"TASK_COMPLEX_PARTNER_SHOVE",
"TASK_COMPLEX_SIGNAL_AT_PED",
"TASK_COMPLEX_PASS_OBJECT",
"TASK_COMPLEX_GANG_HASSLE_PED",
"TASK_COMPLEX_WAIT_FOR_PED",
"TASK_SIMPLE_DO_HAND_SIGNAL",
"TASK_COMPLEX_BE_IN_COUPLE",
"TASK_COMPLEX_GOTO_VEHICLE_AND_LEAN",
"TASK_COMPLEX_LEAN_ON_VEHICLE",
"TASK_COMPLEX_CHAT",
"TASK_COMPLEX_GANG_JOIN_RESPOND",
"[1220]",
"[1221]",
"[1222]",
"[1223]",
"[1224]",
"[1225]",
"[1226]",
"[1227]",
"[1228]",
"[1229]",
"[1230]",
"[1231]",
"[1232]",
"[1233]",
"[1234]",
"[1235]",
"[1236]",
"[1237]",
"[1238]",
"[1239]",
"[1240]",
"[1241]",
"[1242]",
"[1243]",
"[1244]",
"[1245]",
"[1246]",
"[1247]",
"[1248]",
"[1249]",
"[1250]",
"[1251]",
"[1252]",
"[1253]",
"[1254]",
"[1255]",
"[1256]",
"[1257]",
"[1258]",
"[1259]",
"[1260]",
"[1261]",
"[1262]",
"[1263]",
"[1264]",
"[1265]",
"[1266]",
"[1267]",
"[1268]",
"[1269]",
"[1270]",
"[1271]",
"[1272]",
"[1273]",
"[1274]",
"[1275]",
"[1276]",
"[1277]",
"[1278]",
"[1279]",
"[1280]",
"[1281]",
"[1282]",
"[1283]",
"[1284]",
"[1285]",
"[1286]",
"[1287]",
"[1288]",
"[1289]",
"[1290]",
"[1291]",
"[1292]",
"[1293]",
"[1294]",
"[1295]",
"[1296]",
"[1297]",
"[1298]",
"[1299]",
"TASK_ZONE_RESPONSE",
"TASK_SIMPLE_TOGGLE_PED_THREAT_SCANNER",
"TASK_FINISHED",
"TASK_SIMPLE_JETPACK",
"TASK_SIMPLE_SWIM",
"TASK_COMPLEX_SWIM_AND_CLIMB_OUT",
"TASK_SIMPLE_DUCK_TOGGLE",
"TASK_WAIT_FOR_MATCHING_AREA_CODES",
"TASK_SIMPLE_ON_ESCALATOR",
"TASK_COMPLEX_PROSTITUTE_SOLICIT",
"[1310]",
"[1311]",
"[1312]",
"[1313]",
"[1314]",
"[1315]",
"[1316]",
"[1317]",
"[1318]",
"[1319]",
"[1320]",
"[1321]",
"[1322]",
"[1323]",
"[1324]",
"[1325]",
"[1326]",
"[1327]",
"[1328]",
"[1329]",
"[1330]",
"[1331]",
"[1332]",
"[1333]",
"[1334]",
"[1335]",
"[1336]",
"[1337]",
"[1338]",
"[1339]",
"[1340]",
"[1341]",
"[1342]",
"[1343]",
"[1344]",
"[1345]",
"[1346]",
"[1347]",
"[1348]",
"[1349]",
"[1350]",
"[1351]",
"[1352]",
"[1353]",
"[1354]",
"[1355]",
"[1356]",
"[1357]",
"[1358]",
"[1359]",
"[1360]",
"[1361]",
"[1362]",
"[1363]",
"[1364]",
"[1365]",
"[1366]",
"[1367]",
"[1368]",
"[1369]",
"[1370]",
"[1371]",
"[1372]",
"[1373]",
"[1374]",
"[1375]",
"[1376]",
"[1377]",
"[1378]",
"[1379]",
"[1380]",
"[1381]",
"[1382]",
"[1383]",
"[1384]",
"[1385]",
"[1386]",
"[1387]",
"[1388]",
"[1389]",
"[1390]",
"[1391]",
"[1392]",
"[1393]",
"[1394]",
"[1395]",
"[1396]",
"[1397]",
"[1398]",
"[1399]",
"TASK_INTERIOR_USE_INFO",
"TASK_INTERIOR_GOTO_INFO",
"TASK_INTERIOR_BE_IN_HOUSE",
"TASK_INTERIOR_BE_IN_OFFICE",
"TASK_INTERIOR_BE_IN_SHOP",
"TASK_INTERIOR_SHOPKEEPER",
"TASK_INTERIOR_LIE_IN_BED",
"TASK_INTERIOR_SIT_ON_CHAIR",
"TASK_INTERIOR_SIT_AT_DESK",
"TASK_INTERIOR_LEAVE",
"TASK_INTERIOR_SIT_IN_RESTAURANT",
"TASK_INTERIOR_RESERVED2",
"TASK_INTERIOR_RESERVED3",
"TASK_INTERIOR_RESERVED4",
"TASK_INTERIOR_RESERVED5",
"TASK_INTERIOR_RESERVED6",
"TASK_INTERIOR_RESERVED7",
"TASK_INTERIOR_RESERVED8",
"[1418]",
"[1419]",
"[1420]",
"[1421]",
"[1422]",
"[1423]",
"[1424]",
"[1425]",
"[1426]",
"[1427]",
"[1428]",
"[1429]",
"[1430]",
"[1431]",
"[1432]",
"[1433]",
"[1434]",
"[1435]",
"[1436]",
"[1437]",
"[1438]",
"[1439]",
"[1440]",
"[1441]",
"[1442]",
"[1443]",
"[1444]",
"[1445]",
"[1446]",
"[1447]",
"[1448]",
"[1449]",
"[1450]",
"[1451]",
"[1452]",
"[1453]",
"[1454]",
"[1455]",
"[1456]",
"[1457]",
"[1458]",
"[1459]",
"[1460]",
"[1461]",
"[1462]",
"[1463]",
"[1464]",
"[1465]",
"[1466]",
"[1467]",
"[1468]",
"[1469]",
"[1470]",
"[1471]",
"[1472]",
"[1473]",
"[1474]",
"[1475]",
"[1476]",
"[1477]",
"[1478]",
"[1479]",
"[1480]",
"[1481]",
"[1482]",
"[1483]",
"[1484]",
"[1485]",
"[1486]",
"[1487]",
"[1488]",
"[1489]",
"[1490]",
"[1491]",
"[1492]",
"[1493]",
"[1494]",
"[1495]",
"[1496]",
"[1497]",
"[1498]",
"[1499]",
"TASK_GROUP_FOLLOW_LEADER_ANY_MEANS",
"TASK_GROUP_FOLLOW_LEADER_WITH_LIMITS",
"TASK_GROUP_KILL_THREATS_BASIC",
"TASK_GROUP_KILL_PLAYER_BASIC",
"TASK_GROUP_STARE_AT_PED",
"TASK_GROUP_FLEE_THREAT",
"TASK_GROUP_PARTNER_DEAL",
"TASK_GROUP_PARTNER_GREET",
"TASK_GROUP_HASSLE_SEXY_PED",
"TASK_GROUP_HASSLE_THREAT",
"TASK_GROUP_USE_MEMBER_DECISION",
"TASK_GROUP_EXIT_CAR",
"TASK_GROUP_ENTER_CAR",
"TASK_GROUP_ENTER_CAR_AND_PERFORM_SEQUENCE",
"TASK_GROUP_RESPOND_TO_LEADER_COMMAND",
"TASK_GROUP_HAND_SIGNAL",
"TASK_GROUP_DRIVEBY",
"TASK_GROUP_HASSLE_THREAT_PASSIVE",
"[1518]",
"[1519]",
"[1520]",
"[1521]",
"[1522]",
"[1523]",
"[1524]",
"[1525]",
"[1526]",
"[1527]",
"[1528]",
"[1529]",
"[1530]",
"[1531]",
"[1532]",
"[1533]",
"[1534]",
"[1535]",
"[1536]",
"[1537]",
"[1538]",
"[1539]",
"[1540]",
"[1541]",
"[1542]",
"[1543]",
"[1544]",
"[1545]",
"[1546]",
"[1547]",
"[1548]",
"[1549]",
"[1550]",
"[1551]",
"[1552]",
"[1553]",
"[1554]",
"[1555]",
"[1556]",
"[1557]",
"[1558]",
"[1559]",
"[1560]",
"[1561]",
"[1562]",
"[1563]",
"[1564]",
"[1565]",
"[1566]",
"[1567]",
"[1568]",
"[1569]",
"[1570]",
"[1571]",
"[1572]",
"[1573]",
"[1574]",
"[1575]",
"[1576]",
"[1577]",
"[1578]",
"[1579]",
"[1580]",
"[1581]",
"[1582]",
"[1583]",
"[1584]",
"[1585]",
"[1586]",
"[1587]",
"[1588]",
"[1589]",
"[1590]",
"[1591]",
"[1592]",
"[1593]",
"[1594]",
"[1595]",
"[1596]",
"[1597]",
"[1598]",
"[1599]",
"TASK_COMPLEX_USE_MOBILE_PHONE",
"TASK_SIMPLE_PHONE_TALK",
"TASK_SIMPLE_PHONE_IN",
"TASK_SIMPLE_PHONE_OUT",
"TASK_COMPLEX_USE_GOGGLES",
"TASK_SIMPLE_GOGGLES_ON",
"TASK_SIMPLE_GOGGLES_OFF",
"[1607]",
"[1608]",
"[1609]",
"[1610]",
"[1611]",
"[1612]",
"[1613]",
"[1614]",
"[1615]",
"[1616]",
"[1617]",
"[1618]",
"[1619]",
"[1620]",
"[1621]",
"[1622]",
"[1623]",
"[1624]",
"[1625]",
"[1626]",
"[1627]",
"[1628]",
"[1629]",
"[1630]",
"[1631]",
"[1632]",
"[1633]",
"[1634]",
"[1635]",
"[1636]",
"[1637]",
"[1638]",
"[1639]",
"[1640]",
"[1641]",
"[1642]",
"[1643]",
"[1644]",
"[1645]",
"[1646]",
"[1647]",
"[1648]",
"[1649]",
"[1650]",
"[1651]",
"[1652]",
"[1653]",
"[1654]",
"[1655]",
"[1656]",
"[1657]",
"[1658]",
"[1659]",
"[1660]",
"[1661]",
"[1662]",
"[1663]",
"[1664]",
"[1665]",
"[1666]",
"[1667]",
"[1668]",
"[1669]",
"[1670]",
"[1671]",
"[1672]",
"[1673]",
"[1674]",
"[1675]",
"[1676]",
"[1677]",
"[1678]",
"[1679]",
"[1680]",
"[1681]",
"[1682]",
"[1683]",
"[1684]",
"[1685]",
"[1686]",
"[1687]",
"[1688]",
"[1689]",
"[1690]",
"[1691]",
"[1692]",
"[1693]",
"[1694]",
"[1695]",
"[1696]",
"[1697]",
"[1698]",
"[1699]",
"TASK_SIMPLE_INFORM_RESPECTED_FRIENDS",
"[1701]",
"[1702]",
"[1703]",
"[1704]",
"[1705]",
"[1706]",
"[1707]",
"[1708]",
"[1709]",
"[1710]",
"[1711]",
"[1712]",
"[1713]",
"[1714]",
"[1715]",
"[1716]",
"[1717]",
"[1718]",
"[1719]",
"[1720]",
"[1721]",
"[1722]",
"[1723]",
"[1724]",
"[1725]",
"[1726]",
"[1727]",
"[1728]",
"[1729]",
"[1730]",
"[1731]",
"[1732]",
"[1733]",
"[1734]",
"[1735]",
"[1736]",
"[1737]",
"[1738]",
"[1739]",
"[1740]",
"[1741]",
"[1742]",
"[1743]",
"[1744]",
"[1745]",
"[1746]",
"[1747]",
"[1748]",
"[1749]",
"[1750]",
"[1751]",
"[1752]",
"[1753]",
"[1754]",
"[1755]",
"[1756]",
"[1757]",
"[1758]",
"[1759]",
"[1760]",
"[1761]",
"[1762]",
"[1763]",
"[1764]",
"[1765]",
"[1766]",
"[1767]",
"[1768]",
"[1769]",
"[1770]",
"[1771]",
"[1772]",
"[1773]",
"[1774]",
"[1775]",
"[1776]",
"[1777]",
"[1778]",
"[1779]",
"[1780]",
"[1781]",
"[1782]",
"[1783]",
"[1784]",
"[1785]",
"[1786]",
"[1787]",
"[1788]",
"[1789]",
"[1790]",
"[1791]",
"[1792]",
"[1793]",
"[1794]",
"[1795]",
"[1796]",
"[1797]",
"[1798]",
"[1799]",
"TASK_COMPLEX_USE_SCRIPTED_BRAIN" };
