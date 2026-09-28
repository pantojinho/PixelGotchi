// PixelGochi — case para a Waveshare ESP32-S3-Matrix (impressão FDM, sem suporte)
//
// Peças (escolha em PART, ou use tools/build_case.py pra gerar todas):
//   "front"    moldura/corpo, imprime com a FRENTE virada pra mesa
//   "back"     tampa traseira com furos dos botões, imprime com o lado de fora na mesa
//   "pin"      pino de botão (imprima 2), imprime de cabeça pra baixo
//   "assembly" visualização montada (não imprimir)
//   "exploded" visualização explodida (não imprimir)
//
// Eixos: y pra cima (lado do USB); z = 0 na face da frente, crescendo pra
// trás; x pra direita de quem olha o VERSO (de frente, x fica pra esquerda).
// O desenho é simétrico em x, exceto as letras R/B e a argolinha. Medidas em mm.

PART = "assembly";
STYLE = "open";      // "open" (janela aberta) ou "diffuser" (face fina + grade; PLA branco)
KEYCHAIN = true;     // argolinha de chaveiro no canto

/* [Placa] — Waveshare ESP32-S3-Matrix */
board = 25.0;        // placa quadrada
board_r = 1.0;       // raio dos cantos
pcb_t = 1.6;         // espessura da placa
led_h = 1.0;         // altura dos LEDs acima da placa
led_pitch = 2.7;     // passo da matriz (medido: ~2,7). Só afeta a grade do estilo "diffuser".
usb_x = 0;           // deslocamento do USB-C a partir do centro (x)
usb_h = 3.26;        // altura do receptáculo USB-C acima do verso da placa
btn_dx = 7.9;        // botões: |x| a partir do centro (4,6 mm das laterais)
btn_y = 8.9;         // botões: y a partir do centro (3,6 mm da borda do USB)
sw_h = 2.0;          // altura estimada botão+êmbolo acima do verso da placa (pino tolera 1,5–2,5)

/* [Case] */
fit = 0.25;          // folga placa<->parede (por lado)
wall = 1.6;          // parede lateral (4 linhas de 0,4)
corner_r = 2.6;      // raio externo dos cantos
lip = 1.8;           // borda da frente que segura a placa (cobre a fileira de pinos)
window_r = 0.5;      // raio dos cantos da janela (maior que isso come a quina dos LEDs dos cantos)
cavity = 4.2;        // espaço atrás da placa (USB 3,26 + folga pros pinos)
back_t = 1.6;        // espessura da tampa
plate_fit = 0.12;    // folga tampa<->parede (por lado)
usb_open_w = 12.4;   // rasgo do USB (cabe a capa de borracha dos cabos comuns)
usb_open_h = 7.4;
pin_hole = 2.9;      // furo do pino na tampa
pin_d = 2.5;         // haste do pino
pin_flange = 3.4;    // aba do pino (maior que o furo: não cai pra fora)
pin_tip = 1.8;       // ponta que encosta no êmbolo do botão
pin_out = 1.0;       // quanto o pino fica pra fora da tampa em repouso

/* [Estilo difusor] */
diff_t = 0.6;        // face fina que difunde a luz (imprima em PLA branco/natural)
standoff = 3.0;      // espaço difusor<->placa (onde fica a grade)
grid_wall = 0.6;

$fn = 48;
eps = 0.01;

inner = board + 2 * fit;                 // vão interno
outer = inner + 2 * wall;                // tamanho externo
face_t = STYLE == "open" ? 1.2 : diff_t;
pcb_front = face_t + (STYLE == "open" ? 0 : standoff);
pcb_back = pcb_front + pcb_t;
plate_in = pcb_back + cavity;            // face interna da tampa
depth = plate_in + back_t;               // profundidade total
window = board - 2 * lip;
usb_z = pcb_back + usb_h / 2;
sw_top = pcb_back + sw_h;

echo(str("Case externa: ", outer, " x ", outer, " x ", depth, " mm"));

module rsq(size, r, h, z = 0) {
    translate([0, 0, z]) linear_extrude(h) offset(r) square(size - 2 * r, center = true);
}

// Rasgo do USB: retângulo arredondado no plano XZ, atravessando a parede de cima.
module usb_cut() {
    r = 1.6;
    translate([usb_x, inner / 2 - 1, usb_z]) rotate([-90, 0, 0])
        linear_extrude(wall + 2) hull()
            for (sx = [-1, 1], sz = [-1, 1])
                translate([sx * (usb_open_w / 2 - r), sz * (usb_open_h / 2 - r)]) circle(r);
    // abre até a borda de trás pra placa entrar por trás
    translate([usb_x - usb_open_w / 2, inner / 2 - 1, usb_z]) cube([usb_open_w, wall + 2, depth]);
}

// Trilho que "clica" a tampa: sulco nas paredes esquerda/direita.
groove_len = 11;
groove_z = plate_in + back_t / 2;
module plate_grooves(extra = 0) {
    for (s = [-1, 1])
        translate([s * (inner / 2), 0, groove_z])
            cube([2 * (0.35 + extra), groove_len + 2 * extra, 0.8 + 2 * extra], center = true);
}

module keychain() {
    if (KEYCHAIN)
        difference() {
            hull() {
                translate([-outer / 2 + 4.2, outer / 2 + 2.4, 0]) cylinder(d = 6.4, h = 3);
                translate([-outer / 2 + 1.2, outer / 2 - 2, 0]) cube([6.2, 1, 3]);
            }
            translate([-outer / 2 + 4.2, outer / 2 + 2.4, -1]) cylinder(d = 2.8, h = 5);
        }
}

module grid() {
    // grade 8x8: paredes nas divisas entre LEDs, do difusor até acima dos LEDs
    h = pcb_front - led_h - 0.3 - face_t;
    intersection() {
        rsq(window, window_r, h, face_t - eps);
        union() for (k = [-4:4]) {
            translate([k * led_pitch - grid_wall / 2, -board / 2, face_t - eps]) cube([grid_wall, board, h]);
            translate([-board / 2, k * led_pitch - grid_wall / 2, face_t - eps]) cube([board, grid_wall, h]);
        }
    }
}

module front() {
    difference() {
        union() {
            rsq(outer, corner_r, depth);
            keychain();
        }
        // janela (aberta) ou espaço atrás do difusor
        if (STYLE == "open") rsq(window, window_r, pcb_front + 2, -1);
        else rsq(window, window_r, pcb_front - face_t + eps, face_t);
        // vão da placa + eletrônica + tampa
        rsq(inner, board_r + fit, depth, pcb_front);
        usb_cut();
        plate_grooves(0.05);
        // entalhe pra abrir com a unha (embaixo)
        translate([-2, -outer / 2 - 1, depth - 1.2]) cube([4, wall + 1.2, 2]);
    }
    if (STYLE == "diffuser") grid();
}

// Postes que prendem a placa contra a borda da frente (cantos, longe de
// USB e botões).
post = 2.2;
module back() {
    pw = inner - 2 * plate_fit;
    difference() {
        union() {
            rsq(pw, board_r + fit - plate_fit, back_t, plate_in);
            for (sx = [-1, 1], sy = [-1, 1])
                translate([sx * (board / 2 - post / 2 - 0.2) - post / 2, sy * (board / 2 - post / 2 - 0.2) - post / 2, pcb_back + 0.05])
                    cube([post, post, plate_in - pcb_back]);
            // travas que entram nos sulcos da parede
            for (s = [-1, 1])
                translate([s * (pw / 2), 0, groove_z])
                    rotate([90, 0, 0]) cylinder(r = 0.3, h = groove_len - 1, center = true, $fn = 16);
        }
        for (s = [-1, 1]) translate([s * btn_dx, btn_y, plate_in - 1]) cylinder(d = pin_hole, h = back_t + 2);
        // Letras gravadas por fora, lidas olhando o verso (quem olha o verso
        // está do lado +z, então x cresce pra direita): RESET à esquerda,
        // BOOT à direita, como na serigrafia da placa.
        for (b = [[-btn_dx, "R"], [btn_dx, "B"]])
            translate([b[0], btn_y - 4.2, depth - 0.4]) linear_extrude(1)
                text(b[1], size = 2.6, halign = "center", valign = "center", font = "Liberation Sans:style=Bold");
        // entalhe de abertura casando com o da frente
        translate([-2, -pw / 2 - 1, depth - 0.8]) cube([4, 2, 1]);
    }
}

// Pino: ponta (encosta no botão) -> aba (não passa no furo) -> haste (sai pela tampa).
// Os cones de 45° dispensam suporte imprimindo de cabeça pra baixo.
pin_len = depth + pin_out - sw_top;
module pin() {
    cylinder(d1 = pin_tip, d2 = pin_flange, h = (pin_flange - pin_tip) / 2);
    translate([0, 0, (pin_flange - pin_tip) / 2]) cylinder(d = pin_flange, h = 0.2);
    translate([0, 0, (pin_flange - pin_tip) / 2 + 0.2]) cylinder(d1 = pin_flange, d2 = pin_d, h = (pin_flange - pin_d) / 2);
    difference() {
        cylinder(d = pin_d, h = pin_len);
        // chanfro no topo
        translate([0, 0, pin_len - 0.4]) difference() {
            cylinder(d = pin_d + 1, h = 1);
            cylinder(d1 = pin_d, d2 = pin_d - 0.8, h = 0.4);
        }
    }
}
flange_top = (pin_flange - pin_tip) / 2 + 0.2 + (pin_flange - pin_d) / 2;
assert(sw_top + flange_top < plate_in, "aba do pino encosta na tampa: aumente 'cavity'");
assert(sw_top + 0.5 + flange_top < plate_in, "pouca folga pra botões mais altos: aumente 'cavity'");

// Placa simplificada pra conferir encaixe (não imprimir).
module board_model() {
    color("#222") rsq(board, board_r, pcb_t, pcb_front);
    color("#eee") for (i = [0:7], j = [0:7])
        translate([(i - 3.5) * led_pitch - 1, (j - 3.5) * led_pitch - 1, pcb_front - led_h]) cube([2, 2, led_h]);
    color("silver") translate([usb_x - 4.47, board / 2 - 7.35 + 0.8, pcb_back]) cube([8.94, 7.35, usb_h]);
    color("silver") for (s = [-1, 1]) translate([s * btn_dx - 1.35, btn_y - 1.55, pcb_back]) cube([2.7, 3.1, sw_h - 0.4]);
    color("#333") for (s = [-1, 1]) translate([s * btn_dx, btn_y, pcb_back]) cylinder(d = 1.0, h = sw_h);
    color("#111") translate([-3.5, -3.5, pcb_back]) rotate([0, 0, 45]) cube([7, 7, 0.9]);
}

module pins_in_place() {
    for (s = [-1, 1]) translate([s * btn_dx, btn_y, sw_top]) pin();
}

if (PART == "front") front();
else if (PART == "back") translate([0, 0, depth]) mirror([0, 0, 1]) back();   // lado de fora na mesa
else if (PART == "pin") translate([0, 0, pin_len]) mirror([0, 0, 1]) pin();   // de cabeça pra baixo
else if (PART == "board") board_model();
else if (PART == "assembly") {
    color("#ffb347", 0.55) front();
    board_model();
    color("#7fc8ff") pins_in_place();
    color("#9ad", 0.9) back();
} else if (PART == "collide_front") intersection() { front(); board_model(); }
else if (PART == "collide_back") intersection() { union() { back(); pins_in_place(); } board_model(); }
else if (PART == "collide_pins") intersection() { back(); pins_in_place(); }
else if (PART == "exploded") {
    color("#ffb347") front();
    translate([0, 0, 10]) board_model();
    translate([0, 0, 22]) color("#7fc8ff") pins_in_place();
    translate([0, 0, 28]) color("#9ad") back();
}
