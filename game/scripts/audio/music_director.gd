extends Node
## Music cue system. Each game state names the Khon piece that would
## accompany it (mapping from lore/khon-performance/08-music-for-game-audio.md).
##
## If a recording exists at res://audio/music/<cue_id>.ogg it is looped.
## Otherwise a placeholder ฉิ่ง/ตะโพน/กลองทัด pulse is played at the cue's tempo.
## The placeholder patterns are generic and are NOT the real หน้าทับ of each piece.
## Exposes `beat_phase` so dancers can sink (ยืดยุบ) on the beat.

signal cue_changed(cue_id: String)
signal beat(index: int, is_chap: bool)

const Synth := preload("res://scripts/audio/percussion_synth.gd")
const MUSIC_DIR := "res://audio/music/"
const TICKS_PER_BEAT := 2

const CUES := {
	"wa": {"th": "เพลงวา", "rom": "WA", "bpm": 66, "pattern": "slow", "use": "หน้าชื่อบท / title"},
	"kraw_nok": {"th": "กราวนอก", "rom": "KRAW NOK", "bpm": 96, "pattern": "kraw", "use": "ยกทัพฝ่ายพระราม / Rama's army marches"},
	"choet": {"th": "เชิด", "rom": "CHOET", "bpm": 150, "pattern": "choet", "use": "รบ / battle"},
	"choet_klong": {"th": "เชิดกลอง", "rom": "CHOET KLONG", "bpm": 168, "pattern": "choet_klong", "use": "รบ / battle (intense)"},
	"choet_ching": {"th": "เชิดฉิ่ง", "rom": "CHOET CHING", "bpm": 140, "pattern": "ching_only", "use": "แผลงศร / loosing an arrow"},
	"ling_lot": {"th": "ลิงโลด", "rom": "LING LOT", "bpm": 120, "pattern": "kraw", "use": "หนุมานโกรธพระอินทร์แปลง"},
	"ot_haep": {"th": "โอดแหบ", "rom": "OT HAEP", "bpm": 50, "pattern": "slow", "use": "หนุมานสลบ / Hanuman falls"},
	"khaek_bora_thet": {"th": "แขกบรเทศ", "rom": "KHAEK BORATHET", "bpm": 108, "pattern": "kraw", "use": "ฟื้นคืน / revival"},
	"kraw_ram_phama": {"th": "กราวรำพม่า", "rom": "KRAW RAM PHAMA", "bpm": 100, "pattern": "kraw", "use": "ฝ่ายยักษ์ชนะ / demons triumph"},
}

## 8 ticks = 4 beats. C = ฉิ่ง (open), X = ฉับ (closed), T/t = ตะโพน high/low, K = กลองทัด.
const PATTERNS := {
	"slow": ["C K", "", "X", "", "C", "", "X t", ""],
	"kraw": ["C t", "T", "X T", "", "C t", "T", "X K", ""],
	"choet": ["C T", "t", "X T", "t", "C T", "t", "X K", "t"],
	"choet_klong": ["C K", "T", "X K", "T", "C K", "T", "X K", "T t"],
	"ching_only": ["C", "", "C", "", "C", "", "X", ""],
}

var current := ""
var beat_phase := 0.0
var beat_count := 0
var using_recording := false

var _acc := 0.0
var _tick := 0
var _voices: Dictionary = {}
var _sfx_pool: Array[AudioStreamPlayer] = []
var _sfx_next := 0
var _sfx_streams: Dictionary = {}
var _music_player: AudioStreamPlayer


func _ready() -> void:
	add_to_group("music")
	process_mode = Node.PROCESS_MODE_ALWAYS
	for kind in ["ching", "chap", "taphon_hi", "taphon_lo", "klong"]:
		var pl := AudioStreamPlayer.new()
		pl.stream = Synth.make(kind)
		pl.max_polyphony = 4
		pl.volume_db = -6.0 if kind.begins_with("ch") else -3.0
		add_child(pl)
		_voices[kind] = pl
	for kind in ["whoosh", "hit", "clang", "klong", "taphon_hi"]:
		_sfx_streams[kind] = Synth.make(kind)
	for i in 10:
		var p := AudioStreamPlayer.new()
		add_child(p)
		_sfx_pool.append(p)
	_music_player = AudioStreamPlayer.new()
	_music_player.volume_db = -4.0
	add_child(_music_player)


func cue_info(cue_id: String = "") -> Dictionary:
	var id := current if cue_id == "" else cue_id
	return CUES.get(id, {})


func play_cue(cue_id: String) -> void:
	if cue_id == current or not CUES.has(cue_id):
		return
	current = cue_id
	_acc = 0.0
	_tick = 0
	var path := MUSIC_DIR + cue_id + ".ogg"
	using_recording = FileAccess.file_exists(path)
	if using_recording:
		var stream := AudioStreamOggVorbis.load_from_file(path)
		if stream != null:
			stream.loop = true
			_music_player.stream = stream
			_music_player.play()
		else:
			push_warning("Could not load recording: %s" % path)
			using_recording = false
	else:
		_music_player.stop()
	cue_changed.emit(cue_id)


func stop() -> void:
	current = ""
	_music_player.stop()


func sfx(kind: String, pitch := 1.0, volume_db := 0.0) -> void:
	if not _sfx_streams.has(kind):
		return
	var p := _sfx_pool[_sfx_next]
	_sfx_next = (_sfx_next + 1) % _sfx_pool.size()
	p.stream = _sfx_streams[kind]
	p.pitch_scale = pitch * randf_range(0.94, 1.06)
	p.volume_db = volume_db
	p.play()


func _process(delta: float) -> void:
	if current == "":
		return
	var info: Dictionary = CUES[current]
	var tick_len := 60.0 / float(info["bpm"]) / TICKS_PER_BEAT
	# Music keeps real time even during hit-stop / slow motion.
	_acc += delta / maxf(Engine.time_scale, 0.001)
	while _acc >= tick_len:
		_acc -= tick_len
		_on_tick(info)
	var beat_len := tick_len * TICKS_PER_BEAT
	var within := float(posmod(_tick - 1, TICKS_PER_BEAT)) * tick_len + _acc
	beat_phase = clampf(within / beat_len, 0.0, 1.0)


func _on_tick(info: Dictionary) -> void:
	var pattern: Array = PATTERNS[info["pattern"]]
	var step: String = pattern[_tick % pattern.size()]
	if _tick % TICKS_PER_BEAT == 0:
		beat_count += 1
		beat.emit(beat_count, step.contains("X"))
	_tick += 1
	if using_recording:
		return
	for token in step.split(" ", false):
		match token:
			"C": _hit_voice("ching")
			"X": _hit_voice("chap")
			"T": _hit_voice("taphon_hi")
			"t": _hit_voice("taphon_lo")
			"K": _hit_voice("klong")


func _hit_voice(kind: String) -> void:
	var p: AudioStreamPlayer = _voices[kind]
	p.play()
