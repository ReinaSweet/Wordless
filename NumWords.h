#pragma once
#include "Words.h"
#include "stdafx.h"

enum class SingleMatch : uint16_t
{
    kMiss = 0b00,
    kPartial = 0b01,
    kHit = 0b10,
    kAnyMask = 0b11
};

const uint32_t kBitsPerLetter = 5;
const uint16_t kBitsPerMatch = 2;

constexpr uint16_t GenerateMatchMask(const SingleMatch type)
{
    uint16_t mask = 0;
    for (size_t i = 0; i < kWordLength; ++i)
    {
        mask += static_cast<uint16_t>(type) << (i * kBitsPerMatch);
    }
    return mask;
}

const uint32_t kLetterMatchBitmask[kWordLength] = {
    0b0000000000000000000011111,
    0b0000000000000001111100000,
    0b0000000000111110000000000,
    0b0000011111000000000000000,
    0b1111100000000000000000000
};

const uint16_t kEarmarkMatchPattern = 0b100000000000;

const uint16_t kMatchPartialMask = GenerateMatchMask(SingleMatch::kPartial);
const uint16_t kMatchHitMask = GenerateMatchMask(SingleMatch::kHit);

const uint16_t kPerfectMatch = kEarmarkMatchPattern + kMatchHitMask;

class NumberWord
{
public:
    NumberWord(const char* wordText, uint16_t index)
    {
        mValue = 0;
        for (size_t i = 0; i < kWordLength; ++i)
        {
            // a == 1, z == 26
            mValue += static_cast<uint32_t>(wordText[i] - 'a' + 1) << (i * kBitsPerLetter);
        }
        mIndex = index;
    }

    NumberWord(uint32_t wordRawNum, uint16_t index)
        : mValue(wordRawNum), mIndex(index) {}
    NumberWord(const NumberWord& copy) 
        : mValue(copy.mValue), mIndex(copy.mIndex) {}
    NumberWord& operator=(const NumberWord& copy)
    {
        mValue = copy.mValue;
        mIndex = copy.mIndex;
        return *this;
    }
    bool operator==(const NumberWord& other) const { return mValue == other.mValue; }
    bool operator!=(const NumberWord& other) const { return mValue != other.mValue; }


    uint16_t GetTestResults(const NumberWord& testWord) const
    {
        if (sMatchSetToMatchPattern[mIndex][testWord.mIndex] != 0)
        {
            return sMatchSetToMatchPattern[mIndex][testWord.mIndex];
        }

        uint32_t remainingValue = mValue;
        uint32_t remainingTest = testWord.mValue;

        uint16_t testResult = 0;
        for (size_t i = 0; i < kWordLength; ++i)
        {
            if ((remainingTest & kLetterMatchBitmask[i]) == (remainingValue & kLetterMatchBitmask[i]))
            {
                testResult += static_cast<uint16_t>(SingleMatch::kHit) << (i * kBitsPerMatch);
                remainingValue &= ~kLetterMatchBitmask[i];
                remainingTest &= ~kLetterMatchBitmask[i];
            }
        }

        for (size_t i = 0; i < kWordLength; ++i)
        {
            const uint32_t remainingTestLetter = (remainingTest & kLetterMatchBitmask[i]) >> (i * kBitsPerLetter);
            if (remainingTestLetter != 0)
            {
                for (size_t t = 0; t < kWordLength; ++t)
                {
                    const uint32_t remainingValueLetter = (remainingValue & kLetterMatchBitmask[t]) >> (t * kBitsPerLetter);
                    if (remainingTestLetter == remainingValueLetter)
                    {
                        testResult += static_cast<uint16_t>(SingleMatch::kPartial) << (i * kBitsPerMatch);
                        remainingValue &= ~kLetterMatchBitmask[t];
                        break;
                    }
                }
            }
        }

        testResult += kEarmarkMatchPattern; // Earmark, so that the check above will not think it's 0
        sMatchSetToMatchPattern[mIndex][testWord.mIndex] = testResult;
        return testResult;
    }

    bool IsValidWith(const NumberWord& entryWord, uint16_t matchResults) const
    {
        uint16_t competingTestResults = GetTestResults(entryWord);
        if (competingTestResults != matchResults)
        {
            return false;
        }

        uint32_t remainingValue = mValue;
        uint32_t remainingEntry = entryWord.mValue;

        for (size_t i = 0; i < kWordLength; ++i)
        {
            const SingleMatch singleMatch = static_cast<SingleMatch>( (matchResults >> (i * kBitsPerMatch)) & static_cast<uint16_t>(SingleMatch::kAnyMask) );
            if (singleMatch == SingleMatch::kHit)
            {
                remainingValue &= ~kLetterMatchBitmask[i];
            }
        }

        for (size_t i = 0; i < kWordLength; ++i)
        {
            const SingleMatch singleMatch = static_cast<SingleMatch>((matchResults >> (i * kBitsPerMatch))& static_cast<uint16_t>(SingleMatch::kAnyMask));
            if (singleMatch == SingleMatch::kPartial)
            {
                uint32_t entryLetter = (remainingEntry & kLetterMatchBitmask[i]) >> (i * kBitsPerLetter);
                for (size_t t = 0; t < kWordLength; ++t)
                {
                    const uint32_t valueLetter = (remainingValue & kLetterMatchBitmask[t]) >> (t * kBitsPerLetter);
                    if (entryLetter == valueLetter)
                    {
                        entryLetter = 0;
                        remainingValue &= ~kLetterMatchBitmask[t];
                        break;
                    }
                }

                if (entryLetter != 0)
                {
                    return false;
                }
            }
        }

        for (size_t i = 0; i < kWordLength; ++i)
        {
            const SingleMatch singleMatch = static_cast<SingleMatch>((matchResults >> (i * kBitsPerMatch))& static_cast<uint16_t>(SingleMatch::kAnyMask));
            if (singleMatch == SingleMatch::kMiss)
            {
                const uint32_t entryLetter = (remainingEntry & kLetterMatchBitmask[i]) >> (i * kBitsPerLetter);
                for (size_t t = 0; t < kWordLength; ++t)
                {
                    const uint32_t valueLetter = (remainingValue & kLetterMatchBitmask[t]) >> (t * kBitsPerLetter);
                    if (entryLetter == valueLetter)
                    {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    void ConvertToString(std::string& outString) const
    {
        outString.clear();
        outString.reserve(kWordLength + 1);
        for (size_t i = 0; i < kWordLength; ++i)
        {
            // a == 1, z == 26
            const char letter = ((mValue & kLetterMatchBitmask[i]) >> (i * kBitsPerLetter)) + 'a' - 1;
            outString.push_back(letter);
        }
    }

    uint8_t GetLetterValue() const
    {
        uint8_t value = 0;
        for (size_t i = 0; i < kWordLength; ++i)
        {
            const char letter = ((mValue & kLetterMatchBitmask[i]) >> (i * kBitsPerLetter));
            switch (letter)
            {
                case ('z' - 'a' + 1): value ^= 1 << 7; break;
                case ('x' - 'a' + 1): value ^= 1 << 6; break;
                case ('v' - 'a' + 1): value ^= 1 << 5; break;
                case ('j' - 'a' + 1): value ^= 1 << 4; break;
                case ('m' - 'a' + 1): value ^= 1 << 2; break;
                case ('g' - 'a' + 1): value ^= 1 << 1; break;
                default: break;
            }
        }
        return value;
    }

    uint32_t mValue;
    uint16_t mIndex;

    static uint16_t sMatchSetToMatchPattern[kNumWords][kNumWords];
    static std::vector<NumberWord> sAllWords;
    static std::vector<NumberWord> sDifficultToEvalWords;
    static std::vector<NumberWord> sVERYDifficultToEvalWords;
    static std::vector<NumberWord> s2049MatchedWords;
    static void Initialize();
    static void SwapToAllWords();
    static void SwapToSmallSetOfWords();
    static NumberWord LookupNumberWord(const char* const text);
};

const NumberWord kInvalidNumberWord(0u, UINT16_MAX);
const NumberWord kSupposedMostOptimizedFirstWord = NumberWord::LookupNumberWord("stowp");
