XDEVICE=xilinx_u250_gen3x16_xdma_4_1_202210_1
PLATFORM=/opt/xilinx/platforms/$(XDEVICE)/$(XDEVICE).xpfm
KERNEL=grayscale_kernel

all: host xclbin

host:
	/opt/xilinx/Vitis/2022.1/bin/v++ -c -t hw --platform $(PLATFORM) -k $(KERNEL) -o $(KERNEL).xo grayscale.cpp
	/opt/xilinx/Vitis/2022.1/bin/v++ -l -t hw --platform $(PLATFORM) -o $(KERNEL).xclbin $(KERNEL).xo

	# Host compile
	g++ -I/opt/xilinx/Vitis/2022.1/include -I/opt/xilinx/xrt/include -o host host.cpp -L/opt/xilinx/xrt/lib -lxrt_coreutil

xclbin: host

clean:
	rm -f *.xo *.xclbin host