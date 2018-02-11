
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include <algorithm>
#include <vector>

#include <fstream>
#include <iostream>

#include "util/time_diff.h"

#include "util/stdUtil.hpp"

using namespace cl;
using namespace std;

template <typename _Ty, typename _Fn>
void clArrayTest(_Ty * a, _Ty * b, _Ty * c, uint32_t const n, _Fn & f) {
  for (uint32_t i = 0; i < n; ++i) {
    if (f(a[i], b[i], c[i], i)) {
      cout << "clTestError: (i, a, b, c) = (" << i << ", " << a[i] << ", " << b[i] << ", " << c[i] << ")" endl;
      break;
    }
  }
}

template <typename _Ty, typename _Fn>
void clArrayInit(_Ty * a, _Ty * b, _Ty * c, uint32_t const n, _Fn & f) {
  for (uint32_t i = 0; i < n; ++i) {
    auto v = f(i);
    while (v.size() < 3) v.push_back(v[v.size() - 1]);
    a[i] = v[0];
    b[i] = v[1];
    c[i] = v[2];
  }
  clArrayTest(a, b, c, n, [&f](int32_t a, int32_t b, int32_t c, int32_t i) {
    auto v = f(i);
    while (v.size() < 3) v.push_back(v[v.size() - 1]);
    return (a != v[0]) || (b != v[1]) || (c != v[2]);
  });
}

int main() {
  try {
    cout << "clMain: Hello OpenCL!" << endl;

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

    //uint32_t const n = 1000001;
    uint32_t const n = 100000001;
    int32_t * a = new int32_t[n];
    int32_t * b = new int32_t[n];
    int32_t * c = new int32_t[n];
    int32_t val = -2;

    TIME_A(zero);
    TIME_B(zero);

    Context context(devices);

    vector<pair<cl_mem_flags, int32_t *>> bufferArgses = {
        {CL_MEM_USE_HOST_PTR, a},
        {CL_MEM_ALLOC_HOST_PTR, a},
        {CL_MEM_ALLOC_HOST_PTR, nullptr},
        {CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR, a},
        {CL_MEM_COPY_HOST_PTR, a},
    };

    for (auto & bufferArgs : bufferArgses) {
      cout << "clTest     : ========== NEW ==========" << endl;
      TIME_A(time);

      clArrayInit(a, b, c, n, [](int32_t i) { return vector<int32_t>{-1}; });
      cout << "clTest     : array init (-1, -1, -1)" << endl;

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

      int32_t * a_ = (int32_t *)commandQueue.enqueueMapBuffer(bufferA, true, CL_MAP_WRITE_INVALIDATE_REGION, 0, n * sizeof(int32_t));
      if (a_ == a)
        cout << "clTestMap  : a_ eq a" << endl;
      else
        cout << "clTestMap  : a_ not_eq a" << endl;

      TIME_A(init);
      clArrayInit(a, b, c, n, [](int32_t i) { return vector<int32_t>{i, 1, 0}; });
      cout << "clTest     : array init (i, 1, 0)" << endl;
      TIME_B(init);

      commandQueue.enqueueNDRangeKernel(kernel, 0, n);

      clArrayInit(a, b, c, n, [](int32_t i) { return vector<int32_t>{i, 2, 0}; });
      cout << "clTest     : array init (i, 2, 0)" << endl;

      commandQueue.enqueueReadBuffer(bufferC, true, 0, n * sizeof(int32_t), c);

      clArrayTest(a, b, c, n, [](int32_t a, int32_t b, int32_t c, int32_t i) { return (a + b == c); });
      cout << "clTest     : result" << endl;

      TIME_B(time);
    }

  } catch (Error & e) {
    cerr << "clErr: " << e.what() << endl;
    cerr << "clErr: " << e.err() /*<< ", " << cl::clGetErrorName(e.err())*/ << endl;
  }
  return 0;
}
