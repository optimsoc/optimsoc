# Measurement Infrastructure

Infrastructure for throughput measurement. Those modules can be used
to demonstrate glip on a target board.

## `glip_measure`

This is the base measurement module. It counts triggers for the
duration of one second and then samples and resets the counter. The
sampled values are output as decimal digits for display or processing.

Instantiation template:

    glip_measure
	#(.FREQ(),   // Your design frequency, no default
	  .DIGITS(), // Number of digits to sample, default 8
	  .OFFSET(), // Power of ten offset of the digits, useful if
	             // your display is limited, default 0
          .STEP()    // Counter increment on each trigger. Must be less
	             // than 16, default 1
     u_measure
      (
        .clk      (), // IN: Clock at frequency FREQ
        .rst      (), // IN: Reset
	.trigger  (), // IN: Trigger, gets counted as STEP on clk
        .digits   (), // OUT: Digits (width: DIGITS*4)
        .overflow (), // OUT: Shows overflow of digits
      );

## `glip_measure_sevensegment`

Measurement module that outputs the counted triggers as seven segment
output of the digits. For each digit you get seven bit which form the
seven segment digit as:

    +-0-+
    6   1
    +-7-+
    5   2
    +-3-+

Instantiation template

    glip_measure
	#(.FREQ(),   // Your design frequency, no default
	  .DIGITS(), // Number of digits to sample, default 8
	  .OFFSET(), // Power of ten offset of the digits, useful if
	             // your display is limited, default 0
          .STEP()    // Counter increment on each trigger, default 1
     u_measure
      (
        .clk      (), // IN: Clock at frequency FREQ
        .rst      (), // IN: Reset
	.trigger  (), // IN: Trigger, gets counted as STEP on clk
        .digits   (), // OUT: Digits (width: DIGITS*7)
        .overflow (), // OUT: Shows overflow of digits
      );
