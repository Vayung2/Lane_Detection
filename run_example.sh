#!/bin/bash

# Step 1: SSH into the Remote Server
ssh vgupta8@hal-fpga-x86.ncsa.illinois.edu

# Step 2: Copy the example directory
cp -r /opt/xilinx/Vitis/2022.1/examples examples22

# Step 3: Change to the vadd example directory
cd examples22/vadd

# Step 4: Replace the XDEVICE in vitis.mk
# Manually replace "XDEVICE=xilinx_u200_xdma_201920_1" with "XDEVICE=xilinx_u250_gen3x16_xdma_4_1_202210_1" in the vitis.mk file

# Step 5: Change the vitis.mk to define the PLATFORM variable
# Open vitis.mk and add the following line:
# PLATFORM=/opt/xilinx/platforms/${XDEVICE}/${XDEVICE}.xpfm

# Step 6: Enable Developer Toolset 9
scl enable devtoolset-9 bash

# Step 7: Source Vitis environment
source /opt/xilinx/Vitis/2022.1/settings64.sh

# Step 8: Source XRT environment
source /opt/xilinx/xrt/setup.sh

# Step 9: Compile the host code (without the platform option if step 5 is done)
make -f vitis.mk host PLATFORM=/opt/xilinx/platforms/xilinx_u250_gen3x16_xdma_4_1_202210_1/xilinx_u250_gen3x16_xdma_4_1_202210_1.xpfm

# Step 10: Compile the hardware binary (without the platform option if step 5 is done)
make -f vitis.mk xbin_hw PLATFORM=/opt/xilinx/platforms/xilinx_u250_gen3x16_xdma_4_1_202210_1/xilinx_u250_gen3x16_xdma_4_1_202210_1.xpfm

# Step 11: Link and package the hardware binary
v++ -p bin_vadd_hw.link.xclbin -t hw --platform xilinx_u250_gen3x16_xdma_4_1_202210_1 --package.out_dir ./package. -o bin_vadd_hw.xclbin

# Step 12: Source XRT environment again
source /opt/xilinx/xrt/setup.sh

# Step 13: Run the host program
./vadd bin_vadd_hw.xclbin

