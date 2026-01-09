library ieee;
use ieee.std_logic_1164.all;

entity tb_semaforo is
end tb_semaforo;

architecture sim of tb_semaforo is
    -- sinais para interligar ao DUT (Device Under Test)
    signal clk     : std_logic := '0';
    signal rst     : std_logic := '0';
    signal red     : std_logic;
    signal green   : std_logic;
    signal yellow  : std_logic;
    signal buzzer  : std_logic;
    signal disp    : std_logic_vector(3 downto 0);
    signal seg     : std_logic_vector(6 downto 0);

begin
    -- Instância do semáforo
    uut: entity work.semaforo
        port map (
            clk    => clk,
            rst    => rst,
            red    => red,
            green  => green,
            yellow => yellow,
            buzzer => buzzer,
            disp   => disp,
            seg    => seg
        );

    -- Geração do clock (50 MHz = 20 ns de período)
    clk_process: process
    begin
        while true loop
            clk <= '0';
            wait for 10 ns;
            clk <= '1';
            wait for 10 ns;
        end loop;
    end process;

    -- Estímulo de reset
    stim_proc: process
    begin
        -- aplica reset por 100 ns
        rst <= '1';
        wait for 100 ns;
        rst <= '0';

        -- deixa rodar a simulação por alguns microssegundos
        wait for 5000 ns;

        -- finaliza simulação
        assert false report "Fim da simulação" severity failure;
    end process;

end architecture;
