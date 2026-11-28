#include "vex.h"
#include "nio/nio.h"

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: WebMemory (graphvex/web/nio/web_memory.c)
 * ============================================================================
 * Owns the single lazily initialized MemoryArena for graphvex/web drawn from
 * Relational Engine's MemoryPool_default(). Per-tab DOM trees live inside
 * JSHeap realm arenas; Web_arena() serves shared parser/table scratch state.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: WebMemory (nio/web_memory.c)
 * ============================================================================
 * Process-scoped arena accessor and shutdown hook for graphvex/web.
 *
 * --FUNCTIONS----------------------------
 * public   Web_arena()                    returns/initializes repo MemoryArena
 * public   Web_arenaShutdown()            destroys repo MemoryArena
 * ============================================================================
 */

static MemoryArena *s_webArena = nullptr;

/**
 * Returns the lazily initialized MemoryArena owned by graphvex/web.
 * @return non-null MemoryArena pointer, or nullptr on pool exhaustion
 */
MemoryArena *Web_arena()
{
    if(!s_webArena)
        s_webArena = MemoryArena();

    return s_webArena;
}

/**
 * Destroys the graphvex/web MemoryArena and clears the cached handle.
 */
fn Web_arenaShutdown()
{
    if(!s_webArena)
        return;

    MemoryArena_destroy(s_webArena);
    s_webArena = nullptr;
}
