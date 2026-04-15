#ifndef MINI_XR_LSDB_H
#define MINI_XR_LSDB_H

#include <string>
#include <vector>
#include <map>

struct Link {
    std::string neighbor_id;
    int cost;
};

struct RouterLSA {
    std::string router_id;
    int sequence_number;
    int age;
    std::vector<Link> links;
};

class LSDB {
public:
    void install(const RouterLSA& lsa);
    void remove(const std::string& router_id);

    const std::map<std::string, RouterLSA>& get_lsas() const;

    bool is_dirty() const;
    void clear_dirty();

private:
    std::map<std::string, RouterLSA> router_lsas;
    bool dirty = false;
};

#endif

