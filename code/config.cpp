#ifndef CONFIG_CPP
#define CONFIG_CPP

#define CONFIG_FILE "mwsilver.ini"

#define CONFIG_SETTINGS_SECTION "settings"
#define CONFIG_MESSAGE_SECTION "message"
#define CONFIG_KEYS_SECTION "keys"

#define CONFIG_PRESAVE "bSaveGamePreActivation"
#define CONFIG_POSTSAVE "bSaveGamePostActivation"
#define CONFIG_SHOWMESSAGES "bShowMessages"
#define CONFIG_SHOWMESSAGES_RANDOM "bShowMessagesRandom"
#define CONFIG_SAVEFILE "sSaveFile"
#define CONFIG_SAVENAME "sSaveName"
#define CONFIG_MESSAGE "sMessage"
#define CONFIG_MESSAGE_RANDOM "sMessageRandom"
#define CONFIG_MESSAGE_TOGGLE_ON "sMessageToggleOn"
#define CONFIG_MESSAGE_TOGGLE_OFF "sMessageToggleOff"
#define CONFIG_TIMER "iTimeout"
#define CONFIG_AUTOSAVE "bAutoSave"

#define CONFIG_DEFAULT_SAVEFILE "mwsilver_save"
#define CONFIG_DEFAULT_SAVENAME "MWSilver Save"
#define CONFIG_DEFAULT_MESSAGE "%s activated"
#define CONFIG_DEFAULT_MESSAGE_RANDOM "%s activated"
#define CONFIG_DEFAULT_MESSAGE_TOGGLE_ON "Commands are ON"
#define CONFIG_DEFAULT_MESSAGE_TOGGLE_OFF "Commands are OFF"
#define CONFIG_DEFAULT_TIMER (15 * 60 * 1000)

#define STRING_SIZE 256

internal char SaveFileName[STRING_SIZE];
internal char SaveDisplayName[STRING_SIZE];
internal char Message[STRING_SIZE];
internal char MessageRandom[STRING_SIZE];
internal char MessageOn[STRING_SIZE];
internal char MessageOff[STRING_SIZE];

internal bool ShowMessages = false;
internal bool ShowMessagesRandom = true;
internal bool ActualGameplay = false;
internal bool SavePreActivation = false;
internal bool SavePostActivation = false;
internal bool AutoSaveEnabled = true;

internal uint Timeout = 0;

internal void SettingsInitialize(HMODULE module)
{
  //TODO(adm244): rewrite using full path in these functions (pass it into the functions)
  SavePreActivation = IniReadBool(module, CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_PRESAVE, false);
  SavePostActivation = IniReadBool(module, CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_POSTSAVE, true);
  ShowMessages = IniReadBool(module, CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_SHOWMESSAGES, true);
  ShowMessagesRandom = IniReadBool(module, CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_SHOWMESSAGES_RANDOM, true);
  AutoSaveEnabled = IniReadBool(module, CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_AUTOSAVE, true);
  
  IniReadString(module, CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_SAVEFILE,
    CONFIG_DEFAULT_SAVEFILE, SaveFileName, STRING_SIZE);
  IniReadString(module, CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_SAVENAME,
    CONFIG_DEFAULT_SAVENAME, SaveDisplayName, STRING_SIZE);
    
  IniReadString(module, CONFIG_FILE, CONFIG_MESSAGE_SECTION, CONFIG_MESSAGE,
    CONFIG_DEFAULT_MESSAGE, Message, STRING_SIZE);
  IniReadString(module, CONFIG_FILE, CONFIG_MESSAGE_SECTION, CONFIG_MESSAGE_RANDOM,
    CONFIG_DEFAULT_MESSAGE_RANDOM, MessageRandom, STRING_SIZE);
  IniReadString(module, CONFIG_FILE, CONFIG_MESSAGE_SECTION, CONFIG_MESSAGE_TOGGLE_ON,
    CONFIG_DEFAULT_MESSAGE_TOGGLE_ON, MessageOn, STRING_SIZE);
  IniReadString(module, CONFIG_FILE, CONFIG_MESSAGE_SECTION, CONFIG_MESSAGE_TOGGLE_OFF,
    CONFIG_DEFAULT_MESSAGE_TOGGLE_OFF, MessageOff, STRING_SIZE);
  
  Timeout = IniReadInt(module, CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_TIMER, CONFIG_DEFAULT_TIMER);
}

#endif
