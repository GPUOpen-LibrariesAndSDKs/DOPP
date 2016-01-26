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



#include "StdAfx.h"

#define _USE_MATH_DEFINES

#include <math.h>

#include <GL/glew.h>

#include "GLShader.h"
#include "GLDOPPRotate.h"



GLDOPPRotate::GLDOPPRotate() : GLDOPPEngine(),
                               m_pProgram(NULL), m_uiBasmapLoc(0)
{
    // Init model view
    memset(m_pModelViewMat, 0, 16 * sizeof(float));
    
    m_pModelViewMat[0]  = 1.0f;      // [0][0]
    m_pModelViewMat[5]  = 1.0f;      // [1][1]
    m_pModelViewMat[10] = 1.0f;      // [1][1]
    m_pModelViewMat[15] = 1.0f;      // [1][1]

    // Init ortho projection matrix as identity
    memset(m_pProjectionMat, 0, 16 * sizeof(float));

    m_pProjectionMat[0]  = 1.0f;
    m_pProjectionMat[5]  = 1.0f;
    m_pProjectionMat[10] = 1.0f;
    m_pProjectionMat[15] = 1.0f;
}


GLDOPPRotate::~GLDOPPRotate(void)
{
    if (m_pProgram)
    {
        delete m_pProgram; m_pProgram = NULL;
    }

    if (m_uiTransform)
    {
        glDeleteBuffers(1, &m_uiTransform);
    }
}



bool GLDOPPRotate::initEffect()
{
    if (m_pProgram)
    {
        delete m_pProgram; m_pProgram = NULL;
    }

    m_pProgram = new GLShader;

	std::string const vertShaderSource(std::string("./data/transform.vert"));
	std::string const fragShaderSource(std::string("./data/base.frag"));
    if ((!m_pProgram->createVertexShaderFromFile(vertShaderSource.c_str())) || (!m_pProgram->createFragmentShaderFromFile(fragShaderSource.c_str())))
    {
        return false;
    }

    if (!m_pProgram->buildProgram())
    {
        return false;
    }

    m_uiBasmapLoc = glGetUniformLocation(m_pProgram->getProgram(), "baseMap");

    // create UBO to pass transformation matrix
    glGenBuffers(1, &m_uiTransform);

    glBindBuffer(GL_UNIFORM_BUFFER, m_uiTransform);

    glBufferData(GL_UNIFORM_BUFFER, 32 * sizeof(float), m_pModelViewMat, GL_STATIC_DRAW);

    glBufferSubData(GL_UNIFORM_BUFFER, 16 * sizeof(float), 16 * sizeof(float), m_pProjectionMat);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    createQuad();

    return true;
}


void GLDOPPRotate::setRotation(float a)
{
    float r = ((float)M_PI * a) / 180.0f; 

    float ar = (float)m_uiDesktopWidth / (float)m_uiDesktopHeight;

    glBindBuffer(GL_UNIFORM_BUFFER, m_uiTransform);

    float* pMat = (float*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, 32 * sizeof(float), GL_MAP_WRITE_BIT  | GL_MAP_UNSYNCHRONIZED_BIT);

    if (pMat)
    {
        // Build rotation matrix to rotate quad around z axis and scale y to
        // have a quad that matched the AR of the desktop texture
        pMat[0] =  cosf(r);
        pMat[1] = -sinf(r) ;

        pMat[4] =  sinf(r) / ar;
        pMat[5] =  cosf(r) / ar;  

        // Build otho projection matrix (glOrtho(-1.0f, 1.0f, -1.0f/ar, 1.0f/ar, -1.0f, 1.0f))
        // Only Orth[1][1] need to be updated. The ortho frustum needs to match the AR of the desktop
        pMat[21] = ar;
    }

    glUnmapBuffer(GL_UNIFORM_BUFFER);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void GLDOPPRotate::updateTexture()
{
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uiTransform);

    m_pProgram->bind();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_uiDesktopTexture);

    glUniform1i(m_uiBasmapLoc, 1);

    glBindVertexArray(m_uiVertexArray);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    m_pProgram->unbind();
}
