#pragma once
class Stopwatch
{
	long time;
public:
	Stopwatch();
	~Stopwatch();
	void Restart();
	long GetElapsedMilliseconds();
};
