# ramayana-sornprommas
I’m working on a 3D game that brings the Ramayana to life through an interactive experience. My goal is to faithfully represent Khon performances and traditional Thai music, making these art forms accessible to a wider audience. As a Thai musician myself, I’m especially excited about bringing Thai music into the game.

## Lore

All story, character, and Khon performance reference material (scenes, songs, lyrics, masks, choreography) lives in [`lore/`](lore/README.md).

## Hanuman battle demo (Godot 4)

A short playable demo lives in [`game/`](game/): Hanuman leads the vanguard against the demon army and Kampan, the Brahmastra fells Rama's army, then he breaks the necks of Erawan (Indrajit disguised as Indra). Everything is built from primitives in code, so there are no imported assets.

```bash
brew install --cask godot          # Godot 4.7+
godot --path game                  # play
godot -e --path game               # open in the editor
```

| Input | Action |
|---|---|
| WASD / left stick | move |
| Mouse / arrow keys / right stick | camera |
| J / left mouse | trident combo (3 hits) |
| K / right mouse | heavy thrust |
| Space | leap (press J in the air to plunge) |
| Shift | dash |
| Q | นิมิตกาย: grow giant when the blue meter is full |
| R / H / Esc | restart / hide help / release mouse |

Music cues follow `lore/khon-performance/08-music-for-game-audio.md`. The demo plays placeholder ฉิ่ง / ตะโพน / กลองทัด percussion; drop real recordings in `game/audio/music/<cue_id>.ogg` (e.g. `choet_klong.ogg`, `ling_lot.ogg`, `ot_haep.ogg`) and they are used automatically.
