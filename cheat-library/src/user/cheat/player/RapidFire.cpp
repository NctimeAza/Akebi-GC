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
        NF(f_Enabled,			u8"快速攻击",	u8"RapidFire", false),
		NF(f_MultiHit,			u8"多段攻击",			u8"RapidFire", false),
        NF(f_Multiplier,		u8"多段攻击倍数",		u8"RapidFire", 2),
        NF(f_OnePunch,			u8"一击模式",		u8"RapidFire", false),
		NF(f_Randomize,			u8"随机攻击倍数",			u8"RapidFire", false),
		NF(f_minMultiplier,		u8"最小攻击倍数",		u8"RapidFire", 1),
		NF(f_maxMultiplier,		u8"最大攻击倍数",		u8"RapidFire", 3),
		NF(f_MultiTarget,		u8"攻击多个目标",			u8"RapidFire", false),
		NF(f_MultiTargetRadius, u8"多目标攻击范围",	u8"RapidFire", 20.0f)	
    {
		HookManager::install(app::MoleMole_LCBaseCombat_DoHitEntity, LCBaseCombat_DoHitEntity_Hook);
    }

    const FeatureGUIInfo& RapidFire::GetGUIInfo() const
    {
        static const FeatureGUIInfo info{ u8"快速攻击", "Player", true };
        return info;
    }


    void RapidFire::DrawMain()
    {
		ConfigWidget(u8"启用", f_Enabled, u8"启用快速攻击，选择一种模式使其生效。");
		ImGui::SameLine();
		ImGui::TextColored(ImColor(255, 165, 0, 255), u8"选择一种模式");          

		ConfigWidget(u8"多段攻击", f_MultiHit, u8"启用多段攻击\n" \
            u8"倍增你的攻击次数。\n" \
            u8"测试表现不太好，可以被反作弊监测到。\n" \
            u8"不建议在大号上使用，或者使用太高的值\n" \
			u8"已知某些角色的攻击无法正常使用，例如魈的E,绫华的重击等。");

		ImGui::Indent();

		ConfigWidget(u8"一击模式", f_OnePunch, u8"根据敌人的生命值计算出杀死敌人所需的攻击次数\n" \
			u8"并在此次攻击中使用该攻击次数。\n" \
			u8"可能会比较安全，但计算可能不够准确。");

		ConfigWidget(u8"随机攻击倍数", f_Randomize, u8"在最大和最小攻击倍数中取随机数作为攻击次数(两个值不能相等!)");
		ImGui::SameLine();
		ImGui::TextColored(ImColor(255, 165, 0, 255), u8"启用此功能会覆盖一击模式");

		if (!f_OnePunch) {
			if (!f_Randomize)
			{
				ConfigWidget(u8"多段攻击倍数", f_Multiplier, 1, 2, 1000, u8"多段攻击倍数");
			}
			else
			{
				ConfigWidget(u8"最小攻击倍数", f_minMultiplier, 1, 2, 1000, u8"最小攻击倍数");
				ConfigWidget(u8"最大攻击倍数", f_maxMultiplier, 1, 2, 1000, u8"最大攻击倍数");
			}
		}

		ImGui::Unindent();

		ConfigWidget(u8"多目标模式", f_MultiTarget, u8"在一定范围内使用多目标攻击。\n" \
			u8"初始目标附近的有效目标都会被命中。\n" \
			u8"伤害数字只会出现在初始目标上，但范围内所有有效目标都会受到伤害。\n" \
			u8"如果多段攻击没有启用，但被命中目标头上仍然出现了多个伤害数字，请打开调试窗口的实体管理以查看是否有不可见的实体。\n" \
			u8"警告：如果与多段攻击一起使用，可能会导致极高的延迟且更容易受到封禁。"
		);
	
		ImGui::Indent();
		ConfigWidget(u8"范围 (m)", f_MultiTargetRadius, 0.1f, 5.0f, 100.0f, u8"检查有效目标的范围半径");
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
				ImGui::Text(u8"多段攻击 随机模式[%d|%d]", f_minMultiplier.value(), f_maxMultiplier.value());
			else if (f_OnePunch)
				ImGui::Text(u8"多段攻击 [一击模式]");
			else
				ImGui::Text(u8"多段攻击 [%d]", f_Multiplier.value());
		}
		if (f_MultiTarget)
			ImGui::Text(u8"多目标 [%.01fm]", f_MultiTargetRadius.value());
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

