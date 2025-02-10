#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "atividadeu4c04.pio.h"// Biblioteca para WS2812

#define IS_RGBW false
//Define que os LEDs são RGB (e não RGBW)
#define NUM_PIXELS 25
//array com os 25 LEDs (matriz 5x5).
#define WS2812_PIN 7
//Matriz 5x5 de LEDs (endereçáveis) WS2812, conectada à GPIO 7.
#define BTN_A 5
#define BTN_B 6
//Botão A conectado à GPIO 5. Botão B conectado à GPIO 6. 
#define LED_R 13
#define LED_B 12
#define LED_G 11
//LED RGB, com os pinos conectados às GPIOs (11, 12 e 13). Green ->11, Blue->12, Red-> 13

//Variáveis Globais
volatile int numero_exibido = 0; // Número atual exibido na matriz
volatile bool estado_led_r = false; // Estado do LED vermelho

// Alterna o LED vermelho entre ligar e desligar fazendo ele piscar continuamente 5 vezes por segundo. 
void piscar_led() {
    gpio_put(LED_R, estado_led_r);
    estado_led_r = !estado_led_r;
}

// Prototipação da função put_pixel
static inline void put_pixel(uint32_t pixel_grb);

// Buffer da matriz 5x5
bool led_buffer[NUM_PIXELS] = {0};

// Cada número (0-9) é mapeado em uma matriz 5x5, caracteres em estilo digital
const bool numeros_matriz_led [10][NUM_PIXELS] = {
    {1,1,1,1,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,1}, // 0
    {1,0,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,0,1, 1,0,0,0,0}, // 1
    {1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1}, // 2
    {1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1}, // 3
    {1,0,0,0,0, 0,0,0,0,1, 1,1,1,1,1, 1,0,0,0,1, 1,0,0,0,1}, // 4
    {1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,1}, // 5
    {1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,1}, // 6
    {1,0,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,0,1, 1,1,1,1,1}, // 7
    {1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1}, // 8
    {1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1}  // 9
};

// Atualiza o buffer (led_buffer[]) da matriz com o número incrementado ou decrementado.
void atualizar_matriz() {
    for (int i = 0; i < NUM_PIXELS; i++) {
        led_buffer[i] = numeros_matriz_led[numero_exibido][i];
    }
}

// Converte RGB em formato adequado para os LEDs WS2812.Envia os pixels para a matriz WS2812, exibindo o número na matriz usando a cor escolhida.
void exibir_leds(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color = ((uint32_t)r << 8) | ((uint32_t)g << 16) | (uint32_t)b;
    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(led_buffer[i] ? color : 0);
    }
}

// Envia dados (cor) para a WS2812, usando PIO (Programmed I/O)
static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// Evita "debounce" verificando se passaram 200ms desde o último clique. Uma função de interrupção para os botões A e B e incrementa ou decrementa o numero exibido
void gpio_callback(uint gpio, uint32_t events) {
    static uint64_t ultimo_tempo_a = 0;
    static uint64_t ultimo_tempo_b = 0;
    uint64_t tempo_atual = time_us_64();

    if (gpio == BTN_A && tempo_atual - ultimo_tempo_a > 200000) { // Debounce de 200ms
        if (numero_exibido < 9) numero_exibido++;
        atualizar_matriz();
        ultimo_tempo_a = tempo_atual;
    } 
    else if (gpio == BTN_B && tempo_atual - ultimo_tempo_b > 200000) { // Debounce de 200ms
        if (numero_exibido > 0) numero_exibido--;
        atualizar_matriz();
        ultimo_tempo_b = tempo_atual;
    }
}
// funcao auxiliar para configuracoes
void config_gpio() {
    // Inicializa entrada/saída padrão para configuração dos LEDs RGB
    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_put(LED_R, 0); // Garante que o LED começe apagado

    // Configura o periférico PIO para controlar os LEDs WS2812.
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &atividadeu4c04_program);
    atividadeu4c04_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // Inicializando e configurando GPIOs dos botoes como entradas
    gpio_init(BTN_A);
    gpio_init(BTN_B);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_A);
    gpio_pull_up(BTN_B);

    // Ativa as interrupções nos botões para chamar gpio_callback()
    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BTN_B, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
}

int main() {
    stdio_init_all();

    // chama funcao auxiliar
    config_gpio();

    atualizar_matriz(); // Atualiza matriz com o primeiro número

    //loop
    while (1) {
        exibir_leds(255, 0, 0);  // Exibe o número em vermelho
        piscar_led(); // Pisca o LED vermelho a 5 Hz (5 vezes por segundo)
        sleep_ms(100);
    }

    return 0;
}
