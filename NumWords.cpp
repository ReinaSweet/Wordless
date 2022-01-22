#include "NumWords.h"

uint16_t NumberWord::sMatchSetToMatchPattern[kNumWords][kNumWords] = { {0} };
std::vector<NumberWord> NumberWord::sAllWords;
std::vector<NumberWord> NumberWord::sAnswerWords;
std::vector<NumberWord> NumberWord::sDifficultToEvalWords;
std::vector<NumberWord> NumberWord::sVERYDifficultToEvalWords;
std::vector<NumberWord> NumberWord::s2049MatchedWords;

void NumberWord::Initialize()
{
    SwapToAllWords();

    for (size_t i = 0; i < kNumOfAnswerWords; ++i)
    {
        sAnswerWords.push_back(NumberWord::LookupNumberWord(kAnswerWords[i]));
    }

    for (size_t i = 0; i < kNumDifficultToEvalWords; ++i)
    {
        sDifficultToEvalWords.push_back(NumberWord::LookupNumberWord(kDifficultToEvalWords[i]));
    }

    for (size_t i = 0; i < kNumVERYDifficultToEvalWords; ++i)
    {
        sVERYDifficultToEvalWords.push_back(NumberWord::LookupNumberWord(kVERYDifficultToEvalWords[i]));
    }

    for (size_t i = 0; i < kNumFirst2049MatchedWords; ++i)
    {
        s2049MatchedWords.push_back(NumberWord::LookupNumberWord(kFirst2049MatchedWords[i]));
    }
}

void NumberWord::SwapToAllWords()
{
    sAllWords.clear();
    for (size_t i = 0; i < kNumWords; ++i)
    {
        sAllWords.emplace_back(kWordList[i], static_cast<uint16_t>(i));
    }
}

void NumberWord::SwapToSmallSetOfWords()
{
    sAllWords.clear();
    for (size_t i = 0; i < kNumWords; i += 32)
    {
        sAllWords.emplace_back(kWordList[i], static_cast<uint16_t>(i));
    }
}

NumberWord NumberWord::LookupNumberWord(const char* const text)
{
    for (size_t i = 0; i < kNumWords; ++i)
    {
        if (strcmp(text, kWordList[i]) == 0)
        {
            return NumberWord(text, static_cast<uint16_t>(i));
        }
    }
    return kInvalidNumberWord;
}
