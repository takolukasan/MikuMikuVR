#include "stdafx.h"
#include <imagehlp.h>

BOOL HookFunctionOfModule(HMODULE hTargetModule, const char *szTargetFunctionName, void *pHookFunction, void **pTargetFunctionPrevious)
{
	PIMAGE_IMPORT_DESCRIPTOR pImgImpDesc;
	ULONG ulSize;
	PIMAGE_SECTION_HEADER pImgSectHeader;

	char *szImportModule;
	PIMAGE_THUNK_DATA pImport;
	PIMAGE_THUNK_DATA pIAT;
	PIMAGE_IMPORT_BY_NAME pImportName;

	size_t len1,len2;
	INT nCompareResult;
	LPVOID lpWriteAddress;
	void *OldAddr;
	DWORD dwOldProtect;

	pImgImpDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToDataEx(hTargetModule, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize, &pImgSectHeader);
	if( !pImgImpDesc ) {
		return FALSE;
	}

	while( pImgImpDesc->Name ) {
		szImportModule = (char *)((intptr_t)hTargetModule + pImgImpDesc->Name);
		pImport = (PIMAGE_THUNK_DATA)((intptr_t)hTargetModule + pImgImpDesc->OriginalFirstThunk);
		pIAT = (PIMAGE_THUNK_DATA)((intptr_t)hTargetModule + pImgImpDesc->FirstThunk);

		for(; pIAT->u1.Function; pImport++, pIAT++) {
			if( IMAGE_SNAP_BY_ORDINAL(pImport->u1.Ordinal) )	/* 序数は使えねぇ */
				continue;

			/* インポートしてる関数名を取り出す */
			pImportName = (PIMAGE_IMPORT_BY_NAME)((intptr_t)hTargetModule + pImport->u1.AddressOfData);
			if( FAILED(StringCchLengthA((char *)pImportName->Name, MAX_PATH, &len1)) ) continue;
			if( FAILED(StringCchLengthA(szTargetFunctionName, MAX_PATH, &len2)) ) continue;

			/* フック対象関数名？ */
			nCompareResult = CompareStringA(LOCALE_USER_DEFAULT, NORM_IGNORECASE, (char *)pImportName->Name, (int)len1, szTargetFunctionName, (int)len2);
			if( CSTR_EQUAL == nCompareResult ) {
				lpWriteAddress = (LPVOID)(&pIAT->u1.Function);
				OldAddr = (LPVOID)((intptr_t)pIAT->u1.Function);

				if( !VirtualProtect(lpWriteAddress, sizeof(pIAT->u1.Function), PAGE_READWRITE, &dwOldProtect) )
					return FALSE; // エラー
				
				// DO HOOK
				// WriteProcessMemoryに失敗対策も丸投げ
				WriteProcessMemory(GetCurrentProcess(), lpWriteAddress, &pHookFunction, sizeof(pIAT->u1.Function), NULL);

				VirtualProtect(lpWriteAddress, sizeof(pIAT->u1.Function), dwOldProtect, &dwOldProtect);

				*pTargetFunctionPrevious = OldAddr;

				return TRUE;
			}
		}

		pImgImpDesc++;
	}

	return FALSE;
}

