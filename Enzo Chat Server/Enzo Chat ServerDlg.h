
// Enzo Chat ServerDlg.h : header file
//

#pragma once
#include "..\EnzTCP\CSocket.h"
#include <vector>
#include <string>
#ifdef _DEBUG
#pragma comment(lib, "..\\x64\\Debug\\EnzTCP.lib")
#else
#pragma comment(lib, "..\\x64\\Release\\EnzTCP.lib")
#endif

using namespace std;



// CEnzoChatServerDlg dialog
class CEnzoChatServerDlg : public CDialog
{
// Construction
public:
	CEnzoChatServerDlg(CWnd* pParent = nullptr);	// standard constructor
	~CEnzoChatServerDlg()
	{
		delete m_vSocket;
	}
	
	void DisplayConnectedClients()
	{
		int nItem = 0;
	
	//	m_ctrlListConnected.InsertItem(LVIF_TEXT | LVIF_STATE, nItem,
		//	csNumber, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED, 0, 0);
		m_ctrlListConnected.DeleteAllItems();
		for (int nRow = 0; nRow < m_vSocket->size(); nRow++)
		{
			m_ctrlListConnected.InsertItem(LVIF_TEXT | LVIF_STATE, nRow,
				(* m_vSocket)[nRow]->GetIP().c_str(), LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED, 0, 0);
			//m_ctrlListConnected.SetItemText(nItem, nRow + 1, m_vSocket[col]);
		}

		nItem++;
	}
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ENZO_CHAT_SERVER_DIALOG };
#endif

	void SetConnectionHandle(HANDLE handle)
	{
		m_ServerHandle = handle;
	}
	vector < ISocket*>  *GetVectorList()
	{
		return m_vSocket;
	}
	void UpdateChatAreaText(string stringData)
	{
		char stringText[MAX_BUFFER_SIZE];
		m_ctrlChatArea.GetWindowText(stringText, sizeof(stringText));
		string newData(stringText);

		newData += stringData;
		m_ctrlChatArea.SetWindowText(newData.c_str());
	}


	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

		// This is for input area.
	CEdit m_ctrlInputArea;
	// This is for chat area.
	CEdit m_ctrlChatArea;
	// This is for clients connected to the server.
	CListCtrl m_ctrlListConnected;

// Implementation
protected:
	HICON m_hIcon;
	HANDLE m_ServerHandle;
	vector<ISocket*> *m_vSocket;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnClose();
};
//


//void CEnzoChatServerDlg::NewClientConnection(void* pData);
//UINT CEnzoChatServerDlg::HandleClientThreadFunc(LPVOID pParam);