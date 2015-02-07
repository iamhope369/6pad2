#ifndef ___PYTHON___HPP___9
#define ___PYTHON___HPP___9
#include<python/python.h>
#include "strings.hpp"
#include<tuple>
#include<string>

#define GIL_PROTECT RAII_GIL ___RAII_GIL_VAR_##__LINE__;

typedef PyObject*(*PyCFunc)(PyObject*,PyObject*);

struct RAII_GIL {
PyGILState_STATE gil;
RAII_GIL():  gil(PyGILState_Ensure()) {}
~RAII_GIL(){ PyGILState_Release(gil); }
};

void PyStart (void);


///Automatic wrappers!

template<class T> struct PyTypeSpec {  };

template<> struct PyTypeSpec<int> { 
typedef int type;
static constexpr const char c = 'i'; 
static inline int convert (int i) { return i; }
static inline int convert2 (int i) { return i; }
static inline int convert3 (PyObject* o) { return PyLong_AsLong(o); }
};

template<> struct PyTypeSpec<DWORD> { 
typedef DWORD type;
static constexpr const char c = 'I'; 
static inline DWORD convert (DWORD i) { return i; }
static inline DWORD convert2 (DWORD i) { return i; }
static inline DWORD convert3 (PyObject* o) { return PyLong_AsLongLong(o); }
};

template<> struct PyTypeSpec<size_t> { 
typedef size_t type;
static constexpr const char c = 'n'; 
static inline size_t convert (size_t i) { return i; }
static inline size_t convert2 (size_t i) { return i; }
static inline int convert3 (PyObject* o) { return PyLong_AsLongLong(o); }
};

template<> struct PyTypeSpec<double> { 
typedef double type;
static constexpr const char c = 'd'; 
static inline double convert (double i) { return i; }
static inline double convert2 (double i) { return i; }
static inline double convert3 (PyObject* o) { return PyFloat_AsDouble(o); }
};

template<> struct PyTypeSpec<bool> { 
typedef bool type;
static constexpr const char c = 'p'; 
static inline bool convert (bool i) { return i; }
static inline bool convert2 (bool i) { return i; }
static inline bool convert3 (PyObject* o) { return PyLong_AsLong(o); }
};

template<> struct PyTypeSpec<std::string> { 
typedef char* type;
static constexpr const char c = 's'; 
static inline std::string convert (const char* s) { return s; }
static inline const char* convert2 (const std::string& s) { return s.c_str(); }
static inline string convert3 (PyObject* o) { return toString(PyUnicode_AsUnicode(o)); }
};

template<> struct PyTypeSpec<const std::string&> { 
typedef char* type;
static constexpr const char c = 's'; 
static inline std::string convert (const char* s) { return s; }
static inline const char* convert2 (const std::string& s) { return s.c_str(); }
};

template<> struct PyTypeSpec<std::wstring> { 
typedef wchar_t* type;
static constexpr const char c = 'u'; 
static inline std::wstring convert (const wchar_t* s) { return s; }
static inline const wchar_t* convert2 (const std::wstring& s) { return s.c_str(); }
static inline wstring convert3 (PyObject* o) { return toWString(PyUnicode_AsUnicode(o)); }
};

template<> struct PyTypeSpec<const std::wstring&> { 
typedef wchar_t* type;
static constexpr const char c = 'u'; 
static inline std::wstring convert (const wchar_t* s) { return s; }
static inline const wchar_t* convert2 (const std::wstring& s) { return s.c_str(); }
};

template<class... Args> inline const char* PyTypeSpecs (void) {
static constexpr const int n = sizeof...(Args);
static constexpr const char cc[n+1] = { PyTypeSpec<Args>::c... ,0};
return cc;
};

template<class... Args> inline const char* PyTypeSpecsTuple (void) {
static constexpr const int n = sizeof...(Args);
static constexpr const char cc[n+3] = { '(', PyTypeSpec<Args>::c... , ')', 0};
return cc;
};

template<class R, class... A> R PyCallFunc (PyObject* func, A... args) {
PyObject* argtuple = Py_BuildValue(PyTypeSpecsTuple<A...>(), PyTypeSpec<A>::convert2(args)...);
PyObject* pyResult = PyObject_CallObject(func, argtuple);
Py_XDECREF(argtuple);
R cResult = PyTypeSpec<R>::convert3(pyResult);
Py_XDECREF(pyResult);
return cResult;
}

template<int... S> struct TemplateSequence {};
template<int N, int... S> struct TemplateSequenceGenerator: TemplateSequenceGenerator<N -1, N -1, S...> {};
template<int... S> struct TemplateSequenceGenerator<0, S...> { typedef TemplateSequence<S...> sequence; };

template<class... A> struct PyParseTupleSpec {
template<int... S> static int PyArg_ParseTuple (TemplateSequence<S...> seq, PyObject* pyTuple, const char* pyArgSpec, std::tuple<typename PyTypeSpec<A>::type...>& args) { return ::PyArg_ParseTuple(pyTuple, pyArgSpec, &std::get<S>(args)...); }
};

template<class R, class... A> struct PyCTupleCallSpec {
template<int... S> static R call (TemplateSequence<S...> seq, R(*f)(A...), std::tuple<typename PyTypeSpec<A>::type...>& args) {   return f(  PyTypeSpec<typename std::tuple_element<S, std::tuple<A...>>::type>::convert(std::get<S>(args))...);  }
};

template<class... A> struct PyCTupleCallSpec<void, A...> {
template<int... S> static void call (TemplateSequence<S...> seq, void(*f)(A...), std::tuple<typename PyTypeSpec<A>::type...>& args) {  f(  PyTypeSpec<typename std::tuple_element<S, std::tuple<A...>>::type>::convert(std::get<S>(args))...);  }
};

template<class CFunc> struct PyFuncSpec {
template<class R, class... A> static inline PyObject* func2 (R(*cfunc)(A...), PyObject* pySelf, PyObject* pyArgs) {
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq;
std::tuple<typename PyTypeSpec<A>::type...> argtuple;
if (!PyParseTupleSpec<A...>::PyArg_ParseTuple(seq, pyArgs, PyTypeSpecs<A...>(), argtuple)) return NULL;
R result = PyCTupleCallSpec<R,A...>::call(seq, cfunc, argtuple);
return Py_BuildValue(PyTypeSpecs<R>(), PyTypeSpec<R>::convert2(result) );
}
template<class... A> static inline PyObject* func2 (void(*cfunc)(A...), PyObject* pySelf, PyObject* pyArgs) {
typename TemplateSequenceGenerator<sizeof...(A)>::sequence seq;
std::tuple<typename PyTypeSpec<A>::type...> argtuple;
if (!PyParseTupleSpec<A...>::PyArg_ParseTuple(seq, pyArgs, PyTypeSpecs<A...>(), argtuple)) return NULL;
PyCTupleCallSpec<void,A...>::call(seq, cfunc, argtuple);
Py_RETURN_NONE;
}
template<CFunc cfunc> static PyObject* func (PyObject* pySelf, PyObject* pyArgs) { return func2(cfunc, pySelf, pyArgs); }
};

#define PyDecl(n,f) {(n), (PyFuncSpec<decltype(f)>::func<f>), METH_VARARGS, NULL}

#endif
