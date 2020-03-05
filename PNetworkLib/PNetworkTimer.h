#pragma once
#include <Windows.h>

template <typename T>
class PNetworkSingleton
{
public:
	static T& GetInstance()
	{
		static T singleton;
		return singleton;
	}
};

class PNetworkTimer : public PNetworkSingleton<PNetworkTimer>
{

private:
	friend class PNetworkSingleton<PNetworkTimer>;
	PNetworkTimer();
public:
	~PNetworkTimer();
private:
	
public:
	void Init();


};

extern bool g_custom_tick;
extern bool g_skill_time;
extern HANDLE g_handle_200ms_timer_queue_;
extern HANDLE g_handle_timer_;
extern HANDLE g_handle_20s_timer_queue_;
extern HANDLE g_handle_20s_timer_;