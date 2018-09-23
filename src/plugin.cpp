#include "plugin.h"

#include <cstdio>
#include <inttypes.h>

#include "irecipientfilter.h"
#include "KeyValues.h"
#include "iserver.h"
#include "iclient.h"

#include "citadel/client.h"
#include "utils.h"

// Nice helper
#define ENGINE_CALL(func) SH_CALL(engine, &IVEngineServer::func)

// Simple accessor
CitadelAutoMatchPlugin& CitadelAutoMatchPlugin::instance() { return g_CitadelAutoMatchPlugin; }

// Set up sourcehook hooks
SH_DECL_HOOK2_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, edict_t *, const CCommand &);
SH_DECL_HOOK1_void(IServerGameClients, SetCommandClient, SH_NOATTRIB, false, int);
SH_DECL_HOOK1_void(ConCommand, Dispatch, SH_NOATTRIB, false, const CCommand &);
SH_DECL_HOOK5(IServerGameClients, ClientConnect, SH_NOATTRIB, 0, bool, edict_t *, const char *, const char *, char *, int);
SH_DECL_HOOK2_void(IServerGameClients, ClientPutInServer, SH_NOATTRIB, 0, edict_t *, const char *);
SH_DECL_HOOK1_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, edict_t *);

// Some globals
CitadelAutoMatchPlugin g_CitadelAutoMatchPlugin;
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

namespace {
    class RecipientFilter: public IRecipientFilter {
    public:
        virtual bool IsReliable() const override { return true; }
        virtual bool IsInitMessage() const override { return false; }
        virtual int GetRecipientCount() const override { return count; }
        virtual int GetRecipientIndex (int index) const override {
            if (index < 0 || index >= count) {
                return -1;
            }
            return recipients[index];
        }

        void addPlayer(int playerId) {
            assert(count < 255, "Overflow");

            recipients[count] = playerId;
            count++;
        }

        void addAll() {
            for (int i = 0; i < iserver->GetNumClients(); i++) {
                IClient *pClient = iserver->GetClient(i);
                if ( !pClient || !pClient->IsConnected())
                    continue;

                addPlayer(i + 1);
            }
        }

    private:
        int count = 0;
        int recipients[256];
    };
}

static int GetMessageType(const char * messageName) {
    char name[255];
    int size = 0;

    for (int msg_type = 0;; msg_type++) {
        if(!server->GetUserMessageInfo(msg_type, name, 255, size))
            break;

        if (strcmp(name, messageName) == 0)
            return msg_type;
    }

    return -1;
}

static void ShowViewPortPanel(edict_t* pEdict, const char* name, bool bShow, KeyValues* data) {
    RecipientFilter filter; // the corresponding recipient filter

    // pPlayerEdict is a pointer to the edict of a player for which you want this HUD message
    // to be displayed. There can be several recipients in the filter, don't forget.
    filter.addPlayer(engine->IndexOfEdict(pEdict));

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

    bf_write *netmsg = engine->UserMessageBegin(&filter, GetMessageType("VGUIMenu"));

    netmsg->WriteString(name); // the HUD message itself
    netmsg->WriteByte(bShow ? 1 : 0); // index of the player this message comes from. "1" means the server.
    netmsg->WriteByte(count); // I don't know yet the purpose of this byte, it can be 1 or 0

    // write additional data (be carefull not more than 192 bytes!)
    while (subkey) {
        netmsg->WriteString(subkey->GetName()); // the HUD message itself
        netmsg->WriteString(subkey->GetString()); // the HUD message itself
        subkey = subkey->GetNextKey();
    }

    engine->MessageEnd();
}

static void Message(RecipientFilter *filter, const char *message) {
    bf_write *pBuffer = engine->UserMessageBegin(filter, GetMessageType("SayText"));
    pBuffer->WriteByte(0);
    pBuffer->WriteString(message);
    pBuffer->WriteByte(true);
    engine->MessageEnd();
}

namespace {
    class Game : public IGame {
        std::string serverAddress() override {
            return "";
        }

        std::string serverPassword() override {
            return "";
        }

        std::string serverRConPassword() override {
            return "";
        }

        std::vector<SteamID> team1Players() override {
            return std::vector<SteamID>();
        }

        std::vector<SteamID> team2Players() override {
            return std::vector<SteamID>();
        }

        void notifyError(std::string message, SteamID target) override {
            if (target == SteamID::null) {
                notifyAll("ERROR: " + message);
            } else {
                notify(target, "ERROR: " + message);
            }
        }

        void resetMatch() override {
            g_CitadelAutoMatchPlugin.resetMatch();
        }

        void notifyAll(std::string message) override {
            RecipientFilter filter;
            filter.addAll();
            Message(&filter, message.c_str());
        }

        void notify(SteamID player, std::string message) override {
            const auto *info = g_CitadelAutoMatchPlugin.getPlayerBySteamID(player);
            if (info == nullptr) {
                printf("Not opening MOTD due to null info\n");
                return;
            }
            if (info->edict == nullptr) {
                printf("Not opening MOTD due to null edict\n");
                return;
            }

            RecipientFilter filter;
            filter.addPlayer(engine->IndexOfEdict(info->edict));

            Message(&filter, message.c_str());
        }

        void openMOTD(SteamID player, std::string title, std::string url) override {
            assert(player != SteamID::null, "Invalid Steam ID for MOTD");

            printf("Opening MOTD for %"PRIu64" at %s\n", player.value, url.c_str());

            const auto *info = g_CitadelAutoMatchPlugin.getPlayerBySteamID(player);
            if (info == nullptr) {
                printf("Not opening MOTD due to null info\n");
                return;
            }
            if (info->edict == nullptr) {
                printf("Not opening MOTD due to null edict\n");
                return;
            }

            KeyValues* kv = new KeyValues("data");

            kv->SetString("title", title.c_str());
            kv->SetString("type", "2"); // Type=URL
            kv->SetString("msg", url.c_str());
            // TODO: Command on close
            // kv->SetString("cmd", lpcCmd);// exec this command if panel closed
            kv->SetInt("cmd", 0);
            kv->SetInt("customsvr", 1);
            kv->SetInt("unload", 1);

            ShowViewPortPanel(info->edict, "info", true, kv);

            kv->deleteThis();
        }
    };
}

// Set up metamod plugin
PLUGIN_EXPOSE(CitadelAutoMatchPlugin, g_CitadelAutoMatchPlugin);

CitadelAutoMatchPlugin::CitadelAutoMatchPlugin() {}
CitadelAutoMatchPlugin::~CitadelAutoMatchPlugin() {}

bool CitadelAutoMatchPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late) {
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

    // Player events
    SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this, &CitadelAutoMatchPlugin::onClientCommand, false);
    SH_ADD_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, serverClients, this, &CitadelAutoMatchPlugin::onSetCommandClient, false);
    SH_ADD_HOOK_MEMFUNC(ConCommand, Dispatch, icvar->FindCommand("say"), this, &CitadelAutoMatchPlugin::onSay, false);
    SH_ADD_HOOK_MEMFUNC(ConCommand, Dispatch, icvar->FindCommand("say_team"), this, &CitadelAutoMatchPlugin::onSayTeam, false);

    // Server events
    SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientConnect, serverClients, this, &CitadelAutoMatchPlugin::onClientConnect, false);
    SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, serverClients, this, &CitadelAutoMatchPlugin::onClientPutInServer, true);
    SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, serverClients, this, &CitadelAutoMatchPlugin::onClientDisconnect, false);

    META_LOG(g_PLAPI, "CitadelAutoMatchPlugin loaded\n");
    Warning("Yo, Merry Christmas\n");

    requests = std::make_shared<Requests>();
    if (!game) game = std::make_shared<Game>();

    return true;
}

bool CitadelAutoMatchPlugin::Unload(char *error, size_t maxlen) {
    SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this, &CitadelAutoMatchPlugin::onClientCommand, false);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, serverClients, this, &CitadelAutoMatchPlugin::onSetCommandClient, false);
    SH_REMOVE_HOOK_MEMFUNC(ConCommand, Dispatch, icvar->FindCommand("say"), this, &CitadelAutoMatchPlugin::onSay, false);
    SH_REMOVE_HOOK_MEMFUNC(ConCommand, Dispatch, icvar->FindCommand("say_team"), this, &CitadelAutoMatchPlugin::onSayTeam, false);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientConnect, serverClients, this, &CitadelAutoMatchPlugin::onClientConnect, false);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, serverClients, this, &CitadelAutoMatchPlugin::onClientPutInServer, true);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, serverClients, this, &CitadelAutoMatchPlugin::onClientDisconnect, false);

    requests = nullptr;

    return true;
}

void CitadelAutoMatchPlugin::OnVSPListening(IServerPluginCallbacks *iface) {
    vsp_callbacks = iface;
}

static void logCommand(const CCommand& command) {
    for (size_t i = 0; i < command.ArgC(); i++) {
        Warning(format("%d %s\n", i, command.Arg(i)).c_str());
    }
}

void CitadelAutoMatchPlugin::onClientCommand(edict_t *pEdict, const CCommand& args) {
    logCommand(args);
}

void CitadelAutoMatchPlugin::onSetCommandClient(int client) {
    lastClient = client + 1;
}

void CitadelAutoMatchPlugin::onSay(const CCommand& args) {
    handleSay(args, true);
}

void CitadelAutoMatchPlugin::onSayTeam(const CCommand& args) {
    handleSay(args, false);
}

void CitadelAutoMatchPlugin::handleSay(const CCommand& command, bool isToAll) {
    assert(command.ArgC() == 2, "Invalid number of arguments to `say` command");

    std::string line(command.Arg(1));
    trim(line);

    auto& player = players[lastClient];

    printf("Player is connected: %d\n", player.connected);

    if (line[0] == '!') {
        handleCommand(player.steamId, line.substr(1));
    }
}

void CitadelAutoMatchPlugin::handleCommand(const SteamID steamId, const std::string& command) {
    if (activeMatch) {
        activeMatch->onCommand(steamId, command);
    } else if (matchPicker) {
        auto match = matchPicker->onCommand(steamId, command);

        if (match) {
            matchPicker = nullptr;
            activeMatch = std::move(match);
            activeMatch->start(steamId);
        }
    } else {
        if (levenshtein_distance(command, "match") <= 2) {
            startMatch(steamId);
        }
    }
}

void CitadelAutoMatchPlugin::startMatch(const SteamID starter) {
    // TODO: CVar
    std::vector<std::string> endpoints;
    endpoints.push_back("http://127.0.0.1:3000");

    matchPicker = MatchPicker::create<citadel::Client>(starter, requests, endpoints, game);

    // TODO: Get players
    std::vector<SteamID> players;
    players.push_back(starter);

    matchPicker->queryAll(players);
}

bool CitadelAutoMatchPlugin::onClientConnect(edict_t *pEdict, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen) {
    int clientIndex = engine->IndexOfEdict(pEdict);
    PlayerInfo *player = &players[clientIndex];

    // Safely handle if client is still "connected"
    if (player->connected) {
        onClientDisconnect(player->edict);
    }

    // Get the SteamID
    const CSteamID *cSteamId = engine->GetClientSteamID(pEdict);
    auto steamId = SteamID(cSteamId->ConvertToUint64());

    player->Init(pEdict, steamId);

    playerCount++;

    printf("%s - %s [%"PRIu64"] (%d) joined server. Now at %d players.\n", pszName, pszAddress, player->steamId.value, clientIndex, playerCount);
}

void CitadelAutoMatchPlugin::onClientPutInServer(edict_t *pEdict, char const *playername) {
    int clientIndex = engine->IndexOfEdict(pEdict);
    PlayerInfo *player = &players[clientIndex];

    if (player->connected) {
        player->info = playerinfomanager->GetPlayerInfo(pEdict);
    }

    printf("%s connecting\n", playername);
}

void CitadelAutoMatchPlugin::onClientDisconnect(edict_t *pEdict) {
    int clientIndex = engine->IndexOfEdict(pEdict);
    PlayerInfo *player = &players[clientIndex];

    if (player->connected) {
        playerCount--;

        printf("[%"PRIu64"] (%d) disconnected. Now at %d players.\n", player->steamId.value, clientIndex, playerCount);

        *player = PlayerInfo();
    }
}

const CitadelAutoMatchPlugin::PlayerInfo *CitadelAutoMatchPlugin::getPlayerBySteamID(const SteamID steamId) {
    for (size_t i = 1; i < playerCount + 1; i++) {
        const auto *info = &players[i];

        if (info->connected && info->steamId == steamId) {
            return info;
        }
    }

    return nullptr;
}

void CitadelAutoMatchPlugin::resetMatch() {
    printf("Resetting match\n");

    if (activeMatch) {
        activeMatch = nullptr;
    } else if (matchPicker) {
        matchPicker = nullptr;
    }

    game->notifyAll("Match has been reset.");
}
