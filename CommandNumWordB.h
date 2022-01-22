#pragma once
#include "stdafx.h"
#include "NumWords.h"

namespace NumWordB
{
const size_t kMaxSteps = 6;

void PrintWordList(std::string& bufferString, const std::vector<NumberWord>& wordList, const uint32_t maxLines = 4, const bool hasEndingNewLine = true);


// KEY STRATEGY TOOLS
class NumberWordStrategy
{
public:
    enum class StepStrategy : uint8_t
    {
        kOptimalPatternMatch,
        kOptimalPatternMatchWithAlternatives,
        kSpecialWord1, // stowp
        kSpecialWord2,
        kUseInputWord,
        kTakeFirstEntry,
        kSuperSpecialLogicIGuess
    };
    enum class PrintBehavior : uint8_t
    {
        kNone = 0,
        kOptimalWordsAndRemainingPossibilities = 1 << 0,
        kInputWord = 1 << 1,
        kPossibilitiesProgress = 1 << 2,
        kShowFailsAtEndOfPossibilities = 1 << 3,
        kShowAlmostsAtEndOfPossibilities = 1 << 4,
        kNumFails = 1 << 5,
        kPrintPatternMatchAlternatives = 1 << 6,
        kShowAverageStepsToSolve = 1 << 7
    };
    const StepStrategy kDefaultStrategies[kMaxSteps]
    {
        StepStrategy::kSpecialWord1,
        StepStrategy::kOptimalPatternMatch,
        StepStrategy::kSuperSpecialLogicIGuess,
        StepStrategy::kOptimalPatternMatch,
        StepStrategy::kOptimalPatternMatch,
        StepStrategy::kTakeFirstEntry
    };
    const StepStrategy kStrategyPostMax = StepStrategy::kOptimalPatternMatch;
    
    NumberWordStrategy();
    ~NumberWordStrategy() = default;

    size_t TestSpecificWord(const NumberWord& targetWord);
    uint32_t TestAllPossibilities(const std::vector<NumberWord>& allPossibilities);
    uint32_t TestAllPossibilitiesUntilPassXFails(const std::vector<NumberWord>& allPossibilities, uint32_t xFails);
    bool ProceedToNextStepWithPatternAndPrintOptimalWord(uint16_t pattern);

    void SetStrategy(StepStrategy stepStrategyList[kMaxSteps]);
    void SetSpecificStepStrategy(size_t step, StepStrategy stepStrategy);
    void SetInputWord(const NumberWord& inputWord);
    void SetSpecialWord1(const NumberWord& specialWord1);
    void SetSpecialWord2(const NumberWord& specialWord2);
    void AddPrintBehavior(PrintBehavior printBehavior);
    void SetStartingStep(size_t step);

private:
    void PairDownSpecificPossibilities(uint16_t sourceMatch, const NumberWord& entryWord, std::vector<NumberWord>& possibilities) const;
    void PairDownRemainingPossibilities(uint16_t sourceMatch, const NumberWord& entryWord);

    const NumberWord FindOptimalPatternMatch() const;
    const NumberWord FindOptimalPatternMatchWithAlternatives() const;
    const NumberWord SuperSpecialLogicForFindingOptimalWordIGuess() const;

    void PrintWord(const size_t currentStep, const NumberWord& word);
    void PrintWord(const size_t currentStep, const NumberWord& word, const uint16_t results);
    void PrintPossibilities();

    NumberWord GetOptimalWordFromStrategy(StepStrategy strategy) const;
    
    bool HasPrintBehavior(PrintBehavior printBehavior) const;

    std::vector<NumberWord> mRemainingPossibilities;
    std::string mTextForWord;
    size_t mCurrentStep = 0;
    size_t mStartingStep = 0;
    StepStrategy mStepStrategies[kMaxSteps];
    PrintBehavior mPrintBehavior = PrintBehavior::kNone;
    NumberWord mCurrentInputWord = kInvalidNumberWord;
    NumberWord mSpecialWord1 = kInvalidNumberWord;
    NumberWord mSpecialWord2 = kInvalidNumberWord;
    uint16_t mStepResults[kMaxSteps] = { 0 };

    const std::vector<NumberWord>* mRemainingPossibilitiesSource = nullptr;
};

// COMMANDS
void COMMAND_SpecificWord(const char* const word);
void COMMAND_FindOptimalSecondWord();
void COMMAND_ComprehensiveTestFromSetOfFirstWords();
void COMMAND_ComprehensiveTestFromSetOfSecondWords();
void COMMAND_ComprehensiveTest();
void COMMAND_ComprehensiveTest2049WordsOnly();
void COMMAND_IsThereAnyOptimalWordForDouble2049();
void COMMAND_SpecificStartSpecificWord(const char* const start, const char* const word);
void COMMAND_PlayWordle();
} // namespace NumWordB
