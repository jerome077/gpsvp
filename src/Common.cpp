/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "Common.h"
#include "PlatformDef.h"
#include <stdio.h>
#include <math.h>
#include "MapApp.h"
#include <fstream>
// #include <windows.h>
#include <cassert>

void Check(bool fCondition)
{
#ifdef USE_EXCEPTION
	// TODO: we need some specific exception.
	// Also we need some excepton descripton
	if (!fCondition)
		throw "";
#else
	// TODO: dialog & exit
	if (!fCondition)
	{
		int i = 5;
	}
#endif
}

double Degree(Int iDegree)
{
	// Very simple formula. Nothing to change
	return double(iDegree) * (360. / (1 << GPWIDTH));
}

Int FromDegree(double dDegree)
{
	// Also very simple
	return Int(dDegree / (360. / (1 << GPWIDTH)));
}

// Make double from std::string
double myatof(const char * str)
{
	// Start from tabula rasa
	double res = 0;
	// No decimal point yet
	bool fPoint = false;
	// Starting base
	double base = 1;
	// Skip spaces
	while (*str && isspace(*str))
		++str;
	// Sign?
	double sign = 1;
	switch(*str)
	{
	case '-':
		sign = -1;
	case '+':
		++str;
	}
	// Parse std::string to end
	while (*str)
	{
		// For digits
		if (*str >= '0' && *str <= '9')
		{
			// After decimal point
			if (fPoint)
			{
				// Calculate base for the next digit
				base /= 10;
				// Add the digit to its position
				res += base * (*str - '0');
			}
			else
			{
				// Before decimal point
				// Move previous result left
				res *= 10;
				// Add the digit to the first position
				res += *str - '0';
			}
		}
		else
		{
			// For points
			if (*str == '.')
			{
				// No second point
				// We use only first
				if (fPoint)
					break;
				else
					fPoint = true;
			}
			else
				break; // Unknown char
		}
		// Next char
		++str;
	}
	// Return calculated number
	return res * sign;
}

// Make double from std::string
double myatof(const wchar_t * str)
{
	// Start from tabula rasa
	double res = 0;
	// No decimal point yet
	bool fPoint = false;
	// Starting base
	double base = 1;
	// Skip spaces
	while (*str && isspace(*str))
		++str;
	// Sign?
	double sign = 1;
	switch(*str)
	{
	case L'-':
		sign = -1;
	case L'+':
		++str;
	}
	// Parse std::string to end
	while (*str)
	{
		// For digits
		if (*str >= L'0' && *str <= L'9')
		{
			// After decimal point
			if (fPoint)
			{
				// Calculate base for the next digit
				base /= 10;
				// Add the digit to its position
				res += base * (*str - L'0');
			}
			else
			{
				// Before decimal point
				// Move previous result left
				res *= 10;
				// Add the digit to the first position
				res += *str - L'0';
			}
		}
		else
		{
			// For points
			if (*str == L'.')
			{
				// No second point
				// We use only first
				if (fPoint)
					break;
				else
					fPoint = true;
			}
			else
				break; // Unknown wchar_t
		}
		// Next wchar_t
		++str;
	}
	// Return calculated number
	return res * sign;
}

std::wstring DistanceToText(double dDistance)
{
	wchar_t wcDistance[1000] = {0};
	switch (app.m_riMetrics())
	{
	case 0:
		if (dDistance < 100)
			swprintf(wcDistance, 1000, L"%.1f%s", dDistance, L("m"));
		else if (dDistance < 2000)
			swprintf(wcDistance, 1000, L"%d%s", int(dDistance + 0.5), L("m"));
		else if (dDistance < 20000)
			swprintf(wcDistance, 1000, L"%.1f%s", dDistance / 1000, L("km"));
		else
			swprintf(wcDistance, 1000, L"%d%s", int(dDistance / 1000 + 0.5), L("km"));
		break;
	case 1:
		if (dDistance < cdYard * 1000)
		{
			dDistance /= cdYard;
			swprintf(wcDistance, 1000, L"%d%s", int(dDistance + 0.5), L("yd"));
		}
		else
		{
			dDistance /= cdNauticalMile;
			dDistance /= 1000;
			if (dDistance < 10.0)
				swprintf(wcDistance, 1000, L"%.2f%s", dDistance, L("nm"));
			else if (dDistance < 100.0)
				swprintf(wcDistance, 1000, L"%.1f%s", dDistance, L("nm"));
			else
				swprintf(wcDistance, 1000, L"%d%s", int(dDistance + 0.5), L("nm"));
		}
		break;
	case 2:
		if (dDistance < cdYard * 1000)
		{
			dDistance /= cdYard;
			swprintf(wcDistance, 1000, L"%d%s", int(dDistance + 0.5), L("yd"));
		}
		else
		{
			dDistance /= cdLandMile;
			dDistance /= 1000;
			if (dDistance < 10.0)
				swprintf(wcDistance, 1000, L"%.2f%s", dDistance, L("mi"));
			else if (dDistance < 100.0)
				swprintf(wcDistance, 1000, L"%.1f%s", dDistance, L("mi"));
			else
				swprintf(wcDistance, 1000, L"%d%s", int(dDistance + 0.5), L("mi"));
		}
		break;
	}
	return wcDistance;
}

std::wstring SpeedToText(double dSpeed)
{
	wchar_t wcSpeed[1000] = {0};
	switch (app.m_riMetrics())
	{
	case 0:
		if (dSpeed < 100)
			swprintf(wcSpeed, 1000, L"%.1f%s", dSpeed, L("kmh"));
		else
			swprintf(wcSpeed, 1000, L"%d%s", int(dSpeed), L("kmh"));
		break;
	case 1:
		dSpeed /= cdNauticalMile;
		if (dSpeed < 100)
			swprintf(wcSpeed, 1000, L"%.1f%s", dSpeed, L("kn"));
		else
			swprintf(wcSpeed, 1000, L"%d%s", int(dSpeed), L("kn"));
		break;
	case 2:
		dSpeed /= cdLandMile;
		if (dSpeed < 100)
			swprintf(wcSpeed, 1000, L"%.1f%s", dSpeed, L("mph"));
		else
			swprintf(wcSpeed, 1000, L"%d%s", int(dSpeed), L("mph"));
		break;
	}
	return wcSpeed;
}

std::wstring HeightToText(double dDistance)
{
	wchar_t wcDistance[1000] = {0};
	switch (app.m_riMetrics())
	{
	case 0:
	case 2:
		swprintf(wcDistance, 1000, L"%d%s", int(dDistance + 0.5), L("m"));
		break;
	case 1:
		dDistance /= cdFoot;
		swprintf(wcDistance, 1000, L"%d%s", int(dDistance + 0.5), L("ft"));
		break;
	}
	return wcDistance;
}

std::wstring DoubleToText(double dDouble, int iDigits)
{
	wchar_t wcDouble[1000];
	swprintf(wcDouble, 1000, L"%.*f", iDigits, dDouble);
	return wcDouble;
}

std::string DoubleToStr(double dDouble, int iDigits)
{
	char cDouble[1000];
	_snprintf(cDouble, 1000, "%.*f", iDigits, dDouble);
	return cDouble;
}

std::wstring IntToText(int iInt)
{
	wchar_t wcInt[1000];
	swprintf(wcInt, 1000, L"%d", iInt);
	return wcInt;
}

std::wstring IntToHex(int iInt)
{
	wchar_t wcInt[1000];
	swprintf(wcInt, 1000, L"%x", iInt);
	return wcInt;
}

std::wstring MemoryToText(unsigned long ulMemory)
{
	wchar_t wcMemory[1000];
	if (ulMemory < 1 * 1024)
		swprintf(wcMemory, 1000, L"%u%s", ulMemory, L("b"));
	else if (ulMemory < 1 * 1024 * 1024)
		swprintf(wcMemory, 1000, L"%.1f%s", double(ulMemory) / 1024, L("kb"));
	else
		swprintf(wcMemory, 1000, L"%.1f%s", double(ulMemory) / 1024 / 1024, L("mb"));
	return wcMemory;
}

class MyCos100
{
	enum {original_bits = GPWIDTH - 1, bits = 12};
	signed char table[(1 << bits) + 1];
public:
	MyCos100()
	{
		for (int i = 0; i <= (1 << bits); ++i)
			table[i] = Calculate(Widen(i));
	}
	int Guess(int intDegree)
	{
		int res = table[Narrow(intDegree)];
		return res;
	}
	int Calculate(int intDegree)
	{
		int res = int(::cos(::Degree(intDegree) / 180 * pi) * 100);
		return res;
	}
	int Narrow(int intDegree)
	{
		int res = (abs(intDegree) & ((1 << original_bits) - 1)) >> (original_bits - bits);
		return res;
	}
	int Widen(int intNarrow)
	{
		int res = intNarrow << (original_bits - bits);
		return res;
	}
};

MyCos100 mycos100;

int cos100(int degree)
{
	return mycos100.Guess(degree);
}

int sin100(int degree)
{
	return mycos100.Guess(degree - (1 << (GPWIDTH - 2)));
}

std::wstring DegreeToText(double dDegree, bool fLat, int iCoordFormat) 
{
	int n1, n2, n3;
	n1 = n2 = n3 = 1;
	const wchar_t * fmt = L"";
	wchar_t S = L'S';
	wchar_t N = L'N';
	wchar_t E = L'E';
	wchar_t W = L'W';
	switch (iCoordFormat)
	{
	case 0:
	default:
		n1 = 60;
		n2 = 60;
		fmt = L"%c%d°%d'%d\"";
		break;
	case 1:
		n1 = 60;
		n2 = 10000;
		fmt = L"%c%d°%d.%04d\'";
		break;
	case 2:
		n1 = 100000;
		fmt = L"%c%d.%05d°";
		break;
	case 3:
		n1 = 100000;
		n2 = 1;
		fmt = L"%c%d.%05d°";
		N = E = L'+';
		S = W = L'-';
		break;
	case 4:
		n1 = 60;
		n2 = 60;
		n3 = 100;
		fmt = L"%c%d°%d'%d.%02d\"";
		break;
	};
	bool fNeg = false;
	if (dDegree < 0)
	{
		dDegree = -dDegree;
		fNeg = true;
	}
	wchar_t sign( fLat ? (fNeg ? S : N) : (fNeg ? W : E) );
	int i1 = int(dDegree);
	dDegree -= i1;
	if (n1 == 1 && dDegree > 0.5)
		i1 += 1;
	dDegree *= n1;
	int i2 = int(dDegree);
	dDegree -= i2;
	if (n2 == 1 && dDegree > 0.5)
		i2 += 1;
	dDegree *= n2;
	int i3 = int(dDegree);
	dDegree -= i3;
	if (n3 == 1 && dDegree > 0.5)
		i3 += 1;
	dDegree *= n3;
	int i4 = int(dDegree);
	wchar_t wcDegree[1000];
	swprintf(wcDegree, 1000, fmt, sign, i1, i2, i3, i4);
	return wcDegree;
};

double TextToDegree(const wchar_t * wcText)
{
	double dDegree = 0;
	double dCurrent = 0;
	double base = 1;;
	double sign = 1;
	bool fPoint = false;
	while (wcText && *wcText)
	{
		if (*wcText == L'S' || *wcText == L'W' || *wcText == L'-')
			sign = -1;
		if (iswdigit(*wcText))
		{
			if (fPoint)
			{
				base /= 10;
				dCurrent += (*wcText - L'0') * base;
			}
			else
			{
				dCurrent *= 10;
				dCurrent += (*wcText - L'0');
			}
		}
		if (*wcText == L'.')
		{
			base = 1;
			fPoint = true;
		}
		if (*wcText == L'°')
		{
			dDegree += dCurrent;
			dCurrent = 0;
			fPoint = false;
		}
		if (*wcText == L'\'')
		{
			dDegree += dCurrent / 60;
			dCurrent = 0;
			fPoint = false;
		}
		if (*wcText == L'"')
		{
			dDegree += dCurrent / 3600;
			dCurrent = 0;
			fPoint = false;
		}
		++wcText;
	}
	if (dDegree == 0 && dCurrent != 0)
		dDegree = dCurrent;
	return dDegree * sign;
}

void CoordToText(double dLon, double dLat, std::wstring& wstrLon, std::wstring& wstrLat)
{
	int iCoordFormat = app.m_riCoordFormat();
	if (iCoordFormat <= 4)
	{
		wstrLon = DegreeToText(dLon, false, iCoordFormat);
		wstrLat = DegreeToText(dLat, true, iCoordFormat);
	}
	else if (5 == iCoordFormat) // UTM
	{
		int iUtmX, iUtmY, iUtmZone;
		iUtmZone = app.m_riUTMZone();
		LongLatToUTM(dLon, dLat, iUtmZone, iUtmX, iUtmY);
		wstrLon = IntToText(iUtmX)+L" z"+IntToText(iUtmZone);
		wstrLat = IntToText(iUtmY);
	}
}

void TextToCoord(const std::wstring& wstrLon, const std::wstring& wstrLat, double& dLon, double& dLat)
{
	int iCoordFormat = app.m_riCoordFormat();
	if (iCoordFormat <= 4)
	{
		dLon = TextToDegree(wstrLon.c_str());
		dLat = TextToDegree(wstrLat.c_str());
	}
	else if (5 == iCoordFormat) // UTM
	{
		int iUtmX, iUtmY, iUtmZone;
		int nReadFields1 = swscanf(wstrLon.c_str(), L"%d z%d", &iUtmX, &iUtmZone);
		int nReadFields2 = swscanf(wstrLat.c_str(), L"%d", &iUtmY);
		if ((2 == nReadFields1) && (1 == nReadFields2))
		{
			UTMToLongLat(iUtmX, iUtmY, iUtmZone, dLon, dLat);
		}
		else
		{
			dLon = 0.0;
			dLat = 0.0;
		}
	}
}

std::wstring CoordLabelLon()
{
	int iCoordFormat = app.m_riCoordFormat();
	if (5 == iCoordFormat) // UTM
		return L("UTM Easting, Zone");
	else
		return L("Longitude");
}

std::wstring CoordLabelLat()
{
	int iCoordFormat = app.m_riCoordFormat();
	if (5 == iCoordFormat) // UTM
		return L("UTM Northing");
	else
		return L("Latitude");
}


std::wstring MakeFilename(const std::wstring & name, const std::wstring & basename)
{
	if (name[0] == L'/' || name[0] == L'\\')
		return name;
	int pos = basename.find_last_of(L"\\/");
	if (pos == std::wstring::npos)
		return name;
	return basename.substr(0, pos + 1) + name;
}

struct Dict::Data
{
	typedef std::map<std::wstring, std::wstring> Map;
	Map m_map;
};

Dict::Dict() : m_data(new Data)
{
}

Dict::~Dict()
{
	delete m_data;
}

wchar_t * Dict::Translate(const wchar_t * wcOriginal)
{
	Data::Map::iterator it = m_data->m_map.find(wcOriginal);
	if (it == m_data->m_map.end())
		return const_cast<wchar_t *>(wcOriginal);
	return const_cast<wchar_t *>(it->second.c_str());
}

void Dict::Read(const wchar_t * wcFilename)
{
	FILE * f = wfopen(wcFilename, L"rb");
	if (!f)
		return;
	std::auto_ptr<wchar_t> apBuffer(new wchar_t[100000]);
	wchar_t * buffer = apBuffer.get();;
	buffer[fread(buffer, sizeof(*buffer), 100000, f)] = 0;
	fclose(f);
	wchar_t * curr = buffer;
	while (curr && *curr)
	{
		wchar_t * next = wcschr(curr + 1, L'\n');
		if (!next)
			return;
		std::wstring wstr(curr, next - curr);
		curr = next;
		std::wstring::size_type fb, fe, sb, se;
		if (std::wstring::npos == (fb = wstr.find(L'['))) continue;
		++fb;
		if (std::wstring::npos == (fe = wstr.find(L']', fb))) continue;
		if (std::wstring::npos == (sb = wstr.find(L'[', fe))) continue;
		++sb;
		if (std::wstring::npos == (se = wstr.find(L']', sb))) continue;
		std::wstring f = wstr.substr(fb, fe - fb);
		std::wstring s = wstr.substr(sb, se - sb);
		m_data->m_map[f] = s;
	}
}

bool UTCVariantTimeToLocalVariantTime(double dUTCTime, double &dLocalTime)
{
	dLocalTime = 0;
	SYSTEMTIME stUTCTime;
	if (!VariantTimeToSystemTime(dUTCTime, &stUTCTime)) return false;
	FILETIME ftUTCTime;
	if (!SystemTimeToFileTime(&stUTCTime, &ftUTCTime)) return false;
	FILETIME ftLocalTime;
	if (!FileTimeToLocalFileTime(&ftUTCTime, &ftLocalTime)) return false;
	SYSTEMTIME stLocalTime;
	if (!FileTimeToSystemTime(&ftLocalTime, &stLocalTime)) return false;
	if (!SystemTimeToVariantTime(&stLocalTime, &dLocalTime)) return false;
	return true;
}

bool LocalVariantTimeToUTCVariantTime(double dLocalTime, double &dUTCTime)
{
	dUTCTime = 0;
	SYSTEMTIME stLocalTime;
	if (!VariantTimeToSystemTime(dLocalTime, &stLocalTime)) return false;
	FILETIME ftLocalTime;
	if (!SystemTimeToFileTime(&stLocalTime, &ftLocalTime)) return false;
	FILETIME ftUTCTime;
	if (!FileTimeToLocalFileTime(&ftLocalTime, &ftUTCTime)) return false;
	SYSTEMTIME stUTCTime;
	if (!FileTimeToSystemTime(&ftUTCTime, &stUTCTime)) return false;
	if (!SystemTimeToVariantTime(&stUTCTime, &dUTCTime)) return false;
	return true;
}

// ---------------------------------------------------------------

// Calculation based on explaination from Steven Dutch on http://www.uwgb.edu/dutchs/UsefulData/UTMFormulas.HTM
// The function should work, but it could be a good idea to replace it throug
// the gdal DLL, which supports a lot of other coordanites systems.
void LongLatToUTM(double lon360, double lat360, int& utmZone, double& utmX, double& utmY)
{
	if (0 == utmZone)  // 0 => automatic
	{
		utmZone = (int)floor(lon360/6.0) + 31;
		if (lat360 < 0) utmZone = -utmZone; // south hemisphere
	}
	double lon0_360 = abs(utmZone)*6.0 - 183.0;
	
	double lon628 = lon360 * pi / 180.0;
    double lat628 = lat360 * pi / 180.0;
	double cosLat = cos(lat628);
	double sinLat = sin(lat628);
	double tanLat = tan(lat628);
	double cosLat_2 = cosLat*cosLat;
	double cosLat_3 = cosLat*cosLat_2;
	double cosLat_4 = cosLat_2*cosLat_2;
	double sinLat_2 = sinLat*sinLat;
	double tanLat_2 = tanLat*tanLat;

	double a = 6378137;
	//double b = 6356752.3142;
    //double SinOneSecond = 4.8481368e-6;
	//double SinOneSecond_2 = SinOneSecond*SinOneSecond;
	//double SinOneSecond_3 = SinOneSecond*SinOneSecond_2;
	//double SinOneSecond_4 = SinOneSecond_2*SinOneSecond_2;
	double k0 = 0.9996;
	//double e = sqrt(1-b*b/(a*a));
	double e_2 = 0.0066943800042608137; //e*e;
	double ei2 = 0.0067394967565868897; //e_2/(1-e_2);
	//double n = (a-b)/(a+b);

	double nu = a / sqrt(1 - e_2*sinLat_2);
	double p = (lon360-lon0_360)*3600/10000;
	double p_2 = p*p;

	double Ai = 6367449.1458008448;     //a * (1.0 - n + (5.0/4.0)*(n*n - n*n*n) + (81.0/64.0)*(n*n*n*n - n*n*n*n*n));
	double Bi = 16038.508696860588;     //(1.5*a*n)*(1.0 - n + (7.0/8.0)*(n*n - n*n*n) + (55.0/64.0)*(n*n*n*n - n*n*n*n*n));
	double Ci = 16.832613334334404;     //(15.0*a*n*n/16.0)*(1.0 - n + 0.75*(n*n - n*n*n));
	double Di = 0.021984404202070495;   //(35.0*a*n*n*n/48.0)*(1.0 - n + (11.0/16.0)*(n*n - n*n*n));
	double Ei = 0.00031270521795044842; //(315.0*a*n*n*n*n/51.0)*(1 - n);
	double S = Ai*lat628 - Bi*sin(2*lat628) + Ci*sin(4*lat628) - Di*sin(6*lat628) + Ei*sin(8*lat628);

	double P2 = 0.0011747514329670816;   //1e8* k0 * SinOneSecond_2 *0.5;
	double P3 = 2.3009886108747026e-007; //1e16* k0 * SinOneSecond_4 / 24.0;
	double P3b = 0.060655470809282006;   //9.0 * ei2;
	double P3c = 0.00018168326612818083; //4.0 * ei2*ei2;
	double P4 = 0.048461975452799996;    //1e4* k0 * SinOneSecond;
	double P5 = 1.8984518843401473e-005; //1e12* k0 * SinOneSecond_3 / 6.0;

	double K1 = S * k0;
	double K2 = P2 * nu * sinLat * cosLat;
	double K3 = P3 * nu * sinLat * cosLat_3 *(5 - tanLat_2 + P3b * cosLat_2 + P3c * cosLat_4);
	double K4 = P4 * nu * cosLat;
	double K5 = P5 * nu * cosLat_3 * (1 - tanLat_2 + ei2 * cosLat_2);

    // Northing
	utmY = K1 + K2*p_2 + K3*p_2*p_2;	
	if (utmZone < 0)
		utmY = 10000000 + utmY; 	// south hemisphere
	// Conventional UTM easting
    utmX = K4*p + K5*p*p_2 + 500000;

}

void LongLatToUTM(double lon360, double lat360, int& utmZone, int& utmX, int& utmY)
{
	double dUtmX, dUtmY;
	LongLatToUTM(lon360, lat360, utmZone, dUtmX, dUtmY);
	utmX = (int)floor(dUtmX + 0.5);
	utmY = (int)floor(dUtmY + 0.5);
}

void UTMToLongLat(double utmX, double utmY, int utmZone, double& lon360, double& lat360)
{
	if (utmZone < 0)
		utmY = utmY - 10000000;
	double lon0_360 = abs(utmZone)*6.0 - 183.0;

	double a = 6378137;
	double b = 6356752.3142;
	double k0 = 0.9996;
	//double e = sqrt(1-b*b/(a*a));
	double e_2 = 0.0066943800042608137; //e*e;
	double ei2 = 0.0067394967565868897; //e_2/(1-e_2);

	double P1 = 1.5711160578364014e-007; //1 / k0 / (a * (1.0 - e_2/4.0 - 3.0*e_2*e_2/64.0 - 5.0*e_2*e_2*e_2/256.0));
	double mu = utmY * P1;
	//double s12 = sqrt(1.0 - e_2);
	//double e1 = (1.0 - s12) / (1.0 + s12);

	// footprint latitude
	double J1 = 0.0025188265897211743;   //(3.0 * e1 / 2.0 - 27.0 * e1*e1*e1 / 32.0);
	double J2 = 3.7009490512848485e-006; //(21.0 * e1*e1 / 16.0 - 55.0 * e1*e1*e1*e1 / 32.0);
	double J3 = 7.4478138147885742e-009; //(151.0 * e1*e1*e1 / 96.0);
	double J4 = 1.7035993382806964e-011; //(1097.0 * e1*e1*e1*e1 / 512.0);
	double fp628 = mu + J1*sin(2*mu) + J2*sin(4*mu) + J3*sin(6*mu) + J4*sin(8*mu);
	double fp360 = fp628 * 180.0 / pi;

	double sinFp = sin(fp628);
	double sinFp_2 = sinFp*sinFp;
	double cosFp = cos(fp628);
	double tanFp = tan(fp628);
	double C1 = ei2 * cosFp*cosFp;
	double C1_2 = C1*C1;
	double T1 = tanFp*tanFp;
	double T1_2 = T1*T1;
	double R1 = a * (1-e_2)/pow(1-e_2*sinFp_2, 1.5);
	double N1 = a / sqrt(1 - e_2*sinFp_2);
	double D = (500000-utmX) / (N1 * k0);
	double D_2 = D*D;
	double D_4 = D_2*D_2;
	double D_6 = D_4*D_2;

	double Q1 = N1 * tanFp / R1;
	double Q2 = D_2*0.5;
	double Q3 = (5.0 + 3.0*T1 + 10.0*C1 - 4.0*C1_2 - 9.0*ei2) * D_4 / 24.0;
	double Q4 = (61.0 + 90.0*T1 + 298.0*C1 + 45.0*T1_2 - 3.0*C1_2 - 252.0*ei2) * D_6 / 720.0;
	double Q5 = D;
	double Q6 = (1.0 + 2.0*T1 + C1) * D_2*D / 6.0;
	double Q7 = (5.0 - 2.0*C1 + 28.0*T1 - 3.0*C1_2 + 8.0*ei2 + 24.0*T1_2) * D_4*D / 120.0;

	lat360 = fp360 - (Q1 * (Q2 - Q3 + Q4))*180.0/pi;
	lon360 = lon0_360 - ((Q5 - Q6 + Q7)/cosFp)*180.0/pi;
}

void TestUTM()
{
	int iUtmX, iUtmY, iUtmZone = 0;
	double lon, lat;
	
	iUtmZone = 0;
	LongLatToUTM(11, 48, iUtmZone, iUtmX, iUtmY);
	assert(32 == iUtmZone);
	assert(649188 == iUtmX);
	assert(5318236 == iUtmY);
	UTMToLongLat(iUtmX, iUtmY, iUtmZone, lon, lat);
	assert(fabs(lon-11) < 0.00001);
	assert(fabs(lat-48) < 0.00001);

	iUtmZone = 32;
	LongLatToUTM(11, 48, iUtmZone, iUtmX, iUtmY);
	assert(32 == iUtmZone);
	assert(649188 == iUtmX);
	assert(5318236 == iUtmY);
	UTMToLongLat(iUtmX, iUtmY, iUtmZone, lon, lat);
	assert(fabs(lon-11) < 0.00001);
	assert(fabs(lat-48) < 0.00001);

	iUtmZone = 0;
	LongLatToUTM(-11, 48, iUtmZone, iUtmX, iUtmY);
	assert(29 == iUtmZone);
	assert(350812 == iUtmX);
	assert(5318236 == iUtmY);
	UTMToLongLat(iUtmX, iUtmY, iUtmZone, lon, lat);
	assert(fabs(lon-(-11)) < 0.00001);
	assert(fabs(lat-48) < 0.00001);

	iUtmZone = 0;
	LongLatToUTM(11, -48, iUtmZone, iUtmX, iUtmY);
	assert(-32 == iUtmZone);
	assert(649188 == iUtmX);
	assert(4681764 == iUtmY);
	UTMToLongLat(iUtmX, iUtmY, iUtmZone, lon, lat);
	assert(fabs(lon-11) < 0.00001);
	assert(fabs(lat-(-48)) < 0.00001);

	iUtmZone = -32;
	LongLatToUTM(11, -48, iUtmZone, iUtmX, iUtmY);
	assert(-32 == iUtmZone);
	assert(649188 == iUtmX);
	assert(4681764 == iUtmY);
	UTMToLongLat(iUtmX, iUtmY, iUtmZone, lon, lat);
	assert(fabs(lon-11) < 0.00001);
	assert(fabs(lat-(-48)) < 0.00001);

	iUtmZone = 32;
	LongLatToUTM(11, -48, iUtmZone, iUtmX, iUtmY);
	assert(32 == iUtmZone); // forced to north hemisphere although it is in the south
	assert(649188 == iUtmX);
	assert(-5318236 == iUtmY);
	UTMToLongLat(iUtmX, iUtmY, iUtmZone, lon, lat);
	assert(fabs(lon-11) < 0.00001);
	assert(fabs(lat-(-48)) < 0.00001);

	iUtmZone = 0;
	LongLatToUTM(-11, -48, iUtmZone, iUtmX, iUtmY);
	assert(-29 == iUtmZone);
	assert(350812 == iUtmX);
	assert(4681764 == iUtmY);
	UTMToLongLat(iUtmX, iUtmY, iUtmZone, lon, lat);
	assert(fabs(lon-(-11)) < 0.00001);
	assert(fabs(lat-(-48)) < 0.00001);

	iUtmZone = 0;
	LongLatToUTM(11.5, 48.5, iUtmZone, iUtmX, iUtmY);
	assert(32 == iUtmZone);
	assert(684673 == iUtmX);
	assert(5374894 == iUtmY);
	UTMToLongLat(iUtmX, iUtmY, iUtmZone, lon, lat);
	assert(fabs(lon-11.5) < 0.00001);
	assert(fabs(lat-48.5) < 0.00001);
}

// ---------------------------------------------------------------

std::wstring UTMZoneToLongText(int utmZone)
{
	int lon0_360 = abs(utmZone)*6 - 183;
	wchar_t bufDegrees[128];
	wsprintf(bufDegrees, L("%d° to %d°"), lon0_360-3, lon0_360+3);
	std::wstring sHemisphere = (utmZone > 0) ? L("north hemisphere") : L("south hemisphere");
	return IntToText(utmZone) + L" (" + bufDegrees + L", " + sHemisphere + L")";
}

// ---------------------------------------------------------------


