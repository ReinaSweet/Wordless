#include "CommandNumWordB.h"
#include "ConsoleInfo.h"

NumWordB::NumberWordStrategy::NumberWordStrategy()
{
    memcpy(mStepStrategies, kDefaultStrategies, sizeof(kDefaultStrategies));
    mRemainingPossibilitiesSource = &NumberWord::sAllWords;
    mSpecialWord1 = NumberWord::LookupNumberWord("stowp");
}

size_t NumWordB::NumberWordStrategy::TestSpecificWord(const NumberWord& targetWord)
{
    NumberWord optimalWord(kInvalidNumberWord);

    mRemainingPossibilities = *mRemainingPossibilitiesSource;
    mCurrentStep = mStartingStep;

    bool continueToNextStep = true;
    do
    {
        const StepStrategy currentStrategy = (mCurrentStep < kMaxSteps) ? mStepStrategies[mCurrentStep] : kStrategyPostMax;
        ++mCurrentStep;
        optimalWord = GetOptimalWordFromStrategy(currentStrategy);
        if (optimalWord == targetWord)
        {
            continueToNextStep = false;
            if (HasPrintBehavior(PrintBehavior::kOptimalWordsAndRemainingPossibilities))
            {
                PrintWord(mCurrentStep, optimalWord);
            }
        }
        else
        {
            const uint16_t results = targetWord.GetTestResults(optimalWord);
            if (mCurrentStep < kMaxSteps)
            {
                mStepResults[mCurrentStep - 1] = results;
            }
            PairDownRemainingPossibilities(results, optimalWord);
            if (HasPrintBehavior(PrintBehavior::kOptimalWordsAndRemainingPossibilities))
            {
                PrintWord(mCurrentStep, optimalWord, results);
                PrintPossibilities();
            }
        }
    } while (continueToNextStep);

    return mCurrentStep;
}

uint32_t NumWordB::NumberWordStrategy::TestAllPossibilities(const std::vector<NumberWord>& allPossibilities)
{
    return TestAllPossibilitiesUntilPassXFails(allPossibilities, UINT32_MAX);
}

uint32_t NumWordB::NumberWordStrategy::TestAllPossibilitiesUntilPassXFails(const std::vector<NumberWord>& allPossibilities, uint32_t xFails)
{
    mRemainingPossibilitiesSource = &allPossibilities;

    if (HasPrintBehavior(PrintBehavior::kInputWord))
    {
        mCurrentInputWord.ConvertToString(mTextForWord);
        printf("\n%s : ", mTextForWord.c_str());
    }

    const size_t spaceForProgress = ConsoleInfo::GetConsoleWidth() >> 1;
    size_t wordsBeforeProgress = allPossibilities.size() / spaceForProgress;
    if (wordsBeforeProgress < 1)
    {
        wordsBeforeProgress = 1;
    }

    std::map<size_t, std::vector<NumberWord>> stepsToWords;

    uint32_t fails = 0;
    uint32_t wordsProcessedBeforePrint = 0;
    uint32_t totalStepsUsed = 0;
    for (const NumberWord& targetWord : allPossibilities)
    {
        const size_t steps = TestSpecificWord(targetWord);
        totalStepsUsed += static_cast<uint32_t>(steps);
        if (steps > kMaxSteps)
        {
            ++fails;

            if (HasPrintBehavior(PrintBehavior::kShowFailsAtEndOfPossibilities))
            {
                stepsToWords[steps].push_back(targetWord);
            }
            if (fails > xFails)
            {
                break;
            }
        }
        else if (steps > 3)
        {
            if (HasPrintBehavior(PrintBehavior::kShowSuccessPossibilitiesOverThree))
            {
                stepsToWords[steps].push_back(targetWord);
            }
        }

        if (HasPrintBehavior(PrintBehavior::kPossibilitiesProgress))
        {
            ++wordsProcessedBeforePrint;
            if (wordsProcessedBeforePrint == wordsBeforeProgress)
            {
                wordsProcessedBeforePrint = 0;
                putchar((char)178);
            }
        }
    }

    //
    // End Data to store on the strategy
    //
    mAverageStepsToSolve = static_cast<double>(totalStepsUsed) / static_cast<double>(allPossibilities.size());

    //
    // Final Print Behaviors
    //
    if (HasPrintBehavior(PrintBehavior::kNumFails))
    {
        printf(" - %u Ls", fails);
    }

    if (HasPrintBehavior(PrintBehavior::kShowAverageStepsToSolve))
    {
        printf(" - Avg Steps %.4f", mAverageStepsToSolve);
    }

    if (HasPrintBehavior(PrintBehavior::kShowSuccessPossibilitiesOverThree))
    {
        size_t steps = 4;
        auto iter = stepsToWords.find(steps);
        while (iter != stepsToWords.end())
        {
            printf("\n %zu STEPS", steps);
            PrintWordList(mTextForWord, (*iter).second);
            ++steps;
            if (steps > kMaxSteps)
            {
                break;
            }
            iter = stepsToWords.find(steps);
        }
    }

    if (HasPrintBehavior(PrintBehavior::kShowFailsAtEndOfPossibilities))
    {
        size_t steps = kMaxSteps + 1;
        auto iter = stepsToWords.find(steps);
        while (iter != stepsToWords.end())
        {
            printf("\n %zu STEPS", steps);
            PrintWordList(mTextForWord, (*iter).second);
            ++steps;
            iter = stepsToWords.find(steps);
        }
    }

    if (mPrintBehavior != PrintBehavior::kNone)
    {
        putchar('\n');
    }
    return fails;
}

bool NumWordB::NumberWordStrategy::ProceedToNextStepWithPatternAndPrintOptimalWord(uint16_t pattern)
{
    if (mCurrentStep == 0)
    {
        mRemainingPossibilities = *mRemainingPossibilitiesSource;
    }
    else
    {
        if (pattern == kPerfectMatch)
        {
            printf("\n\nCongratulations!");
            return false;
        }

        PairDownRemainingPossibilities(pattern, mCurrentInputWord);
    }

    const StepStrategy currentStrategy = (mCurrentStep < kMaxSteps) ? mStepStrategies[mCurrentStep] : kStrategyPostMax;
    ++mCurrentStep;
    mCurrentInputWord = GetOptimalWordFromStrategy(currentStrategy);
    printf("\n\nINPUT THE FOLLOWING WORD:");
    PrintWord(mCurrentStep, mCurrentInputWord);
    printf("\n\nTHEN INPUT THE WORDLE RESULT.\n * 0 for a miss.\n * 1 for a partial (yellow).\n * 2 for a hit (green)\n\n");
    return true;
}

void NumWordB::NumberWordStrategy::SetStrategy(StepStrategy stepStrategyList[kMaxSteps])
{
    memcpy(mStepStrategies, stepStrategyList, sizeof(kDefaultStrategies));
}

void NumWordB::NumberWordStrategy::SetSpecificStepStrategy(size_t step, StepStrategy stepStrategy)
{
    mStepStrategies[step - 1] = stepStrategy;
}

void NumWordB::NumberWordStrategy::SetInputWord(const NumberWord& inputWord)
{
    mCurrentInputWord = inputWord;
}

void NumWordB::NumberWordStrategy::SetSpecialWord1(const NumberWord& specialWord1)
{
    mSpecialWord1 = specialWord1;
}

void NumWordB::NumberWordStrategy::SetSpecialWord2(const NumberWord& specialWord2)
{
    mSpecialWord2 = specialWord2;
}

void NumWordB::NumberWordStrategy::SetStartingStep(size_t step)
{
    mStartingStep = step - 1;
}

void NumWordB::NumberWordStrategy::AddPrintBehavior(PrintBehavior printBehavior)
{
    mPrintBehavior = static_cast<PrintBehavior>(static_cast<uint8_t>(mPrintBehavior) | static_cast<uint8_t>(printBehavior));
}

bool NumWordB::NumberWordStrategy::HasPrintBehavior(PrintBehavior printBehavior) const
{
    return (static_cast<uint8_t>(mPrintBehavior)& static_cast<uint8_t>(printBehavior)) != 0;
}



void NumWordB::NumberWordStrategy::PairDownSpecificPossibilities(uint16_t sourceMatch, const NumberWord& entryWord, std::vector<NumberWord>& possibilities) const
{
    std::vector<NumberWord> newPossibilities;
    newPossibilities.reserve(possibilities.size() >> 1);
    for (const NumberWord& possibility : possibilities)
    {
        if (possibility.IsValidWith(entryWord, sourceMatch))
        {
            newPossibilities.emplace_back(possibility);
        }
    }
    possibilities.swap(newPossibilities);
}

void NumWordB::NumberWordStrategy::PairDownRemainingPossibilities(uint16_t sourceMatch, const NumberWord& entryWord)
{
    PairDownSpecificPossibilities(sourceMatch, entryWord, mRemainingPossibilities);
}

const NumberWord NumWordB::NumberWordStrategy::FindOptimalPatternMatch() const
{
    if (mRemainingPossibilities.size() < 3)
    {
        return mRemainingPossibilities.at(0);
    }

    // What testWord, when applied, would result in a set of match patterns with the least entries in its largest sized match

    uint16_t currentLowestLargeSetSize = UINT16_MAX;
    NumberWord currentOptimalWord = kInvalidNumberWord;

    for (const NumberWord& testWord : NumberWord::sAllWords)
    {
        uint16_t matchPatterns[1024] = { 0 };
        for (const NumberWord& possibility : mRemainingPossibilities)
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
            currentOptimalWord = testWord;
        }
    }

    return currentOptimalWord;
}

const NumberWord NumWordB::NumberWordStrategy::FindOptimalPatternMatchWithAlternatives() const
{
    if (mRemainingPossibilities.size() < 3)
    {
        return mRemainingPossibilities.at(0);
    }

    // What testWord, when applied, would result in a set of match patterns with the least entries in its largest sized match

    uint16_t currentLowestLargeSetSize = UINT16_MAX;
    NumberWord currentOptimalWord = kInvalidNumberWord;
    std::vector<NumberWord> alternatives;

    for (const NumberWord& testWord : NumberWord::sAllWords)
    {
        uint16_t matchPatterns[1024] = { 0 };
        for (const NumberWord& possibility : mRemainingPossibilities)
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
            currentOptimalWord = testWord;
            alternatives.clear();
        }
        else if (largestSetSize == currentLowestLargeSetSize)
        {
            alternatives.push_back(testWord);
        }
    }

    if (!alternatives.empty())
    {
        uint8_t currentLetterValue = currentOptimalWord.GetLetterValue();
        for (size_t i = 0; i < alternatives.size(); ++i)
        {
            const uint8_t altLetterValue = alternatives.at(i).GetLetterValue();
            if (altLetterValue > currentLetterValue)
            {
                currentLetterValue = altLetterValue;
                const NumberWord oldOptimalWord = currentOptimalWord;
                currentOptimalWord = alternatives.at(i);
                alternatives[i] = oldOptimalWord;
            }
        }

        if (HasPrintBehavior(PrintBehavior::kPrintPatternMatchAlternatives))
        {
            std::string textForWord;
            printf("\n\n ALTS");
            PrintWordList(textForWord, alternatives, 12, false);
        }
    }

    return currentOptimalWord;
}

const NumberWord NumWordB::NumberWordStrategy::SuperSpecialLogicForFindingOptimalWordIGuess() const
{
    if (mStepResults[0] == 2049)
    {
        if (mStepResults[1] == 2049)
        {
            return NumberWord::LookupNumberWord("banjo");
        }
        if (mStepResults[1] == 2440)
        {
            return NumberWord::LookupNumberWord("zarfs");
        }
    }
    return FindOptimalPatternMatch();
}

void NumWordB::NumberWordStrategy::PrintWord(const size_t currentStep, const NumberWord& word)
{
    word.ConvertToString(mTextForWord);
    printf("\n%zu - %s", currentStep, mTextForWord.c_str());
}

void NumWordB::NumberWordStrategy::PrintWord(const size_t currentStep, const NumberWord& word, const uint16_t results)
{
    word.ConvertToString(mTextForWord);
    printf("\n%zu - %s %hu", currentStep, mTextForWord.c_str(), results);
}

void NumWordB::NumberWordStrategy::PrintPossibilities()
{
    PrintWordList(mTextForWord, mRemainingPossibilities);
}

NumberWord NumWordB::NumberWordStrategy::GetOptimalWordFromStrategy(StepStrategy strategy) const
{
    switch (strategy)
    {
        case StepStrategy::kOptimalPatternMatch: return FindOptimalPatternMatch();
        case StepStrategy::kOptimalPatternMatchWithAlternatives: return FindOptimalPatternMatchWithAlternatives();
        case StepStrategy::kSpecialWord1: return mSpecialWord1;
        case StepStrategy::kSpecialWord2: return mSpecialWord2;
        case StepStrategy::kUseInputWord: return mCurrentInputWord;
        case StepStrategy::kTakeFirstEntry: return mRemainingPossibilities.at(0);
        case StepStrategy::kSuperSpecialLogicIGuess: return SuperSpecialLogicForFindingOptimalWordIGuess();

        default: return kInvalidNumberWord;
    }
}

void NumWordB::PrintWordList(std::string& bufferString, const std::vector<NumberWord>& wordList, const uint32_t maxLines /*= 4*/, const bool hasEndingNewLine /*= true*/)
{
    const uint32_t wordsPerLine = 18;

    printf(": ");
    size_t index = 0;

    for (size_t line = 0; line < maxLines; ++line)
    {
        for (size_t word = 0; word < wordsPerLine; ++word)
        {
            if (index >= wordList.size())
            {
                break;
            }

            wordList[index].ConvertToString(bufferString);
            printf("%s, ", bufferString.c_str());

            ++index;
        }

        if ((index >= wordList.size()) || ((line + 1) == maxLines))
        {
            break;
        }

        printf("\n          ");
    }

    if (wordList.size() > index)
    {
        printf("... (+%zu)", wordList.size() - index);
    }
    if (hasEndingNewLine)
    {
        putchar('\n');
    }
}

// COMMANDS
void NumWordB::COMMAND_SpecificWord(const char* const word)
{
    const NumberWord targetWord(NumberWord::LookupNumberWord(word));
    NumberWordStrategy strategy;
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kOptimalWordsAndRemainingPossibilities);
    strategy.TestSpecificWord(targetWord);
}

void NumWordB::COMMAND_FindOptimalSecondWord()
{
    std::vector<NumberWord> limitedStarts;
    const char charStarts[][kWordLengthPlusNullByte] = {
        "babes", "banns", "barfs", "based", "bases", "basic", "basis", "bassi", "bibes", "bices", "biffs", "bises", "bizes", "caffs", "casas", "cased", "caves",
        "dives", "eased", "eases", "eaves", "erses", "faffs", "fangs", "faves", "faxes", "fazes", "fears", "fells", "fills", "fives", "gaffs", "gages", "gases",
        "gazes", "ghees", "gibes", "gives", "glads", "grees", "gyves", "hahas", "hangs", "hides", "hinds", "hives", "huffs", "isbas", "jades", "jaggs", "jambs",
        "javas", "jells", "jibes", "jills", "jinns", "jives", "jujus", "junks", "kings", "kinks", "lases", "laves", "lazes", "maims", "massy", "maxes", "maxis",
        "mells", "mezes", "mimes", "mises", "mixes", "mulls", "nails", "nards", "nares", "nines", "quads", "quass", "raids", "rajes", "ramus", "rangs", "rases",
        "raxes", "rayas", "rezes", "rimes", "rises", "vairs", "vanes", "vangs", "vases", "vexes", "viffs", "vills", "vines", "visas", "vises", "vivas", "vughs",
        "yages", "yards", "yuans", "yucks", "zarfs", "zaxes", "zines", "zymes", "mills", "razes"
    };
    const size_t numLimitedStarts = sizeof(charStarts) / kWordLengthPlusNullByte;


    NumberWordStrategy strategy;
    strategy.SetSpecificStepStrategy(2, NumberWordStrategy::StepStrategy::kUseInputWord);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kInputWord);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kNumFails);

    uint32_t currentMaxFails = UINT32_MAX;
    std::vector<NumberWord> minimumFailureWords;

    for (size_t i = 0; i < numLimitedStarts; ++i)
    {
        const NumberWord inputWord = NumberWord::LookupNumberWord(charStarts[i]);
        strategy.SetInputWord(inputWord);
        const uint32_t fails = strategy.TestAllPossibilitiesUntilPassXFails(NumberWord::s2049MatchedWords, currentMaxFails);
        if (fails < currentMaxFails)
        {
            minimumFailureWords.clear();
            minimumFailureWords.push_back(inputWord);
            currentMaxFails = fails;
        }
        else if (fails == currentMaxFails)
        {
            minimumFailureWords.push_back(inputWord);
        }
    }

    printf("\nLowest fails at: %u\n", currentMaxFails);
    std::string textForWord;
    PrintWordList(textForWord, minimumFailureWords);
}

void NumWordB::COMMAND_ComprehensiveTestFromSetOfFirstWords()
{
    NumberWordStrategy strategy;
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kInputWord);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kNumFails);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kPossibilitiesProgress);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kShowSuccessPossibilitiesOverThree);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kShowFailsAtEndOfPossibilities);

    strategy.SetSpecificStepStrategy(1, NumberWordStrategy::StepStrategy::kUseInputWord);

    const char charStarts[][kWordLengthPlusNullByte] = {
        "bevel", "compt", "feted", "finks", "fiver", "kevil", "sarks", "stowp", "swarf", "vises", "visor"
    };
    const size_t numLimitedStarts = sizeof(charStarts) / kWordLengthPlusNullByte;
    for (size_t i = 0; i < numLimitedStarts; ++i)
    {
        const NumberWord word = NumberWord::LookupNumberWord(charStarts[i]);
        strategy.SetInputWord(word);
        strategy.TestAllPossibilities(NumberWord::sAllWords);
    }
}

void NumWordB::COMMAND_ComprehensiveTestFromSetOfSecondWords()
{
    NumberWordStrategy strategy;
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kInputWord);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kNumFails);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kPossibilitiesProgress);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kShowSuccessPossibilitiesOverThree);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kShowFailsAtEndOfPossibilities);

    strategy.SetSpecificStepStrategy(2, NumberWordStrategy::StepStrategy::kUseInputWord);

    const char charStarts[][kWordLengthPlusNullByte] = {
        "busks", "carns", "fears", "marcs", "narcs", "neifs", "reifs", "vangs"
    };
    const size_t numLimitedStarts = sizeof(charStarts) / kWordLengthPlusNullByte;
    for (size_t i = 0; i < numLimitedStarts; ++i)
    {
        const NumberWord word = NumberWord::LookupNumberWord(charStarts[i]);
        strategy.SetInputWord(word);
        strategy.TestAllPossibilities(NumberWord::s2049MatchedWords);
    }
}

void NumWordB::COMMAND_ComprehensiveTest()
{
    double possiblePatternsEachStep = std::pow(3., 5.);
    const double threeStepAnswers = (static_cast<double>(NumberWord::sAnswerWords.size()) - possiblePatternsEachStep);
    possiblePatternsEachStep -= 1.; // Remove exact first guess
    const double minimumPossibleStepsForAllAnswers = 1. + (2. * possiblePatternsEachStep) + (3. * threeStepAnswers);

    printf("Idealized Perfect Average Steps Score: %.4f\n\n", minimumPossibleStepsForAllAnswers / static_cast<double>(NumberWord::sAnswerWords.size()));

    NumberWordStrategy strategy;
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kPossibilitiesProgress);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kShowSuccessPossibilitiesOverThree);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kShowFailsAtEndOfPossibilities);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kShowAverageStepsToSolve);

    strategy.TestAllPossibilities(NumberWord::sAnswerWords);
}

void NumWordB::COMMAND_ComprehensiveTest2049WordsOnly()
{
    NumberWordStrategy strategy;
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kPossibilitiesProgress);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kShowFailsAtEndOfPossibilities);

    strategy.TestAllPossibilities(NumberWord::s2049MatchedWords);
}

void NumWordB::COMMAND_IsThereAnyOptimalWordForDouble2049()
{
    NumberWordStrategy strategy;
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kInputWord);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kNumFails);

    std::vector<NumberWord> double2049Words;
    const char charStarts[][kWordLengthPlusNullByte] = {
        "bills", "bliss", "blubs", "blush", "buhls", "bulbs", "bulls", "clubs", "culls", "culms", "dills", "dulls", "fills", "films", "flics", "flubs", "flush", "fulls",
        "fusil", "gilds", "gills", "glims", "glugs", "glums", "gulfs", "gulls", "hills", "hilus", "hulls", "idyls", "iglus", "jills", "milds", "mills", "mulls", "nills",
        "nulls", "vills", "vulns", "yills", "zills"
    };
    const size_t numLimitedStarts = sizeof(charStarts) / kWordLengthPlusNullByte;
    for (size_t i = 0; i < numLimitedStarts; ++i)
    {
        const NumberWord word = NumberWord::LookupNumberWord(charStarts[i]);
        double2049Words.push_back(word);
    }
    strategy.SetStartingStep(3);

    for (const NumberWord& word : NumberWord::sAllWords)
    {
        strategy.SetInputWord(word);
        strategy.SetSpecificStepStrategy(3, NumberWordStrategy::StepStrategy::kUseInputWord);
        if (strategy.TestAllPossibilities(double2049Words) == 0)
        {
            std::string textForWord;
            word.ConvertToString(textForWord);
            printf("\n\nFOUND ONE: %s\n\n", textForWord.c_str());
            break;
        }
    }
}

void NumWordB::COMMAND_SpecificStartSpecificWord(const char* const start, const char* const word)
{
    const NumberWord startWord(NumberWord::LookupNumberWord(start));
    const NumberWord targetWord(NumberWord::LookupNumberWord(word));
    NumberWordStrategy strategy;
    strategy.SetSpecialWord1(startWord);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kOptimalWordsAndRemainingPossibilities);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kPrintPatternMatchAlternatives);
    strategy.TestSpecificWord(targetWord);
}

uint32_t GetIndexFromFarRemovedVars(char firstC, char secondC, char thirdC)
{
    if ((firstC == secondC) || (firstC == thirdC) || (secondC == thirdC))
    {
        return UINT32_MAX;
    }

    uint32_t vals[3] = { static_cast<uint32_t>(firstC), static_cast<uint32_t>(secondC), static_cast<uint32_t>(thirdC) };

    if (vals[0] > vals[1])
    {
        std::swap(vals[0], vals[1]);
    }
    if (vals[1] > vals[2])
    {
        std::swap(vals[1], vals[2]);
        if (vals[0] > vals[1])
        {
            std::swap(vals[0], vals[1]);
        }
    }

    return (vals[0] - 'a') + ((vals[1] - 'b') * 26) + ((vals[2] - 'c') * 25 * 26);
};

void NumWordB::COMMAND_DisplayFarRemovedLetterCombinations()
{
    const size_t maxRemovedVarValue = (26 * 25 * 24) + (26 * 25) + (26);
    uint8_t farRemovedVars[maxRemovedVarValue + 1] = { 0 };
    std::string textForWord;

    for (const NumberWord& word : NumberWord::sAllWords)
    {
        word.ConvertToString(textForWord);
        for (size_t x = 0; x < kWordLength; ++x)
        {
            for (size_t y = (x + 1); y < kWordLength; ++y)
            {
                for (size_t z = (y + 1); z < kWordLength; ++z)
                {
                    const uint32_t index = GetIndexFromFarRemovedVars(textForWord[x], textForWord[y], textForWord[z]);
                    if (index != UINT32_MAX)
                    {
                        farRemovedVars[static_cast<size_t>(index)]++;
                    }
                }
            }
        }
    }

    uint8_t lettersCountsInFarRemoved[26] = { 0 };

    uint32_t entriesBuildingToNewLine = 0;
    for (char firstC = 'a'; firstC <= 'z'; ++firstC)
    {
        for (char secondC = (firstC + 1); secondC <= 'z'; ++secondC)
        {
            for (char thirdC = (secondC + 1); thirdC <= 'z'; ++thirdC)
            {
                const uint32_t index = GetIndexFromFarRemovedVars(firstC, secondC, thirdC);
                const uint16_t val = farRemovedVars[static_cast<size_t>(index)];
                if (val < 1)
                {
                    lettersCountsInFarRemoved[static_cast<size_t>(firstC - 'a')]++;
                    lettersCountsInFarRemoved[static_cast<size_t>(secondC - 'a')]++;
                    lettersCountsInFarRemoved[static_cast<size_t>(thirdC - 'a')]++;

                    printf("%c%c%c = %u,  ", firstC, secondC, thirdC, static_cast<uint32_t>(val));
                    ++entriesBuildingToNewLine;
                    if (entriesBuildingToNewLine > 14)
                    {
                        putchar('\n');
                        entriesBuildingToNewLine = 0;
                    }
                }
            }
        }
    }

    printf("\n\n");
    entriesBuildingToNewLine = 0;
    for (char c = 'a'; c <= 'z'; ++c)
    {
        const uint32_t count = static_cast<uint32_t>(lettersCountsInFarRemoved[static_cast<uint32_t>(c - 'a')]);

        printf("%c = %u,  ", c, count);
        ++entriesBuildingToNewLine;
        if (entriesBuildingToNewLine > 12)
        {
            putchar('\n');
            entriesBuildingToNewLine = 0;
        }
    }
}

void NumWordB::COMMAND_FindFewestStepsOpeningWord(uint64_t startingIndex, uint64_t untilIndex)
{
    NumberWordStrategy strategy;
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kInputWord);
    strategy.AddPrintBehavior(NumberWordStrategy::PrintBehavior::kPossibilitiesProgress);

    strategy.SetSpecificStepStrategy(1, NumberWordStrategy::StepStrategy::kUseInputWord);

    double fewestAverageSteps = 100.;
    NumberWord fewestStepsWord = kInvalidNumberWord;
    for (size_t i = startingIndex; (i <= untilIndex) && (i < NumberWord::sAllWords.size()); ++i)
    {
        const NumberWord& word = NumberWord::sAllWords.at(i);

        strategy.SetInputWord(word);
        strategy.TestAllPossibilities(NumberWord::sAnswerWords);
        if (strategy.GetAverageStepsToSolve() < fewestAverageSteps)
        {
            fewestAverageSteps = strategy.GetAverageStepsToSolve();
            fewestStepsWord = word;
        }
    }

    printf("\nLowest average steps at: %.4f\n", fewestAverageSteps);
    std::string textForWord;
    fewestStepsWord.ConvertToString(textForWord);
    printf(" %s", textForWord.c_str());
}

void NumWordB::COMMAND_PlayWordle()
{
    NumberWordStrategy strategy;
    strategy.ProceedToNextStepWithPatternAndPrintOptimalWord(0u);

    char c;
    std::string input;
    bool loop = true;
    do
    {
        c = _getch();
        if (c == '\b' && !input.empty())
        {
            input.pop_back();
            putchar('\b');
        }
        else if ((c == '0' || c == '1' || c == '2') && input.size() < kWordLength)
        {
            input.push_back(c);
            putchar(c);
        }
        else if ((c == '\n' || c == '\r') && input.size() == kWordLength)
        {
            uint16_t pattern = kEarmarkMatchPattern;
            for (size_t i = 0; i < kWordLength; ++i)
            {
                if (input[i] == '1')
                {
                    pattern += static_cast<uint16_t>(SingleMatch::kPartial) << (i * kBitsPerMatch);
                }
                else if (input[i] == '2')
                {
                    pattern += static_cast<uint16_t>(SingleMatch::kHit) << (i * kBitsPerMatch);
                }
            }

            input.clear();
            loop = strategy.ProceedToNextStepWithPatternAndPrintOptimalWord(pattern);
        }
    } while (loop);
}
