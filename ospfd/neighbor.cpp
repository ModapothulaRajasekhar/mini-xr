#include "neighbor.h"
#include "../common/logger.h"

Neighbor::Neighbor(const std::string& rid,
                   std::function<void()> on_full_callback,
                   std::function<void()> on_down_callback)
    : router_id(rid),
      state(NeighborState::DOWN),
      on_full(on_full_callback),
      on_down(on_down_callback) {}

void Neighbor::receive_hello() {

    last_hello = std::chrono::steady_clock::now();

    // State machine transitions only when needed

    if (state == NeighborState::DOWN) {
        transition(NeighborState::INIT);
        return;
    }

    if (state == NeighborState::INIT) {
        transition(NeighborState::TWO_WAY);
        return;
    }

    if (state == NeighborState::TWO_WAY) {
        transition(NeighborState::FULL);
        return;
    }

    // 🔥 IMPORTANT FIX
    // If already FULL, do nothing.
    // This prevents duplicate LSA installation.
    if (state == NeighborState::FULL) {
        return;
    }
}

void Neighbor::check_dead_timer() {

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                       now - last_hello).count();

    if (state != NeighborState::DOWN &&
        elapsed > dead_interval_sec) {

        log_message(LogLevel::WARN, "ospfd",
                    "Neighbor " + router_id + " dead");

        transition(NeighborState::DOWN);
    }
}

void Neighbor::transition(NeighborState new_state) {

    // 🔥 Avoid duplicate transition
    if (state == new_state)
        return;

    state = new_state;

    log_message(LogLevel::INFO, "ospfd",
                "Neighbor " + router_id +
                " → " + get_state_string());

    // FSM-owned callbacks

    if (new_state == NeighborState::FULL && on_full) {
        on_full();
    }

    if (new_state == NeighborState::DOWN && on_down) {
        on_down();
    }
}

NeighborState Neighbor::get_state() const {
    return state;
}

std::string Neighbor::get_state_string() const {

    switch (state) {
        case NeighborState::DOWN:    return "DOWN";
        case NeighborState::INIT:    return "INIT";
        case NeighborState::TWO_WAY: return "2-WAY";
        case NeighborState::FULL:    return "FULL";
    }

    return "UNKNOWN";
}
