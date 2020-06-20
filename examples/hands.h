/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2016 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/

#include <dll.h>
#include <hands_export.h>

// General initialization of three hands to be used in examples.
extern HANDS_EXPORT int trump[3];
extern HANDS_EXPORT int first[3];
extern HANDS_EXPORT int dealer[3];
extern HANDS_EXPORT int vul[3];

extern HANDS_EXPORT char PBN[3][80];

extern HANDS_EXPORT unsigned int holdings[3][4][4];

extern HANDS_EXPORT int playNo[3];

extern HANDS_EXPORT char play[3][106];
extern HANDS_EXPORT int playSuit[3][52];
extern HANDS_EXPORT int playRank[3][52];


HANDS_EXPORT void PrintFut(char title[], futureTricks * fut);
HANDS_EXPORT void equals_to_string(int equals, char * res);
HANDS_EXPORT bool CompareFut(futureTricks * fut, int handno, int solutions);

HANDS_EXPORT void SetTable(ddTableResults * table, int handno);
HANDS_EXPORT bool CompareTable(ddTableResults * table, int handno);
HANDS_EXPORT void PrintTable(ddTableResults * table);

HANDS_EXPORT bool ComparePar(parResults * par, int handno);
HANDS_EXPORT bool CompareDealerPar(parResultsDealer * par, int handno);
HANDS_EXPORT void PrintPar(parResults * par);
HANDS_EXPORT void PrintDealerPar(parResultsDealer * par);

HANDS_EXPORT bool ComparePlay(solvedPlay * trace, int handno);
HANDS_EXPORT void PrintBinPlay(playTraceBin * play, solvedPlay * solved);
HANDS_EXPORT void PrintPBNPlay(playTracePBN * play, solvedPlay * solved);


HANDS_EXPORT void PrintHand(char title[],
  unsigned int rankInSuit[DDS_HANDS][DDS_SUITS]);

HANDS_EXPORT void PrintPBNHand(char title[], char remainCards[]);

HANDS_EXPORT int ConvertPBN(char * dealBuff,
  unsigned int remainCards[DDS_HANDS][DDS_SUITS]);

HANDS_EXPORT int IsACard(char cardChar);

