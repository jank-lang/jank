#include <stddef.h>
#include <stdlib.h>

/* Provide minimal implementations to satisfy the linker.
 * These should ideally never be called if Folly is configured correctly via
 * preprocessor flags like FOLLY_HAVE_JEMALLOC=0. If they *are* called,
 * it indicates a configuration mismatch or unexpected code path in Folly.
 */

size_t nallocx(size_t size, int flags) {
    /* This function normally returns a usable size for a requested allocation.
     * Returning the requested size is a somewhat safe fallback, assuming
     * the caller can handle it not being a jemalloc-optimized size.
     */
    (void)flags;
    return size;
}

void* xallocx(void* ptr, size_t size, size_t extra, int flags) {
    /* This function is tricky. It's related to resizing allocations.
     * A true stub cannot replicate its behavior. Returning NULL is the
     * safest option if this function is unexpectedly called, as it signals
     * failure to resize/allocate without causing memory corruption.
     * Callers like shrink_to_fit might handle a NULL return gracefully,
     * or they might proceed assuming success, which could be problematic.
     */
    (void)ptr;
    (void)size;
    (void)extra;
    (void)flags;
    return NULL;
}

