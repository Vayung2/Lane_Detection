extern "C" {
void grayscale_kernel(const unsigned char *in, unsigned char *out, int size) {
#pragma HLS INTERFACE m_axi port=in  offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=in    bundle=control
#pragma HLS INTERFACE s_axilite port=out   bundle=control
#pragma HLS INTERFACE s_axilite port=size  bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    for (int i = 0; i < size; i += 3) {
#pragma HLS PIPELINE
        unsigned char r = in[i];
        unsigned char g = in[i+1];
        unsigned char b = in[i+2];
        out[i/3] = (r + g + b) / 3;
    }
}
}