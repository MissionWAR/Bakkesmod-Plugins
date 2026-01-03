#include "pch.h"
#include "plugin.h"

BAKKESMOD_PLUGIN(RankedEssentials, "Ranked Essentials", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void RankedEssentials::onLoad()
{
	_globalCvarManager = cvarManager;
	LOG("Ranked Essentials plugin loaded!");

	// Initialize settings with sensible defaults
	skipReplay = std::make_shared<bool>(true);
	checkTeammates = std::make_shared<bool>(true);
	autoQueue = std::make_shared<bool>(false);
	autoFreeplay = std::make_shared<bool>(false);
	queueDelay = std::make_shared<float>(1.0f);        // Default: 1 second
	freeplayDelay = std::make_shared<float>(1.0f);     // Default: 1 second
	freeplayMapIndex = std::make_shared<int>(0);
	queueDisableForCasual = std::make_shared<bool>(false);
	freeplayDisableForCasual = std::make_shared<bool>(false);

	// Register CVars for replay settings
	cvarManager->registerCvar("re_skip_replay", "1", "Auto-skip goal replays", true, true, 0, true, 1)
		.bindTo(skipReplay);

	cvarManager->registerCvar("re_check_teammates", "1", "Don't skip replay if teammate is missing", true, true, 0, true, 1)
		.bindTo(checkTeammates);

	// Register CVars for post-match settings
	cvarManager->registerCvar("re_auto_queue", "0", "Auto-queue after match ends", true, true, 0, true, 1)
		.bindTo(autoQueue);

	cvarManager->registerCvar("re_auto_freeplay", "0", "Enter freeplay after match ends", true, true, 0, true, 1)
		.bindTo(autoFreeplay);

	cvarManager->registerCvar("re_queue_delay", "1.0", "Delay before queueing (seconds)", true, true, 0.0f, true, 5.0f)
		.bindTo(queueDelay);

	cvarManager->registerCvar("re_freeplay_delay", "1.0", "Delay before entering freeplay (seconds)", true, true, 0.0f, true, 5.0f)
		.bindTo(freeplayDelay);

	cvarManager->registerCvar("re_freeplay_map", "0", "Freeplay map index", true, true, 0, true, FREEPLAY_MAP_COUNT - 1)
		.bindTo(freeplayMapIndex);

	// Register CVars for per-feature casual toggles
	cvarManager->registerCvar("re_queue_disable_casual", "0", "Disable auto-queue in casual matches", true, true, 0, true, 1)
		.bindTo(queueDisableForCasual);

	cvarManager->registerCvar("re_freeplay_disable_casual", "0", "Disable auto-freeplay in casual matches", true, true, 0, true, 1)
		.bindTo(freeplayDisableForCasual);

	// Register toggle command for keybinding
	cvarManager->registerNotifier("re_toggle_skip_replay", [this](std::vector<std::string> args) {
		ToggleSkipReplay();
	}, "Toggle auto-skip replay on/off", PERMISSION_ALL);

	// Hook replay start event
	gameWrapper->HookEvent(
		"Function GameEvent_Soccar_TA.ReplayPlayback.ShouldPlayReplay",
		std::bind(&RankedEssentials::OnReplayStart, this, std::placeholders::_1));

	// Hook match end event
	gameWrapper->HookEvent(
		"Function TAGame.GameEvent_Soccar_TA.EventMatchEnded",
		std::bind(&RankedEssentials::OnMatchEnded, this, std::placeholders::_1));

	LOG("Ranked Essentials hooks registered");
}

void RankedEssentials::onUnload()
{
	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.ShouldPlayReplay");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded");
	LOG("Ranked Essentials plugin unloaded");
}

void RankedEssentials::ToggleSkipReplay()
{
	bool newValue = !*skipReplay;
	cvarManager->getCvar("re_skip_replay").setValue(newValue);
	
	std::string message = newValue ? "Auto-skip replays ENABLED" : "Auto-skip replays DISABLED";
	gameWrapper->Toast("Ranked Essentials", message, "default", 3.0f);
	LOG("{}", message);
}

bool RankedEssentials::IsRankedMatch()
{
	auto server = gameWrapper->GetCurrentGameState();
	if (!server) return false;

	auto playlist = server.GetPlaylist();
	if (!playlist) return false;

	int playlistId = playlist.GetPlaylistId();
	return playlistId == 10 || playlistId == 11 || playlistId == 13 ||
	       playlistId == 27 || playlistId == 28 || playlistId == 29 || playlistId == 30;
}

bool RankedEssentials::IsCasualMatch()
{
	auto server = gameWrapper->GetCurrentGameState();
	if (!server) return false;

	auto playlist = server.GetPlaylist();
	if (!playlist) return false;

	int playlistId = playlist.GetPlaylistId();
	return playlistId >= 1 && playlistId <= 4;
}

bool RankedEssentials::IsTournamentMatch()
{
	auto server = gameWrapper->GetCurrentGameState();
	if (!server) return false;

	auto playlist = server.GetPlaylist();
	if (!playlist) return false;

	int playlistId = playlist.GetPlaylistId();
	return playlistId == 22 || playlistId == 34;
}

bool RankedEssentials::IsPrivateMatch()
{
	auto server = gameWrapper->GetCurrentGameState();
	if (!server) return false;

	auto playlist = server.GetPlaylist();
	if (!playlist) return false;

	int playlistId = playlist.GetPlaylistId();
	return playlistId == 6 || playlistId == 8;
}

bool RankedEssentials::HasMissingTeammate()
{
	auto controller = gameWrapper->GetPlayerController();
	if (!controller) return false;

	auto pri = controller.GetPRI();
	if (!pri) return false;

	auto server = gameWrapper->GetCurrentGameState();
	if (!server) return false;

	unsigned char myTeam = pri.GetTeamNum();
	auto players = server.GetPRIs();
	int teamCount = 0;

	for (int i = 0; i < players.Count(); i++) {
		auto player = players.Get(i);
		if (!player) continue;
		if (player.GetTeamNum() == myTeam) {
			teamCount++;
		}
	}

	int maxTeamSize = server.GetMaxTeamSize();
	return teamCount < maxTeamSize;
}

int RankedEssentials::GetRandomMapIndex()
{
	// Random between 1 and FREEPLAY_MAP_COUNT-1 (skip index 0 which is "Random")
	return 1 + (std::rand() % (FREEPLAY_MAP_COUNT - 1));
}

std::string RankedEssentials::GetFreeplayMapCommand()
{
	int index = *freeplayMapIndex;
	
	// If "Random" selected (index 0), pick a random map
	if (index == 0) {
		index = GetRandomMapIndex();
	}
	
	if (index >= 1 && index < FREEPLAY_MAP_COUNT) {
		return std::string("load_freeplay ") + FREEPLAY_MAP_COMMANDS[index];
	}
	return "load_freeplay";
}

void RankedEssentials::OnReplayStart(std::string eventName)
{
	// Skip replays works EVERYWHERE - user can toggle with keybind if needed
	if (!*skipReplay) return;

	if (*checkTeammates && HasMissingTeammate()) {
		gameWrapper->Toast("Ranked Essentials",
			"Not skipping replay - teammate missing!",
			"default", 3.0f);
		return;
	}

	gameWrapper->ExecuteUnrealCommand("ReadyUp");
}

void RankedEssentials::OnMatchEnded(std::string eventName)
{
	bool isCasual = IsCasualMatch();
	bool isTournament = IsTournamentMatch();
	bool isPrivate = IsPrivateMatch();

	// ========== AUTO-QUEUE LOGIC ==========
	// Auto-queue does NOT work in private/tournament matches (makes no sense - you're not the host)
	bool shouldQueue = *autoQueue;
	
	if (shouldQueue && isPrivate) {
		// Silently skip - not the host anyway
		shouldQueue = false;
	}
	
	if (shouldQueue && isTournament) {
		// Tournaments auto-queue themselves - skip silently
		shouldQueue = false;
	}
	
	if (shouldQueue && *queueDisableForCasual && isCasual) {
		shouldQueue = false;
	}

	// ========== AUTO-FREEPLAY LOGIC ==========
	// Auto-freeplay works EVERYWHERE including tournaments (helpful to practice while waiting)
	bool shouldFreeplay = *autoFreeplay;
	
	if (shouldFreeplay && *freeplayDisableForCasual && isCasual) {
		shouldFreeplay = false;
	}

	// ========== EXECUTE ==========
	
	if (shouldQueue) {
		// If freeplay is also enabled, queue immediately (no point waiting)
		float delay = shouldFreeplay ? 0.1f : *queueDelay;
		
		gameWrapper->SetTimeout([this](GameWrapper* gw) {
			cvarManager->executeCommand("queue");
			gameWrapper->Toast("Ranked Essentials", "Queuing for next match...", "default", 2.0f);
			LOG("Auto-queuing for next match...");
		}, delay);
	}

	if (shouldFreeplay) {
		float delay = *freeplayDelay;
		if (shouldQueue) {
			delay += 0.3f; // Small extra delay after queue command
		}
		
		std::string mapName = FREEPLAY_MAPS[*freeplayMapIndex];
		gameWrapper->SetTimeout([this, mapName](GameWrapper* gw) {
			cvarManager->executeCommand(GetFreeplayMapCommand());
			gameWrapper->Toast("Ranked Essentials", "Loading " + mapName + "...", "default", 2.0f);
			LOG("Loading freeplay...");
		}, delay);
	}
}

// Helper to render a disabled checkbox
static bool DisabledCheckbox(const char* label, bool* value, bool enabled) {
	if (!enabled) {
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
	}
	
	bool changed = false;
	if (enabled) {
		changed = ImGui::Checkbox(label, value);
	} else {
		bool displayValue = *value;
		ImGui::Checkbox(label, &displayValue);
	}
	
	if (!enabled) {
		ImGui::PopStyleVar();
	}
	return changed;
}

// Helper to render a disabled slider
static bool DisabledSliderFloat(const char* label, float* value, float min, float max, bool enabled) {
	if (!enabled) {
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
	}
	
	bool changed = false;
	ImGui::PushItemWidth(200);
	if (enabled) {
		changed = ImGui::SliderFloat(label, value, min, max, "%.1f");
	} else {
		float displayValue = *value;
		ImGui::SliderFloat(label, &displayValue, min, max, "%.1f");
	}
	ImGui::PopItemWidth();
	
	if (!enabled) {
		ImGui::PopStyleVar();
	}
	return changed;
}

void RankedEssentials::RenderSettings()
{
	// Get CVars
	auto skipCvar = cvarManager->getCvar("re_skip_replay");
	auto teammateCvar = cvarManager->getCvar("re_check_teammates");
	auto queueCvar = cvarManager->getCvar("re_auto_queue");
	auto freeplayCvar = cvarManager->getCvar("re_auto_freeplay");
	auto queueDelayCvar = cvarManager->getCvar("re_queue_delay");
	auto freeplayDelayCvar = cvarManager->getCvar("re_freeplay_delay");
	auto mapCvar = cvarManager->getCvar("re_freeplay_map");
	auto queueCasualCvar = cvarManager->getCvar("re_queue_disable_casual");
	auto freeplayCasualCvar = cvarManager->getCvar("re_freeplay_disable_casual");

	if (!skipCvar || !teammateCvar || !queueCvar || !freeplayCvar || !mapCvar) return;

	// ============================================
	// REPLAY SETTINGS SECTION
	// ============================================
	if (ImGui::CollapsingHeader("Replay Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Indent(10.0f);
		
		bool skipValue = skipCvar.getBoolValue();
		if (ImGui::Checkbox("Auto-skip goal replays", &skipValue)) {
			skipCvar.setValue(skipValue);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Automatically skip goal replays in all match types");
		}

		// Keybind section
		ImGui::Spacing();
		ImGui::TextUnformatted("Toggle Keybind:");
		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Bind a key to toggle auto-skip on/off during gameplay");
		}
		
		static char keybindBuffer[32] = "";
		static std::string lastSetBind = "";
		
		if (!lastSetBind.empty()) {
			ImGui::Text("Current binding: %s", lastSetBind.c_str());
		}
		
		ImGui::PushItemWidth(100);
		ImGui::InputText("##keybind", keybindBuffer, sizeof(keybindBuffer));
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Button("Set Bind")) {
			if (strlen(keybindBuffer) > 0) {
				std::string cmd = std::string("bind ") + keybindBuffer + " re_toggle_skip_replay";
				cvarManager->executeCommand(cmd);
				lastSetBind = keybindBuffer;
				gameWrapper->Toast("Ranked Essentials", 
					std::string("Bound ") + keybindBuffer + " to toggle skip replay", 
					"default", 3.0f);
			}
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Enter a key name (e.g. F5, X, NumPadZero) and click Set Bind");
		}

		ImGui::Spacing();
		
		bool teammateValue = teammateCvar.getBoolValue();
		if (DisabledCheckbox("Don't skip if teammate is missing", &teammateValue, skipValue)) {
			teammateCvar.setValue(teammateValue);
		}
		if (ImGui::IsItemHovered()) {
			if (skipValue) {
				ImGui::SetTooltip("Prevents skipping when a teammate has disconnected,\ngiving them time to reconnect");
			} else {
				ImGui::SetTooltip("Enable 'Auto-skip goal replays' first");
			}
		}

		ImGui::Unindent(10.0f);
	}

	ImGui::Spacing();
	ImGui::Spacing();

	// ============================================
	// POST-MATCH SETTINGS SECTION
	// ============================================
	if (ImGui::CollapsingHeader("Post-Match Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Indent(10.0f);

		// Auto-queue
		bool queueValue = queueCvar.getBoolValue();
		if (ImGui::Checkbox("Enable Instant Queue", &queueValue)) {
			queueCvar.setValue(queueValue);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Automatically queue for the same playlist after match ends.\nNote: Automatically disabled in private matches and tournaments.");
		}

		// Queue casual toggle
		bool queueCasualValue = queueCasualCvar.getBoolValue();
		if (DisabledCheckbox("  Disable for Casual", &queueCasualValue, queueValue)) {
			queueCasualCvar.setValue(queueCasualValue);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Don't auto-queue in casual (unranked) matches");
		}

		float queueDelayVal = queueDelayCvar.getFloatValue();
		if (DisabledSliderFloat("Queue Delay (seconds)", &queueDelayVal, 0.0f, 5.0f, queueValue)) {
			queueDelayCvar.setValue(queueDelayVal);
		}

		ImGui::Spacing();

		// Auto-freeplay
		bool freeplayValue = freeplayCvar.getBoolValue();
		if (ImGui::Checkbox("Enable Instant Freeplay", &freeplayValue)) {
			freeplayCvar.setValue(freeplayValue);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Enter freeplay after match ends.\nWorks in tournaments too - practice while waiting!");
		}

		// Freeplay casual toggle
		bool freeplayCasualValue = freeplayCasualCvar.getBoolValue();
		if (DisabledCheckbox("  Disable for Casual", &freeplayCasualValue, freeplayValue)) {
			freeplayCasualCvar.setValue(freeplayCasualValue);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Don't auto-freeplay in casual (unranked) matches");
		}

		// Map dropdown
		int mapIndex = mapCvar.getIntValue();
		if (!freeplayValue) {
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
		}
		ImGui::PushItemWidth(200);
		if (ImGui::Combo("Freeplay Map", &mapIndex, FREEPLAY_MAPS, FREEPLAY_MAP_COUNT)) {
			if (freeplayValue) {
				mapCvar.setValue(mapIndex);
			}
		}
		ImGui::PopItemWidth();
		if (!freeplayValue) {
			ImGui::PopStyleVar();
		}

		float freeplayDelayVal = freeplayDelayCvar.getFloatValue();
		if (DisabledSliderFloat("Freeplay Delay (seconds)", &freeplayDelayVal, 0.0f, 5.0f, freeplayValue)) {
			freeplayDelayCvar.setValue(freeplayDelayVal);
		}

		// Info when both enabled
		if (queueValue && freeplayValue) {
			ImGui::Spacing();
			ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), 
				"Both enabled: Queue instantly, then enter freeplay");
		}

		ImGui::Unindent(10.0f);
	}

	ImGui::Spacing();
	ImGui::Spacing();

	// ============================================
	// ABOUT SECTION
	// ============================================
	if (ImGui::CollapsingHeader("About")) {
		ImGui::Indent(10.0f);
		ImGui::TextUnformatted("Ranked Essentials v1.3.0");
		ImGui::TextDisabled("Auto-queue, auto-freeplay, and auto-skip replays");
		ImGui::Spacing();
		ImGui::TextUnformatted("Smart Behavior:");
		ImGui::BulletText("Skip replays works everywhere (use keybind to toggle)");
		ImGui::BulletText("Auto-queue skips in private/tournament matches");
		ImGui::BulletText("Auto-freeplay works in tournaments (practice while waiting!)");
		ImGui::Spacing();
		ImGui::TextUnformatted("Console Commands:");
		ImGui::BulletText("re_toggle_skip_replay - Toggle auto-skip");
		ImGui::Unindent(10.0f);
	}
}
