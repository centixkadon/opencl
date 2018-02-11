
__kernel void clMain(__global int * c, __global int * a, __global int * b, uint n) {
  uint id = get_global_id(0);
  if (n <= id) return;

  c[id] = a[id] + b[id];
#if 0
  for (int i = 1; i < 200000; ++i) {
    c[id] += b[id];
  }
#endif
}

