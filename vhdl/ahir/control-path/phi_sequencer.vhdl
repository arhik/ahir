-- phi-sequencer..
-- written by Madhav P. Desai, December 2012.
library ieee;
use ieee.std_logic_1164.all;
library ahir;
use ahir.Types.all;
use ahir.subprograms.all;
use ahir.BaseComponents.all;


entity phi_sequencer  is
  generic (nreqs : integer; nreenables : integer);
  port (
  selects : in BooleanArray(0 to nreqs-1); -- one out of nreqs..
  reqs : out BooleanArray(0 to nreqs-1); -- one out of nreqs
  ack  : in Boolean; 
  reenables: in BooleanArray(0 to nreenables-1);   -- all need to arrive to reenable
  done : out Boolean;
  clk, reset: in std_logic);
end phi_sequencer;


-- on reset, wait for a transition on any of the in_places.
-- the corresponding req is asserted..  A reenable place
-- is also present to allow the reenabling of the sequence.
architecture Behave of phi_sequencer is
  signal select_token, select_clear : BooleanArray(0 to nreqs-1);
  signal reenable_token, reenable_clear : BooleanArray(0 to nreenables-1);

  signal enabled, ack_token, ack_clear, req_fired: Boolean;
begin  -- Behave

  -- instantiate unmarked places for the in_places.
  InPlaces: for I in 0 to nreqs-1 generate
    placeBlock: block
	signal place_pred, place_succ: BooleanArray(0 downto 0);
    begin
	place_pred(0) <= selects(I);
	place_succ(0) <= select_clear(I);
	pI: place generic map(capacity => 1, marking => 0)
		port map(place_pred,place_succ,select_token(I),clk,reset);
    end block;
  end generate InPlaces;

  -- place for reenable: places are unmarked.. initial state
  -- should be consistently generated by the instantiator.
  ReenablePlaces: for J in 0 to nreenables-1 generate
    rnb_block: block
      signal place_pred, place_succ: BooleanArray(0 downto 0);    
    begin
      place_pred(0) <= reenables(J);
      place_succ(0) <= reenable_clear(J);
      pRnb: place generic map(capacity => 1, marking => 1)
        port map(place_pred,place_succ,reenable_token(J),clk,reset);    
    end block;
  end generate ReenablePlaces;  
    
 
  -- sequencer is enabled by this sig.
  enabled <= AndReduce(reenable_token) and ack_token;

  -- a marker to indicate that a req has been fired.
  req_fired <= OrReduce(select_token) and enabled;

  -- outgoing reqs, etc.
  reqs <= select_token when enabled else (others => false);
  select_clear <= select_token when enabled else (others => false);
  reenable_clear <= (others => true) when (enabled and req_fired)
                          else (others => false);

  -- ack should be received to reenable the sequencer.
  -- this place is initially marked (it is internal
  -- to the sequencer).
  ack_clear <= req_fired;
  ack_block: block
      signal place_pred: BooleanArray(0 downto 0);    
      signal place_succ: BooleanArray(0 downto 0);    
  begin
      place_pred(0) <= ack;
      place_succ(0) <= ack_clear;
      pack: place generic map(capacity => 1, marking => 1)
        port map(place_pred,place_succ,ack_token,clk,reset);    
  end block;
  
  -- outgoing exit.. is the incoming ack..
  done <= ack;

end Behave;