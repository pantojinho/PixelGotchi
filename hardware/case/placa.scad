// Medidas da Waveshare ESP32-S3-Matrix, compartilhadas pelas cases.
// Vieram do desenho cotado da Waveshare e de fotos (régua = passo de 2,54 mm
// dos pinos). Eixos como nas cases: y pra cima (lado do USB), z pra trás,
// x pra direita de quem olha o VERSO.

board = 25.0;        // placa quadrada
board_r = 1.0;       // raio dos cantos
pcb_t = 1.6;         // espessura da placa
led_h = 1.0;         // altura dos LEDs acima da placa
led_pitch = 2.7;     // passo da matriz (medido: ~2,7). Só afeta a grade do estilo "diffuser".
usb_x = 0;           // deslocamento do USB-C a partir do centro (x)
usb_h = 3.26;        // altura do receptáculo USB-C acima do verso da placa
btn_dx = 7.9;        // botões: |x| a partir do centro (4,6 mm das laterais)
btn_y = 8.9;         // botões: y a partir do centro (3,6 mm da borda do USB)
sw_h = 2.0;          // altura estimada botão+êmbolo acima do verso (o pino tolera 1,5–2,5)
pad_5v = [-8.2, 2.9];   // pad "5V" no verso (TP2 no esquemático): aqui entra a bateria
pad_gnd = [-8.2, 4.7];  // pad "GND" no verso (TP3)

// Placa simplificada pra conferir encaixe e checar colisões (não imprimir).
// Usa pcb_front/pcb_back definidos pela case que incluir este arquivo.
module board_model() {
    color("#222") translate([0, 0, pcb_front]) linear_extrude(pcb_t)
        offset(board_r) square(board - 2 * board_r, center = true);
    color("#eee") for (i = [0:7], j = [0:7])
        translate([(i - 3.5) * led_pitch - 1, (j - 3.5) * led_pitch - 1, pcb_front - led_h]) cube([2, 2, led_h]);
    color("silver") translate([usb_x - 4.47, board / 2 - 7.35 + 0.8, pcb_back]) cube([8.94, 7.35, usb_h]);
    color("silver") for (s = [-1, 1]) translate([s * btn_dx - 1.35, btn_y - 1.55, pcb_back]) cube([2.7, 3.1, sw_h - 0.4]);
    color("#333") for (s = [-1, 1]) translate([s * btn_dx, btn_y, pcb_back]) cylinder(d = 1.0, h = sw_h);
    color("#111") translate([-3.5, -3.5, pcb_back]) rotate([0, 0, 45]) cube([7, 7, 0.9]);
}
