#ifndef GEAS_PRIORITY_BRANCH__H
#define GEAS_PRIORITY_BRANCH__H
#include <climits>
#include <geas/solver/branch.h>


namespace branching {

template<int VarC, class V>
forceinline int score(geas::solver_data* s, V x) {
    geas::ctx_t& ctx(s->state.p_vals);
    switch(VarC) {
      case geas::Var_Smallest:
        return x.lb(ctx);
      case geas::Var_Largest:
        return -x.ub(ctx);
      case geas::Var_FirstFail:
        return x.ub(ctx) - x.lb(ctx);
      default:
        return 0;
  }
}


template<int Sel, class V>
class priority : public geas::brancher {
  struct elt { V x; geas::brancher* b; };
public:
  priority(geas::vec<V>& xs, geas::vec<geas::brancher*>& bs)
    : start(0) {
    for(int ii : geas::irange(std::min(xs.size(), bs.size()))) {
      elts.push(elt { xs[ii], bs[ii] });
    }
  }

  bool is_fixed(geas::solver_data* s) {
    elt* e = elts.begin() + start;
    elt* end = elts.end(); 

    if(e != end) {
      if(!e->b->is_fixed(s))
        return false;
      for(++e; e != end; ++e) {
        if(!e->b->is_fixed(s)) {
          start.set(s->persist, e - elts.begin());
          return false;
        }
      }
      start.set(s->persist, elts.size());
    }
    return true;
  }

  geas::patom_t branch(geas::solver_data* s) {
    elt* e = elts.begin() + start;
    elt* end = elts.end();
    // Find the best
    int opt = INT_MAX;
    elt* sel = elts.end();
    for(; e != end; ++e) {
      if(e->b->is_fixed(s))
        continue;

      int sc = score<Sel, V>(s, e->x);
      if(sc < opt) {
        opt = sc;
        sel = e;
      }
    }
    if(sel != end)
      return sel->b->branch(s);
    return geas::at_Undef;
  }
  
  geas::vec<elt> elts;
  // Trailed start of 'live' geas::branchers
  geas::Tint start;
};

};

template<class V>
geas::brancher* priority_brancher(geas::VarChoice sel, geas::vec<V>& xs, geas::vec<geas::brancher*>& br) {
  switch(sel) {
    case geas::Val_Min:
      return new branching::priority<geas::Val_Min, V>(xs, br);
    case geas::Val_Max:
      return new branching::priority<geas::Val_Max, V>(xs, br);
    case geas::Val_Split:
      return new branching::priority<geas::Val_Split, V>(xs, br);
    case geas::Val_Saved:
      return new branching::priority<geas::Val_Saved, V>(xs, br);
    default:
      GEAS_NOT_YET;
      return nullptr;
  }
}


#endif
