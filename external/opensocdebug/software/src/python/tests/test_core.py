# Copyright 2017-2019 The Open SoC Debug Project
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

import pytest
import osd


def test_osd_library_version():
    result = osd.osd_library_version()
    assert('major' in result)
    assert('minor' in result)
    assert('micro' in result)
    assert('suffix' in result)


def test_packet_header():
    p = osd.Packet()
    p.set_header(dest=10, src=20, type=0, type_sub=0)

    assert(p.src == 20)
    assert(p.dest == 10)
    assert(p.type == 0)
    assert(p.type_sub == 0)

    assert(len(p) == 3)
    assert(len(p.payload) == 0)


def test_packet_listbehavior():
    p = osd.Packet()
    p.set_header(dest=0, src=0, type=0, type_sub=0)

    # Initial state: 3 header words only, no payload
    assert(len(p) == 3)
    assert(len(p.payload) == 0)

    # append one item to the end of the list
    p.append(1337)
    assert(p[3] == 1337)
    assert(len(p) == 4)

    # insert one item at position 3 (first payload position)
    p.insert(3, 42)
    assert(p[3] == 42)
    assert(p[4] == 1337)
    assert(len(p) == 5)

    # insert one item at position 0 (first header position == dest)
    assert(p.dest == 0)
    assert(p[0] == 0)
    assert(p[2] == 0)  # last header position; will be payload[0] after insert
    p.insert(0, 17)
    assert(p.dest == 17)
    assert(p[0] == 17)
    assert(p[3] == 0)
    assert(p[4] == 42)
    assert(p[5] == 1337)
    assert(len(p) == 6)

    del p[4]
    assert(p[3] == 0)
    assert(p[4] == 1337)
    assert(len(p) == 5)

    del p[0]
    del p[0]
    assert(len(p) == 3)

    # the header words cannot be deleted
    with pytest.raises(ValueError):
        del p[0]


def test_packet_payloadview():
    p = osd.Packet()
    p.set_header(dest=0, src=0, type=0, type_sub=0)

    p.payload.append(30)
    assert(p.payload[0] == 30)
    assert(p[3] == 30)

    p.payload[0] = 33
    assert(p.payload[0] == 33)
    assert(p[3] == 33)

    del p.payload[0]
    assert(len(p.payload) == 0)
