#include  <stdio.h>
#include  <string>
#include  <stdlib.h>
#include  <winsock2.h>
#include  <winsnmp.h>
#include  <snmp.h>
#include  <mgmtapi.h>
#include <conio.h>
#include <vector>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Wsnmp32.lib")
#pragma comment(lib, "Snmpapi.lib")
#pragma comment(lib, "Mgmtapi.lib")

#define WM_SNMP_INCOMING WM_USER + 1
#define WM_SNMP_DONE WM_USER + 2

smiUINT32 g_valueBefore;
smiUINT32 g_valueCurrent;
DWORD g_dwTimeBefore;
DWORD g_dwTimeCurrent;

typedef struct _SNMP_SESSION
{
    HSNMP_SESSION     hSnmpSession;     // handle to winsnmp session
    HSNMP_ENTITY      hAgentEntity;     // handle to agent entity
    HSNMP_ENTITY      hManagerEntity;   // handle to manager entity
    HSNMP_CONTEXT     hViewContext;     // handle to view context
    HSNMP_PDU         hPdu;             // handle to snmp pdu
    HSNMP_VBL         hVbl;             // handle to var bind list
    HWND              hWnd;             // handle to window

    smiINT32          nPduType;         // current pdu type
    smiINT32          nRequestId;       // current request id
    smiINT32          nError;           // last system error
    smiINT32          nErrorStatus;     // error status
    smiINT32          nErrorIndex;      // error index

} SNMP_SESSION, * PSNMP_SESSION;

int g_index = 0;
BOOL    ProcessNotification(PSNMP_SESSION pSession);
LRESULT CALLBACK NotificationWndProc(HWND   hWnd, UINT   uMsg, WPARAM wParam, LPARAM lParam);
BOOL CreateNotificationWindow(PSNMP_SESSION pSession);

int __cdecl main(int argc, char** argv)
{
    WSAData              wsaData;
    SNMPAPI_STATUS       status;
    ULONG                ulMajorVersion, ulMinorVersion, ulLevel, ulTranslateMode, ulRetransmitMode;
    PSNMP_SESSION        pSession = NULL;
    smiOCTETS smiCommunity;
    char       szRouterIPAddress[32] = "192.168.0.1";
    char       szCommunity[32] = "public";
    g_dwTimeBefore = GetTickCount();
    g_valueBefore = 0;
    if (WSAStartup(0x202, &wsaData) != 0)
            return -1;

    status = SnmpStartup(&ulMajorVersion, &ulMinorVersion, &ulLevel, &ulTranslateMode, &ulRetransmitMode);

    if(status == SNMPAPI_FAILURE)
        return -1;

    SnmpSetTranslateMode(SNMPAPI_UNTRANSLATED_V1); //for v1

    pSession = (PSNMP_SESSION)SnmpUtilMemAlloc(sizeof(SNMP_SESSION));

    if (pSession == NULL)
        return -1;

    if(!CreateNotificationWindow(pSession))
        return -1;

    // create a remote session
    pSession->hSnmpSession = SnmpOpen(pSession->hWnd, WM_SNMP_INCOMING);
    if(SNMPAPI_FAILURE == pSession->hSnmpSession)
        return -1;

    //router's IP
    pSession->hAgentEntity = SnmpStrToEntity(pSession->hSnmpSession, szRouterIPAddress);
    if (SNMPAPI_FAILURE == pSession->hAgentEntity)
        return -1;
    SnmpSetTimeout(pSession->hAgentEntity, 5000 / 10);
    // attach retries specified with agent
    SnmpSetRetry(pSession->hAgentEntity, 5);

    //local PC
    pSession->hManagerEntity = SnmpStrToEntity(pSession->hSnmpSession, "127.0.0.1");
    if (SNMPAPI_FAILURE == pSession->hManagerEntity)
        return -1;
    // attach timeout specified with manager
    SnmpSetTimeout(pSession->hManagerEntity, 5000 / 10);
    // attach retries specified with manager
    SnmpSetRetry(pSession->hManagerEntity, 5);

    smiCommunity.ptr = (smiLPBYTE)szCommunity;
    smiCommunity.len =strlen(szCommunity);

    pSession->hViewContext = SnmpStrToContext(pSession->hSnmpSession,&smiCommunity);

    // validate context handle
    if (SNMPAPI_FAILURE == pSession->hViewContext)
        return -1;

    pSession->nPduType = SNMP_PDU_GET; //Get

    vector<string> vOID = {".1.3.6.1.2.1.2.2.1.10.9",".1.3.6.1.2.1.2.2.1.16.9"};

    while (true)
    {
        smiOID oid;
        SnmpStrToOid(vOID[g_index].c_str(), &oid);
        pSession->hVbl = SnmpCreateVbl(pSession->hSnmpSession, &oid,NULL);

        if (SNMPAPI_FAILURE == pSession->hVbl)
             return -1;


        pSession->nRequestId = 1;

        // create a pdu using the parameters in pSession structure
        pSession->hPdu = SnmpCreatePdu(pSession->hSnmpSession, pSession->nPduType, pSession->nRequestId, 0, 0, pSession->hVbl);

        if (SNMPAPI_FAILURE == pSession->hPdu)
            return -1;

        // send the message to the agent
        pSession->nError = SnmpSendMsg(pSession->hSnmpSession, pSession->hManagerEntity, pSession->hAgentEntity, pSession->hViewContext, pSession->hPdu);

        //  SnmpFreeDescriptor(SNMP_SYNTAX_OID, (smiLPOPAQUE)&oid);
        if (SNMPAPI_FAILURE == pSession->nError)
        {
            pSession->nError = SnmpGetLastError(pSession->hSnmpSession);
            pSession->nError = SnmpFreeVbl(pSession->hVbl);
            if (SNMPAPI_FAILURE == pSession->nError)
            {
                pSession->nError = SnmpGetLastError(pSession->hSnmpSession);
            }
            pSession->nError = SnmpFreePdu(pSession->hPdu);
            if (SNMPAPI_FAILURE == pSession->nError)
            {
                pSession->nError = SnmpGetLastError(pSession->hSnmpSession);
            }
            return -1;
        }

        pSession->nError = SnmpFreeVbl(pSession->hVbl);
        if (SNMPAPI_FAILURE == pSession->nError)
        {
            pSession->nError = SnmpGetLastError(pSession->hSnmpSession);
        }
        pSession->nError = SnmpFreePdu(pSession->hPdu);
        if (SNMPAPI_FAILURE == pSession->nError)
        {
            pSession->nError = SnmpGetLastError(pSession->hSnmpSession);
        }
        MSG     uMsg;
        BOOL    fOk = FALSE;


        // get the next message for this session
        while (GetMessage(&uMsg, pSession->hWnd, 0, 0))
        {

            // check for private message
            if (uMsg.message != WM_SNMP_DONE)
            {
                TranslateMessage(&uMsg);
                DispatchMessage(&uMsg);
            }
            else
            {
                // success
                fOk = TRUE;
                break;
            }
        }
        Sleep(500);
        g_index++;

        if (g_index == 2)
            g_index = 0;
    }


    return 0;
}
BOOL    ProcessNotification(PSNMP_SESSION pSession)
{

    BOOL           fDone = TRUE;
    SNMPAPI_STATUS status;
    HSNMP_ENTITY   hAgentEntity = (HSNMP_ENTITY)NULL;
    HSNMP_ENTITY   hManagerEntity = (HSNMP_ENTITY)NULL;
    HSNMP_CONTEXT  hViewContext = (HSNMP_CONTEXT)NULL;
    smiINT32       nPduType;
    smiINT32       nRequestId;
    char           szBuf[1024];

    // validate pointer
    if (pSession == NULL)
        return FALSE;

    // retrieve message
    status = SnmpRecvMsg(
        pSession->hSnmpSession,
        &hAgentEntity,
        &hManagerEntity,
        &hViewContext,
        &pSession->hPdu
    );

    // validate return code
    if (status != SNMPAPI_FAILURE)
    {
        // retrieve pdu data
        status = SnmpGetPduData(
            pSession->hPdu,
            &nPduType,
            &nRequestId,
            &pSession->nErrorStatus,
            &pSession->nErrorIndex,
            &pSession->hVbl
        );

        // validate return code            
        if (status != SNMPAPI_FAILURE)
        {

            // process reponse to request
            if (nPduType == SNMP_PDU_RESPONSE)
            {
                // validate context information
                if ((pSession->nRequestId == nRequestId) &&
                    (pSession->hViewContext == hViewContext) &&
                    (pSession->hAgentEntity == hAgentEntity) &&
                    (pSession->hManagerEntity == hManagerEntity))
                {

                    // if we hit the end of tree, break.
                        char                 szBuf[2048];
                        char                 szSecString[2048];
                        int                  oidCount;
                        int                  i = 1;
                        smiINT               result;
                        AsnObjectIdentifier  Oid;      //unfortunate that there is no better way
                        LPSTR                string = NULL;

                        smiOID psmilOID;
                        smiVALUE nvalue;
                        oidCount = SnmpCountVbl(pSession->hVbl);

                       
                        SnmpGetVb(pSession->hVbl, i, &psmilOID, &nvalue);
                        g_dwTimeCurrent = GetTickCount();
                        g_valueCurrent =(nvalue.value.uNumber);
                        //printf("%lf   %d\n", ((double)(g_valueCurrent- g_valueBefore)*8)/((double)( abs((long)g_dwTimeCurrent - (long)g_dwTimeBefore))*1000000), g_dwTimeCurrent  - g_dwTimeBefore);
                       if(g_index == 0)
                            printf("inoctet %u\n", nvalue);
                       else if (g_index == 1)
                           printf("outoctet %u\n", nvalue);
                        g_valueBefore = g_valueCurrent;
                        g_dwTimeBefore = g_dwTimeCurrent;
                        fDone = true;
                }
                else
                {
                    // continue
                    fDone = FALSE;
                }

            }
            else
            {
                fDone = FALSE;
            }

        }
  

        // release temporary entity
        SnmpFreeEntity(hAgentEntity);

        // release temporary entity
        SnmpFreeEntity(hManagerEntity);

        // release temporary context
        SnmpFreeContext(hViewContext);

    }

    // release pdu
    pSession->nError = SnmpFreeVbl(pSession->hVbl);
    if (SNMPAPI_FAILURE == pSession->nError)
    {
        pSession->nError = SnmpGetLastError(pSession->hSnmpSession);
    }
    pSession->nError = SnmpFreePdu(pSession->hPdu);
    if (SNMPAPI_FAILURE == pSession->nError)
    {
        pSession->nError = SnmpGetLastError(pSession->hSnmpSession);
    }

    return fDone;

}  //end of ProcessNotification
LRESULT CALLBACK NotificationWndProc(HWND   hWnd, UINT   uMsg,  WPARAM wParam,  LPARAM lParam)
{
    // check for winsnmp notification
    if (uMsg == WM_SNMP_INCOMING)
    {

        PSNMP_SESSION pSession;

        // retrieve session pointer from window
        pSession = (PSNMP_SESSION)GetWindowLongPtr(hWnd, 0);

        // validate session ptr
        if (pSession == NULL)
            return (LRESULT)0;

        // process notification message
        if (ProcessNotification(pSession))
        {
            // post message to break out of message pump
            PostMessage(pSession->hWnd, WM_SNMP_DONE, (WPARAM)0, (LPARAM)0);
        }

        return (LRESULT)0;

    }
    else
    {
        // forward all other messages to windows
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

} // end of NotificationWndProc
BOOL CreateNotificationWindow(PSNMP_SESSION pSession)
{

    BOOL    fOk;
    WNDCLASSA wc;

    if (pSession == NULL)
    {
        return FALSE;
    }

    // initialize notification window class
    wc.lpfnWndProc = (WNDPROC)NotificationWndProc;
    wc.lpszClassName = "SNMPUTIL NOTIFICATION CLASS";
    wc.lpszMenuName = NULL;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.cbWndExtra = sizeof(PSNMP_SESSION);
    wc.cbClsExtra = 0;
    wc.style = 0;    // register class
    fOk = RegisterClassA(&wc);

    if (!fOk)
    {
        return (FALSE);
    }
    // create notification window
    pSession->hWnd = CreateWindowA(
        "SNMPUTIL NOTIFICATION CLASS",
        "SNMP Util Class",                // pointer to window name
        WS_OVERLAPPEDWINDOW,              // window style
        0,                                // horizontal position of window
        0,                                // vertical position of window
        0,                                // window width
        0,                                // window height
        NULL,                             // handle to parent or owner window
        NULL,                             // handle to menu or child-window identifier
        GetModuleHandle(NULL),                    // handle to application instance
        NULL                              // pointer to window-creation data
    );

    // validate window handle
    if (pSession->hWnd != NULL)
    {
        // store pointer to session in window
        SetWindowLongPtr(pSession->hWnd, 0, (INT_PTR)pSession);

        // success
        fOk = TRUE;
    }
    else
    {
        // failure
        fOk = FALSE;
    }

    return fOk;

} // end of CreateNotificationWindow
