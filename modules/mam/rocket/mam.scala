package open_soc_debug

import Chisel._
import cde.{Parameters, Field}
import junctions.ParameterizedBundle

case object MamIODataWidth extends Field[Int]
case object MamIOAddrWidth extends Field[Int]
case object MamIOBeatsBits extends Field[Int]

trait HasMamParameters {
  implicit val p: Parameters
  val mamBits = p(MamIODataWidth)
  val mamBytes = mamBits / 8
  val mamAddrBits = p(MamIOAddrWidth)
  val mamBeatsBits = p(MamIOBeatsBits)
  val mamBytesBits = mamBeatsBits + log2Up(mamBytes)
  require(isPow2(mamBits))
}

abstract class MamModule(implicit val p: Parameters) extends Module
  with HasMamParameters
abstract class MamBundle(implicit val p: Parameters) extends ParameterizedBundle()(p)
  with HasMamParameters

class MamReq(implicit p: Parameters) extends MamBundle()(p) {
  val rw = Bool() // 0: Read, 1: Write
  val addr = UInt(width = mamAddrBits)
  val burst = Bool() // 0: single, 1: incremental burst
  val beats = UInt(width = mamBeatsBits)
}

class MamData(implicit p: Parameters) extends MamBundle()(p) {
  val data = UInt(width = mamBits)
}

class MamWData(implicit p: Parameters) extends MamData()(p) {
  val strb = UInt(width = mamBytes)
}

class MamRData(implicit p: Parameters) extends MamData()(p)

class MamIOReqChannel(implicit p: Parameters) extends MamBundle()(p) {
  val req = Decoupled(new MamReq)
}

class MamIO(implicit p: Parameters) extends MamIOReqChannel()(p) {
  val wdata = Decoupled(new MamWData)
  val rdata = Decoupled(new MamRData).flip
}
