#ifndef CARDLIST_H
#define CARDLIST_H

#include "card.h"
#include "perceptualhash.h"
#include "svdatabase.h"

#include <vector>
class cardlist
{
public:
    std::vector<int> cardsInDeck;   //has id of all cards in deck EASIER TO STORE
    std::vector<int> countInDeck;   //has the counter per card
    std::vector<ulong64> deckPHash; //updates as the same as the upper 2
    void addCard(int id);
    void removeCard(int id);
    void clear(){deckName.clear(); description.clear(); cardsInDeck.clear(); countInDeck.clear(); deckPHash.clear();}
    int getPosition(int id);
    std::string getName(){return deckName;}
    std::string getDescription(){return description;}
    void setName(std::string name){deckName = name;}
    void setDesc(std::string desc){description = desc;}
    cardlist(svDatabase* database);          //name of card
private:
    std::string deckName;
    std::string description;
    svDatabase* databasePtr;

};

#endif // CARDLIST_H
