
#ifndef _TIMER_HEADER
#define _TIMER_HEADER

#include <chrono>

enum WUpdateTimers
{
	WUpdate_Save_Players = 0,
	WUpdate_Server_Instance_Manager = 1,
	WUpdate_End
};

__forceinline uint32 GetMSTime()
{
	using namespace std::chrono;

	static const steady_clock::time_point ApplicationStartTime = steady_clock::now();

	return uint32(duration_cast<std::chrono::milliseconds>(steady_clock::now() - ApplicationStartTime).count());
}

struct IntervalTimer
{
	IntervalTimer() :
		_interval(0), _current(0)
	{

	}

	void Update(time_t diff)
	{
		_current += diff;
		if (_current < 0)
			_current = 0;
	}

	bool Passed()
	{
		return _current >= _interval;
	}

	void ResetTimer()
	{
		if (_current >= _interval)
			_current %= _interval;
	}

	void SetCurrent(time_t current)
	{
		_current = current;
	}

	void SetInterval(time_t interval)
	{
		_interval = interval;
	}

	time_t GetCurrent() const
	{
		return _current;
	}

	time_t GetInterval() const
	{
		return _interval;
	}

private:
	time_t _interval;
	time_t _current;
};

struct TimeTracker
{
public:
	TimeTracker() :
		_expirationTime(0)
	{

	}

	void Update(time_t diff)
	{
		_expirationTime -= diff;
	}

	bool Passed()
	{
		if (_expirationTime <= 0 && !_hasPassed)
		{
			_hasPassed = true;
			return true;
		}
		return false;
	}
	
	void Reset(time_t newExpirationTime)
	{
		_expirationTime = newExpirationTime;
		_hasPassed = false;
	}

private:
	time_t _expirationTime;
	bool _hasPassed;
};

#endif