library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity semaforo is 
	port (clk, rst: in std_logic;
			red, green, yellow, buzzer: out std_logic;
			disp : out std_logic_vector(3 downto 0);
			seg  : out std_logic_vector(6 downto 0));
end semaforo;

architecture fsm of semaforo is
	
	type estados is (vermelho, verde, amarelo);
	signal pr_state, nx_state : estados;
	
   constant clk_freq : integer := 50_000_000;  
   constant tempo_5s : unsigned(31 downto 0) := to_unsigned(250_000_000, 32);
	signal conta_5s : unsigned(31 downto 0) := (others => '0');
	signal mux_count : unsigned(1 downto 0) := (others => '0');
	
	constant seg_V : std_logic_vector(6 downto 0) := "1000001";
	constant seg_A : std_logic_vector(6 downto 0) := "0001000";
	constant seg_I : std_logic_vector(6 downto 0) := "1111001";
	constant seg_P : std_logic_vector(6 downto 0) := "0011000";
	constant seg_R : std_logic_vector(6 downto 0) := "0111001";
	constant seg_E : std_logic_vector(6 downto 0) := "0110001";
	constant seg_O : std_logic_vector(6 downto 0) := "0000001";
	constant seg_L : std_logic_vector(6 downto 0) := "1110001";
	constant seg_H : std_logic_vector(6 downto 0) := "1001000";
	
   type vetor_texto is array (0 to 3) of std_logic_vector(6 downto 0);
   signal texto : vetor_texto;
	
begin

	process(clk,rst)
	begin
			if (rst='1') then
				pr_state <= vermelho;
				conta_5s <= (others => '0');
				mux_count <= (others => '0');
				
			elsif rising_edge(clk) then
			
			   mux_count <= mux_count + 1;
				
				if conta_5s < (tempo_5s - 1) then
					conta_5s <= conta_5s + 1;
				else
					pr_state <= nx_state;
					conta_5s <= (others => '0');
				end if;
			end if;
	end process;
	
	process(pr_state)
	begin
		case pr_state is
		
			when vermelho => nx_state <= verde;
			when verde => nx_state <= amarelo;
			when amarelo => nx_state <= vermelho;
			when others => nx_state <= vermelho;
		end case;
	end process;
		
	process(pr_state)
	begin
		
		red    <= '0';
		yellow <= '0';
		green  <= '0';
		buzzer <= '0';
		
		texto(0) <= "1111111";
		texto(1) <= "1111111";
		texto(2) <= "1111111";
		texto(3) <= "1111111";
		
		case pr_state is
				
			when vermelho =>
				red <= '1';
				texto(0) <= seg_P; texto(1) <= seg_A; texto(2) <= seg_R; texto(3) <= seg_E;
				
			when verde =>
				green <= '1';
				texto(0) <= seg_V; texto(1) <= seg_A; texto(2) <= seg_A; texto(3) <= seg_I;
				
			when amarelo =>
				yellow <= '1';
				buzzer <= '1';
				texto(0) <= seg_O; texto(1) <= seg_L; texto(2) <= seg_H; texto(3) <= seg_E;
			
		   when others =>
				red <= '0'; green <= '0'; yellow <= '0'; buzzer <= '0';
				texto(0) <= "1111111"; texto(1) <= "1111111"; texto(2) <= "1111111"; texto(3) <= "1111111";
		end case;	
	end process;
	
	process(mux_count, texto)
   begin
		case mux_count is
        when "00" =>
            disp <= "1110"; -- liga display 0
            seg  <= texto(0);
        when "01" =>
            disp <= "1101"; -- display 1
            seg  <= texto(1);
        when "10" =>
            disp <= "1011"; -- display 2
            seg  <= texto(2);
        when "11" =>
            disp <= "0111"; -- display 3
            seg  <= texto(3);
		  when others =>
				disp <= "1111"; -- todos desligados
			   seg  <= "1111111"; -- tudo apagado
		end case;
   end process;
end architecture;	
			