#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Ansible Hydra Gr Client 2Tx 2Rx
# Generated: Tue Mar 12 17:01:41 2019
##################################################


from gnuradio import blocks
from gnuradio import digital
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser
import hydra
import threading


class ansible_hydra_gr_client_2tx_2rx(gr.top_block):

    def __init__(self, ansibleIP='192.168.5.73', freqrx=2.22e9+3e6, freqtx=2.22e9, mul=0.01, samp_rate=200e3, vr1offset=-300e3, vr2offset=700e3):
        gr.top_block.__init__(self, "Ansible Hydra Gr Client 2Tx 2Rx")

        ##################################################
        # Parameters
        ##################################################
        self.ansibleIP = ansibleIP
        self.freqrx = freqrx
        self.freqtx = freqtx
        self.mul = mul
        self.samp_rate = samp_rate
        self.vr1offset = vr1offset
        self.vr2offset = vr2offset

        ##################################################
        # Blocks
        ##################################################
        self.hydra_gr_sink_0_0 = hydra.hydra_gr_client_sink(2, ansibleIP, 5000)
        self.hydra_gr_sink_0_0.start_client(freqtx + vr2offset, samp_rate, 1024)
        self.hydra_gr_sink_0 = hydra.hydra_gr_client_sink(1, ansibleIP, 5000)
        self.hydra_gr_sink_0.start_client(freqtx + vr1offset, samp_rate * 2, 1024)
        self.hydra_gr__source_0_0_0 = hydra.hydra_gr_client_source(2, ansibleIP, ansibleIP, 5000)
        self.hydra_gr__source_0_0_0.start_client(freqrx + vr2offset, samp_rate, 10000)

        self.hydra_gr__source_0_0 = hydra.hydra_gr_client_source(1, ansibleIP, ansibleIP, 5000)
        self.hydra_gr__source_0_0.start_client(freqrx + vr1offset, samp_rate * 2, 10000)

        self.digital_ofdm_tx_0_0 = digital.ofdm_tx(
        	  fft_len=64, cp_len=16,
        	  packet_length_tag_key="len",
        	  bps_header=1,
        	  bps_payload=1,
        	  rolloff=0,
        	  debug_log=False,
        	  scramble_bits=False
        	 )
        self.digital_ofdm_tx_0 = digital.ofdm_tx(
        	  fft_len=64, cp_len=16,
        	  packet_length_tag_key="len",
        	  bps_header=1,
        	  bps_payload=1,
        	  rolloff=0,
        	  debug_log=False,
        	  scramble_bits=False
        	 )
        self.digital_ofdm_rx_0_0 = digital.ofdm_rx(
        	  fft_len=64, cp_len=16,
        	  frame_length_tag_key='frame_'+"len",
        	  packet_length_tag_key="len",
        	  bps_header=1,
        	  bps_payload=1,
        	  debug_log=False,
        	  scramble_bits=False
        	 )
        self.digital_ofdm_rx_0 = digital.ofdm_rx(
        	  fft_len=64, cp_len=16,
        	  frame_length_tag_key='frame_'+"len",
        	  packet_length_tag_key="len",
        	  bps_header=1,
        	  bps_payload=1,
        	  debug_log=False,
        	  scramble_bits=False
        	 )
        self.blocks_tuntap_pdu_1_0 = blocks.tuntap_pdu('tap1', 10000, False)
        self.blocks_tuntap_pdu_1 = blocks.tuntap_pdu('tap0', 10000, False)
        self.blocks_tagged_stream_to_pdu_0_0 = blocks.tagged_stream_to_pdu(blocks.byte_t, "len")
        self.blocks_tagged_stream_to_pdu_0 = blocks.tagged_stream_to_pdu(blocks.byte_t, "len")
        self.blocks_tag_debug_0_0 = blocks.tag_debug(gr.sizeof_char*1, 'VR2 RX', ""); self.blocks_tag_debug_0_0.set_display(True)
        self.blocks_tag_debug_0 = blocks.tag_debug(gr.sizeof_char*1, 'VR1 RX', ""); self.blocks_tag_debug_0.set_display(True)
        self.blocks_pdu_to_tagged_stream_0_0 = blocks.pdu_to_tagged_stream(blocks.byte_t, "len")
        self.blocks_pdu_to_tagged_stream_0 = blocks.pdu_to_tagged_stream(blocks.byte_t, "len")
        self.blocks_multiply_const_vxx_0_0 = blocks.multiply_const_vcc((mul, ))
        self.blocks_multiply_const_vxx_0 = blocks.multiply_const_vcc((mul, ))

        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_tagged_stream_to_pdu_0, 'pdus'), (self.blocks_tuntap_pdu_1, 'pdus'))
        self.msg_connect((self.blocks_tagged_stream_to_pdu_0_0, 'pdus'), (self.blocks_tuntap_pdu_1_0, 'pdus'))
        self.msg_connect((self.blocks_tuntap_pdu_1, 'pdus'), (self.blocks_pdu_to_tagged_stream_0, 'pdus'))
        self.msg_connect((self.blocks_tuntap_pdu_1_0, 'pdus'), (self.blocks_pdu_to_tagged_stream_0_0, 'pdus'))
        self.connect((self.blocks_multiply_const_vxx_0, 0), (self.hydra_gr_sink_0, 0))
        self.connect((self.blocks_multiply_const_vxx_0_0, 0), (self.hydra_gr_sink_0_0, 0))
        self.connect((self.blocks_pdu_to_tagged_stream_0, 0), (self.digital_ofdm_tx_0, 0))
        self.connect((self.blocks_pdu_to_tagged_stream_0_0, 0), (self.digital_ofdm_tx_0_0, 0))
        self.connect((self.digital_ofdm_rx_0, 0), (self.blocks_tag_debug_0, 0))
        self.connect((self.digital_ofdm_rx_0, 0), (self.blocks_tagged_stream_to_pdu_0, 0))
        self.connect((self.digital_ofdm_rx_0_0, 0), (self.blocks_tag_debug_0_0, 0))
        self.connect((self.digital_ofdm_rx_0_0, 0), (self.blocks_tagged_stream_to_pdu_0_0, 0))
        self.connect((self.digital_ofdm_tx_0, 0), (self.blocks_multiply_const_vxx_0, 0))
        self.connect((self.digital_ofdm_tx_0_0, 0), (self.blocks_multiply_const_vxx_0_0, 0))
        self.connect((self.hydra_gr__source_0_0, 0), (self.digital_ofdm_rx_0, 0))
        self.connect((self.hydra_gr__source_0_0_0, 0), (self.digital_ofdm_rx_0_0, 0))

    def get_ansibleIP(self):
        return self.ansibleIP

    def set_ansibleIP(self, ansibleIP):
        self.ansibleIP = ansibleIP

    def get_freqrx(self):
        return self.freqrx

    def set_freqrx(self, freqrx):
        self.freqrx = freqrx

    def get_freqtx(self):
        return self.freqtx

    def set_freqtx(self, freqtx):
        self.freqtx = freqtx

    def get_mul(self):
        return self.mul

    def set_mul(self, mul):
        self.mul = mul
        self.blocks_multiply_const_vxx_0_0.set_k((self.mul, ))
        self.blocks_multiply_const_vxx_0.set_k((self.mul, ))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate

    def get_vr1offset(self):
        return self.vr1offset

    def set_vr1offset(self, vr1offset):
        self.vr1offset = vr1offset

    def get_vr2offset(self):
        return self.vr2offset

    def set_vr2offset(self, vr2offset):
        self.vr2offset = vr2offset


def argument_parser():
    parser = OptionParser(usage="%prog: [options]", option_class=eng_option)
    parser.add_option(
        "", "--ansibleIP", dest="ansibleIP", type="string", default='192.168.5.73',
        help="Set ansibleIP [default=%default]")
    return parser


def main(top_block_cls=ansible_hydra_gr_client_2tx_2rx, options=None):
    if options is None:
        options, _ = argument_parser().parse_args()

    tb = top_block_cls(ansibleIP=options.ansibleIP)
    tb.start()
    tb.wait()


if __name__ == '__main__':
    main()
