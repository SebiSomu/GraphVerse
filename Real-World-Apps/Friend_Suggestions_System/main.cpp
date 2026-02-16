#include "socialnetwork.h"
#include <iostream>

int main() {
    SocialNetwork net;

    net.addFriendship("Andrei", "Mihai");
    net.addFriendship("Andrei", "Elena");
    net.addFriendship("Andrei", "Radu");
    net.addFriendship("Mihai", "Elena");
    net.addFriendship("Mihai", "Bogdan");
    net.addFriendship("Elena", "Ioana");
    net.addFriendship("Radu", "Bogdan");
    net.addFriendship("Radu", "Cristina");
    net.addFriendship("Bogdan", "Cristina");
    net.addFriendship("Ioana", "Alexandra");
    net.addFriendship("Ioana", "Vlad");
    net.addFriendship("Alexandra", "Vlad");
    net.addFriendship("Alexandra", "Teodora");
    net.addFriendship("Vlad", "Marius");
    net.addFriendship("Teodora", "Marius");
    net.addFriendship("Marius", "Florin");
    net.addFriendship("Cristina", "Teodora");
    net.addFriendship("Bogdan", "Vlad");
    net.addFriendship("Mihai", "Florin");
    net.addFriendship("Andrei", "Catalin");
    net.addFriendship("Catalin", "Razvan");
    net.addFriendship("Razvan", "Oana");
    net.addFriendship("Oana", "Petra");
    net.addFriendship("Petra", "Silviu");
    net.addFriendship("Catalin", "Oana");
    net.addFriendship("Florin", "Dragos");
    net.addFriendship("Dragos", "Nicoleta");
    net.addFriendship("Nicoleta", "Sorin");
    net.addFriendship("Silviu", "Sorin");
    net.addFriendship("Sebi", "Cezar");

    net.printGraph();
    net.printSuggestionsAll(-1);

    return 0;
}