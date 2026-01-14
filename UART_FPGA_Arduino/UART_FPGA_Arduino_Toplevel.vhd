library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity UART_Arduino_Top is
    port (
        clk       : in  std_logic;
        uart_rx_p : in  std_logic;
        displays  : out std_logic_vector(3 downto 0);
        segments  : out std_logic_vector(6 downto 0)
    );
end UART_Arduino_Top;

architecture rtl of UART_Arduino_Top is

    component UART_RX
        generic (
            g_CLKS_PER_BIT : integer := 5208  -- 50MHz / 9600bps
        );
        port (
            i_Clk       : in  std_logic;
            i_RX_Serial : in  std_logic;
            o_RX_DV     : out std_logic;
            o_RX_Byte   : out std_logic_vector(7 downto 0)
        );
    end component;

    component bcd_seven_seg
        port (
            clock_i : in  std_logic;
            bcd_i   : in  std_logic_vector(3 downto 0);
            disp    : out std_logic_vector(3 downto 0);
            seven_o : out std_logic_vector(6 downto 0)
        );
    end component;

    signal rx_data   : std_logic_vector(7 downto 0) := (others => '0');
    signal rx_valid  : std_logic := '0';
    signal digit_bcd : std_logic_vector(3 downto 0) := (others => '0');

begin

    -- Instancia o UART RX
    uart_rx_inst : UART_RX
        generic map (
            g_CLKS_PER_BIT => 5208
        )
        port map (
            i_Clk        => clk,
            i_RX_Serial  => uart_rx_p,
            o_RX_DV      => rx_valid,
            o_RX_Byte    => rx_data
        );

    -- Converte ASCII recebido para BCD se for entre '0' e '9'
    process(clk)
    begin
        if rising_edge(clk) then
            if rx_valid = '1' then
                if rx_data >= x"30" and rx_data <= x"39" then
						digit_bcd <= std_logic_vector(to_unsigned(to_integer(unsigned(rx_data)) - 48, 4));
					 else
						digit_bcd <= "1110";  -- Exibe 'E' para erro
                end if;
            end if;
        end if;
    end process;

    -- Instancia o display de 7 segmentos com valor convertido
    display_inst : bcd_seven_seg
        port map (
            clock_i => clk,
            bcd_i   => digit_bcd,
            disp    => displays,
            seven_o => segments
        );

end architecture;
