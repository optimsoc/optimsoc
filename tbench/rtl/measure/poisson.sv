`ifndef POISSON_SV
`define POISSON_SV

/**
 * This class provides samples from a poisson distribution.
 */
class poisson;
   real       lambda = 5;
   real       pdf [];
   real       cdf [];
   integer    samples[];

   function integer sample();
      sample=samples[$urandom_range(99)];
   endfunction // sample

   function new(real mean=5);
      real sum;
      integer cur;

      lambda = mean;
      pdf = new[$ceil(lambda*5)];
      pdf[0] = $exp(-1*lambda);

      sum = pdf[0];
      for (int i=1;i<pdf.size();i++) begin
         pdf[i] = lambda/(i*1.0)*pdf[i-1];
         sum = sum+ pdf[i];
      end

      // Equalize because we cut off
      for (int i=0;i<pdf.size();i++) begin
         pdf[i] = pdf[i]/sum;
      end

      cdf = new[pdf.size()];
      cdf[0] = pdf[0];
      for (int i=1;i<cdf.size;i++) begin
         cdf[i] = cdf[i-1]+pdf[i];
      end

      samples = new[100];
      cur = 0;
      for (int i=0;i<100;i++) begin
         while (cdf[cur]*100<i) begin
            cur++;
         end
         samples[i] = cur;
      end
      for (int i=0;i<100;i++) begin
//         $display("%0d %0d",i,samples[i]);
      end
   endfunction
endclass // poisson

`endif