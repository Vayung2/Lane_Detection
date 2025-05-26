#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120

#include <CL/cl2.hpp>
#include <fstream>
#include <iostream>
#include <vector>

#define IMG_SIZE  (1280 * 720 * 3)
#define GRAY_SIZE (1280 * 720)

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <xclbin_file>\n";
        return 1;
    }
    std::string binaryFile = argv[1];

    // Read the .xclbin file into a buffer
    std::ifstream bin_file(binaryFile, std::ios::binary | std::ios::ate);
    if (!bin_file) {
        std::cerr << "Error: Could not open " << binaryFile << "\n";
        return 1;
    }
    size_t nb = bin_file.tellg();
    bin_file.seekg(0, std::ios::beg);
    std::vector<unsigned char> binary(nb);
    bin_file.read(reinterpret_cast<char*>(binary.data()), nb);
    bin_file.close();

    // Discover and initialize the first Xilinx FPGA device
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    if (platforms.empty()) {
        std::cerr << "Error: No OpenCL platforms found.\n";
        return 1;
    }
    cl::Platform platform = platforms[0];

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);
    if (devices.empty()) {
        std::cerr << "Error: No FPGA devices found.\n";
        return 1;
    }
    cl::Device device = devices[0];

    cl::Context context(device);
    // Correctly build the Binaries vector
    cl::Program::Binaries bins;
    bins.push_back(std::make_pair((const void*)binary.data(), binary.size()));

    cl::Program program(context, { device }, bins);
    cl_int err;
    cl::Kernel kernel(program, "grayscale_kernel", &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Error: Failed to create kernel. (" << err << ")\n";
        return 1;
    }

    cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Error: Failed to create command queue. (" << err << ")\n";
        return 1;
    }

    // Load input RGB data
    std::vector<unsigned char> input(IMG_SIZE);
    std::ifstream in_file("data/input.rgb", std::ios::binary);
    if (!in_file.read(reinterpret_cast<char*>(input.data()), IMG_SIZE)) {
        std::cerr << "Error: Failed to read input.rgb\n";
        return 1;
    }

    // Prepare output buffer
    std::vector<unsigned char> output(GRAY_SIZE);

    // Create OpenCL buffers on the device
    cl::Buffer in_buf(
        context,
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        static_cast<cl::size_type>(IMG_SIZE),
        (void*)input.data(),
        &err
    );

    cl::Buffer out_buf(
        context,
        CL_MEM_WRITE_ONLY,
        static_cast<cl::size_type>(GRAY_SIZE),
        nullptr,
        &err
    );

    // Set kernel arguments
    kernel.setArg(0, in_buf);
    kernel.setArg(1, out_buf);
    kernel.setArg(2, (int)IMG_SIZE);

    // Launch the kernel
    err = queue.enqueueTask(kernel);
    if (err != CL_SUCCESS) {
        std::cerr << "Error: Failed to launch kernel. (" << err << ")\n";
        return 1;
    }

    // Read back the result
    err = queue.enqueueReadBuffer(out_buf, CL_TRUE, 0, GRAY_SIZE, output.data());
    if (err != CL_SUCCESS) {
        std::cerr << "Error: Failed to read output buffer. (" << err << ")\n";
        return 1;
    }

    // Save as PGM
    std::ofstream out_file("data/output.pgm", std::ios::binary);
    out_file << "P5\n1280 720\n255\n";
    out_file.write(reinterpret_cast<char*>(output.data()), GRAY_SIZE);
    out_file.close();

    std::cout << "Grayscale conversion complete.\n";
    return 0;
}