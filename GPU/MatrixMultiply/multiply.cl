void kernel multiply(
    global float const * const a, 
    global float const * const b, 
    global float * const c,
    unsigned const m,
    unsigned const n,
    unsigned const k
)
{
    size_t const idxRow = get_global_id(0);
    size_t const idxCol = get_global_id(1);

    float sum = 0.0F;

    for (size_t idx = 0; idx < k; ++idx)
    {
        sum += a[idx * m + idxRow] * b[idxCol * k + idx];
    }

    c[idxCol * m + idxRow] = sum;
}