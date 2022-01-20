#include "CTCPListener.h"


CTCPListener::CTCPListener(string ipAddress, string port, MessageReceived handler) :
	m_ipAddress(ipAddress), m_port(port), m_pfnMessageReceived(handler)
{
    m_bIsRunning = true;
    m_socketServer = new CSocketServer(m_port);
    g_pfnListener = this;
}

CTCPListener::~CTCPListener()
{
    delete m_socketServer;
}


unsigned _stdcall HandleClientThreadFunc(void* pArguments)
{
    CSocket* pListener = (CSocket*)pArguments;
    bool isConnected = true;
    string sData;

    while (isConnected)
    {
        try
        {
            sData = pListener->Receive();
            if (sData.empty())
                break;
            for (int i = 0; i < g_pfnListener->GetListClients().size(); i++)
            {
                if (g_pfnListener->GetListClients()[i] != pListener)
                    g_pfnListener->GetListClients()[i]->Send(sData);
            }
        }
        catch (int nError)
        {
            isConnected = false;
        }
    }
    sData = "";
    sData = pListener->GetIP() + " is disconnected.\r\n";
    g_pfnListener->m_pfnMessageReceived(pListener, sData);
    for (int i = 0; i < g_pfnListener->GetListClients().size(); i++)
    {
        if (g_pfnListener->GetListClients()[i] != pListener)
            g_pfnListener->GetListClients()[i]->Send(sData);
    }
    g_pfnListener->Remove(pListener);
    return 0;
}
void CTCPListener::Remove(CSocket* socket)
{
    v_socketClients.erase(std::remove(v_socketClients.begin(), v_socketClients.end(), socket), v_socketClients.end());
    delete socket;
}
void CTCPListener::Stop()
{
    m_bIsRunning = false;
    m_socketServer->Cleanup();
}
void CTCPListener::Run()
{
    string sMessage ="";
    if (!m_socketServer->Initialize(m_port))
        return;

    while (m_bIsRunning)
    {
        try
        {
            sMessage = "";
            CSocket* socket = m_socketServer->Accept();
            sMessage = socket->GetIP() + " Has connected.\r\n";
            m_pfnMessageReceived(socket, sMessage.c_str());
            socket->Send("Welcome to Lorenzo Leonardo's Chat Messenger\r\n");

            for (int i = 0; i < v_socketClients.size(); i++)
            {
                v_socketClients[i]->Send(sMessage);
            }
            v_socketClients.push_back(socket);

            HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, &HandleClientThreadFunc, socket, 0, 0);
        }
        catch (int nError)
        {
            m_bIsRunning = false;
        }
    }
}