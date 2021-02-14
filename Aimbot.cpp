
using namespace std;
void VectorAngles(const float *forward, float *angles)
{
	float	tmp, yaw, pitch;

	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if (forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / 3.14f);
		if (yaw < 0)
			yaw += 360;

		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / 3.14f);
		if (pitch < 0)
			pitch += 360;
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}


Vector Aimbot::FindSmoothAngels(Vector Target)
{
	float angel;
	float angelhelper;
	Vector SmoothAimAngles;

	Vector SmoothMy;
	SmoothMy.x = Viewangles.x + 180;
	SmoothMy.y = Viewangles.y + 180;

	Vector SmoothEnm;
	SmoothEnm.x = Target.x + 180;
	SmoothEnm.y = Target.y + 180;

	angel = SmoothEnm.x - SmoothMy.x;
	angelhelper = angel / CM.AimbotSMOOTH;
	SmoothAimAngles.x = Viewangles.x + angelhelper;

	angel = SmoothEnm.y - SmoothMy.y;
	angelhelper = angel / CM.AimbotSMOOTH;
	SmoothAimAngles.y = Viewangles.y + angelhelper;
	
	return SmoothAimAngles;
}
Vector Aimbot::calangle(Vector src, Vector dir, float xp, float yp)
{
	float output[3];
	float input[3] = { dir.x - src.x , dir.y - src.y, dir.z - src.z };
	VectorAngles(input, output);
	if (output[0] > 180) output[0] -= 360;
	if (output[0] < -180) output[0] += 360;
	if (output[1] > 180) output[1] -= 360;
	if (output[1] < -180) output[1] += 360;
	return { output[0] - xp * 2.f, output[1] - yp * 2.f, 0.f };
}
int Aimbot::FindClosestEnemy()
{
	float Finish;
	int ClosestEntity = 1;
	Vector Calc = { 0 ,0 ,0 };
	float Closest = FLT_MAX;
	for (int i = 1; i < 25; i++)
	{
		Vector ViewCalc = Viewangles;
		DWORD Entity = Read.GetEntity(i);
		int EnmTeam = Read.GetEntityTeam(Entity); if (EnmTeam == Read.Team) continue;
		int Enmhealth = Read.GetEntityHealth(Entity);  if (Enmhealth < 1 || Enmhealth > 100) continue;
		int Dormant = RPM<int>(Entity + O.m_bDormant);

		if (Dormant == 0) 
		{
			Vector EnmLoc = RPM<Vector>(Entity + O.m_vecOrigin);
			Vector Target = calangle(MyLoc, EnmLoc, 0, 0);
			ViewCalc.y += 180;
			Target.y += 180;
			Calc.y = Target.y - ViewCalc.y;
			if (Calc.y < 0) Calc.y = -Calc.y;
			ViewCalc.x += 180;
			Target.x += 180;
			Calc.x = Target.x - ViewCalc.x;
			if (Calc.x < 0) Calc.x = -Calc.x;

			Finish = Calc.y + Calc.x;

			if (Finish < Closest)
			{
				Closest = Finish;
				ClosestEntity = i;
			}
		}
	}
	return ClosestEntity;
}
void Aimbot::ReadMemory(int bone)
{
	MyLoc = RPM<Vector>(Read.BasePointer + O.m_vecOrigin);
	Vector Origin = RPM<Vector>(Read.BasePointer + O.m_vecViewOffset);

	MyEyeLoc.x = MyLoc.x + Origin.x;
	MyEyeLoc.y = MyLoc.y + Origin.y;
	MyEyeLoc.z = MyLoc.z + Origin.z;

	Clientstate = RPM<DWORD>(Init.EngineDLL + O.dwClientState);
	Viewangles = RPM<Vector>(Clientstate + O.dwClientState_ViewAngles);
	int EntNum = FindClosestEnemy();
	DWORD Entity = Read.GetEntity(EntNum);
	Enemyhealth = Read.GetEntityHealth(Entity); 
	Enemyteam = Read.GetEntityTeam(Entity);
	DWORD Bonebase = RPM<DWORD>(Entity + O.m_dwBoneMatrix);

	HeadBonesEnemy.x = RPM<float>(Bonebase + 0x30 * bone + 0x0C);
	HeadBonesEnemy.y = RPM<float>(Bonebase + 0x30 * bone + 0x1C);
	HeadBonesEnemy.z = RPM<float>(Bonebase + 0x30 * bone + 0x2C);
}
int Aimbot::GetWeaponInHand()
{
	int CurWeaponID = RPM<int>(Read.BasePointer + O.m_hActiveWeapon);
	CurWeaponID &= 0xFFF;
	DWORD WeaponEntity = RPM<DWORD>((Init.ClientDLL + O.dwEntityList + (CurWeaponID - 1) * 16));
	int WeaponIndex = RPM<int>(WeaponEntity + O.m_iItemDefinitionIndex);
	if (WeaponIndex == WEAPON_DECOY || WeaponIndex == WEAPON_FLASHBANG || WeaponIndex == WEAPON_SMOKE || WeaponIndex == WEAPON_MOLOTOV || WeaponIndex == WEAPON_HEGRENADE || WeaponIndex == WEAPON_INCENDARY)                                                                                           return IS_GRENADE;
	else if (WeaponIndex == WEAPON_M4A4 || WeaponIndex == WEAPON_FAMAS || WeaponIndex == WEAPON_GALIL || WeaponIndex == WEAPON_AK47)                                                                                                                                                                    return IS_RIFLE;
	else if (WeaponIndex == WEAPON_USPS || WeaponIndex == WEAPON_P2000 || WeaponIndex == WEAPON_P250 || WeaponIndex == WEAPON_GLOCK || WeaponIndex == WEAPON_FIVE7 || WeaponIndex == WEAPON_DEAGLE || WeaponIndex == WEAPON_DUALS || WeaponIndex == WEAPON_TEC9 || WeaponIndex == WEAPON_CZ75)return IS_PISTOL;
	else if (WeaponIndex == WEAPON_AWP || WeaponIndex == WEAPON_SSG08)                                                                                                                                                                                                                                        return IS_SNIPER;
	else if (WeaponIndex == WEAPON_XM || WeaponIndex == WEAPON_MAG7 || WeaponIndex == WEAPON_SAWEDOFF || WeaponIndex == WEAPON_NOVA)                                                                                                                                                                    return IS_SHOTGUN;
	else if (WeaponIndex > 100)                                                                                                                                                                                                                                                                               return IS_UNKNOWN;
	else                                                                                                                                                                                                                                                                                                       return IS_SMG;
}
void Aimbot::WriteMemory()
{
	if (Enemyhealth > 0 && Enemyhealth <= 100 && Read.Team != Enemyteam &&  GetWeaponInHand() != IS_GRENADE) // != to ==
	{
		Vector Punch;
		if (GetWeaponInHand() == IS_RIFLE || GetWeaponInHand() == IS_SMG)
		{
			Punch = RPM<Vector>(Read.BasePointer + O.m_aimPunchAngle);
		}
		else Punch = { 0,0 };
		Vector Target = calangle(MyEyeLoc, HeadBonesEnemy, Punch.x, Punch.y);
		if ((Viewangles.x - Target.x) <= CM.AimbotFOV && (Viewangles.x - Target.x) >= -CM.AimbotFOV && (Viewangles.y - Target.y) <= CM.AimbotFOV && (Viewangles.y - Target.y) >= -CM.AimbotFOV)
		{
			Target = FindSmoothAngels(Target);
			WPM<float>(Clientstate + O.dwClientState_ViewAngles, Target.x);
			WPM<float>(Clientstate + O.dwClientState_ViewAngles + 0x4, Target.y);
		}
	}
}
void Aimbot::Run()
{
	if (CM.WantAim == 1 && GetAsyncKeyState(VK_LBUTTON))
	{
		ReadMemory(CM.Bonetype);
		WriteMemory();
	}
}
class Vector
{
public:
	float x, y, z;
};

class Aimbot
{
private:

	void SilentWriteMemory(Vector Target); //removed cause you should not paste
	void BackTrackEx(int ticks); //removed cause pasting is bad af
	Vector FindSmoothAngels(Vector Target);
	Vector calangle(Vector src, Vector dir, float xp, float yp);
	int FindClosestEnemy();
	void ReadMemory(int bone);
	int GetWeaponInHand();
	void WriteMemory();
	Vector Viewangles;
	int Enemyteam;
	DWORD Clientstate;
	int Enemyhealth;
	Vector HeadBonesEnemy;
	Vector MyLoc;
	Vector MyEyeLoc;
public:
	void Run();
}; extern Aimbot Am;

#define IS_GRENADE 1
#define IS_RIFLE 2
#define IS_SNIPER 3
#define IS_PISTOL 4
#define IS_SHOTGUN 6
#define IS_SMG 5
#define IS_UNKNOWN 7
#define WEAPON_AK47 7
#define WEAPON_M4A4 16
#define WEAPON_M4A1S  60
#define WEAPON_P2000 32
#define WEAPON_FIVE7 3
#define WEAPON_DEAGLE 1
#define WEAPON_USPS 61
#define WEAPON_P250 36
#define WEAPON_DUALS 2
#define WEAPON_GLOCK 4
#define WEAPON_TEC9 30
#define WEAPON_SSG08 40
#define WEAPON_MP9 34
#define WEAPON_MP7 33
#define WEAPON_NOVA 35
#define WEAPON_XM 25
#define WEAPON_SAWEDOFF 29
#define WEAPON_MAG7 27
#define WEAPON_GALIL 13
#define WEAPON_NEGEV 28
#define WEAPON_P90 19
#define WEAPON_MAC10 17
#define WEAPON_UMP 24
#define WEAPON_AWP 9
#define WEAPON_FAMAS 10
#define WEAPON_CZ75 63
#define WEAPON_DECOY 47
#define WEAPON_SMOKE 45
#define WEAPON_HEGRENADE 44
#define WEAPON_MOLOTOV 46
#define WEAPON_INCENDARY 48
#define WEAPON_FLASHBANG 43