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

case object DiiIOWidth extends Field[Int]

trait HasDiiParameters {
  implicit val p: Parameters
  val diiWidth = p(DiiIOWidth)
}

abstract class DiiModule(implicit val p: Parameters) extends Module
  with HasDiiParameters
abstract class DiiBundle(implicit val p: Parameters) extends ParameterizedBundle()(p)
  with HasDiiParameters

class DiiFlit(implicit p: Parameters) extends DiiBundle()(p) {
  val last = Bool()
  val data = UInt(width = diiWidth)
}

class DiiIO(implicit p:Parameters) extends ParameterizedBundle()(p) {
  val dii_out = Decoupled(new DiiFlit)
  val dii_in = Decoupled(new DiiFlit).flip
}

class DiiBBoxIO(implicit p:Parameters) extends DiiBundle()(p) {
  val dii_in = UInt(INPUT, width=(new DiiFlit).getWidth + 1)
  val dii_in_ready = Bool(OUTPUT)
  val dii_out = UInt(OUTPUT, width=(new DiiFlit).getWidth + 1)
  val dii_out_ready = Bool(INPUT)
}

class DiiBBoxPort(implicit p:Parameters) extends DiiModule()(p) {
  val io = new Bundle {
    val bbox = (new DiiBBoxIO)
    val chisel = (new DiiIO).flip
  }

  io.bbox.dii_out := Cat(io.chisel.dii_out.valid, io.chisel.dii_out.bits.toBits)
  io.chisel.dii_out.ready := io.bbox.dii_out_ready

  val w = (new DiiFlit).getWidth
  io.chisel.dii_in.valid := io.bbox.dii_in(w)
  io.chisel.dii_in.bits.last := io.bbox.dii_in(w-1)
  io.chisel.dii_in.bits.data := io.bbox.dii_in
  io.bbox.dii_in_ready := io.chisel.dii_in.ready
}

class DiiPort(implicit p:Parameters) extends DiiModule()(p) {
  val io = new Bundle {
    val chisel = (new DiiIO)
    val bbox = (new DiiBBoxIO).flip
  }

  io.bbox.dii_in := Cat(io.chisel.dii_in.valid, io.chisel.dii_in.bits.toBits)
  io.chisel.dii_in.ready := io.bbox.dii_in_ready

  val w = (new DiiFlit).getWidth
  io.chisel.dii_out.valid := io.bbox.dii_out(w)
  io.chisel.dii_out.bits.last := io.bbox.dii_out(w-1)
  io.chisel.dii_out.bits.data := io.bbox.dii_out
  io.bbox.dii_out_ready := io.chisel.dii_out.ready
}
