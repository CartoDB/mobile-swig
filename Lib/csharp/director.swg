/* -----------------------------------------------------------------------------
 * director.swg
 *
 * This file contains support for director classes so that C# proxy 
 * methods can be called from C++.
 * ----------------------------------------------------------------------------- */

#ifdef __cplusplus

#if defined(DEBUG_DIRECTOR_OWNED)
#include <iostream>
#endif
#include <string>

namespace Swig {
  /* Director base class */
  class Director {
  public:
    enum { RELEASE_HANDLE = 0, WEAK_HANDLE = 1, STRONG_HANDLE = 2 };
    typedef void* (*SWIG_gchandle_delegate)(void *, int);

    Director() : swig_self_(0), swig_gchandle_delegate_(0) {
    }

    virtual ~Director() {
      swig_disconnect_director_self();
    }

    bool swig_set_self(SWIG_gchandle_delegate gchandle_delegate, void *self, bool mem_own) {
      if (swig_self_) {
        return false;
      }
      swig_gchandle_delegate_ = gchandle_delegate;
      swig_self_ = self;
      if (mem_own && swig_gchandle_delegate_) {
        swig_gchandle_delegate_(swig_self_, STRONG_HANDLE);
      }
      return true;
    }

    void *swig_get_self() const {
      return swig_self_;
    }

    // Change C++ object's ownership, relative to C#
    void swig_csharp_change_ownership(void *self, bool take_or_release) {
      if (swig_gchandle_delegate_) {
        swig_self_ = swig_gchandle_delegate_(self, take_or_release ? WEAK_HANDLE : STRONG_HANDLE);
      }
    }

  protected:
    void swig_disconnect_director_self() {
      if (swig_gchandle_delegate_) {
        swig_gchandle_delegate_(swig_self_, RELEASE_HANDLE);
      }
      swig_self_ = 0;
    }

  private:
    void* swig_self_;
    SWIG_gchandle_delegate swig_gchandle_delegate_;
  };

  /* Base class for director exceptions */
  class DirectorException {
  protected:
    std::string swig_msg;

  public:
    DirectorException(const char* msg) : swig_msg(msg) {
    }
    DirectorException(const std::string &msg) : swig_msg(msg) {
    }
    const std::string& what() const {
      return swig_msg;
    }
    virtual ~DirectorException() {
    }
  };

  /* Pure virtual method exception */
  class DirectorPureVirtualException : public Swig::DirectorException {
  public:
    DirectorPureVirtualException(const char* msg) : DirectorException(std::string("Attempt to invoke pure virtual method ") + msg) {
    }
  };
}

#endif /* __cplusplus */


