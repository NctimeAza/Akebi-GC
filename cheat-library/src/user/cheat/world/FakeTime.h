#pragma once
namespace cheat::feature
{

	class FakeTime : public Feature
	{
	public:
		config::Field<config::Toggle<Hotkey>> f_Enabled;
		config::Field<int> f_TimeHour;
		config::Field<int> f_TimeMinute;
		config::Field<config::Toggle<Hotkey>> f_Timer;
		config::Field<float> f_TimeSpeed;
		config::Field<config::Toggle<Hotkey>> f_GameTime;
		config::Field<config::Toggle<Hotkey>> f_ScaleRatio;
		config::Field<float> f_SR;

		 
		static FakeTime& GetInstance();
		void OnGameUpdate();
		const FeatureGUIInfo& GetGUIInfo() const override;
		void DrawMain() override;
		virtual bool NeedStatusDraw() const override;
		void DrawStatus() override;

	private:
		static void LevelTimeManager_SetInternalTimeOfDay_Hook(void* __this, float inHours, bool force, bool refreshEnviroTime, MethodInfo* method);
		static void PostProcessLayer_set_innerResolutionScaleRatio_Hook(void*__this, float value, MethodInfo* method);
		float ConversionTime();
		float ScaleRatioF();
		FakeTime();
	};
}