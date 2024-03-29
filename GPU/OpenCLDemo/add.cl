void kernel add(global float const * const a, global float const * const b, global float * const c)
{
    size_t const index = get_global_id(0);
    c[index] = a[index] + b[index];
}