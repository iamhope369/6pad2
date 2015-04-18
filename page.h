#ifndef ___PAGE_H9
#define ___PAGE_H9
#include "global.h"
#include "python34.h"
#include "eventlist.h"
#include<boost/enable_shared_from_this.hpp>

#define PF_CLOSED 1
#define PF_READONLY 2
#define PF_NOSAVE 4
#define PF_AUTOLINEBREAK 8

#define PF_NOFIND 0x80000000
#define PF_NOREPLACE 0x40000000
#define PF_NOGOTO 0x20000000
#define PF_NOENCODING 0x10000000
#define PF_NOINDENTATION 0x8000000
#define PF_NOLINEENDING 0x4000000
#define PF_NOAUTOLINEBREAK 0x2000000
#define PF_NOCOPY 0x1000000
#define PF_NOPASTE 0x800000
#define PF_NOUNDO 0x400000
#define PF_NOSELECTALL 0x200000

struct Page: std::enable_shared_from_this<Page>  {
tstring name=TEXT(""), file=TEXT("");
int encoding=-1, indentationMode=-1, lineEnding=-1, markedPosition=0;
DWORD flags = 0;
PySafeObject pyData;
eventlist listeners;

virtual void CreateZone (HWND parent){}
virtual void ResizeZone (const RECT&) {}
virtual void HideZone () {}
virtual void ShowZone (const RECT&) {}
virtual void FocusZone () {}
virtual PyObject* GetPyData () { return *pyData; }
virtual bool IsEmpty () { return true; }
virtual bool IsModified () { return true; }
virtual void SetModified (bool) {}
virtual tstring LoadText (const tstring& fn = TEXT(""), bool guessFormat = true)  { return TEXT(""); }
virtual bool SaveText (const tstring& fn = TEXT("")) { return true; }

virtual void UpdateStatusBar (HWND) {}
virtual void GetSelection (int& start, int& end) {}
virtual tstring GetSelectedText ()  { return TEXT(""); }
virtual int GetTextLength () {}
virtual tstring GetText () { return TEXT(""); }
virtual void SetSelection (int start, int end) {}
virtual void SetSelectedText (const tstring& str) {}
virtual void SetText (const tstring& str) {}
virtual void ReplaceTextRange (int start, int end, const tstring& str) {}
virtual tstring GetLine (int line)  { return TEXT(""); }
virtual int GetLineCount ()  { return -1; }
virtual int GetLineLength (int line) { return -1; }
virtual int GetLineStartIndex (int line) { return -1; }
virtual int GetLineOfPos (int pos) { return -1; }

inline int GetSelectionStart () { int s,e; GetSelection(s,e); return s; }
inline int GetSelectionEnd () { int s,e; GetSelection(s,e); return e; }
inline void SetSelectionStart (int x) { SetSelection(x, GetSelectionEnd()); }
inline void SetSelectionEnd (int x) { SetSelection(GetSelectionStart(), x); }
inline void MarkCurrentPosition () { markedPosition = GetCurrentPosition(); }
inline void SelectToMark () { SetSelection(markedPosition, GetSelectionEnd()); }


virtual void SetName (const tstring& name) ;
virtual void Copy ()  {}
virtual void Cut ()  {}
virtual void Paste ()  {}
virtual void SelectAll () {}
virtual void SetCurrentPosition (int pos) {}
virtual int GetCurrentPosition () { return -1; }
virtual void GoToDialog () {}
virtual void FindDialog ()  {}
virtual void FindReplaceDialog ()  {}
virtual void FindNext () {}
virtual void FindPrev ()  {}
virtual void FindReplace (const tstring& search, const tstring& replace, bool caseSensitive, bool isRegex) {}

template<class R, R initial, class... A> inline R dispatchEvent (const string& type, A... args) { return listeners.dispatch<R,initial>(type, *pyData, args...); }
template<class... A> inline var dispatchEvent (const string& type, const var& def, A... args) { return listeners.dispatch(type, def, *pyData, args...); }
template<class... A> inline void dispatchEvent (const string& type, A... args) { listeners.dispatch(type, *pyData, args...); }
inline void addEvent (const std::string& type, const PyCallback& cb) { listeners.add(type,cb); }
inline void removeEvent (const std::string& type, const PyCallback& cb) { listeners.remove(type,cb); }

virtual ~Page () { printf("Page destroyed!\r\n"); }
static inline Page* createDummy() { return new Page(); }
};

struct TextPage: Page {
HWND zone=0;

virtual bool IsEmpty () ;
virtual bool IsModified () ;
virtual void SetModified (bool);
virtual void CreateZone (HWND parent);
virtual void ResizeZone (const RECT&);
virtual void HideZone ();
virtual void ShowZone (const RECT&);
virtual void FocusZone ();
virtual tstring LoadText (const tstring& fn = TEXT(""), bool guessFormat=true ) ;
virtual bool SaveText (const tstring& fn = TEXT(""));
virtual void Copy () ;
virtual void Cut ();
virtual void Paste ();

virtual PyObject* GetPyData ();
virtual void UpdateStatusBar (HWND) ;
virtual void GetSelection (int& start, int& end);
virtual tstring GetSelectedText () ;
virtual tstring GetText () ;
virtual int GetTextLength () ;
virtual void ReplaceTextRange (int start, int end, const tstring& str);
virtual tstring GetLine (int line) ;
virtual int GetLineCount () ;
virtual int GetLineLength (int line);
virtual int GetLineStartIndex (int line);
virtual int GetLineOfPos (int pos);
virtual void SetSelection (int start, int end);
virtual void SetSelectedText (const tstring& str);
virtual void SetText (const tstring& str);

virtual void SelectAll () ;
virtual int GetCurrentPosition ();
virtual void SetCurrentPosition  (int);
virtual void GoToDialog ();
virtual void FindDialog () ;
virtual void FindReplaceDialog () ;
virtual void FindNext ();
virtual void FindPrev () ;
virtual void FindReplace (const tstring& search, const tstring& replace, bool caseSensitive, bool isRegex);

static inline Page* create () { return new TextPage(); }
};

#endif
