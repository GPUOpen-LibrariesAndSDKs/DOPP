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

#include <GL/glew.h>
#include <GL/wglew.h>

#include "GLShader.h"
#include "GLDOPPEngine.h"

#define DOPP2

#define GL_WAIT_FOR_PREVIOUS_VSYNC                        0x931C

typedef GLuint (APIENTRY* PFNWGLGETDESKTOPTEXTUREAMD)(void);
typedef void   (APIENTRY* PFNWGLENABLEPOSTPROCESSAMD)(bool enable);
typedef GLuint (APIENTRY* WGLGENPRESENTTEXTUREAMD)(void);
typedef GLboolean (APIENTRY* WGLDESKTOPTARGETAMD) (GLuint);
typedef GLuint (APIENTRY* PFNWGLPRESENTTEXTURETOVIDEOAMD)(GLuint presentTexture, const GLuint* attrib_list);

PFNWGLGETDESKTOPTEXTUREAMD      wglGetDesktopTextureAMD;
PFNWGLENABLEPOSTPROCESSAMD      wglEnablePostProcessAMD;
PFNWGLPRESENTTEXTURETOVIDEOAMD  wglPresentTextureToVideoAMD;
WGLDESKTOPTARGETAMD             wglDesktopTargetAMD;
WGLGENPRESENTTEXTUREAMD         wglGenPresentTextureAMD;


#define GET_PROC(xx)                                    \
{                                                       \
	void **x = (void**)&xx;								\
    *x = (void *) wglGetProcAddress(#xx);               \
    if (*x == NULL) {                                   \
		return false;                                   \
    }                                                   \
}





GLDOPPEngine::GLDOPPEngine() : m_uiDesktopWidth(0), m_uiDesktopHeight(0), m_uiDesktopTexture(0), m_uiPresentTexture(0),
                               m_uiFBO(0), m_uiBaseMap(0), m_pShader(NULL), m_uiVertexBuffer(0), m_uiVertexArray(0),
                               m_bStartPostProcessing(false), m_bDoPresent(true)
{
}



GLDOPPEngine::~GLDOPPEngine()
{
    wglEnablePostProcessAMD(false);

    glFinish();

    if (m_pShader)
    {
        delete m_pShader;
        m_pShader = NULL;
    }

    if (m_uiDesktopTexture)
        glDeleteTextures(1, &m_uiDesktopTexture);

    if (m_uiFBO)
        glDeleteFramebuffers(1,  &m_uiFBO);

    if (m_uiVertexBuffer)
        glDeleteBuffers(1, &m_uiVertexBuffer);

    if (m_uiVertexArray)
        glDeleteVertexArrays(1, &m_uiVertexArray);
}



bool GLDOPPEngine::initDOPP(unsigned int uiDesktop, bool bPresent)
{
    if (!setupDOPPExtension())
    {
        return false;
    }

    // Select Desktop to be processed. ID is the same as seen in CCC
    if (!wglDesktopTargetAMD(uiDesktop))
    {
        return false;
    }
    
    glActiveTexture(GL_TEXTURE1);

    // Get Desktop Texture. Instead of creating a regular texture we request the desktop texture
    m_uiDesktopTexture = wglGetDesktopTextureAMD();
    glBindTexture(GL_TEXTURE_2D, m_uiDesktopTexture);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Get size of the desktop. Usually this is the same values as returned by GetSystemMetrics(SM_CXSCREEN)
    // and GetSystemMetrics(SM_CYSCREEN). In some cases it might differ, e.g. if a rotated desktop is used.
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  (GLint*)&m_uiDesktopWidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, (GLint*)&m_uiDesktopHeight);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Create FBO that is used to generate the present texture. We ill render into this FBO 
    // in order to create the present texture
    glGenFramebuffers(1,  &m_uiFBO);

    // generate present texture
    m_uiPresentTexture = wglGenPresentTextureAMD();

    glBindTexture(GL_TEXTURE_2D, m_uiPresentTexture);
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_uiFBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiPresentTexture, 0);

    GLenum FBStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (FBStatus != GL_FRAMEBUFFER_COMPLETE)
    {  
        return false;
    }

    m_bStartPostProcessing = bPresent; 
    m_bDoPresent           = bPresent;

    return true;
}


bool GLDOPPEngine::initEffect()
{
    if (m_pShader)
    {
        delete m_pShader;
    }

    m_pShader = new GLShader;

	    
	std::string const vertShaderSource(std::string("./data/base.vert"));
	std::string const fragShaderSource(std::string("./data/base.frag"));

    // Load basic shader 
    if (!m_pShader->createVertexShaderFromFile((vertShaderSource.c_str())) )
    {
        return false;
    }

    if (!m_pShader->createFragmentShaderFromFile(fragShaderSource.c_str()))
    {
        return false;
    }

    if (!m_pShader->buildProgram())
    {
        return false;
    }

    m_pShader->bind();

    m_uiBaseMap   = glGetUniformLocation(m_pShader->getProgram(), "baseMap");

    createQuad();

    return true;
}




void GLDOPPEngine::processDesktop()
{
    int pVP[4];

    glGetIntegerv(GL_VIEWPORT, pVP);

    // Bind FBO that has the present texture attached
    glBindFramebuffer(GL_FRAMEBUFFER, m_uiFBO);

    // Set viewport for this FBO
    glViewport(0,0, m_uiDesktopWidth, m_uiDesktopHeight);

    // Render to FBO
    updateTexture();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (m_bDoPresent)
    {
        const GLuint attrib[] = { GL_WAIT_FOR_PREVIOUS_VSYNC, 0 };

        // Set the new desktop texture
        wglPresentTextureToVideoAMD(m_uiPresentTexture, attrib);

        if (m_bStartPostProcessing)
        {
            m_bStartPostProcessing = false;

            wglEnablePostProcessAMD(true);
        }

        glFinish();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // restore original viewport
    glViewport(pVP[0], pVP[1], pVP[2], pVP[3]);
}



void GLDOPPEngine::updateTexture()
{
    m_pShader->bind();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_uiDesktopTexture);

    glUniform1i(m_uiBaseMap, 1);

    glBindVertexArray(m_uiVertexArray);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    m_pShader->unbind();
}



void GLDOPPEngine::createQuad()
{
    const float vec[] = { -1.0f, 1.0f, 0.0f, 1.0f,   -1.0f, -1.0f, 0.0f, 1.0f,   1.0f, 1.0f, 0.0f, 1.0f,    1.0f, -1.0f, 0.0f, 1.0f };
    const float tex[] = {  0.0f,  1.0f,               0.0f,  0.0f,               1.0f, 1.0f,                1.0f, 0.0f };
    
    glGenVertexArrays(1, &m_uiVertexArray);
    glBindVertexArray(m_uiVertexArray);

    glGenBuffers(1, &m_uiVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_uiVertexBuffer);

    glBufferData(GL_ARRAY_BUFFER, 24*sizeof(float), vec, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 16*sizeof(float), 8*sizeof(float), tex);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(4);

    glVertexAttribPointer((GLuint)0, 4, GL_FLOAT, GL_FALSE, 0, 0); 
    glVertexAttribPointer((GLuint)4, 2, GL_FLOAT, GL_FALSE, 0, (void*)(16*sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


bool GLDOPPEngine::setupDOPPExtension()
{
    GET_PROC(wglGetDesktopTextureAMD);
    GET_PROC(wglEnablePostProcessAMD);
    GET_PROC(wglPresentTextureToVideoAMD);
    GET_PROC(wglDesktopTargetAMD);
    GET_PROC(wglGenPresentTextureAMD);
    
    return true;
}