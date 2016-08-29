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
//    Stefan Wallentowitz <stefan@wallentowitz.de>

package open_soc_debug

import Chisel._
import cde.{Parameters}

/** Ring router
  * Basic element for a ring network
  * 1 local node, 2 expandible ports
  */

class DebugRingRouter(id:Int, buf_len:Int)(implicit p: Parameters)
    extends ExpandibleDebugNetwork(1,2)(p) {
  /*
   *       /|--------------buffer-out1
   *  in1 -||
   *       \|---+
   *            |
   *       /|---|-------|\
   *  in0 -||   |       ||-buffer-out0
   *       \|-+ |     +-|/
   *          | |     |
   *          ---     |
   *          \_/     |
   *           |      |
   *         buffer   |
   *           |      |
   *          out    in
   */

  def local_id_route(flit: DiiFlit, valid: Bool):UInt = {
    val worm = Reg(init=Bool(false))
    worm := Mux(valid && flit.last, Bool(false), Mux(valid, Bool(true), worm))
    val route = Mux(flit.data(9,0) === UInt(id), UInt(0), UInt(1))
    val route_reg = RegEnable(route, valid && !worm)
    Mux(worm, route_reg, route)
  }

  val ring0_demux = Module(new DebugNetworkDemultiplexer(2)(local_id_route))
  val ring1_demux = Module(new DebugNetworkDemultiplexer(2)(local_id_route))
  val ring0_mux = Module(new DebugNetworkMultiplexer(2))
  val local_mux = Module(new DebugNetworkMultiplexerRR(2))
  val ring0_buffer = Module(new DebugNetworkBuffer(buf_len))
  val local_buffer = Module(new DebugNetworkBuffer(buf_len))

  io.net(0).dii_in <> ring0_demux.io.ip(0)
  io.net(0).dii_out <> ring0_buffer.io.op(0)
  io.net(1).dii_in <> ring1_demux.io.ip(0)
  io.net(1).dii_out <> ring1_demux.io.op(1)

  io.loc(0).dii_in <> local_buffer.io.op(0)
  io.loc(0).dii_out <> ring0_mux.io.ip(1)

  ring0_demux.io.op(1) <> ring0_mux.io.ip(0)
  ring0_demux.io.op(0) <> local_mux.io.ip(0)
  ring1_demux.io.op(0) <> local_mux.io.ip(1)

  ring0_mux.io.op(0) <> ring0_buffer.io.ip(0)
  local_mux.io.op(0) <> local_buffer.io.ip(0)
}

/** Ring network with expandible ports
  */
class ExpandibleRingNetwork(id_assign: Int => Int, nodes:Int, buf_len:Int)(implicit p: Parameters)
    extends ExpandibleDebugNetwork(nodes, 2)(p)
{

  require(nodes > 1)
  val routers = (0 until nodes).map(id => Module(new DebugRingRouter(id_assign(id),buf_len)))

  io.loc.zipWithIndex.foreach { case (loc, i) => {
    val router = routers(i)
    loc <> router.io.loc(0)
    router.io.net(0).dii_in <> (if(i==0) io.net(0).dii_in else routers(i-1).io.net(0).dii_out)
    router.io.net(1).dii_in <> (if(i==0) io.net(1).dii_in else routers(i-1).io.net(1).dii_out)
  }}
  routers(nodes-1).io.net(0).dii_out <> io.net(0).dii_out
  routers(nodes-1).io.net(1).dii_out <> io.net(1).dii_out
}

/** A stand-slone Ring network
  */
class RingNetwork(id_assign: Int => Int, nodes:Int, buf_len:Int)(implicit p: Parameters)
    extends DebugNetwork(nodes)(p)
{

  val ring = Module(new ExpandibleRingNetwork(id_assign,nodes, buf_len))

  io.loc <> ring.io.loc
  ring.io.net(0).dii_in.valid := Bool(false)
  ring.io.net(1).dii_in <> ring.io.net(0).dii_out
  ring.io.net(1).dii_out.ready := Bool(true)
}
