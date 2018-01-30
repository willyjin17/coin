#!/usr/bin/env python3
# Copyright (c) 2015-2017 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test node responses to invalid transactions.

In this test we connect to one node over p2p, and test tx requests."""
import time

from test_framework.blocktools import create_block, create_coinbase, create_transaction
from test_framework.comptool import RejectResult, TestInstance, TestManager
from test_framework.messages import COIN
from test_framework.mininode import network_thread_start
from test_framework.test_framework import ComparisonTestFramework

class InvalidTxRequestTest(ComparisonTestFramework):

    def set_test_params(self):
        self.num_nodes = 1
        self.setup_clean_chain = True

    def run_test(self):
        test = TestManager(self, self.options.tmpdir)
        test.add_all_connections(self.nodes)
        self.tip = None
        self.block_time = None
        network_thread_start()
        test.run()

    def get_tests(self):
        self.tip = int("0x" + self.nodes[0].getbestblockhash(), 0)
        self.block_time = int(time.time()) + 1

        self.log.info("Create a new block with an anyone-can-spend coinbase.")
        height = 1
        block = create_block(self.tip, create_coinbase(height), self.block_time)
        self.block_time += 1
        block.solve()
        # Save the coinbase for later
        self.block1 = block
        self.tip = block.sha256
        height += 1
        yield TestInstance([[block, True]])

        self.log.info("Mature the block.")
        test = TestInstance(sync_every_block=False)
        for i in range(100):
            block = create_block(self.tip, create_coinbase(height), self.block_time)
            block.solve()
            self.tip = block.sha256
            self.block_time += 1
            test.blocks_and_transactions.append([block, True])
            height += 1
        yield test

        # b'\x64' is OP_NOTIF
        # Transaction will be rejected with code 16 (REJECT_INVALID)
        tx1 = create_transaction(self.block1.vtx[0], 0, b'\x64', 50 * COIN - 12000)
        yield TestInstance([[tx1, RejectResult(16, b'mandatory-script-verify-flag-failed')]])

        # TODO: test further transactions...

if __name__ == '__main__':
    InvalidTxRequestTest().main()
