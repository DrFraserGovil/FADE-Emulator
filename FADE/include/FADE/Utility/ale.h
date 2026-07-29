#pragma once

#include <cmath>
inline double ale(double logx, double logy)
{
	if (logx > logy)
	{
		return logx + log1p(exp(logy - logx));
	}
	else
	{
		return logy + log1p(exp(logx - logy));
	}
}
