#define WIDTH                 ( 16 )
#define TILE_SIZE_M           ( 128 )
#define TILE_SIZE_N           ( 128 )
#define TILE_SIZE_K           ( 32 )
#define WORK_THREAD_M         ( 8 )
#define WORK_THREAD_N         ( 8 )
#define RED_TILE_SIZE_M       ( TILE_SIZE_M / WORK_THREAD_M )
#define RED_TILE_SIZE_N       ( TILE_SIZE_N / WORK_THREAD_N )
#define LOADS_PER_THREAD_A    ( ( TILE_SIZE_K * TILE_SIZE_M ) / ( RED_TILE_SIZE_M * RED_TILE_SIZE_N ) )
#define LOADS_PER_THREAD_B    ( ( TILE_SIZE_K * TILE_SIZE_N ) / ( RED_TILE_SIZE_M * RED_TILE_SIZE_N ) )


#if WIDTH == 1
    typedef float float_t;
#elif WIDTH == 2
    typedef float2 float_t;
#elif WIDTH == 4
    typedef float4 float_t;
#elif WIDTH == 8
    typedef float8 float_t;
#elif WIDTH == 16
    typedef float16 float_t;
#endif


void kernel multiply(
    global float const * const a, 
    global float const * const b, 
    global float * const c,
    unsigned const m,
    unsigned const n,
    unsigned const k
)
{
    size_t const offsetM = get_group_id(0) * TILE_SIZE_M;
    size_t const offsetN = get_group_id(1) * TILE_SIZE_N;

    size_t const tileIdM = get_local_id(0);
    size_t const tileIdN = get_local_id(1);

    local float aTile[TILE_SIZE_K][TILE_SIZE_M];
    local float bTile[TILE_SIZE_K][TILE_SIZE_N];

    float aRegister;
    float bRegister[WORK_THREAD_N];
    float accumulator[WORK_THREAD_M][WORK_THREAD_N];

    #pragma unroll
    for (size_t i = 0; i < WORK_THREAD_M; ++i)
    {
        #pragma unroll
        for (size_t j = 0; j < WORK_THREAD_N; ++j)
        {
            accumulator[i][j] = 0.0F;
        }
    }

    size_t const numTiles = k / TILE_SIZE_K;

    #pragma unroll
    for (size_t tile = 0; tile < numTiles; ++tile)
    {
        #pragma unroll
        for (size_t loadA = 0; loadA < LOADS_PER_THREAD_A / WIDTH; ++loadA)
        {
            size_t const tileId = tileIdN * RED_TILE_SIZE_M + tileIdM;
            size_t const id = loadA * RED_TILE_SIZE_N * RED_TILE_SIZE_M + tileId;

            size_t const row = id % (TILE_SIZE_M / WIDTH);
            size_t const col = id / (TILE_SIZE_M / WIDTH);

            size_t const tiledIndex = TILE_SIZE_K * tile + col;

            float_t const vectorA = a[tiledIndex * ( m / WIDTH ) + ( offsetM / WIDTH) + row];
            float_t const vectorB = b[tiledIndex * ( n / WIDTH ) + ( offsetN / WIDTH) + row];

            #if WIDTH == 1
                aTile[col][row] = vectorA;

                bTile[col][row] = vectorB;
            #elif WIDTH == 2
                aTile[col][WIDTH * row + 0] = vectorA.s0;
                aTile[col][WIDTH * row + 1] = vectorA.s1;

                bTile[col][WIDTH * row + 0] = vectorB.s0;
                bTile[col][WIDTH * row + 1] = vectorB.s1;
            #elif WIDTH == 4
                aTile[col][WIDTH * row + 0] = vectorA.s0;
                aTile[col][WIDTH * row + 1] = vectorA.s1;
                aTile[col][WIDTH * row + 2] = vectorA.s2;
                aTile[col][WIDTH * row + 3] = vectorA.s3;

                bTile[col][WIDTH * row + 0] = vectorB.s0;
                bTile[col][WIDTH * row + 1] = vectorB.s1;
                bTile[col][WIDTH * row + 2] = vectorB.s2;
                bTile[col][WIDTH * row + 3] = vectorB.s3;
            #elif WIDTH == 8
                aTile[col][WIDTH * row + 0] = vectorA.s0;
                aTile[col][WIDTH * row + 1] = vectorA.s1;
                aTile[col][WIDTH * row + 2] = vectorA.s2;
                aTile[col][WIDTH * row + 3] = vectorA.s3;
                aTile[col][WIDTH * row + 4] = vectorA.s4;
                aTile[col][WIDTH * row + 5] = vectorA.s5;
                aTile[col][WIDTH * row + 6] = vectorA.s6;
                aTile[col][WIDTH * row + 7] = vectorA.s7;

                bTile[col][WIDTH * row + 0] = vectorB.s0;
                bTile[col][WIDTH * row + 1] = vectorB.s1;
                bTile[col][WIDTH * row + 2] = vectorB.s2;
                bTile[col][WIDTH * row + 3] = vectorB.s3;
                bTile[col][WIDTH * row + 4] = vectorB.s4;
                bTile[col][WIDTH * row + 5] = vectorB.s5;
                bTile[col][WIDTH * row + 6] = vectorB.s6;
                bTile[col][WIDTH * row + 7] = vectorB.s7;
            #elif WIDTH == 16
                aTile[col][WIDTH * row + 0] = vectorA.s0;
                aTile[col][WIDTH * row + 1] = vectorA.s1;
                aTile[col][WIDTH * row + 2] = vectorA.s2;
                aTile[col][WIDTH * row + 3] = vectorA.s3;
                aTile[col][WIDTH * row + 4] = vectorA.s4;
                aTile[col][WIDTH * row + 5] = vectorA.s5;
                aTile[col][WIDTH * row + 6] = vectorA.s6;
                aTile[col][WIDTH * row + 7] = vectorA.s7;
                aTile[col][WIDTH * row + 8] = vectorA.s8;
                aTile[col][WIDTH * row + 9] = vectorA.s9;
                aTile[col][WIDTH * row + 10] = vectorA.sA;
                aTile[col][WIDTH * row + 11] = vectorA.sB;
                aTile[col][WIDTH * row + 12] = vectorA.sC;
                aTile[col][WIDTH * row + 13] = vectorA.sD;
                aTile[col][WIDTH * row + 14] = vectorA.sE;
                aTile[col][WIDTH * row + 15] = vectorA.sF;

                bTile[col][WIDTH * row + 0] = vectorB.s0;
                bTile[col][WIDTH * row + 1] = vectorB.s1;
                bTile[col][WIDTH * row + 2] = vectorB.s2;
                bTile[col][WIDTH * row + 3] = vectorB.s3;
                bTile[col][WIDTH * row + 4] = vectorB.s4;
                bTile[col][WIDTH * row + 5] = vectorB.s5;
                bTile[col][WIDTH * row + 6] = vectorB.s6;
                bTile[col][WIDTH * row + 7] = vectorB.s7;
                bTile[col][WIDTH * row + 8] = vectorB.s8;
                bTile[col][WIDTH * row + 9] = vectorB.s9;
                bTile[col][WIDTH * row + 10] = vectorB.sA;
                bTile[col][WIDTH * row + 11] = vectorB.sB;
                bTile[col][WIDTH * row + 12] = vectorB.sC;
                bTile[col][WIDTH * row + 13] = vectorB.sD;
                bTile[col][WIDTH * row + 14] = vectorB.sE;
                bTile[col][WIDTH * row + 15] = vectorB.sF;
            #endif
        }

        barrier(CLK_LOCAL_MEM_FENCE);

        #pragma unroll
        for (size_t k = 0; k < TILE_SIZE_K; ++k)
        {
            #pragma unroll
            for (size_t workN = 0; workN < WORK_THREAD_N; ++workN)
            {
                size_t const col = tileIdN + workN * RED_TILE_SIZE_N;
                bRegister[workN] = bTile[k][col];
            }

            #pragma unroll
            for (size_t workM = 0; workM < WORK_THREAD_M; ++workM)
            {
                size_t const row = tileIdM + workM * RED_TILE_SIZE_M;
                aRegister = aTile[k][row];

                for (size_t workN = 0; workN < WORK_THREAD_N; ++workN)
                {
                    accumulator[workM][workN] += aRegister * bRegister[workN];
                }
            }
        }

        barrier(CLK_LOCAL_MEM_FENCE);

        #pragma unroll
        for (size_t workM = 0; workM < WORK_THREAD_M; ++workM)
        {
            size_t const globalRow = offsetM + tileIdM + workM * RED_TILE_SIZE_M;

            #pragma unroll
            for (size_t workN = 0; workN < WORK_THREAD_N; ++workN)
            {
                size_t const globalCol = offsetN + tileIdN + workN * RED_TILE_SIZE_N;

                c[globalCol * m + globalRow] = accumulator[workM][workN];
            }
        }
    }
}