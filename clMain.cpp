
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include <algorithm>
#include <vector>

#include <fstream>
#include <iostream>

#define _TIME_DIFF_STRLEN 11
#include "util/time_diff.h"

#include "util/stdUtil.hpp"

using namespace cl;
using namespace std;

template <typename _Ty, typename _Fn>
void clArrayTest(_Ty * a, _Ty * b, _Ty * c, uint32_t n, _Fn f) {
  for (uint32_t i = 0; i < n; ++i) {
    if (f(a[i], b[i], c[i], i)) {
      cerr << "\e[1;31mclTestError: (i, a, b, c) = (" << i << ", " << a[i] << ", " << b[i] << ", " << c[i] << ")\e[0m" << endl;
      break;
    }
  }
}

int main() {
  cout << "clMain: Hello OpenCL!" << endl;

  try {
    vector<Device> devices;
    vector<Platform> platforms;
    Platform::get(&platforms);
    cout << "clPlatformsSize: " << platforms.size() << endl;
    for (auto & platform : platforms) {
      cout << "clPlatformVendor     : " << platform.getInfo<CL_PLATFORM_VENDOR>() << endl;
      cout << "clPlatformName       : " << platform.getInfo<CL_PLATFORM_NAME>() << endl;
      cout << "clPlatformVersion    : " << platform.getInfo<CL_PLATFORM_VERSION>() << endl;
      cout << "clPlatformProfile    : " << platform.getInfo<CL_PLATFORM_PROFILE>() << endl;
      cout << "clPlatformExtensions : " << platform.getInfo<CL_PLATFORM_EXTENSIONS>() << endl;

      vector<Device> platformDevices;
      platform.getDevices(CL_DEVICE_TYPE_GPU, &platformDevices);
      cout << "clPlatformDevicesSize: " << platformDevices.size() << endl;
      for (auto & platformDevice : platformDevices) {
        devices.push_back(platformDevice);
      }
    }
    cout << endl;

    cout << "clDevicesSize: " << devices.size() << endl;
    for (auto & device : devices) {
      cout << "clDeviceName            : " << device.getInfo<CL_DEVICE_NAME>() << endl;
      cout << "clDeviceMaxWorkItemSizes: " << device.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>() << endl;
      cout << "clDeviceMaxWorkGroupSize: " << device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << endl;
    }

    uint32_t const n = 1000001;
    // uint32_t const n = 100000001;
    int32_t * a = new int32_t[n];
    int32_t * b = new int32_t[n];
    int32_t * c = new int32_t[n];

    TIME_A(clTimeZero);
    TIME_B(clTimeZero);

    Context context(devices);

    vector<pair<uint64_t, int32_t *>> bufferArgses = {
        {CL_MEM_USE_HOST_PTR, a},
        {CL_MEM_ALLOC_HOST_PTR, a},
        {CL_MEM_ALLOC_HOST_PTR, nullptr},
        {CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR, a},
        {CL_MEM_COPY_HOST_PTR, a},
    };

    for (auto & bufferArgs : bufferArgses) {
      cout << "clTest     : ========== NEW ==========" << endl;
      TIME_A(clTimeAll);

      for (uint32_t i = 0; i < n; ++i) { b[i] = i, c[i] = 0; }
      TIME_A(clTimeInit1);
      for (uint32_t i = 0; i < n; ++i) { a[i] = 1; }
      cout << "clTest     : array init (a 1)" << endl;
      TIME_B(clTimeInit1);

      Buffer bufferA(context, CL_MEM_READ_WRITE | CL_MEM_HOST_WRITE_ONLY | bufferArgs.first, n * sizeof(int32_t), bufferArgs.second);
      Buffer bufferB(context, CL_MEM_READ_WRITE | CL_MEM_HOST_WRITE_ONLY | CL_MEM_USE_HOST_PTR, n * sizeof(int32_t), b);
      Buffer bufferC(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, n * sizeof(int32_t), nullptr);

      string sources;
      vector<string> sourceFileNames = {"clMain.cl"};
      for (auto & sourceFileName : sourceFileNames) {
        ifstream fs(sourceFileName, ios::binary);
        sources += string(istreambuf_iterator<char>(fs), istreambuf_iterator<char>());
      }

      Program program(context, sources);
      program.build(devices);

      Kernel kernel(program, "clMain");
      kernel.setArg(0, bufferC);
      kernel.setArg(1, bufferA);
      kernel.setArg(2, bufferB);
      kernel.setArg(3, n);

      CommandQueue commandQueue(context, devices[0]);

      // int32_t * a_ = (int32_t *)commandQueue.enqueueMapBuffer(bufferA, true, CL_MAP_WRITE_INVALIDATE_REGION, 0, n * sizeof(int32_t));
      int32_t * a_ = (int32_t *)commandQueue.enqueueMapBuffer(bufferA, true, CL_MAP_WRITE, 0, n * sizeof(int32_t));
      if (a_ == nullptr)
        cerr << "\e[1;31mclTestError: a_ NULL\e[0m" << endl;
      if (a_ == a)
        cout << "clTestMap  : a_ eq a" << endl;
      else {
        cout << "clTestMap  : a_(" << (uint64_t)a_ << ") not_eq a(" << (uint64_t)a << ")" << endl;

        TIME_A(clTimeInit2);
        for (uint32_t i = 0; i < n; ++i) { a_[i] = 2; }
        cout << "clTest     : array init (a_ 2)" << endl;
        TIME_B(clTimeInit2);
      }

      commandQueue.enqueueNDRangeKernel(kernel, 0, n);

      // for (uint32_t i = 0; i < n; ++i) { a[i] = b[i] = c[i] = -2; }
      // cout << "clTest     : array init (-2, -2, -2)" << endl;

      commandQueue.enqueueReadBuffer(bufferC, true, 0, n * sizeof(int32_t), c);

      bool t = true, t_ = true;
      for (uint32_t i = 0; i < n; ++i) {
        t &= (a[i] + b[i] == c[i]);
        t_ &= (a_[i] + b[i] == c[i]);
      }
      if (t) {
        cout << "clTest     : result match b+a" << endl;
      } else if (t_) {
        cout << "clTest     : result match b+a_" << endl;
      } else {
        cerr << "\e[1;31mclTestError: result not match\e[0m" << endl;
        uint32_t i = 100;
        cerr << "\e[1;31mclTestError: i(a/a_, b, c) = " << i << "(" << a[i] << "/" << a_[i] << ", " << b[i] << ", " << c[i] << ")\e[0m" << endl;
      }
      // clArrayTest(a, b, c, n, [](int32_t a, int32_t b, int32_t c, int32_t i) { return (a + b == c); });

      TIME_B(clTimeAll);
    }

  } catch (Error & e) {
    cerr << "clErr: " << e.what() << endl;
    cerr << "clErr: " << e.err() /*<< ", " << cl::clGetErrorName(e.err())*/ << endl;
  }

  return 0;
}
