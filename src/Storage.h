#pragma once
#include <stddef.h>

// Guarda o estado do bichinho na NVS (flash) como um bloco binário.
namespace Storage {
bool load(void *data, size_t len); // false se não houver nada salvo (ou tamanho diferente)
void save(const void *data, size_t len);
} // namespace Storage
