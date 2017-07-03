#pragma once

#define SACM_VERSION											"0.1a"
#define NETGAME_VERSION											0x5FB61AE1

#define RPC_ServerJoin											"\x01"
#define RPC_ServerQuit											"\x02"
#define RPC_InitGame											"\x03"
#define RPC_VehicleSpawn										"\x04"
#define RPC_VehicleDestroy										"\x05"
#define RPC_SetCheckpoint										"\x06"
#define RPC_DisableCheckpoint									"\x07"
#define RPC_SetRaceCheckpoint									"\x08"
#define RPC_DisableRaceCheckpoint								"\x09"
#define RPC_GameModeRestart										"\x0A"
#define RPC_ConnectionRejected									"\x0B"
#define RPC_ClientMessage										"\x0C"
#define RPC_WorldTime											"\x0D"
#define RPC_Pickup												"\x0E"
#define RPC_DestroyPickup										"\x0F"
#define RPC_DestroyWeaponPickup									"\x10"
#define RPC_Weather												"\x11"
#define RPC_Instagib											"\x12"
#define RPC_SetTimeEx											"\x13"
#define RPC_ToggleClock											"\x14"
#define RPC_Chat												"\x15"
#define RPC_RequestClass										"\x16"
#define RPC_RequestSpawn										"\x17"
#define RPC_Spawn												"\x18"
#define RPC_Death												"\x19"
#define RPC_EnterVehicle										"\x1A"
#define RPC_ExitVehicle											"\x1B"
#define RPC_UpdateScoresPingsIPs								"\x1C"
#define RPC_SvrStats											"\x1D"
#define RPC_ScmEvent											"\x1E"

#define RPC_ScrSetSpawnInfo										"\x1F"
#define RPC_ScrSetPlayerTeam									"\x20"
#define RPC_ScrSetPlayerSkin									"\x21"
#define RPC_ScrSetPlayerName									"\x22"
#define RPC_ScrSetPlayerPos										"\x23"
#define RPC_ScrSetPlayerPosFindZ								"\x24"
#define RPC_ScrSetPlayerHealth									"\x25"
#define RPC_ScrPutPlayerInVehicle								"\x26"
#define RPC_ScrRemovePlayerFromVehicle							"\x27"
#define RPC_ScrSetPlayerColor									"\x28"
#define RPC_ScrDisplayGameText									"\x29"
#define RPC_ScrSetInterior										"\x2A"
#define RPC_ScrSetCameraPos										"\x2B"
#define RPC_ScrSetCameraLookAt									"\x2C"
#define RPC_ScrSetVehiclePos									"\x2D"
#define RPC_ScrSetVehicleZAngle									"\x2E"
#define RPC_ScrVehicleParams									"\x2F"
#define RPC_ScrSetCameraBehindPlayer							"\x30"
#define RPC_ScrTogglePlayerControllable							"\x31"
#define RPC_ScrPlaySound										"\x32"
#define RPC_ScrSetWorldBounds									"\x33"
#define RPC_ScrHaveSomeMoney									"\x34"
#define RPC_ScrSetPlayerFacingAngle								"\x35"
#define RPC_ScrResetMoney										"\x36"
#define RPC_ScrResetPlayerWeapons								"\x37"
#define RPC_ScrGivePlayerWeapon									"\x38"
#define RPC_ScrRespawnVehicle									"\x39"
#define RPC_ScrLinkVehicle										"\x3A"
#define RPC_ScrSetPlayerArmour									"\x3B"
#define RPC_ScrDeathMessage										"\x3C"
#define RPC_ScrSetMapIcon										"\x3D"
#define RPC_ScrDisableMapIcon									"\x3E"
#define RPC_ScrSetWeaponAmmo									"\x3F"
#define RPC_ScrSetGravity										"\x40"
#define RPC_ScrSetVehicleHealth									"\x41"
#define RPC_ScrAttachTrailerToVehicle							"\x42"
#define RPC_ScrDetachTrailerFromVehicle							"\x43"
#define RPC_ScrCreateObject										"\x44"
#define RPC_ScrSetObjectPos										"\x45"
#define RPC_ScrSetObjectRotation								"\x46"
#define RPC_ScrDestroyObject									"\x47"
#define RPC_ScrSetPlayerVirtualWorld							"\x48"
#define RPC_ScrSetVehicleVirtualWorld							"\x49"
#define RPC_ScrCreateExplosion									"\x4A"
#define RPC_ScrShowNameTag										"\x4B"
#define RPC_ScrMoveObject										"\x4C"
#define RPC_ScrStopObject										"\x4D"
#define RPC_ScrNumberPlate										"\x4E"
#define RPC_ScrTogglePlayerSpectating							"\x4F"
#define RPC_ScrSetPlayerSpectating								"\x50"
#define RPC_ScrPlayerSpectatePlayer								"\x51"
#define RPC_ScrPlayerSpectateVehicle							"\x52"
#define RPC_ScrRemoveComponent									"\x53"
#define RPC_ScrForceSpawnSelection								"\x54"
#define RPC_ScrAttachObjectToPlayer								"\x55"
#define RPC_ScrInitMenu											"\x56"
#define RPC_ScrShowMenu											"\x57"
#define RPC_ScrHideMenu											"\x58"
#define RPC_ScrSetPlayerWantedLevel								"\x59"
#define RPC_ScrShowTextDraw										"\x5A"
#define RPC_ScrHideTextDraw										"\x5B"
#define RPC_ScrEditTextDraw										"\x5C"
#define RPC_ScrAddGangZone										"\x5D"
#define RPC_ScrRemoveGangZone									"\x5E"
#define RPC_ScrFlashGangZone									"\x5F"
#define RPC_ScrStopFlashGangZone								"\x60"
#define RPC_ScrApplyAnimation									"\x61"
#define RPC_ScrClearAnimations									"\x62"
#define RPC_ScrSetSpecialAction									"\x63"
#define RPC_ScrEnableStuntBonus									"\x64"
#define RPC_ScrSetFightingStyle									"\x65"
#define RPC_ScrSetPlayerVelocity								"\x66"
#define RPC_ScrSetVehicleVelocity								"\x67"
#define RPC_ScrCreateActor										"\x68"
#define RPC_ScrDestroyActor										"\x69"
#define RPC_ScrMoveActorTo										"\x6A"
#define RPC_ScrActorKillPlayer									"\x6B"
#define RPC_ScrActorEnterVehicle								"\x6C"
#define RPC_ScrActorExitVehicle									"\x6D"
#define RPC_ScrActorDriveVehicleToPoint							"\x6E"
#define RPC_ScrSetActorPos										"\x6F"
#define RPC_ScrSetVehicleTireStatus								"\x70"
#define RPC_ScrSetPlayerDrunkVisuals							"\x71"
#define RPC_ScrSetPlayerDrunkHandling							"\x72"

#define RPC_ClientJoin											"\x73"
#define RPC_ServerCommand										"\x74"
#define RPC_SetInteriorId										"\x75"
#define RPC_AdminMapTeleport									"\x76"
#define RPC_VehicleDestroyed									"\x77"
#define RPC_PickedUpWeapon										"\x78"
#define RPC_PickedUpPickup										"\x79"
#define RPC_MenuSelect											"\x7A"
#define RPC_MenuQuit											"\x7B"
#define RPC_RconConnect											"\x7C"
#define RPC_RconCommand											"\x7D"
#define RPC_RconEvent											"\x7E"
#define RPC_RconPlayerInfo										"\x7F"
#define RPC_DamageVehicle										"\x80"
#define RPC_ScriptCash											"\x81"
#define RPC_WorldPlayerAdd										"\x82"
#define RPC_WorldPlayerDeath									"\x83"
#define RPC_WorldPlayerRemove									"\x84"
#define RPC_WorldVehicleAdd										"\x85"
#define RPC_WorldVehicleRemove									"\x86"

typedef unsigned short SACMVEHICLE;
typedef unsigned char  SACMACTOR;
typedef unsigned short SACMPLAYER;
typedef unsigned short SACMOBJECT;

#pragma pack(push, 1)
typedef struct _C_VECTOR1 {
	char data[9];
} C_VECTOR1;

typedef struct _C_VECTOR2 {
	short X, Y, Z;
} C_VECTOR2;

typedef struct _RGBA {
	unsigned char r, g, b, a;
} RGBA, *PRGBA;

typedef struct _VECTOR
{
	float X, Y, Z;
} VECTOR, *PVECTOR;

typedef struct _VECTOR2D {
	float X, Y;
} VECTOR2D, *PVECTOR2D;

typedef struct _MATRIX4X4 {
	VECTOR right;
	DWORD  flags;
	VECTOR up;
	float  pad_u;
	VECTOR at;
	float  pad_a;
	VECTOR pos;
	float  pad_p;
} MATRIX4X4, *PMATRIX4X4;

typedef struct _PLAYER_SPAWN_INFO
{
	BYTE byteTeam;
	int iSkin;
	VECTOR vecPos;
	float fRotation;
	int iSpawnWeapons[3];
	int iSpawnWeaponsAmmo[3];
} PLAYER_SPAWN_INFO;

typedef struct _ONFOOT_SYNC_DATA
{
	WORD lrAnalog;
	WORD udAnalog;
	WORD wKeys;
	VECTOR vecPos;
	float fRotation;
	BYTE byteHealth;
	BYTE byteArmour;
	BYTE byteCurrentWeapon;
	BYTE byteSpecialAction;
	VECTOR vecMoveSpeed;
	BOOL bSurfing;
	SACMVEHICLE surfVehicleID;
	VECTOR vecSurfSpeed;
	VECTOR vecSurfTurn;
	MATRIX4X4 matSurf;
} ONFOOT_SYNC_DATA;

typedef struct _AIM_SYNC_DATA
{
	BYTE byteCamMode;
	BYTE byteCamExtZoom : 6;	// 0-63 normalized
	BYTE byteWeaponState : 2;	// see eWeaponState
	VECTOR vecAimf1;
	VECTOR vecAimf2;
	VECTOR vecAimPos;
	float fAimZ;
} AIM_SYNC_DATA;

typedef struct _INCAR_SYNC_DATA
{
	SACMVEHICLE VehicleID;
	WORD lrAnalog;
	WORD udAnalog;
	WORD wKeys;
	C_VECTOR1 cvecRoll;
	C_VECTOR1 cvecDirection;
	VECTOR vecPos;
	VECTOR vecMoveSpeed;
	float fCarHealth;
	BYTE bytePlayerHealth;
	BYTE bytePlayerArmour;
	BYTE byteCurrentWeapon;
	BYTE byteSirenOn;
	BYTE byteLandingGearState;
	BYTE byteTires[4];
	SACMVEHICLE TrailerID;
	DWORD dwHydraThrustAngle;
	FLOAT fTrainSpeed;
} INCAR_SYNC_DATA;

typedef struct _PASSENGER_SYNC_DATA
{
	SACMVEHICLE VehicleID;
	BYTE byteSeatFlags : 7;
	BYTE byteDriveBy : 1;
	BYTE byteCurrentWeapon;
	BYTE bytePlayerHealth;
	BYTE bytePlayerArmour;
	WORD lrAnalog;
	WORD udAnalog;
	WORD wKeys;
	VECTOR vecPos;
} PASSENGER_SYNC_DATA;

typedef struct _SPECTATOR_SYNC_DATA
{
	WORD lrAnalog;
	WORD udAnalog;
	WORD wKeys;
	VECTOR vecPos;
} SPECTATOR_SYNC_DATA;

typedef struct _TRAILER_SYNC_DATA
{
	C_VECTOR1 cvecRoll;
	C_VECTOR1 cvecDirection;
	VECTOR vecPos;
	VECTOR vecMoveSpeed;
} TRAILER_SYNC_DATA;

typedef struct _UNOCCUPIED_SYNC_DATA
{
	SACMVEHICLE VehicleID;
	C_VECTOR1 cvecRoll;
	C_VECTOR1 cvecDirection;
	VECTOR vecPos;
	VECTOR vecMoveSpeed;
	VECTOR vecTurnSpeed;
	float fHealth;
	DWORD dwPanelStatus;
	DWORD dwDoorStatus;
	BYTE byteLightStatus;
} UNOCCUPIED_SYNC_DATA;

typedef struct _CAR_MOD_INFO
{
	BYTE byteCarMod[14];
	BYTE bytePaintJob;
	int iColor0;
	int iColor1;
} CAR_MOD_INFO;

typedef struct _VEHICLE_SPAWN_INFO {
	SACMVEHICLE VehicleId;
	int		  iVehicleType;
	VECTOR	  vecPos;
	float	  fRotation;
	char	  aColor1;
	char	  aColor2;
	float	  fHealth;
	BYTE	  byteInterior;
	BYTE	  byteDoorsLocked;
	DWORD	  dwDoorDamageStatus;
	DWORD	  dwPanelDamageStatus;
	BYTE	  byteLightDamageStatus;
} VEHICLE_SPAWN_INFO;
#pragma pack(pop)