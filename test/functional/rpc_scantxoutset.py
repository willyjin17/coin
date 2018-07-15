#!/usr/bin/env python3
# Copyright (c) 2018 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test the scantxoutset rpc call."""
from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import *

import shutil
import os

class ScantxoutsetTest(BitcoinTestFramework):
    def set_test_params(self):
        self.num_nodes = 1
        self.setup_clean_chain = True
    def run_test(self):
        self.log.info("Mining blocks...")
        self.nodes[0].generate(110)

        addr_P2SH_SEGWIT = self.nodes[0].getnewaddress("", "p2sh-segwit")
        pubk1 = self.nodes[0].getaddressinfo(addr_P2SH_SEGWIT)['pubkey']
        addr_LEGACY = self.nodes[0].getnewaddress("", "legacy")
        pubk2 = self.nodes[0].getaddressinfo(addr_LEGACY)['pubkey']
        addr_BECH32 = self.nodes[0].getnewaddress("", "bech32")
        pubk3 = self.nodes[0].getaddressinfo(addr_BECH32)['pubkey']
        self.nodes[0].sendtoaddress(addr_P2SH_SEGWIT, 1)
        self.nodes[0].sendtoaddress(addr_LEGACY, 2)
        self.nodes[0].sendtoaddress(addr_BECH32, 3)
        self.nodes[0].generate(1)

        self.log.info("Stop node, remove wallet, mine again some blocks...")
        self.stop_node(0)
        shutil.rmtree(os.path.join(self.nodes[0].datadir, "regtest", 'wallets'))
        self.start_node(0)
        self.nodes[0].generate(110)

        self.restart_node(0, ['-nowallet'])
        self.log.info("Test if we have found the non HD unspent outputs.")
        assert_equal(self.nodes[0].scantxoutset("start", [ {"pubkey": {"pubkey": pubk1}}, {"pubkey": {"pubkey": pubk2}}, {"pubkey": {"pubkey": pubk3}}])['total_amount'], 6)
        assert_equal(self.nodes[0].scantxoutset("start", [ {"address": addr_P2SH_SEGWIT}, {"address": addr_LEGACY}, {"address": addr_BECH32}])['total_amount'], 6)
        assert_equal(self.nodes[0].scantxoutset("start", [ {"address": addr_P2SH_SEGWIT}, {"address": addr_LEGACY}, {"pubkey": {"pubkey": pubk3}} ])['total_amount'], 6)

        self.log.info("Test invalid parameters.")
        assert_raises_rpc_error(-8, 'Scanobject "pubkey" must contain an object as value', self.nodes[0].scantxoutset, "start", [ {"pubkey": pubk1}]) #missing pubkey object
        assert_raises_rpc_error(-8, 'Scanobject "address" must contain a single string as value', self.nodes[0].scantxoutset, "start", [ {"address": {"address": addr_P2SH_SEGWIT}}]) #invalid object for address object

if __name__ == '__main__':
    ScantxoutsetTest().main()
