#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

// Complete freeplay map list (2026) - from rlstats.net
static const char* FREEPLAY_MAPS[] = {
	"Random",
	// AquaDome
	"AquaDome",
	"AquaDome (Salty Shallows)",
	// Beckwith Park
	"Beckwith Park",
	"Beckwith Park (Midnight)",
	"Beckwith Park (Snowy)",
	"Beckwith Park (Stormy)",
	// Boostfield Mall
	"Boostfield Mall",
	// Champions Field
	"Champions Field",
	"Champions Field (Day)",
	"Champions Field (NFL)",
	// Deadeye Canyon
	"Deadeye Canyon",
	"Deadeye Canyon (Oasis)",
	// DFH Stadium
	"DFH Stadium",
	"DFH Stadium (Day)",
	"DFH Stadium (Stormy)",
	"DFH Stadium (Snowy)",
	"DFH Stadium (Circuit)",
	// Drift Woods
	"Drift Woods (Dawn)",
	"Drift Woods (Night)",
	// Estadio Vida
	"Estadio Vida",
	// Farmstead
	"Farmstead",
	"Farmstead (Night)",
	"Farmstead (Spooky)",
	"Farmstead (The Upside Down)",
	// Forbidden Temple
	"Forbidden Temple",
	"Forbidden Temple (Day)",
	"Forbidden Temple (Fire & Ice)",
	// Futura Garden
	"Futura Garden",
	// Mannfield
	"Mannfield",
	"Mannfield (Dusk)",
	"Mannfield (Night)",
	"Mannfield (Snowy)",
	"Mannfield (Stormy)",
	// Neo Tokyo
	"Neo Tokyo",
	"Neo Tokyo (Arcade)",
	"Neo Tokyo (Comic)",
	"Neo Tokyo (Hacked)",
	// Neon Fields
	"Neon Fields",
	// Parc de Paris
	"Parc de Paris",
	// Rivals Arena
	"Rivals Arena",
	// Salty Shores
	"Salty Shores",
	"Salty Shores (Night)",
	"Salty Shores (Salty Fest)",
	// Sovereign Heights
	"Sovereign Heights",
	// Starbase ARC
	"Starbase ARC",
	"Starbase ARC (Aftermath)",
	// Urban Central
	"Urban Central",
	"Urban Central (Dawn)",
	"Urban Central (Night)",
	"Urban Central (Haunted)",
	// Utopia Coliseum
	"Utopia Coliseum",
	"Utopia Coliseum (Dusk)",
	"Utopia Coliseum (Gilded)",
	"Utopia Coliseum (Snowy)",
	// Wasteland
	"Wasteland",
	"Wasteland (Night)"
};

static const char* FREEPLAY_MAP_COMMANDS[] = {
	"",  // Random
	// AquaDome
	"underwater_p",
	"underwater_grs_p",
	// Beckwith Park
	"park_p",
	"park_night_p",
	"park_snowy_p",
	"park_rainy_p",
	// Boostfield Mall
	"mall_day_p",
	// Champions Field
	"cs_p",
	"cs_day_p",
	"bb_p",
	// Deadeye Canyon
	"outlaw_p",
	"outlaw_oasis_p",
	// DFH Stadium
	"stadium_p",
	"stadium_day_p",
	"stadium_foggy_p",
	"stadium_winter_p",
	"stadium_race_day_p",
	// Drift Woods
	"woods_p",
	"woods_night_p",
	// Estadio Vida
	"ff_dusk_p",
	// Farmstead
	"farm_p",
	"farm_night_p",
	"farm_hw_p",
	"farm_upsidedown_p",
	// Forbidden Temple
	"chn_stadium_p",
	"chn_stadium_day_p",
	"fni_stadium_p",
	// Futura Garden
	"uf_day_p",
	// Mannfield
	"eurostadium_p",
	"eurostadium_dusk_p",
	"eurostadium_night_p",
	"eurostadium_snownight_p",
	"eurostadium_rainy_p",
	// Neo Tokyo
	"neotokyo_standard_p",
	"neotokyo_arcade_p",
	"neotokyo_toon_p",
	"neotokyo_hax_p",
	// Neon Fields
	"music_p",
	// Parc de Paris
	"paname_dusk_p",
	// Rivals Arena
	"cs_hw_p",
	// Salty Shores
	"beach_p",
	"beach_night_p",
	"beach_night_grs_p",
	// Sovereign Heights
	"street_p",
	// Starbase ARC
	"arc_standard_p",
	"arc_darc_p",
	// Urban Central
	"trainstation_p",
	"trainstation_dawn_p",
	"trainstation_night_p",
	"trainstation_spooky_p",
	// Utopia Coliseum
	"utopiastadium_p",
	"utopiastadium_dusk_p",
	"utopiastadium_lux_p",
	"utopiastadium_snow_p",
	// Wasteland
	"wasteland_s_p",
	"wasteland_night_s_p"
};

static const int FREEPLAY_MAP_COUNT = sizeof(FREEPLAY_MAPS) / sizeof(FREEPLAY_MAPS[0]);

class RankedEssentials : public BakkesMod::Plugin::BakkesModPlugin,
	public SettingsWindowBase
{
private:
	// Replay settings
	std::shared_ptr<bool> skipReplay;
	std::shared_ptr<bool> checkTeammates;

	// Post-match settings
	std::shared_ptr<bool> autoQueue;
	std::shared_ptr<bool> autoFreeplay;
	std::shared_ptr<float> queueDelay;
	std::shared_ptr<float> freeplayDelay;
	std::shared_ptr<int> freeplayMapIndex;

	// Per-feature casual toggles
	std::shared_ptr<bool> queueDisableForCasual;
	std::shared_ptr<bool> freeplayDisableForCasual;

	// Helper methods
	bool HasMissingTeammate();
	bool IsRankedMatch();
	bool IsTournamentMatch();
	bool IsPrivateMatch();
	bool IsCasualMatch();
	void OnReplayStart(std::string eventName);
	void OnMatchEnded(std::string eventName);
	void ToggleSkipReplay();
	std::string GetFreeplayMapCommand();
	int GetRandomMapIndex();

public:
	void onLoad() override;
	void onUnload() override;
	void RenderSettings() override;
};
