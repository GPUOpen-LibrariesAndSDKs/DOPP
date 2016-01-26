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



#include "os_include.hpp"

#include <GL/glew.h>
#include <iostream>

#if defined(WIN32)
#include <GL/wglew.h>
#define GETPROC GetProcAddress
#endif



#include "GLWindow.hpp"


using namespace std;




#ifdef WIN32
void MyDebugFunc(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
  MessageBoxA(NULL, message, "GL Debug Message", MB_ICONWARNING | MB_OK);
}
#endif


GLWindow::GLWindow(const char *strWinName, const char* strClsaaName)
{
  m_strClassName = strClsaaName;
  m_strWinName   = strWinName;

  m_hDC   = NULL;
  m_hGLRC = NULL;
  m_hWnd  = NULL;

  m_uiWidth = 800;
  m_uiHeight = 600;

  m_uiPosX = 0;
  m_uiPosY = 0;

  tempWindow = 0;
  m_bFullScreen = false;
}


GLWindow::~GLWindow(void)
{

  destroy();
}




void GLWindow::resize(unsigned int uiWidth, unsigned int uiHeight)
{
  m_uiWidth  = uiWidth;
  m_uiHeight = uiHeight;
}



//----------------------------------------------------------------------------------------------
//  Windows implementation functions
//----------------------------------------------------------------------------------------------

#ifdef WIN32

bool GLWindow::create(unsigned int uiWidth, unsigned int uiHeight, bool FullScreen, bool Stereo, bool Need10Bits, bool bDecoration)
{
  if (!WindowCreation(uiWidth, uiHeight, FullScreen, bDecoration))
    return false;
  tempWindow = 1;
  return SetSimplePF(Need10Bits, Stereo);


}

bool GLWindow::WindowCreation(unsigned int uiWidth, unsigned int uiHeight, bool FullScreen, bool bDecoration)
{
  RECT        WinSize;
  DWORD		dwExStyle;
  DWORD		dwStyle;

  m_bFullScreen = FullScreen;



  if (m_bFullScreen)
  {
    dwExStyle = WS_EX_APPWINDOW;								
    dwStyle   = WS_POPUP;										
  }
  else
  {
    m_uiWidth  = uiWidth;
    m_uiHeight = uiHeight;

    dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;	

    if (bDecoration)
    {
      dwStyle   = WS_OVERLAPPEDWINDOW;

      // Adjust window size so that the ClientArea has the initial size
      // of uiWidth and uiHeight
      WinSize.bottom = uiHeight; 
      WinSize.left   = 0;
      WinSize.right  = uiWidth;
      WinSize.top    = 0;

      AdjustWindowRect(&WinSize, WS_OVERLAPPEDWINDOW, false);



      if (WinSize.left < 0)
      {
        WinSize.right -= WinSize.left;
        WinSize.left = 0;
      }
      if (WinSize.top < 0)
      {
        WinSize.bottom -= WinSize.top;
        WinSize.top = 0;
      }
      m_uiWidth  = WinSize.right  - WinSize.left;
      m_uiHeight = WinSize.bottom - WinSize.top;

    }
    else
      dwStyle   = WS_POPUP;
  }

  m_hWnd = CreateWindowExA( dwExStyle,
    m_strClassName.c_str(), 
    m_strWinName.c_str(),
    dwStyle,
    m_uiPosX,
    m_uiPosY,
    m_uiWidth,
    m_uiHeight,
    NULL,
    NULL,
    (HINSTANCE)GetModuleHandle(NULL),
    NULL);

  if (!m_hWnd)
    return FALSE;

  return true;

}


bool GLWindow::SetSimplePF( bool Need10Bits, bool Stereo)
{
  int			nPixelFormat;

  pfd.nSize           = sizeof(PIXELFORMATDESCRIPTOR); 
  pfd.nVersion        = 1; 
  pfd.dwFlags         = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL  | PFD_DOUBLEBUFFER  ;
  pfd.iPixelType      = PFD_TYPE_RGBA; 
  pfd.cColorBits      = 32; 
  pfd.cRedBits        = Need10Bits?10:8; 
  pfd.cRedShift       = 0; 
  pfd.cGreenBits      = Need10Bits?10:8; 
  pfd.cGreenShift     = 0; 
  pfd.cBlueBits       = Need10Bits?10:8; 
  pfd.cBlueShift      = 0; 
  pfd.cAlphaBits      = Need10Bits?2:8;
  pfd.cAlphaShift     = 0; 
  pfd.cAccumBits      = 0; 
  pfd.cAccumRedBits   = 0; 
  pfd.cAccumGreenBits = 0; 
  pfd.cAccumBlueBits  = 0; 
  pfd.cAccumAlphaBits = 0; 
  pfd.cDepthBits      = 24; 
  pfd.cStencilBits    = 8; 
  pfd.cAuxBuffers     = 0; 
  pfd.iLayerType      = PFD_MAIN_PLANE;
  pfd.bReserved       = 0; 
  pfd.dwLayerMask     = 0;
  pfd.dwVisibleMask   = 0; 
  pfd.dwDamageMask    = 0;

  if (Stereo)
    pfd.dwFlags |= PFD_STEREO;

  m_hDC = GetDC(m_hWnd);

  if (!m_hDC)
    return false;

  nPixelFormat = ChoosePixelFormat(m_hDC, &pfd);

  if (!nPixelFormat)
    return false;

  SetPixelFormat(m_hDC, nPixelFormat, &pfd);

  DescribePixelFormat(m_hDC, nPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
  return true;
}


void GLWindow::destroy()
{
  if (m_hGLRC)
  {
    wglMakeCurrent(m_hDC, NULL);
    wglDeleteContext(m_hGLRC);
  }

  if (m_hWnd)
  {
    ReleaseDC(m_hWnd, m_hDC);
    DestroyWindow(m_hWnd);
    m_hDC= NULL;
    m_hWnd =  NULL;
  }
}

void GLWindow::open()
{
  ShowWindow(m_hWnd, SW_SHOW);
  SetForegroundWindow(m_hWnd);
  SetFocus(m_hWnd);

  UpdateWindow(m_hWnd);
}





void GLWindow::makeCurrent()
{
  wglMakeCurrent(m_hDC, m_hGLRC);
}


void GLWindow::swapBuffers()
{
  SwapBuffers(m_hDC);
}

int GLWindow::GetAdvancedPF( int NBSamples)
{
  m_hDC = GetDC(m_hWnd);

  if (!m_hDC)
  {
    std::cout<<"can't get a valid DC to create a context"<<std::endl;
    return 0;
  }

  int iAttributes[] = 
  { 
    WGL_DRAW_TO_WINDOW_ARB,  GL_TRUE,
    WGL_SUPPORT_OPENGL_ARB,  GL_TRUE,
    WGL_ACCELERATION_ARB,    WGL_FULL_ACCELERATION_ARB,
    WGL_COLOR_BITS_ARB,      pfd.cColorBits,
    WGL_RED_BITS_ARB,        pfd.cRedBits, 
    WGL_GREEN_BITS_ARB,      pfd.cGreenBits, 
    WGL_BLUE_BITS_ARB,       pfd.cBlueBits, 
    WGL_ALPHA_BITS_ARB,      pfd.cAlphaBits,
    WGL_DEPTH_BITS_ARB,      pfd.cDepthBits,
    WGL_STENCIL_BITS_ARB,    pfd.cStencilBits,
    WGL_DOUBLE_BUFFER_ARB,   GL_TRUE,
    WGL_SAMPLE_BUFFERS_ARB,  GL_FALSE,
    WGL_STEREO_ARB,          GL_FALSE,
    WGL_SAMPLES_ARB,         NBSamples,
    0, 0
  };


  if (NBSamples>0)
    iAttributes[23] = GL_TRUE;
  if (pfd.dwFlags & PFD_STEREO)
    iAttributes[25] = GL_TRUE;

  const float  fAttributes[] = {0.0f, 0.0f};

  UINT numFormats = 0;
  int pixelFormat = 0;
  wglChoosePixelFormatARB(m_hDC, iAttributes, fAttributes, 1,&pixelFormat, &numFormats);
  return pixelFormat;
}

bool GLWindow::createContext(int NBSamples)
{
  if (!m_hDC)
    return false;

  m_hGLRC = wglCreateContext(m_hDC);

  if (!m_hGLRC)
    return false;

  wglMakeCurrent( m_hDC, m_hGLRC );

  if (glewInit() != GLEW_OK)
  {
    return false;
  }

  if (WGLEW_ARB_create_context)
  {
    destroy();
    WindowCreation(m_uiWidth, m_uiHeight, m_bFullScreen);
    int newPF = GetAdvancedPF(NBSamples);

    SetPixelFormat(m_hDC, newPF, &pfd);
    DescribePixelFormat(m_hDC, newPF, sizeof(PIXELFORMATDESCRIPTOR), &pfd);




    int attribs[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB, SAMPLE_MAJOR_VERSION,
      WGL_CONTEXT_MINOR_VERSION_ARB, SAMPLE_MINOR_VERSION,
      WGL_CONTEXT_PROFILE_MASK_ARB , WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifdef _DEBUG
      WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
      0
    }; 

    m_hGLRC = wglCreateContextAttribsARB(m_hDC, 0, attribs);

    if (m_hGLRC)
    {
      wglMakeCurrent(m_hDC, m_hGLRC);
#ifdef _DEBUG
      glDebugMessageControlARB = (PFNGLDEBUGMESSAGECONTROLARBPROC)wglGetProcAddress("glDebugMessageControlARB");
      glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)wglGetProcAddress("glDebugMessageCallbackARB");
      glDebugMessageInsertARB = (PFNGLDEBUGMESSAGEINSERTARBPROC)wglGetProcAddress("glDebugMessageInsertARB");
#endif
      tempWindow = 0;
      return true;            
    }
  }
  tempWindow = 0;
  return false;
}

bool GLWindow::IsTempWindow()
{
  return tempWindow==0?false:true;
}

void GLWindow::deleteContext()
{
  wglMakeCurrent(m_hDC, NULL);
  wglDeleteContext(m_hGLRC);
}

#endif

