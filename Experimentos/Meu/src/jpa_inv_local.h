#ifndef _JPA_INV_LOCAL_H_
#define _JPA_INV_LOCAL_H_

#include <vector>
using namespace std;

// Read-only context shared across threads.
// Constructed once from global data, then passed by const-reference.
struct JpaContext {
    int natv;
    int nteam;
    int nmaq;
    float max_tempo;
    const vector<vector<float>>& atv;   // read-only
    const vector<vector<float>>& team;  // read-only

    JpaContext(int natv_, int nteam_, int nmaq_, float max_tempo_,
              const vector<vector<float>>& atv_,
              const vector<vector<float>>& team_)
        : natv(natv_), nteam(nteam_), nmaq(nmaq_), max_tempo(max_tempo_),
          atv(atv_), team(team_) {}
};

// Thread-safe version of jpa_inv.
// All mutable state (t_team, t_maq, s_atv, uni_maq, qteam, qatv)
// is allocated locally inside the function.
// Only 'contador' (global) is updated atomically when cont > 0.
int jpa_inv_local(int n, vector<int>& s, vector<int>& s_ind,
                  int cont, const JpaContext& ctx);

#endif // _JPA_INV_LOCAL_H_
