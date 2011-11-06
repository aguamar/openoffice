/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/




#undef UNICODE
#undef _UNICODE

#define _WIN32_WINDOWS 0x0410

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <msiquery.h>
#include <shellapi.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>
#include <assert.h>
#include <string.h>

#ifdef UNICODE
#define _UNICODE
#define _tstring	wstring
#else
#define _tstring	string
#endif
#include <tchar.h>
#include <string>


static std::_tstring GetMsiProperty( MSIHANDLE handle, const std::_tstring& sProperty )
{
    std::_tstring result;
    TCHAR szDummy[1] = TEXT("");
    DWORD nChars = 0;

    if ( MsiGetProperty( handle, sProperty.c_str(), szDummy, &nChars ) == ERROR_MORE_DATA )
    {
        DWORD nBytes = ++nChars * sizeof(TCHAR);
        LPTSTR buffer = reinterpret_cast<LPTSTR>(_alloca(nBytes));
        ZeroMemory( buffer, nBytes );
        MsiGetProperty(handle, sProperty.c_str(), buffer, &nChars);
        result = buffer;
    }

    return result;
}

extern "C" UINT __stdcall copyExtensionData(MSIHANDLE handle) {

    std::_tstring sSourceDir = GetMsiProperty( handle, TEXT("SourceDir") );
    std::_tstring sExtensionDir = sSourceDir + TEXT("extension\\");
    std::_tstring sPattern = sExtensionDir + TEXT("*.oxt");
    // std::_tstring mystr;

    // Finding all oxt files in sExtensionDir

    WIN32_FIND_DATA aFindFileData;

    HANDLE hFindOxt = FindFirstFile( sPattern.c_str(), &aFindFileData );

    if ( hFindOxt != INVALID_HANDLE_VALUE )
    {
        bool fNextFile = false;
        bool fSuccess = true;
        bool bFailIfExist = true;

        std::_tstring sDestDir = GetMsiProperty( handle, TEXT("INSTALLLOCATION") );
        std::_tstring sShareInstallDir = sDestDir + TEXT("share\\extension\\install\\");

        // creating directories
        std::_tstring sShareDir = sDestDir + TEXT("share");
        std::_tstring sExtDir = sShareDir + TEXT("\\extension");
        std::_tstring sExtInstDir = sExtDir + TEXT("\\install");
        bool bDir = CreateDirectory(sShareDir.c_str(), NULL);
        bDir = CreateDirectory(sExtDir.c_str(), NULL);
        bDir = CreateDirectory(sExtInstDir.c_str(), NULL);

        do
        {
            std::_tstring sOxtFile = aFindFileData.cFileName;

            std::_tstring sSourceFile = sExtensionDir + sOxtFile;
            std::_tstring sDestFile = sShareInstallDir + sOxtFile;

            fSuccess = CopyFile( sSourceFile.c_str(), sDestFile.c_str(), bFailIfExist );

            fNextFile = FindNextFile( hFindOxt, &aFindFileData );

        } while ( fNextFile );

        FindClose( hFindOxt );
    }

    return ERROR_SUCCESS;
}
