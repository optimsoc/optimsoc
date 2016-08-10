@defgroup backend_uart-protocol UART Protocol
@ingroup backend_uart

Standard UART does not have any out-of-band signaling, meaning we
cannot use control signals. The glip interface requires a reset signal
to reset the logic without board interaction. This signal needs to be
controlled by in-band signaling.

In-Band Signaling
-----------------

UART is one byte per transfer and we obviously want to use the full
byte in the default case. Hence one word is defined as a special
word. When this word needs to be transmitted, it needs to be
transferred twice. If the second word is not the same, that is a
control message.  We use `0xfe` as `0x00` and `0xff` along with lower
numbers are safely assumed to be in the stream at a higher rate.

Example user data stream:

    0xaf 0xfe 0x00 0xff

UART data stream:

    0xaf 0xfe 0xfe 0x00 0xff 

Credit-based Flow Control
-------------------------

The reset can happen at anytime, and especially when the input FIFO is
full as the user logic has deadlocked or similar. The problem now is
how to transfer a control word into the device when the input is
blocked. Hence, we need credit-based flow control, where the host gets
clearance from the device to send a certain amount of data. With each
transfer the host loses one of the credits and it needs to wait for
new credit from the device. This credit also needs to be transfered
in-band, meaning that we apply the same in-band signaling scheme
described above.

One may think it is safe to not implement flow control in the other
direction (logic->host), but to have a robust it is also required to
implement credit-based flow control there. In glip we don't make any
assumptions about when data is read and how input and output data
relate. Hence the back-pressure from the input may block the credit
messages and a deadlock in the user code may occur.

Summarizing, we need credit-based flow control in both directions.

Protocol Datagrams
------------------

From logic to host we have the following protocol datagrams:

| word 0             | word 1        | Description   |
|--------------------|---------------|---------------|
| `{credit[14:8],1}` | `credit[7:0]` | Credit update | 

From host to logic we have the following protocol datagrams:

| word 0               | word 1        | Description              |
|----------------------|---------------|--------------------------|
| `{0,credit[13:8],1}` | `credit[7:0]` | Credit update            |
| `100000r1`           | -             | Set logic rst pin to `r` |
| `100001r1`           | -             | Set comm rst pin to `r`  |

The credit datagrams are exchanged as follows:

For the *ingress* path (host to logic), the credit is sent right after
reset of the communication. This reset is usually a board reset, but
most importantly when connecting from the host. The communication
controller then observes the data stream and updates the credit each
time it falls below the 50% threshold. This gives the host sufficient
time to actually receive this message before running out of credits,
and on the other hand does not imply too many messages.

For the *egress* path (logic to host), the host gives all initial
credit in a few tranches to the FPGA. It then also observes the
incoming data and sends a new credit of the maximum number that can be
transferred in one datagram (`0x3fff`) once the remaining credit falls
below the threshold of `HOST_BUFFER_SIZE - 0x3fff`.

The rst pin is set on demand by the user application. The
communications reset pin is used by the communication controller to
reset itself to a defined state.
