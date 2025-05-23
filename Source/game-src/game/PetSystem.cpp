#include "stdafx.h"
#include "utils.h"
#include "vector.h"
#include "char.h"
#include "sectree_manager.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "PetSystem.h"
#include "packet.h"
#include "item_manager.h"
#include "entity.h"
#include "item.h"
#include "desc.h"

CPetActor::CPetActor(LPITEM pItem, BYTE petType)
{
	m_petType = petType;
	m_dwSummonItem = pItem;
	m_pkPet = NULL;
}

CPetActor::~CPetActor()
{
	Unsummon();
	m_petType = 0;
	m_dwSummonItem = NULL;
	if(m_pkPet)
		M2_DESTROY_CHARACTER(m_pkPet);
	m_pkPet = NULL;
}

void CPetActor::Unsummon(bool isReal)
{
	if (GetPet())
	{
		//ClearBuffSkill();
#ifdef ENABLE_NEW_PET_SYSTEM
		LPITEM summonItem = GetSummonItem();
		if (summonItem)
		{
			LPCHARACTER owner = summonItem->GetOwner();
			if (owner)
				if (GetType() == NORMAL_LEVEL_PET)
					owner->ChatPacket(CHAT_TYPE_COMMAND, "PetClearData");
		}
#endif
		M2_DESTROY_CHARACTER(m_pkPet);
		m_pkPet = NULL;

		if (isReal)
			SetSummonItem(NULL);
	}
}

DWORD CPetActor::Summon(const char* petName, DWORD mobVnum)
{
	LPITEM summonItem = GetSummonItem();
	if(!summonItem)
		return 0;

	LPCHARACTER owner = summonItem->GetOwner();
	if(!owner)
		return 0;

	long x = owner->GetX()+number(-100, 100);
	long y = owner->GetY()+number(-100, 100);
	long z = owner->GetZ();

	if (NULL != m_pkPet)
	{
		m_pkPet->Show(owner->GetMapIndex(), x, y);
		return m_pkPet->GetVID();
	}

	m_pkPet = CHARACTER_MANAGER::instance().SpawnMob(mobVnum,owner->GetMapIndex(),x, y, z,false, (int)(owner->GetRotation() + 180), false);

	if (NULL == m_pkPet)
	{
		sys_err("[CPetSystem::Summon] Failed to summon the pet. (vnum: %d)", mobVnum);
		return 0;
	}

	//m_pkPet->SetPet();
	m_pkPet->SetEmpire(owner->GetEmpire());
	m_pkPet->SetOwner(owner);
	m_pkPet->SetName(petName);
	

#ifdef ENABLE_NEW_PET_SYSTEM
	if (GetType() == NORMAL_LEVEL_PET)
	{
		m_pkPet->SetNewPet();
		m_pkPet->SetLevel(summonItem->GetSocket(1));
		owner->ChatPacket(CHAT_TYPE_COMMAND, "PetSetSlotIndex %d", summonItem->GetCell());
	}
#endif

	m_pkPet->Show(owner->GetMapIndex(), x, y, z);
	return m_pkPet->GetVID();
}

void CPetActor::GiveBuffSkill()
{
	LPITEM summonItem = GetSummonItem();
	if (summonItem)
	{
		LPCHARACTER owner = summonItem->GetOwner();
		if (owner)
		{
			summonItem->ModifyPoints(true);
			owner->ComputePoints();
		}
	}
}

void CPetActor::ClearBuffSkill()
{
	LPITEM summonItem = GetSummonItem();
	if (summonItem)
	{
		LPCHARACTER owner = summonItem->GetOwner();
		if (owner)
		{
			summonItem->ModifyPoints(false);
			owner->ComputePoints();
		}
	}
}

#ifdef ENABLE_NEW_PET_SYSTEM
BYTE CPetActor::GetMaxLevelByEvolution(BYTE evolution)
{
	return petEvolutionLimits[evolution];
}

DWORD CPetActor::GetNextExp()
{
	LPCHARACTER pet = GetPet();
	if (!pet)
		return 2500000000u;

	if (PET_MAX_LEVEL_CONST < pet->GetLevel())
		return 2500000000u;
	else
		return exp_table[pet->GetLevel()];
}

void CPetActor::IncreaseEvolve()
{
	LPCHARACTER pet = GetPet();
	if (!pet)
		return;

	LPITEM summonItem = GetSummonItem();
	if (!summonItem)
		return;

	LPCHARACTER owner = summonItem->GetOwner();
	if (!owner)
		return;

	long evolution = summonItem->GetSocket(POINT_PET_EVOLVE);


	BYTE byLimit = GetMaxLevelByEvolution(evolution);

	if (evolution == PET_MAX_EVOLUTION_CONST)
	{
		owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("731"));
		return;
	}

	if (pet->GetLevel() < byLimit)
	{
		owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("718"), byLimit);
		return;
	}


	auto itemVec = petEvolutionItems[evolution];
	for (const auto& item : itemVec)
	{
		if (owner->CountSpecifyItem(item.first) < item.second)
		{
			TItemTable* p = ITEM_MANAGER::instance().GetTable(item.first);
			if(p)
				owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("730"), p->szName, item.second);
			return;
		}
	}
	for (const auto& item : itemVec)
		owner->RemoveSpecifyItem(item.first, item.second);

	PointChange(POINT_PET_EVOLVE, 1);
	owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("733"));

	if (pet->GetLevel() == 100)
	{
		char name[49];
		strlcpy(name, pet->GetName(), sizeof(name));
		Unsummon(false);
#ifdef ENABLE_CHANGELOOK_SYSTEM
		Summon(name, summonItem->GetTransmutation()? summonItem->GetTransmutation()+1:summonItem->GetValue(1));
#else
		Summon(name, summonItem->GetValue(1));
#endif
	}

	pet = GetPet();
	if (!pet)
		return;
	TPacketGCSpecificEffect p;
	p.header = HEADER_GC_SPECIFIC_EFFECT;
	p.vid = pet->GetVID();
	memcpy(p.effect_file, "d:/ymir work/effect/jin_han/work/efect_duel_jin_han_sender.mse", MAX_EFFECT_FILE_NAME);
	owner->PacketAround(&p, sizeof(TPacketGCSpecificEffect));
}

BYTE CPetActor::CheckSkillIndex(LPITEM summonItem, BYTE skillIndex)
{
	for (BYTE j = 0; j < ITEM_ATTRIBUTE_MAX_NUM; ++j)
	{
		if (summonItem->GetAttributeType(j) == skillIndex)
			return j;
	}
	return 99;
}

BYTE CPetActor::CheckEmptyIndex(LPITEM summonItem)
{
	for (BYTE j = 0; j < ITEM_ATTRIBUTE_MAX_NUM; ++j)
	{
		if (summonItem->GetAttributeType(j) == 0)
			return j;
	}
	return 99;
}

bool CPetActor::IncreaseSkill(const BYTE skillIndex)
{
	LPCHARACTER pet = GetPet();
	if (!pet)
		return false;
	LPITEM summonItem = GetSummonItem();
	if (!summonItem)
		return false;
	LPCHARACTER owner = summonItem->GetOwner();
	if (!owner)
		return false;

	BYTE skillIndexInItem = CheckSkillIndex(summonItem, skillIndex);
	if (skillIndexInItem == 99)
	{
		BYTE emptyIndex = CheckEmptyIndex(summonItem);
		if (emptyIndex == 99)
		{
			owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("723"));
			return false;
		}
		PointChange(POINT_PET_SKILL_INDEX_1 + emptyIndex, skillIndex);
		TItemTable* p = ITEM_MANAGER::instance().GetTable(55009+skillIndex);
		if(p)
			owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("724"), p->szName);
		return true;
	}
	else
	{
		if (PointChange(POINT_PET_SKILL_LEVEL_1 + skillIndexInItem, 1)) {
			TItemTable* p = ITEM_MANAGER::instance().GetTable(55009+skillIndex);
			if(p)
				owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("726"), p->szName);
			return true;
		}
		else
		{
			TItemTable* p = ITEM_MANAGER::instance().GetTable(55009+skillIndex);
			if(p)
				owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("725"), p->szName);
		}
	}
	return false;
}


bool CPetActor::IncreaseBonus(const BYTE bonusType, const BYTE bonusStep)
{
	LPCHARACTER pet = GetPet();
	if (!pet)
		return false;
	LPITEM summonItem = GetSummonItem();
	if (!summonItem)
		return false;
	LPCHARACTER owner = summonItem->GetOwner();
	if (!owner)
		return false;

	long bonusLevel = summonItem->GetSocket(POINT_PET_BONUS_1 + bonusType);

	if (bonusStep == 1)
	{
		if (bonusLevel >= 0 && bonusLevel <= 4)
		{
			PointChange(POINT_PET_BONUS_1 + bonusType, 1);
			return true;
		}
		else
		{
			owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("722"));
			return false;
		}
	}
	else if (bonusStep == 2)
	{
		if (pet->GetLevel() < 40)
		{
			owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("718"), 40);
			return false;
		}

		if (bonusLevel >= 5 && bonusLevel <= 9)
		{
			PointChange(POINT_PET_BONUS_1 + bonusType, 1);
			return true;
		}
		else
		{
			owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("722"));
			return false;
		}

	}
	else if (bonusStep == 3)
	{
		if (pet->GetLevel() < 75)
		{
			owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("718"), 75);
			return false;
		}

		if (bonusLevel >= 10 && bonusLevel <= 14)
		{
			PointChange(POINT_PET_BONUS_1 + bonusType, 1);
			return true;
		}
		else
		{
			owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("722"));
			return false;
		}
	}
	else if (bonusStep == 4)
	{
		if (pet->GetLevel() < 100)
		{
			owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("718"), 100);
			return false;
		}

		if (bonusLevel >= 15 && bonusLevel <= 19)
		{
			PointChange(POINT_PET_BONUS_1 + bonusType, 1);
			return true;
		}
		else
		{
			owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("722"));
			return false;
		}
	}
}


bool CPetActor::PointChange(BYTE byType, int amount, bool bBroadcast)
{
	LPCHARACTER pet = GetPet();
	if (!pet)
		return false;

	LPITEM summonItem = GetSummonItem();
	if (!summonItem)
		return false;

	LPCHARACTER owner = summonItem->GetOwner();
	if (!owner)
		return false;

	int val = 0;
	BYTE type = 0;

	switch (byType)
	{
	case POINT_PET_LEVEL:
	{
		if ((pet->GetLevel() + amount) > GetMaxLevelByEvolution(summonItem->GetSocket(POINT_PET_EVOLVE)))
			return false;

		pet->SetLevel(pet->GetLevel() + amount);
		val = pet->GetLevel();
		type = POINT_LEVEL;

		summonItem->SetSocket(POINT_PET_LEVEL, pet->GetLevel());
		//summonItem->UpdatePacket();
	}
	break;

	case POINT_PET_DURATION:
	{
		long oldTime = summonItem->GetSocket(POINT_PET_DURATION) - time(0);
		if (oldTime < 0)
			oldTime = 0;

		if (oldTime >= ((60 * 60 * 24 * 14) * 6) - 100)
			return false;

		long newTime = MIN((oldTime + amount), (60*60*24*14)*6);
		summonItem->SetSocket(POINT_PET_DURATION, time(0)+ newTime);
		//summonItem->UpdatePacket();
		val = newTime;
	}
	break;

	case POINT_PET_EXP:
	{
		DWORD exp = pet->GetExp();
		DWORD next_exp = GetNextExp();

		if ((amount < 0) && (exp < (DWORD)(-amount)))
		{
			amount -= exp;
			pet->SetExp(exp + amount);
			val = pet->GetExp();
		}
		else
		{
			if (pet->GetLevel() >= GetMaxLevelByEvolution(summonItem->GetSocket(POINT_PET_EVOLVE)))
				return false;

			DWORD iExpBalance = 0;
			if (exp + amount >= next_exp)
			{
				iExpBalance = (exp + amount) - next_exp;
				amount = next_exp - exp;
				pet->SetExp(0);
				PointChange(POINT_PET_LEVEL, 1, true);

				summonItem->SetSocket(POINT_PET_EXP, 0);
			}
			else
				pet->SetExp(exp + amount);

			if (iExpBalance)
				PointChange(POINT_PET_EXP, iExpBalance);
			val = pet->GetExp();

			summonItem->SetSocket(POINT_PET_EXP, val);
		}
	}
	break;

	case POINT_PET_EVOLVE:
	{
		long evolution = summonItem->GetSocket(POINT_PET_EVOLVE) + amount;
		summonItem->SetSocket(POINT_PET_EVOLVE, evolution);
		summonItem->SetForceAttribute(evolution-1, 0, 0);
		val = evolution;
	}
	break;

	case POINT_PET_BONUS_1:
	case POINT_PET_BONUS_2:
	case POINT_PET_BONUS_3:
	{
		summonItem->ModifyPoints(false);
		val = summonItem->GetSocket(byType)+1;
		summonItem->SetSocket(byType, val);
		summonItem->ModifyPoints(true);
	}
	break;

	case POINT_PET_SKILL_INDEX_1:
	case POINT_PET_SKILL_INDEX_2:
	case POINT_PET_SKILL_INDEX_3:
	case POINT_PET_SKILL_INDEX_4:
	case POINT_PET_SKILL_INDEX_5:
	case POINT_PET_SKILL_INDEX_6:
	case POINT_PET_SKILL_INDEX_7:
	case POINT_PET_SKILL_INDEX_8:
	case POINT_PET_SKILL_INDEX_9:
	case POINT_PET_SKILL_INDEX_10:
	case POINT_PET_SKILL_INDEX_11:
	case POINT_PET_SKILL_INDEX_12:
	case POINT_PET_SKILL_INDEX_13:
	case POINT_PET_SKILL_INDEX_14:
	case POINT_PET_SKILL_INDEX_15:
	{
		BYTE bySlotIndex = byType - POINT_PET_SKILL_INDEX_1;
		summonItem->SetForceAttribute(bySlotIndex, amount, 0);
		val = bySlotIndex;
	}
	break;

	case POINT_PET_SKILL_LEVEL_1:
	case POINT_PET_SKILL_LEVEL_2:
	case POINT_PET_SKILL_LEVEL_3:
	case POINT_PET_SKILL_LEVEL_4:
	case POINT_PET_SKILL_LEVEL_5:
	case POINT_PET_SKILL_LEVEL_6:
	case POINT_PET_SKILL_LEVEL_7:
	case POINT_PET_SKILL_LEVEL_8:
	case POINT_PET_SKILL_LEVEL_9:
	case POINT_PET_SKILL_LEVEL_10:
	case POINT_PET_SKILL_LEVEL_11:
	case POINT_PET_SKILL_LEVEL_12:
	case POINT_PET_SKILL_LEVEL_13:
	case POINT_PET_SKILL_LEVEL_14:
	case POINT_PET_SKILL_LEVEL_15:
	{
		BYTE bySlotIndex = byType - POINT_PET_SKILL_LEVEL_1;

		BYTE type = summonItem->GetAttributeType(bySlotIndex);
		long value = summonItem->GetAttributeValue(bySlotIndex);
		if (value > 19)
			return false;
		summonItem->ModifyPoints(false);
		summonItem->SetForceAttribute(bySlotIndex, type, value+ amount);
		summonItem->ModifyPoints(true);
		val = value + amount;
	}
	break;
	}

	if (bBroadcast)
	{
		struct packet_point_change pack;
		pack.header = HEADER_GC_CHARACTER_POINT_CHANGE;
		pack.dwVID = pet->GetVID();
		pack.type = type;
		pack.value = val;
		owner->PacketAround(&pack, sizeof(pack));
	}

	owner->ChatPacket(CHAT_TYPE_COMMAND, "UpdatePet %d", byType);
	return true;
}
#endif

/* 

MANAGER
CLASS


*/
CPetSystem::CPetSystem(LPCHARACTER owner)
{
	m_pkOwner = owner;
}

CPetSystem::~CPetSystem()
{
	Destroy();
	m_pkOwner = NULL;
}

void CPetSystem::Destroy()
{
	for (auto iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		LPPET petActor = iter->second;
		if (petActor)
			delete petActor;
		petActor = NULL;
	}
	m_petActorMap.clear();
}

void CPetSystem::DeletePet(DWORD itemID)
{
	if (m_pkOwner->PetBlockMap())
		return;
	
	auto iter = m_petActorMap.find(itemID);
	if (m_petActorMap.end() == iter)
	{
		sys_err("[CPetSystem::DeletePet] Can't find pet on my list (ITEM ID: %d)", itemID);
		return;
	}
	LPPET petActor = iter->second;
	if (petActor)
		delete petActor;
	petActor = NULL;
	m_petActorMap.erase(iter);
}

void CPetSystem::DeletePet(LPPET petActor)
{
	if(!petActor)
		return;

	if (m_pkOwner->PetBlockMap())
		return;

	for (auto iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		if (iter->second == petActor)
		{
			delete petActor;
			m_petActorMap.erase(iter);
			break;
		}
	}
}

LPPET CPetSystem::Summon(DWORD mobVnum, LPITEM pSummonItem, const char* petName, BYTE petType)
{
	LPPET petActor = GetByID(pSummonItem->GetID());
	if (petActor)
	{
		sys_err("wtf have same pet owner %s item vnum %d petvnum %d ", m_pkOwner->GetName(), pSummonItem->GetVnum(), mobVnum);
		return petActor;
	}

	petActor = M2_NEW CPetActor(pSummonItem, petType);
	m_petActorMap.insert(std::make_pair(pSummonItem->GetID(), petActor));
	DWORD petVID = petActor->Summon(petName, mobVnum);
	return petActor;
}

void CPetSystem::HandlePetCostumeItem()
{
	if (m_pkOwner->PetBlockMap())
		return;
	LPITEM pkPetCostume = m_pkOwner->GetWear(WEAR_COSTUME_PET);
	if (!pkPetCostume)
		return;
#ifdef ENABLE_CHANGELOOK_SYSTEM
	DWORD dwPetVnum = pkPetCostume->GetTransmutation()?pkPetCostume->GetTransmutation():pkPetCostume->GetValue(0);
#else
	DWORD dwPetVnum = pkPetCostume->GetValue(0);
#endif
	if (!dwPetVnum)
		return;
#ifdef ENABLE_NEW_NAME_ITEM
	char name[49];
	sprintf(name, "^%s", m_pkOwner->GetName());
	if(!strstr(pkPetCostume->GetNewName(),"^"))
		sprintf(name, "%s", pkPetCostume->GetNewName());
	Summon(dwPetVnum, pkPetCostume, name, NORMAL_PET);
#else
	Summon(dwPetVnum, pkPetCostume, m_pkOwner->GetName(), false, NORMAL_PET);
#endif
}

void CPetSystem::ChangeGmPetStatus()
{
	LPITEM pkPetCostume = m_pkOwner->GetWear(WEAR_COSTUME_PET);
	if (!pkPetCostume)
		return;
	LPPET pet = GetByID(pkPetCostume->GetID());
	if (!pet)
		return;
	if (pet->GetPet())
		pet->GetPet()->ViewReencode();
}


LPPET CPetSystem::GetByVID(DWORD vid) const
{
	LPPET petActor = NULL;
	for (auto iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		LPPET petActor = iter->second;
		if(petActor)
		{
			LPCHARACTER pet = petActor->GetPet();
			if (pet)
			{
				if (pet->GetVID() == vid)
					return petActor;
			}
		}
	}
	return NULL;
}

LPPET CPetSystem::GetByID(DWORD itemID) const
{
	auto iter = m_petActorMap.find(itemID);
	if (m_petActorMap.end() != iter)
		return iter->second;
	return NULL;
}

size_t CPetSystem::CountSummoned() const
{
	size_t count = 0;
	for (auto iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		LPPET petActor = iter->second;
		if (0 != petActor)
			if (petActor->IsSummoned())
				++count;
	}
	return count;
}

#ifdef ENABLE_NEW_PET_SYSTEM
void CPetSystem::PointChange(BYTE byType, int amount, bool bBroadcast)
{
	for (auto iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		LPPET petActor = iter->second;
		if (petActor)
		{
			if (petActor->GetPet() && petActor->GetType() == NORMAL_LEVEL_PET)
			{
				petActor->PointChange(byType, amount, bBroadcast);
				break;
			}
		}
	}
}

LPPET CPetSystem::GetNewPet()
{
	for (auto iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		LPPET petActor = iter->second;
		if (petActor)
		{
			if (petActor->GetPet() && petActor->GetType() == NORMAL_LEVEL_PET)
				return petActor;
		}
	}
	return NULL;
}

void CPetSystem::HandleNewPetItem()
{
	if (m_pkOwner->PetBlockMap())
		return;

	LPITEM pkPetCostume = m_pkOwner->GetWear(WEAR_PET);

	if (!pkPetCostume)
		return;

	if (time(0) > pkPetCostume->GetSocket(POINT_PET_DURATION))
	{
		m_pkOwner->UnequipItem(pkPetCostume);
		TItemTable* p = ITEM_MANAGER::instance().GetTable(55001);
		if(p)
			m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("719"), p->szName);
		return;
	}

#ifdef ENABLE_CHANGELOOK_SYSTEM
	DWORD dwPetVnum = pkPetCostume->GetTransmutation() ? pkPetCostume->GetTransmutation(): pkPetCostume->GetValue(0);
#else
	DWORD dwPetVnum = pkPetCostume->GetValue(0);
#endif
	if (pkPetCostume->GetSocket(POINT_PET_LEVEL) >= 100 && pkPetCostume->GetSocket(POINT_PET_EVOLVE) == 3)
		dwPetVnum += 1;

	if (!dwPetVnum)
		return;

#ifdef ENABLE_NEW_NAME_ITEM
	char name[49];
	sprintf(name, "^%s", m_pkOwner->GetName());
	if (!strstr(pkPetCostume->GetNewName(), "^"))
		sprintf(name, "%s", pkPetCostume->GetNewName());
	Summon(dwPetVnum, pkPetCostume, name, NORMAL_LEVEL_PET);
#else
	Summon(dwPetVnum, pkPetCostume, m_pkOwner->GetName(), false, NORMAL_LEVEL_PET);
#endif
}
#endif
