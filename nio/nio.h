#ifndef GRAPHVEX_WEB_NIO_H
#define GRAPHVEX_WEB_NIO_H

#include "vex.h"
#include "re/nio/mem.h"

/**
 * Returns the lazily initialized MemoryArena owned by graphvex/web.
 * @return non-null MemoryArena pointer
 */
MemoryArena *Web_arena();

/**
 * Destroys the graphvex/web MemoryArena and returns all pages to MemoryPool.
 */
fn Web_arenaShutdown();

#endif
