#ifndef COMMON_H
#define COMMON_H

#include "PlatformDef.h"
#include <ctype.h>
#include <string>

using namespace std;

//! Translate to double degrees for output
double Degree(Int iDegree);
//! Translate to Int degrees for internal usage
Int FromDegree(double dDegree);
//! Check condition and throw exception if false
void Check(bool fCondition);
//! Make double from string
double myatof(const char * str);
double myatof(const wchar_t * str);

/*
	//! Template implementation for swapping values
	template<class T>
	void swap(T &a, T &b)
	{
		T c = a;
		a = b;
		b = c;
	}
*/

//! Template function for square
template<class T>
T sqr(T a)
{
	return a * a;
}

wstring DoubleToText(double dDouble, int iDigits = 1);
wstring DegreeToText(double dDegree, bool fLat);
double TextToDergee(wchar_t * wcText);
wstring IntToText(int iInt);
wstring IntToHex(int iInt);
wstring DistanceToText(double dDistance);
wstring HeightToText(double dDistance);
wstring SpeedToText(double dDistance);
wstring MemoryToText(unsigned long ulMemory);
inline wstring a2w(const char * s)
{
	wchar_t buff[1000];
	swprintf(buff, 1000, L"%S", s);
	return wstring(buff);
}

struct IListAcceptor
{
	virtual void AddItem(const wchar_t * wcLabel, int iId) = 0;
	virtual void UpdateCurrent(const wchar_t * wcLabel) = 0;
};

struct IListAcceptor2
{
	virtual int AddItem(const wchar_t * wcLabel, int iId, int iSubItem, int lParam) = 0;
	virtual void UpdateCurrent(const wchar_t * wcLabel, int iSubItem) = 0;
};

struct IListAcceptor2Acceptor
{
	virtual void UpdateCurrent(int iId, IListAcceptor2 * pAcceptor) = 0;
};

enum enumObjTypes
{
	maskPoints = 0x10,
	maskIndexedPoints = 0x20,
	maskPolylines = 0x40,
	maskPolygons = 0x80
};

template <class T>
T abs(T val)
{
	return (val > T(0)) ? val : -val;
}

inline int int_sqrt(int input)
{
	if (input < 0)
		return 0;
	int nv, v = input>>1, c = 0;
	if (!v)
		return input;
	do
	{
		nv = (v + input/v)>>1;
		if (abs(v - nv) <= 1)
			return nv;
		v = nv;
	}
	while (c++ < 25);
	return nv;
}

int cos100(int degree);
int sin100(int degree);
static const double cdNauticalMile = 1.8522;
static const double cdLandMile = 1.609;
static const double cdFoot = 0.3048;
static const double cdYard = 0.9144;
static const double pi = 3.14159265358979;
std::wstring MakeFilename(const std::wstring & name, const std::wstring & basename);

#define L(x) (GetDict().Translate(L##x))

class Dict
{
public:
	Dict();
	~Dict();
	void Read(const wchar_t * wcFilename);
	wchar_t * Translate(const wchar_t * wcOriginal);
private:
	struct Data;
	Data * m_data;
};

extern Dict & GetDict();

#endif // COMMON_H
