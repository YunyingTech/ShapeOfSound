/*
录音
https://github.com/xiangyuecn/Recorder
src: engine/pcm.js
*/
!
function() {
	"use strict";
	Recorder.prototype.enc_pcm = {
		stable: !0,
		testmsg: "pcm为未封装的原始音频数据，pcm数据文件无法直接播放；支持位数8位、16位（填在比特率里面），采样率取值无限制"
	},
	Recorder.prototype.pcm = function(e, t, r) {
		var a = this.set,
		n = e.length,
		o = 8 == a.bitRate ? 8 : 16,
		c = new ArrayBuffer(n * (o / 8)),
		s = new DataView(c),
		l = 0;
		if (8 == o) for (var p = 0; p < n; p++, l++) {
			var i = 128 + (e[p] >> 8);
			s.setInt8(l, i, !0)
		} else for (p = 0; p < n; p++, l += 2) s.setInt16(l, e[p], !0);
		t(new Blob([s.buffer], {
			type: "audio/pcm"
		}))
	},
	Recorder.pcm2wav = function(e, a, n) {
		e.slice && null != e.type && (e = {
			blob: e
		});
		var o = e.sampleRate || 16e3,
		c = e.bitRate || 16;
		if (e.sampleRate && e.bitRate || console.warn("pcm2wav必须提供sampleRate和bitRate"), Recorder.prototype.wav) {
			var s = new FileReader;
			s.onloadend = function() {
				var e;
				if (8 == c) {
					var t = new Uint8Array(s.result);
					e = new Int16Array(t.length);
					for (var r = 0; r < t.length; r++) e[r] = t[r] - 128 << 8
				} else e = new Int16Array(s.result);
				Recorder({
					type: "wav",
					sampleRate: o,
					bitRate: c
				}).mock(e, o).stop(function(e, t) {
					a(e, t)
				},
				n)
			},
			s.readAsArrayBuffer(e.blob)
		} else n("pcm2wav必须先加载wav编码器wav.js")
	}
} ();