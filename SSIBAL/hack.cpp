#include "hack.hpp"

#define DegToRad(val1) ((val1 * PI) / 180)  

// xhair���� ���Ǵ� �Ÿ��� �����Ϸ��� �� ���� ����
float fov = 10;
float PI = 3.14159265358f;

DWORD Client = (DWORD)GetModuleHandleA("client.dll");
DWORD Engine = (DWORD)GetModuleHandleA("engine.dll");
DWORD LocalPlayer = *(DWORD*)(Client + dwLocalPlayer);
DWORD GlowObj = *(DWORD*)(Client + dwGlowObjectManager);
DWORD EntityList = *(DWORD*)(Client + dwEntityList);
int myTeam = *(int*)(LocalPlayer + m_iTeamNum);
vec3* viewAngles = (vec3*)(*(uintptr_t*)(Engine + dwClientState) + dwClientState_ViewAngles);

int closestEnt = 99999;
uintptr_t* bestEnt = nullptr;

vec3 CalculateAngle(vec3 src, vec3 dst) {
	// dst���� src ��ǥ�� ���� ���� ������ ���� �÷��̾ ����(0, 0, 0)���� ����ϱ� ���ؼ���
	vec3 angle, delta = { dst.x - src.x, dst.y - src.y, dst.z - src.z };
	float hyp = sqrt(delta.x * delta.x + delta.y * delta.y);

	angle.x = atan2(-delta.z, hyp) * 180 / PI;
	angle.y = atan2(delta.y, delta.x) * 180 / PI;
	angle.z = 0.0f;

	return angle;
}

vec3 CalcAngle(const vec3& src, const vec3& dst) {
	// dst���� src ��ǥ�� ���� ���� ������ ���� �÷��̾ ����(0, 0, 0)���� ����ϱ� ���ؼ���
	vec3 angle;
	vec3 delta = { (dst.x - src.x), (dst.y - src.y), (dst.z - src.z) };
	// ���� C(hyp)�� ���ϴ� ���� 
	// c2 = a2 + b2 -> hyp = ��Ʈ(a2 + b2); 
	float hyp = sqrt(delta.x * delta.x + delta.y * delta.y);

	// 180.0 / PI�� �ϴ� ������ ���� ���� 60�й�(��)���� �ٲٴ� �뵵
	angle.x = -asin(delta.z / hyp) * (180.f / PI); // Pitch
	angle.y = atan2(delta.y, delta.x) * (180.f / PI); // Yaw, opp / adj
	angle.z = 0.0f;
	//std::cout << angle.x << std::endl;
	if (delta.x >= 0.0f) angle.y += 180.0f;

	return angle;
}

vec3 getBone(const DWORD boneaddy, const int id) {
	vec3 bone;
	bone.x = *(float*)((boneaddy + 0x30 * id + 0x0C));
	bone.y = *(float*)((boneaddy + 0x30 * id + 0x1C));
	bone.z = *(float*)((boneaddy + 0x30 * id + 0x2C));
	return bone;
}


namespace Hack
{
	void Glow()
	{
		if (Config::bGlow)
		{
			for (int i = 1; i < 64; ++i)
			{
				DWORD Entity = *(DWORD*)((Client + dwEntityList) + i * 0x10);
				if (Entity == NULL) continue;

				int GlowIndex = *(int*)(Entity + m_iGlowIndex);
				int EntityTeam = *(int*)(Entity + m_iTeamNum);

				if (EntityTeam == myTeam)
				{
					// LocalTeam
					*(float*)(GlowObj + (GlowIndex * 0x38) + 0x4) = 0.f;  // R
					*(float*)(GlowObj + (GlowIndex * 0x38) + 0x8) = 1.f;  // G
					*(float*)(GlowObj + (GlowIndex * 0x38) + 0xC) = 0.f;  // B
					*(float*)(GlowObj + (GlowIndex * 0x38) + 0x10) = 1.f;// A
				}
				else
				{
					*(float*)(GlowObj + (GlowIndex * 0x38) + 0x4) = 1.f;  // R
					*(float*)(GlowObj + (GlowIndex * 0x38) + 0x8) = 0.f;  // G
					*(float*)(GlowObj + (GlowIndex * 0x38) + 0xC) = 0.f;  // B
					*(float*)(GlowObj + (GlowIndex * 0x38) + 0x10) = 1.f;// A
				}
				*(bool*)(GlowObj + (GlowIndex * 0x38) + 0x24) = true;
				*(bool*)(GlowObj + (GlowIndex * 0x38) + 0x25) = false;
			}
		}
	}

	void Aimbot()
	{
		if(Config::bTestAimbot)
		{
			for (int i = 1; i <= 32; i++) { // ent loop
				
				// 0x4D9EAE4 - dwEntityList
				// Get the Current Entity
				uintptr_t* curEnt = (uintptr_t*)(Client + dwEntityList + (i * 0x10));
				//vec3 enHeadPos = getBone(*curEnt + m_dwBoneMatrix, 8);
				// entity�� �����ϴ��� Ȯ��
				if (curEnt != nullptr && *(uintptr_t*)curEnt != NULL ) {
					// 0x138 - vecOrigin
					//vec3 tempAngles = CalcAngle(*(vec3*)(*localPlayer + 0x138), *(vec3*)(*curEnt + 0x138));
					vec3 tempAngles = CalculateAngle(*(vec3*)(LocalPlayer + m_vecOrigin), *(vec3*)(*curEnt + m_vecOrigin));

					// �÷��̾��� fov ȹ��
					// ��ޱ� (x^2) + (y^2)���� ������ �� ������ �� ���� distance�� �ִ´�.
					float dist = std::sqrt(std::powf((viewAngles->x - tempAngles.x), 2) + std::powf((viewAngles->y - tempAngles.y), 2));
					// 0xED - Dormant, 0x100 - Health
					if (bestEnt == curEnt || dist < closestEnt || *(bool*)(*bestEnt + m_bDormant)
						|| *(int*)(*bestEnt + m_iHealth) <= 0 && (!*(bool*)(*curEnt + m_bDormant) && *(int*)(*curEnt + m_iHealth) > 0)) {
						// �����͸� ������ entity�� �����Ѵ�.
						bestEnt = curEnt;
						// ���� ����� fov ����
						closestEnt = dist;
						// fov�� �ش� ������ ������ġ���� Ŀ���ϰ�, ���� �����ϰ�, Dormant�� �����ؾ��ϰ�, ü���� 0���� Ŀ�� angle�� �ٲ�
						if ((closestEnt < fov) && GetAsyncKeyState(VK_LBUTTON) && (!*(bool*)(*curEnt + m_bDormant) && *(int*)(*curEnt + m_iHealth) > 0)) { // if we click our pew pew button and our ent isnt dead n shit then we snap n shit
							*viewAngles = tempAngles; // set our actual viewangle
						}
					}
				}
			}
		}
	}
}