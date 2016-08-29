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

class SoftwareTrace(implicit p: Parameters) extends DebugModuleBundle()(p) {
  val id    = UInt(width=16)
  val value = UInt(width=sysWordLength)
}

class SoftwareTraceIO(implicit p: Parameters) extends DebugModuleBBoxIO()(p) {
  val trace = (new ValidIO(new SoftwareTrace)).flip
  trace.valid.setName("trace_valid")
  trace.bits.id.setName("trace_id")
  trace.bits.value.setName("trace_value")

  val tracking_enable = Bool(OUTPUT)
  tracking_enable.setName("trace_reg_enable")

  val tracking_addr = UInt(OUTPUT, width=regAddrWidth)
  tracking_addr.setName("trace_reg_addr")
}

// black box wrapper
class osd_stm(implicit val p: Parameters) extends BlackBox with HasDebugModuleParameters {
  val io = new SoftwareTraceIO

  setVerilogParameters("#(.REG_ADDR_WIDTH(" + regAddrWidth + "))")

  addClock(Driver.implicitClock)
  renameReset("rst")
}

class RocketSoftwareTraceIO(implicit p: Parameters) extends DebugModuleIO()(p) {
  val retire = Bool(INPUT)
  val reg_wdata = UInt(INPUT, width=sysWordLength)
  val reg_waddr = UInt(INPUT, width=regAddrWidth)
  val reg_wen = Bool(INPUT)
  val csr_wdata = UInt(INPUT, width=sysWordLength)
  val csr_waddr = UInt(INPUT, width=csrAddrWidth)
  val csr_wen = Bool(INPUT)
}

class RocketSoftwareTracer(coreid:Int, latch:Boolean = false)(rst:Bool = null)(implicit p: Parameters)
    extends DebugModuleModule(coreid)(rst)(p)
{
  val io = new RocketSoftwareTraceIO

  val tracer = Module(new osd_stm)
  val bbox_port = Module(new DiiPort)
  io.net <> bbox_port.io.chisel
  bbox_port.io.bbox <> tracer.io.net
  tracer.io.id := UInt(baseID + coreid*subIDSize + stmID)

  def input_latch[T <: Data](in:T):T = if(latch) RegNext(in) else in

  val retire      = input_latch(io.retire)
  val reg_wdata   = input_latch(io.reg_wdata)
  val reg_waddr   = input_latch(io.reg_waddr)
  val reg_wen     = input_latch(io.reg_wen)
  val csr_wdata   = input_latch(io.csr_wdata)
  val csr_waddr   = input_latch(io.csr_waddr)
  val csr_wen     = input_latch(io.csr_wen)

  val user_reg   = RegEnable(reg_wdata,
    retire && reg_wen && reg_waddr === UInt(stmUserRegAddr))

  val tracking_trigger = tracer.io.tracking_enable && retire && reg_wen && reg_waddr === tracer.io.tracking_addr
  val software_trigger = retire && csr_wen && csr_waddr === UInt(stmCsrAddr)
  tracer.io.trace.valid := tracking_trigger || software_trigger
  tracer.io.trace.bits.value := Mux(tracking_trigger, reg_wdata, user_reg)
  tracer.io.trace.bits.id := Mux(tracking_trigger, UInt(stmThreadPtrChgID), csr_wdata)
}
