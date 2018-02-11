
#ifndef TIME_DIFF_H_
#define TIME_DIFF_H_

#if 1

#if (defined(WIN64) || defined(_WIN64) || defined(__WIN64__)) || (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#include <windows.h>
#define TIME_A(str)          \
  LARGE_INTEGER t_##str##_a; \
  QueryPerformanceCounter(&t_##str##_a);
#define TIME_B(str)                                                                      \
  LARGE_INTEGER t_##str##_b;                                                             \
  QueryPerformanceCounter(&t_##str##_b);                                                 \
  string t_##str##_s = #str;                                                             \
  double t_##str;                                                                        \
  {                                                                                      \
    LARGE_INTEGER t_##str##_c;                                                           \
    QueryPerformanceFrequency(&t_##str##_c);                                             \
    t_##str = 1.0 * (t_##str##_b.QuadPart - t_##str##_a.QuadPart) / t_##str##_c.QuadPart \
  }                                                                                      \
  cout << #str << string(20 - t_##str##_s.size(), ' ') << " : " << t_##str << endl;
#elif defined(__linux__) || defined(__linux)
#define TIME_A(str)            \
  struct timespec t_##str##_a; \
  clock_gettime(CLOCK_MONOTONIC, &t_##str##_a);
#define TIME_B(str)                                                           \
  struct timespec t_##str##_b;                                                \
  clock_gettime(CLOCK_MONOTONIC, &t_##str##_b);                               \
  string t_##str##_s = #str;                                                  \
  double t_##str = t_##str##_b.tv_sec - t_##str##_a.tv_sec +                  \
                   0.000000001 * (t_##str##_b.tv_nsec - t_##str##_a.tv_nsec); \
  cout << #str << string(20 - t_##str##_s.size(), ' ') << " : " << t_##str << endl;
#else
#define TIME_A(str)
#define TIME_B(str) cout << "get time : system error." << endl;
#endif
#else
#define TIME_A(str)
#define TIME_B(str)
#endif

#endif
