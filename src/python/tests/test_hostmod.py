# Copyright 2019 The Open SoC Debug Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import logging
import pytest
import time
import unittest
from unittest.mock import Mock
import osd

HOSTCTRL_BIND_ADDR = 'tcp://0.0.0.0:9537'
HOSTCTRL_ADDR = 'tcp://127.0.0.1:9537'


@pytest.fixture
def log():
    logging.basicConfig(level=logging.DEBUG)
    log = osd.Log()
    assert log
    return log


@pytest.fixture
def hostctrl(log):
    hostctrl = osd.Hostctrl(log, HOSTCTRL_BIND_ADDR)

    hostctrl.start()
    assert hostctrl.is_running()

    yield hostctrl

    hostctrl.stop()


def test_hostmod_connecterror(log):
    """Test what happens if a host module cannot connect to the host controller"""
    hm = osd.Hostmod(log, 'tcp://127.0.0.1:1234')

    with pytest.raises(osd.OsdErrorException) as excinfo:
        hm.connect()
    assert excinfo.value.result == osd.Result.CONNECTION_FAILED


def test_event_hostmod_to_hostmod(log, hostctrl):
    """Send a small event packet between host modules"""
    hm1 = osd.Hostmod(log, HOSTCTRL_ADDR)
    hm2 = osd.Hostmod(log, HOSTCTRL_ADDR)

    hm1.connect()
    assert hm1.is_connected()

    hm2.connect()
    assert hm2.is_connected()

    pkg_sent = osd.Packet()
    pkg_sent.set_header(dest=hm2.diaddr, src=hm1.diaddr,
                        type=osd.Packet.TYPE_EVENT, type_sub=0)
    pkg_sent.payload.append(1337)
    hm1.event_send(pkg_sent)

    pkg_received = hm2.event_receive()
    assert pkg_sent == pkg_received

    hm2.disconnect()
    hm1.disconnect()


def test_event_hostmod_to_hostmod_large(log, hostctrl):
    """Send a large event packet which will be split into two"""
    hm1 = osd.Hostmod(log, HOSTCTRL_ADDR)
    hm2 = osd.Hostmod(log, HOSTCTRL_ADDR)

    hm1.connect()
    assert hm1.is_connected()

    hm2.connect()
    assert hm2.is_connected()

    pkg_sent = osd.Packet()
    pkg_sent.set_header(dest=hm2.diaddr, src=hm1.diaddr,
                        type=osd.Packet.TYPE_EVENT, type_sub=0)
    for i in range(hm1.get_max_event_words(hm2.diaddr) + 1):
        pkg_sent.payload.append(i + 1337)

    hm1.event_send(pkg_sent)

    pkg_received = hm2.event_receive()
    assert pkg_sent == pkg_received

    hm2.disconnect()
    hm1.disconnect()


def test_event_hostmod_to_hostmod_callback(log, hostctrl):
    """Receive packet with callback"""

    mock_event_handler = Mock()

    hm1 = osd.Hostmod(log, HOSTCTRL_ADDR)
    hm2 = osd.Hostmod(log, HOSTCTRL_ADDR, event_handler=mock_event_handler)

    hm1.connect()
    assert hm1.is_connected()

    hm2.connect()
    assert hm2.is_connected()

    pkg_sent = osd.Packet()
    pkg_sent.set_header(dest=hm2.diaddr, src=hm1.diaddr,
                        type=osd.Packet.TYPE_EVENT, type_sub=0)
    pkg_sent.payload.append(1337)

    hm1.event_send(pkg_sent)

    # XXX: That's a bit fragile: expect the event to be delivered within 0.5 s
    time.sleep(0.5)

    mock_event_handler.assert_called_with(pkg_sent)

    hm2.disconnect()
    hm1.disconnect()


def test_event_hostmod_to_hostmod_callback_class(log, hostctrl):
    """Receive packet with callback living inside a class"""

    test_finish = False

    # Hostmod 1 (sender)
    hm1 = osd.Hostmod(log, HOSTCTRL_ADDR)
    hm1.connect()
    assert hm1.is_connected()

    # Hostmod 2 (receiver)
    class Receiver:

        def __init__(self, log, host_controller):
            self.hm = osd.Hostmod(log, host_controller,
                                  event_handler=self.event_handler)
            self.hm.connect()
            assert self.hm.is_connected()

        def event_handler(self, pkg_received):
            nonlocal test_finish
            assert pkg_received == pkg_sent
            test_finish = True

    rcv = Receiver(log, HOSTCTRL_ADDR)

    # packet to send out
    pkg_sent = osd.Packet()
    pkg_sent.set_header(dest=rcv.hm.diaddr, src=hm1.diaddr,
                        type=osd.Packet.TYPE_EVENT, type_sub=0)
    pkg_sent.payload.append(1337)

    # send it out
    hm1.event_send(pkg_sent)

    # wait for packet to be received by callback
    for _ in range(10):
        if test_finish:
            break
        time.sleep(0.1)
    assert test_finish == True

    hm1.disconnect()
