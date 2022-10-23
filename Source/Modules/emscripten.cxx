#include "swigmod.h"
#include "cparse.h"
#include <ctype.h>
#include <iostream>

static const char *usage = (char *) "";

class EMSCRIPTEN:public Language {
public:
  File *f_begin;
  File *f_runtime;
  File *f_cxx_header;
  File *f_cxx_wrapper;

  String *module;
  virtual void main(int argc, char *argv[]);
  virtual int top(Node *n);
  virtual int functionWrapper(Node *n);
  virtual int variableWrapper(Node *n);
  virtual int constantWrapper(Node *n);
  //  virtual int classDeclaration(Node *n);
  virtual int enumDeclaration(Node *n);
  virtual int typedefHandler(Node *n);

  //c++ specific code
  virtual int constructorHandler(Node *n);
  virtual int destructorHandler(Node *n);
  virtual int memberfunctionHandler(Node *n);
  virtual int membervariableHandler(Node *n);
  virtual int classHandler(Node *n);

private:

};

void EMSCRIPTEN::main(int argc, char *argv[]) {
  int i;

  Preprocessor_define("SWIGEMSCRIPTEN 1", 0);
  SWIG_library_directory("emscripten");
  SWIG_config_file("emscripten.swg");

  allow_overloading();
}

int EMSCRIPTEN::top(Node *n) {
  module = Getattr(n, "name");

  String *cxx_filename = Getattr(n, "outfile");

  f_begin = NewFile(cxx_filename, "w", SWIG_output_files());
  if (!f_begin) {
    Printf(stderr, "Unable to open %s for writing\n", cxx_filename);
    SWIG_exit(EXIT_FAILURE);
  }

  f_runtime = NewString("");
  f_cxx_header = NewString("");
  f_cxx_wrapper = NewString("");

  Printf(f_cxx_header, "#include <emscripten/bind.h>\n");

  Swig_register_filebyname("header", f_cxx_header);
  Swig_register_filebyname("wrapper", f_cxx_wrapper);
  Swig_register_filebyname("begin", f_begin);
  Swig_register_filebyname("runtime", f_runtime);

  Swig_banner(f_begin);

  Language::top(n);

  Dump(f_cxx_header, f_begin);
  Dump(f_cxx_wrapper, f_begin);
  Delete(f_runtime);
  Delete(f_begin);
  Delete(f_cxx_header);
  Delete(f_cxx_wrapper);

  return SWIG_OK;
}

int EMSCRIPTEN::classHandler(Node *n) {
  String *name = Getattr(n, "sym:name");
  String *nsname = Getattr(n, "name");
  String *kind = Getattr(n, "kind");
  String *bases = Getattr(n, "bases");
  String *classType = Getattr(n, "classtype");

  bool isListener = Len(name) > 8 && std::string(Char(name)).substr(Len(name) - 8) == "Listener";

  if (std::string(Char(classType)).substr(0, 12) == "std::vector<") {
    String *params = NewString(std::string(Char(classType)).substr(5).c_str());
    Printf(f_cxx_wrapper, "EMSCRIPTEN_BINDINGS(%s) {\n  emscripten::register_%s(\"%s\");\n}\n\n", name, params, name);
  } else if (std::string(Char(classType)).substr(0, 9) == "std::map<") {
    String *params = NewString(std::string(Char(classType)).substr(5).c_str());
    Printf(f_cxx_wrapper, "EMSCRIPTEN_BINDINGS(%s) {\n  emscripten::register_%s(\"%s\");\n}\n\n", name, params, name);
  } else {
    if (isListener) {
      String *className = NewString(name);
      String *listenerHeader = NewString("");
      String *listenerBody = NewString("");
      String *listenerCall = NewString("");
      Printf(listenerHeader, "#include <emscripten/threading.h>\n#include <emscripten/proxying.h>\n\ntypedef union em_variant_val { int i; int64_t i64; float f; double d; void *vp; char *cp; } em_variant_val;\n#define EM_QUEUED_CALL_MAX_ARGS 11\ntypedef struct em_queued_call { int functionEnum; void *functionPtr; _Atomic uint32_t operationDone; em_variant_val args[EM_QUEUED_JS_CALL_MAX_ARGS]; em_variant_val returnValue; void *satelliteData; int calleeDelete; } em_queued_call;\n");
      Printf(listenerHeader, "\nclass EmRunOnMainThread {\npublic:\n");
      Printf(listenerCall, "struct %sWrapper : public emscripten::wrapper<%s> {\n  EMSCRIPTEN_WRAPPER(%sWrapper);\n", name, nsname, name);
      
      for (Node *c = firstChild(n); c; c = nextSibling(c)) {
        String *returnType = Getattr(c, "type");
        Replace(returnType, "(", "", DOH_REPLACE_ANY);
        Replace(returnType, ")", "", DOH_REPLACE_ANY);
        
        String *name = Getattr(c, "name");
        if (returnType) {
          String *params = NewString("");
          String *variables = NewString("");
          String *variables2 = NewString("");
          String *variables3 = NewString("");
          ParmList *pl = Getattr(c, "parms");
          int argnum = 0;
          for (Parm *p = pl; p; p = nextSibling(p), argnum++) {
            String *pName = Getattr(p, "name");
            String *pTypeSplit = Split(Getattr(p, "type"), '.', -1);
            String *pType = Getitem(pTypeSplit, Len(pTypeSplit) - 1);
            Replace(pType, "(", "", DOH_REPLACE_ANY);
            Replace(pType, ")", "", DOH_REPLACE_ANY);
            String *pTypeAll = NewString("");
            String *pTypeAll2 = NewString("");
            if (Len(pTypeSplit) > 1 && Strcmp(Getitem(pTypeSplit, Len(pTypeSplit) - 2), "q(const)") == 0) {
              Printf(pTypeAll, "const %s& %s", pType, pName);
              Printf(pTypeAll2, "const %s", pType);
            } else {
              Printf(pTypeAll, "%s %s", pType, pName);
              Printf(pTypeAll2, "%s", pType);
            }

            if (argnum == 0) {
              Printf(params, "%s", pTypeAll);
            } else {
              Printf(params, ", %s", pTypeAll);
            }
            Printf(variables, ", %s", pName);
            Printf(variables2, "    call.args[%d].vp = (void *)&%s;\n", argnum + 1, pName);
            Printf(variables3, "    %s %s = *((%s*) q->args[%d].vp);\n", pTypeAll2, pName, pTypeAll2, argnum + 1);
            // Printf(f_cxx_wrapper, "%s\n", p);
          }

          String *returnBody = NewString("");
          String *returnCall = NewString("");
          String *returnCallEnd = NewString("");
          Printf(listenerHeader, "  static void %s (void* arg);\n", name);
          if (Strcmp(returnType, "void") != 0) {
            Printf(listenerHeader, "  struct Return%s {\n    Return%s(%s value) {\n      this->value = std::move(value);\n    }\n    %s value;\n  };\n  struct Return%sContainer {\n    std::shared_ptr<Return%s> value;\n  };\n", name, name, returnType, returnType, name, name );
            Printf(returnBody, "    Return%sContainer r = *((Return%sContainer*) q->returnValue.vp);\n    r.value = std::shared_ptr<Return%s>(new Return%s(self->call<%s>(\"%s\"%s)));\n", name, name, name, name, returnType, name, variables);
            Printf(returnCall, "    EmRunOnMainThread::Return%sContainer r;\n    call.returnValue.vp = (void *)&r;\n", name);
            Printf(returnCallEnd, "    return r.value->value;\n");
          } else {
            Printf(returnBody, "    self->call<void>(\"%s\"%s);\n", name, variables);
          }
          // Printf(listenerCall, "  %s %s(%s) { return call<%s>(\"%s\"%s); }\n", returnType, name, params, returnType, name, variables);
          Printf(listenerCall, "  %s %s(%s) {\n    em_queued_call call = {EM_FUNC_SIG_V};\n%s\n    call.args[0].vp = (void *)this;\n%s\n    if (pthread_equal(emscripten_main_browser_thread_id(), pthread_self())) {\n      EmRunOnMainThread::%s(&call);\n    } else {\n      emscripten_proxy_async(emscripten_proxy_get_system_queue(), emscripten_main_browser_thread_id(), EmRunOnMainThread::%s, &call);\n      emscripten_wait_for_call_v(&call, INFINITY);\n    }\n%s  }\n\n", returnType, name, params, returnCall, variables2, name, name, returnCallEnd);
          Printf(listenerBody, "void EmRunOnMainThread::%s (void* arg) {\n    em_queued_call* q = (em_queued_call*)arg;\n    %sWrapper* self = (%sWrapper*) q->args[0].vp;\n%s\n%s    q->operationDone = 1;\n    emscripten_futex_wake(&q->operationDone, INT_MAX);\n}\n", name, className, className, variables3, returnBody);
        }
      }

      Printf(listenerHeader, "};\n");
      Printf(listenerCall, "};\n");
      Printf(f_cxx_wrapper, "%s\n\n%s\n\n%s\n", listenerHeader, listenerCall, listenerBody);
    }

    String *super = NewString("");
    if (bases) {
      Printf(super, ", emscripten::base<%s>", Getattr(First(bases).item, "name"));
    }

    Printf(f_cxx_wrapper, "EMSCRIPTEN_BINDINGS(%s) {\n  emscripten::class_<%s%s>(\"%s\")\n", name, nsname, super, name);
    if (Getattr(n, "feature:shared_ptr")) Printf(f_cxx_wrapper, "    .smart_ptr<std::shared_ptr<%s>>(\"%s\")\n", nsname, name);
    if (isListener) Printf(f_cxx_wrapper, "    .allow_subclass<%sWrapper, std::shared_ptr<%sWrapper>>(\"%sWrapper\", \"%sWrapperSharedPtr\")\n", name, name, name, name);
    Language::classHandler(n);
    Printf(f_cxx_wrapper, "  ;\n}\n\n");
  }

  return SWIG_OK;
}

int EMSCRIPTEN::constructorHandler(Node *n) {
  Language::constructorHandler(n);
  return SWIG_OK;
}

int EMSCRIPTEN::destructorHandler(Node *n) {
  Language::destructorHandler(n);
  return SWIG_OK;
}

int EMSCRIPTEN::memberfunctionHandler(Node *n) {
  return Language::memberfunctionHandler(n);
}

int EMSCRIPTEN::membervariableHandler(Node *n) {
  return Language::membervariableHandler(n);
}

int EMSCRIPTEN::functionWrapper(Node *n) {

  ParmList *parms = Getattr(n, "parms");
  String *name = Getattr(n, "name");
  String *name2 = Getattr(n, "memberfunctionHandler:sym:name");
  String *staticName = Getattr(n, "staticmemberfunctionHandler:name");
  String *type = Getattr(n, "nodeType");

  String *qualifier = Getattr(n, "qualifier");
  String *constFunctionStr = NewString("");

  if (qualifier && Strcmp(qualifier, "q(const).") == 0) {
    Printf(constFunctionStr, "const");
  }


  String *returnTypeSplit = Split(Getattr(n, "type"), '.', -1);
  String *returnType = Getitem(returnTypeSplit, Len(returnTypeSplit) - 1);
  Replace(returnType, "(", "", DOH_REPLACE_ANY);
  Replace(returnType, ")", "", DOH_REPLACE_ANY);

  if (Strcmp(type, "constructor") != 0 && Strcmp(Getitem(returnTypeSplit, 0), "p") == 0) {
    return SWIG_OK;
  }

  std::string nameStr = std::string(Char(name));

  if (nameStr.length() > 9 && nameStr.substr(0, 9) == "operator ") {
    return SWIG_OK;
  }

  Node *parent = parentNode(n);
  String *className = Getattr(parent, "name");
  bool isListener = Len(className) > 8 && std::string(Char(className)).substr(Len(className) - 8) == "Listener";
  bool isUsingSameName = false;

  int order = 0;
  bool order_break = false;

  for (Node *c = firstChild(parent); c; c = nextSibling(c)) {
    String *childName = Getattr(c, "name");
    
    if (!childName) continue;

    if (c == n) {
      order_break = true;
      continue;
    }
    std::string childNameStr = std::string(Char(childName));

    std::string realNameStr = "";
    if (Len(name) != 0 && Len(name2) != 0) {
      realNameStr = nameStr;
    } else if (Len(staticName) != 0) {
      realNameStr = std::string(Char(staticName));
    }

    if (realNameStr == "") continue;

    if (childNameStr == realNameStr) {
      isUsingSameName = true;
      if (!order_break) order += 1;
    }
  }
  
  String *smartPtrParams = NewString("");
  String *constructorParams = NewString("");
  {
    ParmList *pl = Getattr(n, "parms");
    int argnum = 0;
    for (Parm *p = pl; p; p = nextSibling(p), argnum++) {
      String *tempPType = Getattr(p, "type");
      std::string tempPTypeStr = std::string(Char(tempPType));

      if (Strcmp(type, "constructor") == 0 || Len(staticName) != 0 || argnum > 0) {
        if (tempPTypeStr.length() > 2 && tempPTypeStr.substr(0, 2) == "p.") {
          return SWIG_OK;
        }
      }

      String *pTypeSplit = Split(tempPType, '.', -1);
      String *pType = Getitem(pTypeSplit, Len(pTypeSplit) - 1);
      Replace(pType, "(", "", DOH_REPLACE_ANY);
      Replace(pType, ")", "", DOH_REPLACE_ANY);

      if (argnum == 0) Printf(constructorParams, "%s", pType);
      else Printf(constructorParams, ", %s", pType);

      Printf(smartPtrParams, ", %s", pType);
    }
  }

  // Printf(f_cxx_wrapper, "\n%s\n", n);

  if (Strcmp(type, "constructor") == 0) {
    // Printf(f_cxx_wrapper, "\n%s\n", parent);

    if (Getattr(parent, "feature:polymorphic_shared_ptr")) {
      Printf(f_cxx_wrapper, "    .smart_ptr_constructor(\"%s\", &std::make_shared<%s%s>)\n", name, className, smartPtrParams);
    } else {
      Printf(f_cxx_wrapper, "    .constructor<%s>()\n", constructorParams);
    }
  } else if (Len(className) != 0) {
    String *params = NewString("");
    String *variables = NewString("");
    String *types = NewString("");
    ParmList *pl = Getattr(n, "parms");
    int argnum = 0;
    for (Parm *p = pl; p; p = nextSibling(p), argnum++) {
      String *pName = Getattr(p, "name");
      if (Strcmp(pName, "self") == 0) {--argnum; continue;}

      String *pTypeSplit = Split(Getattr(p, "type"), '.', -1);
      String *pType = Getitem(pTypeSplit, Len(pTypeSplit) - 1);
      Replace(pType, "(", "", DOH_REPLACE_ANY);
      Replace(pType, ")", "", DOH_REPLACE_ANY);
      String *pTypeAll = NewString("");
      if (Len(pTypeSplit) > 1 && Strcmp(Getitem(pTypeSplit, Len(pTypeSplit) - 2), "q(const)") == 0) {
        String *temp = NewString("");
        Printf(temp, "const %s&", pType);
        pType = temp;
      }
      Printf(pTypeAll, "%s %s", pType, pName);

      if (argnum == 0) {
        Printf(variables, "%s", pName);
        Printf(types, "%s", pType);
      } else {
        Printf(variables, ", %s", pName);
        Printf(types, ", %s", pType);
      }
      Printf(params, ", %s", pTypeAll);
      // Printf(f_cxx_wrapper, "%s\n", p);
    }

    if (Len(name) != 0 && Len(name2) != 0) {
      if (isListener) {
        if (Getattr(n, "abstract")) {
          Printf(f_cxx_wrapper, "    .function(\"%s\", &%s::%s, emscripten::pure_virtual())\n", name, className, name);
        } else {
          Printf(f_cxx_wrapper, "    .function(\"%s\", emscripten::optional_override([](%s& self%s) {\n      return self.%s(%s);\n    }))\n", name, className, params, name, variables);
        }
      } else if (isUsingSameName) {
        Printf(f_cxx_wrapper, "    .function(\"%s\", emscripten::select_overload<%s(%s)%s>(&%s::%s))\n", name, returnType, types, constFunctionStr, className, name);
      } else {
        Printf(f_cxx_wrapper, "    .function(\"%s\", &%s::%s)\n", name, className, name);
      }
    } else if (Len(staticName) != 0) {
      if (isUsingSameName) {
        Printf(f_cxx_wrapper, "    .class_function(\"%s\", emscripten::select_overload<%s(%s)%s>(&%s::%s))\n", staticName, returnType, types, constFunctionStr, className, staticName);
      } else {
        Printf(f_cxx_wrapper, "    .class_function(\"%s\", &%s::%s)\n", staticName, className, staticName);
      }
    }
    
  }

  return SWIG_OK;
}

int EMSCRIPTEN::constantWrapper(Node *n) {
  return SWIG_OK;
}

int EMSCRIPTEN::variableWrapper(Node *n) {
  return SWIG_OK;
}

int EMSCRIPTEN::typedefHandler(Node *n) {
  return Language::typedefHandler(n);
}

int EMSCRIPTEN::enumDeclaration(Node *n) {
  if (!ImportMode) {
      if (getCurrentClass() && (cplus_mode != PUBLIC)) return SWIG_NOWRAP;

      String *symname = Getattr(n, "sym:name");
      String *name = Getattr(n, "name");

      Printf(f_cxx_wrapper, "EMSCRIPTEN_BINDINGS(%s) {\n  emscripten::enum_<%s>(\"%s\")\n", symname, name, symname);

      for (Node *c = firstChild(n); c; c = nextSibling(c)) {
        String *cName = Getattr(c, "name");
        String *cValue = Getattr(c, "value");
        Printf(f_cxx_wrapper, "    .value(\"%s\", %s)\n", cName, cValue);
      }

      Printf(f_cxx_wrapper, "    ;\n}\n\n");
  }

  return SWIG_OK;
}

extern "C" Language *swig_emscripten(void) {
  return new EMSCRIPTEN();
}
