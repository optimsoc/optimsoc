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
