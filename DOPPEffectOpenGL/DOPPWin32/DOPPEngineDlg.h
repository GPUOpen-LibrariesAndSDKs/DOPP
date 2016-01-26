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




// DOPPEngineDlg.h : header file
//

#pragma once


class DisplayManager;
class DOPPRotationDlg;


// CDOPPEngineDlg dialog
class CDOPPEngineDlg : public CDialogEx
{
// Construction
public:
	CDOPPEngineDlg(CWnd* pParent = NULL);	// standard constructor
    ~CDOPPEngineDlg();

    // Dialog Data
	enum { IDD = IDD_DOPPENGINE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL        OnInitDialog();
	afx_msg void        OnPaint();
    afx_msg void        OnSize(UINT nType, int cx, int cy);
	afx_msg HCURSOR     OnQueryDragIcon();
    afx_msg void        OnStart();
    afx_msg void        OnStop();
    afx_msg void        OnExit();

	DECLARE_MESSAGE_MAP()

private:

    enum EffectId { NO_EFFECT, COLOR_INVERT, EDGE_FILTER, DISTORT_EFFECT, ROTATE_DESKTOP };

    bool                createGLWindow();
    void                showWindow();
    void                deleteWindow();

    static DWORD        threadFunction(void* pData);
    DWORD               threadLoop();

    DWORD               m_dwThreadId;
    bool                m_bEngineRunning;
    HANDLE              m_hEngineThread;
    
    HWND                m_hWnd; 
    HDC                 m_hDC;
    HGLRC               m_hCtx;

    unsigned int        m_uiEffectSelection;
    unsigned int        m_uiDesktopSelection;
    unsigned int        m_uiRotationAngle;

    CComboBox*          m_pEffectComboBox;
    CComboBox*          m_pDesktopComboBox;
    CButton*            m_pShowWinCheckBox;

    DisplayManager*     m_pDisplayManager;

    DOPPRotationDlg*    m_pRotationDlg;

public:
    afx_msg void OnBnClickedCheckWindow();
};
