// Copyright 2016 by the authors
//
// Copyright and related rights are licensed under the Solderpad
// Hardware License, Version 0.51 (the "License"); you may not use
// this file except in compliance with the License. You may obtain a
// copy of the License at http://solderpad.org/licenses/SHL-0.51.
// Unless required by applicable law or agreed to in writing,
// software, hardware and materials distributed under this License is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
// OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the
// License.
//
// Authors:
//    Wei Song <ws327@cam.ac.uk>

package open_soc_debug

import Chisel._
import cde.{Parameters}

/** Generic buffer (Register Queue) for debug network
  * @param width Number of parallel queues
  * @param length Depth of each queue (in unit of flit)
  */
class DebugNetworkBufferLike(width:Int, length:Int)(implicit p: Parameters) extends
    DebugNetworkConnector(width, width)(p)
{
  io.ip zip io.op foreach { case (i,o) => o <> Queue(i, length) }
}

class DebugNetworkBuffer(length:Int)(implicit p: Parameters)
    extends DebugNetworkBufferLike(1, length)(p)
