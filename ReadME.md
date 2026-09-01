# COUNTED [TRUST NO ONE]

*A Retro Dystopian Immersive Action/Stealth Game built on the CyberVGA Software 3D Engine.*

Developed by **Novus Idea** for Brackeys Game Jam 2026.2 (Theme: *Trust No One*).

---

## 📖 Premise & Lore

You are **Unit 9**, an enforcer of the Directorate deployed into **Sector 9**. 

The Directorate claims that all quotas are met, all supplies are accounted for, and every citizen is counted. However, physical evidence indicates that workers were deprived, quotas were falsified, and unrest is brewing beneath the surface.

Throughout the mission, you will encounter the Directorate's propaganda broadcasts, moral illusions under institutional lighting, and critical choices that alter the sector's history forever.

---

## 🎮 Core Mechanics & Features

- **Software-Rendered 3D Engine**: Built on **CyberVGA**, featuring 320×240 paletted rasterization, Q16.16 fixed-point math, 8-angle character billboard sprites, and brush-to-octree level geometry.
- **Atmospheric "Warm vs. Cold Light" Morality System**:
  - **Warm Light (Institutional Comfort / Lies)**: The Directorate's radar and HUD report false counts (+5 ammo illusion, false hostile pings, ambush medkit traps).
  - **Cold Light (Objective Truth)**: Exposes real ammo counts, true enemy locations, and physical discrepancies.
- **Stealth, Combat & Tactical Navigation**:
  - Suppressed pistol with hit-stun flinching and localized audio propagation.
  - A* pathfinding enforcer AI with state machine awareness.
  - Enemy ammo drops and tactical supply caches.
- **Three Unique Endings**:
  - **Ending A — "Reliable"**: Obey the Directorate, burn the ledger, and detain the laborer. The civic record wall becomes blank.
  - **Ending B — "Untrustworthy"**: Surrender your access card to the laborer, trigger the Directorate alarm, withstand Unit 4's betrayal, and escape through the service tunnel. The civic record proclaims: *"THE RECORD SURVIVES"*.
  - **Ending C — "Successor"**: Seize the Continuity Terminal using physical ledger leverage, sever the leadership broadcast, and claim control. The civic record proclaims: *"ONLY THE AUTHOR COUNTS"*.
- **Quality of Life & Settings**:
  - Full In-Game Settings menu with Mouse Sensitivity adjustments and Master Volume control.
  - Authentic Retro CRT Monitor Scanlines, Chromatic Optics, and Phosphor Smear filter toggles.
  - Checkpoint Autosave & "Resume Duty" system.

---

## 🕹️ Controls

| Action | Key / Input |
| :--- | :--- |
| **Move / Strafe** | `W`, `A`, `S`, `D` |
| **Look & Aim** | `Mouse` |
| **Fire Suppressed Pistol** | `Left Mouse Button` |
| **Interact / Open Doors / Pickups** | `E` |
| **Pause Sector Operation** | `ESC` or `P` |
| **Options & Configuration** | `O` |
| **Assistance Manual** | `H` |
| **Fast Forward Credits** | `SPACE` |

---

## 🛠️ Building & Running

### Windows (Win64)
1. Open an **x64 Native Tools Command Prompt for VS 2022**.
2. Run the Release build script:
   ```cmd
   BuildReleaseWin64.bat
   ```
3. Run the executable from the project root:
   ```cmd
   BIN\COUNTED_WIN64.exe
   ```

### MS-DOS (DJGPP / DOSBox-X)
1. Launch via DOSBox-X using the pre-configured runner:
   ```cmd
   RUN_DOS.bat
   ```
2. Compile inside DOS using:
   ```cmd
   JACKINDOS.BAT TALLY
   ```
3. Run the executable from the project root in DOS:
   ```cmd
   BIN\CONTD.EXE
   ```

---

## 👥 Credits & Attributions

- **Programming & Architecture**: Burak Yazar
- **Narrative, Design & Music**: Berk Yazar
- **Engine**: CyberVGA 1.1.0 (Win64 & MS-DOS)
- **Audio Subsystem**: Sound Blaster 16 (SMIX) & OpenAL Soft
- **Character Models & Rigs**: Quaternius & Max Parata
- **Audio Assets**: Pixabay & Tunetank (Creative Commons / Royalty Free)

---
*© 2026 Novus Idea. All rights reserved.*

