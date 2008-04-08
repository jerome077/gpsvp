#include "Common.h"
#include "PlatformDef.h"
#include <stdio.h>
#include <math.h>
#include "MapApp.h"
#include <fstream>
// #include <windows.h>

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
	return double(iDegree) / (1 << 24) * 360;
}

Int FromDegree(double dDegree)
{
	// Also very simple
	return Int(dDegree / 360 * (1 << 24));
}

// Make double from string
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
	// Parse string to end
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

// Make double from string
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
	// Parse string to end
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

wstring DistanceToText(double dDistance)
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

wstring SpeedToText(double dSpeed)
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

wstring HeightToText(double dDistance)
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

wstring DoubleToText(double dDouble, int iDigits)
{
	wchar_t wcDouble[1000];
	swprintf(wcDouble, 1000, L"%.*f", iDigits, dDouble);
	return wcDouble;
}

wstring IntToText(int iInt)
{
	wchar_t wcInt[1000];
	swprintf(wcInt, 1000, L"%d", iInt);
	return wcInt;
}

wstring IntToHex(int iInt)
{
	wchar_t wcInt[1000];
	swprintf(wcInt, 1000, L"%x", iInt);
	return wcInt;
}

wstring MemoryToText(unsigned long ulMemory)
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
	enum {original_bits = 23, bits = 12};
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
		int res = int(::cos(::Degree(intDegree) / 180 * 3.14159) * 100);
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
	return mycos100.Guess(degree - (1 << 22));
}

wstring DegreeToText(double dDegree, bool fLat) 
{
	int n1, n2, n3;
	n1 = n2 = n3 = 1;
	const wchar_t * fmt = L"";
	wchar_t S = L'S';
	wchar_t N = L'N';
	wchar_t E = L'E';
	wchar_t W = L'W';
	switch (app.m_riCoordFormat())
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

double TextToDergee(wchar_t * wcText)
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
	return dDegree * sign;
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
		wstring wstr(curr, next - curr);
		curr = next;
		std::wstring::size_type fb, fe, sb, se;
		if (std::wstring::npos == (fb = wstr.find(L'['))) continue;
		++fb;
		if (std::wstring::npos == (fe = wstr.find(L']', fb))) continue;
		if (std::wstring::npos == (sb = wstr.find(L'[', fe))) continue;
		++sb;
		if (std::wstring::npos == (se = wstr.find(L']', sb))) continue;
		wstring f = wstr.substr(fb, fe - fb);
		wstring s = wstr.substr(sb, se - sb);
		m_data->m_map[f] = s;
	}
}