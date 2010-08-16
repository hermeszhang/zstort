/*
    WDL - virtwnd.h
    Copyright (C) 2006 and later Cockos Incorporated

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
      

    This file provides interfaces for the WDL Virtual Windows layer, a system that allows
    creating many controls within one system device context.

    The base class is a WDL_VWnd.

    If you create a WDL_VWnd, you should send it (or its parent) mouse messages etc.

    To paint a WDL_VWnd, use a WDL_VWnd_Painter in WM_PAINT etc.


    More documentation should follow...
*/



#ifndef _WDL_VIRTWND_H_
#define _WDL_VIRTWND_H_

#ifdef _WIN32
#include <windows.h>
#else
#include "../swell/swell.h"
#endif
#include "../ptrlist.h"
#include "../wdlstring.h"

#include "virtwnd-skin.h"

#ifndef _WIN32
#define WM_COMMAND 100
#define WM_HSCROLL 101
#define WM_VSCROLL 102
#define WM_USER 14000
#define STN_CLICKED 1000
#define STN_DBLCLK 1001
#define SB_ENDSCROLL 1000
#define SB_THUMBTRACK 1001
#define CBN_SELCHANGE 3000
#endif


class LICE_SysBitmap;
class LICE_IBitmap;


// for mac, HWND should be a subclassed NSView:
// -(int)virtWndCommand:(int)msg, p1:(int)parm1, p2:(int)parm2 sender:(WDL_VWnd*)src

// deprecated
#define WDL_VirtualWnd_ChildList WDL_VWnd
#define WDL_VirtualWnd WDL_VWnd
#define WDL_VirtualWnd_Painter WDL_VWnd_Painter
class WDL_VWnd
{
public:
  WDL_VWnd();
  virtual ~WDL_VWnd();
  virtual void SetID(int id) { m_id=id; }
  virtual int GetID() { return m_id; }
  virtual void SetPosition(const RECT *r) { m_position=*r; }
  virtual void GetPosition(RECT *r) { *r=m_position; }
  virtual void GetPositionPaintExtent(RECT *r) { *r=m_position; }
  virtual void GetPositionPaintOverExtent(RECT *r) { *r=m_position; }
  virtual void SetVisible(bool vis) { m_visible=vis; }
  virtual bool IsVisible() { return m_visible; }
  virtual bool WantsPaintOver() { return m_children && m_children->GetSize(); }
  virtual WDL_VWnd *GetParent() { return m_parent; }
  virtual void SetParent(WDL_VWnd *par) { m_parent=par; }

  virtual void RequestRedraw(RECT *r); 
  virtual void OnPaint(LICE_SysBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect);
  virtual void OnPaintOver(LICE_SysBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect);

  virtual bool OnMouseDown(int xpos, int ypos);
  virtual bool OnMouseDblClick(int xpos, int ypos);
  virtual bool OnMouseWheel(int xpos, int ypos, int amt);

  virtual void OnMouseMove(int xpos, int ypos);
  virtual void OnMouseUp(int xpos, int ypos);

  // child windows
  virtual WDL_VWnd *EnumChildren(int x);
  virtual int GetNumChildren();
  virtual WDL_VWnd *GetChildByID(int id);
  virtual void AddChild(WDL_VWnd *wnd, int pos=-1);
  virtual void RemoveChild(WDL_VWnd *wnd, bool dodel=false);
  virtual void RemoveAllChildren(bool dodel=true);
  virtual WDL_VWnd *GetCaptureWnd() { return m_children ? m_children->Get(m_captureidx) : 0; }
  virtual WDL_VWnd *VirtWndFromPoint(int xpos, int ypos, int maxdepth=-1); // maxdepth=0 only direct children, etc, -1 is unlimited

  // OS access
  virtual HWND GetRealParent() { if (m_realparent) return m_realparent; if (GetParent()) return GetParent()->GetRealParent(); return 0; }
  virtual void SetRealParent(HWND par) { m_realparent=par; }

  virtual int SendCommand(int command, int parm1, int parm2, WDL_VWnd *src);

protected:
  WDL_VWnd *m_parent;
  bool m_visible;
  int m_id;
  RECT m_position;


  HWND m_realparent;
  int m_captureidx;
  int m_lastmouseidx;
  WDL_PtrList<WDL_VWnd> *m_children;
#ifndef _WIN32
  int Mac_SendCommand(HWND hwnd, int msg, int parm1, int parm2, WDL_VWnd *src);
  void Mac_Invalidate(HWND hwnd, RECT *r);
#endif

};

// painting object (can be per window or per thread or however you like)
#define WDL_VWP_SUNKENBORDER 0x00010000
#define WDL_VWP_SUNKENBORDER_NOTOP 0x00020000
#define WDL_VWP_DIVIDER_VERT 0x00030000
#define WDL_VWP_DIVIDER_HORZ 0x00040000


class WDL_VWnd_Painter
{
public:
  WDL_VWnd_Painter();
  ~WDL_VWnd_Painter();


  void SetGSC(int (*GSC)(int));
#ifdef _WIN32
  void PaintBegin(HWND hwnd, int bgcolor=-1);
#else
  void PaintBegin(void *ctx, int bgcolor, RECT *clipr, int wnd_w, int wnd_h);
#endif
  void SetBGImage(LICE_IBitmap *bitmap, int tint=-1) { m_bgbm=bitmap; m_bgbmtintcolor=tint; } // call before every paintbegin (resets if you dont)
  void SetBGGradient(int wantGradient, double start, double slope); // wantg < 0 to use system defaults

  void PaintVirtWnd(WDL_VWnd *vwnd, int borderflags=0);
  void PaintBorderForHWND(HWND hwnd, int borderflags);
  void PaintBorderForRect(const RECT *r, int borderflags);

  LICE_SysBitmap *GetBuffer() { return m_bm; }

  void PaintEnd();

private:

  int m_wantg;
  double m_gradstart,m_gradslope;
  int (*m_GSC)(int);
  void DoPaintBackground(int bgcolor, RECT *clipr, int wnd_w, int wnd_h);
  LICE_SysBitmap *m_bm;
  LICE_IBitmap *m_bgbm;
  int m_bgbmtintcolor;

#ifdef _WIN32
  HWND m_cur_hwnd;
  PAINTSTRUCT m_ps;
#else
  void *m_cur_hwnd;
  struct
  {
    RECT rcPaint;
  } m_ps;
#endif

};


// child list management + UI


// an app should implement these
extern int WDL_STYLE_WantGlobalButtonBorders();
extern bool WDL_STYLE_WantGlobalButtonBackground(int *col);
extern int WDL_STYLE_GetSysColor(int);
extern void WDL_STYLE_ScaleImageCoords(int *x, int *y);
extern bool WDL_Style_WantTextShadows(int *col);

// this is the default, you can override per painter if you want
extern bool WDL_STYLE_GetBackgroundGradient(double *gradstart, double *gradslope); // return values 0.0-1.0 for each, return false if no gradient desired

// for slider
extern HBITMAP WDL_STYLE_GetSliderBitmap(bool vert, int *w, int *h);
extern bool WDL_STYLE_AllowSliderMouseWheel();
extern int WDL_STYLE_GetSliderDynamicCenterPos();

// virtwnd-iconbutton.cpp
class WDL_VirtualIconButton : public WDL_VWnd
{
  public:
    WDL_VirtualIconButton();
    ~WDL_VirtualIconButton();
    void SetEnabled(bool en) {m_en=en; }
    bool GetEnabled() { return m_en; }
    void OnPaint(LICE_SysBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect);
    void OnPaintOver(LICE_SysBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect);
    void SetIcon(WDL_VirtualIconButton_SkinConfig *cfg) { m_iconCfg=cfg; RequestRedraw(NULL); }
    void SetIsButton(bool isbutton) { m_is_button=isbutton; }

    bool OnMouseDown(int xpos, int ypos);
    void OnMouseMove(int xpos, int ypos);
    void OnMouseUp(int xpos, int ypos);
    bool OnMouseDblClick(int xpos, int ypos);

    bool WantsPaintOver();
    void GetPositionPaintOverExtent(RECT *r);

  private:
    bool m_is_button;
    WDL_VirtualIconButton_SkinConfig *m_iconCfg;
    int m_pressed;
    bool m_en;
};



class WDL_VirtualStaticText : public WDL_VWnd
{
  public:
    WDL_VirtualStaticText();
    ~WDL_VirtualStaticText();
    void SetWantSingleClick(bool ws) {m_wantsingle=ws; }
    void OnPaint(LICE_SysBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect);
    void SetFont(HFONT font) { m_font=font; }
    void SetAlign(int align) { m_align=align; } // -1=left,0=center,1=right
    void SetText(const char *text) { m_text.Set(text); if (m_font) RequestRedraw(NULL); }
    void SetBorder(bool bor) { m_wantborder=bor; }
    const char *GetText() { return m_text.Get(); }
    void SetColor(int fg=-1, int bg=-1) { m_fg=fg; m_bg=bg; }
    void SetMargins(int l, int r) { m_margin_l=l; m_margin_r=r; }
    void SetBkImage(LICE_IBitmap **bm) { m_bkbm=bm; }

    bool OnMouseDblClick(int xpos, int ypos);
    bool OnMouseDown(int xpos, int ypos);

  private:
    LICE_IBitmap **m_bkbm;
    int m_align;
    int m_fg,m_bg;
    int m_margin_r, m_margin_l;
    bool m_wantborder;
    bool m_wantsingle;
    HFONT m_font;
    WDL_String m_text;
};

class WDL_VirtualComboBox : public WDL_VWnd
{
  public:
    WDL_VirtualComboBox();
    ~WDL_VirtualComboBox();
    void OnPaint(LICE_SysBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect);
    void SetFont(HFONT font) { m_font=font; }
    void SetAlign(int align) { m_align=align; } // -1=left,0=center,1=right

    int GetCurSel() { if (m_items.Get(m_curitem)) return m_curitem; return -1; }
    void SetCurSel(int sel) { if (!m_items.Get(sel)) sel=-1; if (m_curitem != sel) { m_curitem=sel; if (m_font) RequestRedraw(NULL); } }

    int GetCount() { return m_items.GetSize(); }
    void Empty() { m_items.Empty(true,free); m_itemdatas.Empty(); }

    int AddItem(const char *str, void *data=NULL) { m_items.Add(strdup(str)); m_itemdatas.Add(data); return m_items.GetSize()-1; }
    const char *GetItem(int item) { return m_items.Get(item); }
    void *GetItemData(int item) { return m_itemdatas.Get(item); }

    bool OnMouseDown(int xpos, int ypos);

  private:
    int m_align;
    int m_curitem;
    HFONT m_font;

    WDL_PtrList<char> m_items;
    WDL_PtrList<void> m_itemdatas;
};



class WDL_VirtualSlider : public WDL_VWnd
{
  public:
    WDL_VirtualSlider();
    ~WDL_VirtualSlider();

    void SetBGCol1Callback(int msg) { m_bgcol1_msg=msg; }
    void SetScrollMessage(int msg) { m_scrollmsg=msg; }
    void SetRange(int minr, int maxr, int center) { m_minr=minr; m_maxr=maxr; m_center=center; }
    void GetRange(int *minr, int *maxr, int *center) { if (minr) *minr=m_minr; if (maxr) *maxr=m_maxr; if (center) *center=m_center; }
    int GetSliderPosition();
    void SetSliderPosition(int pos);
    bool GetIsVert();

    void OnPaint(LICE_SysBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect);

    bool OnMouseDown(int xpos, int ypos);
    void OnMouseMove(int xpos, int ypos);
    void OnMouseUp(int xpos, int ypos);
    bool OnMouseDblClick(int xpos, int ypos);
    bool OnMouseWheel(int xpos, int ypos, int amt);

    void SetSkinImageInfo(WDL_VirtualSlider_SkinConfig *cfg) { m_skininfo=cfg; }

    // override
    virtual void GetPositionPaintExtent(RECT *r);

  private:
    WDL_VirtualSlider_SkinConfig *m_skininfo;

    int m_bgcol1_msg,m_scrollmsg;
    void OnMoveOrUp(int xpos, int ypos, int isup);
    int m_minr, m_maxr, m_center, m_pos;

    int m_tl_extra, m_br_extra;

    bool m_captured;
    bool m_needflush;
    
};


#endif