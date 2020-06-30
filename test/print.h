/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/


#ifndef DTEST_PRINT_H
#define DTEST_PRINT_H

#include <string>

#include "dll.h"


void print_PBN(std::ostream &out, const dealPBN& dl);

void print_FUT(std::ostream &out, const futureTricks& fut);

void print_TABLE(std::ostream &out, const ddTableResults& table);

void print_PAR(std::ostream &out, const parResults& par);

void print_DEALERPAR(std::ostream &out, const parResultsDealer& par);

void print_PLAY(std::ostream &out, const playTracePBN& play);

void print_TRACE(std::ostream &out, const solvedPlay& solved);

void print_double_TRACE(std::ostream &out,
  const solvedPlay& solved,
  const solvedPlay& ref);

#endif

