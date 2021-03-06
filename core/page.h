#ifndef ___PAGE_H9
#define ___PAGE_H9
#include "global.h"
#include "python34.h"
#include<boost/signals2.hpp>
#include<functional>
using boost::signals2::signal;
using boost::signals2::connection;

#define PF_CLOSED 1
#define PF_READONLY 2
#define PF_MUSTSAVEAS 4
#define PF_AUTOLINEBREAK 8
#define PF_WRITETOSTDOUT 0x10

#define PF_NOMARKSEL 0x100000000ULL
#define PF_NOFIND 0x80000000ULL
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
#define PF_NOSAVE 0x100000
#define PF_NORELOAD 0x80000

#define PA_NAME 0
#define PA_ENCODING 1
#define PA_LINE_ENDING 2
#define PA_INDENTATION_MODE 3
#define PA_AUTOLINEBREAK 4
#define PA_FOCUS 5
#define PA_TAB_WIDTH 6

struct export Page;

struct export UndoState {
virtual void Undo (Page&) = 0;
virtual void Redo (Page&) = 0;
virtual bool Join (UndoState& s) { return false; }
virtual int GetTypeId () { return 0; }
virtual ~UndoState(){}
};

struct BoolSignalCombiner {
typedef bool result_type;
template<class I> bool operator() (I start, I end) {
bool re = true;
while(start!=end && (re=*start)) ++start;
return re;
}};

struct VarSignalCombiner {
typedef var result_type;
template<class I> var operator() (I start, I end) {
var re;
while(start!=end) {
re = *start++;
if (re.getType()==T_BOOL && !re) break;
}
return re;
}};

struct export Page: std::enable_shared_from_this<Page>  {
tstring name=TEXT(""), file=TEXT("");
int encoding=-1, indentationMode=-1, tabWidth=-2, lineEnding=-1, markedPosition=0, curUndoState=0;
unsigned long long flags = 0, lastSave=0;
HWND zone=0;
PySafeObject pyData;
std::vector<shared_ptr<UndoState>> undoStates;

signal<void(shared_ptr<Page>)> ondeactivated, onactivated, onclosed, onsaved;
signal<void(shared_ptr<Page>, int,var)> onattrChange;
signal<bool(shared_ptr<Page>), BoolSignalCombiner> onclose, ondeactivate;
signal<bool(shared_ptr<Page>,int), BoolSignalCombiner> onkeyDown, onkeyUp, oncontextMenu;
signal<bool(shared_ptr<Page>, const tstring&, int, int), BoolSignalCombiner> onfileDropped;
signal<var(shared_ptr<Page>,const tstring&), VarSignalCombiner> onsave, onbeforeSave, onload, onkeyPress, onstatus;
signal<var(shared_ptr<Page>, const tstring&, int), VarSignalCombiner> onenter;


virtual ~Page() {}
virtual void SetName (const tstring& name) ;
virtual void SetEncoding (int e);
virtual void SetLineEnding (int e);
virtual void SetIndentationMode (int e);
virtual void SetTabWidth (int e);
virtual void SetAutoLineBreak (bool b);
virtual bool IsEmpty () ;
virtual bool IsModified () ;
virtual void SetModified (bool);
virtual bool IsReadOnly ();
virtual void SetReadOnly (bool);
virtual void CreateZone (HWND parent, bool useEditFieldSubclass=true);
virtual void ResizeZone (const RECT&);
virtual void HideZone ();
virtual void ShowZone (const RECT&);
virtual void FocusZone ();
virtual void Focus ();
virtual void SetFont (HFONT);
virtual bool Close () ;
virtual int LoadFile (const tstring& fn = TEXT(""), bool guessFormat=true ) ;
virtual bool LoadData (const string& data, bool guessFormat=true);
virtual bool Save (bool saveAs=false);
virtual bool SaveFile (const tstring& fn = TEXT(""));
virtual string SaveData ();
virtual bool CheckFileModification ();
virtual void Copy () ;
virtual void Cut ();
virtual void Paste ();
virtual void Undo () ;
virtual void Redo () ;
virtual void PushUndoState (shared_ptr<UndoState> state, bool tryToJoin = true);

virtual PyObject* GetPyData ();
virtual void UpdateStatusBar (HWND) ;
virtual void GetSelection (int& start, int& end);
virtual tstring GetSelectedText () ;
virtual tstring GetText () ;
virtual tstring GetTextSubstring (int start, int end);
virtual int GetTextLength () ;
virtual void ReplaceTextRange (int start, int end, const tstring& str, bool keepOldSelection=true);
virtual tstring GetLine (int line) ;
virtual int GetLineCount () ;
virtual int GetLineLength (int line);
virtual int GetLineStartIndex (int line);
virtual int GetLineSafeStartIndex (int line);
virtual int GetLineIndentLevel (int line);
virtual int GetLineOfPos (int pos);
virtual void SetSelection (int start, int end);
virtual void SetSelectedText (const tstring& str);
virtual void SetText (const tstring& str);

virtual void SelectAll () ;
virtual int GetCurrentPosition ();
virtual void SetCurrentPosition  (int);
virtual void SetCurrentPositionLC (int, int);
virtual void GoToDialog ();
virtual void FindDialog () ;
virtual void FindReplaceDialog () ;
virtual void Find(const tstring& searchText, bool scase, bool regex, bool up, bool stealthty);
virtual void FindNext ();
virtual void FindPrev () ;
virtual void FindReplace (const tstring& search, const tstring& replace, bool caseSensitive, bool isRegex, bool stealthty);

inline int GetSelectionStart () { int s,e; GetSelection(s,e); return s; }
inline int GetSelectionEnd () { int s,e; GetSelection(s,e); return e; }
inline void SetSelectionStart (int x) { SetSelection(x, GetSelectionEnd()); }
inline void SetSelectionEnd (int x) { SetSelection(GetSelectionStart(), x); }
inline void MarkCurrentPosition () { markedPosition = GetCurrentPosition(); }
inline void SelectToMark () { SetSelection(markedPosition, GetSelectionEnd()); }
inline void GoToMark () { SetCurrentPosition(markedPosition); }

//template<class R, R initial, class... A> inline R dispatchEvent (const string& type, A... args) {  return listeners.dispatch<R,initial>(type, *pyData, args...);  }
//template<class... A> inline var dispatchEvent (const string& type, const var& def, A... args) { return listeners.dispatch(type, def, *pyData, args...); }
//template<class... A> inline void dispatchEvent (const string& type, A... args) { listeners.dispatch(type, *pyData, args...); }

int AddEvent (const std::string& type, const PySafeObject& cb) ;
bool RemoveEvent (const std::string& type, int id);
};

template<> struct PyTypeSpec<shared_ptr<Page>> { 
typedef PyObject* type;
static constexpr const char c = 'O'; 
//static inline shared_ptr<Page> convert (PyObject* i) { return i; }
static inline PyObject* convert2 (const shared_ptr<Page>& p) { return p->GetPyData(); }
//static inline PySafeObject convert3 (PyObject* o) {  return o;  }
};

int export AddSignalConnection (const connection& con);
connection export RemoveSignalConnection (int id);

#endif
