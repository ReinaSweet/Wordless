#pragma once

#include "ConsoleInfo.h"

enum class InputMode
{
    kCommand,
    kDecimal,
    kBinary,
    kHex,
	kText,
	kSubMenu,
	kInvalid
};

class ConsoleMenuCommandI;

class ConsoleMenuCommandInput
{
public:
	ConsoleMenuCommandInput()
		: mInputMode(InputMode::kInvalid)
	{}

    ConsoleMenuCommandInput(const InputMode inputMode)
        : mInputMode(inputMode)
    {}

    ConsoleMenuCommandInput(const InputMode inputMode, const std::string& description)
        : mInputMode(inputMode)
        , mDescription(description)
    {}

    const InputMode mInputMode;
    const std::string mDescription;
};

class ConsoleMenuCommandI
{
public:
	virtual ~ConsoleMenuCommandI() = default;
	virtual void Execute() {}
	virtual size_t GetNumArgs() const { return 0u; }
	virtual void SetArg(size_t /*pos*/, uint64_t /*val*/) {};
	virtual void SetArg(size_t /*pos*/, const char* const /*text*/) {}
	virtual bool IsSubMenu() const { return false; }

	/*
	* -1 = No Match
	* N  = All characters match, but we're N characters short of the end
	* 0  = All character match
	**/
	int32_t Matches(const char* const command) const;

	const char* GetCommand() const { return mCommand.c_str(); }
	const char* GetDescription() const { return mDescription.c_str(); }
	const ConsoleMenuCommandInput& GetInput(size_t pos) const;

protected:
	ConsoleMenuCommandI(const char* const command, const char* const description);
	ConsoleMenuCommandI(const char* const command, const char* const description, InputMode forcedInputMode);

	std::string mCommand;
	std::string mDescription;
	std::vector<ConsoleMenuCommandInput> mInputs;
};

using FuncPtr0Num = void(*)();
class ConsoleMenuCommand0Num : public ConsoleMenuCommandI
{
public:
    ConsoleMenuCommand0Num(const char* const command, const char* const description, FuncPtr0Num func) 
		: ConsoleMenuCommandI(command, description)
    {
        mFunc = func;
    }

	virtual ~ConsoleMenuCommand0Num() = default;

	virtual void Execute() override { mFunc(); }

private:
	FuncPtr0Num mFunc;
};

#define CONSOLE_MENU_COMMAND_DECLARE(NUMARGS, ...) \
	using FuncPtr##NUMARGS##Num = void(*)(__VA_ARGS__); \
	class ConsoleMenuCommand##NUMARGS##Num : public ConsoleMenuCommandI \
	{ \
	public: \
		ConsoleMenuCommand##NUMARGS##Num(const char* const command, const char* const description, FuncPtr##NUMARGS##Num func) \
			: ConsoleMenuCommandI(command, description) \
		{ mFunc = func; } \
		virtual ~ConsoleMenuCommand##NUMARGS##Num() = default; \
		virtual void Execute() override; \
		virtual size_t GetNumArgs() const override { return NUMARGS; } \
		virtual void SetArg(size_t pos, uint64_t val) override { mArgs[pos] = val; } \
	private: \
		FuncPtr##NUMARGS##Num mFunc; \
		uint64_t mArgs[ NUMARGS ]; \
	}; \
	void AddCommand(const char* const input, const char* const description, FuncPtr##NUMARGS##Num func)

#define CONSOLE_MENU_COMMAND_DEFINE(NUMARGS, ...) \
void ConsoleMenu::AddCommand(const char* const input, const char* const description, FuncPtr##NUMARGS##Num func) \
{ mCommands.push_back(new ConsoleMenuCommand##NUMARGS##Num(input, description, func)); } \
void ConsoleMenu::ConsoleMenuCommand##NUMARGS##Num::Execute() \
{ mFunc( __VA_ARGS__ ); }

#define CONSOLE_MENU_TEXT_COMMAND_DECLARE(NUMARGS, ...) \
	using FuncPtrText##NUMARGS##Num = void(*)(__VA_ARGS__); \
	class ConsoleMenuCommandText##NUMARGS##Num : public ConsoleMenuCommandI \
	{ \
	public: \
		ConsoleMenuCommandText##NUMARGS##Num(const char* const command, const char* const description, FuncPtrText##NUMARGS##Num func) \
			: ConsoleMenuCommandI(command, description, InputMode::kText) \
		{ mFunc = func; } \
		virtual ~ConsoleMenuCommandText##NUMARGS##Num() = default; \
		virtual void Execute() override; \
		virtual size_t GetNumArgs() const override { return NUMARGS; } \
		virtual void SetArg(size_t pos, const char* const val) override { mArgs[pos] = val; } \
	private: \
		FuncPtrText##NUMARGS##Num mFunc; \
		std::string mArgs[ NUMARGS ]; \
	}; \
	void AddCommand(const char* const input, const char* const description, FuncPtrText##NUMARGS##Num func)

#define CONSOLE_MENU_TEXT_COMMAND_DEFINE(NUMARGS, ...) \
void ConsoleMenu::AddCommand(const char* const input, const char* const description, FuncPtrText##NUMARGS##Num func) \
{ mCommands.push_back(new ConsoleMenuCommandText##NUMARGS##Num(input, description, func)); } \
void ConsoleMenu::ConsoleMenuCommandText##NUMARGS##Num::Execute() \
{ mFunc( __VA_ARGS__ ); }

class ConsoleMenu
{
public:
	ConsoleMenu(const char* const description)
		: mDescription(description)
	{
		mInputBuffer[0] = '\0';
    }

    ConsoleMenu(const char* const description, ConsoleMenu& parentMenu)
        : mDescription(description)
        , mParentMenu(&parentMenu)
    {
        mInputBuffer[0] = '\0';
    }

	~ConsoleMenu();
	
	void ResetMenu();
	void PrintHorizontalBreak();
	bool ReceiveInput(char c);

    void AddCommand(const char* const input, const char* const description, FuncPtr0Num func);
    CONSOLE_MENU_COMMAND_DECLARE(1, uint64_t);
    CONSOLE_MENU_COMMAND_DECLARE(2, uint64_t, uint64_t);
    CONSOLE_MENU_COMMAND_DECLARE(3, uint64_t, uint64_t, uint64_t);
    CONSOLE_MENU_COMMAND_DECLARE(4, uint64_t, uint64_t, uint64_t, uint64_t);

    CONSOLE_MENU_TEXT_COMMAND_DECLARE(1, const char* const);
    CONSOLE_MENU_TEXT_COMMAND_DECLARE(2, const char* const, const char* const);
    CONSOLE_MENU_TEXT_COMMAND_DECLARE(3, const char* const, const char* const, const char* const);
    CONSOLE_MENU_TEXT_COMMAND_DECLARE(4, const char* const, const char* const, const char* const, const char* const);

    // SubMenus
    class ConsoleMenuCommandSubMenu : public ConsoleMenuCommandI
    {
    public:
		ConsoleMenuCommandSubMenu(const char* const command, ConsoleMenu& subMenu)
            : ConsoleMenuCommandI(command, subMenu.mDescription.c_str(), InputMode::kText)
			, mConsoleMenu(subMenu)
        {
        }
        virtual ~ConsoleMenuCommandSubMenu() = default;

        virtual bool IsSubMenu() const override { return true; }
		ConsoleMenu& GetMenu() { return mConsoleMenu; }
    private:
		ConsoleMenu& mConsoleMenu;
    };
	void AddSubmenu(const char* const input, ConsoleMenu& subMenu);

private:
	void ReceiveCommandInput(char c);
	void ReceiveTextInput(char c);
	void ReceiveNumberInput(char c);

	void ConvertModeTo(InputMode mode);

	void StartParamInput();
	void PutInput(char c);
	void BackspaceLastInput();
	void ClearInput();
	void StartNewInputOnNewLine();
	void ExecuteCurrentCommand();

	InputMode mInputMode = InputMode::kCommand;

	std::vector<ConsoleMenuCommandI*> mCommands;
	std::string mDescription;
	char mInputBuffer[16];
	size_t mCurrentPos = 0u;
	size_t mCurrentParam = 0u;

	ConsoleMenu* mParentMenu = nullptr;
	ConsoleMenuCommandI* mCurrentCommand = nullptr;
	ConsoleMenu* mCurrentSubMenu = nullptr;
};