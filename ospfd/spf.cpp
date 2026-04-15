#include "spf.h"
#include <limits>
#include <set>

SPFEngine::SPFEngine(const std::string& local_id)
    : local_router_id(local_id) {}

void SPFEngine::run(const LSDB& lsdb) {

    distances.clear();
    next_hops.clear();

    const auto& db = lsdb.get_lsas();

    std::set<std::string> visited;

    // Initialize distances and next-hops
    for (const auto& [rid, _] : db) {
        distances[rid] = std::numeric_limits<int>::max();
        next_hops[rid] = {};
    }

    if (!db.count(local_router_id))
        return;

    distances[local_router_id] = 0;

    while (visited.size() < db.size()) {

        std::string current;
        int min_dist = std::numeric_limits<int>::max();

        // Select closest unvisited router
        for (const auto& [rid, dist] : distances) {
            if (!visited.count(rid) && dist < min_dist) {
                min_dist = dist;
                current = rid;
            }
        }

        if (current.empty())
            break;

        visited.insert(current);

        const auto& lsa = db.at(current);

        for (const auto& link : lsa.links) {

            const std::string& nbr = link.neighbor_id;

            if (!db.count(nbr))
                continue;

            int new_cost = distances[current] + link.cost;

            // 🔹 Better path found
            if (new_cost < distances[nbr]) {

                distances[nbr] = new_cost;
                next_hops[nbr].clear();

                // Direct neighbor
                if (current == local_router_id) {
                    next_hops[nbr].insert(nbr);
                }
                // Inherit first-hop from parent
                else {
                    next_hops[nbr] = next_hops[current];
                }
            }

            // 🔹 Equal-cost path → ECMP
            else if (new_cost == distances[nbr]) {

                // Direct equal-cost neighbor
                if (current == local_router_id) {
                    next_hops[nbr].insert(nbr);
                }
                else {
                    // Merge ECMP next-hops
                    next_hops[nbr].insert(
                        next_hops[current].begin(),
                        next_hops[current].end()
                    );
                }
            }
        }
    }
}

const std::map<std::string, int>& SPFEngine::get_distances() const {
    return distances;
}

const std::map<std::string, std::set<std::string>>&
SPFEngine::get_next_hops() const {
    return next_hops;
}
