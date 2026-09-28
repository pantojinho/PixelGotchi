#include "Events.h"

namespace {
constexpr uint8_t CAP = 16;
Ev queue[CAP];
uint8_t head = 0, tail = 0;
} // namespace

namespace Events {

void push(Ev e) {
    uint8_t next = (tail + 1) % CAP;
    if (next == head) return; // cheio: descarta, entrada humana não perde nada importante
    queue[tail] = e;
    tail = next;
}

Ev pop() {
    if (head == tail) return Ev::None;
    Ev e = queue[head];
    head = (head + 1) % CAP;
    return e;
}

void clear() { head = tail = 0; }

} // namespace Events
