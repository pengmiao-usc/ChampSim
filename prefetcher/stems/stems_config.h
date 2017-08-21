/*
 * You can configure parameters for the STeMS prefetcher through this header.
 * Documentation relevant to each parameter is given above its corresponding
 * #define.
 *
 * These parameters can be overriden at compile time by defining their values
 * through the '-D' flag for the compiler/preprocessor.
 *
 * Implementation author: Arka Majumdar
 */

#ifndef STEMS_RECONSTRUCTION_RESUME_THRESHOLD
/*
 * Number of entries that a stream queue must go under to resume its
 * reconstruction process.
 *
 * Default: 4
 * This default is a complete guess because the original Spatio-Temporal Memory
 * Streaming paper mentions this threshold in section 4.2.Streaming (page 5):
 *     "When the number of available
 *     prefetch addresses in a queue drops below a threshold, STeMS
 *     resumes reconstruction from where it left off previously, adding
 *     more addresses to the end of the stream queue."
 * ...but never gives a value for this threshold.
 */
#define STEMS_RECONSTRUCTION_RESUME_THRESHOLD 4
#endif

#ifndef STEMS_RECONSTRUCTION_BUFFER_SIZE
/*
 * The number of entries should the reconstruction buffer should be able to
 * hold.
 *
 * Default: 256
 */
#define STEMS_RECONSTRUCTION_BUFFER_SIZE 256
#endif

#ifndef STEMS_STREAMING_ENGINE_NUM_STREAM_QUEUES
/*
 * Maximum number of streams maintained at any one time by the streaming engine.
 *
 * Default: 8
 * This default is specified by the original Spatio-Temporal Memory Streaming
 * paper in section 4.3.Streaming (page 6):
 *     "Even though only one stream is typically productive at
 *     any time, several stream queues are necessary to prevent thrashing
 *     when new streams are initiated on misses. STeMS orders the
 *     streams based on activity (i.e., prefetches and hits) and chooses the
 *     LRU stream as a victim when a new stream must be allocated. We
 *     use eight stream queues in our evaluation."
 */
#define STEMS_STREAMING_ENGINE_NUM_STREAM_QUEUES 1
#endif

#ifndef STEMS_SVB_SIZE
/*
 * Size of the Streamed Value Buffer in number of entries.
 *
 * Default: 64
 */
#define STEMS_SVB_SIZE 64
#endif

#ifndef STEMS_SVB_STREAM_LOOKAHEAD
/*
 * Number of number of blocks the prefetcher tried to keep in the SVB for each
 * active stream.
 *
 * Default: 8
 */
#define STEMS_SVB_STREAM_LOOKAHEAD 8
#endif

#ifndef STEMS_SPATIAL_REGION_SIZE
/*
 * Size of each SMS spatial region in cache blocks. Must be a power of 2.
 *
 * Default: 32 (4KB)
 */
#define STEMS_SPATIAL_REGION_SIZE 32
#endif

#ifndef STEMS_AGT_SIZE
/*
 * Size of the Active Generation Table (used for spatial prediction) in number
 * of entries).
 *
 * Default: 64
 */
#define STEMS_AGT_SIZE 64
#endif

#ifndef STEMS_PST_SIZE
/*
 * Size of the Pattern Sequence Table (used for spatial prediction) in number of
 * entries.
 *
 * Default: 16000
 */
#define STEMS_PST_SIZE 16000
#endif

#ifndef STEMS_RMOB_SIZE
/*
 * Size of the Region Miss Order Buffer (basically a GHB used by STeMS for
 * temporal prediction) in number of entries.
 *
 * Default: 128000
 */
#define STEMS_RMOB_SIZE 128000
#endif
