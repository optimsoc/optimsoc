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

class CoreTrace(implicit p: Parameters) extends DebugModuleBundle()(p) {
  val pc        = UInt(width=sysWordLength)
  val npc       = UInt(width=sysWordLength)
  val jal       = Bool()
  val jalr      = Bool()
  val br        = Bool()
  val load      = Bool()
  val store     = Bool()
  val trap      = Bool()
  val xcpt      = Bool()
  val csr       = Bool()
  val mem       = Bool()
  val taken     = Bool()
  val prv       = UInt(width=2)
  val addr      = UInt(width=sysWordLength)
  val rdata     = UInt(width=sysWordLength)
  val wdata     = UInt(width=sysWordLength)
  val time      = UInt(width=sysWordLength)
}

class CoreTraceIO(implicit p: Parameters) extends DebugModuleBBoxIO()(p) {
  val trace = (new ValidIO(new CoreTrace)).flip
  trace.valid.setName("trace_valid")
  trace.bits.pc.setName("trace_pc")
  trace.bits.npc.setName("trace_npc")
  trace.bits.jal.setName("trace_jal")
  trace.bits.jalr.setName("trace_jalr")
  trace.bits.br.setName("trace_branch")
  trace.bits.load.setName("trace_load")
  trace.bits.store.setName("trace_store")
  trace.bits.trap.setName("trace_trap")
  trace.bits.xcpt.setName("trace_xcpt")
  trace.bits.mem.setName("trace_mem")
  trace.bits.csr.setName("trace_csr")
  trace.bits.taken.setName("trace_br_taken")
  trace.bits.prv.setName("trace_prv")
  trace.bits.addr.setName("trace_addr")
  trace.bits.rdata.setName("trace_rdata")
  trace.bits.wdata.setName("trace_wdata")
  trace.bits.time.setName("trace_time")
}

// black box wrapper
class osd_ctm(implicit val p: Parameters) extends BlackBox with HasDebugModuleParameters {
  val io = new CoreTraceIO

  addClock(Driver.implicitClock)
  renameReset("rst")
}

class RocketCoreTraceIO(implicit p: Parameters) extends DebugModuleIO()(p) {
  val wb_valid = Bool(INPUT)
  val wb_pc = UInt(INPUT, width=sysWordLength)
  val wb_wdata = UInt(INPUT, width=sysWordLength)
  val wb_jal = Bool(INPUT)
  val wb_jalr = Bool(INPUT)
  val wb_br = Bool(INPUT)
  val wb_mem = Bool(INPUT)
  val wb_mem_cmd = UInt(INPUT, width=memOpSize)
  val wb_xcpt = Bool(INPUT)
  val wb_csr = Bool(INPUT)
  val wb_csr_cmd = UInt(INPUT, width=csrCmdWidth)
  val wb_csr_addr = UInt(INPUT, width=csrAddrWidth)

  val mem_br_taken = Bool(INPUT)
  val mem_npc = UInt(INPUT, width=sysWordLength)

  val csr_eret = Bool(INPUT)
  val csr_xcpt = Bool(INPUT)
  val csr_prv = UInt(INPUT, width=2)
  val csr_wdata = UInt(INPUT, width=sysWordLength)
  val csr_evec = UInt(INPUT, width=sysWordLength)
  val csr_time = UInt(INPUT, width=sysWordLength)

  val dmem_has_data = Bool(INPUT)
  val dmem_replay = Bool(INPUT)
  val dmem_rdata = UInt(INPUT, width=sysWordLength)
  val dmem_wdata = UInt(INPUT, width=sysWordLength)
  val dmem_addr = UInt(INPUT, width=sysWordLength)

}

class RocketCoreTracer(coreid:Int,
  isRead:UInt => Bool,
  isWrite:UInt => Bool,
  isCsrRead:(UInt, UInt) => Bool,
  isCsrWrite:(UInt, UInt) => Bool,
  isCsrTrap:(UInt, UInt) => Bool)
  (rst:Bool = null)
  (implicit p: Parameters)
    extends DebugModuleModule(coreid)(rst)(p)
{
  val io = new RocketCoreTraceIO

  val tracer = Module(new osd_ctm)
  val trace = tracer.io.trace.bits
  val bbox_port = Module(new DiiPort)
  io.net <> bbox_port.io.chisel
  bbox_port.io.bbox <> tracer.io.net
  tracer.io.id := UInt(baseID + coreid*subIDSize + ctmID)
  tracer.io.trace.valid := Bool(false)

  def input_latch[T <: Data](in:T):T = RegNext(in)

  val wb_valid        = input_latch(io.wb_valid)
  val wb_pc           = input_latch(io.wb_pc)
  val wb_wdata        = input_latch(io.wb_wdata)
  val wb_jal          = input_latch(io.wb_jal)
  val wb_jalr         = input_latch(io.wb_jalr)
  val wb_br           = input_latch(io.wb_br)
  val wb_mem          = input_latch(io.wb_mem)
  val wb_mem_cmd      = input_latch(io.wb_mem_cmd)
  val wb_xcpt         = input_latch(io.wb_xcpt)
  val wb_csr          = input_latch(io.wb_csr)
  val wb_csr_cmd      = input_latch(io.wb_csr_cmd)
  val wb_csr_addr     = input_latch(io.wb_csr_addr)
  val wb_br_taken     = RegNext(input_latch(io.mem_br_taken))
  val wb_xcpt_npc     = input_latch(io.csr_evec)
  val wb_br_npc       = RegNext(input_latch(io.mem_npc))

  val csr_eret        = input_latch(io.csr_eret)
  val csr_xcpt        = input_latch(io.csr_xcpt)
  val csr_prv         = io.csr_prv // updated prv is only available the next cycle
  val csr_wdata       = input_latch(io.csr_wdata)
  val csr_time        = input_latch(io.csr_time)

  val dmem_has_data   = input_latch(io.dmem_has_data)
  val dmem_replay     = input_latch(io.dmem_replay)
  val dmem_rdata      = input_latch(io.dmem_rdata)
  val dmem_wdata      = input_latch(io.dmem_wdata)
  val dmem_addr       = input_latch(io.dmem_addr)

  val retire_event = wb_valid && (wb_jal || wb_jalr || wb_br || wb_xcpt || wb_mem && (dmem_has_data || !isRead(wb_mem_cmd)) || wb_csr) || csr_xcpt || csr_eret

  tracer.io.trace.valid := retire_event
  trace.pc := wb_pc
  trace.npc := Mux(wb_xcpt || csr_xcpt || csr_eret, wb_xcpt_npc, wb_br_npc)
  trace.jal := wb_jal
  trace.jalr := wb_jalr
  trace.br := wb_br
  trace.load := wb_mem && isRead(wb_mem_cmd) || wb_csr && isCsrRead(wb_csr_cmd, wb_csr_addr)
  trace.store := wb_mem && isWrite(wb_mem_cmd) || wb_csr && isCsrWrite(wb_csr_cmd, wb_csr_addr)
  trace.trap := csr_xcpt && isCsrTrap(wb_csr_cmd, wb_csr_addr)
  trace.xcpt := wb_xcpt || csr_xcpt && !isCsrTrap(wb_csr_cmd, wb_csr_addr)
  trace.csr := wb_csr
  trace.mem := wb_mem
  trace.taken := wb_br_taken
  trace.prv := csr_prv
  trace.addr := dmem_addr
  trace.rdata := Mux(wb_mem, dmem_rdata, wb_wdata)
  trace.wdata := Mux(wb_mem, dmem_wdata, csr_wdata)
  trace.time := csr_time

  // handle cache miss
  val sb = new Scoreboard(ctmScoreBoardSize)

  when(wb_valid && wb_mem && !dmem_has_data) { // non-blocked cache miss
    sb.add(dmem_addr, csr_time)
  }

  when(dmem_replay && dmem_has_data) { // dcache replay
    sb.replay(dmem_addr, dmem_rdata)
  }

  when(!retire_event && sb.fire) {
    tracer.io.trace.valid := Bool(true)
    trace.load := Bool(true)
    trace.mem := Bool(true)
    trace.addr := sb.addr
    trace.rdata := sb.data
    trace.time := sb.time
    sb.clear
  }

  class Scoreboard(n:Int) {
    private val availQ = Reg(init = Bits(1,n))
    private val fillQ = Reg(init = Bits(0,n))
    private val fireQ = ~availQ & fillQ
    private val addrQ = Reg(Vec(n, UInt(width=sysWordLength)))
    private val timeQ = Reg(Vec(n, UInt(width=sysWordLength)))
    private val dataQ = Reg(Vec(n, UInt(width=sysWordLength)))

    private val add_index = PriorityEncoder(availQ)
    private val fire_index = PriorityEncoder(fireQ)

    val fire = orR(fireQ)
    val addr = addrQ(fire_index)
    val data = dataQ(fire_index)
    val time = timeQ(fire_index)

    def add(a:UInt, t:UInt) = {
      availQ(add_index) := Bool(false)
      fillQ(add_index) := Bool(false)
      addrQ(add_index) := a
      timeQ(add_index) := t
    }
    def replay(a:UInt, d:UInt) = {
     addrQ.zipWithIndex.zip(dataQ).map{ case((addr, i), data) => {
        when(addr === a) {
          data := d
          fillQ(i) := Bool(true)
        }
      }}
    }
    def clear = {
      availQ(fire_index) := Bool(true)
    }

  }
}
