#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Virtualization Daemon
# GNU Radio version: 3.7.13.5
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
            print("Warning: failed to XInitThreads()")

from PyQt5 import Qt, QtCore
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser
import hydra
import sys
import threading
from gnuradio import qtgui


class ansible_hydra_gr_server(gr.top_block, Qt.QWidget):

    def __init__(self, ansibleIPPort='127.0.1.1:5000', freqrx=1.1e9+5e6, freqtx=1.1e9):
        gr.top_block.__init__(self, "Ansible Hydra Gr Server")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Ansible Hydra Gr Server")
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

        self.settings = Qt.QSettings("GNU Radio", "ansible_hydra_gr_server")
        self.restoreGeometry(self.settings.value("geometry", type=QtCore.QByteArray))


        ##################################################
        # Parameters
        ##################################################
        self.ansibleIPPort = ansibleIPPort
        self.freqrx = freqrx
        self.freqtx = freqtx

        ##################################################
        # Blocks
        ##################################################
        self.ahydra_gr_server_0 = hydra.hydra_gr_server(ansibleIPPort, 'default')
        if freqtx > 0 and 2e6 > 0 and 2048 > 0 and 0.6 >= 0 and 0.6 >= 0:
           self.ahydra_gr_server_0.set_tx_config(freqtx, 2e6, 0.6,2048, "USRP")
        if freqrx > 0 and 2e6 > 0 and 2048 > 0 and 0.0 >= 0 and 0.0 >= 0:
           self.ahydra_gr_server_0.set_rx_config(freqrx, 2e6, 0.0, 2048, "USRP")
        self.ahydra_gr_server_0_thread = threading.Thread(target=self.ahydra_gr_server_0.start_server)
        self.ahydra_gr_server_0_thread.daemon = True
        self.ahydra_gr_server_0_thread.start()



    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "ansible_hydra_gr_server")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_ansibleIPPort(self):
        return self.ansibleIPPort

    def set_ansibleIPPort(self, ansibleIPPort):
        self.ansibleIPPort = ansibleIPPort

    def get_freqrx(self):
        return self.freqrx

    def set_freqrx(self, freqrx):
        self.freqrx = freqrx

    def get_freqtx(self):
        return self.freqtx

    def set_freqtx(self, freqtx):
        self.freqtx = freqtx


def argument_parser():
    parser = OptionParser(usage="%prog: [options]", option_class=eng_option)
    parser.add_option(
        "", "--ansibleIPPort", dest="ansibleIPPort", type="string", default='127.0.1.1:5000',
        help="Set ansibleIPPort [default=%default]")
    parser.add_option(
        "", "--freqrx", dest="freqrx", type="eng_float", default=eng_notation.num_to_str(1.1e9+5e6),
        help="Set freqrx [default=%default]")
    parser.add_option(
        "", "--freqtx", dest="freqtx", type="eng_float", default=eng_notation.num_to_str(1.1e9),
        help="Set freqtx [default=%default]")
    return parser


def main(top_block_cls=ansible_hydra_gr_server, options=None):
    if options is None:
        options, _ = argument_parser().parse_args()

    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls(ansibleIPPort=options.ansibleIPPort, freqrx=options.freqrx, freqtx=options.freqtx)
    tb.start()
    tb.show()

    def quitting():
        tb.stop()
        tb.wait()
    qapp.aboutToQuit.connect(quitting)
    qapp.exec_()


if __name__ == '__main__':
    main()
