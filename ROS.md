# ROS — Robotic Operating System

A web-based **Android 16** experience, built as a single self-contained HTML file
(`ros.html`). No build step, no dependencies — open it in any modern browser.

> ℹ️ A literal AOSP / Android 16 build (the real OS) needs tens of gigabytes of
> source and hours of compilation, which isn't possible in this environment.
> ROS instead recreates the **Android 16 look, feel and behavior** (Material 3
> Expressive / "Material You") as an interactive emulator you can actually use.

## Run it

```bash
# Just open the file:
xdg-open ros.html        # Linux
open ros.html            # macOS
# …or serve it:
python3 -m http.server 8000   # then visit http://localhost:8000/ros.html
```

## Features

- **Boot animation** → **Lock screen** (live clock, weather, notifications, swipe to unlock)
- **Home screen** with at-a-glance widget, app grid, Google-style search pill and dock
- **App drawer** with live search (swipe up from the bottom)
- **Notification shade + Quick Settings** (swipe down from the top): toggle tiles,
  brightness slider, dismissable notifications
- **Recent apps** overview
- **Power menu** (screenshot / restart / power off)
- **Material You theming** — tap a color swatch in *Settings → Wallpaper & style*
  to recolor the entire system instantly (6 dynamic palettes)

### Working apps

| App | What it does |
|-----|--------------|
| 📞 Phone | Functional dialer + call action |
| 💬 Messages | Conversation threads with auto-replies |
| 👤 Contacts | Contact list, tap to call |
| 🧮 Calculator | Fully working calculator |
| ⏰ Clock | Live time + world clocks |
| ⛅ Weather | Hourly + weekly forecast |
| 🌐 Chrome | Real embedded browser (iframe) |
| 📝 Notes | Create/delete notes (saved in `localStorage`) |
| 📷 Camera | Shutter + flash animation |
| 🖼️ Photos | Gallery grid |
| 🎵 Music | Now-playing screen with controls |
| ✨ Gemini | Built-in AI-style assistant |
| ⚙️ Settings | Connectivity, theming, About phone |
| 🛍️ Store / 📁 Files / 📅 Calendar / 🗺️ Maps … | Demo surfaces |

## Gestures (touch) & keyboard (desktop)

- **Swipe up** on lock screen → unlock
- **Swipe down** from the top → notification shade
- **Swipe up** from the bottom (on home) → app drawer
- **Esc** → go back / close the current overlay

## Tech

- Pure HTML + CSS + vanilla JavaScript, ~one file
- CSS custom properties drive the dynamic Material You theme engine
- Notes persist via `localStorage`

System info reported by ROS: *Android 16 · API level 36 · Material 3 Expressive ·
kernel 6.18.5-ros-android · build `ROS.16.0`.*
