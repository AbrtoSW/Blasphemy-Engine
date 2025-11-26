#ifdef _MSC_VER
// Disable Code Analysis (SAL) warnings for VMA
#pragma warning(push)
#pragma warning(disable: 6001)  // uninitialized memory (false positive)
#pragma warning(disable: 6011)  // null pointer dereference (false positive)
#pragma warning(disable: 6385)  // invalid read (false positive)
#pragma warning(disable: 6386)  // invalid write (false positive)
#pragma warning(disable: 28182) // mutex unlock false positive
#pragma warning(disable: 26495) // uninitialized member variable
#pragma warning(disable: 26812) // enum class warning
#pragma warning(disable: 28251) // function annotation false positive

#endif

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif
