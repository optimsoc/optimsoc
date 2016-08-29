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
import cde.{Parameters, Field}
import junctions.ParameterizedBundle

case object UseDebug extends Field[Boolean]
case object DebugCtmID extends Field[Int]
case object DebugStmID extends Field[Int]
case object DebugCtmScorBoardSize extends Field[Int]
case object DebugStmCsrAddr extends Field[Int]
case object DebugBaseID extends Field[Int]
case object DebugSubIDSize extends Field[Int]
case object DebugRouterBufferSize extends Field[Int]

trait HasDebugModuleParameters {
  implicit val p: Parameters
  val sysWordLength = 64          // system word length
  val regAddrWidth = 5            // 32 user registers
  val csrAddrWidth = 12           // address width of CSRs
  val csrCmdWidth = 3             // size of CSR commends
  val memOpSize = 5               // size of memory operations
  val ctmScoreBoardSize = p(DebugCtmScorBoardSize)
                                  // size of scoreboard in CTM, the same as L1 MISHS
  val stmUserRegAddr = 10         // the address of the user register for software trace
  val stmThreadPtrChgID = 0x8000  // the software trace id for register tracking
  val stmCsrAddr = p(DebugStmCsrAddr)
                                  // the CSR used for software trace
  val ctmID = p(DebugCtmID)       // the debug module ID of the core trace module
  val stmID = p(DebugStmID)       // the debug module ID of the software trace module
  val baseID = p(DebugBaseID)
                                  // the starting ID for rocket cores
  val subIDSize = 8               // the section size of each core
  val bufSize = p(DebugRouterBufferSize)
                                  // the size of buffer of the ring network
}

abstract class DebugModuleModule(coreid:Int)(rst:Bool = null)(implicit val p: Parameters)
    extends Module(_reset = rst) with HasDebugModuleParameters
abstract class DebugModuleBundle(implicit val p: Parameters)
    extends ParameterizedBundle()(p) with HasDebugModuleParameters

class DebugModuleIO(implicit p: Parameters) extends DebugModuleBundle()(p) {
  val net = new DiiIO
}

class DebugModuleBBoxIO(implicit p: Parameters) extends DebugModuleBundle()(p) {
  val net = new DiiBBoxIO
  net.dii_in.setName("debug_in")
  net.dii_in_ready.setName("debug_in_ready")
  net.dii_out.setName("debug_out")
  net.dii_out_ready.setName("debug_out_ready")

  val id = UInt(INPUT, width=10)
  id.setName("id")
}

class RocketDebugNetwork(coreid:Int)(rst:Bool = null)(implicit p: Parameters)
    extends DebugModuleModule(coreid)(rst)(p) {
  val io = new Bundle {
    val net = Vec(2, new DiiIO)
    val ctm = (new DiiIO).flip
    val stm = (new DiiIO).flip
  }

  def idAssign: Int => Int = _ match {
    case 0 => baseID + coreid*subIDSize + ctmID
    case 1 => baseID + coreid*subIDSize + stmID
  }

  val network = Module(new ExpandibleRingNetwork(idAssign, 2, bufSize))

  network.io.loc(0) <> io.ctm
  network.io.loc(1) <> io.stm
  network.io.net <> io.net
}
