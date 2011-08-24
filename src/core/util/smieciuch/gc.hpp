/*
 * Smieciuch -- portable GC library for C
 * (c) 2003-2005  InForma-Sebastian Kaliszewski
 * published under NCSA License
 *
 * Code is based on a one associated with
 * article by William E. Kempf published
 * at The Code Project site in Jan 2001
 * http://www.codeproject.com/cpp/garbage_collect2.asp
 *
 * (large) portions by William E. Kempf  2001
 *
 * gc.hpp
 * class templates wraping around C interface
 */

#ifndef HDR__GC_HPP__
#define HDR__GC_HPP__	1

#include "Config.h"

#ifndef __cplusplus
#  error "this file works only in C++"
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1000)
#  pragma once
#endif

#ifdef _MSC_VER
#  pragma warning(disable : 4786)
#endif

#ifndef _SMGC_POOR_STL
#  include <cstdlib>
#  include <cstddef>
#  include <climits>
#  include <cassert>
#else
#  include <stdlib.h>
#  include <stddef.h>
#  include <limits.h>
#  include <assert.h>
#endif
#include <exception>
#include <stdexcept>
#include <functional>
#include <new>

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
#  define _SMGC_NO_FRIEND_TEMPLATES
#  define _SMGC_NO_ARRAY_NEW
#  define _SMGC_POOR_TEMPLATES
#endif

#if defined(_SMGC_POOR_TEMPLATES) && !defined(_SMGC_NO_FRIEND_TEMPLATES)
#  define _SMGC_NO_FRIEND_TEMPLATES
#endif

namespace gc
{
  using namespace std;

  template<typename T> class gc_ptr;
  template<typename T> class const_gc_ptr;
  template<typename T> class gc_arr;
  template<typename T> class const_gc_arr;
  template<typename T> class wk_ptr;
  template<typename T> class const_wk_ptr;
  template<typename T> class wk_arr;
  template<typename T> class const_wk_arr;

  typedef std::bad_alloc bad_alloc;
  typedef std::invalid_argument invalid_argument;

  void collect();
  void set_threshold(size_t bytes);
  size_t get_threshold();
  size_t get_dynamic_threshold();

  extern const size_t no_threshold;
  extern const size_t default_threshold;

  struct bad_reg: public exception
  {
    virtual ~bad_reg() throw();
    virtual const char *what() const throw();
  };

  struct no_space: public exception
  {
    virtual ~no_space() throw();
    virtual const char *what() const throw();
  };

  class mem_corrupt: public exception
  {
    char* msg;
  public:
    mem_corrupt(const char *txt="");
    virtual ~mem_corrupt() throw();
    virtual const char *what() const throw();
  };


  namespace detail
  {
    struct gc_t {};

    struct gc_null_t
    {
      operator void* () const
      {
        return 0;
      }
    };

    template <typename T>
    struct destructor
    {
      static void destroy(void* obj, void* base, size_t)
      {
        T* ptr = static_cast<T*>(obj);
        ptr->~T();
        ::operator delete(base);
      }
      static void destroy_array(void* obj, void* base, size_t size)
      {
        T* arr = static_cast<T*>(obj);
        for(size_t i = 0; i < size/sizeof(T); ++i)
        {
          arr[i].~T();
        }
#if _MSC_VER <= 1200
        ::operator delete(base);
#else
        ::operator delete[](base);
#endif
      }
    };

    struct node_base {};

#define ASSERT_VALID if(0){}else
#define ASSERT_SUPER_VALID if(0){}else
#define ASSERT_PTR_VALID(_p_)  if(0){}else
#ifndef NDEBUG
# ifdef DEBUG_COMPILER
#  undef ASSERT_VALID
#  undef ASSERT_SUPER_VALID
#  undef ASSERT_PTR_VALID
#  define ASSERT_VALID assert(!destroyed)
#  define ASSERT_SUPER_VALID assert(!super::destroyed)
#  define ASSERT_PTR_VALID(_p_) assert(!(_p_).destroyed)
# endif
#endif

    struct pointer_base
    {
      int pos;
#ifndef NDEBUG
      bool destroyed;
# ifdef DEBUG_COMPILER
      char *trash;
      //void assert_valid()
      //{ ASSERT_VALID; }
# endif
#endif

    protected:
      pointer_base();
      pointer_base(const pointer_base& pb);
      ~pointer_base();

      void operator= (const pointer_base& pb) {}

      void reset_node(const void* obj, void (*destroy)(void*, void*, size_t));
      void swap(pointer_base &pb)
      { ASSERT_VALID; }
    };

    struct weak_base
    {
      int pos;
#ifndef NDEBUG
      bool destroyed;
# ifdef DEBUG_COMPILER
      char *trash;
# endif
#endif

    protected:
      weak_base();
      weak_base(const weak_base& pb);
      ~weak_base();

      void operator= (const weak_base& pb) {}
    };


    void reg(void *obj, size_t size, void (*destroy)(void*, void*, size_t));


    struct dynamic_cast_t {};
    struct static_cast_t {};
    struct const_cast_t {};
    struct light_cons_t {};
  }

  /* Pointer registation function, allocated memory
     must be freeable using supplied destroy function */
  template<typename T>
  T* reg(T *ptr, void (*destroy)(void*, void*, size_t))
  {
    gc::detail::reg(static_cast<void*>(ptr), sizeof(T), destroy);
    return ptr;
  }

  /* Array pointer registation function, allocated memory
     must be freeable using supplied destroy function */
  template<typename T>
  T* reg(T arr[], size_t len, void (*destroy)(void*, void*, size_t))
  {
    gc::detail::reg(static_cast<void*>(arr), sizeof(T)*len, destroy);
    return arr;
  }

  /* Implicit cast operation */
  template<typename T, typename U> T implicit_cast(U u) { return u; }

  /* Managed pointer class template */
  template<typename T>
  class const_gc_ptr: public detail::pointer_base
  {
#ifdef _SMGC_NO_FRIEND_TEMPLATES
  public:
#else
  protected:
    template<typename U> friend class const_gc_ptr;
    template<typename U> friend class gc_ptr;
    template<typename U> friend class const_wk_ptr;
    template<typename U> friend class wk_ptr;
    friend struct std::less<const_gc_ptr<T> >;
    friend struct std::less<gc_ptr<T> >;
#endif
    friend void collect();

    T* ptr;

  public:
    typedef T element_type;

  private:
    typedef detail::pointer_base super;

  protected:
    template <typename U>
    void reset_node(const U* obj)
    {
      super::reset_node(obj, detail::destructor<U>::destroy);
    }

    /*void reset_node(const T* obj)
    {
      super::reset_node(obj, detail::destructor<T>::destroy);
    }*/

    const_gc_ptr(const detail::light_cons_t&) {}

  public:
    const_gc_ptr(): ptr(0) {}

    const_gc_ptr(const detail::gc_null_t&): ptr(0) {}

    template<typename U>
    explicit const_gc_ptr(const U *p)//: ptr(0)
    {
      reset(p);
    }

    template<typename U>
    const_gc_ptr(const U *p, const detail::dynamic_cast_t*)//: ptr(0)
    {
      reset(dynamic_cast<T*>(p));
    }

    template<typename U>
    const_gc_ptr(const U *p, const detail::static_cast_t*)//: ptr(0)
    {
      reset(static_cast<T*>(p));
    }

    const_gc_ptr(const T *p)//: ptr(0)
    {
      reset(p);
    }

    template<typename U>
    const_gc_ptr(const const_gc_ptr<U> &p): super(p), ptr(implicit_cast<T*>(p.ptr)) {}

    template<typename U>
    const_gc_ptr(const const_gc_ptr<U> &p, const detail::dynamic_cast_t*):
      super(p), ptr(dynamic_cast<T*>(p.ptr)) {}

    template<typename U>
    const_gc_ptr(const const_gc_ptr<U> &p, const detail::static_cast_t*):
      super(p), ptr(static_cast<T*>(p.ptr)) {}

#ifndef NDEBUG
    ~const_gc_ptr()
    {
      ptr=0;
    }
#endif

    const_gc_ptr<T>& operator = (const detail::gc_null_t&)
    {
      ASSERT_VALID;
      ptr = 0;
      return *this;
    }

    template<typename U>
    const_gc_ptr<T>& operator = (const U *p)
    {
      ASSERT_VALID;
      reset(p);
      return *this;
    }

    template<typename U>
    const_gc_ptr<T>& operator = (const const_gc_ptr<U> &p)
    {
      ASSERT_VALID;
      ptr = p.ptr;
      return *this;
    }

    const T& operator * () const
    {
      ASSERT_VALID;
      return *ptr;
    }

    const T* operator -> () const
    {
      ASSERT_VALID;
      return ptr;
    }

    const T* get() const
    {
      ASSERT_VALID;
      return ptr;
    }

    bool operator ! () const
    {
      ASSERT_VALID;
      return ptr==0;
    }

    operator bool () const
    {
      ASSERT_VALID;
      return ptr!=0;
    }

    bool operator ==(const detail::gc_null_t&) const
    {
      ASSERT_VALID;
      return ptr == 0;
    }

    bool operator !=(const detail::gc_null_t&) const
    {
      ASSERT_VALID;
      return ptr != 0;
    }

    template<typename U>
    bool operator ==(const const_gc_ptr<U> &p) const
    {
      ASSERT_VALID;
      return ptr == p.ptr;
    }

    template<typename U>
    bool operator !=(const const_gc_ptr<U> &p) const
    {
      ASSERT_VALID;
      return ptr != p.ptr;
    }

    template<typename U>
    bool operator <=(const const_gc_ptr<U> &p) const
    {
      ASSERT_VALID;
      return ptr <= p.ptr;
    }

    template<typename U>
    bool operator >=(const const_gc_ptr<U> &p) const
    {
      ASSERT_VALID;
      return ptr >= p.ptr;
    }

    template<typename U>
    bool operator <(const const_gc_ptr<U> &p) const
    {
      ASSERT_VALID;
      return ptr < p.ptr;
    }

    template<typename U>
    bool operator >(const const_gc_ptr<U> &p) const
    {
      ASSERT_VALID;
      return ptr > p.ptr;
    }


    template<typename U>
    bool operator ==(const U *p) const
    {
      ASSERT_VALID;
      return ptr == p;
    }

    template<typename U>
    bool operator !=(const U *p) const
    {
      ASSERT_VALID;
      return ptr != p;
    }

#ifdef _SMGC_PTR_COMPARABLE
    template<typename U>
    bool operator <=(const U *p) const
    {
      ASSERT_VALID;
      return ptr <= p;
    }

    template<typename U>
    bool operator >=(const U *p) const
    {
      ASSERT_VALID;
      return ptr >= p;
    }

    template<typename U>
    bool operator <(const U *p) const
    {
      ASSERT_VALID;
      return ptr < p;
    }

    template<typename U>
    bool operator >(const U *p) const
    {
      ASSERT_VALID;
      return ptr > p;
    }
#endif

    void reset(const T* p=0)
    {
      ASSERT_VALID;
      reset_node(p);
      ptr = const_cast<T*>(p);
    }

    void swap(const_gc_ptr &p)
    {
      ASSERT_VALID;
      T* tmp = ptr;
      ptr = p.ptr;
      p.ptr = tmp;
      super::swap(p);
    }
  };


  /* Managed pointer class template */
  template<typename T>
  class gc_ptr: public const_gc_ptr<T>
  {
  public:
    typedef T element_type;

  private:
    typedef const_gc_ptr<T> super;

  public:
    gc_ptr() {}

    gc_ptr(const detail::gc_null_t&) {}

    template<typename U>
    explicit gc_ptr(U *p): super(detail::light_cons_t())
    {
      reset(p);
    }

    template<typename U>
    gc_ptr(U *p, const detail::dynamic_cast_t*): super(detail::light_cons_t())
    {
      reset(dynamic_cast<T*>(p));
    }

    template<typename U>
    gc_ptr(U *p, const detail::static_cast_t*): super(detail::light_cons_t())
    {
      reset(static_cast<T*>(p));
    }

    template<typename U>
    gc_ptr(const U *p, const detail::const_cast_t*): super(detail::light_cons_t())
    {
      reset(const_cast<T*>(p));
    }

    gc_ptr(T *p): super(detail::light_cons_t())
    {
      reset(p);
    }

    template<typename U>
    gc_ptr(const gc_ptr<U> &p): super(p) {}

    template<typename U>
    gc_ptr(const gc_ptr<U> &p, const detail::dynamic_cast_t*):
      super(p, static_cast<detail::dynamic_cast_t*>(0)) {}

    template<typename U>
    gc_ptr(const gc_ptr<U> &p, const detail::static_cast_t*):
      super(p, static_cast<detail::static_cast_t*>(0)) {}

    template<typename U>
    gc_ptr(const const_gc_ptr<U> &p, const detail::const_cast_t*):
      super(p) {}

#ifndef NDEBUG
    ~gc_ptr()
    {
      ASSERT_SUPER_VALID;
      super::ptr=0;
    }
#endif

    gc_ptr<T>& operator = (const detail::gc_null_t&)
    {
      ASSERT_SUPER_VALID;
      super::ptr = 0;
      return *this;
    }

    template<typename U>
    gc_ptr<T>& operator = (U *p)
    {
      ASSERT_SUPER_VALID;
      reset(p);
      return *this;
    }

    template<typename U>
    gc_ptr<T>& operator = (const gc_ptr<U> &p)
    {
      ASSERT_SUPER_VALID;
      super::ptr = const_cast<U*>(p.ptr);
      return *this;
    }

    T& operator * () const
    {
      ASSERT_SUPER_VALID;
      return *super::ptr;
    }

    T* operator -> () const
    {
      ASSERT_SUPER_VALID;
      return super::ptr;
    }

    T* get() const
    {
      ASSERT_SUPER_VALID;
      return super::ptr;
    }

    void reset(T* p=0)
    {
      ASSERT_SUPER_VALID;
      reset_node(p);
      super::ptr = p;
    }

    void swap(gc_ptr &p)
    {
      ASSERT_SUPER_VALID;
      T* tmp = super::ptr;
      super::ptr = p.ptr;
      p.ptr = tmp;
      super::swap(p);
    }
  };


  template<typename U>
  bool operator ==(const detail::gc_null_t &l, const const_gc_ptr<U> &p)
  {
    return p.operator ==(l);
  }

  template<typename U>
  bool operator !=(const detail::gc_null_t &l, const const_gc_ptr<U> &p)
  {
    return p.operator !=(l);
  }

  template<typename T, typename U>
  bool operator ==(const T *l, const const_gc_ptr<U> &p)
  {
    return p.operator ==(l);
  }

  template<typename T, typename U>
  bool operator !=(const T *l, const const_gc_ptr<U> &p)
  {
    return p.operator !=(l);
  }

#ifdef _SMGC_PTR_COMPARABLE
  template<typename T, typename U>
  bool operator <=(const T *l, const const_gc_ptr<U> &p)
  {
    return p.operator >=(l);
  }

  template<typename T, typename U>
  bool operator >=(const T *l, const const_gc_ptr<U> &p)
  {
    return p.operator <=(l);
  }

  template<typename T, typename U>
  bool operator <(const T *l, const const_gc_ptr<U> &p)
  {
    return p.operator >(l);
  }

  template<typename T, typename U>
  bool operator >(const T *l, const const_gc_ptr<U> &p)
  {
    return p.operator <(l);
  }
#endif


  /* Smartpointer cast operations */

  template<typename T, typename U>
  gc_ptr<T> dynamic_cast_gc_ptr(U *u)
  {
    return gc_ptr<T>(u, static_cast<detail::dynamic_cast_t*>(0));
  }

  template<typename T, typename U>
  const_gc_ptr<T> dynamic_cast_gc_ptr(const U *u)
  {
    return const_gc_ptr<T>(u, static_cast<detail::dynamic_cast_t*>(0));
  }

  template<typename T, typename U>
  gc_ptr<T> dynamic_cast_gc_ptr(const gc_ptr<U> &u)
  {
    return gc_ptr<T>(u, static_cast<detail::dynamic_cast_t*>(0));
  }

  template<typename T, typename U>
  const_gc_ptr<T> dynamic_cast_gc_ptr(const const_gc_ptr<U> &u)
  {
    return const_gc_ptr<T>(u, static_cast<detail::dynamic_cast_t*>(0));
  }

  template<typename T, typename U>
  gc_ptr<T> static_cast_gc_ptr(U *u)
  {
    return gc_ptr<T>(u, static_cast<detail::static_cast_t*>(0));
  }

  template<typename T, typename U>
  const_gc_ptr<T> static_cast_gc_ptr(const U *u)
  {
    return const_gc_ptr<T>(u, static_cast<detail::static_cast_t*>(0));
  }

  template<typename T, typename U>
  gc_ptr<T> static_cast_gc_ptr(const gc_ptr<U> &u)
  {
    return gc_ptr<T>(u, static_cast<detail::static_cast_t*>(0));
  }

  template<typename T, typename U>
  const_gc_ptr<T> static_cast_gc_ptr(const const_gc_ptr<U> &u)
  {
    return const_gc_ptr<T>(u, static_cast<detail::static_cast_t*>(0));
  }

  template<typename T, typename U>
  gc_ptr<T> const_cast_gc_ptr(const U *u)
  {
    return gc_ptr<T>(u, static_cast<detail::const_cast_t*>(0));
  }

  template<typename T, typename U>
  gc_ptr<T> const_cast_gc_ptr(const const_gc_ptr<U> &u)
  {
    return gc_ptr<T>(u, static_cast<detail::const_cast_t*>(0));
  }


  /* Managed const array class template */
  template<typename T>
  class const_gc_arr: public detail::pointer_base
  {
  protected:
    friend class gc_arr<T>;
    friend class const_wk_arr<T>;
    friend class wk_arr<T>;
#ifndef _SMGC_POOR_TEMPLATES
    friend struct std::less<const_gc_arr<T> >;
    friend struct std::less<gc_arr<T> >;
#endif
    T* ptr;

  public:
    typedef T element_type;

  private:
    typedef detail::pointer_base super;

  protected:
    void reset_node(const T* obj)
    {
      super::reset_node(obj, detail::destructor<T>::destroy_array);
    }

    const_gc_arr(const detail::light_cons_t&) {}

  public:
    const_gc_arr(): ptr(0)
    {}

    const_gc_arr(const detail::gc_null_t&): ptr(0)
    {}

    /*explicit*/ const_gc_arr(const T p[])//: ptr(0)
    {
      reset(p);
    }

#ifndef NDEBUG
    ~const_gc_arr()
    {
      ptr = 0;
    }
#endif


    const_gc_arr& operator = (const detail::gc_null_t&)
    {
      ptr = 0;
      return *this;
    }

    const_gc_arr& operator = (const T p[])
    {
      reset(p);
      return *this;
    }

    const T& operator [] (size_t i) const
    {
      return ptr[i];
    }

    const T* get() const
    {
      return ptr;
    }

    bool operator ! () const
    {
      return ptr == 0;
    }

    operator bool () const
    {
      return ptr != 0;
    }

    bool operator ==(const detail::gc_null_t&) const
    {
      return ptr == 0;
    }

    bool operator !=(const detail::gc_null_t&) const
    {
      return ptr != 0;
    }

    template<typename U>
    bool operator ==(const const_gc_arr<U> &p) const
    {
      return ptr == p.ptr;
    }

    template<typename U>
    bool operator !=(const const_gc_arr<U> &p) const
    {
      return ptr != p.ptr;
    }

    template<typename U>
    bool operator <=(const const_gc_arr<U> &p) const
    {
      return ptr <= p.ptr;
    }

    template<typename U>
    bool operator >=(const const_gc_arr<U> &p) const
    {
      return ptr >= p.ptr;
    }

    template<typename U>
    bool operator <(const const_gc_arr<U> &p) const
    {
      return ptr < p.ptr;
    }

    template<typename U>
    bool operator >(const const_gc_arr<U> &p) const
    {
      return ptr > p.ptr;
    }


    template<typename U>
    bool operator ==(const U *p) const
    {
      return ptr == p;
    }

    template<typename U>
    bool operator !=(const U *p) const
    {
      return ptr != p;
    }

    template<typename U>
    bool operator <=(const U *p) const
    {
      return ptr <= p;
    }

    template<typename U>
    bool operator >=(const U *p) const
    {
      return ptr >= p;
    }

    template<typename U>
    bool operator <(const U *p) const
    {
      return ptr < p;
    }

    template<typename U>
    bool operator >(const U *p) const
    {
      return ptr > p;
    }

    const_gc_arr& operator += (ptrdiff_t d)
    {
      ptr += d;
      return *this;
    }

    const_gc_arr& operator -= (ptrdiff_t d)
    {
      ptr -= d;
      return *this;
    }

    const_gc_arr& operator ++ ()
    {
      ++ptr;
      return *this;
    }

    const_gc_arr& operator -- ()
    {
      --ptr;
      return *this;
    }

    const_gc_arr operator ++ (int)
    {
      const_gc_arr<T> old(*this);
      ++ptr;
      return old;
    }

    const_gc_arr operator -- (int)
    {
      const_gc_arr<T> old(*this);
      --ptr;
      return old;
    }

    const_gc_arr operator + (ptrdiff_t d) const
    {
      const_gc_arr<T> res(*this);
      res += d;
      return res;
    }

    const_gc_arr operator - (ptrdiff_t d) const
    {
      const_gc_arr<T> res(*this);
      res -= d;
      return res;
    }

    void reset(const T* p=0)
    {
      reset_node(p);
      ptr = const_cast<T*>(p);
    }

    void swap(const_gc_arr &p)
    {
      T* tmp = ptr;
      ptr = p.ptr;
      p.ptr = tmp;
      super::swap(p);
    }
  };

  /* Managed array class template */
  template<typename T>
  class gc_arr: public const_gc_arr<T>
  {
  public:
    typedef T element_type;

  private:
    typedef const_gc_arr<T> super;

  public:
    gc_arr()
    {}

    gc_arr(const detail::gc_null_t&)
    {}

    /*explicit*/ gc_arr(T p[]): super(detail::light_cons_t())
    {
      reset(p);
    }

#ifndef NDEBUG
    ~gc_arr()
    {
      super::ptr = 0;
    }
#endif


    gc_arr& operator = (const detail::gc_null_t&)
    {
      super::ptr = 0;
      return *this;
    }

    gc_arr& operator = (T p[])
    {
      reset(p);
      return *this;
    }

    T& operator [] (size_t i) const
    {
      return super::ptr[i];
    }

    T* get() const
    {
      return super::ptr;
    }

    gc_arr& operator += (ptrdiff_t d)
    {
      super::ptr += d;
      return *this;
    }

    gc_arr& operator -= (ptrdiff_t d)
    {
      super::ptr -= d;
      return *this;
    }

    gc_arr& operator ++ ()
    {
      ++super::ptr;
      return *this;
    }

    gc_arr& operator -- ()
    {
      --super::ptr;
      return *this;
    }

    gc_arr operator ++ (int)
    {
      gc_arr<T> old(*this);
      ++super::ptr;
      return old;
    }

    gc_arr operator -- (int)
    {
      gc_arr<T> old(*this);
      --super::ptr;
      return old;
    }

    gc_arr operator + (ptrdiff_t d) const
    {
      gc_arr<T> res(*this);
      res += d;
      return res;
    }

    gc_arr operator - (ptrdiff_t d) const
    {
      gc_arr<T> res(*this);
      res -= d;
      return res;
    }

    void reset(T* p=0)
    {
      reset_node(p);
      super::ptr = /*const_cast<T*>*/(p);
    }

    void swap(gc_arr &p)
    {
      T* tmp = super::ptr;
      super::ptr = p.ptr;
      p.ptr = tmp;
      super::swap(p);
    }
  };

  template<typename U>
  bool operator ==(const detail::gc_null_t &l, const const_gc_arr<U> &p)
  {
    return p.operator ==(l);
  }

  template<typename U>
  bool operator !=(const detail::gc_null_t &l, const const_gc_arr<U> &p)
  {
    return p.operator !=(l);
  }

  template<typename T, typename U>
  bool operator ==(const T *l, const const_gc_arr<U> &p)
  {
    return p.operator ==(l);
  }

  template<typename T, typename U>
  bool operator !=(const T *l, const const_gc_arr<U> &p)
  {
    return p.operator !=(l);
  }

  template<typename T, typename U>
  bool operator <=(const T *l, const const_gc_arr<U> &p)
  {
    return p.operator >=(l);
  }

  template<typename T, typename U>
  bool operator >=(const T *l, const const_gc_arr<U> &p)
  {
    return p.operator <=(l);
  }

  template<typename T, typename U>
  bool operator <(const T *l, const const_gc_arr<U> &p)
  {
    return p.operator >(l);
  }

  template<typename T, typename U>
  bool operator >(const T *l, const const_gc_arr<U> &p)
  {
    return p.operator <(l);
  }

  template<typename T>
  const_gc_arr<T> operator +(ptrdiff_t l, const const_gc_arr<T> &p)
  {
    return p+l;
  }

  template<typename T>
  gc_arr<T> operator +(ptrdiff_t l, const gc_arr<T> &p)
  {
    return p+l;
  }


  /* Const weak pointer template */
  template<typename T>
  class const_wk_ptr: public detail::weak_base
  {
#ifdef _SMGC_NO_FRIEND_TEMPLATES
  public:
#else
  protected:
    template<typename U> friend class const_wk_ptr;
    template<typename U> friend class wk_ptr;
#endif
    friend void collect();

    T *ptr;

  public:
    const_wk_ptr(): ptr(0) {}

    const_wk_ptr(const detail::gc_null_t&): ptr(0) {}

    template<typename U>
    const_wk_ptr(const const_wk_ptr<U> &p): ptr(implicit_cast<T*>(p.ptr)) {}

    template<typename U>
    const_wk_ptr(const const_gc_ptr<U> &p): ptr(implicit_cast<T*>(p.ptr)) {}

    template<typename U>
    const_wk_ptr(const const_gc_ptr<U> &p, const detail::dynamic_cast_t*):
      ptr(dynamic_cast<T*>(p.ptr)) {}

    template<typename U>
    const_wk_ptr(const const_gc_ptr<U> &p, const detail::static_cast_t*):
      ptr(static_cast<T*>(p.ptr)) {}

#ifndef NDEBUG
    ~const_wk_ptr()
    {
      ptr=0;
    }
#endif

    const_wk_ptr<T>& operator = (const detail::gc_null_t&)
    {
      ptr = 0;
      return *this;
    }

    template<typename U>
    const_wk_ptr<T>& operator = (const const_gc_ptr<U> &p)
    {
      ptr = const_cast<U*>(p.ptr);
      return *this;
    }

    const_gc_ptr<T> get() const
    {
      return const_gc_ptr<T>(ptr);
    }
  };


  template<typename T>
  class wk_ptr: public const_wk_ptr<T>
  {
    typedef const_wk_ptr<T> super;

  public:
    wk_ptr(const detail::gc_null_t&) {}

    template<typename U>
    wk_ptr(const wk_ptr<U> &p): super(p) {}

    template<typename U>
    wk_ptr(const gc_ptr<U> &p): super(p) {}

    template<typename U>
    wk_ptr(const gc_ptr<U> &p, const detail::dynamic_cast_t*):
      super(p, static_cast<detail::dynamic_cast_t*>(0)) {}

    template<typename U>
    wk_ptr(const gc_ptr<U> &p, const detail::static_cast_t*):
      super(p, static_cast<detail::static_cast_t*>(0)) {}

    template<typename U>
    wk_ptr(const const_gc_ptr<U> &p, const detail::const_cast_t*):
      super(p) {}

    wk_ptr<T>& operator = (const detail::gc_null_t &nt)
    {
      return super::operator=(nt);
    }

    template<typename U>
    wk_ptr<T>& operator = (const gc_ptr<U> &p)
    {
      super::ptr = const_cast<U*>(p.ptr);
      return *this;
    }

    //using super::get;

    gc_ptr<T> get() const
    {
      return gc_ptr<T>(super::ptr);
    }
  };

  /* Const weak array template */
  template<typename T>
  class const_wk_arr: public detail::weak_base
  {
  protected:
    friend class wk_arr<T>;
    friend void collect();

    T *ptr;

  public:
    const_wk_arr(): ptr(0) {}

    const_wk_arr(const detail::gc_null_t&): ptr(0) {}

    const_wk_arr(const const_gc_arr<T> &p): ptr(p.ptr) {}

#ifndef NDEBUG
    ~const_wk_arr()
    {
      ptr=0;
    }
#endif

    const_wk_arr<T>& operator = (const detail::gc_null_t&)
    {
      ptr = 0;
      return *this;
    }

    const_wk_arr<T>& operator = (const const_gc_arr<T> &p)
    {
      ptr = const_cast<T*>(p.ptr);
      return *this;
    }

    const_gc_arr<T> get() const
    {
      return const_gc_arr<T>(ptr);
    }
  };


  template<typename T>
  class wk_arr: public const_wk_arr<T>
  {
    typedef const_wk_arr<T> super;

  public:
    wk_arr(const detail::gc_null_t&) {}

    template<typename U>
    wk_arr(const wk_arr<U> &p): super(p) {}

    template<typename U>
    wk_arr(const gc_arr<U> &p): super(p) {}

    template<typename U>
    wk_arr(const const_gc_arr<U> &p, const detail::const_cast_t*):
      super(p) {}

    wk_arr<T>& operator = (const detail::gc_null_t &nt)
    {
      return super::operator=(nt);
    }

    wk_arr<T>& operator = (const gc_arr<T> &p)
    {
      super::ptr = const_cast<T*>(p.ptr);
      return *this;
    }

    //using super::get;

    gc_arr<T> get() const
    {
      return gc_arr<T>(super::ptr);
    }
  };


  namespace
  {
    gc::detail::gc_null_t NIL;
  }
}


namespace std
{
#ifdef _SMGC_POOR_TEMPLATES
  struct less<gc::detail::pointer_base>
  {
    bool operator()(const gc::detail::pointer_base &l, const gc::detail::pointer_base &p)
    {
      static less<void**> cmp;
      ASSERT_PTR_VALID(l);
      ASSERT_PTR_VALID(p);
      return cmp((reinterpret_cast<const gc::const_gc_ptr<void*>&>(l)).ptr, (reinterpret_cast<const gc::const_gc_ptr<void*>&>(p)).ptr);
    }
  };
#else

  template<typename T>
  struct less<gc::const_gc_ptr<T> >
  {
    bool operator()(const gc::const_gc_ptr<T> &l, const gc::const_gc_ptr<T> &p)
    {
      static less<T> cmp;
      ASSERT_PTR_VALID(l);
      ASSERT_PTR_VALID(p);
      return cmp(l.ptr, p.ptr);
    }
  };

  template<typename T>
  struct less<gc::gc_ptr<T> >
  {
    bool operator()(const gc::gc_ptr<T> &l, const gc::gc_ptr<T> &p)
    {
      static less<T> cmp;
      ASSERT_PTR_VALID(l);
      ASSERT_PTR_VALID(p);
      return cmp(l.ptr, p.ptr);
    }
  };

  template<typename T>
  struct less<gc::const_gc_arr<T> >
  {
    bool operator()(const gc::const_gc_arr<T> &l, const gc::const_gc_arr<T> &p)
    {
      static less<T> cmp;
      ASSERT_PTR_VALID(l);
      ASSERT_PTR_VALID(p);
      return cmp(l.ptr, p.ptr);
    }
  };

  template<typename T>
  struct less<gc::gc_arr<T> >
  {
    bool operator()(const gc::gc_arr<T> &l, const gc::gc_arr<T> &p)
    {
      static less<T> cmp;
      ASSERT_PTR_VALID(l);
      ASSERT_PTR_VALID(p);
      return cmp(l.ptr, p.ptr);
    }
  };
#endif
}

namespace
{
  gc::detail::gc_t GC;
  gc::detail::gc_null_t GC_NIL;
}

#ifndef _SMGC_NO_PLACEMENT_NEW
void* operator new(size_t s, const gc::detail::gc_t&) throw (gc::bad_alloc);
void operator delete(void *obj, const gc::detail::gc_t&) throw ();
#  ifndef _SMGC_NO_ARRAY_NEW
void* operator new[](size_t s, const gc::detail::gc_t&) throw (gc::bad_alloc);
void operator delete[](void *obj, const gc::detail::gc_t&) throw ();
#  endif
#endif


#undef GC_INTERNAL

#endif
