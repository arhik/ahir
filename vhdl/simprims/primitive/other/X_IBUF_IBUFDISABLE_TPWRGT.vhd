-------------------------------------------------------------------------------
-- Copyright (c) 1995/2010 Xilinx, Inc.
-- All Right Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /    Vendor : Xilinx
-- \   \   \/     Version : 11.1
--  \   \         Description : Xilinx Timing Simulation Library Component
--  /   /                  Input Buffer
-- /___/   /\     Filename : X_IBUF_IBUFDISABLE_TPWRGT.vhd
-- \   \  /  \    Timestamp : Mon Oct 31 16:03:06 PDT 2011
--  \___\/\___\
--
-- Revision:
--    10/31/11 - Initial version -- Simprim only (for tristate powergating)
-- End Revision

library IEEE;
use IEEE.STD_LOGIC_1164.all;

library IEEE;
use IEEE.Vital_Primitives.all;
use IEEE.Vital_Timing.all;

entity X_IBUF_IBUFDISABLE_TPWRGT is
  generic(
    IBUF_LOW_PWR : string :=  "TRUE";
    IOSTANDARD  : string := "DEFAULT";
    USE_IBUFDISABLE : string :=  "TRUE";

    Xon   : boolean := true;
    MsgOn : boolean := true;
    LOC            : string  := "UNPLACED";

    tipd_I : VitalDelayType01 := (0.000 ns, 0.000 ns);
    tpd_I_O : VitalDelayType01 := (0.000 ns, 0.000 ns);
    tipd_IBUFDISABLE : VitalDelayType01 := (0.000 ns, 0.000 ns);
    tpd_IBUFDISABLE_O : VitalDelayType01 := (0.000 ns, 0.000 ns);
    tipd_TPWRGT  : VitalDelayType01 := (0.000 ns, 0.000 ns);
    tpd_TPWRGT_O : VitalDelayType01 := (0.000 ns, 0.000 ns);
    PATHPULSE : time := 0 ps

    );

  port(
    O              : out std_ulogic;

    I              : in std_ulogic;
    IBUFDISABLE    : in std_ulogic;
    TPWRGT         : in std_ulogic
    );

  attribute VITAL_LEVEL0 of
    X_IBUF_IBUFDISABLE_TPWRGT : entity is true;

end X_IBUF_IBUFDISABLE_TPWRGT;

architecture X_IBUF_IBUFDISABLE_TPWRGT_V of X_IBUF_IBUFDISABLE_TPWRGT is

  attribute VITAL_LEVEL0 of
    X_IBUF_IBUFDISABLE_TPWRGT_V : architecture is true;

  signal I_ipd		    : std_ulogic := 'X';
  signal IBUFDISABLE_ipd    : std_ulogic := 'X';
  signal TPWRGT_ipd         : std_ulogic := 'X';

begin

  WireDelay    : block
  begin
    VitalWireDelay (I_ipd,            I,            tipd_I);
    VitalWireDelay (IBUFDISABLE_ipd,  IBUFDISABLE,  tipd_IBUFDISABLE);
    VitalWireDelay (TPWRGT_ipd,       TPWRGT,       tipd_TPWRGT);
  end block;
    
  prcs_init : process
  variable FIRST_TIME        : boolean    := TRUE;
  begin
     
     If(FIRST_TIME = true) then

        if((IBUF_LOW_PWR = "TRUE") or
           (IBUF_LOW_PWR = "FALSE")) then
           FIRST_TIME := false;
        else
           assert false
           report "Attribute Syntax Error: The Legal values for IBUF_LOW_PWR are TRUE or FALSE"
           severity Failure;
        end if;

--  

        if((USE_IBUFDISABLE = "TRUE") or
           (USE_IBUFDISABLE = "FALSE")) then
           FIRST_TIME := false;
        else
           assert false
           report "Attribute Syntax Error: The Legal values for USE_IBUFDISABLE are TRUE or FALSE"
           severity Failure;
        end if;

     end if;

     wait; 
  end process prcs_init;
    

  prcs_timing : process
     variable O_zd         : std_ulogic;
     variable O_GlitchData : VitalGlitchDataType;
     variable TPWRGT_OR_IBUFDISABLE   : std_ulogic := '0';

  begin 
     if(USE_IBUFDISABLE = "TRUE") then

        TPWRGT_OR_IBUFDISABLE := IBUFDISABLE_ipd OR (not TPWRGT_ipd);

        if(TPWRGT_OR_IBUFDISABLE = '1') then
           O_zd  := '1';
        elsif (TPWRGT_OR_IBUFDISABLE = '0') then
           O_zd  := TO_X01(I_ipd);
        end if;
     elsif(USE_IBUFDISABLE = "FALSE") then
           O_zd  := TO_X01(I_ipd);
     end if;

     VitalPathDelay01 (
       OutSignal     => O,
       GlitchData    => O_GlitchData,
       OutSignalName => "O",
       OutTemp       => O_zd,
       Paths         => (0 => (I_ipd'last_event,           tpd_I_O,           true),
                         1 => (IBUFDISABLE_ipd'last_event, tpd_IBUFDISABLE_O, true),
                         2 => (TPWRGT_ipd'last_event,      tpd_TPWRGT_O,      true)),
       Mode          => VitalTransport,
       Xon           => Xon,
       MsgOn         => MsgOn,
       MsgSeverity   => warning);


     wait on I_ipd, IBUFDISABLE_ipd, TPWRGT_ipd;
  end process;


end X_IBUF_IBUFDISABLE_TPWRGT_V;
