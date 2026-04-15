#ifndef MINI_XR_OSPF_NEIGHBOR_H
#define MINI_XR_OSPF_NEIGHBOR_H

#include <string>
#include <functional>
#include <chrono>

enum class NeighborState {
    DOWN,
    INIT,
    TWO_WAY,
    FULL
};

class Neighbor {

public:
    Neighbor(const std::string& router_id,
             std::function<void()> on_full_callback,
             std::function<void()> on_down_callback);

    void receive_hello();
    void check_dead_timer();

    NeighborState get_state() const;
    std::string get_state_string() const;

private:
    void transition(NeighborState new_state);

    std::string router_id;
    NeighborState state;

    std::function<void()> on_full;
    std::function<void()> on_down;

    std::chrono::steady_clock::time_point last_hello;
    int dead_interval_sec = 10;
};

#endif
