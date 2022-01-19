#include "stdafx.h"

#include "ConsoleMenu.h"

InputMode GetInputModeFromChar(const char c)
{
    switch (c)
    {
        case 'x': return InputMode::kHex;
        case 'd': return InputMode::kDecimal;
        case 'b': return InputMode::kBinary;

		default: return InputMode::kInvalid;
    }
}

int GetBaseFromInputMode(const InputMode inputMode)
{
	switch (inputMode)
	{
		case InputMode::kHex: return 16;
		case InputMode::kDecimal: return 10;
		case InputMode::kBinary: return 2;

		default: return 10;
	}
}

CONSOLE_MENU_COMMAND_DEFINE(1, mArgs[0]);
CONSOLE_MENU_COMMAND_DEFINE(2, mArgs[0], mArgs[1]);
CONSOLE_MENU_COMMAND_DEFINE(3, mArgs[0], mArgs[1], mArgs[2]);
CONSOLE_MENU_COMMAND_DEFINE(4, mArgs[0], mArgs[1], mArgs[2], mArgs[3]);

CONSOLE_MENU_TEXT_COMMAND_DEFINE(1, mArgs[0].c_str());
CONSOLE_MENU_TEXT_COMMAND_DEFINE(2, mArgs[0].c_str(), mArgs[1].c_str());
CONSOLE_MENU_TEXT_COMMAND_DEFINE(3, mArgs[0].c_str(), mArgs[1].c_str(), mArgs[2].c_str());
CONSOLE_MENU_TEXT_COMMAND_DEFINE(4, mArgs[0].c_str(), mArgs[1].c_str(), mArgs[2].c_str(), mArgs[3].c_str());

int32_t ConsoleMenuCommandI::Matches(const char* const command) const
{
	size_t pos = 0;
	char c = command[pos];
	while (c != '\0')
	{
		if (c != mCommand[pos])
		{
			return -1;
		}
		c = command[++pos];
	}

	return (int32_t)(mCommand.length()) - (int32_t)(pos);
}

const ConsoleMenuCommandInput& ConsoleMenuCommandI::GetInput(size_t pos) const
{
	if (pos < mInputs.size())
	{
		return mInputs.at(pos);
    }

    static const ConsoleMenuCommandInput skInvalidReturn;
	return skInvalidReturn;
}

ConsoleMenuCommandI::ConsoleMenuCommandI(const char* const command, const char* const description)
    : mCommand(command)
    , mDescription(description)
{
	size_t semiColonLoc = mDescription.find(';');
	if (semiColonLoc != std::string::npos)
    {
		const size_t firstColonLoc = semiColonLoc;
        do
        {
            const char inputModeCharacter = mDescription.at(semiColonLoc + 1);
            const InputMode newMode = GetInputModeFromChar(inputModeCharacter);
			const size_t inputDescriptionStart = semiColonLoc + (newMode == InputMode::kInvalid ? 1 : 2);

            semiColonLoc = mDescription.find(';', semiColonLoc + 1);
			if (semiColonLoc == std::string::npos)
            {
				mInputs.emplace_back(newMode, mDescription.substr(inputDescriptionStart));
			}
			else
            {
                mInputs.emplace_back(newMode, mDescription.substr(inputDescriptionStart, semiColonLoc - inputDescriptionStart));
			}
        }
		while (semiColonLoc != std::string::npos);

		mDescription.resize(firstColonLoc);
	}
}

ConsoleMenuCommandI::ConsoleMenuCommandI(const char* const command, const char* const description, InputMode forcedInputMode)
	: mCommand(command)
	, mDescription(description)
{
	mInputs.emplace_back(forcedInputMode);
}

ConsoleMenu::~ConsoleMenu()
{
	for (ConsoleMenuCommandI* command : mCommands)
	{
		delete command;
	}
	mCommands.clear();
}

void ConsoleMenu::ResetMenu()
{
	ClearInput();

	putchar('\n');
	PrintHorizontalBreak();
	putchar('\n');
	for (ConsoleMenuCommandI* command : mCommands)
	{
		printf(" %s = %s\n", command->GetCommand(), command->GetDescription());
	}

	if (mParentMenu)
	{
		printf(" x = Return to %s\n\n", mParentMenu->mDescription.c_str());
	}
	else
    {
        printf(" x = Exit\n\n");
	}

	mInputMode = InputMode::kCommand;
}

void ConsoleMenu::PrintHorizontalBreak()
{
	putchar('\n');
	const size_t consoleWidth = ConsoleInfo::GetConsoleWidth();
	for (size_t i = 0; i < consoleWidth; ++i)
	{
		putchar('=');
	}
	putchar('\n');
}

bool ConsoleMenu::ReceiveInput(char c)
{
	if (mInputMode == InputMode::kSubMenu)
	{
		if (!mCurrentSubMenu->ReceiveInput(c))
		{
			ResetMenu();
		}
		return true;
	}

	// Lowercase
	if (c > 64 && c < 91)
	{
		c += 32;
	}

	if (c == '\b')
	{
		BackspaceLastInput();
	}
	else if (mInputMode == InputMode::kCommand)
	{
		// Special command: eXit
		if ((mCurrentPos == 0u) && (c == 'x'))
		{
			return false;
		}

		ReceiveCommandInput(c);
	}
	else if (mInputMode == InputMode::kText)
	{
		ReceiveTextInput(c);
	}
	else
	{
		const InputMode newMode = GetInputModeFromChar(c);
		if (newMode == InputMode::kInvalid)
        {
            ReceiveNumberInput(c);
		}
		else
        {
            ConvertModeTo(newMode);
		}
	}

	return true;
}

void ConsoleMenu::AddCommand(const char* const input, const char* const description, FuncPtr0Num func)
{
	mCommands.push_back(new ConsoleMenuCommand0Num(input, description, func));
}

void ConsoleMenu::AddSubmenu(const char* const input, ConsoleMenu& subMenu)
{
	mCommands.push_back(new ConsoleMenuCommandSubMenu(input, subMenu));
}

void ConsoleMenu::ReceiveCommandInput(char c)
{
	PutInput(c);

	size_t partialMatches = 0u;
	for (ConsoleMenuCommandI* command : mCommands)
	{
		const int32_t matchResults = command->Matches(mInputBuffer);
		if (matchResults == 0)
		{
			mCurrentCommand = command;

			StartNewInputOnNewLine();

			if (mCurrentCommand->IsSubMenu())
			{
				mCurrentSubMenu = &(static_cast<ConsoleMenuCommandSubMenu*>(mCurrentCommand)->GetMenu());
                mInputMode = InputMode::kSubMenu;
                ClearInput();
                printf("\n\n");
                mCurrentSubMenu->ResetMenu();
			}
			else if (mCurrentCommand->GetNumArgs() > 0u)
			{
				mCurrentParam = 0u;
				StartParamInput();
			}
			else
            {
				ExecuteCurrentCommand();
			}
			return;
		}
		else if (matchResults > 0)
		{
			++partialMatches;
		}
	}

	if (partialMatches == 0u)
	{
		ClearInput();
	}
}

void ConsoleMenu::ReceiveTextInput(char c)
{
    if (c == '\n' || c == '\r')
    {
        mCurrentCommand->SetArg(mCurrentParam - 1, mInputBuffer);

        StartNewInputOnNewLine();
        if (mCurrentCommand->GetNumArgs() > mCurrentParam)
        {
            StartParamInput();
        }
        else
        {
            ExecuteCurrentCommand();
        }
	}
	else
    {
        PutInput(c);
	}
}

void ConsoleMenu::ReceiveNumberInput(char c)
{
	if (c == ' ' || c == '\n' || c == '\r')
	{
		const int64_t val = strtoull(mInputBuffer + 1, nullptr, GetBaseFromInputMode(mInputMode));
		mCurrentCommand->SetArg(mCurrentParam - 1, (uint64_t)val);

        StartNewInputOnNewLine();
		if (mCurrentCommand->GetNumArgs() > mCurrentParam)
        {
            StartParamInput();
		}
		else
        {
            ExecuteCurrentCommand();
		}
	}
	else
    {
        if (mInputMode == InputMode::kBinary)
        {
            if (c == '0' || c == '1')
            {
                PutInput(c);
            }
        }
        else if (mInputMode == InputMode::kDecimal)
        {
            if (c >= '0' && c <= '9')
            {
                PutInput(c);
            }
        }
        else if (mInputMode == InputMode::kHex)
        {
            if ((c >= '0' && c <= '9')
                || (c >= 'a' && c <= 'f'))
            {
                PutInput(c);
            }
        }
	}
}

void ConsoleMenu::ConvertModeTo(InputMode mode)
{
    ClearInput();
	mInputMode = mode;
    if (mInputMode == InputMode::kBinary)
    {
		PutInput('b');
    }
	else if (mInputMode == InputMode::kDecimal)
	{
		PutInput('d');
	}
	else if (mInputMode == InputMode::kHex)
	{
		PutInput('x');
	}
}

void ConsoleMenu::StartParamInput()
{
	const ConsoleMenuCommandInput& input = mCurrentCommand->GetInput(mCurrentParam);
	++mCurrentParam;
	printf("\n%s (Param %zu):\n", input.mDescription.c_str(), mCurrentParam);

	ConvertModeTo((input.mInputMode == InputMode::kInvalid ? mInputMode : input.mInputMode));
}

void ConsoleMenu::PutInput(char c)
{
    putchar(c);

    mInputBuffer[mCurrentPos] = c;
    ++mCurrentPos;
    mInputBuffer[mCurrentPos] = '\0';
}

void ConsoleMenu::BackspaceLastInput()
{
    if (mCurrentPos > 0u)
    {
        printf("\b \b");
        mInputBuffer[mCurrentPos] = '\0';
        --mCurrentPos;
    }
}

void ConsoleMenu::ClearInput()
{
	while (mCurrentPos > 0u)
	{
		printf("\b \b");
		--mCurrentPos;
	}
	mInputBuffer[0] = '\0';
}

void ConsoleMenu::StartNewInputOnNewLine()
{
    mInputBuffer[0] = '\0';
	mCurrentPos = 0;
	putchar('\n');
}

void ConsoleMenu::ExecuteCurrentCommand()
{
    printf("\n\n");
    mCurrentCommand->Execute();
    printf("\n\n");
    ResetMenu();
}
