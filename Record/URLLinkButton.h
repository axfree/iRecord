#if !defined(AFX_URLLINKBUTTON_H__19A95AD8_340E_4CE0_9971_138A765EF60A__INCLUDED_)
#define AFX_URLLINKBUTTON_H__19A95AD8_340E_4CE0_9971_138A765EF60A__INCLUDED_

/////////////////////////////////////////////////////////////////////////////
// File:	URLLinkButton.h
// By:		Nguyen Duc Thanh, Vegasoft, JSC.
//			Copyright 2004, Nguyen Duc Thanh.
// Email:	thanhnd@vegasoft.ws
//			thanhnd@eztechs.com
// Website: http:\\www.vegasoft.ws
// Date:	July 15, 2004
/////////////////////////////////////////////////////////////////////////////
//
// The CURLLinkButton class extends the functionality of CButton 
// by providing support for URL links
// It displays the URL link and invokes the shell when clicked.
// Tooltip supported, full color customization
//
/////////////////////////////////////////////////////////////////////////////
//
// This code is free for non-commercial use, providing this notice remains 
// intact in the source files and all eventual changes are clearly marked 
// with comments. Do not misrepresent the origin of this code.
//
// No warrantee of any kind, express or implied, is included with this
// software; use at your own risk
// 
// If you find this code useful, credits would be nice!
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// URLLinkButton.h : header file
//
const UINT WM_LINK_CLICKED = ::RegisterWindowMessage (_T ("WM_LINK_CLICKED"));

/////////////////////////////////////////////////////////////////////////////
// CURLLinkButton window

class CURLLinkButton : public CButton
{
// Construction
public:
	CURLLinkButton();

// Attributes
public:
	static HCURSOR		m_hCursorHand;
	static CFont*		m_pULFont;
	
protected:
	COLORREF		m_clrRegular;
	COLORREF		m_clrHover;
	COLORREF		m_clrVisited;
	COLORREF		m_clrDisabled;

	BOOL		m_bHover;			// indicates if mouse is over the button
	BOOL		m_bTracking;
	BOOL		m_bVisited;

	CString		m_sURL;
	CString		m_sPrefix;
	
private:
	CToolTipCtrl m_ToolTip;
// Operations
public:
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CURLLinkButton)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SizeToContent();
	void SetLinkColor(COLORREF clrRegular, COLORREF clrHover, COLORREF clrVisited);
	void SetToolTipColor(COLORREF clrTextColor, COLORREF clrBkColor);
	void SetToolTipText(CString sTip=_T(""));
	void SetToolTipBkColor(COLORREF clrBkgrdColor);
	void SetToolTipTextColor(COLORREF clrTextColor);
	
	void SetVisitedColor(COLORREF clrVisited);
	void SetHoverColor(COLORREF clrHover);
	void SetRegularColor(COLORREF clrRegular);
	
	CString GetDefaultTipText();
	
	void SetURL (LPCTSTR lpszURL);			// By default, window text will be used
	void SetURLPrefix (LPCTSTR lpszPrefix);	// For example "mailto:"
	
	virtual ~CURLLinkButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CURLLinkButton)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnMouseHover(WPARAM wparam, LPARAM lparam) ;
	afx_msg void OnClicked();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_URLLINKBUTTON_H__19A95AD8_340E_4CE0_9971_138A765EF60A__INCLUDED_)
