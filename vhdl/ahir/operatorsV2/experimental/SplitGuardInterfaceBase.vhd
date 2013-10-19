library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library ahir;
use ahir.Types.all;
use ahir.Subprograms.all;
use ahir.Utilities.all;
use ahir.BaseComponents.all;


-- a guard-interface to conform with the split protocol.
-- With this interface, the guard at the time of initiation
-- of the operation will be remembered (using a queue) 
-- and the remembered value will be used to generate the
-- completion protocol.
-- 
-- The benefit is that the guard-expression can be reevaluated
-- as soon as the operation starts (instead of waiting
-- for the operation to finish...).  This helps pipelining.
-- TODO: QueueBase can be replaced with a simpler shift-stage?
--       (maybe not.., because this slows down the guard=0 case).
entity SplitGuardInterfaceBase is
	generic (buffering:integer);
	port (sr_in: in Boolean;
	      sa_out: out Boolean;
	      sr_out: out Boolean;
	      sa_in: in Boolean;
	      cr_in: in Boolean;
	      ca_out: out Boolean;
	      cr_out: out Boolean;
	      ca_in: in Boolean;
	      guard_interface: in std_logic;
	      clk: in std_logic;
	      reset: in std_logic);
end entity;


architecture Behave of SplitGuardInterfaceBase is
  signal push, push_ack, pop, pop_ack: std_logic;
  signal qdata_in, qdata : std_logic_vector(0 downto 0);

  type LhsState is (l_Idle, l_Wait_On_Ack_In, l_Wait_On_Queue);
  signal lhs_state : LhsState;

  type RhsState is (r_Idle, r_Wait_On_Ack_In, r_Wait_On_Queue);
  signal rhs_state: RhsState;
begin
	qdata_in(0) <= guard_interface;

	qI: QueueBase
		generic map(queue_depth => buffering, data_width => 1)
		port map(clk => clk, reset => reset,
				data_in => qdata_in,
				push_req => push,
				push_ack => push_ack,
				data_out => qdata,
				pop_req => pop,
				pop_ack => pop_ack);

	-- LHS state machine.
        -- 
	------------------------------------------------------------------------------------------
        --   Present-state  sr_in  push_ack guard_interface sa_in    Nstate  sr_out  sa_out  push
	------------------------------------------------------------------------------------------
        --     l_Idle        0        _          _            _      l_Idle
 	--     l_Idle        1        1          0            _      l_Idle            1      1
	--     l_Idle        1        1          1            0      W-ack-in  1              1
	--     l_Idle        1        1          1            1      l_Idle    1       1      1
	--     l_Idle        1        0          _            _      W-Queue   
	--     W-Queue       _        0          _            _      W-Queue
	--     W-Queue       _        1          1            1      l_Idle    1       1      1
	--     W-Queue       _        1          1            0      W-ack-in  1              1
	--     W-Queue       _        1          0            _      l_Idle            1      1
	--     W-Ack-In      _        _          _            0      W-ack-in
	--     W-Ack-In      _        _          _            1      l_Idle            1
	------------------------------------------------------------------------------------------
	process(clk, sr_in, push_ack, guard_interface, sa_in, lhs_state, reset)
		variable nstate : LhsState;
	begin
		nstate 	:= lhs_state;
		sr_out 	<= false;
		sa_out 	<= false;
		push	<= '0';

		case lhs_state is
			when l_Idle => 
				if sr_in then
					if((push_ack = '1') and (guard_interface = '0')) then
						sa_out <= true;
						push   <= '1';
					elsif ((push_ack = '1') and (guard_interface = '1')) then
						if sa_in then
							sr_out 	<= true;
							sa_out 	<= true;
							push 	<= '1';
						else	
							nstate 	:= l_Wait_On_Ack_In;
							sr_out 	<= true;
							push 	<= '1';
						end if;
					elsif (push_ack = '0') then
						nstate := l_Wait_On_Queue;
					end if;
				end if;
			when l_Wait_On_Queue => 
				if(push_ack  = '1') then
					if((guard_interface = '1') and sa_in) then
						nstate := l_Wait_On_Ack_In;
						sr_out <= true;
						push <= '1';
					elsif (guard_interface = '0') then
						nstate := l_Idle;
						sa_out <= true;
						push <= '1';
					end if;
				end if;
			when l_Wait_On_Ack_In => 
				if sa_in then
					nstate := l_Idle;
					sa_out <= true;
				end if;
		end case;

		if(clk'event and clk = '1') then
			if(reset = '1') then
				lhs_state <= l_Idle;
			else
				lhs_state <= nstate;
			end if;
		end if;
	end process;


	-- RHS State machine.
	------------------------------------------------------------------------------------------
        --   Present-state  cr_in  pop_ack      qdata        ca_in    Nstate  cr_out  ca_out  pop
	------------------------------------------------------------------------------------------
	--   r_Idle          0        _           _            _      r_Idle
	--   r_Idle          1        0           _            1       ERROR.
	--   r_Idle          1        0           _            0      W-Queue
	--   r_Idle          1        1           1            0      W-Ack-In  1              1
	--   r_Idle          1        1           0            _      r_Idle           1       1
	--   r_Idle          1        1           1            1      r_Idle    1      1       1
	--   W-Queue         _        0           _            _      W-Queue
	--   W-Queue         _        1           1            0      W-Ack-In  1              1
	--   W-Queue         _        1           0            _      r_Idle           1       1
	--   W-Ack-In        _        _           _            0      W-Ack-In  
	--   W-Ack-In        _        _           _            1      r_Idle           1 
	process(clk,cr_in,pop_ack,qdata,ca_in,rhs_state,reset)
		variable nstate : RhsState;
		variable ca_out_var : Boolean;
	begin
		nstate := rhs_state;
		pop <= '0';
		cr_out <= false;
		ca_out_var := false;

		case rhs_state is
			when r_Idle =>
				if cr_in then
					if(pop_ack = '0') then
						if ca_in then
							assert false report "ERROR: invalid RHS state transition." severity error;
							nstate := r_Wait_On_Queue;
						else
							nstate := r_Wait_On_Queue;			
						end if;	
					else
						if((qdata(0) = '1') and (not ca_in)) then
							nstate := r_Wait_On_Ack_In;
							cr_out <= true;
							pop <= '1';
						elsif(qdata(0) = '0') then
							nstate := r_Idle;
							ca_out_var := true;
							pop <= '1';
						elsif((qdata(0) = '1') and ca_in) then
							nstate := r_Idle;
							cr_out <= true;
							ca_out_var := true;
							pop <= '1';
						end if;
					end if;
				end if;
			when r_Wait_On_Queue =>
				if(pop_ack = '1') then
					if((qdata(0) = '1') and (not ca_in)) then
						nstate := r_Wait_On_Ack_In;
						cr_out <= true;
						pop <= '1';
					elsif (qdata(0) = '0') then
						nstate := r_Idle;
						ca_out_var := true;
						pop <= '1';
					end if;
				end if;
			when r_Wait_On_Ack_In =>
				if(ca_in) then 
					nstate := r_Idle;
					ca_out_var := true;
				end if;
		end case;

		if(clk'event and clk = '1') then
			if(reset = '1') then
				rhs_state <= r_Idle;
				ca_out <= false;
			else
				-- single cycle delay guaranteed between
				-- cr_in and ca_out.
				ca_out <= ca_out_var;
				rhs_state <= nstate;
			end if;
		end if;
	end process;

end Behave;