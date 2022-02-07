#include "pch.h"
#include "CSNMP.h"

smiOID    CSNMP::m_psmilOID;
smiVALUE  CSNMP::m_nvalue;

CSNMP::CSNMP()
{

}
CSNMP::~CSNMP()
{

}

bool CSNMP::InitSNMP(const char* szAgentIPAddress, const char* szCommunity, int nVersion, DWORD& dwLastError)
{
    if (WSAStartup(0x202, &m_wsaData) != 0)
        return false;

    m_status = SnmpStartup(&m_ulMajorVersion, &m_ulMinorVersion, &m_ulLevel, &m_ulTranslateMode, &m_ulRetransmitMode);

    if (m_status == SNMPAPI_FAILURE)
        return false;

    if(nVersion == 1)
        SnmpSetTranslateMode(SNMPAPI_UNTRANSLATED_V1); //for v1
    else if(nVersion == 2)
        SnmpSetTranslateMode(SNMPAPI_UNTRANSLATED_V2); //for v2

    m_pSession = (PSNMP_SESSION)SnmpUtilMemAlloc(sizeof(SNMP_SESSION));

    if (m_pSession == NULL)
        return false;

    if (!CreateNotificationWindow(m_pSession))
        return false;

    // create a remote session
    m_pSession->hSnmpSession = SnmpOpen(m_pSession->hWnd, WM_SNMP_INCOMING);
    if (SNMPAPI_FAILURE == m_pSession->hSnmpSession)
    {
        dwLastError = SnmpGetLastError(m_pSession->hSnmpSession);
        SnmpUtilMemFree(m_pSession);
        return false;
    }

    //router's IP
    m_pSession->hAgentEntity = SnmpStrToEntity(m_pSession->hSnmpSession, szAgentIPAddress);
    if (SNMPAPI_FAILURE == m_pSession->hAgentEntity)
    {
        dwLastError = SnmpGetLastError(m_pSession->hSnmpSession);
        SnmpUtilMemFree(m_pSession);
        return false;
    }
    SnmpSetTimeout(m_pSession->hAgentEntity, 5000 / 10);
    // attach retries specified with agent
    SnmpSetRetry(m_pSession->hAgentEntity, 5);

    //local PC
    m_pSession->hManagerEntity = SnmpStrToEntity(m_pSession->hSnmpSession, "127.0.0.1");
    if (SNMPAPI_FAILURE == m_pSession->hManagerEntity)
    {
        dwLastError = SnmpGetLastError(m_pSession->hSnmpSession);
        SnmpUtilMemFree(m_pSession);
        return false;
    }
    // attach timeout specified with manager
    SnmpSetTimeout(m_pSession->hManagerEntity, 5000 / 10);
    // attach retries specified with manager
    SnmpSetRetry(m_pSession->hManagerEntity, 5);

    m_smiCommunity.ptr = (smiLPBYTE)szCommunity;
    m_smiCommunity.len = strlen(szCommunity);

    m_pSession->hViewContext = SnmpStrToContext(m_pSession->hSnmpSession, &m_smiCommunity);

    // validate context handle
    if (SNMPAPI_FAILURE == m_pSession->hViewContext)
    {
        dwLastError = SnmpGetLastError(m_pSession->hSnmpSession);
        SnmpUtilMemFree(m_pSession);
        return false;
    }

    return true;
}
void CSNMP::EndSNMP()
{
    SnmpUtilMemFree(m_pSession);
    WSACleanup();
}

smiVALUE CSNMP::Get(const char* szOID, DWORD &dwLastError)
{
    smiVALUE value;
    m_pSession->nPduType = SNMP_PDU_GET; //Get

    memset(&value, 0, sizeof(value));
    smiOID oid;
    SnmpStrToOid(szOID, &oid);
    m_pSession->hVbl = SnmpCreateVbl(m_pSession->hSnmpSession, &oid, NULL);

    if (SNMPAPI_FAILURE == m_pSession->hVbl)
    {
        dwLastError = SnmpGetLastError(m_pSession->hSnmpSession);
        return value;
    }
    m_pSession->nRequestId = 1;

    m_pSession->hPdu = SnmpCreatePdu(m_pSession->hSnmpSession, m_pSession->nPduType, m_pSession->nRequestId, 0, 0, m_pSession->hVbl);

    if (SNMPAPI_FAILURE == m_pSession->hPdu)
        return value;

    // send the message to the agent
    m_pSession->nError = SnmpSendMsg(m_pSession->hSnmpSession, m_pSession->hManagerEntity, m_pSession->hAgentEntity, m_pSession->hViewContext, m_pSession->hPdu);

    SnmpFreeDescriptor(SNMP_SYNTAX_OID, (smiLPOPAQUE)&oid);
    if (SNMPAPI_FAILURE == m_pSession->nError)
    {
        m_pSession->nError = SnmpGetLastError(m_pSession->hSnmpSession);
        m_pSession->nError = SnmpFreeVbl(m_pSession->hVbl);
        if (SNMPAPI_FAILURE == m_pSession->nError)
        {
            m_pSession->nError = SnmpGetLastError(m_pSession->hSnmpSession);
            return value;
        }
        m_pSession->nError = SnmpFreePdu(m_pSession->hPdu);
        if (SNMPAPI_FAILURE == m_pSession->nError)
        {
            m_pSession->nError = SnmpGetLastError(m_pSession->hSnmpSession);
            return value;
        }
       
    }
    m_pSession->nError = SnmpFreeVbl(m_pSession->hVbl);
    if (SNMPAPI_FAILURE == m_pSession->nError)
    {
        m_pSession->nError = SnmpGetLastError(m_pSession->hSnmpSession);
        return value;
    }
    m_pSession->nError = SnmpFreePdu(m_pSession->hPdu);
    if (SNMPAPI_FAILURE == m_pSession->nError)
    {
        m_pSession->nError = SnmpGetLastError(m_pSession->hSnmpSession);
        return value;
    }
    MSG     uMsg;
    BOOL    fOk = FALSE;


    // get the next message for this session
    while (GetMessage(&uMsg, m_pSession->hWnd, 0, 0))
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
    return m_nvalue;
}

bool    CSNMP::ProcessNotification(PSNMP_SESSION pSession)
{

    BOOL           fDone = TRUE;
    SNMPAPI_STATUS status;
    HSNMP_ENTITY   hAgentEntity = (HSNMP_ENTITY)NULL;
    HSNMP_ENTITY   hManagerEntity = (HSNMP_ENTITY)NULL;
    HSNMP_CONTEXT  hViewContext = (HSNMP_CONTEXT)NULL;
    smiINT32       nPduType;
    smiINT32       nRequestId;


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
                    int                  oidCount;
                    int                  i = 1;
                    LPSTR                string = NULL;

                  
                    oidCount = SnmpCountVbl(pSession->hVbl);


                    SnmpGetVb(pSession->hVbl, i, &m_psmilOID, &m_nvalue);

                }
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

LRESULT CALLBACK CSNMP::NotificationWndProc(HWND   hWnd, UINT   uMsg, WPARAM wParam, LPARAM lParam)
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

bool CSNMP::CreateNotificationWindow(PSNMP_SESSION pSession)
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

}