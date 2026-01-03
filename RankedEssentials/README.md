# Ranked Essentials

Auto-queue, auto-freeplay, and auto-skip replays for Rocket League.

## Features

- **Auto-skip goal replays** - With keybind toggle and teammate detection
- **Auto-queue** - Queue for next match automatically
- **Auto-freeplay** - Enter freeplay with 60+ map choices

## Install

1. **[Download the latest release](../../../releases/latest)**
2. Copy `RankedEssentials.dll` to `%APPDATA%\bakkesmod\bakkesmod\plugins\`
3. Configure in-game: F2 → Plugins → "Ranked Essentials"


## Settings

### Replay Settings
- Auto-skip goal replays (on/off)
- Toggle keybind (e.g., F5)
- Don't skip if teammate is missing

### Post-Match Settings
- Instant Queue (with casual toggle)
- Instant Freeplay (60+ maps with Random option)

## Smart Behavior

| Match Type | Skip Replays | Auto-Queue | Auto-Freeplay |
|------------|--------------|------------|---------------|
| Ranked | ✅ | ✅ | ✅ |
| Casual | ✅ | Optional | Optional |
| Tournament | ✅ | ❌ (auto) | ✅ |
| Private | ✅ | ❌ (auto) | ✅ |

## Console Commands

```
re_toggle_skip_replay    # Toggle skip on/off
re_skip_replay 0/1
re_auto_queue 0/1
re_auto_freeplay 0/1
```

## Build

```powershell
msbuild BakkesPluginTemplate.vcxproj /p:Configuration=Release /p:Platform=x64
```
