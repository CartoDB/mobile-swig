/* Define a C preprocessor symbol that can be used in interface files
   to distinguish between the SWIG language modules. */ 

#define SWIG_EMSCRIPTEN

/* Typespecs for basic types. */

%typemap(cin) void ":void";

%typemap(cin) char ":char";
%typemap(cin) char * ":string";
%typemap(cin) unsigned char ":unsigned-char";
%typemap(cin) signed char ":char";

%typemap(cin) short ":short";
%typemap(cin) signed short ":short";
%typemap(cin) unsigned short ":unsigned-short";

%typemap(cin) int ":int";
%typemap(cin) signed int ":int";
%typemap(cin) unsigned int ":unsigned-int";

%typemap(cin) long ":long";
%typemap(cin) signed long ":long";
%typemap(cin) unsigned long ":unsigned-long";

%typemap(cin) long long ":long-long";
%typemap(cin) signed long long ":long-long";
%typemap(cin) unsigned long long ":unsigned-long-long";

%typemap(cin) float ":float";
%typemap(cin) double ":double";
%typemap(cin) SWIGTYPE ":pointer";

%typemap(cout) void ":void";

%typemap(cout) char ":char";
%typemap(cout) char * ":string";
%typemap(cout) unsigned char ":unsigned-char";
%typemap(cout) signed char ":char";

%typemap(cout) short ":short";
%typemap(cout) signed short ":short";
%typemap(cout) unsigned short ":unsigned-short";

%typemap(cout) int ":int";
%typemap(cout) signed int ":int";
%typemap(cout) unsigned int ":unsigned-int";

%typemap(cout) long ":long";
%typemap(cout) signed long ":long";
%typemap(cout) unsigned long ":unsigned-long";

%typemap(cout) long long ":long-long";
%typemap(cout) signed long long ":long-long";
%typemap(cout) unsigned long long ":unsigned-long-long";

%typemap(cout) float ":float";
%typemap(cout) double ":double";
%typemap(cout) SWIGTYPE ":pointer";


%typemap(ctype) bool                       "int";
%typemap(ctype) char, unsigned char, signed char,
                short, signed short, unsigned short,
                int, signed int, unsigned int,
                long, signed long, unsigned long,
                float, double, long double, char *, void *, void,
                enum SWIGTYPE, SWIGTYPE *,
                SWIGTYPE[ANY], SWIGTYPE &  "$1_ltype";
%typemap(ctype) SWIGTYPE                   "$&1_type";

%typemap(in) bool                          "$1 = (bool)$input;";
%typemap(in) char, unsigned char, signed char,
             short, signed short, unsigned short,
             int, signed int, unsigned int,
             long, signed long, unsigned long,
             float, double, long double, char *, void *, void,
             enum SWIGTYPE, SWIGTYPE *,
             SWIGTYPE[ANY], SWIGTYPE &     "$1 = $input;";
%typemap(in) SWIGTYPE                      "$1 = *$input;";

%typemap(out) void                         "";
%typemap(out) bool                          "$result = (int)$1;";
%typemap(out) char, unsigned char, signed char,
              short, signed short, unsigned short,
              int, signed int, unsigned int,
              long, signed long, unsigned long,
              float, double, long double, char *, void *,
              enum SWIGTYPE, SWIGTYPE *,
              SWIGTYPE[ANY], SWIGTYPE &    "$result = $1;";

%typecheck(SWIG_TYPECHECK_BOOL) bool { $1 = 1; };
%typecheck(SWIG_TYPECHECK_CHAR) char { $1 = 1; };
%typecheck(SWIG_TYPECHECK_FLOAT) float { $1 = 1; };
%typecheck(SWIG_TYPECHECK_DOUBLE) double { $1 = 1; };
%typecheck(SWIG_TYPECHECK_STRING) char * { $1 = 1; };
%typecheck(SWIG_TYPECHECK_INTEGER)
                    unsigned char, signed char,
                    short, signed short, unsigned short,
                    int, signed int, unsigned int,
                    long, signed long, unsigned long,
                    enum SWIGTYPE { $1 = 1; };
%typecheck(SWIG_TYPECHECK_POINTER) SWIGTYPE *, SWIGTYPE &,
                                   SWIGTYPE[ANY], SWIGTYPE { $1 = 1; };
