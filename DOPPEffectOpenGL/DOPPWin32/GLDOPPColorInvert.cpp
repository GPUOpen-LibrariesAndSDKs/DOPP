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
#include <GL/glew.h>


#include "GLShader.h"
#include "GLDOPPColorInvert.h"




GLDOPPColorInvert::GLDOPPColorInvert() : GLDOPPEngine(),
                                         m_pProgram(NULL), m_uiBasmapLoc(0)
{
}


GLDOPPColorInvert::~GLDOPPColorInvert(void)
{
    if (m_pProgram)
    {
        delete m_pProgram; m_pProgram = NULL;
    }
}


bool GLDOPPColorInvert::initEffect()
{
    if (m_pProgram)
    {
        delete m_pProgram; m_pProgram = NULL;
    }

    m_pProgram = new GLShader;

	std::string const vertShaderSource(std::string("./data/base.vert"));
	std::string const fragShaderSource(std::string("./data/colorInv.frag"));
    if ((!m_pProgram->createVertexShaderFromFile(vertShaderSource.c_str())) || (!m_pProgram->createFragmentShaderFromFile(fragShaderSource.c_str())))
    {
        return false;
    }

    if (!m_pProgram->buildProgram())
    {
        return false;
    }

    m_uiBasmapLoc = glGetUniformLocation(m_pProgram->getProgram(), "baseMap");

    createQuad();

    return true;
}


void GLDOPPColorInvert::updateTexture()
{
    m_pProgram->bind();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_uiDesktopTexture);

    glUniform1i(m_uiBasmapLoc, 1);

    glBindVertexArray(m_uiVertexArray);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    m_pProgram->unbind();
}
