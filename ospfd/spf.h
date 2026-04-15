#ifndef MINI_XR_SPF_H
#define MINI_XR_SPF_H

#include "lsdb.h"
#include <string>
#include <map>
#include <set>

class SPFEngine {
public:
    SPFEngine(const std::string& local_id);

    void run(const LSDB& lsdb);

    const std::map<std::string, int>& get_distances() const;
    const std::map<std::string, std::set<std::string>>& get_next_hops() const;

private:
    std::string local_router_id;

    std::map<std::string, int> distances;

    // ECMP next-hop tracking
    std::map<std::string, std::set<std::string>> next_hops;
};

#endif
