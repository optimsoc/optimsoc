
package dii_package;

   typedef struct packed unsigned {
      logic       valid;
      logic       last;
      logic [15:0] data;
   } dii_flit;

   function dii_flit
     dii_flit_assemble(
                       input logic        m_valid,
                       input logic        m_last,
                       input logic [15:0] m_data                       
                       );
      return dii_flit'{m_valid, m_last, m_data};
   endfunction      

endpackage // dii_package

