// ----------------------------------------------------------------------------------
// Company: 
// Engineer: 
//
// Create Date: 11/29/2016 05:37:28 PM
// Design Name: 
// Module Name: FanControl - Behavioral
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
//
// Dependencies: 
//
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
//
// ----------------------------------------------------------------------------------

// fancontrol simple version, just overheating protection
module fancontrol
(
   input		clk,
   input		reset_n,
   input		user_temp_alarm_in,
   input		ot_in,			
   input[9:0]	temp_in,
   input		SM_FAN_TACH,
   output reg	SM_FAN_PWM);

always @(user_temp_alarm_in, ot_in)
begin
	if (user_temp_alarm_in == 1 || ot_in == 1)
	begin
		SM_FAN_PWM <= 1'b1;
	end
	else
	begin
		SM_FAN_PWM <= 1'b0;
	end
end
endmodule

// fancontrol with fan speed control
// following code to be verified
/*module fancontrol
(
    input clk;
    input reset_n;
    input[15:0] temp_in;
    input SM_FAN_TACH;
    
    input temp_alm;
    input ot_alm;
    
    output SM_FAN_PWM; 
    output[15:0] temp_out;
    output[15:0] pid_error;
    
    --declared as OP only for testing PWM
    output[15:0] x_duty_counter;
    output[15:0] x_duty_select);
end FanControl;

reg[15:0] s_duty_counter       // UNSIGNED(15 downto 0);
reg[15:0] s_duty_select        // UNSIGNED(15 downto 0);
reg[15:0] s_temp_in_unsigned   // UNSIGNED(15 downto 0);
reg[15:0] s_pid_error          // SIGNED(15 downto 0); 
reg[26:0] s_check_counter      //: integer range 0 to 100 000 000;
reg[15:0] s_prev_pid_error     //: SIGNED(15 downto 0);

--updating OPs
x_duty_counter <= s_duty_counter;
x_duty_select <= s_duty_select;
pid_error <= s_pid_error;
s_temp_in_unsigned <= unsigned(temp_in);
temp_out <= s_temp_in_unsigned;

//prev value saved with a delay of clock cycle for comparison
s_prev_pid_error <= s_pid_error when rising_edge(clk);

--process to compute error
process(clk, reset_n)
begin
    if reset_n = '0' then
        s_pid_error <= to_signed(0,16);
    elsif rising_edge(clk) then
        -- 40200 = 34C
        s_pid_error <= to_signed(to_integer(s_temp_in_unsigned) - 40200,16);
    end if;    
end process;


--process to vary PWM counter variable
process (clk, reset_n)
begin
    if reset_n = '0' then
        s_duty_select <= to_unsigned(0,16);
        s_check_counter <= 0;
    elsif rising_edge(clk) then
        
        --checking once ever 0.5 sec on a 100Mhz clock
        if s_check_counter = 50000000 then
            s_check_counter <= 0;
            
            --600 is almost a 5C change
            if abs(s_pid_error) < 600 then
                if s_prev_pid_error > s_pid_error then
                    if s_duty_select > to_unsigned(500,16) then
                        s_duty_select <= s_duty_select - to_unsigned(300,16);
                    end if;
                else
                    if s_duty_select <  to_unsigned(9000,16) then
                        s_duty_select <= s_duty_select + to_unsigned(200,16);
                    end if;
                end if;
            elsif s_pid_error > 600 then
                if s_duty_select <  to_unsigned(9000,16) then
                    s_duty_select <= s_duty_select + to_unsigned(400,16);
                end if;
            elsif s_pid_error < -600 then
                if s_duty_select > to_unsigned(500,16) then
                    s_duty_select <= s_duty_select - to_unsigned(400,16);
                end if;
            end if;
            
        
        else
            s_check_counter <= s_check_counter + 1;
        end if;
        
          
    end if;
end process;

// process to generate PWM signal
always @(posedge clk)
begin
	if (reset_n == 0)
	begin
		s_duty_counter <= 10000;
	end
    else
	begin
	    if (temp_alm == 0 && ot_alm == 0)
		begin
	        if (s_duty_counter > s_duty_select)
			begin
	            SM_FAN_PWM <= '0';
			end
	        else
			begin
	            SM_FAN_PWM <= '1';
			end
        
        	if (s_duty_counter < 10000)
			begin
        	    s_duty_counter <= s_duty_counter + 1;
			end
        	else
			begin
            	//initialize PWM signal with a duty cycle of atleast 20%
            	s_duty_counter <= 2000;
			end
        end
    	else
		begin
        	SM_FAN_PWM <= '1';
		end
	end
end
*/








