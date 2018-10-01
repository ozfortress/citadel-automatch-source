#include <cstdio>
#include <memory>
#include <inttypes.h>

#define META_NO_HL2SDK
#include "iserverplugin.h"
#include "igameevents.h"
#include "game/server/iplayerinfo.h"
#include "game/server/ientityinfo.h"
#include "edict.h"
#include "eiface.h"
#include "steam/steamclientpublic.h"
#include "ISmmPlugin.h"
#include "provider/provider_ep2.h"
#include "irecipientfilter.h"
#include "KeyValues.h"
#include "iserver.h"
#include "iclient.h"
// Fix std::min and std::max
#undef min
#undef max

#include "citadel/client.h"
#include "utils.h"
#include "match.h"
#include "match_picker.h"
#include "requests.h"
#include "steam_id.h"

#define VERSION "0.1.0"

#define MAX_PLAYERS 65

// Metamod stuff
extern CGlobalVars *gpGlobals;
PLUGIN_GLOBALVARS();

// Set up sourcehook hooks
SH_DECL_HOOK2_void(IServerGameClients, ClientCommand, SH_NOATTRIB, false, edict_t *, const CCommand &);
SH_DECL_HOOK1_void(IServerGameClients, SetCommandClient, SH_NOATTRIB, false, int);
SH_DECL_HOOK1_void(ConCommand, Dispatch, SH_NOATTRIB, false, const CCommand &);
SH_DECL_HOOK5(IServerGameClients, ClientConnect, SH_NOATTRIB, false, bool, edict_t *, const char *, const char *, char *, int);
SH_DECL_HOOK2_void(IServerGameClients, ClientPutInServer, SH_NOATTRIB, false, edict_t *, const char *);
SH_DECL_HOOK1_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, false, edict_t *);
SH_DECL_HOOK5_void(IServerGameDLL, OnQueryCvarValueFinished, SH_NOATTRIB, false, QueryCvarCookie_t, edict_t *, EQueryCvarValueStatus, const char *, const char *);
SH_DECL_HOOK5_void(IServerPluginCallbacks, OnQueryCvarValueFinished, SH_NOATTRIB, false, QueryCvarCookie_t, edict_t *, EQueryCvarValueStatus, const char *, const char *);

// Some globals
// CitadelAutoMatchPlugin g_CitadelAutoMatchPlugin;
// Metamod stuff
IServerGameDLL *server = NULL;
IServerGameClients *gameclients = NULL;
IVEngineServer *engine = NULL;
IServerGameClients *serverClients = NULL;
IServerPluginHelpers *helpers = NULL;
IGameEventManager2 *gameevents = NULL;
IServerPluginCallbacks *vsp_callbacks = NULL;
IPlayerInfoManager *playerinfomanager = NULL;
IServer *iserver = NULL;
ICvar *icvar = NULL;
CGlobalVars *gpGlobals = NULL;

// Declare vars
extern ConCommand cas_confirmation_complete;
extern ConCommand cas_confirmation_progress;

// Pre-declare some stuff
namespace {
    // Simple IRecipientFilter implementation. Required for sending messages
    class RecipientFilter: public IRecipientFilter {
        int count = 0;
        int recipients[MAX_PLAYERS];

    public:
        bool IsReliable() const override { return true; }
        bool IsInitMessage() const override { return false; }
        int GetRecipientCount() const override { return count; }
        int GetRecipientIndex(int index) const override {
            if (index < 0 || index >= count) {
                return -1;
            }
            return recipients[index];
        }

        void addPlayer(int playerId) {
            sassert(count < MAX_PLAYERS, "Overflow");

            recipients[count] = playerId;
            count++;
        }

        // Define this later
        void addAll();
    };

    // TODO: Maybe add a single recipient filter

    // TODO: Cache results?
    int getMessageType(const char * messageName) {
        char name[255];
        int size = 0;

        for (int msg_type = 0;; msg_type++) {
            if (!server->GetUserMessageInfo(msg_type, name, 255, size)) {
                break;
            }

            if (strcmp(name, messageName) == 0) {
                return msg_type;
            }
        }

        return -1;
    }

    void sendMessage(IRecipientFilter *filter, const char *message) {
        bf_write *pBuffer = engine->UserMessageBegin(filter, getMessageType("SayText"));
        pBuffer->WriteByte(0);
        pBuffer->WriteString(message);
        pBuffer->WriteByte(true);
        engine->MessageEnd();
    }

    void showViewPortPanel(IRecipientFilter *filter, const char* name, bool bShow, KeyValues* data) {
        int count = 0;
        KeyValues *subkey = NULL;

        if (data) {
            subkey = data->GetFirstSubKey();
            while (subkey) {
                count++;
                subkey = subkey->GetNextKey();
            }

            subkey = data->GetFirstSubKey(); // reset
        }

        bf_write *netmsg = engine->UserMessageBegin(filter, getMessageType("VGUIMenu"));

        netmsg->WriteString(name); // the HUD message itself
        netmsg->WriteByte(bShow ? 1 : 0); // index of the player this message comes from. "1" means the server.
        netmsg->WriteByte(count); // I don't know yet the purpose of this byte, it can be 1 or 0

        // write additional data (be careful not more than 192 bytes!)
        while (subkey) {
            netmsg->WriteString(subkey->GetName()); // the HUD message itself
            netmsg->WriteString(subkey->GetString()); // the HUD message itself
            subkey = subkey->GetNextKey();
        }

        engine->MessageEnd();
    }
}

class CitadelAutoMatchPlugin : public ISmmPlugin, public IMetamodListener, public IGame {
    struct Player final : public IPlayer {
        int id;
        edict_t *edict = nullptr;
        IPlayerInfo *info = nullptr;
        SteamID steam_id = SteamID::null;
        bool allow_html_motd = false;

        Player() {
            this->id = -1;
        }

        Player(int id) {
            this->id = id;
        }

        Player(edict_t *edict) {
            this->id = engine->IndexOfEdict(edict);
            this->edict = edict;
            this->info = playerinfomanager->GetPlayerInfo(edict);
            printf("Creating player {index: %d, id: %d}\n", engine->IndexOfEdict(edict), this->info->GetUserID());
            sassert(this->info != nullptr, "PlayerInfo is null");

            const CSteamID *cSteamId = engine->GetClientSteamID(edict);
            sassert(cSteamId != nullptr, "CSteamID is null");

            this->steam_id = SteamID(cSteamId->ConvertToUint64());
        }

        bool isValid() const {
            return edict != nullptr;
        }

        const std::string_view getName() const override {
            return std::string_view(engine->GetClientConVarValue(id, "name"));
        }

        void setName(const std::string_view name) override {
            std::string str;
            str.reserve(name.length());

            // Truncate and escape
            for (size_t i = 0; i < MAX_PLAYER_NAME_LENGTH - 1 && i < name.length(); i++) {
                const char c = name[i];

                if (c == '"') {
                    str += "\\\"";
                } else if (c == '\\') {
                    str += "\\\\";
                } else {
                    str += c;
                }
            }

            // Override the con var locally
            // char *current_name = (char *)engine->GetClientConVarValue(id, "name");
            // strcpy(current_name, str.c_str());

            // Send the client a set name command
            engine->ClientCommand(edict, format("name \"%s\"", str.c_str()).c_str());
        }

        SteamID getSteamID() const override { return steam_id; }

        Team getTeam() const override {
            auto team_index = info->GetTeamIndex();
            if (team_index == 2) {
                return Team::team1;
            } else if (team_index == 3) {
                return Team::team2;
            } else {
                return Team::other;
            }
        }

        void notify(const std::string_view message) override {
            RecipientFilter filter;
            filter.addPlayer(id);

            sendMessage(&filter, message.data());
        }

        void notifyError(const std::string_view message) override {
            RecipientFilter filter;
            filter.addPlayer(id);

            sendMessage(&filter, "Citadel AutoMatch Error:");
            sendMessage(&filter, message.data());
        }

        void openMOTD(const std::string_view title, const std::string_view url) override {
            if (!allow_html_motd) {
                notify("ERROR: You have cl_disablehtmlmotd not set to 0. You can follow this url instead:");
                notify(url);
                return;
            }

            RecipientFilter filter;
            filter.addPlayer(id);

            KeyValues* kv = new KeyValues("data");

            kv->SetString("title", title.data());
            kv->SetString("type", "2"); // Type=URL
            kv->SetString("msg", url.data());
            kv->SetString("cmd", "cas_player_motd_close");
            kv->SetInt("customsvr", 1);
            kv->SetInt("unload", 1);

            showViewPortPanel(&filter, "info", true, kv);

            kv->deleteThis();
        }
    };

    std::shared_ptr<Requests> requests;

    std::unique_ptr<MatchPicker> match_picker;
    std::unique_ptr<Match> active_match;

    // Keep track of which client called a command last
    // Use this for determining who ran a "say" command
    int last_set_command_client_id = -1;

    typedef Player ConnectedPlayers[MAX_PLAYERS];

    // Source sucks, so we need to keep track of players manually
    int player_count = 0;
    ConnectedPlayers connected_players;

    CitadelAutoMatchPlugin() {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            connected_players[i] = Player(i);
        }
    }
    ~CitadelAutoMatchPlugin() {}

    void handleSay(const CCommand& command, bool isToAll) {
        sassert(command.ArgC() == 2, "Invalid number of arguments to `say` command");

        std::string line(command.Arg(1));
        trim(line);

        auto& player = getPlayerByID(last_set_command_client_id);

        if (line[0] == '!') {
            handleCommand(&player, line.substr(1));
        }
    }

    void handleCommand(Player *player, const std::string& command) {
        if (active_match) {
            active_match->onCommand(player, command);
        } else if (match_picker) {
            auto match = match_picker->onCommand(player, command);

            if (match) {
                match_picker.reset();
                active_match = std::move(match);
                active_match->start(player);
            }
        } else {
            if (levenshtein_distance(command, "match") <= 2) {
                startMatch(player);
            }
        }
    }

    void startMatch(Player *starter) {
        // TODO: CVar
        std::vector<std::string> endpoints;
        endpoints.push_back("http://127.0.0.1:3000");

        match_picker = MatchPicker::create<citadel::Client>(starter, requests, endpoints, (IGame *)this);

        auto players = getValidPlayers();
        // Retarded C++ casting required
        match_picker->queryAll((std::vector<IPlayer *>&)players);
    }

public:
    // Singleton instance
    static CitadelAutoMatchPlugin& instance() {
        static CitadelAutoMatchPlugin g_instance;

        return g_instance;
    }

    // ISmmPlugin
    const char *GetAuthor() override { return "Benjamin Schaaf"; };
    const char *GetName() override { return "CitadelAutoMatch"; };
    const char *GetDescription() override { return "Automagically handle matches for the citadel league framework."; };
    const char *GetURL() override { return ""; };
    const char *GetLicense() override { return "GPLv3"; };
    const char *GetVersion() override { return VERSION; };
    const char *GetDate() override { return "??"; };
    const char *GetLogTag() override { return "CAS"; };

    // Called on plugin load
    bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late) override {
        PLUGIN_SAVEVARS();

        GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
        GET_V_IFACE_CURRENT(GetServerFactory, serverClients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
        GET_V_IFACE_CURRENT(GetEngineFactory, gameevents, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);
        GET_V_IFACE_CURRENT(GetEngineFactory, helpers, IServerPluginHelpers, INTERFACEVERSION_ISERVERPLUGINHELPERS);
        GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
        GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
        GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
        GET_V_IFACE_ANY(GetServerFactory, playerinfomanager, IPlayerInfoManager, INTERFACEVERSION_PLAYERINFOMANAGER);

        iserver = engine->GetIServer();
        gpGlobals = ismm->GetCGlobals();

        META_LOG(g_PLAPI, "Starting plugin.");

        /* Load the VSP listener.  This is usually needed for IServerPluginHelpers. */
        if ((vsp_callbacks = ismm->GetVSPInfo(NULL)) == NULL)
        {
            ismm->AddListener(this, this);
            ismm->EnableVSPListener();
        }

        printf("!!!!!!!!!!! GameDLLVersion = %d\n", ismm->GetGameDLLVersion());

        // OnVPSListening won't get called if it's already there
        if (vsp_callbacks) {
            OnVSPListening(vsp_callbacks);
        }

        // Player events
        SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this, &CitadelAutoMatchPlugin::onClientCommand, false);
        SH_ADD_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, serverClients, this, &CitadelAutoMatchPlugin::onSetCommandClient, false);
        SH_ADD_HOOK_MEMFUNC(ConCommand, Dispatch, icvar->FindCommand("say"), this, &CitadelAutoMatchPlugin::onSay, false);
        SH_ADD_HOOK_MEMFUNC(ConCommand, Dispatch, icvar->FindCommand("say_team"), this, &CitadelAutoMatchPlugin::onSayTeam, false);
        SH_ADD_HOOK_MEMFUNC(IServerGameDLL, OnQueryCvarValueFinished, server, this, &CitadelAutoMatchPlugin::onQueryCvarValueFinished, false);

        // Server events
        SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientConnect, serverClients, this, &CitadelAutoMatchPlugin::onClientConnect, false);
        SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, serverClients, this, &CitadelAutoMatchPlugin::onClientPutInServer, true);
        SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, serverClients, this, &CitadelAutoMatchPlugin::onClientDisconnect, false);

        // Commands
        icvar->RegisterConCommand(&cas_confirmation_complete);
        icvar->RegisterConCommand(&cas_confirmation_progress);

        META_LOG(g_PLAPI, "CitadelAutoMatchPlugin loaded\n");
        Warning("Yo, Merry Christmas\n");

        requests = std::make_shared<Requests>();

        return true;
    }

    bool Unload(char *error, size_t maxlen) override {
        SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this, &CitadelAutoMatchPlugin::onClientCommand, false);
        SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, serverClients, this, &CitadelAutoMatchPlugin::onSetCommandClient, false);
        SH_REMOVE_HOOK_MEMFUNC(ConCommand, Dispatch, icvar->FindCommand("say"), this, &CitadelAutoMatchPlugin::onSay, false);
        SH_REMOVE_HOOK_MEMFUNC(ConCommand, Dispatch, icvar->FindCommand("say_team"), this, &CitadelAutoMatchPlugin::onSayTeam, false);
        SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientConnect, serverClients, this, &CitadelAutoMatchPlugin::onClientConnect, false);
        SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, serverClients, this, &CitadelAutoMatchPlugin::onClientPutInServer, true);
        SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, serverClients, this, &CitadelAutoMatchPlugin::onClientDisconnect, false);
        SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, OnQueryCvarValueFinished, server, this, &CitadelAutoMatchPlugin::onQueryCvarValueFinished, false);

        icvar->UnregisterConCommand(&cas_confirmation_complete);
        icvar->UnregisterConCommand(&cas_confirmation_progress);

        if (vsp_callbacks) {
            SH_REMOVE_HOOK_MEMFUNC(IServerPluginCallbacks, OnQueryCvarValueFinished, vsp_callbacks, this, &CitadelAutoMatchPlugin::onQueryCvarValueFinished, false);
        }

        requests = nullptr;

        return true;
    }

    // IMetamodListener
    void OnVSPListening(IServerPluginCallbacks *iface) override {
        vsp_callbacks = iface;

        SH_ADD_HOOK_MEMFUNC(IServerPluginCallbacks, OnQueryCvarValueFinished, vsp_callbacks, this, &CitadelAutoMatchPlugin::onQueryCvarValueFinished, false);
    }

    // Source Hooks
    void onClientCommand(edict_t *pEntity, const CCommand& args) {}

    void onSetCommandClient(int client_index) {
        printf("Pre-command client %d\n", client_index);

        last_set_command_client_id = client_index + 1;
    }

    void onSay(const CCommand& args) { handleSay(args, true); }

    void onSayTeam(const CCommand& args) { handleSay(args, false); }

    bool onClientConnect(edict_t *edict, const char *name, const char *address, char *reject, int maxrejectlen) {
        // TODO: Rejection logic?
    }

    void onClientPutInServer(edict_t * edict, const char * name) {
        Player player(edict);

        // Safely handle if client is still "connected"
        if (connected_players[player.id].isValid()) {
            onClientDisconnect(connected_players[player.id].edict);
        }

        player_count++;
        connected_players[player.id - 1] = player;

        // Query for cl_disablehtmlmod. Maybe do this for every openMOTD in-case it changes?
        helpers->StartQueryCvarValue(edict, "cl_disablehtmlmotd");

        printf("%s [%" PRIu64 "] (%d) joined server. Now at %d players.\n", name, player.steam_id.value, player.id, player_count);
    }

    void onClientDisconnect(edict_t *edict) {
        auto& player = getPlayer(edict);

        if (player.isValid()) {
            printf("[%" PRIu64 "] (%d) disconnected. Now at %d players.\n", player.steam_id.value, player.id, player_count);

            player = Player(player.id);
            player_count--;
        }
    }

    void onQueryCvarValueFinished(
        QueryCvarCookie_t iCookie,
        edict_t *edict,
        EQueryCvarValueStatus status,
        const char *name,
        const char *value) {

        printf("!!!!!!!!!! OnQueryCvarValueFinished\n");
        if (status != eQueryCvarValueStatus_ValueIntact) {
            return;
        }

        printf("Received CVar %s = %s\n", name, value);
        if (strcmp(name, "cl_disablehtmlmotd") != 0) {
            return;
        }

        auto& player = getPlayer(edict);
        player.allow_html_motd = strcmp(value, "0") == 0;
    }

    // Commands
    void onConfirmationComplete() {
        if (active_match) {
            active_match->onServerConfirm();
        }
    }

    void onConfirmationProgress() {
        if (active_match) {
            active_match->onServerConfirmationProgress();
        }
    }

    // IGame
    int serverPort() const override {
        return iserver->GetUDPPort();
    }

    const std::string_view serverPassword() const override {
        return iserver->GetPassword();
    }

    const std::string_view serverRConPassword() const override {
        return icvar->FindVar("rcon_password")->GetString();
    }

    const std::string_view serverName() const override {
        return iserver->GetName();
    }

    std::vector<IPlayer *> team1Players() override {
        return getPlayersByTeam<IPlayer>(Team::team1);
    }

    std::vector<IPlayer *> team2Players() override {
        return getPlayersByTeam<IPlayer>(Team::team2);
    }

    std::vector<IPlayer *> nonTeamPlayers() override {
        return getPlayersByTeam<IPlayer>(Team::other);
    }

    void resetMatch() override {
        printf("Resetting match\n");

        if (active_match) {
            active_match = nullptr;
        } else if (match_picker) {
            match_picker = nullptr;
        }

        notifyAll("Match has been reset.");
    }

    void notifyAll(const std::string_view message) override {
        RecipientFilter filter;
        filter.addAll();

        sendMessage(&filter, message.data());
    }

    void notifyAllError(const std::string_view message) override {
        RecipientFilter filter;
        filter.addAll();

        sendMessage(&filter, "Citadel AutoMatch Error:");
        sendMessage(&filter, message.data());
    }

    // Accessors
    Player& getPlayerByID(int id) {
        sassert(0 <= id && id < MAX_PLAYERS, "Player ID out of range");

        auto& p = connected_players[id - 1];
        sassert(p.isValid(), "Player not valid");
        return p;
    }

    // Player& getPlayerByIndex(int index) {
    //     auto *info = iserver->GetClient(index);
    //     return getPlayerByID(info->GetUserID());
    // }

    Player& getPlayer(edict_t *edict) {
        auto id = engine->IndexOfEdict(edict);
        return getPlayerByID(id);
    }

    Player *getPlayerBySteamID(SteamID id) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            auto *player = &connected_players[i];

            if (player->steam_id == id) {
                return player;
            }
        }

        return nullptr;
    }

    ConnectedPlayers& getPlayers() {
        return connected_players;
    }

    template<typename T = Player>
    std::vector<T *> getValidPlayers() {
        std::vector<T *> players;
        for (auto& player : connected_players) {
            if (player.isValid()) {
                players.push_back(&player);
            }
        }
        return players;
    }

    template<typename T = Player>
    std::vector<T *> getPlayersByTeam(Team team) {
        std::vector<T *> players;
        for (auto &player : connected_players) {
            if (player.isValid() && player.getTeam() == team) {
                players.push_back(&player);
            }
        }
        return players;
    }

    int getPlayerCount() { return player_count; }
};

// Notification of registration confirmation complete
static void onConfirmationComplete(const CCommand &args) {
    CitadelAutoMatchPlugin::instance().onConfirmationComplete();
}

ConCommand cas_confirmation_complete("cas_confirmation_complete", onConfirmationComplete , "", 0);

// Notification of registration confirmation progress
static void onConfirmationProgress(const CCommand &args) {
    CitadelAutoMatchPlugin::instance().onConfirmationProgress();
}

ConCommand cas_confirmation_progress("cas_confirmation_progress", onConfirmationProgress , "", 0);


// Set up metamod plugin
PLUGIN_EXPOSE(CitadelAutoMatchPlugin, CitadelAutoMatchPlugin::instance());

// Define some co-dependent things
namespace {
    void RecipientFilter::addAll() {
        for (auto& player : CitadelAutoMatchPlugin::instance().getPlayers()) {
            if (player.isValid()) {
                addPlayer(player.id);
            }
        }
    }
}
