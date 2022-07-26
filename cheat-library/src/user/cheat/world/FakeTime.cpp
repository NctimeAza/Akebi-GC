#include "pch-il2cpp.h"
#include "FakeTime.h"
#include <cheat/events.h>


namespace cheat::feature
{
	//CNLouisLiu
	void* LevelTimeManager = NULL;
	void* SRManager = NULL;
	FakeTime::FakeTime() : Feature(),
		NF(f_Enabled, u8"���ʱ��", u8"���ʱ��", false),
		NF(f_TimeHour, u8"���ʱ��", "TimeHour", 12),
		NF(f_TimeMinute, u8"���ʱ��", "TimeMinute", 0),
		NF(f_Timer, u8"ȫ�ּ���", u8"ȫ�ּ���", false),
		NF(f_TimeSpeed, u8"ȫ�ּ���", "TimeSpeed", 1),
		NF(f_GameTime, u8"ʱ���趨", u8"ʱ���趨", false),
		NF(f_ScaleRatio, u8"����", u8"test", false),
		NF(f_SR, u8"����", u8"testSR", 1)
	{
		HookManager::install(app::LevelTimeManager_SetInternalTimeOfDay, LevelTimeManager_SetInternalTimeOfDay_Hook);

		//HookManager::install(app::PostProcessLayer_set_innerResolutionScaleRatio, PostProcessLayer_set_innerResolutionScaleRatio_Hook);

		events::GameUpdateEvent += MY_METHOD_HANDLER(FakeTime::OnGameUpdate);
	}
	FakeTime& FakeTime::GetInstance()
	{
		static FakeTime instance;
		return instance;
	}
	const FeatureGUIInfo& FakeTime::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ u8"��Ϸʱ���趨", "World", true };
		return info;
	}
	void FakeTime::DrawMain()
	{
		ConfigWidget(u8"����", f_GameTime, u8"��Ϸʱ���趨");
		if (f_GameTime)
		{
			ConfigWidget(u8"�������ʱ��", f_Enabled, u8"ʹ�ͻ���ʱ�䱣����ĳ��ʱ��");
			ConfigWidget(u8"ʱ", f_TimeHour, 1, 0, 24);
			ConfigWidget(u8"��", f_TimeMinute, 1, 0, 60);
			ConfigWidget(u8"ȫ�ּ���", f_Timer, u8"�޸���Ϸȫ���ٶ�");
			ConfigWidget(u8"����", f_ScaleRatio, u8"test");
			ConfigWidget(u8"float", f_SR, 0, 1, 5);

			if (f_Timer)
			{
				ConfigWidget(u8"���ٱ���", f_TimeSpeed, 1, 0, 100);
			}

		}
	}
	bool FakeTime::NeedStatusDraw() const
	{
		//return f_Enabled;
		//return f_Timer;
		return f_GameTime;
	}
	void FakeTime::DrawStatus()
	{
		float S = app::Time_get_timeScale(nullptr);
		if (f_Enabled)
		{
			ImGui::Text(u8"�Զ���ʱ��|%d:%d", f_TimeHour.value(), f_TimeMinute.value());
		}
		ImGui::Text(u8"��Ϸ�ٶ� %f", S);
	}

	float FakeTime::ConversionTime() {

		float time = float(f_TimeHour);
		float timemin = f_TimeMinute / 60;
		return time + timemin;
	}
	float FakeTime::ScaleRatioF() {

		float sr = f_SR;
		return sr;
	}
	void FakeTime::OnGameUpdate()
	{
		if (LevelTimeManager != NULL && f_Enabled) {
			auto& faketime = GetInstance();
			CALL_ORIGIN(LevelTimeManager_SetInternalTimeOfDay_Hook, LevelTimeManager, faketime.ConversionTime(), false, false, (MethodInfo*)0);
		}
		float gameSpeed = app::Time_get_timeScale(nullptr);
		if (f_Timer)
			app::Time_set_timeScale(f_TimeSpeed, nullptr);
		else if(gameSpeed != 1.0f)
			app::Time_set_timeScale(1.0f, nullptr);
	}
	
	void FakeTime::PostProcessLayer_set_innerResolutionScaleRatio_Hook(void* __this, float value, MethodInfo* method) {
		//float Hours = inHours;
		
		//if (faketime.f_Enabled)
		//{
		//	Hours = faketime.ConversionTime();
		//}
		float SR = value;
	    auto& faketime = GetInstance();
		if (faketime.f_ScaleRatio)
		{
			SR = faketime.ScaleRatioF();
		}
		
		CALL_ORIGIN(PostProcessLayer_set_innerResolutionScaleRatio_Hook, __this, SR, method);

	}

	void FakeTime::LevelTimeManager_SetInternalTimeOfDay_Hook(void* __this, float inHours, bool force, bool refreshEnviroTime, MethodInfo* method) {
		float Hours = inHours;
		auto& faketime = GetInstance(); 
		if (faketime.f_Enabled)
		{
			Hours = faketime.ConversionTime();
		}
		CALL_ORIGIN(LevelTimeManager_SetInternalTimeOfDay_Hook, __this, Hours, force, refreshEnviroTime, method);

	}

}