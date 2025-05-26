#include <CL/cl2.hpp>
#include <fstream>
#include <iostream>
#include <vector>

#define IMG_SIZE (1280 * 720 * 3)
#define GRAY_SIZE (1280 * 720)

int main(int argc, char** argv) {
    std::string binaryFile = argv[1];

    std::ifstream bin_file(binaryFile, std::ifstream::binary);
    bin_file.seekg(0, bin_file.end);
    unsigned nb = bin_file.tellg();
    bin_file.seekg(0, bin_file.beg);
    std::vector<unsigned char> binary(nb);
    bin_file.read(reinterpret_cast<char*>(binary.data()), nb);

    // Get Xilinx devices
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform platform = platforms[0];

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);
    cl::Device device = devices[0];

    cl::Context context(device);
    cl::Program::Binaries bins{{binary.data(), nb}};
    cl::Program program(context, {device}, bins);
    cl::CommandQueue queue(context, device);
    cl::Kernel kernel(program, "grayscale_kernel");

    // Buffers
    std::vector<unsigned char> input(IMG_SIZE);
    std::vector<unsigned char> output(GRAY_SIZE);
    std::ifstream in("data/input.rgb", std::ios::binary);
    in.read(reinterpret_cast<char*>(input.data()), IMG_SIZE);

    cl::Buffer in_buf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, IMG_SIZE, input.data());
    cl::Buffer out_buf(context, CL_MEM_WRITE_ONLY, GRAY_SIZE);

    // Set args
    kernel.setArg(0, in_buf);
    kernel.setArg(1, out_buf);
    kernel.setArg(2, IMG_SIZE);

    queue.enqueueTask(kernel);
    queue.enqueueReadBuffer(out_buf, CL_TRUE, 0, GRAY_SIZE, output.data());

    std::ofstream out("data/output.pgm", std::ios::binary);
    out << "P5\n1280 720\n255\n";
    out.write(reinterpret_cast<char*>(output.data()), GRAY_SIZE);

    std::cout << "Done.\n";
    return 0;
}