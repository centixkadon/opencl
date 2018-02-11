
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include <vector>

#include <fstream>
#include <iostream>

#include "util/time_diff.h"

#include "util/stdUtil.hpp"

using namespace cl;
using namespace std;

#define cout_a_b_c(index) cout << "index = " << (index) << ": " << a[index] << " + " << b[index] << " = " << c[index] << endl

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
    for (uint32_t i = 0; i < n; ++i) { a[i] = 9; }
    for (uint32_t i = 0; i < n; ++i) { b[i] = 8; }
    for (uint32_t i = 0; i < n; ++i) { c[i] = 7; }

    //cout_a_b_c(0); cout_a_b_c(n-1);

    TIME_A(zero);
    TIME_B(zero);

    TIME_A(time);

    Context context(devices);

    //    Buffer bufferA(context, CL_MEM_READ_WRITE | CL_MEM_HOST_WRITE_ONLY | CL_MEM_USE_HOST_PTR, n * sizeof(int32_t), a);
    Buffer bufferA(context, CL_MEM_READ_WRITE | CL_MEM_HOST_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, n * sizeof(int32_t), nullptr);
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

    a = (int32_t *)commandQueue.enqueueMapBuffer(bufferA, true, CL_MAP_WRITE_INVALIDATE_REGION, 0, n * sizeof(int32_t));
    TIME_A(init);
    for (uint32_t i = 0; i < n; ++i) { a[i] = i; }
    for (uint32_t i = 0; i < n; ++i) { b[i] = 1; }
    for (uint32_t i = 0; i < n; ++i) { c[i] = 0; }
    TIME_B(init);

    commandQueue.enqueueNDRangeKernel(kernel, 0, n);

    for (uint32_t i = 0; i < n; ++i) { a[i] = i; }
    for (uint32_t i = 0; i < n; ++i) { b[i] = 0; }
    for (uint32_t i = 0; i < n; ++i) { c[i] = 0; }

    commandQueue.enqueueReadBuffer(bufferC, true, 0, n * sizeof(int32_t), c);

    cout_a_b_c(0);
    cout_a_b_c(n - 1);

    TIME_B(time);

  } catch (Error & e) {
    cerr << "clErr: " << e.what() << endl;
    cerr << "clErr: " << e.err() /*<< ", " << cl::clGetErrorName(e.err())*/ << endl;
  }
  return 0;
}
