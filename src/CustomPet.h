#pragma once
#include <stddef.h>
#include <stdint.h>
#include "art/ArtTypes.h"

// Bichinho criado no editor do site e enviado pelo USB, sem recompilar.
// Ele vira uma espécie extra (índice PET_COUNT) ao lado das seis de fábrica.
//
// Pacote "PGP1" (little-endian), o mesmo gerado por preview/petpack.js:
//   "PGP1" | flags (bit0 = de perfil) | comida (0..5) | nome (u8 n, n bytes)
//   | cores (u8 n = 1..15, n × RGB; o índice 0 é o LED apagado)
//   | ovo (4 × RGB: casca, pintas, brilho, rachadura)
//   | frames (u8 n = 1..40; cada um: w, h (1..8) e w×h índices)
//   | 9 animações na ordem de PET_ANIMS (idle, blink, walk, eat, sleep, happy,
//     sad, hungry, tired): u16 ms, u8 n = 1..12, n índices de frame
//   | CRC-32 (IEEE) de tudo o que vem antes
//
// Protocolo por linhas de texto na mesma USB dos logs; respostas começam
// com "PG " para o editor separá-las dos logs:
//   PG?            -> PG HELLO <protocolo> <máx bytes> <tem pet 0/1> <ativo 0/1> <nome|->
//   PGPUT <bytes>  -> PG READY          (começa um envio, descarta o anterior)
//   PGD <hex>      -> PG ACK <recebidos> (até 64 bytes por linha)
//   PGEND          -> PG SAVED <nome> | PG ERR <motivo> (valida antes de gravar)
//   PGADOPT        -> PG ADOPTED        (troca o bichinho atual por um ovo dele)
//   PGDEL          -> PG DELETED
namespace CustomPet {

constexpr uint8_t PROTOCOL = 1;
constexpr size_t MAX_BYTES = 3072;
constexpr uint8_t MAX_COLORS = 15;
constexpr uint8_t MAX_FRAMES = 40;
constexpr uint8_t MAX_ANIM_FRAMES = 12;
constexpr uint8_t MAX_NAME = 12;
constexpr uint8_t ANIM_COUNT = 9;
constexpr uint8_t FOOD_COUNT = 6;
constexpr size_t MAX_LINE = 160; // "PGD " + 128 dígitos hex, com folga

void begin();      // carrega o pacote salvo, se houver
bool available();  // há um pacote válido carregado
const Art::PetDef &def();

// nullptr se o pacote é válido; senão, o motivo (texto curto, sem espaços).
const char *validate(const uint8_t *data, size_t len);
// Valida, grava na NVS e passa a usar o pacote. Em erro, nada muda.
const char *install(const uint8_t *data, size_t len);
void remove();

uint32_t crc32(const uint8_t *data, size_t len);

// Processa uma linha do protocolo. Devolve false se a linha não é do
// protocolo; em true, `reply` recebe a resposta (sem quebra de linha).
bool handleLine(const char *line, char *reply, size_t n);

} // namespace CustomPet
