#pragma once

#include <memory>

#define META_NO_HL2SDK
#include "iserverplugin.h"
#include "igameevents.h"
#include "game/server/iplayerinfo.h"
#include "edict.h"
#include "eiface.h"
#include "steam/steamclientpublic.h"
#include "ISmmPlugin.h"
#include "provider/provider_ep2.h"
#undef min
#undef max

#include "match.h"
#include "match_picker.h"
#include "requests.h"
#include "steam_id.h"

#define AUTHOR "Benjamin Schaaf"
#define NAME "CitadelAutoMatch"
#define DESCRIPTION "Automagically handle matches for the citadel league framework."
#define URL ""
#define LICENSE "GPLv3"
#define VERSION "0.1.0"
#define DATE "???"
#define LOG_TAG "CAS"

#define MAX_PLAYERS 65

// Shitty metamod stuff
extern CGlobalVars *gpGlobals;

class CitadelAutoMatchPlugin : public ISmmPlugin, public IMetamodListener {
public:
    struct PlayerInfo {
        bool connected = false;
        edict_t *edict = nullptr;
        IPlayerInfo *info = nullptr;
        SteamID steamId = SteamID::null;

        void Init(edict_t *edict, SteamID steamId) {
            this->connected = true;
            this->edict = edict;
            this->steamId = steamId;
        }
    };

    static CitadelAutoMatchPlugin& instance();

    std::shared_ptr<Requests> requests;
    std::shared_ptr<IGame> game;

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

    void onClientCommand(edict_t *pEntity, const CCommand& args);
    void onSetCommandClient(int client);
    void onSay(const CCommand& args);
    void onSayTeam(const CCommand& args);
    bool onClientConnect(edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen);
    void onClientPutInServer(edict_t *pEntity, char const *playername);
    void onClientDisconnect(edict_t *pEntity);

    const PlayerInfo *getPlayerBySteamID(const SteamID steamId);

    void resetMatch();

private:
    int lastClient;

    // Source sucks, so we need to keep track of players manually
    size_t playerCount = 0;
    PlayerInfo players[MAX_PLAYERS + 1]; // 1 extra as they start at 1

    void handleSay(const CCommand& command, bool isToAll);
    void handleCommand(const SteamID player, const std::string& command);
    void startMatch(const SteamID starter);
};

extern CitadelAutoMatchPlugin g_CitadelAutoMatchPlugin;

PLUGIN_GLOBALVARS();
