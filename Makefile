XDEVICE=xilinx_u250_gen3x16_xdma_4_1_202210_1
PLATFORM=/opt/xilinx/platforms/$(XDEVICE)/$(XDEVICE).xpfm
KERNEL=grayscale_kernel

all: host xclbin

xclbin:
	/opt/xilinx/Vitis/2022.1/bin/v++ -c -t hw --platform $(PLATFORM) -k $(KERNEL) -o $(KERNEL).xo grayscale.cpp
	/opt/xilinx/Vitis/2022.1/bin/v++ -l -t hw --platform $(PLATFORM) -o $(KERNEL).xclbin $(KERNEL).xo

host:
	# FPGA kernel build is in xclbin target; now compile host with C++17
	g++ -std=c++17 \
	    -I/opt/xilinx/Vitis/2022.1/include \
	    -I/opt/xilinx/xrt/include \
	    host.cpp -o host \
	    -L/opt/xilinx/xrt/lib -lxrt_coreutil

clean:
	rm -f *.xo *.xclbin host