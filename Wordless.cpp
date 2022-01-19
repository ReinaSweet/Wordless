#include "stdafx.h"

#include "ConsoleInfo.h"
#include "ConsoleMenu.h"

#include "Words.h"
#include "NumWords.h"

#include "CommandNumWordA.h"
#include "CommandNumWordB.h"

int main()
{
    putchar('\n');

    NumberWord::Initialize();

    ConsoleMenu mainMenu("Main Menu");

    ConsoleMenu menuNumWordAExploration("NumWord Exploration A", mainMenu);
    menuNumWordAExploration.AddCommand("nf", "Find first optimal NumWord", NumWordA::COMMAND_OptimalFirstNumWord);
    menuNumWordAExploration.AddCommand("nt", "Tangy Test", NumWordA::COMMAND_TangyTest);
    menuNumWordAExploration.AddCommand("na", "Abele Test", NumWordA::COMMAND_AbeleTest);
    menuNumWordAExploration.AddCommand("nz", "Zills Test", NumWordA::COMMAND_ZillsTest);
    menuNumWordAExploration.AddCommand("nc", "Comprehensive Test", NumWordA::COMMAND_ComprehensiveTest);
    menuNumWordAExploration.AddCommand("nds", "Small Dictionary Size", NumWordA::COMMAND_SmallDictSize);
    menuNumWordAExploration.AddCommand("ndf", "Full Dictionary Size", NumWordA::COMMAND_FullDictSize);
    menuNumWordAExploration.AddCommand("sw", "Test Specific Word", NumWordA::COMMAND_SpecificWord);
    menuNumWordAExploration.AddCommand("ofwa", "Find optimal first word (all)", NumWordA::COMMAND_FindOptimalFirstWord);
    menuNumWordAExploration.AddCommand("ofwb", "Find optimal first word (difficult only)", NumWordA::COMMAND_FindOptimalFirstWordFromDifficultWordsOnly);
    menuNumWordAExploration.AddCommand("ofwc", "Find optimal first word (VERY difficult only)", NumWordA::COMMAND_FindOptimalFirstWordFromVERYDifficultWordsOnly);
    menuNumWordAExploration.AddCommand("ofws", "Find optimal first word (difficult only, special starts)", NumWordA::COMMAND_FindOptimalFirstWordLimitedFilter);
    menuNumWordAExploration.AddCommand("ssw", "Find all words with specific second word", NumWordA::COMMAND_FindAllWordsWithSpecificSecondWord);
    menuNumWordAExploration.AddCommand("ssp", "Find all words with specific second pattern;d", NumWordA::COMMAND_FindAllWordsWithSpecificSecondPattern);
    menuNumWordAExploration.AddCommand("ctt", "Comprehensive But Limited to 2049 Matched Test", NumWordA::COMMAND_ComprehensiveLimitedTo2049Test);

    ConsoleMenu menuNumWordBExploration("NumWord Exploration B (current)", mainMenu);
    menuNumWordBExploration.AddCommand("osw", "Find optimal second word", NumWordB::COMMAND_FindOptimalSecondWord);
    menuNumWordBExploration.AddCommand("sw", "Test Specific Word", NumWordB::COMMAND_SpecificWord);
    menuNumWordBExploration.AddCommand("ctfw", "Comprehensive Test from Set of First Words", NumWordB::COMMAND_ComprehensiveTestFromSetOfFirstWords);
    menuNumWordBExploration.AddCommand("ctsw", "Comprehensive Test from Set of Seconds Words", NumWordB::COMMAND_ComprehensiveTestFromSetOfSecondWords);
    menuNumWordBExploration.AddCommand("cta", "Comprehensive Test", NumWordB::COMMAND_ComprehensiveTest);
    menuNumWordBExploration.AddCommand("ctb", "Comprehensive Test - 2049 Words Only", NumWordB::COMMAND_ComprehensiveTest2049WordsOnly);
    menuNumWordBExploration.AddCommand("sssw", "Test Specific Start, Specific Word;Start;Target Word", NumWordB::COMMAND_SpecificStartSpecificWord);
    menuNumWordBExploration.AddCommand("grrrrr", "Try to find any optimal word for double 2049", NumWordB::COMMAND_IsThereAnyOptimalWordForDouble2049);

    mainMenu.AddSubmenu("nwa", menuNumWordAExploration);
    mainMenu.AddSubmenu("nwb", menuNumWordBExploration);
    mainMenu.AddCommand("wordle", "Play Wordle Solver", NumWordB::COMMAND_PlayWordle);

    mainMenu.ResetMenu();

    char c;
    do
    {
        c = _getch();
    } while (mainMenu.ReceiveInput(c));

    return 0;
}
