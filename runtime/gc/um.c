
/*
hello.1.c:(.text+0xb8fa): undefined reference to `UM_Header_alloc'
hello.1.c:(.text+0xb912): undefined reference to `UM_Payload_alloc'
hello.1.c:(.text+0xb92d): undefined reference to `UM_CPointer_offset'
*/

#define DBG(x,y,z,m) fprintf (stderr, "%s:%d: %s(%x, %d, %d): %s\n", \
		__FILE__, __LINE__, __FUNCTION__, (x), (int)y, (int)z, m?m:"na")

Pointer
UM_Header_alloc(__attribute__ ((unused)) GC_state gc_stat, Pointer umfrontier,
                C_Size_t s)
{
   if (DEBUG_MEM)
        DBG(umfrontier, s, 0, "enter");
	return (umfrontier + s);
}

Pointer
UM_Payload_alloc(GC_state gc_stat, Pointer umfrontier, C_Size_t s)
{
	if (DEBUG_MEM)
       DBG(umfrontier, s, 0, "enter");

    GC_UM_Chunk next_chunk = allocNextChunk(gc_stat, &(gc_stat->umheap));
    GC_UM_Chunk current_chunk = (GC_UM_Chunk) umfrontier;

    if (DEBUG_MEM) {
        fprintf(stderr, "Sentinel: %d \n", current_chunk->sentinel);
        fprintf(stderr, "Nextchunk: %x \n", next_chunk);
    }
    current_chunk->next_chunk = NULL;

    if (s <= UM_CHUNK_PAYLOAD_SIZE) {
        if (DEBUG_MEM)
            DBG(umfrontier, s, 0, "move frontier to next chunk");
        return next_chunk->ml_object;
    }

    if (DEBUG_MEM)
       DBG(umfrontier, s, 0, "allocate next chunk");

    /* Assume a maximum of 2 chunks
     * It can actually be 3 (fragmented chunks of LARGE objects)
     * TODO: Set header to represent chunked object
     */
    current_chunk->next_chunk = next_chunk;
    GC_UM_Chunk next_chunk_next = allocNextChunk(gc_stat, &(gc_stat->umheap));
    next_chunk->next_chunk = NULL;
    return (Pointer) next_chunk_next->ml_object;
}

Pointer
UM_CPointer_offset(GC_state gc_stat, Pointer p, C_Size_t o, C_Size_t s)
{
	if (DEBUG_MEM)
       DBG(p, o, s, "enter");

    Pointer heap_end = (gc_stat->umheap).start + (gc_stat->umheap).size;

    /* Not on our heap! */
    if (p < (gc_stat->umheap).start ||
        p >= heap_end) {
        if (DEBUG_MEM)
            DBG(p, o, s, "not UM Heap");
        return (p + o);
    }

    /* On current chunk */
    if (o + s + 4 <= UM_CHUNK_PAYLOAD_SIZE) {
        if (DEBUG_MEM)
            DBG(p, o, s, "current chunk");
        return (p + o);
    }

    if (DEBUG_MEM)
       DBG(p, o, s, "next chunk");

    /* On next chunk */
    GC_UM_Chunk current_chunk = (GC_UM_Chunk) (p - 4);
    if (current_chunk->sentinel == UM_CHUNK_SENTINEL_UNUSED) {
        current_chunk->sentinel = o + 4;
        return (Pointer)(current_chunk->next_chunk->ml_object);
    }

    return (Pointer)(current_chunk->next_chunk + o - current_chunk->sentinel);
}
