#include "plugin.h"

#include <cstdio>

#include "citadel/client.h"
#include "utils.h"

// #include <igameevents.h>
// #include <iplayerinfo.h>

// Nice helper
#define ENGINE_CALL(func) SH_CALL(engine, &IVEngineServer::func)

// Simple accessor
CitadelAutoMatchPlugin& CitadelAutoMatchPlugin::instance() { return g_CitadelAutoMatchPlugin; }

// Set up sourcehook hooks
SH_DECL_HOOK2_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, edict_t *, const CCommand &);
SH_DECL_HOOK1_void(IServerGameClients, SetCommandClient, SH_NOATTRIB, false, int);
SH_DECL_HOOK1_void(ConCommand, Dispatch, SH_NOATTRIB, false, const CCommand &);

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
ICvar *icvar = NULL;
CGlobalVars *gpGlobals = NULL;

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

    gpGlobals = ismm->GetCGlobals();

    META_LOG(g_PLAPI, "Starting plugin.");

    /* Load the VSP listener.  This is usually needed for IServerPluginHelpers. */
    if ((vsp_callbacks = ismm->GetVSPInfo(NULL)) == NULL)
    {
        ismm->AddListener(this, this);
        ismm->EnableVSPListener();
    }

    // Hook functions
    SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this, &CitadelAutoMatchPlugin::onClientCommand, false);
    SH_ADD_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, serverClients, this, &CitadelAutoMatchPlugin::onSetCommandClient, false);
    SH_ADD_HOOK_MEMFUNC(ConCommand, Dispatch, icvar->FindCommand("say"), this, &CitadelAutoMatchPlugin::onSay, false);
    SH_ADD_HOOK_MEMFUNC(ConCommand, Dispatch, icvar->FindCommand("say_team"), this, &CitadelAutoMatchPlugin::onSayTeam, false);

    META_LOG(g_PLAPI, "CitadelAutoMatchPlugin loaded\n");
    Warning("Yo, Merry Christmas\n");

    requests = std::make_shared<Requests>();

    return true;
}

bool CitadelAutoMatchPlugin::Unload(char *error, size_t maxlen) {
    SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this, &CitadelAutoMatchPlugin::onClientCommand, false);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, serverClients, this, &CitadelAutoMatchPlugin::onSetCommandClient, false);
    SH_REMOVE_HOOK_MEMFUNC(ConCommand, Dispatch, icvar->FindCommand("say"), this, &CitadelAutoMatchPlugin::onSay, false);
    SH_REMOVE_HOOK_MEMFUNC(ConCommand, Dispatch, icvar->FindCommand("say_team"), this, &CitadelAutoMatchPlugin::onSayTeam, false);

    requests = nullptr;

    return true;
}

void CitadelAutoMatchPlugin::OnVSPListening(IServerPluginCallbacks *iface) {
    vsp_callbacks = iface;
}

static void logCommand(const CCommand &command) {
    for (size_t i = 0; i < command.ArgC(); i++) {
        Warning(format("%d %s\n", i, command.Arg(i)).c_str());
    }
}

void CitadelAutoMatchPlugin::onClientCommand(edict_t *pEntity, const CCommand &args) {
    logCommand(args);
}

void CitadelAutoMatchPlugin::onSetCommandClient(int client) {
    lastClient = client;
}

void CitadelAutoMatchPlugin::onSay(const CCommand &args) {
    handleSay(args, true);
}

void CitadelAutoMatchPlugin::onSayTeam(const CCommand &args) {
    handleSay(args, false);
}

void CitadelAutoMatchPlugin::handleSay(const CCommand &command, bool isToAll) {
    assert(command.ArgC() == 2, "Invalid number of arguments to `say` command");

    std::string line(command.Arg(1));
    trim(line);

    if (line[0] == '!') {
        handleCommand(line.substr(1));
    }
}

void CitadelAutoMatchPlugin::handleCommand(const std::string &command) {
    if (activeMatch) {

    } else if (matchPicker) {
        // auto match = matchPicker->onClientCommand();

        // if (match) {
        //     matchPicker = nullptr;
        //     activeMatch = std::move(match);
        // }
    } else {
        if (levenshtein_distance(command, "match") <= 2) {
            startMatch();
        }
    }
}

void CitadelAutoMatchPlugin::startMatch() {
    // TODO: CVar
    std::vector<std::string> endpoints;
    endpoints.push_back("https://127.0.0.1:3000");

    matchPicker = MatchPicker::create<citadel::Client>(requests, endpoints);

    // TODO:
    // matchPicket.queryAll()
}

