#pragma once
#include "stdafx.h"
#include "NumWords.h"

namespace NumWordA
{
NumberWord FindOptimalNumWordViaMatchingPatterns(const std::vector<NumberWord>& remainingPossibilities)
{
    if (remainingPossibilities.size() < 3)
    {
        return remainingPossibilities.at(0);
    }

    // What testWord, when applied, would result in a set of match patterns with the least entries in its largest sized match

    uint16_t currentLowestLargeSetSize = UINT16_MAX;
    NumberWord currentLowestLargeSetWord = kInvalidNumberWord;

    for (const NumberWord& testWord : NumberWord::sAllWords)
    {
        uint16_t matchPatterns[1024] = { 0 };
        for (const NumberWord& possibility : remainingPossibilities)
        {
            const uint16_t pattern = possibility.GetTestResults(testWord) - kEarmarkMatchPattern;
            matchPatterns[pattern] += 1;
        }

        uint16_t largestSetSize = 0;
        for (size_t i = 0; i < 1024; ++i)
        {
            if (matchPatterns[i] > largestSetSize)
            {
                largestSetSize = matchPatterns[i];
            }
        }

        if (largestSetSize < currentLowestLargeSetSize)
        {
            currentLowestLargeSetSize = largestSetSize;
            currentLowestLargeSetWord = testWord;
        }
    }

    return currentLowestLargeSetWord;
}

void PairDownSetForWordFromEntryWord(uint16_t sourceMatch, const NumberWord& entryWord, const std::vector<NumberWord>& sourcePossibilities, std::vector<NumberWord>& outNewPossibilities)
{
    for (const NumberWord& possibility : sourcePossibilities)
    {
        if (possibility.IsValidWith(entryWord, sourceMatch))
        {
            outNewPossibilities.emplace_back(possibility);
        }
    }
}

void PrintWordAndPossibilities(const NumberWord& word, const uint16_t results, const std::vector<NumberWord>& possibilities, const uint32_t maxLines, const uint32_t wordsPerLine)
{
    std::string textForWord;
    word.ConvertToString(textForWord);
    printf("\n%s %hu: ", textForWord.c_str(), results);

    size_t index = 0;

    for (size_t line = 0; line < maxLines; ++line)
    {
        for (size_t word = 0; word < wordsPerLine; ++word)
        {
            if (index >= possibilities.size())
            {
                break;
            }

            possibilities[index].ConvertToString(textForWord);
            printf("%s, ", textForWord.c_str());

            ++index;
        }

        if ((index >= possibilities.size()) || ((line + 1) == maxLines))
        {
            break;
        }

        printf("...\n        ");
    }

    if (possibilities.size() > index)
    {
        printf("...");
    }
    putchar('\n');
}

void COMMAND_OptimalFirstNumWord()
{
    NumberWord optimalWord = FindOptimalNumWordViaMatchingPatterns(NumberWord::sAllWords);
    std::string textForWord;
    optimalWord.ConvertToString(textForWord);
    printf("\n\n%s\n", textForWord.c_str());
}

void TestSpecificWord(const NumberWord& targetWord)
{
    std::string textForWord;

    const NumberWord firstPairing(kSupposedMostOptimizedFirstWord);

    printf("\n\n");

    std::vector<NumberWord>* possibilities = new std::vector<NumberWord>();
    NumberWord optimalWord(kInvalidNumberWord);

    // First Step
    {
        const uint16_t results = targetWord.GetTestResults(firstPairing);
        PairDownSetForWordFromEntryWord(results,
            firstPairing, NumberWord::sAllWords, *possibilities);
        PrintWordAndPossibilities(firstPairing, results, *possibilities, 4, 18);
        optimalWord = FindOptimalNumWordViaMatchingPatterns(*possibilities);
    }

    for (size_t i = 0; i < 12; ++i)
    {
        std::vector<NumberWord>* newPossibilities = new std::vector<NumberWord>();
        const uint16_t results = targetWord.GetTestResults(optimalWord);
        PairDownSetForWordFromEntryWord(results,
            optimalWord, *possibilities, *newPossibilities);
        PrintWordAndPossibilities(optimalWord, results, *newPossibilities, 4, 18);

        if (i == 3)
        {
            optimalWord = newPossibilities->at(0);
        }
        else
        {
            optimalWord = FindOptimalNumWordViaMatchingPatterns(*newPossibilities);
        }

        delete possibilities;
        possibilities = newPossibilities;

        if (i == 4)
        {
            printf("====END====\n");
        }
        if (optimalWord == targetWord)
        {
            optimalWord.ConvertToString(textForWord);
            printf("%s", textForWord.c_str());
            break;
        }
    }
    delete possibilities;
}

void COMMAND_SpecificWord(const char* const word)
{
    const NumberWord targetWord(NumberWord::LookupNumberWord(word));
    TestSpecificWord(targetWord);
}

void COMMAND_AbeleTest()
{
    const NumberWord targetWord(NumberWord::LookupNumberWord("abele"));
    TestSpecificWord(targetWord);
}

void COMMAND_JapesTest()
{
    const NumberWord targetWord(NumberWord::LookupNumberWord("japes"));
    TestSpecificWord(targetWord);
}

void COMMAND_TangyTest()
{
    const NumberWord targetWord(NumberWord::LookupNumberWord("tangy"));
    TestSpecificWord(targetWord);
}

void COMMAND_ZillsTest()
{
    const NumberWord targetWord(NumberWord::LookupNumberWord("zills"));
    TestSpecificWord(targetWord);
}

void COMMAND_SmallDictSize()
{
    NumberWord::SwapToSmallSetOfWords();
}

void COMMAND_FullDictSize()
{
    NumberWord::SwapToAllWords();
}

void COMMAND_ComprehensiveTest()
{
    std::string textForWord;
    const NumberWord firstPairing(kSupposedMostOptimizedFirstWord);

    std::map<uint32_t, uint32_t> stepsToWinningWords;
    std::map<uint32_t, std::vector<NumberWord>> stepsToLosingWords;

    for (const NumberWord& targetWord : NumberWord::sAllWords)
    {
        targetWord.ConvertToString(textForWord);
        printf("%s,  ", textForWord.c_str());

        std::vector<NumberWord>* possibilities = new std::vector<NumberWord>();
        NumberWord optimalWord(kInvalidNumberWord);

        uint32_t steps = 0;

        if (targetWord != firstPairing)
        {
            PairDownSetForWordFromEntryWord(targetWord.GetTestResults(firstPairing),
                firstPairing, NumberWord::sAllWords, *possibilities);
            ++steps;

            for (; steps < 12; ++steps)
            {
                // Last step, we can't get any more info. Don't pair down and instead just choose one
                if (steps == 5)
                {
                    optimalWord = possibilities->at(0);
                }
                else
                {
                    optimalWord = FindOptimalNumWordViaMatchingPatterns(*possibilities);
                }

                if (optimalWord == targetWord)
                {
                    break;
                }

                std::vector<NumberWord>* newPossibilities = new std::vector<NumberWord>();
                PairDownSetForWordFromEntryWord(targetWord.GetTestResults(optimalWord),
                    optimalWord, *possibilities, *newPossibilities);

                delete possibilities;
                possibilities = newPossibilities;
            }
        }
        delete possibilities;

        std::map<uint32_t, uint32_t>::iterator it = stepsToWinningWords.find(steps);
        if (it == stepsToWinningWords.end())
        {
            stepsToWinningWords[steps] = 1;
        }
        else
        {
            stepsToWinningWords[steps] = (*it).second + 1;
        }

        if (steps > 4)
        {
            stepsToLosingWords[steps].push_back(targetWord);
        }
    }

    printf("\n\n===Wins===");
    for (auto& pair : stepsToWinningWords)
    {
        printf("\n Steps %u --- Wins: %u", pair.first + 1u, pair.second);
        if (pair.first > 4)
        {
            uint32_t perLine = 0;
            printf(" : ");
            for (const NumberWord& losingNumber : stepsToLosingWords[pair.first])
            {
                losingNumber.ConvertToString(textForWord);
                printf("%s, ", textForWord.c_str());
                ++perLine;
                if (perLine > 16)
                {
                    perLine = 0;
                    printf("\n          ");
                }
            }
        }
    }
}


void FindOptimalFirstWord(const std::vector<NumberWord>& firstPairings, const std::vector<NumberWord>& evaluateThroughWords, uint32_t incrementVisualEveryNWords)
{
    std::string textForWord;

    uint32_t lowestTotalFailed = UINT32_MAX;
    std::vector<NumberWord> lowestFailedWords;

    for (const NumberWord& firstPairing : firstPairings)
    {
        firstPairing.ConvertToString(textForWord);
        printf("\n%s : ", textForWord.c_str());

        uint32_t visualIncrementControls = 0;
        uint32_t totalFailed = 0;

        //
        for (const NumberWord& targetWord : evaluateThroughWords)
        {
            std::vector<NumberWord>* possibilities = new std::vector<NumberWord>();
            NumberWord optimalWord(kInvalidNumberWord);

            uint32_t steps = 0;

            if (targetWord != firstPairing)
            {
                PairDownSetForWordFromEntryWord(targetWord.GetTestResults(firstPairing),
                    firstPairing, NumberWord::sAllWords, *possibilities);
                ++steps;

                for (; steps < 12; ++steps)
                {
                    // Last step, we can't get any more info. Don't pair down and instead just choose one
                    if (steps == 5)
                    {
                        optimalWord = possibilities->at(0);
                    }
                    else
                    {
                        optimalWord = FindOptimalNumWordViaMatchingPatterns(*possibilities);
                    }

                    if (optimalWord == targetWord)
                    {
                        break;
                    }

                    std::vector<NumberWord>* newPossibilities = new std::vector<NumberWord>();
                    PairDownSetForWordFromEntryWord(targetWord.GetTestResults(optimalWord),
                        optimalWord, *possibilities, *newPossibilities);

                    delete possibilities;
                    possibilities = newPossibilities;
                }
            }
            delete possibilities;

            if (steps > 5)
            {
                ++totalFailed;
                if (totalFailed > lowestTotalFailed)
                {
                    break;
                }
            }

            ++visualIncrementControls;
            if (visualIncrementControls > incrementVisualEveryNWords)
            {
                visualIncrementControls = 0;
                putchar((char)178);
            }
        }
        //

        printf(" - FAILED: %u", totalFailed);

        if (totalFailed < lowestTotalFailed)
        {
            lowestTotalFailed = totalFailed;
            lowestFailedWords.clear();
            lowestFailedWords.emplace_back(firstPairing);
        }
        else if (totalFailed == lowestTotalFailed)
        {
            lowestFailedWords.emplace_back(firstPairing);
        }
    }

    printf("\n\n With %u fails, the lowest failing starting words:\n", lowestTotalFailed);
    for (const NumberWord& word : lowestFailedWords)
    {
        word.ConvertToString(textForWord);
        printf("%s, ", textForWord.c_str());
    }
}

void COMMAND_FindOptimalFirstWord()
{
    FindOptimalFirstWord(NumberWord::sAllWords, NumberWord::sAllWords, 92);
}

void COMMAND_FindOptimalFirstWordFromDifficultWordsOnly()
{
    FindOptimalFirstWord(NumberWord::sAllWords, NumberWord::sDifficultToEvalWords, 5);
}

void COMMAND_FindOptimalFirstWordFromVERYDifficultWordsOnly()
{
    FindOptimalFirstWord(NumberWord::sAllWords, NumberWord::sVERYDifficultToEvalWords, 0);
}

void COMMAND_FindOptimalFirstWordLimitedFilter()
{
    std::vector<NumberWord> limitedStarts;
    const char charStarts[11][kWordLengthPlusNullByte] = {
        "bevel", "compt", "feted", "finks", "fiver", "kevil", "sarks", "stowp", "swarf", "vises", "visor"
    };
    for (size_t i = 0; i < 11; ++i)
    {
        limitedStarts.push_back(NumberWord::LookupNumberWord(charStarts[i]));
    }

    FindOptimalFirstWord(limitedStarts, NumberWord::sDifficultToEvalWords, 5);
}

void COMMAND_FindAllWordsWithSpecificSecondWord(const char* const secondWordText)
{
    std::string textForWord;
    const NumberWord firstPairing(kSupposedMostOptimizedFirstWord);
    const NumberWord secondWord = NumberWord::LookupNumberWord(secondWordText);

    for (const NumberWord& targetWord : NumberWord::sAllWords)
    {
        if (targetWord != firstPairing)
        {
            std::vector<NumberWord> possibilities;
            PairDownSetForWordFromEntryWord(targetWord.GetTestResults(firstPairing),
                firstPairing, NumberWord::sAllWords, possibilities);
            const NumberWord optimalWord = FindOptimalNumWordViaMatchingPatterns(possibilities);
            if (optimalWord == secondWord)
            {
                targetWord.ConvertToString(textForWord);
                printf("%s, ", textForWord.c_str());
            }
        }
    }
}

void COMMAND_FindAllWordsWithSpecificSecondPattern(uint64_t patternLong)
{
    std::string textForWord;
    const NumberWord firstPairing(kSupposedMostOptimizedFirstWord);
    const uint16_t targetPattern = (uint16_t)patternLong;

    for (const NumberWord& targetWord : NumberWord::sAllWords)
    {
        if (targetWord != firstPairing)
        {
            const uint16_t secondPattern = targetWord.GetTestResults(firstPairing);

            if (targetPattern == secondPattern)
            {
                targetWord.ConvertToString(textForWord);
                printf("%s, ", textForWord.c_str());
            }
        }
    }
}

void COMMAND_ComprehensiveLimitedTo2049Test()
{
    std::string textForWord;
    const NumberWord firstPairing(kSupposedMostOptimizedFirstWord);

    std::map<uint32_t, uint32_t> stepsToWinningWords;
    std::map<uint32_t, std::vector<NumberWord>> stepsToLosingWords;

    for (const NumberWord& targetWord : NumberWord::s2049MatchedWords)
    {
        targetWord.ConvertToString(textForWord);
        printf("%s,  ", textForWord.c_str());

        std::vector<NumberWord>* possibilities = new std::vector<NumberWord>();
        NumberWord optimalWord(kInvalidNumberWord);

        uint32_t steps = 0;

        if (targetWord != firstPairing)
        {
            PairDownSetForWordFromEntryWord(targetWord.GetTestResults(firstPairing),
                firstPairing, NumberWord::sAllWords, *possibilities);
            ++steps;

            for (; steps < 12; ++steps)
            {
                // Last step, we can't get any more info. Don't pair down and instead just choose one
                if (steps == 5)
                {
                    optimalWord = possibilities->at(0);
                }
                else
                {
                    optimalWord = FindOptimalNumWordViaMatchingPatterns(*possibilities);
                }

                if (optimalWord == targetWord)
                {
                    break;
                }

                std::vector<NumberWord>* newPossibilities = new std::vector<NumberWord>();
                PairDownSetForWordFromEntryWord(targetWord.GetTestResults(optimalWord),
                    optimalWord, *possibilities, *newPossibilities);

                delete possibilities;
                possibilities = newPossibilities;
            }
        }
        delete possibilities;

        std::map<uint32_t, uint32_t>::iterator it = stepsToWinningWords.find(steps);
        if (it == stepsToWinningWords.end())
        {
            stepsToWinningWords[steps] = 1;
        }
        else
        {
            stepsToWinningWords[steps] = (*it).second + 1;
        }

        if (steps > 3)
        {
            stepsToLosingWords[steps].push_back(targetWord);
        }
    }

    printf("\n\n===Wins===");
    for (auto& pair : stepsToWinningWords)
    {
        printf("\n Steps %u --- Wins: %u", pair.first + 1u, pair.second);
        if (pair.first > 3)
        {
            uint32_t perLine = 0;
            printf(" : ");
            for (const NumberWord& losingNumber : stepsToLosingWords[pair.first])
            {
                losingNumber.ConvertToString(textForWord);
                printf("%s, ", textForWord.c_str());
                ++perLine;
                if (perLine > 16)
                {
                    perLine = 0;
                    printf("\n          ");
                }
            }
        }
    }
}

}// namespace NumWordA