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



#include <fstream>
#include <GL/glew.h>

#include "GLShader.h"
 
using namespace std;


GLShader::GLShader() : m_uiVertexShader(0), m_uiFragmentShader(0), m_uiProgram(0)
{
    m_strErrorMessage.clear();
}


GLShader::~GLShader()
{
    glUseProgram(0);

    glDeleteShader(m_uiVertexShader);
    glDeleteShader(m_uiFragmentShader);

    glDeleteProgram(m_uiProgram);
}


bool GLShader::createVertexShaderFromFile(const char* pFileName)
{
    string strSource;
    
    if (!readShaderSource(pFileName, strSource))
    {
        return false;
    }

    const char* pSource = strSource.c_str();

    if (!createShader(pSource, GL_VERTEX_SHADER))
    {
        return false;
    }

    return true;
}


bool GLShader::createFragmentShaderFromFile(const char* pFileName)
{
    string strSource;
    
    if (!readShaderSource(pFileName, strSource))
    {
        return false;
    }

    const char* pSource = strSource.c_str();

    if (!createShader(pSource, GL_FRAGMENT_SHADER))
    {
        return false;
    }

    return true;
}


bool GLShader::createVertexShaderFromString(const char* pSource)
{
    if (!createShader(pSource, GL_VERTEX_SHADER))
    {
        return false;
    }

    return true;
}


bool GLShader::createFragmentShaderFromString(const char* pSource)
{
    if (!createShader(pSource, GL_FRAGMENT_SHADER))
    {
        return false;
    }

    return true;
}

 

bool GLShader::buildProgram()
{
    if (!m_uiVertexShader || !m_uiFragmentShader)
    {
        return false;
    }

    if (m_uiProgram)
    {
        glDeleteProgram(m_uiProgram);
    }

    m_uiProgram = glCreateProgram();

    glAttachShader(m_uiProgram, m_uiVertexShader);
    glAttachShader(m_uiProgram, m_uiFragmentShader);

    glLinkProgram(m_uiProgram);

    int  nStatus;
    int  nLength;
    char cMessage[256];

    glGetProgramiv(m_uiProgram, GL_LINK_STATUS, &nStatus);

    if (nStatus != GL_TRUE)
    {
        glGetProgramInfoLog(m_uiProgram, 256, &nLength, cMessage);

        m_strErrorMessage = cMessage;

        return false;
    }

    return true;
}



bool GLShader::createShader(const char* pSource, unsigned int uiType)
{
    unsigned int uiShader = 0;

    if (uiType == GL_VERTEX_SHADER)
    {
        m_uiVertexShader = glCreateShader(GL_VERTEX_SHADER);

        uiShader = m_uiVertexShader;
    }
    else if (uiType == GL_FRAGMENT_SHADER)
    {
        m_uiFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        uiShader = m_uiFragmentShader;
    }

    glShaderSource(uiShader, 1, &pSource, NULL);

    glCompileShader(uiShader);

    int  nStatus;
    int  nLength;
    char cMessage[256];

    glGetShaderiv(uiShader, GL_COMPILE_STATUS, &nStatus);

    if (nStatus != GL_TRUE)
    {
        glGetShaderInfoLog(uiShader, 256, &nLength, cMessage);

        m_strErrorMessage = cMessage;

        return false;
    }

    return true;
}


bool GLShader::readShaderSource(const char* pFileName, std::string &strSource)
{
    string		strLine;
	ifstream	ShaderFile(pFileName);

	if (!ShaderFile.is_open())
		return false;

	while(!ShaderFile.eof())
	{
		getline(ShaderFile, strLine);
		strSource += strLine;
        strSource += "\n";
	}
	
	return true;
}

