extends RefCounted
## Synthesises placeholder percussion one-shots at startup so the demo has a
## pulse without shipping any recordings. These are NOT recordings of a
## ปี่พาทย์ ensemble: just rough stand-ins for ฉิ่ง (ching / chap), ตะโพน
## (taphon high / low), กลองทัด (klong that), plus a few combat SFX.

const RATE := 22050

## Inharmonic partials for a small bronze cymbal pair (ฉิ่ง), in Hz.
const CHING_PARTIALS := [1187.0, 2764.0, 4031.0, 5322.0, 6911.0]


static func make(kind: String) -> AudioStreamWAV:
	match kind:
		"ching": return _to_wav(_ching(1.3, 0.0))
		"chap": return _to_wav(_ching(0.09, 0.5))
		"taphon_hi": return _to_wav(_drum(430.0, 360.0, 0.22, 0.25))
		"taphon_lo": return _to_wav(_drum(170.0, 128.0, 0.34, 0.12))
		"klong": return _to_wav(_drum(74.0, 52.0, 0.9, 0.3))
		"whoosh": return _to_wav(_whoosh(0.2))
		"hit": return _to_wav(_hit())
		"clang": return _to_wav(_ching(0.35, 0.3, 0.6))
	return _to_wav(PackedFloat32Array([0.0]))


static func _ching(decay: float, noise_amt: float, pitch := 1.0) -> PackedFloat32Array:
	var length := int(RATE * (decay * 2.5 + 0.05))
	var out := PackedFloat32Array()
	out.resize(length)
	var rng := RandomNumberGenerator.new()
	rng.seed = 3
	for i in length:
		var t := float(i) / RATE
		var v := 0.0
		for k in CHING_PARTIALS.size():
			var f: float = CHING_PARTIALS[k] * pitch
			var d := decay * (1.0 - k * 0.12)
			v += sin(TAU * f * t) * exp(-t / d) / (k + 1.5)
		v += rng.randf_range(-1.0, 1.0) * noise_amt * exp(-t / 0.02)
		out[i] = v * 0.55
	return out


static func _drum(f_start: float, f_end: float, decay: float, click: float) -> PackedFloat32Array:
	var length := int(RATE * decay * 3.0)
	var out := PackedFloat32Array()
	out.resize(length)
	var rng := RandomNumberGenerator.new()
	rng.seed = 5
	var phase := 0.0
	for i in length:
		var t := float(i) / RATE
		var f := f_end + (f_start - f_end) * exp(-t / 0.04)
		phase += TAU * f / RATE
		var body := sin(phase) + 0.35 * sin(phase * 1.59) * exp(-t / (decay * 0.4))
		var v := body * exp(-t / decay)
		v += rng.randf_range(-1.0, 1.0) * click * exp(-t / 0.006)
		out[i] = v * 0.7
	return out


static func _whoosh(length_s: float) -> PackedFloat32Array:
	var length := int(RATE * length_s)
	var out := PackedFloat32Array()
	out.resize(length)
	var rng := RandomNumberGenerator.new()
	rng.seed = 9
	var lp := 0.0
	for i in length:
		var u := float(i) / length
		var cutoff := 0.05 + 0.35 * sin(PI * u)
		lp += (rng.randf_range(-1.0, 1.0) - lp) * cutoff
		out[i] = lp * sin(PI * u) * 0.9
	return out


static func _hit() -> PackedFloat32Array:
	var a := _drum(300.0, 140.0, 0.12, 0.8)
	var n := _ching(0.05, 0.9, 0.7)
	var length := maxi(a.size(), n.size())
	var out := PackedFloat32Array()
	out.resize(length)
	for i in length:
		var v := 0.0
		if i < a.size():
			v += a[i]
		if i < n.size():
			v += n[i] * 0.5
		out[i] = v
	return out


static func _to_wav(samples: PackedFloat32Array) -> AudioStreamWAV:
	var bytes := PackedByteArray()
	bytes.resize(samples.size() * 2)
	for i in samples.size():
		bytes.encode_s16(i * 2, int(clampf(samples[i], -1.0, 1.0) * 32767.0))
	var w := AudioStreamWAV.new()
	w.format = AudioStreamWAV.FORMAT_16_BITS
	w.mix_rate = RATE
	w.stereo = false
	w.data = bytes
	return w
