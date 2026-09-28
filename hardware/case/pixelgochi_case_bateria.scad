// PixelGochi — case COM BATERIA para a Waveshare ESP32-S3-Matrix (FDM, sem suporte)
//
// Mesma largura da case simples (28,7 mm), mais alta: embaixo da tela fica o
// carregador (TP4056); atrás da placa, uma prateleira e depois a bateria, com a
// chave liga/desliga numa faixa ao lado dela.
//
// Peças (PART), ou tools/build_case.py pra gerar todas:
//   "front" corpo com a tela; imprime com a FRENTE na mesa
//   "mid"   prateleira entre a placa e a bateria; imprime como vem (lado liso na mesa)
//   "lid"   tampa de trás; imprime como vem (lado de fora na mesa)
//   "pin"   pino de botão (2 + reserva); imprime como vem
//   "assembly" / "exploded" só pra visualizar
//
// Ligação elétrica (ver README): bateria -> TP4056 (B+/B-); B+ -> chave ->
// diodo Schottky -> pad 5V da placa; B- -> pad GND; IN+/IN- do TP4056 no pad
// 5V/GND. O diodo D1 da placa impede a bateria de voltar pro USB.
//
// Eixos: y pra cima (lado do USB), z = 0 na frente crescendo pra trás, x pra
// direita de quem olha o VERSO. Medidas em mm.

include <placa.scad>

PART = "assembly";
STYLE = "open";      // "open" ou "diffuser" (face fina + grade; PLA branco)
KEYCHAIN = true;

/* [Bateria] — HC 801723, 160 mAh (confira com régua!) */
bat_l = 28;          // comprimento TOTAL, com a plaquinha/fita da ponta
bat_w = 17.5;
bat_t = 8.2;
bat_gap = 0.8;       // folga em volta
bat_top = 6.4;       // borda de cima da bateria (abaixo dos pinos dos botões)

/* [Carregador] — TP4056 com proteção, micro-USB (~25 x 18); o simples (22 x 17) também cabe */
mod_w = 25.0;
mod_h = 18.0;
mod_t = 5.2;         // com o conector USB dele (que não é usado)
bay_h = 19;          // altura do compartimento abaixo da placa
bay_wall = 1.2;      // parede mais fina só ali, pra caber a largura dos módulos

/* [Chave] — deslizante SS12D00 */
sw_len = 8.7;        // corpo
sw_wid = 3.7;
sw_body_h = 3.6;
sw_lever = 1.6;      // alavanca (quadrada)
sw_travel = 2.0;
sw_y = -14;          // centro da chave (y)

/* [Case] */
fit = 0.25;
wall = 1.6;
corner_r = 2.6;
lip = 1.8;
window_r = 0.5;
cavity = 4.2;        // atrás da placa: USB, fios e diodo
mid_t = 1.2;         // prateleira
lid_t = 1.6;
plate_fit = 0.12;
usb_open_w = 12.4;
usb_open_h = 7.4;
pin_hole = 2.9;
pin_d = 2.5;
pin_flange = 3.4;
pin_tip = 1.8;
pin_out = 1.0;

/* [Estilo difusor] */
diff_t = 0.6;
standoff = 3.0;
grid_wall = 0.6;

$fn = 48;
eps = 0.01;

inner = board + 2 * fit;                       // 25,5
outer_w = inner + 2 * wall;
top_in = inner / 2;
bot_in = -inner / 2 - bay_h;
inner_h = top_in - bot_in;
cy = (top_in + bot_in) / 2;                    // centro vertical da case
outer_h = inner_h + 2 * wall;
face_t = STYLE == "open" ? 1.2 : diff_t;
pcb_front = face_t + (STYLE == "open" ? 0 : standoff);
pcb_back = pcb_front + pcb_t;
plate_in = pcb_back + cavity;
mid_back = plate_in + mid_t;
bat_layer = bat_t + bat_gap;
lid_in = mid_back + bat_layer;
depth = lid_in + lid_t;
window = board - 2 * lip;
usb_z = pcb_back + usb_h / 2;
sw_top = pcb_back + sw_h;
bat_x0 = -inner / 2 + 0.15;                    // bateria encostada na parede da esquerda
bat_x1 = bat_x0 + bat_w + bat_gap;
rib_x = bat_x1;                                // divisória bateria | chave
bat_bottom = bat_top - bat_l - bat_gap;

assert(bat_bottom > bot_in, "bateria não cabe: aumente bay_h");
bay_w = outer_w - 2 * bay_wall;
assert(mod_w < bay_w - 0.4 && mod_h < bay_h - 0.4, "carregador não cabe no compartimento");
assert(face_t + mod_t < plate_in, "carregador mais alto que o espaço atrás da tela");
assert(inner / 2 - rib_x - 0.8 > sw_body_h, "chave não cabe ao lado da bateria");

echo(str("Case externa: ", outer_w, " x ", outer_h, " x ", depth, " mm"));

// retângulo arredondado centrado em (0, y0)
module rr(w, h, r, z, hh, y0 = 0) {
    translate([0, y0, z]) linear_extrude(hh) offset(r) square([w - 2 * r, h - 2 * r], center = true);
}
module rsq(size, r, h, z = 0) { rr(size, size, r, z, h); }

module keychain() {
    if (KEYCHAIN)
        difference() {
            hull() {
                translate([-outer_w / 2 + 4.2, cy + outer_h / 2 + 2.4, 0]) cylinder(d = 6.4, h = 3);
                translate([-outer_w / 2 + 1.2, cy + outer_h / 2 - 2, 0]) cube([6.2, 1, 3]);
            }
            translate([-outer_w / 2 + 4.2, cy + outer_h / 2 + 2.4, -1]) cylinder(d = 2.8, h = 5);
        }
}

module grid() {
    h = pcb_front - led_h - 0.3 - face_t;
    intersection() {
        rsq(window, window_r, h, face_t - eps);
        union() for (k = [-4:4]) {
            translate([k * led_pitch - grid_wall / 2, -board / 2, face_t - eps]) cube([grid_wall, board, h]);
            translate([-board / 2, k * led_pitch - grid_wall / 2, face_t - eps]) cube([board, grid_wall, h]);
        }
    }
}

// Rasgo do cabo USB (fechado em cima) + canal por dentro da parede pra o
// USB da placa passar quando ela entra por trás.
module usb_cut() {
    r = 1.6;
    translate([usb_x, inner / 2 - 1, usb_z]) rotate([-90, 0, 0])
        linear_extrude(wall + 2) hull()
            for (sx = [-1, 1], sz = [-1, 1])
                translate([sx * (usb_open_w / 2 - r), sz * (usb_open_h / 2 - r)]) circle(r);
    translate([usb_x - 4.47 - 0.4, inner / 2 - 1, pcb_back - 0.2]) cube([8.94 + 0.8, 1 + 0.9, depth]);
}

groove_len = 12;
module lid_grooves(extra = 0) {
    for (s = [-1, 1])
        translate([s * (inner / 2), cy + 8, lid_in + lid_t / 2])
            cube([2 * (0.35 + extra), groove_len + 2 * extra, 0.8 + 2 * extra], center = true);
}

switch_slot_len = sw_lever + sw_travel + 0.6;
sw_z = mid_back + sw_wid / 2;                  // centro da alavanca (z)

module front() {
    difference() {
        union() {
            rr(outer_w, outer_h, corner_r, 0, depth, cy);
            keychain();
        }
        if (STYLE == "open") rsq(window, window_r, pcb_front + 2, -1);
        else rsq(window, window_r, pcb_front - face_t + eps, face_t);
        // tudo atrás da placa
        rr(inner, inner_h, board_r + fit, pcb_front, depth, cy);
        // compartimento do carregador, desde a face (abaixo da placa), um pouco
        // mais largo que o resto do vão (parede de bay_wall só nessa faixa)
        translate([-bay_w / 2, bot_in, face_t]) cube([bay_w, -inner / 2 - bot_in - 0.5, plate_in - face_t]);
        translate([-inner / 2, bot_in, face_t]) cube([inner, -inner / 2 - bot_in + eps, depth]);
        usb_cut();
        lid_grooves(0.05);
        // alavanca da chave, na lateral direita (olhando o verso)
        translate([inner / 2 - 1, sw_y - switch_slot_len / 2, sw_z - (sw_lever + 0.5) / 2])
            cube([wall + 2, switch_slot_len, sw_lever + 0.5]);
        // entalhe pra abrir
        translate([-2, cy - outer_h / 2 - 1, depth - 1.2]) cube([4, wall + 1.2, 2]);
    }
    if (STYLE == "diffuser") grid();
}

post = 2.2;
module mid() {
    pw = inner - 2 * plate_fit;
    difference() {
        union() {
            rr(pw, inner_h - 2 * plate_fit, board_r + fit - plate_fit, plate_in, mid_t, cy);
            // postes que prendem a placa contra a borda da frente
            for (sx = [-1, 1], sy = [-1, 1])
                translate([sx * (board / 2 - post / 2 - 0.2) - post / 2, sy * (board / 2 - post / 2 - 0.2) - post / 2, pcb_back + 0.05])
                    cube([post, post, plate_in - pcb_back]);
        }
        for (s = [-1, 1]) translate([s * btn_dx, btn_y, plate_in - 1]) cylinder(d = pin_hole, h = mid_t + 2);
        // passagem dos fios: bateria/conector -> carregador, chave -> diodo
        translate([bat_x0 + 2, bot_in + 1.2, plate_in - 1]) cube([10, bat_bottom - bot_in - 2, mid_t + 2]);
        translate([rib_x + 1.6, sw_y - sw_len / 2 - 5, plate_in - 1]) cube([3.5, 3.5, mid_t + 2]);
    }
}

module lid() {
    pw = inner - 2 * plate_fit;
    difference() {
        union() {
            rr(pw, inner_h - 2 * plate_fit, board_r + fit - plate_fit, lid_in, lid_t, cy);
            // travas nos sulcos da parede
            for (s = [-1, 1])
                translate([s * (pw / 2), cy + 8, lid_in + lid_t / 2])
                    rotate([90, 0, 0]) cylinder(r = 0.3, h = groove_len - 1, center = true, $fn = 16);
            // divisórias da bateria: seguram a bateria e apertam a prateleira contra a placa
            translate([rib_x, bat_bottom, mid_back + 0.05]) cube([0.8, bat_top - bat_bottom + 0.8, bat_layer - 0.05]);
            translate([bat_x0, bat_top, mid_back + 0.05]) cube([rib_x - bat_x0 + 0.8, 0.8, bat_layer - 0.05]);
            // batentes da chave
            for (s = [-1, 1])
                translate([rib_x + 0.8, sw_y + s * (sw_len / 2 + 0.2) - (s < 0 ? 1 : 0), mid_back + 0.05])
                    cube([inner / 2 - plate_fit - rib_x - 0.8, 1, sw_wid]);
            // calombinhos que encostam na bateria (tira a folga)
            for (y = [bat_bottom + 6, (bat_bottom + bat_top) / 2, bat_top - 6])
                translate([(bat_x0 + rib_x) / 2 - 4, y - 0.6, lid_in - 0.5]) cube([8, 1.2, 0.5 + eps]);
        }
        for (s = [-1, 1]) translate([s * btn_dx, btn_y, lid_in - 1]) cylinder(d = pin_hole, h = lid_t + 2);
        for (b = [[-btn_dx, "R"], [btn_dx, "B"]])
            translate([b[0], btn_y - 4.2, depth - 0.4]) linear_extrude(1)
                text(b[1], size = 2.6, halign = "center", valign = "center", font = "Liberation Sans:style=Bold");
        translate([-2, cy - pw / 2 - (inner_h - pw) / 2 - 1, depth - 0.8]) cube([4, 2, 1]);
    }
}

pin_len = depth + pin_out - sw_top;
flange_top = (pin_flange - pin_tip) / 2 + 0.2 + (pin_flange - pin_d) / 2;
assert(sw_top + 0.5 + flange_top < plate_in, "pouca folga pra aba do pino: aumente 'cavity'");
module pin() {
    cylinder(d1 = pin_tip, d2 = pin_flange, h = (pin_flange - pin_tip) / 2);
    translate([0, 0, (pin_flange - pin_tip) / 2]) cylinder(d = pin_flange, h = 0.2);
    translate([0, 0, (pin_flange - pin_tip) / 2 + 0.2]) cylinder(d1 = pin_flange, d2 = pin_d, h = (pin_flange - pin_d) / 2);
    difference() {
        cylinder(d = pin_d, h = pin_len);
        translate([0, 0, pin_len - 0.4]) difference() {
            cylinder(d = pin_d + 1, h = 1);
            cylinder(d1 = pin_d, d2 = pin_d - 0.8, h = 0.4);
        }
    }
}
module pins_in_place() { for (s = [-1, 1]) translate([s * btn_dx, btn_y, sw_top]) pin(); }

// Modelos simplificados pra conferir encaixe e colisões (não imprimir).
module battery_model() {
    color("silver") translate([bat_x0 + bat_gap / 2, bat_bottom + bat_gap / 2, mid_back + 0.1]) cube([bat_w, bat_l, bat_t]);
}
module charger_model() {
    color("#1a4fa0") translate([-mod_w / 2, -inner / 2 - 0.4 - mod_h, face_t + 0.05]) cube([mod_w, mod_h, mod_t]);
}
module switch_model() {
    color("#555") translate([inner / 2 - sw_body_h - 0.05, sw_y - sw_len / 2, mid_back + 0.1]) cube([sw_body_h, sw_len, sw_wid]);
    color("#999") translate([inner / 2 - 0.05, sw_y - sw_lever / 2, sw_z - sw_lever / 2]) cube([wall + 1.6, sw_lever, sw_lever]);
}
module parts_model() { board_model(); battery_model(); charger_model(); switch_model(); }

if (PART == "front") front();
else if (PART == "mid") translate([0, 0, mid_back]) mirror([0, 0, 1]) mid();    // lado liso na mesa
else if (PART == "lid") translate([0, 0, depth]) mirror([0, 0, 1]) lid();       // lado de fora na mesa
else if (PART == "pin") translate([0, 0, pin_len]) mirror([0, 0, 1]) pin();
else if (PART == "assembly") {
    color("#ffb347", 0.45) front();
    parts_model();
    color("#9ad") mid();
    color("#7fc8ff") pins_in_place();
    color("#9ad", 0.6) lid();
} else if (PART == "exploded") {
    color("#ffb347") front();
    translate([0, 0, 8]) { board_model(); charger_model(); }
    translate([0, 0, 18]) color("#9ad") mid();
    translate([0, 0, 28]) { battery_model(); switch_model(); }
    translate([0, 0, 36]) color("#7fc8ff") pins_in_place();
    translate([0, 0, 46]) color("#9ad") lid();
}
else if (PART == "collide_front") intersection() { front(); parts_model(); }
else if (PART == "collide_mid") intersection() { mid(); parts_model(); }
else if (PART == "collide_lid") intersection() { lid(); parts_model(); }
else if (PART == "collide_pins") intersection() { union() { mid(); lid(); } pins_in_place(); }
else if (PART == "collide_pins_parts") intersection() { pins_in_place(); union() { battery_model(); charger_model(); switch_model(); } }
