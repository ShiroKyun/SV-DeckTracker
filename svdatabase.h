#ifndef SVDATABASE_H
#define SVDATABASE_H

#include <QHash>
#include "card.h"
#include <QString>
#include <QPixmap>

typedef unsigned long long int ulong64;

class svDatabase
{
public:
    svDatabase();
    void addCard(int id, Card card);
    Card getCard(int id);
    const QPixmap *getPortrait(int id);
    const QPixmap *getCost(int cost);
    std::vector<ulong64> getCostPHash();
    int getCostfromPHash(ulong64);
    void load();
    int size();
private:
    QHash<int , Card> cardMap;
    QHash<int , QPixmap> portraitMap;
    QHash<int , QPixmap> costMap;
    QHash<int , ulong64> costPHashMap;
};

#endif // SVDATABASE_H
