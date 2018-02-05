#include "plugin.h"

#define DESCRIPTION "Automagically handle matches for the citadel league framework."

CitadelAutoMatchPlugin _singletonInstance;

// Expose the singleton to source
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CitadelAutoMatchPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, _singletonInstance);

CitadelAutoMatchPlugin& CitadelAutoMatchPlugin::instance() {
    return _singletonInstance;
}

CitadelAutoMatchPlugin::CitadelAutoMatchPlugin() {}
CitadelAutoMatchPlugin::~CitadelAutoMatchPlugin() {}

bool CitadelAutoMatchPlugin::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) {
    Warning("Yo, Merry Christmas\n");

    return true;
}


void CitadelAutoMatchPlugin::Unload() {

}

void CitadelAutoMatchPlugin::Pause() {

}

void CitadelAutoMatchPlugin::UnPause() {

}

const char *CitadelAutoMatchPlugin::GetPluginDescription() {
    return DESCRIPTION;
}

void CitadelAutoMatchPlugin::LevelInit(char const *pMapName) {

}

void CitadelAutoMatchPlugin::ServerActivate(edict_t *pEdictList, int edictCount, int clientMax) {

}

void CitadelAutoMatchPlugin::GameFrame(bool simulating) {

}

void CitadelAutoMatchPlugin::LevelShutdown() {

}

void CitadelAutoMatchPlugin::ClientActive(edict_t *pEntity) {

}

void CitadelAutoMatchPlugin::ClientDisconnect(edict_t *pEntity) {

}

void CitadelAutoMatchPlugin::ClientPutInServer(edict_t *pEntity, char const *playername) {

}

void CitadelAutoMatchPlugin::SetCommandClient(int index) {

}

void CitadelAutoMatchPlugin::ClientSettingsChanged(edict_t *pEdict) {

}

PLUGIN_RESULT CitadelAutoMatchPlugin::ClientConnect(bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen) {
    return PLUGIN_CONTINUE;
}

PLUGIN_RESULT CitadelAutoMatchPlugin::ClientCommand(edict_t *pEntity, const CCommand &args) {
    return PLUGIN_CONTINUE;
}

PLUGIN_RESULT CitadelAutoMatchPlugin::NetworkIDValidated(const char *pszUserName, const char *pszNetworkID) {
    return PLUGIN_CONTINUE;
}

void CitadelAutoMatchPlugin::OnQueryCvarValueFinished(QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue) {

}

void CitadelAutoMatchPlugin::OnEdictAllocated(edict_t *edict) {

}

void CitadelAutoMatchPlugin::OnEdictFreed(const edict_t *edict ) {

}

void CitadelAutoMatchPlugin::FireGameEvent(KeyValues * event) {

}

