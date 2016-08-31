#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H
#define COMMAND_HANDLER(name) bool name (CommandArgs & vargs, const char *args, const char *description, CUser* pSendUser)

typedef std::list<std::string> CommandArgs;

template <class T>
class Command
{
public:
	const char* name;
	COMMAND_HANDLER((T::*handler));
	const char* help;
};

class CommandHandler
{
public:
	CommandHandler();
	~CommandHandler();
	bool InitCommands();

	bool HandleChatCommand(Packet& pkt, CUser* pSender);

	bool HandleGlobalChat(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
	bool HandleHelp(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
	bool HandleTest(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
	bool HandleHome(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
	bool HandleGivePlayerItem(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
	bool HandleUpgradeItem(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
	bool HandleMakeHole(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
	bool HandleFindItem(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
	bool HandleZoneChange(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
	bool HandleTeleport(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
	bool HandleGMAnnounce(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
	bool HandleMorph(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
	bool HandleDemorph(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
	bool HandleSetLevel(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
	bool HandleModSpeed(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
	bool HandleResetStats(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
	bool HandleHeal(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
	bool HandleAddGold(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser);
};

typedef std::map<std::string, Command<CommandHandler> *> CommandTable;

static CommandTable s_commandTable;

__forceinline void* allocate_and_copy(uint32 len, void * pointer)
{
	void * data = (void*)malloc(len);
	if (data == NULL)
		return data;

	memcpy(data, pointer, len);
	return data;
}

#define init_command_table(t, command_table, command_map) \
	for (int i = 0; i < sizeof(command_table) / sizeof(*command_table); i++) \
		command_map.insert(std::make_pair(command_table[i].name, (Command<t> *)(allocate_and_copy(sizeof(*command_table), (void *)&command_table[i]))));

#define free_command_table(command_map) \
	for (auto itr = command_map.begin(); itr != command_map.end(); ++itr) \
		delete itr->second; \
	command_map.clear();

static std::list<std::string> StrSplit(const std::string &src, const std::string &sep)
{
	std::list<std::string> r;
	std::string s;
	for (std::string::const_iterator i = src.begin(); i != src.end(); ++i)
	{
		if (sep.find(*i) != std::string::npos)
		{
			if (!s.empty())
				r.push_back(s);
			s = "";
		}
		else
		{
			s += *i;
		}
	}
	if (!s.empty())
		r.push_back(s);
	return r;
}

extern CommandHandler* g_commandHandler;
#endif