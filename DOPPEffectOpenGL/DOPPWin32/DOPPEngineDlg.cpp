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




#include "stdafx.h"
#include <string>

#include "DOPPEngine.h"
#include "DOPPEngineDlg.h"
#include "afxdialogex.h"

#include <GL/glew.h>
#include <GL/wglew.h>

#include "DOPPRotationDlg.h"
#include "DisplayManager.h"
#include "GLWindow.hpp"
#include "GLDOPPEngine.h"
#include "GLDOPPColorInvert.h"
#include "GLDOPPDistort.h"
#include "GLDOPPEdgeFilter.h"
#include "GLDOPPRotate.h"


static unsigned int g_uiWindowWidth  = 800;
static unsigned int g_uiWindowHeight = 600;

using namespace std;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



CDOPPEngineDlg::CDOPPEngineDlg(CWnd* pParent ) : CDialogEx(CDOPPEngineDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_dwThreadId     =  0;
    m_bEngineRunning = false;
    m_hEngineThread  = NULL;

    m_pEffectComboBox  = NULL;
    m_pShowWinCheckBox = NULL;

    m_uiEffectSelection  = 0;
    m_uiDesktopSelection = 0;

    m_pDisplayManager = NULL;

    m_pRotationDlg = NULL;
}


CDOPPEngineDlg::~CDOPPEngineDlg()
{
    if (m_pDisplayManager)
        delete m_pDisplayManager;
}

void CDOPPEngineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDOPPEngineDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_COMMAND(IDC_START,  &CDOPPEngineDlg::OnStart)
    ON_COMMAND(IDC_STOP,   &CDOPPEngineDlg::OnStop)
    ON_COMMAND(IDC_EXIT,   &CDOPPEngineDlg::OnExit)
END_MESSAGE_MAP()


// CDOPPEngineDlg message handlers

BOOL CDOPPEngineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    m_pEffectComboBox = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_EFFECT));

    m_pEffectComboBox->Clear();
    m_pEffectComboBox->InsertString(0, _T("No Effect"));
    m_pEffectComboBox->InsertString(1, _T("Invert Colors"));
    m_pEffectComboBox->InsertString(2, _T("Edge Filter"));
    m_pEffectComboBox->InsertString(3, _T("Distort"));
    m_pEffectComboBox->InsertString(4, _T("Rotate"));

    m_pEffectComboBox->SetCurSel(0);
    m_uiEffectSelection = 0;

    m_pDisplayManager = new DisplayManager;

    m_pShowWinCheckBox = static_cast<CButton*>(GetDlgItem(IDC_CHECK_WINDOW));

    m_pShowWinCheckBox->SetCheck(BST_UNCHECKED);

    m_pDesktopComboBox = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_DESKTOP));

    m_pDesktopComboBox->Clear();

    unsigned int uiNumDesktops = m_pDisplayManager->enumDisplays();

    for (unsigned int i = 0; i < uiNumDesktops; ++i)
    {
        char buf[8];

        // Add 1 to the id to have the matching Desktop Id with CCC
        sprintf_s(buf," %d", (i + 1));

        m_pDesktopComboBox->InsertString(i, CA2CT(buf));
    }
    
    m_pDesktopComboBox->SetCurSel(0);

    m_uiDesktopSelection = 0;

    m_pRotationDlg = new DOPPRotationDlg;

	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CDOPPEngineDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); 

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}



// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDOPPEngineDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CDOPPEngineDlg::OnStart()
{
    if (!m_bEngineRunning)
    {
        m_uiEffectSelection  = m_pEffectComboBox->GetCurSel();
        m_uiDesktopSelection = m_pDesktopComboBox->GetCurSel();

        if (m_uiEffectSelection == ROTATE_DESKTOP && m_pRotationDlg)
        {
            if (m_pRotationDlg->DoModal() == IDOK)
            {
                m_uiRotationAngle = m_pRotationDlg->getAngle();
            }
            else
            {
                return;
            }
        }

        m_hEngineThread = CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&CDOPPEngineDlg::threadFunction), this, 0, &m_dwThreadId);

        m_bEngineRunning = true;
    }
}


void CDOPPEngineDlg::OnStop()
{
    if (m_bEngineRunning)
    {
        m_bEngineRunning = false;

        WaitForSingleObject(m_hEngineThread, INFINITE);
    }
}



void CDOPPEngineDlg::OnExit()
{
    // stop thread if required
    OnStop();

    OnOK();
}


DWORD CDOPPEngineDlg::threadFunction(void* pData)
{
    if (pData)
    {
        CDOPPEngineDlg* pInstance = reinterpret_cast<CDOPPEngineDlg*>(pData);

        pInstance->threadLoop();
    }

    return 0;
}


DWORD CDOPPEngineDlg::threadLoop()
{
    bool   bCapture = false;

    bCapture = (m_pShowWinCheckBox->GetCheck() == BST_CHECKED);

    // Create window with ogl context to load extensions
    WNDCLASSEX		wndclass;
	const LPCWSTR   cClassName   = _T("OGL");
	const LPCWSTR	cWindowName  = _T("DOPP Capture");

	// Register WindowClass for GL window
	wndclass.cbSize         = sizeof(WNDCLASSEX);
	wndclass.style          = CS_OWNDC;
	wndclass.lpfnWndProc    = WndProc;
	wndclass.cbClsExtra     = 0;
	wndclass.cbWndExtra     = 0;
	wndclass.hInstance      = (HINSTANCE)GetModuleHandle(NULL);
	wndclass.hIcon		    = (HICON)LoadImage((HINSTANCE)AfxGetInstanceHandle(),  MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, LR_DEFAULTSIZE, LR_DEFAULTSIZE, NULL); 
	wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground  = NULL;
	wndclass.lpszMenuName   = NULL;
	wndclass.lpszClassName  = cClassName;
	wndclass.hIconSm		= (HICON)LoadImage((HINSTANCE)AfxGetInstanceHandle(),  MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, LR_DEFAULTSIZE, LR_DEFAULTSIZE, NULL); 

	if (!RegisterClassEx(&wndclass))
		return FALSE;
     
    GLWindow* pWin = new GLWindow("DOPP Capture", "OGL");

    pWin->create(g_uiWindowWidth, g_uiWindowHeight, false, false, false, true);

    // GLWindow::create context will destroy the initial window and create a new one. To avoid that
    // WndProc emits a PostQuitMessage a pointer to the initial window is passed. If this pointer is 
    // valid we know that it is the temporary window and should not emit PostQuitMessage.
    SetWindowLongPtr(pWin->getWnd(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWin));

    pWin->createContext(0);

    GLDOPPEngine* pEngine = NULL;

    switch (m_uiEffectSelection)
    {
        case COLOR_INVERT:
            pEngine = new GLDOPPColorInvert;
            break;

        case EDGE_FILTER:
            pEngine = new GLDOPPEdgeFilter;
            break;

        case DISTORT_EFFECT:
            pEngine = new GLDOPPDistort;
            break;

        case ROTATE_DESKTOP:
            pEngine = new GLDOPPRotate;
            break;

        default:
            pEngine = new GLDOPPEngine;
            break;
    }

    // m_uiDesktopSelection is the id of the selected element in the Combo Box
    // Need to add 1 to get the desktop id as shown in CCC
    if (!pEngine->initDOPP(m_uiDesktopSelection + 1))
    {
        // prevent thread loop to start due to error
        m_bEngineRunning = false;
    }



    if (!pEngine->initEffect())
    {
        // prevent thread loop to start due to error
        m_bEngineRunning = false;
    }


	if (m_bEngineRunning && m_uiEffectSelection == ROTATE_DESKTOP)
    {
        GLDOPPRotate* pRot = dynamic_cast<GLDOPPRotate*>(pEngine);
        pRot->setRotation((float)m_uiRotationAngle);
    }



    if (bCapture && m_bEngineRunning)
    {
        // Open window only if this option was checked in the GUI
        pWin->open();
    }

    // Enable SW mouse
    SystemParametersInfo(SPI_SETMOUSETRAILS, 2, 0, 0);

    MSG mMsg;

    while(m_bEngineRunning)
    {
        pEngine->processDesktop();
        
        if (bCapture)
        {
            // Blit FBO of DOPPEngine into the window
            glViewport(0, 0, g_uiWindowWidth, g_uiWindowHeight);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, pEngine->getPresentBuffer());

            glBlitFramebuffer(0, pEngine->getDesktopHeight(), pEngine->getDesktopWidth(), 0, 0, 0, g_uiWindowWidth, g_uiWindowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            pWin->swapBuffers();

            if (PeekMessage(&mMsg, NULL, 0, 0, PM_REMOVE))
		    {
			    if (mMsg.message == WM_QUIT)
			    {
				    m_bEngineRunning = false;
			    }
                else
			    {
				    TranslateMessage(&mMsg);
				    DispatchMessage(&mMsg);
			    }
		    }
        }
    } 

    // Disable SW mouse
    SystemParametersInfo(SPI_SETMOUSETRAILS, 1, 0, 0);

    delete pEngine;

    delete pWin;

    ::UnregisterClass(cClassName, NULL);

    return 0;
}


// Window proc for GL window
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
    case WM_CHAR:
        {
            char c = (char)wParam;

			if (c ==  VK_ESCAPE)
		        PostQuitMessage(0);
        }
        return 0;


	case WM_SIZE:
		 g_uiWindowWidth  = LOWORD(lParam);
         g_uiWindowHeight = HIWORD(lParam);

         return 0;

    case WM_CREATE:
        return 0;

	case WM_DESTROY:
        GLWindow* pWin = reinterpret_cast<GLWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

        if (!pWin || !pWin->IsTempWindow())
		    PostQuitMessage(0);

		return 0;

	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

