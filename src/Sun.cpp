/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/*********************************************************
 * sun.cpp
 * ������� ��� ����������� ������� � ������ ������
 * �����: ������� ��������� 
 * e-mail: support@lavresearch.com
 * www: http://lavresearch.com
 * ��������� ���������: 31 ����� 2005 �.
 *********************************************************/

#include "Sun.h"
#include "Commands.h"
#include "MapApp.h"
#include <time.h>
#include <math.h>

#define	PI		(3.1415926535897932384626433832795)
#define C_T		36525.0

// ��������� ��� ����������� ������������ SK ��� ������
double AskS[53];
// ��������� ��� ����������� ������������ CK ��� ������
double AckS[49];
// ��������� ��� ����������� ������������ SR ��� ������
double AsrS[49];
// ��������� ��� ����������� ������������ CR ��� ������
double AcrS[53];

// ������������ ��� ����������� ��������� ������
// ���� ����������� ����������
double sk1[245] = {//4*5  // ����� 49*5
					 0, 0, 0, 0, 0,
					 1, 0, 0, 0, 0,
					 2, 0, 0, 0, 0,
					 3, 0, 0, 0, 0,

// ���� ���������� �� ������ //15*5
					 0, 1, 0, 0, 0,
					 1,-1, 0, 0, 0,
					 1,-2, 0, 0, 0,
					 2,-2, 0, 0, 0,
					 3,-2, 0, 0, 0,
					 3,-3, 0, 0, 0,
					 4,-3, 0, 0, 0,
					 5,-3, 0, 0, 0,
					 4,-4, 0, 0, 0,
					 5,-4, 0, 0, 0,
					 6,-4, 0, 0, 0,
					 5,-5, 0, 0, 0,
					 6,-6, 0, 0, 0,
					 7,-5, 0, 0, 0,
					 8,-5, 0, 0, 0,
// ���� ���������� �� ����� //10*5
					 1, 0,-1, 0, 0,
					 2, 0,-2, 0, 0,
					 1, 0,-2, 0, 0,
					 2, 0,-3, 0, 0,
					 2, 0,-4, 0, 0,
					 3, 0,-4, 0, 0,
					 3, 0,-5, 0, 0,
					 3, 0,-3, 0, 0,
					 4, 0,-3, 0, 0,
					 4, 0,-5, 0, 0,
// ���� ���������� �� ������� //12*5
					 0, 0, 0, 1, 0,
					 1, 0, 0,-3, 0,
					 1, 0, 0,-2, 0,
					 1, 0, 0,-1, 0,
					 1, 0, 0, 1, 0,
					 2, 0, 0,-4, 0,
					 2, 0, 0,-3, 0,
					 2, 0, 0,-2, 0,
					 2, 0, 0,-1, 0,
					 3, 0, 0,-4, 0,
					 3, 0, 0,-3, 0,
					 3, 0, 0,-2, 0,
// ���� ���������� �� ������� //4*5
					 0, 0, 0, 0, 1,
					 1, 0, 0, 0,-2,
					 1, 0, 0, 0,-1,
					 2, 0, 0, 0,-2,
// ���� ��������� ���������� //4*5
					 0, 0, 0, 0, 1,
					 1, 0, 0, 0,-2,
					 1, 0, 0, 0,-1,
					 2, 0, 0, 0,-2};
// ���� ���������� �� ����
double sk2[16] = {//4*4
					 0, 0, 0, 1,
					 1, 0, 0, 1,
					 1, 0, 0,-1,
					 0, 1, 0,-1};


/*********************************************************
 **������**
 * ��������� �����. ��� ���������� ��������� ������
 * t - ��������� ����� �� J2000
 *********************************************************/
void FillAmS(double t)
{
	// ��������� ��� ����������� ������������ SK ��� ������
//double AskS[49];
// ��������� ��� ����������� ������������ CK ��� ������
//double A�kS[49];
// ��������� ��� ����������� ������������ SR ��� ������
//double AsrS[49];
// ��������� ��� ����������� ������������ CR ��� ������
//double A�rS[49];
	double	T = (t+36525)/C_T;//+ ������� �� ����� �� ����� 1900 �.
	AskS[0] = 0;								AckS[0] = 0;AcrS[0] = 30570e-9-150e-9*T;				AsrS[0] = 0;
	AskS[1] = 33502e-6-83.58e-6*T-0.25e-6*T*T;	AckS[1] = 0;AcrS[1] =-7274120e-9+18140e-9*T+5e-9*T*T;	AsrS[1] = 0;
	AskS[2] = 351e-6-1.75e-6*T;					AckS[2] = 0;AcrS[2] =-91380e-9+460e-9*T;				AsrS[2] =0;
	AskS[3] = 5e-6;								AckS[3] = 0;AcrS[3] =-1450e-9+10e-9*T;					AsrS[3] =0;
	
	AskS[4] = 0;		AckS[4] = 0;		AsrS[4] = 0;		AcrS[4] = 85e-9;
	AskS[5] =-20e-6;	AckS[5] = 11e-6;	AsrS[5] =-1146e-9;	AcrS[5] =-2062e-9;
	AskS[6] = 0;		AckS[6] = 0;		AsrS[6] = 136e-9;	AcrS[6] = 84e-9;
	AskS[7] = 14e-6;	AckS[7] =-23e-6;	AsrS[7] = 5822e-9;	AcrS[7] = 3593e-9;
	AskS[8] =-8e-6;		AckS[7] = 9e-6;		AsrS[8] =-632e-9;	AcrS[8] =-596e-9;
	AskS[9] = 0;		AckS[8] =-3e-6;		AsrS[9] = 1044e-9;	AcrS[9] = 0;
	AskS[10] =-2e-6;	AckS[10] = 7e-6;	AsrS[10] =-1448e-9;	AcrS[10] =-381e-9;
	AskS[11] =-3e-6;	AckS[11] = 4e-6;	AsrS[11] = 148e-9;	AcrS[11] = 126e-9;
	AskS[12] = 0;		AckS[12] =-1e-6;	AsrS[12] = 337e-9;	AcrS[12] =-166e-9;
	AskS[13] = 0;		AckS[13] = 0;		AsrS[13] = 189e-9;	AcrS[13] = 0;
	AskS[14] = 0;		AckS[14] = 1e-6;	AsrS[14] =-91e-9;	AcrS[14] = 0;
	AskS[15] = 0;		AckS[15] = 0;		AsrS[15] = 93e-9;	AcrS[15] =-134e-9;
	AskS[16] = 0;		AckS[16] = 0;		AsrS[16] = 0e-9;	AcrS[16] =-80e-9;
	AskS[17] = 0;		AckS[17] = 0;		AsrS[17] = 136e-9;	AcrS[17] = 0;
	AskS[18] = 0;		AckS[18] = 1e-6;	AsrS[18] = 0;		AcrS[18] = 0;
	
	AskS[19] = 1e-6;	AckS[19] =-1e-6;	AsrS[19] =-119e-9;	AcrS[19] =-92e-9;
	AskS[20] = 3e-6;	AckS[20] = 10e-6;	AsrS[20] = 1976e-9;	AcrS[20] =-573e-9;
	AskS[21] = 3e-6;	AckS[21] =-8e-6;	AsrS[21] = 137e-9;	AcrS[21] = 0;
	AskS[22] = 1e-6;	AckS[22] = 2e-6;	AsrS[22] = 201e-9;	AcrS[22] =-77e-9;
	AskS[23] = 1e-6;	AckS[23] = 3e-6;	AsrS[23] =-96e-9;	AcrS[23] = 0;
	AskS[24] =-2e-6;	AckS[24] = 0;		AsrS[24] =-125e-9;	AcrS[24] = 461e-9;
	AskS[25] =-1e-6;	AckS[25] = 0;		AsrS[25] = 0;		AcrS[25] = 87e-9;
	AskS[26] = 0;		AckS[26] = 0;		AsrS[26] = 0;		AcrS[26] =-154e-9;
	AskS[27] = 0;		AckS[27] = 0;		AsrS[27] =-94e-9;	AcrS[27] =-102e-9;
	AskS[28] = 0;		AckS[28] = 0;		AsrS[28] = 0;		AcrS[28] = 87e-9;

	AskS[29] =-13e-6;	AckS[29] =-1e-6;	AsrS[29] =-89e-9;	AcrS[29] = 227e-9;
	AskS[30] =-1e-6;	AckS[30] = 0;		AsrS[30] = 0;		AcrS[30] = 172e-9;
	AskS[31] =-7e-6;	AckS[31] =-3e-6;	AsrS[31] =-486e-9;	AcrS[31] = 1376e-9;
	AskS[32] = 0;		AckS[32] =-35e-6;	AsrS[32] =-7067e-9;	AcrS[32] = 0;
	AskS[33] = 0;		AckS[33] = 0;		AsrS[33] = 0;		AcrS[33] = 79e-9;
	AskS[34] = 0;		AckS[34] = 0	;	AsrS[34] = 0;		AcrS[34] = 110e-9;
	AskS[35] =-3e-6;	AckS[35] = 0;		AsrS[35] = 104e-9;	AcrS[35] = 796e-9;
	AskS[36] =-13e-6;	AckS[36] = 0;		AsrS[36] = 203e-9;	AcrS[36] = 4021e-9;
	AskS[37] = 0;		AckS[37] =-1e-6;	AsrS[37] =-193e-9;	AcrS[37] =-78e-9;
	AskS[38] = 0;		AckS[38] = 0;		AsrS[38] =-73e-9;	AcrS[38] = 0;
	AskS[39] = 0;		AckS[39] =-1e-6;	AsrS[39] =-278e-9;	AcrS[39] = 0;
	AskS[40] = 0;		AckS[40] = 0;		AsrS[40] = 0;		AcrS[40] = 102e-9;

	AskS[41] =-2e-6;	AckS[41] = 0;		AsrS[41] = 0;		AcrS[41] = 0;
	AskS[42] = 0;		AckS[42] = 0;		AsrS[42] = 0;		AcrS[42] =-103e-9;
	AskS[43] =-2e-6;	AckS[43] = 0;		AsrS[43] =-79e-9;	AcrS[43] = 422e-9;
	AskS[44] = 0;		AckS[44] = 0;		AsrS[44] = 0;		AcrS[44] =-152e-9;

	AskS[45] = 0;		AckS[45] = 0;		AsrS[45] = 0;		AcrS[45] = 91e-9;
	AskS[46] = 25e-6;	AckS[46] = 18e-6;	AsrS[46] = 0;		AcrS[46] = 0;
	AskS[47] = 0;		AckS[47] = 0;		AsrS[47] = 0;		AcrS[47] =-91e-9;
	AskS[48] = 0;		AckS[48] =-1e-6;	AsrS[48] = 0;		AcrS[48] = 0;
	// ���������� �� ����
	AskS[49] = 31e-6;											AcrS[49] = 13360e-9;
	AskS[50] = 1e-6;											AcrS[50] = 0;
	AskS[51] = 2e-6;											AcrS[51] =-1330e-9;
	AskS[52] =-1e-6;											AcrS[52] = 0;

}



/**************************************************
 *	������� ����������� ���� � ��������� �����
 * �� 2000 ���� 1�� ����� 12 �����
 * return: ����� ��������� ���� (�� ������ 2000�.)
 **************************************************/
double getJG(struct tm *newtime)
{
	double km = 12*(newtime->tm_year+1900+4800)+newtime->tm_mon-2;// � �����
	double vj = (2*(km-12*floor(km/12))+7+365*km)/12;
	vj = floor(vj)+newtime->tm_mday+floor(km/48)-32083;
	if (vj>2299171)
		vj += floor(km/4800)-floor(km/1200)+38;
	vj += -2451545+
	(newtime->tm_hour/24.)+
	(newtime->tm_min/(24.*60))+
	(newtime->tm_sec/(24*3600.))-0.5;
	return vj;
}

/*********************************************************
 * �������� ������ ��������� � �������� �����
 * ��� ����� �������         � �������� 
 *********************************************************/
double  getE(double T)
{
	double ret;
	double	T1 = T/C_T;
	ret = (84381.448-46.8150*T1-0.00059*T1*T1+0.008813*T1*T1*T1)/3600.0;
	ret = fmod(ret,360.0);
	if (ret<0)	ret += 360.0;
	return	ret;
}

/*********************************************************
 * ������� �������� ���� � �������� 
 * ����� J2000
 *********************************************************/
double  retlJ(double T)
{
	double ret;
	double	T1 = T/C_T;
	ret = 485866.733+ (1325*1296000+715922.633)*T1+31.310*T1*T1+0.064*T1*T1*T1;
	ret /= 3600;//������� � �������
	ret = fmod(ret,360.0);
	if (ret<0)	ret += 360.0;
	return	ret;
}

/*********************************************************
 * ������� �������� C����� � �������� 
 * ����� J2000
 *********************************************************/
double  retl1J(double T)
{
	double ret;
	double	T1 = T/C_T;
	ret = 1287099.804+ (99*1296000+1292581.224)*T1-0.577*T1*T1-0.012*T1*T1*T1;
	ret /= 3600;//������� � �������
	ret = fmod (ret,360.0);
	if (ret<0)	ret += 360.0;
	return	ret;
}

/*********************************************************
 **������** - ������� ������� � �������� 
 * ����� J2000
 *********************************************************/
double  retl_Wn(double T)
{
	double ret;
	double	T1 = T/C_T;
	ret =181*3600+58*60+47.283+210669166.909*T1+1.1182*T1*T1+0.0001*T1*T1*T1;
	ret/=3600;// ������� � ������� �� ������
	ret = fmod(ret,360.0);
	if (ret<0)	ret += 360.0;
	return	ret;
}

/*********************************************************
 **������** - ������� ������� ������� � �������� 
 * ����� J2000
 *********************************************************/
double  retp_Wn(double T)
{
	double ret;
	double	T1 = T/C_T;
	ret =131*3600+33*60+49.346+5047.994*T1-3.8618*T1*T1-0.0189*T1*T1*T1;
	ret/=3600;// ������� � ������� �� ������
	ret = fmod(ret,360.0);
	if (ret<0)	ret += 360.0;
	return	ret;
}

/*********************************************************
 **����** - ������� ������� � �������� 
 * ����� J2000
 *********************************************************/
double  retl_Mar(double T)
{
	double ret;
	double	T1 = T/C_T;
	ret =355*3600+25*60+59.789+68910107.309*T1+1.1195*T1*T1+0.0001*T1*T1*T1;
	ret/=3600;// ������� � ������� �� ������
	ret = fmod(ret,360.0);
	if (ret<0)	ret += 360.0;
	return	ret;
}

/*********************************************************
 **����** - ������� ������� ������� � �������� 
 * ����� J2000
 *********************************************************/
double  retp_Mar(double T)
{
	double ret;
	double	T1 = T/C_T;
	ret =336*3600+3*60+36.842+6627.759*T1+0.4864*T1*T1+0.0010*T1*T1*T1;
	ret/=3600;// ������� � ������� �� ������
	ret = fmod(ret,360.0);
	if (ret<0)	ret += 360.0;
	return	ret;
}

/*********************************************************
 **������** - ������� ������� � �������� 
 * ����� J2000
 *********************************************************/
double  retl_Jup(double T)
{
	double ret;
	double	T1 = T/C_T;
	ret =34*3600+21*60+5.342+10930690.040*T1+0.8055*T1*T1+0.0001*T1*T1*T1;
	ret/=3600;// ������� � ������� �� ������
	ret = fmod(ret,360.0);
	if (ret<0)	ret += 360.0;
	return	ret;
}

/*********************************************************
 **������** - ������� ������� ������� � ��������
 * ����� J2000
 *********************************************************/
double  retp_Jup(double T)
{
	double ret;
	double	T1 = T/C_T;
	ret =14*3600+19*60+52.713+5805.497*T1+3.7132*T1*T1-0.0159*T1*T1*T1;
	ret/=3600;// ������� � ������� �� ������
	ret = fmod(ret,360.0);
	if (ret<0)	ret += 360.0;
	return	ret;
}


/*********************************************************
 **������** - ������� ������� � ��������
 * ����� J2000
 *********************************************************/
double  retl_Sat(double T)
{
	double ret;
	double	T1 = T/C_T;
	ret =50*3600+4*60+38.897+4404639.651*T1+1.8703*T1*T1;
	ret/=3600;// ������� � ������� �� ������
	ret = fmod(ret,360.0);
	if (ret<0)	ret += 360.0;
	return	ret;
}

/*********************************************************
 **������** - ������� ������� ������� � ��������
 * ����� J2000
 *********************************************************/
double  retp_Sat(double T)
{
	double ret;
	double	T1 = T/C_T;
	ret =93*3600+3*60+24.434+7069.538*T1+3.0150*T1*T1+0.0181*T1*T1*T1;
	ret/=3600;// ������� � ������� �� ������
	ret = fmod(ret,360.0);
	if (ret<0)	ret += 360.0;
	return	ret;
}

/*********************************************************
 * ������� �������� ������ ����, � ��������
 * ����� J2000
 *********************************************************/
double  retFJ(double T)
{
	double ret;
	double	T1 = T/C_T;
	ret = 335778.877+ (1342*1296000+295263.137)*T1-13.257*T1*T1+0.011*T1*T1*T1;
	ret /= 3600;//������� � �������
	ret = fmod (ret,360.0);
	if (ret<0)	ret += 360.0;
	return	ret;
}

/*********************************************************
 * �������� ������� ������ ���� � ������ (���������)� ��������
 * ����� J2000
 *********************************************************/
double  retDJ(double T)
{
	double ret;
	double	T1 = T/C_T;
	ret = 1072261.307+ (1236*1296000+1105601.328)*T1-6.891*T1*T1+0.019*T1*T1*T1;
	ret /= 3600;//������� � �������
	ret = fmod (ret,360.0);
	if (ret<0)	ret += 360.0;
	return	ret;
}

/*********************************************************
 * ������� ������� ������ - � ��������
 * ����� J2000
 *********************************************************/
double  retl_Sl(double T)
{
	double ret;
	double	T1 = T/C_T;
	ret =1009667.850+(129600000+2771.270)*T1+1.089*T1*T1;
	ret/=3600;// ������� � ������� �� ������
	ret = fmod(ret,360.0);
	if (ret<0)	ret += 360.0;
	return	ret;
}

/*********************************************************
 * ��������� ������� ������ 
 * (� ��������) (������ �� +(2-8) ������) 
 * t - ��������� ����� �� J2000
 *********************************************************/
double get_lon(double t)
{
	double	T = t/C_T;
	double	ret=0,
			vl=0,
			lgr=0;
	double	l,l1,D,F,
			g1,g3,g4,g5;
	// ������� �������� ����
	l = retlJ(t);
	// ������� �������� C�����
	l1= retl1J(t);
	// ������� �������� ������
	g1= retl_Wn(t)-retp_Wn(t);
	// ������� �������� �����
	g3= retl_Mar(t)-retp_Mar(t);
	// ������� �������� �������
	g4= retl_Jup(t)-retp_Jup(t);
	// ������� �������� �������
	g5= retl_Sat(t)-retp_Sat(t);
	l  *= PI/180.0;
	l1 *= PI/180.0;
	g1 *= PI/180.0;
	g3 *= PI/180.0;
	g4 *= PI/180.0;
	g5 *= PI/180.0;
	FillAmS(t);
	for (int i=0;i<49;i++)
	{
		vl += *(AskS+i)*sin((*(sk1+i*5+0))*l1+
							(*(sk1+i*5+1))*g1+
							(*(sk1+i*5+2))*g3+
							(*(sk1+i*5+3))*g4+
							(*(sk1+i*5+4))*g5);
		vl += *(AckS+i)*cos((*(sk1+i*5+0))*l1+
							(*(sk1+i*5+1))*g1+
							(*(sk1+i*5+2))*g3+
							(*(sk1+i*5+3))*g4+
							(*(sk1+i*5+4))*g5);
	}
	double a = 13*l1-8*g1+3.8990655+0.0785398*(t+36525)/C_T;
	vl += 6e-6*cos(a)+7e-6*sin(a);
	// ������� �������� ������ ����
	F = retFJ(t);
	// �������� ������� ������ ���� � ������
	D = retDJ(t);
	F *= PI/180.0;
	D *= PI/180.0;
	for (int i=0;i<4;i++)
	{
		vl +=  *(AskS+i+49)*sin((*(sk2+i*4+0))*l+
								(*(sk2+i*4+1))*l1+
								(*(sk2+i*4+2))*F+
								(*(sk2+i*4+3))*D);
	}
	ret = retl_Sl(t);// ������� ������� ������
	vl *= 180/PI;
	ret+= vl;
	ret = fmod(ret,360);
	if (ret<0)	ret+=360;
	return ret;

}

/*********************************************************
 * ��������� ������-������� ������ � ������� ����������� ��
 * ����� �� ������ (�� 5 ����� ����� �������)
 * (� ��������������� ��������)
 * t - ��������� ����� �� J2000
 *********************************************************/
double get_ri(double t)
{
	double	T = t/C_T;
	double	ret=0,
			vl=0,
			lgr=0;
	double	l,l1,D,F,
			g1,g3,g4,g5;
	// ������� �������� ����
	l = retlJ(t);
	// ������� �������� C�����
	l1= retl1J(t);
	// ������� �������� ������
	g1= retl_Wn(t)-retp_Wn(t);
	// ������� �������� �����
	g3= retl_Mar(t)-retp_Mar(t);
	// ������� �������� �������
	g4= retl_Jup(t)-retp_Jup(t);
	// ������� �������� �������
	g5= retl_Sat(t)-retp_Sat(t);
	l  *= PI/180.0;
	l1 *= PI/180.0;
	g1 *= PI/180.0;
	g3 *= PI/180.0;
	g4 *= PI/180.0;
	g5 *= PI/180.0;
	FillAmS(t);
	for (register int i=0;i<49;i++)
	{
		vl += *(AsrS+i)*sin((*(sk1+i*5+0))*l1+
							(*(sk1+i*5+1))*g1+
							(*(sk1+i*5+2))*g3+
							(*(sk1+i*5+3))*g4+
							(*(sk1+i*5+4))*g5);
		vl += *(AcrS+i)*cos((*(sk1+i*5+0))*l1+
							(*(sk1+i*5+1))*g1+
							(*(sk1+i*5+2))*g3+
							(*(sk1+i*5+3))*g4+
							(*(sk1+i*5+4))*g5);
	}
	// ������� �������� ������ ����
	F = retFJ(t);
	// �������� ������� ������ ���� � ������
	D = retDJ(t);
	F *= PI/180.0;
	D *= PI/180.0;
	for (int i=0;i<4;i++)
	{
		vl +=  *(AcrS+i+49)*cos((*(sk2+i*4+0))*l+
								(*(sk2+i*4+1))*l1+
								(*(sk2+i*4+2))*F+
								(*(sk2+i*4+3))*D);
	}
	//vl /= (3600);
	ret = ::pow((double)10.0,(double)vl);
	ret = pow((double)10.0,(double)vl);
	//if (ret<0)	ret+=360;
	return ret;
}

/**********************************************
 * ������������ �������� ����� ��
 * ��������� ��������
 * ����:
 * t - ����� ��������� ���� �� J2000
 * �����: �������� ����� � �����
 **********************************************/

double	star_time(double t)
{
	double	t0, // ����� ��������� ���� �� ������������ ��������
			so, // ������������ ����� �� �������� � �����
			tmp,
			M,M1; // ������������ ����� �� �������� � �����
	
	// ����� �� ������ �����
	M1 = t-(int)t;
	if (M1>=0.5)
	{
		M = M1-0.5;
		t0 = (int)(t)+0.5;// ����� ��������� ���� �� ������������ ��������
	}
	if (M1<0.5)
	{
		M = M1+0.5;
		t0 = (int)(t)-0.5;// ����� ��������� ���� �� ������������ ��������
	}
	M *= 24;// ������� � ����;
	// �������� ����� � ������������ ������� �������� ���
	tmp = t0/36525.0;//tmp = (t0-2415020.0)/36525.0;
	so = (21600+2460+50.54841+8640184.812866*tmp+0.093104*tmp*tmp-6.2*tmp*tmp*tmp);// � ��������
	// ����� ������� ����� ��. ����, �� ��� �� �� ��������, � �� ���. �����
	tmp = t/36525.0;
	// ������� �������
	//double na1 = (cos(rete(tmp*36525.0)*PI/180.0)/15.0)*retDfJ(tmp*36525.0);
	//double na2 = (cos(rete(tmp*36525.0)*PI/180.0)/15.0)*retdfJ(tmp*36525.0);
	//so += (cos(rete(tmp*36525.0)*PI/180.0)/15.0)*retDfJ(tmp*36525.0);
	//so += (cos(rete(tmp*36525.0)*PI/180.0)/15.0)*retdfJ(tmp*36525.0);
	so /= 3600.0; // � �����
	so += M*1.0027379093;// ���� ������� ����� �� ������������ ��������
	so = fmod(so, 24.0);
	if (so<0)	so+=24;
	return so;
}

/*********************************************************
 * ��������� �������� ������� ����������� � ���������
 * �� ������ � �������
 * La - ������ - ������ ������ ������������ - � ��������
 * Lo - ������� - ������ ����������  - � ��������
 * //t - ��������� ����� J2000
 * e - ���� ��������( ������ ���������) - � ��������
 *********************************************************/
void get_LaLo(double& La,double& Lo,double e)
{
	double	z;
	double	f,// ������
			t,// ������� ����
			l,// ���������
			A;// 
			//a;// ����. �����������

	f = 90-e;
	t = 90-Lo;
	l = La;
	
	f *= (PI/180.0);
	t *= (PI/180.0);
	l *= (PI/180.0);

	z = sin(f)*sin(l)+cos(f)*cos(l)*cos(t);
	//ASSERT(z>=-1 && z<=1);
	double z1 = acos(z);
	z1 = acos(z);// ������, ����� �� ��������!!!
	z1 = z1*(180.0/PI);
	Lo = 90-z1;// ���������
	Lo = fmod(Lo,360);
	if (Lo<0)	Lo += 360;

	A = atan2((cos(l)*sin(t)),(-sin(l)*cos(f)+cos(l)*sin(f)*cos(t)));
	A *= (180.0/PI);
	La = 90-A;
	La = fmod(La,360);
	if (La<0)	La += 360;
}

// ���������� ������� � ������ ������ (����� ����� �������)
// ����:
// loc_time : ��������� ����� (����� ����� �������� ���)
// time_zone : ������� ����� �� �������� (�������� ��� ������ �����=4, �����=5)
// f, l : ������ � ������� �������������� (� ��������)
// �����:
// t_rise : ��������� �� �����, ������� ����� ��������� ����� ������� ������ (� �����)
// t_set  : ��������� �� �����, ������� ����� ��������� ����� ������ ������ (� �����)
// ����������: 
// 1 : ��� ��
//-1 : ������� �� ������� (���� ���� ����� ��� �������� ����)
//-2 : ������� �� ������� (���� ���� ������ ��� �������� ����)

int sun_rise_set(struct tm *loc_time,double time_zone,double f, double l, double *t_rise,double *t_set)
{
	double t0;// ��������� ����� � ����������� �������
	double t;// ������� ��������� �����
	double lat,lon;// ������, ������� ������
	//double h,A;// ������, ������
	double e;// ������ ��������� � ��������
	double r;// ������������� ������-������ ~1
	double s1,s2;// �������� ����� ������� � ������
	double s0;//�������� �����
	double	z;//�������� ���������� ���������
	double dz;//������� � ��������� ���������� ���������
	double cost;// �������� �������� (�.�. > 1!)
	double	v = 1-0.0027304336;// ��� �������� �� ��������� � ������� �����

	// ��������� �� ����� �� �������� ���
	loc_time->tm_hour = 12-(int)time_zone;
	loc_time->tm_min = 0;
	loc_time->tm_sec = 0;
	// ���������� ��������� ����� (�� �������� �������� ���)
	t = getJG(loc_time);

	// ��������� �� ����� �� ����������� �������
	loc_time->tm_hour = 0;
	// ���������� ��������� ����� (�� ����������� �������)
	t0 = getJG(loc_time);

	// ���������� ��������� ������ �� ���������
	e = getE(t);// ������ ��������� � ��������
	lon = get_lon(t);// ������� ������ - longitude
	lat = 0;// ������ ������ - latitude
	r = get_ri(t);// ������������� ������-������ ~1
	
	//��������� ������� � ��������� ���������� ���������
	dz = 0;
	dz+= 34.5/60.;// ���������=35'
	//dz+= d;// ������ �����
	dz+= (961.18/r)/3600.;// ������� ������ ������ ~ 16'
	dz-= (8.794/r)/3600.;// ��������� ~ 9"
	z = 90+dz;
	
	// ����� �������� ������� ����������� � ���������
	// lat: ������  -> ������ ����������� (� ��������)
	// lon: ������� -> ��������� (� ��������)
	get_LaLo(lat,lon,e);
	lat/=15;// ��������� ������ ����������� �� �������� � ����
	

	//��������� ������� �������� �����
	s0 = star_time(t0)*15;
	s0 += l;//������� �������� �����
	if (s0<0)	s0+=360;
	s0 = fmod(s0,360)/15.;// ������� � ����
	
	// ������� � �������
	lon *= (PI/180.0);
	z *= (PI/180.0);
	f *= (PI/180.0);
	cost = (cos(z)-sin(f)*sin(lon))/(cos(f)*cos(lon));
	if (cost>1)// ������ �� �������
		return -1;
	if (cost<-1)// ������ �� �������
		return -2;
	// �������� ������� �� ��������� �������
	double dt = acos(cost);
	dt *= (12./PI);// ��������� � ���� �� ������
	if (dt<0)	dt+=24;
	if (dt<=12)
	{
		s1  = lat-dt;
		s2 = lat+dt;
	}
	else
	{
		s1  = lat+dt;
		s2 = lat-dt;
	}
	*t_rise  = (s1-s0)*v+time_zone;
	*t_set = (s2-s0)*v+time_zone;
	if ((*t_rise)<0) (*t_rise)+=24;
	if ((*t_set)<0) (*t_set)+=24;

	return 1;
}

void CSun::Fix(const CTimeMonitor & monTime, const GeoPoint & p)
{
	struct tm stm;
	memset(&stm, 0, sizeof(stm));
	if (monTime.Get(stm.tm_year, stm.tm_mon, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec))
	{
		stm.tm_year -= 1900;
		stm.tm_mon -= 1;
		double rise;
		double set;
		double hour = stm.tm_hour + double(stm.tm_min) / 60 + double(stm.tm_sec) / 3600;
		if (1 == sun_rise_set(&stm, 0 /* double(iBias) / 60 */, Degree(p.lat), Degree(p.lon), &rise, &set))
		{
			m_monSunrise.Set(rise);
			m_monSunset.Set(set);
			while (rise > set) 
				set += 24;
			while (hour < rise)
				hour += 24;
			bool fDay = (hour < set);
			if (fDay)
				m_monDaytime = L("Day");
			else
				m_monDaytime = L("Night");
			if (app.m_Options[mcoAutoLight])
			{
				bool fToSet = !fDay;
				if (!m_fSet || fToSet != m_fLastSet)
				{
					m_fSet = true;
					m_fLastSet = fToSet;
					if (app.m_Options[mcoLowLight] != fToSet)
					{
						app.ProcessCommand(mcoLowLight);
					}
				}
			}
			return;
		}
	}
	m_monSunrise.Reset();
	m_monSunset.Reset();
	m_monDaytime.Reset();
}
