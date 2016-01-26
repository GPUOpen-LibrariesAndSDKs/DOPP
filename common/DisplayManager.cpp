//--------------------------------------------------------------------------------------
// 
// Copyright 2014 ADVANCED MICRO DEVICES, INC.  All Rights Reserved.
//
// AMD is granting you permission to use this software and documentation (if
// any) (collectively, the "Materials") pursuant to the terms and conditions
// of the Software License Agreement included with the Materials.  If you do
// not have a copy of the Software License Agreement, contact your AMD
// representative for a copy.
// You agree that you will not reverse engineer or decompile the Materials,
// in whole or in part, except as allowed by applicable law.
//
// WARRANTY DISCLAIMER: THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND.  AMD DISCLAIMS ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY,
// INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE, NON-INFRINGEMENT, THAT THE SOFTWARE
// WILL RUN UNINTERRUPTED OR ERROR-FREE OR WARRANTIES ARISING FROM CUSTOM OF
// TRADE OR COURSE OF USAGE.  THE ENTIRE RISK ASSOCIATED WITH THE USE OF THE
// SOFTWARE IS ASSUMED BY YOU.
// Some jurisdictions do not allow the exclusion of implied warranties, so
// the above exclusion may not apply to You. 
// 
// LIMITATION OF LIABILITY AND INDEMNIFICATION:  AMD AND ITS LICENSORS WILL
// NOT, UNDER ANY CIRCUMSTANCES BE LIABLE TO YOU FOR ANY PUNITIVE, DIRECT,
// INCIDENTAL, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES ARISING FROM USE OF
// THE SOFTWARE OR THIS AGREEMENT EVEN IF AMD AND ITS LICENSORS HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  
// In no event shall AMD's total liability to You for all damages, losses,
// and causes of action (whether in contract, tort (including negligence) or
// otherwise) exceed the amount of $100 USD.  You agree to defend, indemnify
// and hold harmless AMD and its licensors, and any of their directors,
// officers, employees, affiliates or agents from and against any and all
// loss, damage, liability and other expenses (including reasonable attorneys'
// fees), resulting from Your use of the Software or violation of the terms and
// conditions of this Agreement.  
//
// U.S. GOVERNMENT RESTRICTED RIGHTS: The Materials are provided with "RESTRICTED
// RIGHTS." Use, duplication, or disclosure by the Government is subject to the
// restrictions as set forth in FAR 52.227-14 and DFAR252.227-7013, et seq., or
// its successor.  Use of the Materials by the Government constitutes
// acknowledgement of AMD's proprietary rights in them.
// 
// EXPORT RESTRICTIONS: The Materials may be subject to export restrictions as
// stated in the Software License Agreement.
//
//--------------------------------------------------------------------------------------



#include <windows.h>
#include <string>
#include <tchar.h>

#include "ADL/adl_sdk.h"

#include "DisplayManager.h"

typedef int ( *ADL_MAIN_CONTROL_CREATE )            (ADL_MAIN_MALLOC_CALLBACK, int );
typedef int ( *ADL_MAIN_CONTROL_DESTROY )           ();
typedef int ( *ADL_ADAPTER_NUMBEROFADAPTERS_GET )   ( int* );
typedef int ( *ADL_ADAPTER_ACTIVE_GET )             ( int, int* );
typedef int ( *ADL_ADAPTER_ADAPTERINFO_GET )        ( LPAdapterInfo, int );
typedef int ( *ADL_ADAPTER_ACTIVE_GET )             ( int, int* );
typedef int ( *ADL_DISPLAY_DISPLAYINFO_GET )        ( int, int *, ADLDisplayInfo **, int );



typedef struct
{
    void*                               hDLL;
    ADL_MAIN_CONTROL_CREATE             ADL_Main_Control_Create;
    ADL_MAIN_CONTROL_DESTROY            ADL_Main_Control_Destroy;
    ADL_ADAPTER_NUMBEROFADAPTERS_GET    ADL_Adapter_NumberOfAdapters_Get;
    ADL_ADAPTER_ACTIVE_GET              ADL_Adapter_Active_Get;
    ADL_ADAPTER_ADAPTERINFO_GET         ADL_Adapter_AdapterInfo_Get;
    ADL_DISPLAY_DISPLAYINFO_GET         ADL_Display_DisplayInfo_Get;
} ADLFunctions;


static ADLFunctions g_AdlCalls = { 0, 0, 0, 0, 0, 0, 0 };


typedef struct DisplayData
{
    unsigned int        uiGPUId;
    unsigned int        uiDisplayId;
    unsigned int        uiDisplayLogicalId;
    int                 nOriginX;
    int                 nOriginY;
    unsigned int        uiWidth;
    unsigned int        uiHeight;
    std::string         strDisplayname;
} DISPLAY_DATA;



// Memory Allocation function needed for ADL
void* __stdcall ADL_Alloc ( int iSize )
{
    void* lpBuffer = malloc ( iSize );
    return lpBuffer;
}

// Memory Free function needed for ADL
void __stdcall ADL_Free ( void* lpBuffer )
{
    if ( NULL != lpBuffer )
    {
        free ( lpBuffer );
        lpBuffer = NULL;
    }
}


using namespace std;


DisplayManager::DisplayManager()
{
    m_uiNumGPU = 0;

    m_Displays.clear();
}


DisplayManager::~DisplayManager()
{
    deleteDisplays();
}


unsigned int DisplayManager::enumDisplays()
{
    int				nNumDisplays = 0;
	int				nNumAdapters = 0;
    int             nCurrentBusNumber = 0;
	LPAdapterInfo   pAdapterInfo = NULL;
    unsigned int    uiCurrentGPUId     = 0;
    unsigned int    uiCurrentDisplayId = 0;

    // load all required ADL functions
    if (!setupADL())
        return 0;

    if (m_Displays.size() > 0)
        deleteDisplays();

    // Determine how many adapters and displays are in the system
	g_AdlCalls.ADL_Adapter_NumberOfAdapters_Get(&nNumAdapters);

	if (nNumAdapters > 0)
	{
		pAdapterInfo = (LPAdapterInfo)ADL_Alloc( sizeof (AdapterInfo) * nNumAdapters );
        memset ( pAdapterInfo,'\0', sizeof (AdapterInfo) * nNumAdapters );
	}

	g_AdlCalls.ADL_Adapter_AdapterInfo_Get (pAdapterInfo, sizeof (AdapterInfo) * nNumAdapters);

    // Loop through all adapters 
	for (int i = 0; i < nNumAdapters; ++i)
	{
		int				nAdapterIdx; 
		int				nAdapterStatus;
		
		nAdapterIdx = pAdapterInfo[i].iAdapterIndex;

		g_AdlCalls.ADL_Adapter_Active_Get(nAdapterIdx, &nAdapterStatus);

		if (nAdapterStatus)
		{
			LPADLDisplayInfo	pDisplayInfo = NULL;

			g_AdlCalls.ADL_Display_DisplayInfo_Get(nAdapterIdx, &nNumDisplays, &pDisplayInfo, 0);

			for (int j = 0; j < nNumDisplays; ++j)
			{
				// check if display is connected 
				if (pDisplayInfo[j].iDisplayInfoValue & ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED)
                {
					// check if display is mapped on adapter
					if (pDisplayInfo[j].iDisplayInfoValue & ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED && pDisplayInfo[j].displayID.iDisplayLogicalAdapterIndex == nAdapterIdx)
					{
                        if (nCurrentBusNumber == 0)
                        {
                            // Found first GPU in the system
                            ++m_uiNumGPU;
                            nCurrentBusNumber = pAdapterInfo[nAdapterIdx].iBusNumber;
                        }
                        else if (nCurrentBusNumber != pAdapterInfo[nAdapterIdx].iBusNumber)
                        {
                            // found new GPU
                            ++m_uiNumGPU;
                            ++uiCurrentGPUId;
                            nCurrentBusNumber = pAdapterInfo[nAdapterIdx].iBusNumber;
                        }     

                        // Found mapped display, store relevant information
                        DisplayData* pDsp = new DisplayData;
                        
                        pDsp->uiGPUId               = uiCurrentGPUId;
                        pDsp->uiDisplayId           = uiCurrentDisplayId;         
                        pDsp->uiDisplayLogicalId    = pDisplayInfo[j].displayID.iDisplayLogicalIndex;
                        pDsp->strDisplayname        = string(pAdapterInfo[i].strDisplayName);
                        pDsp->nOriginX              = 0;
                        pDsp->nOriginY              = 0;
                        pDsp->uiWidth               = 0;
                        pDsp->uiHeight              = 0;

                        DEVMODEA DevMode;
                        memset((void*)&DevMode, 0, sizeof(DEVMODEA));

                        EnumDisplaySettingsA(pDsp->strDisplayname.c_str(), ENUM_CURRENT_SETTINGS, &DevMode);

                        pDsp->nOriginX             = DevMode.dmPosition.x;
                        pDsp->nOriginY             = DevMode.dmPosition.y;
                        pDsp->uiWidth              = DevMode.dmPelsWidth;
                        pDsp->uiHeight             = DevMode.dmPelsHeight;

                        m_Displays.push_back(pDsp);

                        ++uiCurrentDisplayId; 
                    }
                }
            }

            ADL_Free(pDisplayInfo);
        }
    }

    if (pAdapterInfo)
        ADL_Free(pAdapterInfo);

    return m_Displays.size();
}


void DisplayManager::deleteDisplays()
{
    vector<DisplayData*>::iterator itr;

    for (itr = m_Displays.begin(); itr != m_Displays.end(); ++itr)
    {
        if (*itr)
            delete (*itr);
    }

    m_Displays.clear();

    m_uiNumGPU = 0;
}


unsigned int DisplayManager::getNumGPUs()
{
    if (m_Displays.size() > 0)
        return m_uiNumGPU;

    return 0;
}


unsigned int DisplayManager::getNumDisplays()
{
    return m_Displays.size();
}


unsigned int DisplayManager::getNumDisplaysOnGPU(unsigned int uiGPU)
{
    unsigned int uiNumDsp = 0;
    vector<DisplayData*>::iterator itr;

    // loop through all display and check if they are on the requested GPU
    for (itr = m_Displays.begin(); itr != m_Displays.end(); ++itr)
    {
        if ((*itr)->uiGPUId == uiGPU)
            ++uiNumDsp;
    }

    return uiNumDsp;
}


unsigned int DisplayManager::getDisplayOnGPU(unsigned int uiGPU, unsigned int n)
{
    unsigned int uiCurrentDisplayOnGpu = 0;

    vector<DisplayData*>::iterator itr;

    // loop through all display and return the n-th display that is on GPU uiGPU
    for (itr = m_Displays.begin(); itr != m_Displays.end(); ++itr)
    {
        if ((*itr)->uiGPUId == uiGPU)
        {
            if (uiCurrentDisplayOnGpu == n)
            {
                return (*itr)->uiDisplayId;
            }

            ++uiCurrentDisplayOnGpu;
        }
    }

    return 0;
}


const char* DisplayManager::getDisplayName(unsigned int uiDisplay)
{
    if (uiDisplay < m_Displays.size())
    {
        return m_Displays[uiDisplay]->strDisplayname.c_str();
    }

    return NULL;
}


unsigned int DisplayManager::getGpuId(unsigned int uiDisplay)
{
    if (uiDisplay < m_Displays.size())
    {
        return m_Displays[uiDisplay]->uiGPUId;
    }

    return 0;
}


bool DisplayManager::getOrigin(unsigned int uiDisplay, int &nOriginX, int &nOriginY)
{
    nOriginX = 0;
    nOriginY = 0;

    if (uiDisplay < m_Displays.size())
    {
        nOriginX = m_Displays[uiDisplay]->nOriginX;
        nOriginY = m_Displays[uiDisplay]->nOriginY;

        return true;
    }

    return false;
}


bool DisplayManager::getSize(unsigned int uiDisplay, unsigned int &uiWidth, unsigned int &uiHeight)
{
    uiWidth  = 0;
    uiHeight = 0;

    if (uiDisplay < m_Displays.size())
    {
        uiWidth  = m_Displays[uiDisplay]->uiWidth;
        uiHeight = m_Displays[uiDisplay]->uiHeight;

        return true;
    }

    return false;
}


bool DisplayManager::setupADL()
{
    // check if ADL was already loaded
    if (g_AdlCalls.hDLL)
    {
        return true;
    }

    g_AdlCalls.hDLL = (void*)LoadLibraryA("atiadlxx.dll");

	if (g_AdlCalls.hDLL == NULL)
       g_AdlCalls.hDLL = (void*)LoadLibraryA("atiadlxy.dll");


	if (!g_AdlCalls.hDLL)
		return false;

	// Get proc address of needed ADL functions
	g_AdlCalls.ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE)GetProcAddress((HMODULE)g_AdlCalls.hDLL,"ADL_Main_Control_Create");
	if (!g_AdlCalls.ADL_Main_Control_Create)
		return false;

	g_AdlCalls.ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress((HMODULE)g_AdlCalls.hDLL, "ADL_Main_Control_Destroy");
	if (!g_AdlCalls.ADL_Main_Control_Destroy)
		return false;

	g_AdlCalls.ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress((HMODULE)g_AdlCalls.hDLL,"ADL_Adapter_NumberOfAdapters_Get");
	if (!g_AdlCalls.ADL_Adapter_NumberOfAdapters_Get)
		return false;

	g_AdlCalls.ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress((HMODULE)g_AdlCalls.hDLL,"ADL_Adapter_AdapterInfo_Get");
	if (!g_AdlCalls.ADL_Adapter_AdapterInfo_Get)
		return false;

	g_AdlCalls.ADL_Display_DisplayInfo_Get = (ADL_DISPLAY_DISPLAYINFO_GET)GetProcAddress((HMODULE)g_AdlCalls.hDLL,"ADL_Display_DisplayInfo_Get");
	if (!g_AdlCalls.ADL_Display_DisplayInfo_Get)
		return false;

	g_AdlCalls.ADL_Adapter_Active_Get = (ADL_ADAPTER_ACTIVE_GET)GetProcAddress((HMODULE)g_AdlCalls.hDLL,"ADL_Adapter_Active_Get");
	if (!g_AdlCalls.ADL_Adapter_Active_Get)
		return false;
 
	// Init ADL
	if (g_AdlCalls.ADL_Main_Control_Create(ADL_Alloc, 0) != ADL_OK)
		return false;

	return true;
}




