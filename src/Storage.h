#pragma once
#include <stddef.h>

// Guarda dados na NVS (flash) como blocos binários: o estado do bichinho e,
// opcionalmente, o pacote de arte de um bichinho criado no editor.
namespace Storage {
bool load(void *data, size_t len); // false se não houver nada salvo (ou tamanho diferente)
void save(const void *data, size_t len);

// Bloco de tamanho variável. loadBlob devolve o tamanho lido (0 se não houver
// ou se não couber em max). A NVS grava o valor novo antes de soltar o antigo:
// uma queda de energia no meio mantém a versão anterior.
size_t loadBlob(const char *key, void *data, size_t max);
bool saveBlob(const char *key, const void *data, size_t len);
void eraseBlob(const char *key);
} // namespace Storage
