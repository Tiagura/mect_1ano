library ieee;
use ieee.std_logic_1164.all;

entity HA_1bit is
  port (a, b: in std_logic;
    sum, carry_out: out std_logic); 
  end HA_1bit;

architecture dataflow of HA_1bit is 
  begin
  sum <= a xor b;
  carry_out <= a and b; 
end dataflow;