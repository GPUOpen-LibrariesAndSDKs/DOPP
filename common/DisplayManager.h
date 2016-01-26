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



#pragma once

#include <vector>

struct DisplayData;

class DisplayManager
{
public:

    DisplayManager();
    ~DisplayManager();

    // Enumerates the active desktops.This functions needs to be called before any other method of this class
    // The displays are stored in the order in which they are enumerated. An index will be assigned to each display.
    // The indexing starts at 0.
    // return: The number of mapped displays.
    unsigned int enumDisplays();
    
    // returns the number of GPUs in the system
    unsigned int    getNumGPUs();

    // Returns the number of mapped displays.
    unsigned int    getNumDisplays();

    // returns the number of displays mapped on GPU uiGPU
    unsigned int    getNumDisplaysOnGPU(unsigned int uiGPU);
    
    // returns the DisplayID of n-th Display on GPU uiGPU
    // n=0 will return the first display on GPU uiGPU if available
    // n=1 will return the second display on GPU uiGPU if available ...
    unsigned int    getDisplayOnGPU(unsigned int uiGPU, unsigned int n=0);

    // Returns the display name of display uiDisplay. 
    const char*     getDisplayName(unsigned int uiDisplay);

    // Returns the GPU ID of display uiDisplay. 
    unsigned int    getGpuId(unsigned int uiDisplay);    

    // Returns the origin of display uiDisplay. 
    bool            getOrigin(unsigned int uiDisplay, int &uiOriginX, int &uiOriginY);

    // Returns the size of display uiDisplay. 
    bool            getSize(unsigned int uiDisplay, unsigned int &uiWidth, unsigned int &uiHeight);

private:

    bool            setupADL();
    void            deleteDisplays();

    unsigned int                m_uiNumGPU;
    
    std::vector<DisplayData* >  m_Displays;
};