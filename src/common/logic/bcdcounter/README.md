# BCD Counter

A simple BCD counter. It sums up the `carry_in` values up to ten and
generates `carry_out` on overflow. `carry_in` can have take values up
to ten, `carry_out` is a single digit. You can cascade then by
connecting the carry output of one to the LSB of the carry input of
the next.



