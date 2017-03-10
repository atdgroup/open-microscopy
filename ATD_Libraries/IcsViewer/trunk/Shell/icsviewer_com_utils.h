#include <objbase.h>

#define VTABLE(ob, member) (*((ob)->lpVtbl->member))

#define IUNK_VTABLE_OF(x) ((IUnknownVtbl *)((x)->lpVtbl))

#define QueryInterface(pif, iid, pintf) \
    (IUNK_VTABLE_OF(pif)->QueryInterface((IUnknown *)(pif), iid, (void **)(pintf)))

#define AddRef(pif) \
    (IUNK_VTABLE_OF(pif)->AddRef((IUnknown *)(pif)))

#define Release(pif) \
	(IUNK_VTABLE_OF(pif)->Release((IUnknown *)(pif)))

#ifndef offsetof
#ifndef offsetof
#define offsetof(t, mem) ((size_t) ((char *)&(((t *)8)->mem) - (char *)8)) 
#endif
#endif

#define IMPL(class, member, pointer) \
    (&((class *)8)->member == pointer, ((class *) (((long) pointer) - offsetof(class, member))))

extern HRESULT Alloc (size_t, void **ppv);
extern HRESULT Free (void *pv);

#define QITYPE HRESULT (*)(void *, REFIID, void **)
#define ARTYPE ULONG (*)(void *)
#define RLTYPE ULONG (*)(void *)

extern int vcObjects;
