#pragma once

#include <memory>

#define META_NO_HL2SDK
#include "iserverplugin.h"
#include "igameevents.h"
#include "game/server/iplayerinfo.h"
#include "edict.h"
#include "eiface.h"
#include "ISmmPlugin.h"
#include "provider/provider_ep2.h"
#undef min
#undef max

#include "match.h"
#include "match_picker.h"
#include "requests.h"

#define AUTHOR "Benjamin Schaaf"
#define NAME "CitadelAutoMatch"
#define DESCRIPTION "Automagically handle matches for the citadel league framework."
#define URL ""
#define LICENSE "GPLv3"
#define VERSION "0.1.0"
#define DATE "???"
#define LOG_TAG "CAS"

// Shitty metamod stuff
extern CGlobalVars *gpGlobals;

class CitadelAutoMatchPlugin : public ISmmPlugin, public IMetamodListener {
public:
    static CitadelAutoMatchPlugin& instance();

    std::shared_ptr<Requests> requests;

    std::unique_ptr<MatchPicker> matchPicker;
    std::unique_ptr<Match> activeMatch;

    CitadelAutoMatchPlugin();
    ~CitadelAutoMatchPlugin();

    const char *GetAuthor() override { return AUTHOR; };
    const char *GetName() override { return NAME; };
    const char *GetDescription() override { return DESCRIPTION; };
    const char *GetURL() override { return URL; };
    const char *GetLicense() override { return LICENSE; };
    const char *GetVersion() override { return VERSION; };
    const char *GetDate() override { return DATE; };
    const char *GetLogTag() override { return LOG_TAG; };

    bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlength, bool late) override;
    bool Unload(char *error, size_t maxlen) override;

    void OnVSPListening(IServerPluginCallbacks *iface) override;

    void onClientCommand(edict_t *pEntity, const CCommand &args);
    void onSetCommandClient(int client);
    void onSay(const CCommand &args);
    void onSayTeam(const CCommand &args);

private:
    int lastClient;

    void handleSay(const CCommand &command, bool isToAll);
    void handleCommand(const std::string &command);
    void startMatch();
};

extern CitadelAutoMatchPlugin g_CitadelAutoMatchPlugin;

PLUGIN_GLOBALVARS();
