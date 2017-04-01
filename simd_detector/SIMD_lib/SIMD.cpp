/*
SIMD class to detect presence of SIMD version 1.4.0

Copyright (c) 2010 Wong Shao Voon
1.1.0 - Contains the 3DNow, 3DNow+ and MMX+ detection bug fix by Leonardo Tazzini
1.2.0 - Added AMD's SSE4a and SSE5
1.3.0 - Use 2 unsigned shorts to store SIMD info, instead of 1 boolean for each SIMD type.
1.4.0 - Added the detection of Intel AES instructions

The Code Project Open License (CPOL)
http://www.codeproject.com/info/cpol10.aspx
*/

//#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "SIMD.h"

#include <intrin.h>

#define EDX_MMX_bit      0x800000   // 23 bit
#define EDX_SSE_bit      0x2000000  // 25 bit
#define EDX_SSE2_bit     0x4000000  // 26 bit
#define EDX_3DnowExt_bit 0x40000000 // 30 bit
#define EDX_3Dnow_bit 	 0x80000000 // 31 bit
#define EDX_MMXplus_bit  0x400000 // 22 bit

#define ECX_SSE3_bit     0x1        // 0 bit
#define ECX_SSSE3_bit    0x200      // 9 bit
#define ECX_SSE41_bit    0x80000    // 19 bit
#define ECX_SSE42_bit    0x100000   // 20 bit

#define ECX_SSE4A_bit    0x40    // 6 bit
#define ECX_SSE5_bit     0x800    // 11 bit

#define ECX_AES_bit     0x2000000    // 25 bit
#define ECX_AVX_bit     0x10000000    // 28 bit

// My own SIMD bitmask
#define INTEL_MMX 	0x0001
#define INTEL_SSE 	0x0002
#define INTEL_SSE2 	0x0004
#define INTEL_SSE3 	0x0008
#define INTEL_SSSE3 0x0010
#define INTEL_SSE41 0x0020
#define INTEL_SSE42 0x0040
#define INTEL_AES 	0x0080
#define INTEL_AVX 	0x0100

#define AMD_MMXPLUS 	0x0001
#define AMD_3DNOW 		0x0002
#define AMD_3DNOWEXT 	0x0004
#define AMD_SSE4A 		0x0008
#define AMD_SSE5 		0x0010


SIMD::SIMD()
{
	m_nIntelSIMDs = 0;
	m_nAMD_SIMDs = 0;

	InitParams();
}

void SIMD::InitParams()
{
	int CPUInfo[4];
	int CPUInfoExt[4];
	DWORD dwECX=0;
	DWORD dwEDX=0;

	__cpuid(CPUInfo, 0);

	if( CPUInfo[0] >= 1 )
	{
		__cpuid(CPUInfo, 1);
		dwECX = CPUInfo[2];
		dwEDX = CPUInfo[3];
	}

	__cpuid(CPUInfo, 0x80000000);

	if( CPUInfo[0] >= 0x80000001 )
		__cpuid(CPUInfoExt, 0x80000001);

	if( EDX_MMX_bit & dwEDX )
		m_nIntelSIMDs |= INTEL_MMX;

	if( EDX_SSE_bit & dwEDX )
		m_nIntelSIMDs |= INTEL_SSE;

	if( EDX_SSE2_bit & dwEDX )
		m_nIntelSIMDs |= INTEL_SSE2;

	if( ECX_SSE3_bit & dwECX )
		m_nIntelSIMDs |= INTEL_SSE3;

	if( ECX_SSSE3_bit & dwECX )
		m_nIntelSIMDs |= INTEL_SSSE3;

	if( ECX_SSE41_bit & dwECX )
		m_nIntelSIMDs |= INTEL_SSE41;

	if( ECX_SSE42_bit & dwECX )
		m_nIntelSIMDs |= INTEL_SSE42;

	if( ECX_AES_bit & dwECX )
		m_nIntelSIMDs |= INTEL_AES;

	if( ECX_AVX_bit & dwECX )
		m_nIntelSIMDs |= INTEL_AVX;

	if( EDX_3DnowExt_bit & CPUInfoExt[3] )
		m_nAMD_SIMDs |= AMD_3DNOWEXT;

	if( EDX_3Dnow_bit & CPUInfoExt[3] )
		m_nAMD_SIMDs |= AMD_3DNOW;

	if( EDX_MMXplus_bit & CPUInfoExt[3] )
		m_nAMD_SIMDs |= AMD_MMXPLUS;

	if( ECX_SSE4A_bit & CPUInfoExt[2] )
		m_nAMD_SIMDs |= AMD_SSE4A;

	if( ECX_SSE5_bit & CPUInfoExt[2] )
		m_nAMD_SIMDs |= AMD_SSE5;
}

// intel SIMD
bool SIMD::HasMMX()		{return m_nIntelSIMDs & INTEL_MMX;}
bool SIMD::HasSSE()		{return m_nIntelSIMDs & INTEL_SSE;}
bool SIMD::HasSSE2()	{return m_nIntelSIMDs & INTEL_SSE2;}
bool SIMD::HasSSE3()	{return m_nIntelSIMDs & INTEL_SSE3;}
bool SIMD::HasSSSE3() 	{return m_nIntelSIMDs & INTEL_SSSE3;}
bool SIMD::HasSSE41() 	{return m_nIntelSIMDs & INTEL_SSE41;}
bool SIMD::HasSSE42() 	{return m_nIntelSIMDs & INTEL_SSE42;}
bool SIMD::HasAES() 	{return m_nIntelSIMDs & INTEL_AES;}

// code from http://insufficientlycomplicated.wordpress.com/2011/11/07/detecting-intel-advanced-vector-extensions-avx-in-visual-studio/
bool SIMD::HasAVX() 	
{
	bool avxSupported = false;

	// If Visual Studio 2010 SP1 or later
#if (_MSC_FULL_VER >= 160040219)
	// Checking for AVX requires 3 things:
	// 1) CPUID indicates that the OS uses XSAVE and XRSTORE
	//     instructions (allowing saving YMM registers on context
	//     switch)
	// 2) CPUID indicates support for AVX
	// 3) XGETBV indicates the AVX registers will be saved and
	//     restored on context switch
	//
	// Note that XGETBV is only available on 686 or later CPUs, so
	// the instruction needs to be conditionally run.
	int cpuInfo[4];
	__cpuid(cpuInfo, 1);

	bool osUsesXSAVE_XRSTORE = cpuInfo[2] & (1 << 27) || false;
	bool cpuAVXSuport = cpuInfo[2] & (1 << 28) || false;

	if (osUsesXSAVE_XRSTORE && cpuAVXSuport)
	{
		// Check if the OS will save the YMM registers
		unsigned long long xcrFeatureMask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
		avxSupported = (xcrFeatureMask & 0x6) || false;
	}
#endif

	return avxSupported;
}

// AMD SIMD
bool SIMD::HasMMXplus()	{return m_nAMD_SIMDs & AMD_MMXPLUS;}
bool SIMD::Has3Dnow()	{return m_nAMD_SIMDs & AMD_3DNOW;}
bool SIMD::Has3DnowExt(){return m_nAMD_SIMDs & AMD_3DNOWEXT;}
bool SIMD::HasSSE4a() 	{return m_nAMD_SIMDs & AMD_SSE4A;}
bool SIMD::HasSSE5() 	{return m_nAMD_SIMDs & AMD_SSE5;}

std::wstring SIMD::GetCPUModel()
{
	int CPUInfo[4] = {-1};
	__cpuid(CPUInfo, 0x80000000);
	unsigned int nExIds = CPUInfo[0];
	// Get the information associated with each extended ID.
	char CPUBrand[0x40] = { 0 };
	// Interpret CPU brand string and cache information.
	__cpuid(CPUInfo, 0x80000002);
	memcpy( CPUBrand, CPUInfo, sizeof(CPUInfo));
	__cpuid(CPUInfo, 0x80000003);
	memcpy( CPUBrand + 16, CPUInfo, sizeof(CPUInfo));
	__cpuid(CPUInfo, 0x80000004);
	memcpy(CPUBrand + 32, CPUInfo, sizeof(CPUInfo));

	std::wstring wCPU = ConvToWString(CPUBrand);

	return Trim(wCPU);
}

std::wstring SIMD::ConvToWString(const std::string& text)
{
	std::wstring str = L"";
	for(size_t i=0; i<text.length(); ++i)
	{
		str += (wchar_t)(text[i]);
	}

	return str;
}

std::wstring SIMD::TrimRight(const std::wstring& str, const std::wstring& trimChars)
{
	std::wstring result = L"";
	// trim trailing spaces
	size_t endpos = str.find_last_not_of(trimChars);
	if( std::wstring::npos != endpos )
	{
		result = str.substr( 0, endpos+1 );
	}
	else
		result = str;

	return result;
}

std::wstring SIMD::TrimLeft(const std::wstring& str, const std::wstring& trimChars)
{
	std::wstring result = L"";

	// trim leading spaces
	size_t startpos = str.find_first_not_of(L" \t\r\n");
	if( std::wstring::npos != startpos )
	{
		result = str.substr( startpos );
	}
	else
		result = str;

	return result;
}

std::wstring SIMD::Trim(const std::wstring& str, const std::wstring& trimChars)
{
	return TrimLeft(TrimRight(str, trimChars), trimChars);
}

