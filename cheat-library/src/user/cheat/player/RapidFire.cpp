#include "pch-il2cpp.h"
#include "RapidFire.h"

#include <helpers.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/util.h>
#include <cheat/game/filters.h>
#include <imgui.h>

namespace cheat::feature 
{
	static void LCBaseCombat_DoHitEntity_Hook(app::LCBaseCombat* __this, uint32_t targetID, app::AttackResult* attackResult,
		bool ignoreCheckCanBeHitInMP, MethodInfo* method);

    RapidFire::RapidFire() : Feature(),
        NF(f_Enabled,			u8"���ٹ���",	u8"RapidFire", false),
		NF(f_MultiHit,			u8"��ι���",			u8"RapidFire", false),
        NF(f_Multiplier,		u8"��ι�������",		u8"RapidFire", 2),
        NF(f_OnePunch,			u8"һ��ģʽ",		u8"RapidFire", false),
		NF(f_Randomize,			u8"�����������",			u8"RapidFire", false),
		NF(f_minMultiplier,		u8"��С��������",		u8"RapidFire", 1),
		NF(f_maxMultiplier,		u8"��󹥻�����",		u8"RapidFire", 3),
		NF(f_MultiTarget,		u8"�������Ŀ��",			u8"RapidFire", false),
		NF(f_MultiTargetRadius, u8"��Ŀ�깥����Χ",	u8"RapidFire", 20.0f)	
    {
		HookManager::install(app::MoleMole_LCBaseCombat_DoHitEntity, LCBaseCombat_DoHitEntity_Hook);
    }

    const FeatureGUIInfo& RapidFire::GetGUIInfo() const
    {
        static const FeatureGUIInfo info{ u8"���ٹ���", "Player", true };
        return info;
    }


    void RapidFire::DrawMain()
    {
		ConfigWidget(u8"����", f_Enabled, u8"���ÿ��ٹ�����ѡ��һ��ģʽʹ����Ч��");
		ImGui::SameLine();
		ImGui::TextColored(ImColor(255, 165, 0, 255), u8"ѡ��һ��ģʽ");          

		ConfigWidget(u8"��ι���", f_MultiHit, u8"���ö�ι���\n" \
            u8"������Ĺ���������\n" \
            u8"���Ա��ֲ�̫�ã����Ա������׼�⵽��\n" \
            u8"�������ڴ����ʹ�ã�����ʹ��̫�ߵ�ֵ\n" \
			u8"��֪ĳЩ��ɫ�Ĺ����޷�����ʹ�ã������̵�E,类����ػ��ȡ�");

		ImGui::Indent();

		ConfigWidget(u8"һ��ģʽ", f_OnePunch, u8"���ݵ��˵�����ֵ�����ɱ����������Ĺ�������\n" \
			u8"���ڴ˴ι�����ʹ�øù���������\n" \
			u8"���ܻ�Ƚϰ�ȫ����������ܲ���׼ȷ��");

		ConfigWidget(u8"�����������", f_Randomize, u8"��������С����������ȡ�������Ϊ��������(����ֵ�������!)");
		ImGui::SameLine();
		ImGui::TextColored(ImColor(255, 165, 0, 255), u8"���ô˹��ܻḲ��һ��ģʽ");

		if (!f_OnePunch) {
			if (!f_Randomize)
			{
				ConfigWidget(u8"��ι�������", f_Multiplier, 1, 2, 1000, u8"��ι�������");
			}
			else
			{
				ConfigWidget(u8"��С��������", f_minMultiplier, 1, 2, 1000, u8"��С��������");
				ConfigWidget(u8"��󹥻�����", f_maxMultiplier, 1, 2, 1000, u8"��󹥻�����");
			}
		}

		ImGui::Unindent();

		ConfigWidget(u8"��Ŀ��ģʽ", f_MultiTarget, u8"��һ����Χ��ʹ�ö�Ŀ�깥����\n" \
			u8"��ʼĿ�긽������ЧĿ�궼�ᱻ���С�\n" \
			u8"�˺�����ֻ������ڳ�ʼĿ���ϣ�����Χ��������ЧĿ�궼���ܵ��˺���\n" \
			u8"�����ι���û�����ã���������Ŀ��ͷ����Ȼ�����˶���˺����֣���򿪵��Դ��ڵ�ʵ������Բ鿴�Ƿ��в��ɼ���ʵ�塣\n" \
			u8"���棺������ι���һ��ʹ�ã����ܻᵼ�¼��ߵ��ӳ��Ҹ������ܵ������"
		);
	
		ImGui::Indent();
		ConfigWidget(u8"��Χ (m)", f_MultiTargetRadius, 0.1f, 5.0f, 100.0f, u8"�����ЧĿ��ķ�Χ�뾶");
		ImGui::Unindent();
    }

    bool RapidFire::NeedStatusDraw() const
{
        return f_Enabled && (f_MultiHit || f_MultiTarget);
    }

    void RapidFire::DrawStatus() 
    {
		if (f_MultiHit) 
		{
			if (f_Randomize)
				ImGui::Text(u8"��ι��� ���ģʽ[%d|%d]", f_minMultiplier.value(), f_maxMultiplier.value());
			else if (f_OnePunch)
				ImGui::Text(u8"��ι��� [һ��ģʽ]");
			else
				ImGui::Text(u8"��ι��� [%d]", f_Multiplier.value());
		}
		if (f_MultiTarget)
			ImGui::Text(u8"��Ŀ�� [%.01fm]", f_MultiTargetRadius.value());
    }

    RapidFire& RapidFire::GetInstance()
    {
        static RapidFire instance;
        return instance;
    }


	int RapidFire::CalcCountToKill(float attackDamage, uint32_t targetID)
	{
		if (attackDamage == 0)
			return f_Multiplier;
		
		auto& manager = game::EntityManager::instance();
		auto targetEntity = manager.entity(targetID);
		if (targetEntity == nullptr)
			return f_Multiplier;

		auto baseCombat = targetEntity->combat();
		if (baseCombat == nullptr)
			return f_Multiplier;

		auto safeHP = baseCombat->fields._combatProperty_k__BackingField->fields.HP;
		auto HP = app::MoleMole_SafeFloat_get_Value(safeHP, nullptr);
		int attackCount = (int)ceil(HP / attackDamage);
		return std::clamp(attackCount, 1, 200);
	}

	int RapidFire::GetAttackCount(app::LCBaseCombat* combat, uint32_t targetID, app::AttackResult* attackResult)
	{
		if (!f_MultiHit)
			return 1;

		auto& manager = game::EntityManager::instance();
		auto targetEntity = manager.entity(targetID);
		auto baseCombat = targetEntity->combat();
		if (baseCombat == nullptr)
			return 1;

		int countOfAttacks = f_Multiplier;
		if (f_OnePunch)
		{
			app::MoleMole_Formula_CalcAttackResult(combat->fields._combatProperty_k__BackingField,
				baseCombat->fields._combatProperty_k__BackingField,
				attackResult, manager.avatar()->raw(), targetEntity->raw(), nullptr);
			countOfAttacks = CalcCountToKill(attackResult->fields.damage, targetID);
		}
		if (f_Randomize)
		{
			countOfAttacks = rand() % (f_maxMultiplier.value() - f_minMultiplier.value()) + f_minMultiplier.value();
			return countOfAttacks;
		}

		return countOfAttacks;
	}

	bool IsAvatarOwner(game::Entity entity)
	{
		auto& manager = game::EntityManager::instance();
		auto avatarID = manager.avatar()->runtimeID();
		
		while (entity.isGadget())
		{
			game::Entity temp = entity;
			entity = game::Entity(app::MoleMole_GadgetEntity_GetOwnerEntity(reinterpret_cast<app::GadgetEntity*>(entity.raw()), nullptr));
			if (entity.runtimeID() == avatarID)
				return true;
		} 

		return false;
		
	}

	bool IsAttackByAvatar(game::Entity& attacker)
	{
		if (attacker.raw() == nullptr)
			return false;

		auto& manager = game::EntityManager::instance();
		auto avatarID = manager.avatar()->runtimeID();
		auto attackerID = attacker.runtimeID();

		return attackerID == avatarID || IsAvatarOwner(attacker);
	}

	bool IsValidByFilter(game::Entity* entity)
	{
		if (game::filters::combined::OrganicTargets.IsValid(entity) ||
			game::filters::combined::Ores.IsValid(entity) ||
			//game::filters::guide::gad.IsValid(entity) ||
			game::filters::guide::CampfireTorch.IsValid(entity) ||
			//game::filters::guide::MonsterSkillObj.IsValid(entity) ||
			//game::filters::guide::MiracleRing.IsValid(entity) ||
			//game::filters::guide::Qqtotem.IsValid(entity) ||
			//game::filters::guide::Qqfly.IsValid(entity) ||
			game::filters::combined::Doodads.IsValid(entity) ||
			game::filters::puzzle::Geogranum.IsValid(entity) ||
		  //game::filters::living::AvatarOwn.IsValid(entity) ||
			game::filters::living::AvatarTeammate.IsValid(entity) ||
			game::filters::living::Npc. IsValid(entity) ||
			//game::filters::monster::SentryTurrets.IsValid(entity) ||
			game::filters::puzzle::LargeRockPile.IsValid(entity) ||
			game::filters::puzzle::SmallRockPile.IsValid(entity))
			return true;
		return false;
	}

	// Raises when any entity do hit event.
	// Just recall attack few times (regulating by combatProp)
	// It's not tested well, so, I think, anticheat can detect it.
	static void LCBaseCombat_DoHitEntity_Hook(app::LCBaseCombat* __this, uint32_t targetID, app::AttackResult* attackResult,
		bool ignoreCheckCanBeHitInMP, MethodInfo* method)
	{
		auto attacker = game::Entity(__this->fields._._._entity);
		RapidFire& rapidFire = RapidFire::GetInstance();
		if (!IsAttackByAvatar(attacker) || !rapidFire.f_Enabled)
			return CALL_ORIGIN(LCBaseCombat_DoHitEntity_Hook, __this, targetID, attackResult, ignoreCheckCanBeHitInMP, method);

		auto& manager = game::EntityManager::instance();
		auto originalTarget = manager.entity(targetID);
		if (!IsValidByFilter(originalTarget))
			return CALL_ORIGIN(LCBaseCombat_DoHitEntity_Hook, __this, targetID, attackResult, ignoreCheckCanBeHitInMP, method);

		std::vector<cheat::game::Entity*> validEntities;
		validEntities.push_back(originalTarget);

		if (rapidFire.f_MultiTarget)
		{
			auto filteredEntities = manager.entities();
			for (const auto& entity : filteredEntities) {
				auto distance = originalTarget->distance(entity);

				if (entity->runtimeID() == manager.avatar()->runtimeID())
					continue;

				if (entity->runtimeID() == targetID)
					continue;

				if (distance > rapidFire.f_MultiTargetRadius)
					continue;

				if (!IsValidByFilter(entity))
					continue;

				validEntities.push_back(entity);
			}
		}

		for (const auto& avatar : validEntities) {
			if (rapidFire.f_MultiHit) {
				int attackCount = rapidFire.GetAttackCount(__this, avatar->runtimeID(), attackResult);
				for (int i = 0; i < attackCount; i++)
					CALL_ORIGIN(LCBaseCombat_DoHitEntity_Hook, __this, avatar->runtimeID(), attackResult, ignoreCheckCanBeHitInMP, method);
			} else CALL_ORIGIN(LCBaseCombat_DoHitEntity_Hook, __this, avatar->runtimeID(), attackResult, ignoreCheckCanBeHitInMP, method);
		}
	}
}

