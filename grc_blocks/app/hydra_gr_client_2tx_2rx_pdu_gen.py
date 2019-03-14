#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Hydra Gr Client 2Tx 2Rx Pdu Gen
# Generated: Tue Mar 12 14:58:10 2019
##################################################

from distutils.version import StrictVersion

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"

from PyQt5 import Qt, QtCore
from gnuradio import blocks
from gnuradio import digital
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from gnuradio.qtgui import Range, RangeWidget
from optparse import OptionParser
import hydra
import pmt
import sys
import threading
from gnuradio import qtgui


class hydra_gr_client_2tx_2rx_pdu_gen(gr.top_block, Qt.QWidget):

    def __init__(self, freqrx=2.22e9 + 3e6, freqtx=2.22e9, hydraClient='192.168.5.73', samp_rate=200e3, vr1offset=-300e3, vr2offset=700e3):
        gr.top_block.__init__(self, "Hydra Gr Client 2Tx 2Rx Pdu Gen")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Hydra Gr Client 2Tx 2Rx Pdu Gen")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "hydra_gr_client_2tx_2rx_pdu_gen")

        if StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
            self.restoreGeometry(self.settings.value("geometry").toByteArray())
        else:
            self.restoreGeometry(self.settings.value("geometry", type=QtCore.QByteArray))

        ##################################################
        # Parameters
        ##################################################
        self.freqrx = freqrx
        self.freqtx = freqtx
        self.hydraClient = hydraClient
        self.samp_rate = samp_rate
        self.vr1offset = vr1offset
        self.vr2offset = vr2offset

        ##################################################
        # Variables
        ##################################################
        self.mul = mul = 0.01

        ##################################################
        # Blocks
        ##################################################
        self._mul_range = Range(0, 1, 0.01, 0.01, 200)
        self._mul_win = RangeWidget(self._mul_range, self.set_mul, 'mul', "counter_slider", float)
        self.top_layout.addWidget(self._mul_win)
        self.hydra_gr_sink_0_0 = hydra.hydra_gr_client_sink(2, hydraClient, 5000)
        self.hydra_gr_sink_0_0.start_client(freqtx + vr2offset, samp_rate, 1024)
        self.hydra_gr_sink_0 = hydra.hydra_gr_client_sink(1, hydraClient, 5000)
        self.hydra_gr_sink_0.start_client(freqtx + vr1offset, samp_rate * 2, 1024)
        self.hydra_gr__source_0_0_0 = hydra.hydra_gr_client_source(2, hydraClient, hydraClient, 5000)
        self.hydra_gr__source_0_0_0.start_client(freqrx + vr2offset, samp_rate, 10000)

        self.hydra_gr__source_0_0 = hydra.hydra_gr_client_source(1, hydraClient, hydraClient, 5000)
        self.hydra_gr__source_0_0.start_client(freqrx + vr1offset, samp_rate * 2, 1024)

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
        self.blocks_tag_debug_0_0 = blocks.tag_debug(gr.sizeof_char*1, 'VR2 RX', ""); self.blocks_tag_debug_0_0.set_display(True)
        self.blocks_tag_debug_0 = blocks.tag_debug(gr.sizeof_char*1, 'VR1 RX', ""); self.blocks_tag_debug_0.set_display(True)
        self.blocks_random_pdu_0_0 = blocks.random_pdu(50, 100, chr(0xFF), 2)
        self.blocks_random_pdu_0 = blocks.random_pdu(50, 100, chr(0xFF), 2)
        self.blocks_pdu_to_tagged_stream_0_0 = blocks.pdu_to_tagged_stream(blocks.byte_t, "len")
        self.blocks_pdu_to_tagged_stream_0 = blocks.pdu_to_tagged_stream(blocks.byte_t, "len")
        self.blocks_multiply_const_vxx_0_0 = blocks.multiply_const_vcc((mul, ))
        self.blocks_multiply_const_vxx_0 = blocks.multiply_const_vcc((mul, ))
        self.blocks_message_strobe_0_0 = blocks.message_strobe(pmt.intern("TEST"), 500)
        self.blocks_message_strobe_0 = blocks.message_strobe(pmt.intern("TEST"), 1000)

        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_message_strobe_0, 'strobe'), (self.blocks_random_pdu_0, 'generate'))
        self.msg_connect((self.blocks_message_strobe_0_0, 'strobe'), (self.blocks_random_pdu_0_0, 'generate'))
        self.msg_connect((self.blocks_random_pdu_0, 'pdus'), (self.blocks_pdu_to_tagged_stream_0, 'pdus'))
        self.msg_connect((self.blocks_random_pdu_0_0, 'pdus'), (self.blocks_pdu_to_tagged_stream_0_0, 'pdus'))
        self.connect((self.blocks_multiply_const_vxx_0, 0), (self.hydra_gr_sink_0, 0))
        self.connect((self.blocks_multiply_const_vxx_0_0, 0), (self.hydra_gr_sink_0_0, 0))
        self.connect((self.blocks_pdu_to_tagged_stream_0, 0), (self.digital_ofdm_tx_0, 0))
        self.connect((self.blocks_pdu_to_tagged_stream_0_0, 0), (self.digital_ofdm_tx_0_0, 0))
        self.connect((self.digital_ofdm_rx_0, 0), (self.blocks_tag_debug_0, 0))
        self.connect((self.digital_ofdm_rx_0_0, 0), (self.blocks_tag_debug_0_0, 0))
        self.connect((self.digital_ofdm_tx_0, 0), (self.blocks_multiply_const_vxx_0, 0))
        self.connect((self.digital_ofdm_tx_0_0, 0), (self.blocks_multiply_const_vxx_0_0, 0))
        self.connect((self.hydra_gr__source_0_0, 0), (self.digital_ofdm_rx_0, 0))
        self.connect((self.hydra_gr__source_0_0_0, 0), (self.digital_ofdm_rx_0_0, 0))

    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "hydra_gr_client_2tx_2rx_pdu_gen")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_freqrx(self):
        return self.freqrx

    def set_freqrx(self, freqrx):
        self.freqrx = freqrx

    def get_freqtx(self):
        return self.freqtx

    def set_freqtx(self, freqtx):
        self.freqtx = freqtx

    def get_hydraClient(self):
        return self.hydraClient

    def set_hydraClient(self, hydraClient):
        self.hydraClient = hydraClient

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

    def get_mul(self):
        return self.mul

    def set_mul(self, mul):
        self.mul = mul
        self.blocks_multiply_const_vxx_0_0.set_k((self.mul, ))
        self.blocks_multiply_const_vxx_0.set_k((self.mul, ))


def argument_parser():
    parser = OptionParser(usage="%prog: [options]", option_class=eng_option)
    parser.add_option(
        "", "--hydraClient", dest="hydraClient", type="string", default='192.168.5.73',
        help="Set hydraClient [default=%default]")
    return parser


def main(top_block_cls=hydra_gr_client_2tx_2rx_pdu_gen, options=None):
    if options is None:
        options, _ = argument_parser().parse_args()

    if StrictVersion("4.5.0") <= StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls(hydraClient=options.hydraClient)
    tb.start()
    tb.show()

    def quitting():
        tb.stop()
        tb.wait()
    qapp.aboutToQuit.connect(quitting)
    qapp.exec_()


if __name__ == '__main__':
    main()
