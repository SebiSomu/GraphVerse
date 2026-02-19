#include "translationnetwork.h"
#include <iostream>
#include <vector>
#include <tuple>
#include <cstdlib>
#include <ctime>

void buildNetwork(TranslationNetwork& net) {
    // Format: {lang1, lang2, cost}
    std::vector<std::tuple<std::string, std::string, int>> connections = {
        // Romance family (1-3)
        {"Spanish", "French", 2}, {"Spanish", "Portuguese", 1}, {"Spanish", "Italian", 2},
        {"Spanish", "Romanian", 2}, {"Spanish", "Catalan", 2}, {"Spanish", "Galician", 1},
        {"French", "Portuguese", 2}, {"French", "Italian", 1}, {"French", "Romanian", 2},
        {"French", "Catalan", 2}, {"French", "Occitan", 1}, {"Portuguese", "Italian", 2},
        {"Portuguese", "Romanian", 2}, {"Portuguese", "Catalan", 1}, {"Italian", "Romanian", 1},
        {"Italian", "Catalan", 2}, {"Italian", "Occitan", 2}, {"Catalan", "Galician", 1},

        // Germanic family (2-4)
        {"English", "German", 3}, {"English", "Dutch", 3}, {"English", "Swedish", 3},
        {"English", "Danish", 3}, {"English", "Norwegian", 3}, {"English", "Afrikaans", 3},
        {"German", "Dutch", 2}, {"German", "Swedish", 3}, {"German", "Danish", 3},
        {"German", "Norwegian", 3}, {"Swedish", "Danish", 2}, {"Swedish", "Norwegian", 2},
        {"Norwegian", "Danish", 1}, {"Danish", "Afrikaans", 3}, {"Dutch", "Afrikaans", 3},

        // Slavic family (2-4)
        {"Russian", "Ukrainian", 2}, {"Russian", "Polish", 3}, {"Russian", "Czech", 3},
        {"Russian", "Bulgarian", 3}, {"Russian", "Serbian", 2}, {"Ukrainian", "Polish", 3},
        {"Ukrainian", "Bulgarian", 2}, {"Ukrainian", "Belarusian", 2}, {"Polish", "Czech", 2},
        {"Polish", "Slovak", 2}, {"Polish", "Croatian", 3}, {"Czech", "Slovak", 1},
        {"Czech", "Serbian", 3}, {"Bulgarian", "Serbian", 2}, {"Serbian", "Croatian", 1},
        {"Serbian", "Slovenian", 2}, {"Croatian", "Slovenian", 2}, {"Belarusian", "Polish", 3},

        // Sino-Tibetan family (4-7)
        {"Mandarin", "Cantonese", 4}, {"Mandarin", "Wu", 5}, {"Mandarin", "Burmese", 6},
        {"Mandarin", "Tibetan", 5}, {"Cantonese", "Wu", 4},

        // Indo-Iranian family (2-5)
        {"Hindi", "Urdu", 1}, {"Hindi", "Bengali", 4}, {"Hindi", "Punjabi", 3},
        {"Hindi", "Marathi", 3}, {"Urdu", "Punjabi", 2}, {"Urdu", "Persian", 3},
        {"Bengali", "Punjabi", 3}, {"Persian", "Kurdish", 3}, {"Persian", "Pashto", 4},
        {"Kurdish", "Pashto", 3},

        // Finno-Ugric family (5-7)
        {"Finnish", "Estonian", 5}, {"Finnish", "Hungarian", 6}, {"Estonian", "Hungarian", 6},

        // Semitic family (3-6)
        {"Arabic", "Hebrew", 4}, {"Arabic", "Amharic", 5}, {"Arabic", "Tigrinya", 4},
        {"Arabic", "Maltese", 4}, {"Hebrew", "Amharic", 5}, {"Hebrew", "Tigrinya", 5},

        // Japonic & Koreanic (8-9)
        {"Japanese", "Korean", 8},

        // Dravidian family (2-4)
        {"Tamil", "Telugu", 3}, {"Tamil", "Kannada", 3}, {"Tamil", "Malayalam", 3},
        {"Telugu", "Kannada", 2}, {"Telugu", "Malayalam", 3},

        // Niger-Congo (African) family (5-8)
        {"Swahili", "Hausa", 6}, {"Swahili", "Yoruba", 6}, {"Swahili", "Zulu", 6},
        {"Swahili", "Igbo", 7}, {"Hausa", "Yoruba", 5}, {"Hausa", "Fula", 5},
        {"Yoruba", "Igbo", 6}, {"Zulu", "Shona", 5},

        // Austronesian family (2-4)
        {"Indonesian", "Malay", 1}, {"Indonesian", "Tagalog", 3}, {"Indonesian", "Javanese", 2},
        {"Malay", "Tagalog", 3}, {"Vietnamese", "Thai", 6},

        // Turkic family (2-3)
        {"Turkish", "Azerbaijani", 2}, {"Turkish", "Uzbek", 3}, {"Turkish", "Kazakh", 3},
        {"Azerbaijani", "Uzbek", 2}, {"Uzbek", "Kazakh", 2},

        // Other isolated/distinct languages (6-9)
        {"Greek", "Armenian", 7}, {"Greek", "Georgian", 8}, {"Greek", "Albanian", 7},
        {"Armenian", "Georgian", 6}, {"Georgian", "Albanian", 7}, {"Basque", "Spanish", 8},

        // English as global hub
        {"English", "Spanish", 4}, {"English", "French", 3}, {"English", "Portuguese", 4},
        {"English", "Italian", 3}, {"English", "Romanian", 5}, {"English", "Russian", 6},
        {"English", "Ukrainian", 6}, {"English", "Polish", 5}, {"English", "Czech", 5},
        {"English", "Bulgarian", 6}, {"English", "Serbian", 6}, {"English", "Arabic", 7},
        {"English", "Hebrew", 6}, {"English", "Hindi", 5}, {"English", "Bengali", 6},
        {"English", "Mandarin", 8}, {"English", "Japanese", 9}, {"English", "Korean", 8},
        {"English", "Turkish", 6}, {"English", "Swahili", 5}, {"English", "Indonesian", 6},
        {"English", "Vietnamese", 7}, {"English", "Thai", 8}, {"English", "Finnish", 7},
        {"English", "Hungarian", 8}, {"English", "Basque", 10},

        // Mandarin as Asian hub
        {"Mandarin", "Japanese", 8}, {"Mandarin", "Korean", 7}, {"Mandarin", "Hindi", 8},
        {"Mandarin", "Bengali", 9}, {"Mandarin", "Russian", 8}, {"Mandarin", "Vietnamese", 6},
        {"Mandarin", "Thai", 7},

        // French as Romance/African hub
        {"French", "Swahili", 6}, {"French", "Hausa", 5}, {"French", "Yoruba", 6},
        {"French", "Fula", 5},

        // Arabic as Semitic/African hub
        {"Arabic", "Turkish", 6}, {"Arabic", "Persian", 5}, {"Arabic", "Urdu", 6},
        {"Arabic", "Swahili", 6}, {"Arabic", "Hausa", 5}, {"Arabic", "Yoruba", 7},

        // Hindi as Indo-Iranian/Dravidian hub
        {"Hindi", "Tamil", 6}, {"Hindi", "Telugu", 5}, {"Hindi", "Kannada", 5},

        // Russian as Slavic/Finno-Ugric/Turkic hub
        {"Russian", "Finnish", 7}, {"Russian", "Estonian", 7}, {"Russian", "Hungarian", 8},
        {"Russian", "Turkish", 6}, {"Russian", "Azerbaijani", 6}, {"Russian", "Kazakh", 6}
    };

    for (const auto& [lang1, lang2, cost] : connections) {
        net.addTranslationPair(lang1, lang2, cost);
    }
}

int main() {
    srand(static_cast<unsigned>(time(0)));
    TranslationNetwork net;

    buildNetwork(net);

    std::cout << "=== Translation Network - Kruskal MST ===\n";
    std::cout << "Problem: Given 75 of the world's most spoken languages,\n";
    std::cout << "find the minimum set of translator specialists needed so\n";
    std::cout << "all languages can communicate (possibly via relay translation).\n\n";
    std::cout << "Cost represents training difficulty (1=easy, 10=very hard)\n";

    net.computeAndPrintMST();
    std::cout << "\n" << std::string(60, '=') << "\n";
    net.printMSTAsTree("English");

    std::cout << "\n\nPress Enter to see full graph...";
    std::cin.get();
    net.printFullGraph();

    return 0;
}