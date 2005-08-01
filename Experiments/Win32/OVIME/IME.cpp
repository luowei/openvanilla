#include "OVIME.h"

void
MyGenerateMessage(HIMC hIMC, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LPINPUTCONTEXT lpIMC;

	if((lpIMC = ImmLockIMC(hIMC)) == NULL)
		return;    
    
    if (IsWindow(lpIMC->hWnd))
    {
        LPTRANSMSG lpTransMsg;
        if (!(lpIMC->hMsgBuf = ImmReSizeIMCC(lpIMC->hMsgBuf,
			(lpIMC->dwNumMsgBuf + 1) * sizeof(TRANSMSG))))
            return;
		
        if (!(lpTransMsg = (LPTRANSMSG)ImmLockIMCC(lpIMC->hMsgBuf)))
            return;
		
        lpTransMsg += (lpIMC->dwNumMsgBuf);
        lpTransMsg->uMsg = msg;
		lpTransMsg->wParam=wParam;
		lpTransMsg->lParam=lParam;

        ImmUnlockIMCC(lpIMC->hMsgBuf);
        lpIMC->dwNumMsgBuf++;
		
        ImmGenerateMessage(hIMC);
    }
	ImmUnlockIMC(hIMC); 
}

BOOL
MyGenerateMessageToTransKey(LPDWORD lpdwTransKey, UINT *uNumTranMsgs,
							UINT msg, WPARAM wParam, LPARAM lParam) 
{
	// This code is from FreePY. 
	// It seems that the first byte is the number of bytes.
	LPDWORD lpdwTemp;
	
	if (((*uNumTranMsgs) + 1) >= (UINT)*lpdwTransKey)
        return FALSE;
	
	lpdwTemp = (LPDWORD)lpdwTransKey+1+(*uNumTranMsgs)*3;
	*(lpdwTemp++) = msg;
	*(lpdwTemp++) = wParam;
	*(lpdwTemp++) = lParam;

	(*uNumTranMsgs)++;

    return TRUE;
}

BOOL APIENTRY 
ImeInquire(LPIMEINFO lpIMEInfo, LPTSTR lpszUIClass, LPCTSTR lpszOption)
{
    lpIMEInfo->dwPrivateDataSize = sizeof(MYPRIVATE);

    lpIMEInfo->fdwProperty = IME_PROP_KBD_CHAR_FIRST | IME_PROP_UNICODE;
                             //IME_PROP_SPECIAL_UI;
							 //IME_PROP_UNICODE

    lpIMEInfo->fdwConversionCaps = IME_CMODE_FULLSHAPE |
								IME_CMODE_NATIVE;

    lpIMEInfo->fdwSentenceCaps = IME_SMODE_NONE;
    lpIMEInfo->fdwUICaps = UI_CAP_2700;

	lpIMEInfo->fdwSCSCaps = 0;

    lpIMEInfo->fdwSelectCaps = SELECT_CAP_CONVERSION;

    _tcscpy(lpszUIClass, UICLASSNAME);

    return TRUE;
}

BOOL APIENTRY 
ImeConfigure(HKL hKL,HWND hWnd, DWORD dwMode, LPVOID lpData)
{
	InvalidateRect(hWnd,NULL,FALSE);
	return TRUE;
}

DWORD APIENTRY 
ImeConversionList(HIMC hIMC,LPCTSTR lpSource,LPCANDIDATELIST lpCandList,DWORD dwBufLen,UINT uFlag)
{
    return 0;
}

BOOL APIENTRY 
ImeDestroy(UINT uForce)
{
    return FALSE;
}

LRESULT APIENTRY 
ImeEscape(HIMC hIMC,UINT uSubFunc,LPVOID lpData)
{
	return FALSE;
}

BOOL APIENTRY 
ImeProcessKey(HIMC hIMC, UINT uVKey, LPARAM lKeyData, CONST LPBYTE lpbKeyState)
{
	LPINPUTCONTEXT lpIMC;
	/* Note IMC's private is used for exchange info between UI and Conversion
	   Interface */

	if (!hIMC)
		return 0;

	if (!(lpIMC = ImmLockIMC(hIMC)))
		return 0;

	ImmUnlockIMC(hIMC);

	return TRUE; 
}

BOOL APIENTRY 
ImeSelect(HIMC hIMC, BOOL fSelect)
{
    LPINPUTCONTEXT lpIMC;
    
    if (!hIMC)
        return TRUE;

	if (!fSelect)
		return TRUE;

    if (lpIMC = ImmLockIMC(hIMC)) {		
		LPCOMPOSITIONSTRING lpCompStr;
		LPCANDIDATEINFO lpCandInfo;
		LPMYPRIVATE lpMyPrivate;
		
		// Resize of compsiting string of IMC
		lpIMC->hCompStr = ImmReSizeIMCC(lpIMC->hCompStr,sizeof(MYCOMPSTR));
		if (lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr))
		{
			InitCompStr(lpCompStr);
			ImmUnlockIMCC(lpIMC->hCompStr);
		}
		lpIMC->hCandInfo = ImmReSizeIMCC(lpIMC->hCandInfo,sizeof(MYCAND));
		if (lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo))
		{
			InitCandInfo(lpCandInfo);
			ImmUnlockIMCC(lpIMC->hCandInfo);
		}
		
		if ((lpMyPrivate = (LPMYPRIVATE)ImmLockIMCC(lpIMC->hPrivate)) != NULL) {
			for(int i = 0; i < MAXSTRSIZE; i++)
				lpMyPrivate->PreEditStr[i]='\0';
			for(int i = 0; i < MAXSTRSIZE; i++)
				lpMyPrivate->CandStr[i]='\0';
			ImmUnlockIMCC(lpIMC->hPrivate);	
		}
		ImmUnlockIMC(hIMC);
    }
	return TRUE;
}

BOOL APIENTRY 
ImeSetActiveContext(HIMC hIMC,BOOL fFlag)
{
    return TRUE;
}

UINT APIENTRY 
ImeToAsciiEx (UINT uVKey, UINT uScanCode,
			  CONST LPBYTE lpbKeyState,
			  LPDWORD lpdwTransKey, UINT fuState,HIMC hIMC)
{
	LPINPUTCONTEXT lpIMC;
	LPCOMPOSITIONSTRING lpCompStr;

	UINT uNumTranKey;
	LPMYPRIVATE lpMyPrivate;

	int k;
	char str[1024];
	int rlen;
	char result[100][1024];
		
	if (!hIMC)
		return 0;

	if (!(lpIMC = ImmLockIMC(hIMC)))
		return 0;

	// �Ȯɤ��z�| KeyUP
	if (uScanCode & 0x8000)
		return 0;

	lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
	lpMyPrivate = (LPMYPRIVATE)ImmLockIMCC(lpIMC->hPrivate);

	uNumTranKey = 0;

	if (_tcslen(GETLPCOMPSTR(lpCompStr)) == 0)
		MyGenerateMessageToTransKey(lpdwTransKey, &uNumTranKey,
			WM_IME_STARTCOMPOSITION, 0, 0);
	k = LOWORD(uVKey);
	if( k >= 65 && k <= 90)
		k = k + 32;
	switch(LOWORD(uVKey))
	{
	case VK_PRIOR: // pageup
		k = 11;
		break;
	case VK_NEXT: // pagedown
		k = 12;
		break;
	case VK_END:
		k = 4;
		break;
	case VK_HOME:
		k = 1;
		break;
	case VK_LEFT:
		k = 28;
		break;
	case VK_UP:
		k = 30;
		break;
	case VK_RIGHT:
		k = 29;
		break;
	case VK_DOWN:
		k = 31;
		break;
	case VK_DELETE:
		k = 127;
		break;
	case 189: // -
		k = 45;
		break;
	case 0xba: // ;
		k = ';';
		break;
	case 0xc0: // `
		k = '`';
		break;
	case 0xbc: // ,
		k = ',';
		break;
	case 0xbe: // .
		k = '.';
		break;
	case 0xbf: // /
		k = '/';
		break;
	case 0xdc:
		k = '\\';
		break;
	case 0xde: // '
		k = '\'';
		break;
	case 0xdb: // [
		k = '[';
		break;
	case 0xdd: // ]
		k = ']';
		break;
	case 0xbb: // =
		k = '=';
		break;
	case 'Q':
		k = 'q';
	default:
		DebugLog("uVKey: %x\n", (void*)LOWORD(uVKey));
	}
	
	memset(str, 0, 1024);
	rlen = keyevent(tolower(k), str);
	int n = 0;
	int ln = 0;
	DebugLog("str: %s\n", str);
	for( int i = 0; i < rlen; i++ )
	{
		if(str[i] == ' ' || i == rlen - 1)
		{
			int dst = i - 1;
			if( i == rlen - 1)
				dst = i + 1;
			memset(result[n], 0, 100);
			strncpy(result[n], str+ln, dst - ln + 1);
			ln = i + 1;
			n++;
		}
	}

	LPTSTR decoded = NULL;
	_tcscpy(lpMyPrivate->CandStr, _T(""));
	_tcscpy(lpMyPrivate->PreEditStr, _T(""));
	for( int j = 0; j < n; j++ )
	{
		char *x = result[j];
		if(!strcmp(x, "bufclear"))
		{
			_tcscpy(lpMyPrivate->PreEditStr, _T(""));
			MakeCompStr(lpMyPrivate, lpCompStr);
			MyGenerateMessageToTransKey(lpdwTransKey, &uNumTranKey,
				WM_IME_COMPOSITION, 0, GCS_COMPSTR);
		}
		else if(!strcmp(x, "bufupdate"))
		{
			j++;
			decoded = UTF16toWCHAR(result[j]);

			_tcscpy(lpMyPrivate->PreEditStr, decoded);
			MakeCompStr(lpMyPrivate, lpCompStr);
			MyGenerateMessageToTransKey(lpdwTransKey, &uNumTranKey,
				WM_IME_COMPOSITION, 0, GCS_COMPSTR);
		}
		else if(!strcmp(x, "bufsend"))
		{
			j++;
			decoded = UTF16toWCHAR(result[j]);

			_tcscpy(GETLPRESULTSTR(lpCompStr),decoded);
			lpCompStr->dwResultStrLen = _tcslen(decoded);
			_tcscpy(lpMyPrivate->PreEditStr, _T(""));
			MakeCompStr(lpMyPrivate, lpCompStr);
			
			MyGenerateMessageToTransKey(lpdwTransKey, &uNumTranKey,
				WM_IME_COMPOSITION, 0, GCS_RESULTSTR);
			MyGenerateMessageToTransKey(lpdwTransKey, &uNumTranKey,
				WM_IME_ENDCOMPOSITION, 0, 0);
		}
		else if(!strcmp(x, "candiclear"))
		{
			ClearCandidate((LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo));
			ImmUnlockIMCC(lpIMC->hCandInfo);
		}
		else if(!strcmp(x, "candiupdate"))
		{
			j++;
			decoded = UTF16toWCHAR(result[j]);
			_tcscpy(lpMyPrivate->CandStr,decoded);
			UpdateCandidate(lpIMC, decoded);
			MyGenerateMessageToTransKey(lpdwTransKey, &uNumTranKey,
				WM_IME_COMPOSITION, 0, GCS_COMPSTR);
		}
		else if(!strcmp(x, "candishow"))
		{
		}
		else if(!strcmp(x, "candihide"))
		{
			HideCandWindow();
		}
		else if(!strcmp(x, "unprocessed"))
		{
			LPTSTR s = _tcsdup(_T("ORZ"));
			_stprintf(s, _T("%c"), k);
			_tcscpy(GETLPRESULTSTR(lpCompStr), s);
			lpCompStr->dwResultStrLen = _tcslen(s);
			
			MyGenerateMessageToTransKey(lpdwTransKey, &uNumTranKey,
				WM_IME_COMPOSITION, 0, GCS_RESULTSTR);
			MyGenerateMessageToTransKey(lpdwTransKey, &uNumTranKey,
				WM_IME_ENDCOMPOSITION, 0, 0);
			free(s);
		}
		else if(!strcmp(x, "processed"))
		{

		}
		//if(decoded)
		//	free(decoded);
	}

	ImmUnlockIMCC(lpIMC->hPrivate);
	ImmUnlockIMCC(lpIMC->hCompStr);
	ImmUnlockIMC(hIMC);
	
	return uNumTranKey;
}

BOOL APIENTRY
NotifyIME(HIMC hIMC,DWORD dwAction,DWORD dwIndex,DWORD dwValue)
{
    BOOL bRet = FALSE;
	LPINPUTCONTEXT lpIMC;
	
    switch(dwAction)
    {
	case NI_OPENCANDIDATE:
		break;
	case NI_CLOSECANDIDATE:
		break;
	case NI_SELECTCANDIDATESTR:
		break;
	case NI_CHANGECANDIDATELIST:
		break;
	case NI_SETCANDIDATE_PAGESTART:
		break;
	case NI_SETCANDIDATE_PAGESIZE:
		break;
	case NI_CONTEXTUPDATED:
		switch (dwValue)
		{
		case IMC_SETCOMPOSITIONWINDOW:
			break;
		case IMC_SETCONVERSIONMODE:
			break;
		case IMC_SETSENTENCEMODE:
			break;
		case IMC_SETCANDIDATEPOS:
			break;
		case IMC_SETCOMPOSITIONFONT:
			break;
		case IMC_SETOPENSTATUS:
			lpIMC = ImmLockIMC(hIMC);
			if (lpIMC)
			{

			}

			bRet = TRUE;
			break;
		default:
			break;
		}
		break;
		
	case NI_COMPOSITIONSTR:
		switch (dwIndex)
		{
		case CPS_COMPLETE:
			break;
		case CPS_CONVERT:
			break;
		case CPS_REVERT:
			break;
		case CPS_CANCEL:
			break;
		default:
			break;
		}
		break;
			
	default:
		break;
    }
    return bRet;
}

BOOL APIENTRY
ImeRegisterWord(LPCTSTR lpRead, DWORD dw, LPCTSTR lpStr)
{
    return FALSE;
}

BOOL APIENTRY
ImeUnregisterWord(LPCTSTR lpRead, DWORD dw, LPCTSTR lpStr)
{
    return FALSE;
}

UINT APIENTRY
ImeGetRegisterWordStyle(UINT nItem, LPSTYLEBUF lp)
{
	return 0;
}

UINT APIENTRY
ImeEnumRegisterWord(REGISTERWORDENUMPROC lpfn,
					LPCTSTR lpRead, DWORD dw,
					LPCTSTR lpStr, LPVOID lpData)
{
	return 0;
}

BOOL APIENTRY
ImeSetCompositionString(HIMC hIMC, DWORD dwIndex,
						LPCVOID lpComp, DWORD dwComp,
						LPCVOID lpRead, DWORD dwRead)
{
    return FALSE;
}