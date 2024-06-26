#define DIMENSIONS    ( 128UL )


float l1Norm(local float const * const set1, local float const * const set2);


kernel void sad(global float const * const set1, global float const * const set2, global unsigned long * const indices, unsigned const numOfPoints)
{
    size_t const idx = get_global_id(0);

    local float vector[DIMENSIONS];

    float const * const vectorStart = set1 + idx * DIMENSIONS;

    #pragma unroll
    for (size_t idx = 0; idx < DIMENSIONS; ++idx)
    {
        vector[idx] = vectorStart[idx];
    }

    float minDistance = INFINITY;
    unsigned long minIndex = 0;

    local float currentVector[DIMENSIONS];

    #pragma unroll
    for (size_t vectorIdx = 0; vectorIdx < numOfPoints; ++vectorIdx)
    {
        float const * const currentVectorStart = set2 + vectorIdx * DIMENSIONS;

        #pragma unroll
        for (size_t idx = 0; idx < DIMENSIONS; ++idx)
        {
            currentVector[idx] = currentVectorStart[idx];
        }

        float const distance = l1Norm(vector, currentVector);

        if (distance < minDistance)
        {
            minDistance = distance;
            minIndex = vectorIdx;
        }
    }

    indices[idx] = minIndex;
}


float l1Norm(local float const * const set1, local float const * const set2)
{
    float sum = 0.0f;

    #pragma unroll
    for (size_t idx = 0; idx < DIMENSIONS; ++idx)
    {
        sum += fabs(set1[idx] - set2[idx]);
    }

    return sum;
}