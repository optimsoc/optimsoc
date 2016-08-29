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
import junctions.ParameterizedBundle

abstract class DebugNetworkModule(implicit val p: Parameters) extends Module
abstract class DebugNetworkBundle(implicit val p: Parameters) extends ParameterizedBundle()(p)

/** Expandible debug network
  * Base module for all debug networks
  * @param nodes Number of end nodes connected to this network
  * @param eps Number of expandible ports (bidirectional)
  */
class ExpandibleDebugNetwork(nodes:Int, eps:Int)(implicit p: Parameters)
    extends DebugNetworkModule()(p)
{
  val io = new Bundle {
    val loc = Vec(nodes, new DiiIO).flip
    val net = Vec(eps, new DiiIO)
  }
}

/** Stand-alone debug network
  * Base module for all debug networks
  * @param nodes Number of end nodes connected to this network
  * @param eps Number of expandible ports (bidirectional)
  */
class DebugNetwork(nodes:Int)(implicit p: Parameters) extends DebugNetworkModule()(p) {
  val io = new Bundle {
    val loc = Vec(nodes, new DiiIO).flip
  }
}

/** Link connector base class
  * @param ips Number of input ports
  * @param ops Number of output ports
  */
class DebugNetworkConnector(ips:Int, ops:Int)(implicit p: Parameters)
    extends DebugNetworkModule()(p)
{
  val io = new Bundle {
    val ip = Vec(ips, new DecoupledIO(new DiiFlit)).flip
    val op = Vec(ops, new DecoupledIO(new DiiFlit))
  }
}

/** Multiplexer for debug network (static)
  * @param ips Number of input ports
  */
class DebugNetworkMultiplexer(ips:Int)(implicit p: Parameters)
    extends DebugNetworkConnector(ips,1)(p)
{
  val arb = Module(new DebugWormholeArbiter(new DiiFlit,ips))
  io.ip <> arb.io.in
  io.op(0) <> arb.io.out
}

/** Multiplexer for debug network (round-robin)
  * @param ips Number of input ports
  */
class DebugNetworkMultiplexerRR(ips:Int)(implicit p: Parameters)
    extends DebugNetworkConnector(ips,1)(p)
{
  val arb = Module(new DebugWormholeRRArbiter(new DiiFlit,ips))
  io.ip <> arb.io.in
  io.op(0) <> arb.io.out
}

/** Demultiplexer for debug network
  * @param ops Number of output ports
  */
class DebugNetworkDemultiplexer(ops:Int)(route: (DiiFlit,Bool) => UInt)(implicit p: Parameters)
    extends DebugNetworkConnector(1,ops)(p)
{
  val selection = route(io.ip(0).bits, io.ip(0).fire())
  io.op.zipWithIndex.foreach { case (o, i) => {
    o.valid := io.ip(0).valid && selection === UInt(i)
    o.bits := io.ip(0).bits
  }}
  io.ip(0).ready := io.op(selection).ready
}
