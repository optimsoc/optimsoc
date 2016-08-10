
interface dii_channel_flat
  #(parameter N = 'x);
   logic [N*16-1:0] data;
   logic [N-1:0]    first;
   logic [N-1:0]    last;
   logic [N-1:0]    valid;
   logic [N-1:0]    ready;

endinterface // dii_channel_flat
