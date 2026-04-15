#include "lsdb.h"

void LSDB::install(const RouterLSA& lsa) {
    auto it = router_lsas.find(lsa.router_id);

    if (it == router_lsas.end() ||
        lsa.sequence_number > it->second.sequence_number) {

        router_lsas[lsa.router_id] = lsa;
        dirty = true;
    }
}

void LSDB::remove(const std::string& router_id) {
    if (router_lsas.erase(router_id) > 0) {
        dirty = true;
    }
}

const std::map<std::string, RouterLSA>& LSDB::get_lsas() const {
    return router_lsas;
}

bool LSDB::is_dirty() const {
    return dirty;
}

void LSDB::clear_dirty() {
    dirty = false;
}
