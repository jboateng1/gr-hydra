#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Ansible Vr2 Rx
# Generated: Mon Dec  3 15:33:24 2018
##################################################


from gnuradio import blocks
from gnuradio import digital
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio import uhd
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser
import time


class ansible_vr2_rx(gr.top_block):

    def __init__(self, samp_rate=200e3, freqtx=1.1e9, vr2offset=700e3, freqrx=1.2e9, vr1offset=-100e3):
        gr.top_block.__init__(self, "Ansible Vr2 Rx")

        ##################################################
        # Parameters
        ##################################################
        self.samp_rate = samp_rate
        self.freqtx = freqtx
        self.vr2offset = vr2offset
        self.freqrx = freqrx
        self.vr1offset = vr1offset

        ##################################################
        # Blocks
        ##################################################
        self.uhd_usrp_source_0 = uhd.usrp_source(
        	",".join(("", "")),
        	uhd.stream_args(
        		cpu_format="fc32",
        		channels=range(1),
        	),
        )
        self.uhd_usrp_source_0.set_samp_rate(samp_rate*2)
        self.uhd_usrp_source_0.set_center_freq(freqrx + vr2offset, 0)
        self.uhd_usrp_source_0.set_normalized_gain(0, 0)
        self.uhd_usrp_source_0.set_antenna('RX2', 0)
        self.digital_ofdm_rx_0 = digital.ofdm_rx(
        	  fft_len=64, cp_len=16,
        	  frame_length_tag_key='frame_'+"len",
        	  packet_length_tag_key="len",
        	  bps_header=1,
        	  bps_payload=1,
        	  debug_log=False,
        	  scramble_bits=False
        	 )
        self.blocks_tagged_stream_to_pdu_0 = blocks.tagged_stream_to_pdu(blocks.byte_t, "len")
        self.blocks_message_debug_0 = blocks.message_debug()

        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_tagged_stream_to_pdu_0, 'pdus'), (self.blocks_message_debug_0, 'print_pdu'))
        self.connect((self.digital_ofdm_rx_0, 0), (self.blocks_tagged_stream_to_pdu_0, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.digital_ofdm_rx_0, 0))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.uhd_usrp_source_0.set_samp_rate(self.samp_rate*2)

    def get_freqtx(self):
        return self.freqtx

    def set_freqtx(self, freqtx):
        self.freqtx = freqtx

    def get_vr2offset(self):
        return self.vr2offset

    def set_vr2offset(self, vr2offset):
        self.vr2offset = vr2offset
        self.uhd_usrp_source_0.set_center_freq(self.freqrx + self.vr2offset, 0)

    def get_freqrx(self):
        return self.freqrx

    def set_freqrx(self, freqrx):
        self.freqrx = freqrx
        self.uhd_usrp_source_0.set_center_freq(self.freqrx + self.vr2offset, 0)

    def get_vr1offset(self):
        return self.vr1offset

    def set_vr1offset(self, vr1offset):
        self.vr1offset = vr1offset


def argument_parser():
    parser = OptionParser(usage="%prog: [options]", option_class=eng_option)
    return parser


def main(top_block_cls=ansible_vr2_rx, options=None):
    if options is None:
        options, _ = argument_parser().parse_args()

    tb = top_block_cls()
    tb.start()
    tb.wait()


if __name__ == '__main__':
    main()
