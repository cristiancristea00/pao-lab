#define TILE_SIZE    ( 32 )


void kernel multiply(
    global float const * const a, 
    global float const * const b, 
    global float * const c,
    unsigned const m,
    unsigned const n,
    unsigned const k
)
{
    size_t const row = get_local_id(0);
    size_t const col = get_local_id(1);

    size_t const globalRow = get_group_id(0) * TILE_SIZE + row;
    size_t const globalCol = get_group_id(1) * TILE_SIZE + col;

    local float aTile[TILE_SIZE][TILE_SIZE];
    local float bTile[TILE_SIZE][TILE_SIZE];

    size_t const numTiles = k / TILE_SIZE;

    float sum = 0.0F;

    for (size_t idx = 0; idx < numTiles; ++idx)
    {
        size_t const tiledRow = idx * TILE_SIZE + row;
        size_t const tiledCol = idx * TILE_SIZE + col;

        aTile[col][row] = a[tiledCol * m + globalRow];
        bTile[col][row] = b[globalCol * k + tiledRow];

        barrier(CLK_LOCAL_MEM_FENCE);

        for (size_t idxTile = 0; idxTile < TILE_SIZE; ++idxTile)
        {
            sum += aTile[idxTile][row] * bTile[col][idxTile];
        }

        barrier(CLK_LOCAL_MEM_FENCE);
    }

    c[globalCol * m + globalRow] = sum;
}