# GhostSurf VST3 — Build Instructions

## What you need (free tools)

| Tool | Download | Notes |
|------|----------|-------|
| **Visual Studio 2022** | visualstudio.microsoft.com | Community edition — free. Install "Desktop development with C++" |
| **CMake 3.15+** | cmake.org/download | Add to PATH during install |
| **Git** | git-scm.com | To clone JUCE |
| **Inno Setup 6** | jrsoftware.org/isinfo.php | To build the .exe installer |

---

## Step 1 — Clone JUCE into the project

Open **PowerShell** or **Git Bash** inside `C:\Users\bmenc\Desktop\GhostSurf\`:

```bash
cd "C:\Users\bmenc\Desktop\GhostSurf"
git clone https://github.com/juce-framework/JUCE.git
```

This creates `GhostSurf/JUCE/` — required by CMakeLists.txt.

---

## Step 2 — Generate the Visual Studio project

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
```

If you have Visual Studio 2019 instead:
```bash
cmake .. -G "Visual Studio 16 2019" -A x64
```

---

## Step 3 — Build the plugin (Release mode)

**Option A — Visual Studio GUI:**
1. Open `build\GhostSurf.sln`
2. Set configuration to **Release** (top dropdown)
3. Right-click **GhostSurf_VST3** → Build

**Option B — Command line:**
```bash
cmake --build . --config Release
```

The built plugin will be at:
```
build\GhostSurf_artefacts\Release\VST3\GhostSurf.vst3\
```

---

## Step 4 — Install manually (quick test)

Copy the entire `GhostSurf.vst3` **folder** to:
```
C:\Program Files\Common Files\VST3\
```

Then rescan plugins in FL Studio:  
`Options → Manage Plugins → Scan`

---

## Step 5 — Build the installer (.exe)

1. Make sure the plugin built successfully (Step 3)
2. Open `install.iss` in **Inno Setup**
3. Click **Build → Compile** (or press F9)
4. The installer appears in `GhostSurf\Installer\GhostSurf_Setup_v1.0.0.exe`

---

## Project structure

```
GhostSurf/
├── CMakeLists.txt          ← Build system
├── install.iss             ← Inno Setup installer script
├── BUILD.md                ← This file
├── JUCE/                   ← JUCE framework (git clone)
└── Source/
    ├── PluginProcessor.h   ← DSP engine header
    ├── PluginProcessor.cpp ← DSP: Spring Reverb, Tremolo, Drive, LoFi, EQ
    ├── PluginEditor.h      ← GUI header
    └── PluginEditor.cpp    ← Retro GUI: cream knobs, VU meter, presets
```

---

## DSP Architecture

```
Input (stereo)
    │
    ├─► Tube Saturation (tanh soft clip, Drive knob)
    │
    ├─► LoFi Tape (noise injection + tape bandwidth LP filter)
    │
    ├─► Spring Reverb
    │     ├─ 2× Allpass diffusers (blur transients)
    │     └─ 4× Parallel Comb filters w/ LP damping
    │         (feedback ← Decay, damp ← Tone)
    │
    ├─► EQ (Bass shelf 200Hz + Treble shelf 4kHz)
    │
    └─► Tremolo (sine LFO amplitude mod, Speed + Depth)
         │
         ▼
       Output
```

---

## Presets

| Preset | Character |
|--------|-----------|
| **Surf Clean** | Classic Dick Dale tone — long reverb, light tremolo |
| **Dirty Cramps** | Psychobilly garage — heavy drive, fast tremolo, dark |
| **Night Waves** | Dreamy, slow, ethereal |
| **Haunted Motel** | Gloomy, lo-fi, wobbling — like a jukebox in a ghost town |
| **Zen Surf** | Clean, minimal, contemplative |

---

## FL Studio 24 — Quick Setup

1. Copy `GhostSurf.vst3` folder to `C:\Program Files\Common Files\VST3\`
2. FL Studio → `Options → Manage Plugins`
3. Check "VST3" in the search folders, click **Start scan**
4. GhostSurf appears in the Plugin Browser under "Effects"
5. Drag onto any mixer track

---

## Troubleshooting

**"Plugin not found after scan"**  
→ Make sure you copied the entire `GhostSurf.vst3` *folder* (not just the DLL inside).

**Build error: JUCE not found**  
→ Run `git clone https://github.com/juce-framework/JUCE.git` inside the project folder.

**CMake error: generator not found**  
→ Adjust the `-G` flag to match your installed VS version.
