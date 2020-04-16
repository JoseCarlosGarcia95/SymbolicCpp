/*
    SymbolicC++ : An object oriented computer algebra system written in C++

    Copyright (C) 2008 Yorick Hardy and Willi-Hans Steeb

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

// solve.h

#ifndef SYMBOLIC_CPLUSPLUS_SOLVE

#ifdef SYMBOLIC_FORWARD
#ifndef SYMBOLIC_CPLUSPLUS_SOLVE_FORWARD
#define SYMBOLIC_CPLUSPLUS_SOLVE_FORWARD
#define NEWTON_MAX_PRECISSION 0.00001
#endif
#endif

#ifdef SYMBOLIC_DECLARE
#ifndef SYMBOLIC_CPLUSPLUS_SOLVE_DECLARE
#define SYMBOLIC_CPLUSPLUS_SOLVE_DECLARE

Equations solve(const Symbolic &, const Symbolic &);
Equations solve(const Equation &, const Symbolic &);
list<Equations> solve(const Equations &, const list<Symbolic> &);

#endif
#endif

#define LIBSYMBOLICCPLUSPLUS

#ifdef SYMBOLIC_DEFINE
#ifndef SYMBOLIC_CPLUSPLUS_SOLVE_DEFINE
#define SYMBOLIC_CPLUSPLUS_SOLVE_DEFINE
double find_root(Symbolic, Symbolic, double);

int solve_polynomials(const Symbolic &e, const Symbolic &x, Equations *soln) {
  if (e == 0) {
    *soln = (*soln, x == x);
    return 1;
  } else if (df(e, x, 2) == 0) {
    *soln = (*soln, x == -e.coeff(x, 0) / e.coeff(x, 1));
    return 1;
  } else if (df(e, x, 3) == 0) {
    Symbolic a = e.coeff(x, 2), b = e.coeff(x, 1), c = e.coeff(x, 0);
    Symbolic d = b * b - 4 * a * c;
    *soln =
        (*soln, x == (-b + sqrt(d)) / (2 * a), x == (-b - sqrt(d)) / (2 * a));
    return 1;
  } else if (df(e, x, 4) == 0) {
    Symbolic l = e / e.coeff(x, 3);
    Symbolic a1 = l.coeff(x, 2), a2 = l.coeff(x, 1), a3 = l.coeff(x, 0);
    Symbolic Q = (3 * a2 - a1 * a1) / 9,
             R = (9 * a1 * a2 - 27 * a3 - 2 * a1 * a1 * a1) / 54;
    Symbolic S1 = (R + sqrt(Q * Q * Q + R * R)) ^ (Symbolic(1) / 3);
    Symbolic S2 = (R - sqrt(Q * Q * Q + R * R)) ^ (Symbolic(1) / 3);
    *soln = (*soln, x == S1 + S2 - a1 / 3,
             x == -(S1 + S2) / 2 - a1 / 3 +
                      SymbolicConstant::i * sqrt(Symbolic(3)) * (S1 - S2),
             x == -(S1 + S2) / 2 - a1 / 3 -
                      SymbolicConstant::i * sqrt(Symbolic(3)) * (S1 - S2));
    return 1;
  }

  return 0;
}

int solve_inverse_eqn(const Symbolic &e, const Symbolic &x, Equations *soln) {
  if (e.coeff(x, -1) != 0) {
    Equations::iterator i;
    *soln = solve(x * e, x);
    for (i = (*soln).begin(); i != (*soln).end();)
      if (i->lhs == x && i->rhs == 0)
        i = (*soln).erase(i);
      else
        ++i;
    return 1;
  }

  return 0;
}

int solve_exponential(const Symbolic &e, const Symbolic &x, Equations *soln) {
  Symbolic a, b, c, z("z"), poly_expr;
  list<Equations> matches;
  UniqueSymbol am, bm, cm;

  matches = (bm * exp(am * x) + cm).match(e, (am, bm, cm));

  if (matches.size()) {
    a = rhs(matches.front(), am);
    b = rhs(matches.front(), bm);
    c = rhs(matches.front(), cm);

    *soln = (*soln, x == log(SymbolicConstant::e, -c / b) / a);
    return 1;
  }

  matches = (exp(am * x) + cm).match(e, (am, bm, cm));

  if (matches.size()) {
    a = rhs(matches.front(), am);
    c = rhs(matches.front(), cm);

    *soln = (*soln, x == log(SymbolicConstant::e, -c) / a);
    return 1;
  }

  matches = (bm * exp(am * (x ^ 2)) + cm).match(e, (am, bm, cm));

  if (matches.size()) {
    a = rhs(matches.front(), am);
    b = rhs(matches.front(), bm);
    c = rhs(matches.front(), cm);

    if (solve_polynomials(a * x * x - log(SymbolicConstant::e, -c / b), x,
                          soln)) {
      return 1;
    }
  }

  matches = (exp(am * (x ^ 2)) + cm).match(e, (am, bm, cm));

  if (matches.size()) {
    a = rhs(matches.front(), am);
    c = rhs(matches.front(), cm);

    if (solve_polynomials(a * x * x - log(SymbolicConstant::e, -c), x, soln)) {
      return 1;
    }
  }

  return 0;
}

int solve_numerical(const Symbolic &e, const Symbolic &x, Equations *soln) {
  *soln = (*soln, x == Symbolic(find_root(e, x, NEWTON_MAX_PRECISSION)));
  return 1;
}

Equations solve(const Symbolic &e, const Symbolic &x) {
  Equations soln;

  int (*solvers[])(const Symbolic &, const Symbolic &, Equations *) = {
      solve_polynomials,
      // TODO add generic method for generic functions
      solve_exponential,
      solve_inverse_eqn,
      solve_numerical,
  };

  for (int i = 0; i < sizeof(solvers) / sizeof(solvers[0]); i++) {
    if (solvers[i](e, x, &soln)) {
      return soln;
    }
  }

  return soln;
}

Equations solve(const Equation &e, const Symbolic &x) {
  return solve(e.lhs - e.rhs, x);
}

list<Equations> solve(const Equations &e, const list<Symbolic> &l) {
  int sc = 0, free = 1;
  list<Equations> soln;
  Equations::const_iterator i, j, k;
  list<Equations>::const_iterator u;
  list<Symbolic>::const_iterator li;

  if (e.empty()) {
    Equations eq;
    for (li = l.begin(); li != l.end(); ++li)
      eq = (eq, *li == *li);
    soln.push_back(eq);
  }
  if (l.empty())
    return soln; // no variables to solve for
  for (i = e.begin(); i != e.end(); ++i) {
    Symbolic eqi = i->lhs - i->rhs;
    if (df(eqi, l.front()) != 0) {
      Equations s = solve(eqi, l.front());
      free = 0;
      for (j = s.begin(); j != s.end(); ++j) {
        Equations eq;
        for (k = e.begin(); k != e.end(); ++k)
          if (k != i)
            eq.push_back(k->lhs->subst(j->lhs, j->rhs, sc) ==
                         k->rhs->subst(j->lhs, j->rhs, sc));
        list<Equations> slns = solve(eq, list<Symbolic>(++l.begin(), l.end()));
        for (u = slns.begin(); u != slns.end(); ++u)
          pattern_match_OR(soln, (j->lhs == j->rhs[*u], *u));
      }
    }
  }

  if (free) {
    if (l.size() == 1) {
      Equations s;
      s = (s, l.front() == l.front());
      soln.push_back(s);
    } else {
      list<Equations> slns = solve(e, list<Symbolic>(++l.begin(), l.end()));
      for (u = slns.begin(); u != slns.end(); ++u)
        soln.push_back((l.front() == l.front(), *u));
    }
  }

  return soln;
}

double find_root(Symbolic f, Symbolic x,
                 double MAX_PRECISSION = NEWTON_MAX_PRECISSION) {
  double x0, x1;
  Symbolic fd = df(f, x);

  x1 = 0;

  while (double(fd[x == x1]) == 0) {
    x1++;
  }

  do {
    x0 = x1;
    x1 = x0 - double(f[x == x1]) / double(fd[x == x1]);
  } while (fabs(x1 - x0) >= MAX_PRECISSION);

  return x1;
}

#endif
#endif

#undef LIBSYMBOLICCPLUSPLUS

#endif
