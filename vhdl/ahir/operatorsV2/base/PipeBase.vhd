library ieee;
use ieee.std_logic_1164.all;

library ahir;
use ahir.Types.all;
use ahir.Subprograms.all;
use ahir.Utilities.all;
use ahir.BaseComponents.all;

--
-- base Pipe.
--  If full_rate is true, we will go for an implementation which
--  gives a throughput of one word/cycle.
--

entity PipeBase is
  generic (name : string;
	   num_reads: integer;
           num_writes: integer;
           data_width: integer;
           lifo_mode: boolean := false;
           depth: integer := 1;
	   signal_mode: boolean := false;
           shift_register_mode: boolean := false;
	   full_rate: boolean);
  port (
    read_req       : in  std_logic_vector(num_reads-1 downto 0);
    read_ack       : out std_logic_vector(num_reads-1 downto 0);
    read_data      : out std_logic_vector((num_reads*data_width)-1 downto 0);
    write_req       : in  std_logic_vector(num_writes-1 downto 0);
    write_ack       : out std_logic_vector(num_writes-1 downto 0);
    write_data      : in std_logic_vector((num_writes*data_width)-1 downto 0);
    clk, reset : in  std_logic);
  
end PipeBase;

architecture default_arch of PipeBase is

  signal pipe_data, pipe_data_repeated : std_logic_vector(data_width-1 downto 0);
  signal pipe_req, pipe_ack, pipe_req_repeated, pipe_ack_repeated: std_logic;
  signal signal_data : std_logic_vector(data_width-1 downto 0); 
  signal written_at_least_once: std_logic;
  
begin  -- default_arch


  manyWriters: if (num_writes > 1) generate
    wmux : OutputPortLevel generic map (
      name => name & "-wmux",
      num_reqs       => num_writes,
      data_width     => data_width,
      no_arbitration => false)
      port map (
        req   => write_req,
        ack   => write_ack,
        data  => write_data,
        oreq  => pipe_req,                -- no cross-over, drives req
        oack  => pipe_ack,                -- no cross-over, receives ack
        odata => pipe_data,
        clk   => clk,
        reset => reset);
  end generate manyWriters;

  singleWriter: if (num_writes = 1) generate
    pipe_req <= write_req(0);
    write_ack(0) <= pipe_ack;
    pipe_data <= write_data;
  end generate singleWriter;
 
  -- in signal mode, the pipe is just a flag
  SignalMode: if signal_mode generate

     -- write always succeeds.
     pipe_ack <= '1';
     process(clk,reset) 
     begin
	if(clk'event and clk = '1') then
		if(reset = '1') then
			signal_data <= (others => '0');	
  			written_at_least_once <= '0';
		else
			if(pipe_req = '1') then
				signal_data <= pipe_data;
  				written_at_least_once <= '1';
			end if;
		end if;
	end if;
     end process;

	-- read always succeeds, provided that it has been written
	-- into at least once.
     ReaderGen: for R in 0 to num_reads-1 generate
	read_ack(R) <= written_at_least_once;
	read_data(((R+1)*data_width)-1 downto (R*data_width)) <= signal_data;
     end generate ReaderGen;

  end generate SignalMode;

  Shallow: if (not signal_mode) and (depth < 3) and (not lifo_mode) generate

    singleBufferedFullRateCase: if((depth = 1) and full_rate) generate
       preg: PipelineRegister
		generic map (name => name & "-preg", 
				data_width => data_width)
		port map(
        		write_req   => pipe_req,
        		write_ack => pipe_ack,
        		write_data  => pipe_data,
        		read_req  => pipe_req_repeated,
        		read_ack  => pipe_ack_repeated,
        		read_data => pipe_data_repeated,
        		clk      => clk,
        		reset    => reset
			);
    end generate singleBufferedFullRateCase;

    notSingleBufferedOrFullRateCase: if ((not full_rate) or (depth /= 1)) generate
      queue : QueueBase generic map (	
        name => name & "-queue",	
        queue_depth => depth,
        data_width       => data_width)
        port map (
          push_req   => pipe_req,
          push_ack => pipe_ack,
          data_in  => pipe_data,
          pop_req  => pipe_req_repeated,
          pop_ack  => pipe_ack_repeated,
          data_out => pipe_data_repeated,
          clk      => clk,
          reset    => reset);
    end generate notSingleBufferedOrFullRateCase; 
  end generate Shallow;

  DeepFifo: if (not signal_mode) and (depth > 2) and (not lifo_mode) generate
    
   notShiftReg: if (not shift_register_mode) generate
    queue : SynchFifo generic map (
      name => name & "-queue", 
      queue_depth => depth,
      data_width       => data_width)
      port map (
        push_req   => pipe_req,
        push_ack => pipe_ack,
        data_in  => pipe_data,
        pop_req  => pipe_req_repeated,
        pop_ack  => pipe_ack_repeated,
        data_out => pipe_data_repeated,
        nearly_full => open,
        clk      => clk,
        reset    => reset);
   end generate notShiftReg;
    
   shiftReg: if shift_register_mode generate
      srqueue : ShiftRegisterQueue generic map (	
        name => name & "-srqueue",	
        queue_depth => depth,
        data_width       => data_width)
        port map (
          push_req   => pipe_req,
          push_ack => pipe_ack,
          data_in  => pipe_data,
          pop_req  => pipe_req_repeated,
          pop_ack  => pipe_ack_repeated,
          data_out => pipe_data_repeated,
          clk      => clk,
          reset    => reset);
   end generate shiftReg;
 
  end generate DeepFifo;

  Lifo: if (not signal_mode) and  lifo_mode generate
    stack : SynchLifo generic map (
      name => name & "-stack",
      queue_depth => depth,
      data_width       => data_width)
      port map (
        push_req   => pipe_req,
        push_ack => pipe_ack,
        data_in  => pipe_data,
        pop_req  => pipe_req_repeated,
        pop_ack  => pipe_ack_repeated,
        data_out => pipe_data_repeated,
        nearly_full => open,
        clk      => clk,
        reset    => reset);
  end generate Lifo;
  

  manyReaders: if  (not signal_mode) and (num_reads > 1) generate
    rmux : InputPortLevel generic map (
	name => name & "-rmux",
      num_reqs       => num_reads,
      data_width     => data_width,
      no_arbitration => false)
      port map (
        req => read_req,
        ack => read_ack,
        data => read_data,
        oreq => pipe_req_repeated,       
        oack => pipe_ack_repeated,       
        odata => pipe_data_repeated,
        clk => clk,
        reset => reset);
  end generate manyReaders;

  singleReader: if  (not signal_mode) and (num_reads = 1) generate
    read_ack(0) <= pipe_ack_repeated;
    pipe_req_repeated <= read_req(0);
    read_data <= pipe_data_repeated;
  end generate singleReader;
  
end default_arch;
