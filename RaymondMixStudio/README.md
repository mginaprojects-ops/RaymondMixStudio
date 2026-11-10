# Raymond Mix Studio (VST3 + Standalone) — Windows

Plugin ya mixing na mastering: EQ, saturation, compressor, reverb, delay (inafuata tempo ya DAW),
preset 18 (Bongo Flava, Amapiano, Afrobeat, Singeli, Gengetone/Hip-Hop, Pop, Injili na Kwaya, Mastering),
auto-BPM kutoka kwenye audio, na export ya WAV.

## Unachohitaji (mara moja tu)
1. **Visual Studio 2022** (Community ni bure). Wakati wa kusakinisha chagua workload
   **"Desktop development with C++"** (hii inajumuisha CMake).
2. **Git for Windows**: https://git-scm.com (CMake inapakua JUCE yenyewe kutoka GitHub, hivyo unahitaji internet).

## Njia A — bonyeza mara mbili (rahisi)
1. Toa (extract) ZIP hili kwenye folda, mfano `C:\RaymondMixStudio`.
2. Bonyeza mara mbili **`build_windows.bat`**. Itachukua dakika 5–15 mara ya kwanza.
3. Ikimaliza, plugin iko hapa:
   `build\RaymondMixStudio_artefacts\Release\VST3\Raymond Mix Studio.vst3`
   Standalone (programu ya kujitegemea):
   `build\RaymondMixStudio_artefacts\Release\Standalone\Raymond Mix Studio.exe`

## Njia B — kupitia Visual Studio
1. Fungua Visual Studio 2022 → **File → Open → Folder…** → chagua folda ya mradi (yenye `CMakeLists.txt`).
2. Subiri CMake ikamilishe "configure", chagua **x64-Release**, kisha **Build → Build All**.
   (Au tengeneza solution: `cmake -B build -G "Visual Studio 17 2022" -A x64` kisha fungua `build\RaymondMixStudio.sln`.)

## Kusakinisha kwenye DAW
1. Nakili folda/faili `Raymond Mix Studio.vst3` kwenda
   `C:\Program Files\Common Files\VST3\` (inahitaji ruhusa ya Administrator).
2. Fungua Cubase au Studio One na ufanye **Plug-in scan / Rescan**.
3. Ingiza kwenye track au master bus: **Raymond Mix Studio**.

## Jinsi ya kutumia
- **Ndani ya DAW (Cubase / Studio One):** delay na reverb zinafuata tempo ya project *automatically*
  (utaona "DAW HOST" juu). Export ya mwisho (WAV/MP3) ifanywe na DAW yako: File → Export → Audio Mixdown.
- **Standalone:** "Load Audio" → tempo inatambuliwa yenyewe → chagua preset → "Export WAV".
- **Auto Tempo** (kitufe juu): inarekebisha compressor release na urefu wa reverb kulingana na tempo.
- Bonyeza mara mbili kwenye kifundo kurudisha thamani ya awali.

## Maelezo
- MP3: JUCE haina MP3 encoder iliyojengewa. Tumia export ya MP3 ya Cubase/Studio One, au geuza WAV kuwa MP3 kwa ffmpeg/Audacity.
- Leseni: JUCE ina leseni mbili (AGPLv3 au ya kibiashara). Kwa matumizi binafsi ni sawa; ukiuza plugin, soma https://juce.com/juce-8-licence
- Msimbo uko kwenye `Source/`: `Engine.h` (DSP), `PluginProcessor.*` (parameters, presets), `PluginEditor.*` (muonekano).
